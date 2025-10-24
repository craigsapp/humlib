//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Fri Oct 10 09:56:34 PDT 2025
// Last Modified: Mon Oct 13 09:07:29 PDT 2025
// Filename:      tool-restit.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/tool-restit.cpp
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Switch between restit encoding and corrected encoding.
//

#include "tool-restit.h"
#include "Convert.h"
#include "HumRegex.h"
#include <iostream>

using namespace std;

namespace hum {

// START_MERGE


/////////////////////////////////
//
// Tool_restit::Tool_restit -- Set the recognized options for the tool.
//

Tool_restit::Tool_restit(void) {
	define("k|kern=s", "process only the given spines");
	define("m|midi=s", "process only the given midi pitches");

}



/////////////////////////////////
//
// Tool_restit::run -- Do the main work of the tool.
//

bool Tool_restit::run(HumdrumFileSet& infiles) {
	bool status = true;
	for (int i=0; i<infiles.getCount(); i++) {
		status &= run(infiles[i]);
	}
	return status;
}


bool Tool_restit::run(const string& indata, ostream& out) {
	HumdrumFile infile(indata);
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_restit::run(HumdrumFile& infile, ostream& out) {
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_restit::run(HumdrumFile& infile) {
	initialize(infile);
	processFile(infile);
	return true;
}



//////////////////////////////
//
// Tool_restit::initialize --  Initializations that only have to be done once
//    for all HumdrumFile segments.
//

void Tool_restit::initialize(HumdrumFile& infile) {
	if (getBoolean("kern")) {
		string klist = getString("kern");
		infile.makeBooleanTrackList(m_spines, klist);
		for (int z=0; z<(int)infile.getMaxTrack(); z++) {
		cout << "\t" << m_spines.at(z) << endl;
	}
	} else {
		m_spines.resize(infile.getMaxTrack());
		fill(m_spines.begin(), m_spines.end(), true);
	}
	m_modifiedQ = false;
	if (getBoolean("midi")) {
		string midi = getString("midi");
		infile.makeBooleanTrackList(m_midi, midi);
		Convert::makeBooleanTrackList(m_midi, midi, 128);

	} else {
		m_midi.resize(128);
		fill(m_midi.begin(), m_midi.end(), true);
	}

}



//////////////////////////////
//
// Tool_restit::processFile --
//

void Tool_restit::processFile(HumdrumFile& infile) {
	vector<HTp> starts;
	infile.getSpineStartList(starts);
	for (int i=0; i<(int)starts.size(); i++) {
		if (!starts[i]->isKern()) {
			continue;
		}
		int track = starts[i]->getTrack();
		cerr << "TRACK: " << track << endl;
		if (!m_spines.at(track-1)) {
			continue;
		}
		processSpine(starts[i]);
	}

	infile.createLinesFromTokens();
	m_humdrum_text << infile;
}


///////////////////////////////
//
// Tool_restit::processSpine --
//

void Tool_restit::processSpine(HTp token) {
	HTp current = token;
	if (token->isNull()) {
		return;
	}
	int spine = token->getTrack() - 1;
	if (!m_spines.at(spine)) {
		current = current->getNextToken();
		return;
	}
	while (current) {
		if (!current->isData()) {
			current = current->getNextToken();
			continue;
		}
		vector<string>subtokens = current->getSubtokens();
		for (int z=0; z<(int)subtokens.size(); z++) {
			subtokens[z] = filterNote(subtokens[z]);
		} 
		string newtoken = "";
		if (!subtokens.empty()) {
			newtoken += subtokens[0];
		} else {
			current = current->getNextToken();
			continue;
		}
		for (int i=1; i<(int)subtokens.size(); i++) {
			newtoken += " ";
			newtoken += subtokens[i];
		}
		if (*current == newtoken) {
			current = current->getNextToken();
			continue;
		} else {
		}
		current->setText(newtoken);
		current = current->getNextToken();
	}
	
}


//////////////////////////////
//
// Tool_restit::filterNote --
//

string Tool_restit::filterNote(string& value) {
	int midikey = Convert::kernToMidiNoteNumber(value);
	midikey = midikey == m_midi[midikey]? 0: 1;
	string output = to_string(midikey);
	return output;
}

// END_MERGE

} // end namespace hum



