//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Wed Sep  9 22:03:46 PDT 2020
// Last Modified: Wed Sep  9 22:03:49 PDT 2020
// Filename:      tool-colortriads.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/tool-colortriads.cpp
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Switch between colortriads encoding and corrected encoding.
//

#include "tool-colortriads.h"
#include "tool-msearch.h"
#include "Convert.h"
#include "HumRegex.h"

using namespace std;

namespace hum {

// START_MERGE


/////////////////////////////////
//
// Tool_colortriads::Tool_colortriads -- Set the recognized options for the tool.
//

Tool_colortriads::Tool_colortriads(void) {
	define("A=b",            "do not color triads with diatonic A root");
	define("B=b",            "do not color triads with diatonic B root");
	define("C=b",            "do not color triads with diatonic C root");
	define("D=b",            "do not color triads with diatonic D root");
	define("E=b",            "do not color triads with diatonic E root");
	define("F=b",            "do not color triads with diatonic F root");
	define("G=b",            "do not color triads with diatonic G root");
	define("a=s:darkviolet", "color for A triads");
	define("b=s:darkorange", "color for B triads");
	define("c=s:limegreen",  "color for C triads");
	define("d=s:royalblue",  "color for D triads");
	define("e=s:crimson",    "color for E triads");
	define("f=s:gold",       "color for F triads");
	define("g=s:skyblue",    "color for G triads");
	define("r|relative=b",   "functional coloring (green = key tonic)");
	define("k|key=s",        "key to transpose coloring to");
	define("commands=b",     "print msearch commands only");
	define("filters=b",      "print filter commands only");
}



/////////////////////////////////
//
// Tool_colortriads::run -- Do the main work of the tool.
//

bool Tool_colortriads::run(HumdrumFileSet& infiles) {
	bool status = true;
	for (int i=0; i<infiles.getCount(); i++) {
		status &= run(infiles[i]);
	}
	return status;
}


bool Tool_colortriads::run(const string& indata, ostream& out) {
	HumdrumFile infile(indata);
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_colortriads::run(HumdrumFile& infile, ostream& out) {
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_colortriads::run(HumdrumFile& infile) {
	initialize();
	processFile(infile);
	return true;
}



//////////////////////////////
//
// Tool_colortriads::initialize --  Initializations that only have to be done once
//    for all HumdrumFile segments.
//

void Tool_colortriads::initialize(void) {
	m_colorState.resize(7);
	fill(m_colorState.begin(), m_colorState.end(), true);
	if (getBoolean("A")) { m_colorState[0] = false; }
	if (getBoolean("B")) { m_colorState[1] = false; }
	if (getBoolean("C")) { m_colorState[2] = false; }
	if (getBoolean("D")) { m_colorState[3] = false; }
	if (getBoolean("E")) { m_colorState[4] = false; }
	if (getBoolean("F")) { m_colorState[5] = false; }
	if (getBoolean("G")) { m_colorState[6] = false; }

	m_color.resize(7);
	m_color[0] = getString("a");
	m_color[1] = getString("b");
	m_color[2] = getString("c");
	m_color[3] = getString("d");
	m_color[4] = getString("e");
	m_color[5] = getString("f");
	m_color[6] = getString("g");

	m_searches.resize(7);
	m_searches[0] = "(=ace)";
	m_searches[1] = "(=bdf)";
	m_searches[2] = "(=ceg)";
	m_searches[3] = "(=dfa)";
	m_searches[4] = "(=egb)";
	m_searches[5] = "(=fac)";
	m_searches[6] = "(=gbd)";

	m_marks.resize(7);
	m_marks[0] = "V";
	m_marks[1] = "Z";
	m_marks[2] = "@";
	m_marks[3] = "|";
	m_marks[4] = "j";
	m_marks[5] = "+";
	m_marks[6] = "N";


	m_filtersQ  = getBoolean("filters");
	m_commandsQ = getBoolean("commands");
	m_relativeQ = getBoolean("relative");
	m_key       = getString("key");
}


//////////////////////////////
//
// Tool_colortriads::getDiatonicTransposition -- Transpose
//    amount to allow for funcional colors: green = tonic,
//    light blue = dominant, yellow = subdominant, etc.
//    Only the first key designation will be considered, and
//    it must come before any data lines in the score.
//

int Tool_colortriads::getDiatonicTransposition(HumdrumFile& infile) {
	int key;
	char ch;
	int output = 0;
	if (!m_key.empty()) {
		ch = m_key[0];
		if (isupper(ch)) {
			key = m_key.at(0) - 'A';
		} else {
			key = m_key.at(0) - 'a';
		}
		output = 2 - key; // C index is at 2
		if (abs(output) >= 7) {
			output = 0;
		}
		return output;
	}

	for (int i=0; i<infile.getLineCount(); i++) {
		if (infile[i].isData()) {
			break;
		}
		if (!infile[i].isInterpretation()) {
			continue;
		}
		for (int j=0; j<infile[i].getFieldCount(); j++) {
			HTp token = infile.token(i, j);
			if (!token->isKern()) {
				continue;
			}
			if (!token->isKeyDesignation()) {
				continue;
			}
			if (token->size() < 2) {
				continue;
			}
			char ch = token->at(1);
			if (isupper(ch)) {
				key = token->at(1) - 'A';
			} else {
				key = token->at(1) - 'a';
			}
			output = 2 - key; // C index is at 2
			if (abs(output) >= 7) {
				output = 0;
			}
			break;
		}
	}
	return output;
}



//////////////////////////////
//
// Tool_colortriads::processFile --
//

void Tool_colortriads::processFile(HumdrumFile& infile) {
	Tool_msearch msearch;
	vector<string> argv;

	int dtranspose = 0;
	if (m_relativeQ) {
		dtranspose = getDiatonicTransposition(infile);
	}

	int index;
	for (int i=0; i<7; i++) {
		if (dtranspose) {
			index = (i + dtranspose + 70) % 7;
		} else {
			index = i;
		}
		if (m_colorState[index]) {
			argv.clear();
			argv.push_back("msearch"); // name of program (placeholder)
			argv.push_back("-p");
			argv.push_back(m_searches[i]);
			argv.push_back("-m");
			argv.push_back(m_marks[index]);
			argv.push_back("--color");
			argv.push_back(m_color[index]);
			if (m_commandsQ) {
				m_free_text << argv[0];
				for (int j=1; j<(int)argv.size(); j++) {
					if (argv[j] == "|") {
						m_free_text << " '|'";
					} else {
						m_free_text << " " << argv[j];
					}
				}
				m_free_text << endl;
			} else if (m_filtersQ) {
				m_free_text << "!!!filter: " << argv[0];
				for (int j=1; j<(int)argv.size(); j++) {
					if (argv[j] == "|") {
						m_free_text << " '|'";
					} else {
						m_free_text << " " << argv[j];
					}
				}
				m_free_text << endl;
			} else {
				msearch.process(argv);
				msearch.run(infile);
			}
		}
	}
}



// END_MERGE

} // end namespace hum



