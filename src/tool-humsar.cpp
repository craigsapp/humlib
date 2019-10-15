//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sun Oct 13 11:41:16 PDT 2019
// Last Modified: Sun Oct 13 11:41:19 PDT 2019
// Filename:      tool-humsar.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/tool-humsar.cpp
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   sar-like tool for Humdrum data.
//

#include "tool-humsar.h"
#include "Convert.h"
#include "HumRegex.h"

using namespace std;

namespace hum {

// START_MERGE


/////////////////////////////////
//
// Tool_humsar::Tool_humsar -- Set the recognized options for the tool.
//

Tool_humsar::Tool_humsar(void) {
	define("q|query=s", "query string");
	define("r|replace=s", "replace string");
	define("k|kern=b", "process kern spines only");
	define("x|exinterps|exinerp|exclusive-interpretation|exclusive-interpretations=s", "process only specified spines");
	define("s|spine|spines=s", "list of spines to process");
	define("I|interpretation=b", "process interpretation tokens only");
	define("i|ignore-case=b", "Ignore case of letters");
}



/////////////////////////////////
//
// Tool_humsar::run -- Do the main work of the tool.
//

bool Tool_humsar::run(HumdrumFileSet& infiles) {
	bool status = true;
	for (int i=0; i<infiles.getCount(); i++) {
		status &= run(infiles[i]);
	}
	return status;
}


bool Tool_humsar::run(const string& indata, ostream& out) {
	HumdrumFile infile(indata);
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_humsar::run(HumdrumFile& infile, ostream& out) {
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_humsar::run(HumdrumFile& infile) {
	initialize();
	initializeSegment(infile);
	processFile(infile);
	return true;
}



//////////////////////////////
//
// Tool_humsar::initialize --  Initializations that only have to be done once
//    for all HumdrumFile segments.
//

void Tool_humsar::initialize(void) {
	m_search = getString("query");
	HumRegex hre;
	hre.replaceDestructive(m_search, "\\\\", "\\", "g");
	if (getBoolean("kern")) {
		m_exinterps.push_back("**kern");
	} else if (getBoolean("exclusive-interpretations")) {
		fillInExInterpList();
	}
	if (getBoolean("ignore-case")) {
		m_grepoptions += "i";
	}
}


//////////////////////////////
//
// Tool_humsar::initializeSegment -- Recalculate variables for each Humdrum input segment.
//

void Tool_humsar::initializeSegment(HumdrumFile& infile) {
	m_spines.clear();
	if (getBoolean("spines")) {
		int maxtrack = infile.getMaxTrack();
		Convert::makeBooleanTrackList(m_spines, getString("spines"), maxtrack);
	}
}



//////////////////////////////
//
// Tool_humsar::fillInExInterpList --
//

void Tool_humsar::fillInExInterpList(void) {
	m_exinterps.clear();
	m_exinterps.resize(1);
	string elist = getString("exclusive-interpretations");
	for (int i=0; i<(int)elist.size(); i++) {
		if (isspace(elist[i]) || (elist[i] == ',')) {
			if (!m_exinterps.back().empty()) {
				m_exinterps.push_back("");
			}
		} else {
			m_exinterps.back() += elist[i];
		}
	}
	if (m_exinterps.back().empty()) {
		m_exinterps.resize((int)m_exinterps.size() - 1);
	}
	for (int i=0; i<(int)m_exinterps.size(); i++) {
		if (m_exinterps[i].compare(0, 2, "**") == 0) {
			continue;
		}
		if (m_exinterps[i].compare(0, 1, "*") == 0) {
			m_exinterps[i] = "*" + m_exinterps[i];
			continue;
		}
		m_exinterps[i] = "**" + m_exinterps[i];
	}
}



//////////////////////////////
//
// Tool_humsar::processFile --
//

void Tool_humsar::processFile(HumdrumFile& infile) {
	if (m_search == "") {
		// nothing to do
		return;
	}
	m_modified = false;
	m_replace = getString("replace");

	m_interpretation = getBoolean("interpretation");

	if (m_interpretation) {
		searchAndReplaceInterpretation(infile);
	//if (m_barline) {
	//	searchAndReplaceBarline(infile);
	} else {
		searchAndReplaceData(infile);
	}

	if (m_modified) {
		infile.createLinesFromTokens();
	}
}



//////////////////////////////
//
// Tool_humsar::searchAndReplaceBarline --
//

void Tool_humsar::searchAndReplaceBarline(HumdrumFile& infile) {
	string isearch;
	if (m_search[0] == '^') {
		isearch = "^=" + m_search.substr(1);
	} else {
		isearch = "^=.*" + m_search;
	}
	HumRegex hre;
	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isBarline()) {
			continue;
		}
		for (int j=0; j<infile[i].getFieldCount(); j++) {
			HTp token = infile.token(i, j);
			if (token->isNull()) {	
				// Don't mess with null interpretations
				continue;
			}
			if (!isValid(token)) {
				continue;
			}
			if (hre.search(token, isearch, m_grepoptions)) {
				string text = token->getText();
				hre.replaceDestructive(text, m_replace, isearch, m_grepoptions);
				hre.replaceDestructive(text, "", "^=+");
				text = "=" + text;
				token->setText(text);
				m_modified = true;
			}
		}
	}
}


//////////////////////////////
//
// Tool_humsar::searchAndReplaceInterpretation --
//

void Tool_humsar::searchAndReplaceInterpretation(HumdrumFile& infile) {
	string isearch;
	if (m_search[0] == '^') {
		isearch = "^\\*" + m_search.substr(1);
	} else {
		isearch = "^\\*.*" + m_search;
	}
	HumRegex hre;
	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isInterpretation()) {
			continue;
		} else if (infile[i].isExclusiveInterpretation()) {
			continue;
		} else if (infile[i].isManipulator()) {
			continue;
		}
		for (int j=0; j<infile[i].getFieldCount(); j++) {
			HTp token = infile.token(i, j);
			if (token->isNull()) {	
				// Don't mess with null interpretations
				continue;
			}
			if (!isValid(token)) {
				continue;
			}
			if (hre.search(token, isearch, m_grepoptions)) {
				string text = token->getText();
				hre.replaceDestructive(text, m_replace, isearch, m_grepoptions);
				hre.replaceDestructive(text, "", "^\\*+");
				text = "*" + text;
				token->setText(text);
				m_modified = true;
			}
		}
	}
}



//////////////////////////////
//
// Tool_humsar::searchAndReplaceData --
//

void Tool_humsar::searchAndReplaceData(HumdrumFile& infile) {
	string dsearch = m_search;

	HumRegex hre;
	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isData()) {
			continue;
		}
		for (int j=0; j<infile[i].getFieldCount(); j++) {
			HTp token = infile.token(i, j);
			if (token->isNull()) {
				// Don't mess with null interpretations
				continue;
			}
			if (!isValid(token)) {
				continue;
			}
			if (hre.search(token, dsearch, m_grepoptions)) {
				string text = token->getText();
				hre.replaceDestructive(text, m_replace, dsearch, m_grepoptions);
				if (text == "") {
					text = ".";
				}
				token->setText(text);
				m_modified = true;
			}
		}
	}
}



//////////////////////////////
//
// Tool_humsar::isValidDataType -- usar with -x and -k options.
//

bool Tool_humsar::isValidDataType(HTp token) {
	if (m_exinterps.empty()) {
		return true;
	}
	string datatype = token->getDataType();
	for (int i=0; i<(int)m_exinterps.size(); i++) {
		if (datatype == m_exinterps[i]) {
			return true;
		}
	}
	return false;
}



//////////////////////////////
//
// Tool_humsar::isValidSpine -- usar with -s option.
//

bool Tool_humsar::isValidSpine(HTp token) {
	if (m_spines.empty()) {
		return true;
	}
	int track = token->getTrack();
	return m_spines.at(track);
}



//////////////////////////////
//
// Tool_humsar::isValid --
//

bool Tool_humsar::isValid(HTp token) {
	if (isValidDataType(token) && isValidSpine(token)) {
		return true;
	}
	return false;
}



// END_MERGE

} // end namespace hum



