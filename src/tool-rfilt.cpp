//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Fri Oct 10 09:56:34 PDT 2025
// Last Modified: Thu Oct 30 09:27:00 PDT 2025
// Filename:      tool-rfilt.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/tool-rfilt.cpp
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Switch between rfilt encoding and corrected encoding.
//

#include "tool-rfilt.h"
#include "Convert.h"
#include "HumRegex.h"
#include <iostream>

using namespace std;

namespace hum {

// START_MERGE


/////////////////////////////////
//
// Tool_rfilt::Tool_rfilt -- Set the recognized options for the tool.
//

Tool_rfilt::Tool_rfilt(void) {
	define("k|kern=s", "process only the given spines");
	define("m|midi=s", "process only the given midi pitches");
}



/////////////////////////////////
//
// Tool_rfilt::run -- Do the main work of the tool.
//

bool Tool_rfilt::run(HumdrumFileSet& infiles) {
	bool status = true;
	for (int i = 0; i < infiles.getCount(); i++) {
		status &= run(infiles[i]);
	}
	return status;
}


bool Tool_rfilt::run(const string& indata, ostream& out) {
	HumdrumFile infile(indata);
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_rfilt::run(HumdrumFile& infile, ostream& out) {
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_rfilt::run(HumdrumFile& infile) {
	initialize(infile);
	processFile(infile);
	return true;
}



//////////////////////////////
//
// Tool_rfilt::initialize --  Initializations that only have to be done once
//    for all HumdrumFile segments.
//

void Tool_rfilt::initialize(HumdrumFile& infile) {
	if (getBoolean("kern")) {
		string klist = getString("kern");
		infile.makeBooleanTrackList(m_spines, klist);
		for (int z = 0; z < (int)infile.getMaxTrack(); z++) {
			cout << "\t" << m_spines.at(z) << endl;
		}
	} else {
		m_spines.resize(infile.getMaxTrack());
		fill(m_spines.begin(), m_spines.end(), true);
	}

	m_modifiedQ = false;

	cout << "GOT HERE AA" << endl;

	if (getBoolean("midi")) {
		string midi = getString("midi");
		infile.makeBooleanTrackList(m_midi, midi);
		Convert::makeBooleanTrackList(m_midi, midi, 128);
		for (int i = 0; i < (int)m_midi.size(); i++) {
			cout << "MIDI " << i << "=\t" << m_midi[i] << endl;
		}
	} else {
		m_midi.resize(128);
		fill(m_midi.begin(), m_midi.end(), true);
	}
}



//////////////////////////////
//
// Tool_rfilt::processFile --
//

void Tool_rfilt::processFile(HumdrumFile& infile) {
	vector<HTp> starts;
	infile.getSpineStartList(starts);
	for (int i = 0; i < (int)starts.size(); i++) {
		if (!starts[i]->isKern()) continue;

		int track = starts[i]->getTrack();
		if (!m_spines.at(track - 1)) continue;

		processSpine(starts[i]);
	}

	infile.createLinesFromTokens();
	m_humdrum_text << infile;
}



//////////////////////////////
//
// Tool_rfilt::processSpine --
//

void Tool_rfilt::processSpine(HTp token) {
	HTp current = token;
	if (token->isNull()) return;

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

		cerr << "CURRENT: " << current << endl;
		vector<string> subtoks = current->getSubtokens();
		cerr << "SIZE OF SUBTOKEN: " << subtoks.size() << endl;

		string newtoken;

		for (int i = 0; i < (int)subtoks.size(); i++) {
			string temp;

			if (isDelete(subtoks[i])) {
				// Convert this subtoken to a rest with the same duration
				temp = addRest(subtoks[i]);
				cerr << "DELETE i=" << i << ":\t" << subtoks[i] << " -> " << temp << endl;
			} else {
				// Keep original subtoken as-is
				temp = subtoks[i];
			}

			cerr << "SUBROK i=" << i << ":\t" << temp << endl;

			if (!newtoken.empty()) newtoken += " ";
			newtoken += temp;
		}

		// Only update token text if it changed
		if (*current != newtoken) {
			current->setText(newtoken);
		}

		current = current->getNextToken();
	}
}



//////////////////////////////
//
// Tool_rfilt::isDelete --
//

bool Tool_rfilt::isDelete(string& value) {
	int midikey = Convert::kernToMidiNoteNumber(value);
	return m_midi[midikey];
}



//////////////////////////////
//
// Tool_rfilt::addRest --
//

string Tool_rfilt::addRest(const string& input) {
	cerr << "INPUT = " << input << endl;
	HumRegex hre;
	string output = input;
	hre.replaceDestructive(output, "", "[#n-]", "g");
	hre.replaceDestructive(output, "r", "[A-G]+", "gi");
	cerr << "OUTPUT= " << output << endl;
	return output;
}

// END_MERGE

} // end namespace hum

