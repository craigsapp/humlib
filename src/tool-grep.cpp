//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Wed Apr 12 19:04:38 PDT 2023
// Last Modified: Wed Apr 12 19:04:41 PDT 2023
// Filename:      tool-grep.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/tool-grep.cpp
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   C++ implementation of the grep command (sort of).
//                Mostly useful for removing lines with the -v option.
//

#include "tool-grep.h"
#include "HumRegex.h"

#include <cstdlib>
#include <cstring>
#include <iostream>

using namespace std;

namespace hum {

// START_MERGE


/////////////////////////////////
//
// Tool_grep::Tool_grep -- Set the recognized options for the tool.
//

Tool_grep::Tool_grep(void) {
	define("v|remove-matching-lines=b", "Remove lines that match regex");
	define("e|regex|regular-expression=s", "Regular expression to search with");
}


/////////////////////////////////
//
// Tool_grep::run -- Do the main work of the tool.
//

bool Tool_grep::run(HumdrumFileSet& infiles) {
	bool status = true;
	for (int i=0; i<infiles.getCount(); i++) {
		status &= run(infiles[i]);
	}
	return status;
}


bool Tool_grep::run(const string& indata, ostream& out) {
	HumdrumFile infile(indata);
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_grep::run(HumdrumFile& infile, ostream& out) {
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_grep::run(HumdrumFile& infile) {
	initialize();
	processFile(infile);
	return true;
}



//////////////////////////////
//
// Tool_grep::initialize --  Initializations that only have to be
//    done one for all HumdrumFile segments.
//

void Tool_grep::initialize(void) {
	m_negateQ = getBoolean("remove-matching-lines");
	m_regex = getString("regular-expression");
}



//////////////////////////////
//
// Tool_grep::processFile --
//

void Tool_grep::processFile(HumdrumFile& infile) {
	HumRegex hre;
	bool match;
	for (int i=0; i<infile.getLineCount(); i++) {
		match = hre.search(infile[i], m_regex);
		if (m_negateQ) {
			if (match) {
				continue;
			}
		} else {
			if (!match) {
				continue;
			}
		}
		m_humdrum_text << infile[i] << "\n";
	}
}


// END_MERGE

} // end namespace hum



