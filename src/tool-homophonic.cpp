//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Fri Aug  9 17:58:05 EDT 2019
// Last Modified: Fri Aug  9 17:58:08 EDT 2019
// Filename:      tool-homophonic.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/tool-homophonic.cpp
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Identify homophonic regions of music.
//

#include "tool-homophonic.h"
#include "Convert.h"

using namespace std;

namespace hum {

// START_MERGE


/////////////////////////////////
//
// Tool_homophonic::Tool_homophonic -- Set the recognized options for the tool.
//

Tool_homophonic::Tool_homophonic(void) {
	define("a|append=b", "Append analysis to end of input data");
	define("p|prepend=b", "Prepend analysis to end of input data");
	define("M|no-marks=b", "Do not mark homophonic section notes");
	define("n=i:4", "number of sonorities in a row required to define homophonic");
}



/////////////////////////////////
//
// Tool_homophonic::run -- Do the main work of the tool.
//

bool Tool_homophonic::run(HumdrumFileSet& infiles) {
	bool status = true;
	for (int i=0; i<infiles.getCount(); i++) {
		status &= run(infiles[i]);
	}
	return status;
}


bool Tool_homophonic::run(const string& indata, ostream& out) {
	HumdrumFile infile;
	infile.readStringNoRhythm(indata);
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_homophonic::run(HumdrumFile& infile, ostream& out) {
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_homophonic::run(HumdrumFile& infile) {
	initialize();
	processFile(infile);

	infile.createLinesFromTokens();
	return true;
}



//////////////////////////////
//
// Tool_homophonic::markHomophonicNotes --
//

void Tool_homophonic::markHomophonicNotes(void) {
	

}



//////////////////////////////
//
// Tool_homophonic::initialize --
//

void Tool_homophonic::initialize(void) {
	m_count = getInteger("n");
	if (m_count < 1) {
		m_count = 1;
	}
}



//////////////////////////////
//
// Tool_homophonic::processFile --
//

void Tool_homophonic::processFile(HumdrumFile& infile) {
	vector<int> data;
	data.reserve(infile.getLineCount());

	m_homophonic.clear();
	m_homophonic.resize(infile.getLineCount());

	m_notecount.clear();
	m_notecount.resize(infile.getLineCount());
	fill(m_notecount.begin(), m_notecount.end(), 0);

	m_attacks.clear();
	m_attacks.resize(infile.getLineCount());
	fill(m_attacks.begin(), m_attacks.end(), 0);

	m_notes.clear();
	m_notes.resize(infile.getLineCount());

	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isData()) {
			continue;
		}
		data.push_back(i);
		analyzeLine(infile, i);
	}

	// change Y N Y patterns to Y Y Y
	for (int i=1; i<data.size() - 1; i++) {
		if (m_homophonic[data[i]] == "Y") {
			continue;
		}
		if (m_homophonic[data[i+1]] == "N") {
			continue;
		}
		if (m_homophonic[data[i-1]] == "N") {
			continue;
		}
		m_homophonic[data[i]] = "NY";
	}

	vector<double> score(infile.getLineCount(), 0);

	double sum;
	for (int i=0; i<(int)data.size(); i++) {
		if (m_homophonic[data[i]].find("Y") != string::npos) {
			if (m_homophonic[data[i]].find("N") != string::npos) {
				sum += 0.5;
			} else {
				sum += 1;
			}
		} else {
			sum = 0;
		}
		score[data[i]] = sum;
	}

	for (int i=(int)data.size()-2; i>=0; i--) {
		if (score[data[i]] == 0) {
			continue;
		}
		if (score[data[i+1]] > score[data[i]]) {
			score[data[i]] = score[data[i+1]];
		}
	}

	for (int i=0; i<(int)data.size(); i++) {
		if (score[data[i]] > m_count) {
			if (m_attacks[data[i]] < (int)m_notes[data[i]].size() - 1) {
				m_homophonic[data[i]] = "chartreuse";
			} else {
				m_homophonic[data[i]] = "red";
			}
		} else {
			m_homophonic[data[i]] = "black";
		}
	}
	
	//if (getBoolean("append")) {
		infile.appendDataSpine(m_homophonic, "", "**color");
	//} else if (getBoolean("prepend")) {
	//	infile.prependDataSpine(m_homophonic, "", "**color");
	//}
	//if (!getBoolean("no-marks")) {
	//	markHomophonicNotes();
	//}
}


//////////////////////////////
//
// Tool_homophonic::analyzeLine --
//

void Tool_homophonic::analyzeLine(HumdrumFile& infile, int line) {
	m_notes[line].reserve(10);
	HPNote note;
	if (!infile[line].isData()) {
		return;
	}
	int nullQ = 0;
	for (int i=0; i<infile[line].getFieldCount(); i++) {
		HTp token = infile.token(line, i);
		if (!token->isKern()) {
			continue;
		}
		if (token->isRest()) {
			continue;
		}
		if (token->isNull()) {
			nullQ = 1;
			token = token->resolveNull();
			if (!token) {
				continue;
			}
			if (token->isRest()) {
				continue;
			}
		} else {
			nullQ = 0;
		}
		int track = token->getTrack();
		vector<string> subtokens = token->getSubtokens();
		for (int j=0; j<(int)subtokens.size(); j++) {
			note.track = track;
			note.line = token->getLineIndex();
			note.field = token->getFieldIndex();
			note.subfield = j;
			note.token = token;
			note.text = subtokens[j];
			if (nullQ) {
				note.attack = false;
				note.nullQ = true;
			} else {
				note.nullQ = false;
				if ((note.text.find("_") != string::npos) || 
				    (note.text.find("]") != string::npos)) {
					note.attack = false;
				} else {
					note.attack = true;
				}
			}
			m_notes[line].push_back(note);
		}
	}

	// There must be at least notecount - 1 attacks to be considered homophonic
	for (int i=0; i<(int)m_notes[line].size(); i++) {
		if (m_notes[line][i].attack) {
			m_attacks[line]++;
		}
	}
	if ((int)m_attacks[line] >= (int)m_notes[line].size() - 1) {
		string value = "Y";
		// value += to_string(m_attacks[line]);
		m_homophonic[line] = value;
	} else {
		string value = "N";
		// value += to_string(m_attacks[line]);
		m_homophonic[line] = value;
	}
	if (m_notes[line].size() <= 2) {
		m_homophonic[line] = "N";
	}
}


// END_MERGE

} // end namespace hum



