//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Fri May  6 19:30:42 PDT 2022
// Last Modified: Fri May  6 19:30:45 PDT 2022
// Filename:      tool-pbar.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/tool-pbar.cpp
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   
//

#include "HumRegex.h"

#include "tool-pbar.h"

using namespace std;

namespace hum {

// START_MERGE


/////////////////////////////////
//
// Tool_pbar::Tool_pbar -- Set the recognized options for the tool.
//

Tool_pbar::Tool_pbar(void) {
	define("i|invisible-barlines=b", "make barlines invisible");
}



//////////////////////////////
//
// Tool_pbar::initialize --  Initializations that only have to be done once
//    for all HumdrumFile segments.
//

void Tool_pbar::initialize(void) {
	m_invisibleQ = getBoolean("invisible-barlines");
}



/////////////////////////////////
//
// Tool_pbar::run -- Do the main work of the tool.
//

bool Tool_pbar::run(HumdrumFileSet& infiles) {
	bool status = true;
	for (int i=0; i<infiles.getCount(); i++) {
		status &= run(infiles[i]);
	}
	return status;
}



bool Tool_pbar::run(const string& indata, ostream& out) {
	HumdrumFile infile(indata);
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_pbar::run(HumdrumFile& infile, ostream& out) {
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_pbar::run(HumdrumFile& infile) {
	initialize();
	processFile(infile);
	return true;
}



//////////////////////////////
//
// Tool_pbar::processFile --
//

void Tool_pbar::processFile(HumdrumFile& infile) {
	vector<HTp> kstarts = infile.getKernSpineStartList();
	for (int i=0; i<(int)kstarts.size(); i++) {
		processSpine(kstarts[i]);
	}

	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].hasSpines()) {
			m_humdrum_text << infile[i] << endl;
			continue;
		}
		if (infile[i].isData()) {
			printDataLine(infile, i);
		} else if (infile[i].isCommentLocal()) {
			printLocalCommentLine(infile, i);
		} else if (infile[i].isBarline()) {
			printBarLine(infile, i);
			if (m_invisibleQ) {
				printInvisibleBarlines(infile, i);
			} else {
				m_humdrum_text << infile[i] << endl;
			}
		} else {
			m_humdrum_text << infile[i] << endl;
		}
	}
}



//////////////////////////////
//
// Tool_pbar::printInvisibleBarlines --
//

void Tool_pbar::printInvisibleBarlines(HumdrumFile& infile, int index) {
	HumRegex hre;
	for (int i=0; i<infile[index].getFieldCount(); i++) {
		HTp token = infile.token(index, i);
		if (hre.search(token, "-")) {
			m_humdrum_text << token;
		} else if (hre.search(token, "==")) {
			m_humdrum_text << token;
		} else if (hre.search(token, "\\|\\|")) {
			m_humdrum_text << token;
		} else {
			m_humdrum_text << token << "-";
		}
		if (i < infile[index].getFieldCount() - 1) {
			m_humdrum_text << "\t";
		}
	}
	m_humdrum_text << "\n";
}



///////////////////////////////
//
// Tool_pbar::printDataLine --
//

void Tool_pbar::printDataLine(HumdrumFile& infile, int index) {
	printBarLine(infile, index);
	m_humdrum_text << infile[index] << endl;
}



///////////////////////////////
//
// Tool_pbar::printBarLine -- Add *bar line.
//

void Tool_pbar::printBarLine(HumdrumFile& infile, int index) {
	bool hasBarline = false;
	for (int j=0; j<infile[index].getFieldCount(); j++) {
		HTp token = infile.token(index, j);
		string value = token->getValue("auto", "pbar");
		if (value == "true") {
			hasBarline = true;
			break;
		}
	}

	if (hasBarline) {
		for (int j=0; j<infile[index].getFieldCount(); j++) {
			HTp token = infile.token(index, j);
			string value = token->getValue("auto", "pbar");
			if (value == "true") {
				m_humdrum_text << "*bar";
			} else {
				m_humdrum_text << "*";
			}
			if (j < infile[index].getFieldCount() - 1) {
				m_humdrum_text << "\t";
			}
		}
		m_humdrum_text << "\n";
	}
}



///////////////////////////////
//
// Tool_pbar::printLocalCommentLine --
//

void Tool_pbar::printLocalCommentLine(HumdrumFile& infile, int index) {
	HumRegex hre;
	bool hasKp = false;
	bool hasOther = false;
	for (int i=0; i<infile[index].getFieldCount(); i++) {
		HTp token = infile.token(index, i);
		if (hre.search(token, "kreska pseudotaktowa")) {
			hasKp = true;
		} else if (*token != "!") {
			hasOther = true;
		}
	}

	if (!hasKp) {
		m_humdrum_text << infile[index] << endl;
		return;
	} 

	if (hasOther) {
		for (int i=0; i<infile[index].getFieldCount(); i++) {
			HTp token = infile.token(index, i);
			if (hre.search(token, "kreska pseudotaktowa")) {
				m_humdrum_text << "!";
			} else {
				m_humdrum_text << token;
			}
			if (i < infile[index].getFieldCount() - 1) {
				m_humdrum_text << "\t";
			}
		}
		m_humdrum_text << "\n";
	}
}



//////////////////////////////
//
// Tool_pbar::processSpine --
//

void Tool_pbar::processSpine(HTp spineStart) {
	HTp current = spineStart;
	HumRegex hre;
	while (current) {
		if (!current->isLocalComment()) {
			current = current->getNextToken();
			continue;
		}
		if (hre.search(current, "kreska\\s*pseudotaktowa")) {
			addBarLineToFollowingNoteOrRest(current);
		}
		current = current->getNextToken();
	}
}



//////////////////////////////
//
// Tool_pbar::addBarLineToFollowingNoteOrRest --
//

void Tool_pbar::addBarLineToFollowingNoteOrRest(HTp token) {
	HTp current = token->getNextToken();
	int counter = 0;
	while (current) {
		if (!current->isBarline()) {
			if (!current->isData() || current->isNull()) {
				current = current->getNextToken();
				continue;
			}
		}
		counter++;
		if (counter == 2) {
			current->setValue("auto", "pbar", "true");
			break;
		}
		current = current->getNextToken();
	}
}


// END_MERGE

} // end namespace hum



