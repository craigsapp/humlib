//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sun Oct 13 11:41:16 PDT 2019
// Last Modified: Sun Oct 13 11:41:19 PDT 2019
// Filename:      tool-humsed.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/tool-humsed.cpp
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Sort data spines in a Humdrum file.
//

#include "tool-humsed.h"
#include "Convert.h"
#include "HumRegex.h"

using namespace std;

namespace hum {

// START_MERGE


/////////////////////////////////
//
// Tool_humsed::Tool_humsed -- Set the recognized options for the tool.
//

Tool_humsed::Tool_humsed(void) {
	define("s|search=s", "search string");
	define("r|replace=s", "replace string");
	define("k|kern=b", "process kern spines only");
	define("x|exinerp|exclusive-interpretation=s", "process kern spines only");
	define("i|interpretation=b", "process interpretation tokens only");
}



/////////////////////////////////
//
// Tool_humsed::run -- Do the main work of the tool.
//

bool Tool_humsed::run(HumdrumFileSet& infiles) {
	bool status = true;
	for (int i=0; i<infiles.getCount(); i++) {
		status &= run(infiles[i]);
	}
	return status;
}


bool Tool_humsed::run(const string& indata, ostream& out) {
	HumdrumFile infile(indata);
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_humsed::run(HumdrumFile& infile, ostream& out) {
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_humsed::run(HumdrumFile& infile) {
	processFile(infile);
	return true;
}



//////////////////////////////
//
// Tool_humsed::processFile --
//

void Tool_humsed::processFile(HumdrumFile& infile) {
	m_search = getString("search");
	if (m_search == "") {
		// nothing to do
		return;
	}
	m_modified = false;
	m_replace = getString("replace");

	m_interpretation = getBoolean("interpretation");

	if (!m_interpretation) {
		// don't know what to do (only interpretation processing is currently implemented)
		return;
	}

	if (m_interpretation) {
		searchAndReplaceInterpretation(infile);
	}

	if (m_modified) {
		infile.createLinesFromTokens();
	}
}



//////////////////////////////
//
// Tool_humsed::searchAndReplaceInterpretation --
//

void Tool_humsed::searchAndReplaceInterpretation(HumdrumFile& infile) {
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
			if (*token == "*") {	
				// Don't mess with null interpretations
				continue;
			}
			if (hre.search(token, isearch)) {
				string text = token->getText();
				hre.replaceDestructive(text, m_replace, isearch);
				hre.replaceDestructive(text, "", "^\\*+");
				text = "*" + text;
				token->setText(text);
				m_modified = true;
			}
		}
	}
}



// END_MERGE

} // end namespace hum



