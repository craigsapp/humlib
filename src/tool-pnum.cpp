//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Wed Apr 10 08:59:49 EDT 2019
// Last Modified: Wed Apr 10 10:21:32 EDT 2019
// Filename:      tool-pnum.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-pnum.cpp
// Syntax:        C++11; humlib
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Extract numeric pitch data from **kern data.
//
//

#include "tool-pnum.h"
#include "Convert.h"
#include "HumRegex.h"

#include <algorithm>
#include <cmath>

using namespace std;

namespace hum {

// START_MERGE


/////////////////////////////////
//
// Tool_gridtest::Tool_pnum -- Set the recognized options for the tool.
//

Tool_pnum::Tool_pnum(void) {
	define("b|base=i:midi",      "numeric base of pitch to extract");
	define("D|no-duration=b",    "do not include duration");
	define("c|pitch-class=b",    "give numeric pitch-class rather than pitch");
	define("o|octave=b",         "give octave rather than pitch");
	define("r|rest=s:0",         "representation string for rests");
	define("R|no-rests=b",       "do not include rests in conversion");
	define("x|attacks-only=b",   "only mark lines with note attacks");
}



///////////////////////////////
//
// Tool_pnum::run -- Primary interfaces to the tool.
//

bool Tool_pnum::run(HumdrumFileSet& infiles) {
	bool status = true;
	for (int i=0; i<infiles.getCount(); i++) {
		status &= run(infiles[i]);
	}
	return status;
}


bool Tool_pnum::run(const string& indata, ostream& out) {
	HumdrumFile infile(indata);
	return run(infile, out);
}


bool Tool_pnum::run(HumdrumFile& infile, ostream& out) {
	bool status = run(infile);
	out << infile;
	return status;
}


bool Tool_pnum::run(HumdrumFile& infile) {
   initialize(infile);
	processFile(infile);
	infile.createLinesFromTokens();
	return true;
}



//////////////////////////////
//
// Tool_pnum::initialize --
//

void Tool_pnum::initialize(HumdrumFile& infile) {
	m_midiQ = false;
	if (getString("base") == "midi") {
		m_base = 12;
		m_midiQ = true;
	} else {
		// check base for valid numbers, but for now default to 12 if unknown
		m_base = getInteger("base");
	}

	m_durationQ = !getBoolean("no-duration");
	m_classQ    =  getBoolean("pitch-class");
	m_octaveQ   =  getBoolean("octave");
	m_attacksQ  =  getBoolean("attacks-only");
	m_rest      =  getString("rest");
	m_restQ     = !getBoolean("no-rests");
}



//////////////////////////////
//
// Tool_pnum::processFile --
//

void Tool_pnum::processFile(HumdrumFile& infile) {
	vector<HTp> kex;

	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].hasSpines()) {
			continue;
		}
		for (int j=0; j<infile[i].getFieldCount(); j++) {
			HTp token = infile.token(i, j);
			if (!token->isKern()) {
				continue;
			}
			if (*token == "**kern") {
				kex.push_back(token);
				continue;
			}
			if (!token->isData()) {
				continue;
			}
			if (token->isNull()) {
				continue;
			}
			convertTokenToBase(token);
		}
	}

	string newex;
	for (int i=0; i<(int)kex.size(); i++) {
		if (m_midiQ) {
			newex = "**pmid";
		} else {
			newex = "**b" + to_string(m_base);
		}
		kex[i]->setText(newex);
	}
}



//////////////////////////////
//
// Tool_pnum::convertTokenToBase --
//

void Tool_pnum::convertTokenToBase(HTp token) {
	string output;
	int scount = token->getSubtokenCount();
	for (int i=0; i<scount; i++) {
		string subtok = token->getSubtoken(i);
		output += convertSubtokenToBase(subtok);
		if (i < scount - 1) {
			output += " ";
		}
	}
	token->setText(output);
}



//////////////////////////////
//
// Tool_pnum::convertSubtokenToBase --
//

string Tool_pnum::convertSubtokenToBase(const string& text) {
	int pitch = 0;
	if (text.find("r") == string::npos) {
		switch (m_base) {
			case 7:
				pitch = Convert::kernToBase7(text);
				break;
			case 40:
				pitch = Convert::kernToBase40(text);
				break;
			default:
				pitch = Convert::kernToBase12(text);
		}
	} else if (!m_restQ) {
		return ".";
	}
	string recip;
	if (m_durationQ) {
		HumRegex hre;
		if (hre.search(text, "(\\d+%?\\d*\\.*)")) {
			recip = hre.getMatch(1);
		}
	}

	string output;

	int pc = pitch % m_base;
	int oct = pitch / m_base;

	if (m_midiQ) {
		// MIDI numbers use 5 for middle-C octave.
		pitch += 12;
	}

	int tie = 1;
	if (text.find("_") != string::npos) {
		tie = -1;
	}
	if (text.find("]") != string::npos) {
		tie = -1;
	}
	pitch *= tie;
	if (m_attacksQ && pitch < 0) {
		return ".";
	}

	if (m_durationQ) {
		output += recip;
		output += "/";
	}
	
	if (text.find("r") != string::npos) {
		output += m_rest;
	} else {
		if (!m_octaveQ && !m_classQ) {
			output += to_string(pitch);
		} else {
			if (m_classQ) {
				if (pitch < 0) {
					output += "-";
				}
				output += to_string(pc);
			}
			if (m_classQ && m_octaveQ) {
				output += ":";
			}
			if (m_octaveQ) {
				output += to_string(oct);
			}
		}
	}

	return output;
}



// END_MERGE

} // end namespace hum



