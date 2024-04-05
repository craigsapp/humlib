//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Thu Apr  4 23:11:06 PDT 2024
// Last Modified: Thu Apr  4 23:11:09 PDT 2024
// Filename:      tool-humbreak.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/tool-humbreak.cpp
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Insert line/system breaks and page breaks before input measures.
//
// Options:
//                -m measures == comma-delimited list of measures to add line breaks before
//                -p measures == comma-delimited list of measures to add page breaks before
//                -g label    == use the given label for breaks (defualt "original")
//                -r          == remove line/page breaks
//                -l          == convert page breaks to line breaks
//

#include "tool-humbreak.h"
#include "HumRegex.h"

#include <vector>

using namespace std;

namespace hum {

// START_MERGE


/////////////////////////////////
//
// Tool_humbreak::Tool_humbreak -- Set the recognized options for the tool.
//

Tool_humbreak::Tool_humbreak(void) {
	define("m|measures=s", "Measures numbers to place linebreaks before");
	define("p|page-breaks=s", "Measure numbers to place page breaks before");
	define("g|group=s:original", "Line/page break group");
	define("r|remove|remove-breaks=b", "Remove line/page breaks");
	define("l|page-to-line-breaks=b", "Convert page breaks to line breaks");
}



/////////////////////////////////
//
// Tool_humbreak::run -- Do the main work of the tool.
//

bool Tool_humbreak::run(HumdrumFileSet& infiles) {
	bool status = true;
	for (int i=0; i<infiles.getCount(); i++) {
		status &= run(infiles[i]);
	}
	return status;
}


bool Tool_humbreak::run(const string& indata, ostream& out) {
	HumdrumFile infile(indata);
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_humbreak::run(HumdrumFile& infile, ostream& out) {
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_humbreak::run(HumdrumFile& infile) {
	processFile(infile);
	return true;
}



//////////////////////////////
//
// Tool_humbreak::initialize --  Initializations that only have to be done once
//    for all HumdrumFile segments.
//

void Tool_humbreak::initialize(void) {
	string systemMeasures = getString("measures");
	string pageMeasures = getString("page-breaks");
	m_group = getString("group");
	m_removeQ = getBoolean("remove-breaks");
	m_page2lineQ = getBoolean("page-to-line-breaks");

	vector<string> lbs;
	vector<string> pbs;
	HumRegex hre;
	hre.split(lbs, systemMeasures, "[^\\d]+");
	hre.split(pbs, pageMeasures, "[^\\d]+");

	for (int i=0; i<(int)lbs.size(); i++) {
		int number = std::stoi(lbs[i]);
		m_lineMeasures[number] = 1;
	}

	for (int i=0; i<(int)pbs.size(); i++) {
		int number = std::stoi(lbs[i]);
		m_pageMeasures[number] = 1;
	}
}


//////////////////////////////
//
// Tool_humbreak::addBreaks --
//

void Tool_humbreak::addBreaks(HumdrumFile& infile) {

	HumRegex hre;
	for (int i=0; i<infile.getLineCount(); i++) {

		if ((i<infile.getLineCount()-1) && (infile.token(i, 0)->compare(0, 8, "!!LO:PB:") == 0)) {
			// Add group to existing LO:PB:
			HTp token = infile.token(i, 0);
			HTp barToken = infile.token(i+1, 0);
			if (barToken->isBarline()) {
				int measure = infile[i+1].getBarNumber();
				int pbStatus = m_pageMeasures[measure];
				if (pbStatus) {
					string query = "\\b" + m_group + "\\b";
					if (!hre.match(token, query)) {
						m_humdrum_text << token << ", " << m_group << endl;
					} else {
						m_humdrum_text << token << endl;
					}
				} else {
					m_humdrum_text << token << endl;
				}
				m_humdrum_text << infile[i+1] << endl;
				i++;
				continue;
			}
		}

		if ((i<infile.getLineCount()-1) && (infile.token(i, 0)->compare(0, 8, "!!LO:LB:") == 0)) {
			// Add group to existing LO:LB:
			HTp token = infile.token(i, 0);
			HTp barToken = infile.token(i+1, 0);
			if (barToken->isBarline()) {
				int measure = infile[i+1].getBarNumber();
				int lbStatus = m_lineMeasures[measure];
				if (lbStatus) {
					string query = "\\b" + m_group + "\\b";
					if (!hre.match(token, query)) {
						m_humdrum_text << token << ", " << m_group << endl;
					} else {
						m_humdrum_text << token << endl;
					}
				} else {
					m_humdrum_text << token << endl;
				}
				m_humdrum_text << infile[i+1] << endl;
				i++;
				continue;
			}
		}

		if (!infile[i].isBarline()) {
			m_humdrum_text << infile[i] << endl;
			continue;
		}
		
		int measure = infile[i].getBarNumber();
		int pbStatus = m_pageMeasures[measure];
		int lbStatus = m_lineMeasures[measure];


		if (pbStatus) {
			m_humdrum_text << "!!LO:PB:g=" << m_group << endl;
		} else if (lbStatus) {
			m_humdrum_text << "!!LO:LB:g=" << m_group << endl;
		}

		m_humdrum_text << infile[i] << endl;
	}
}




//////////////////////////////
//
// Tool_humbreak::processFile --
//

void Tool_humbreak::processFile(HumdrumFile& infile) {
	initialize();
	if (m_removeQ) {
		removeBreaks(infile);
	} else if (m_page2lineQ) {
		convertPageToLine(infile);
	} else {
		addBreaks(infile);
	}
}



//////////////////////////////
//
// Tool_humbreak::removeBreaks --
//

void Tool_humbreak::removeBreaks(HumdrumFile& infile) {
	for (int i=0; i<infile.getLineCount(); i++) {
		if (infile[i].token(0)->compare(0, 7, "!!LO:LB") == 0) {
			continue;
		}
		if (infile[i].token(0)->compare(0, 7, "!!LO:PB") == 0) {
			continue;
		}
		m_humdrum_text << infile[i] << endl;
	}
}



//////////////////////////////
//
// Tool_humbreak::convertPageToLine --
//

void Tool_humbreak::convertPageToLine(HumdrumFile& infile) {
	HumRegex hre;
	for (int i=0; i<infile.getLineCount(); i++) {
		if (infile[i].token(0)->compare(0, 7, "!!LO:PB") == 0) {
			string text = *infile[i].token(0);
			hre.replaceDestructive(text, "!!LO:LB", "!!LO:PB");
			m_humdrum_text << text << endl;
			continue;
		}
		m_humdrum_text << infile[i] << endl;
	}
}


// END_MERGE

} // end namespace hum



