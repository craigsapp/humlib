//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Tue Sep 26 04:27:04 PDT 2023
// Last Modified: Tue Sep 26 04:27:07 PDT 2023
// Filename:      tool-addic.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/tool-addic.cpp
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Adds and fills in *IC lines for instrument code lines.
//
// Todo:          Allow forms such as *Iflt&Ipicco  and *Iflt|Ipicco
//

#include "tool-addic.h"
#include "Convert.h"
#include "HumRegex.h"

using namespace std;

namespace hum {

// START_MERGE


/////////////////////////////////
//
// Tool_addic::Tool_addic -- Set the recognized options for the tool.
//

Tool_addic::Tool_addic(void) {
}



/////////////////////////////////
//
// Tool_addic::run -- Do the main work of the tool.
//

bool Tool_addic::run(HumdrumFileSet& infiles) {
	bool status = true;
	for (int i=0; i<infiles.getCount(); i++) {
		status &= run(infiles[i]);
	}
	return status;
}


bool Tool_addic::run(const string& indata, ostream& out) {
	HumdrumFile infile(indata);
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_addic::run(HumdrumFile& infile, ostream& out) {
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_addic::run(HumdrumFile& infile) {
	initialize();
	processFile(infile);
	return true;
}


//////////////////////////////
//
// Tool_addic::initialize --
//

void Tool_addic::initialize(void) {
	m_instrumentList = Convert::getInstrumentList();
}



//////////////////////////////
//
// Tool_addic::processFile --
//

void Tool_addic::processFile(HumdrumFile& infile) {

	int codeIndex = getInstrumentCodeIndex(infile);
	int classIndex = getInstrumentClassIndex(infile);

	if (!codeIndex) {
		// No code index, so nothing to do.
	}
	if (classIndex) {
		// Instrument class line already exists so adjust it:
		updateInstrumentClassLine(infile, codeIndex, classIndex);
	} else {
		string classLine = makeClassLine(infile, codeIndex);
		for (int i=0; i<infile.getLineCount(); i++) {
			if (i == codeIndex) {
				m_humdrum_text << classLine << endl;
			}
			m_humdrum_text << infile[i] << endl;
		}
	}
}



//////////////////////////////
//
// Tool_addic::makeClassLine --
//

string Tool_addic::makeClassLine(HumdrumFile& infile, int codeIndex) {
	string output;
	HumRegex hre;
	int count = infile[codeIndex].getFieldCount();
	for (int i=0; i<count; i++) {
		HTp codeToken = infile.token(codeIndex, i);
		if (!hre.search(codeToken, "^\\*I([a-z].*)")) {
			output += "*";
			if (i < count - 1) {
				output += "\t";
			}
			continue;
		}
		string code = hre.getMatch(1);
		string iclass = getInstrumentClass(code);
		if (iclass.empty()) {
			output += "*UNKNOWN";
			if (i < count - 1) {
				output += "\t";
			}
			continue;
		}
		string text = "*IC" + iclass;
		output += text;
		if (i < count - 1) {
			output += "\t";
		}
	}
	return output;
}



//////////////////////////////
//
// Tool_addic::updateInstrumentClassLine --
//

void Tool_addic::updateInstrumentClassLine(HumdrumFile& infile, int codeIndex,
		int classIndex) {

	int codeSize  = infile[codeIndex].getFieldCount();
	int classSize = infile[classIndex].getFieldCount();
	if (codeSize != classSize) {
		cerr << "Instrument code line length does not match that of class line" << endl;
		return;
	}

	HumRegex hre;
	for (int i=0; i<infile[codeIndex].getFieldCount(); i++) {
		HTp codeToken  = infile.token(codeIndex, i);
		HTp classToken = infile.token(classIndex, i);
		if (*codeToken == "*") {
			continue;
		}
		if ((*classToken != "*") || !hre.search(classToken, "^\\*IC")) {
			cerr << "Not overwriting non-class content: " << classToken << endl;
			continue;
		}
		if (!hre.search(codeToken, "^\\*I([a-z].*)")) {
			continue;
		}
		string code = hre.getMatch(1);
		string iclass = getInstrumentClass(code);
		if (iclass.empty()) {
			continue;
		}
		string text = "*IC" + iclass;
		classToken->setText(text);
	}
	infile[classIndex].createLineFromTokens();
}



//////////////////////////////
//
// Tool_addic::getInstrumentCodeIndex --
//

int Tool_addic::getInstrumentCodeIndex(HumdrumFile& infile) {
	HumRegex hre;
	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].hasSpines()) {
			continue;
		}
		if (infile[i].isData()) {
			break;
		}
		if (!infile[i].isInterpretation()) {
			continue;
		}
		for (int j=0; j<infile[i].getFieldCount(); j++) {
			HTp token = infile.token(i, j);
			if (*token == "*") {
				continue;
			}
			if (hre.search(token, "^\\*I[a-z]")) {
				return i;
			}
		}
	}
	return 0;
}



//////////////////////////////
//
// Tool_addic::getInstrumentClassIndex --
//

int Tool_addic::getInstrumentClassIndex(HumdrumFile& infile) {
	HumRegex hre;
	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].hasSpines()) {
			continue;
		}
		if (infile[i].isData()) {
			break;
		}
		if (!infile[i].isInterpretation()) {
			continue;
		}
		for (int j=0; j<infile[i].getFieldCount(); j++) {
			HTp token = infile.token(i, j);
			if (*token == "*") {
				continue;
			}
			if (hre.search(token, "^\\*IC[a-z]")) {
				return i;
			}
		}
	}
	return 0;
}



//////////////////////////////
//
// Tool_addic --
//

string Tool_addic::getInstrumentClass(const string& code) {
	for (int i=0; i<(int)m_instrumentList.size(); i++) {
		if (code == m_instrumentList[i].first) {
			return m_instrumentList[i].second;
		}
	}
	return "UNKNOWN" + code;
}




// END_MERGE

} // end namespace hum



