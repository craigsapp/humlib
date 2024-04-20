//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Wed Aug 26 15:24:30 PDT 2020
// Last Modified: Sun Sep 20 12:36:46 PDT 2020
// Filename:      tool-flipper.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/tool-flipper.cpp
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Switch between flipper encoding and corrected encoding.
//

#include "tool-flipper.h"
#include "Convert.h"
#include "HumRegex.h"

using namespace std;

namespace hum {

// START_MERGE


/////////////////////////////////
//
// Tool_flipper::Tool_flipper -- Set the recognized options for the tool.
//

Tool_flipper::Tool_flipper(void) {
	define("k|keep=b",                      "keep *flip/*Xflip instructions");
	define("a|all=b",                       "flip globally, not just inside *flip/*Xflip regions");
	define("s|strophe=b",                   "flip inside of strophes as well");
	define("S|strophe-only|only-strophe=b", "flip only inside of strophes as well");
	define("i|interp=s:kern",               "flip only in this interpretation");
}



/////////////////////////////////
//
// Tool_flipper::run -- Do the main work of the tool.
//

bool Tool_flipper::run(HumdrumFileSet& infiles) {
	bool status = true;
	for (int i=0; i<infiles.getCount(); i++) {
		status &= run(infiles[i]);
	}
	return status;
}


bool Tool_flipper::run(const string& indata, ostream& out) {
	HumdrumFile infile(indata);
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_flipper::run(HumdrumFile& infile, ostream& out) {
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_flipper::run(HumdrumFile& infile) {
	initialize();
	processFile(infile);
	return true;
}



//////////////////////////////
//
// Tool_flipper::initialize --  Initializations that only have to be done once
//    for all HumdrumFile segments.
//

void Tool_flipper::initialize(void) {
	m_allQ         = getBoolean("all");
	m_keepQ        = getBoolean("keep");
	m_kernQ        = true;
	m_stropheQ     = getBoolean("strophe");
	m_interp       = getString("interp");
	if (m_interp != "kern") {
		m_kernQ = false;
	}
}



//////////////////////////////
//
// Tool_flipper::processFile --
//

void Tool_flipper::processFile(HumdrumFile& infile) {

	m_fliplines.resize(infile.getLineCount());
	fill(m_fliplines.begin(), m_fliplines.end(), false);

	m_flipState.resize(infile.getMaxTrack()+1);
	if (m_allQ) {
		fill(m_flipState.begin(), m_flipState.end(), true);
	} else {
		fill(m_flipState.begin(), m_flipState.end(), false);
	}

	m_strophe.resize(infile.getMaxTrack()+1);
	fill(m_strophe.begin(), m_strophe.end(), false);

	for (int i=0; i<infile.getLineCount(); i++) {
		processLine(infile, i);
		if (!m_keepQ) {
			if (!m_fliplines[i]) {
				m_humdrum_text << infile[i] << endl;
			}
		}
	}

	if (m_keepQ) {
		m_humdrum_text << infile;
	}
}



//////////////////////////////
//
// Tool_flipper::checkForFlipChanges --
//

void Tool_flipper::checkForFlipChanges(HumdrumFile& infile, int index) {
	if (!infile[index].isInterpretation()) {
		return;
	}

	int track;

	for (int i=0; i<infile[index].getFieldCount(); i++) {
		HTp token = infile.token(index, i);
		if (*token == "*strophe") {
			track = token->getTrack();
			m_strophe[track] = true;
		} else if (*token == "*Xstrophe") {
			track = token->getTrack();
			m_strophe[track] = false;
		}
	}


	if (m_allQ) {
		// state always stays on in this case
		return;
	}

	for (int i=0; i<infile[index].getFieldCount(); i++) {
		HTp token = infile.token(index, i);
		if (*token == "*flip") {
			track = token->getTrack();
			m_flipState[track] = true;
			m_fliplines[i] = true;
		} else if (*token == "*Xflip") {
			track = token->getTrack();
			m_flipState[track] = false;
			m_fliplines[i] = true;
		}
	}

}



//////////////////////////////
//
// Tool_flipper::processLine --
//

void Tool_flipper::processLine(HumdrumFile& infile, int index) {
	if (!infile[index].hasSpines()) {
		return;
	}
	if (infile[index].isInterpretation()) {
		checkForFlipChanges(infile, index);
	}

	vector<vector<HTp>> flipees;
	extractFlipees(flipees, infile, index);
	if (!flipees.empty()) {
		int status = flipSubspines(flipees);
		if (status) {
			infile[index].createLineFromTokens();
		}
	}
}



//////////////////////////////
//
// Tool_flipper::flipSubspines --
//

bool Tool_flipper::flipSubspines(vector<vector<HTp>>& flipees) {
	bool regenerateLine = false;
	for (int i=0; i<(int)flipees.size(); i++) {
		if (flipees[i].size() > 1) {
			flipSpineTokens(flipees[i]);
			regenerateLine = true;
		}
	}
	return regenerateLine;
}


//////////////////////////////
//
// Tool_flipper::flipSpineTokens --
//

void Tool_flipper::flipSpineTokens(vector<HTp>& subtokens) {
	if (subtokens.size() < 2) {
		return;
	}
	int count = (int)subtokens.size();
	count = count / 2;
	HTp tok1;
	HTp tok2;
	string str1;
	string str2;
	for (int i=0; i<count; i++) {
		tok1 = subtokens[i];
		tok2 = subtokens[subtokens.size() - 1 - i];
		str1 = *tok1;
		str2 = *tok2;
		tok1->setText(str2);
		tok2->setText(str1);
	}
}



//////////////////////////////
//
// Tool_flipper::extractFlipees --
//

void Tool_flipper::extractFlipees(vector<vector<HTp>>& flipees,
		 HumdrumFile& infile, int index) {
	flipees.clear();

	HLp line = &infile[index];
	int track;
	int lastInsertTrack = -1;
	for (int i=0; i<line->getFieldCount(); i++) {
		HTp token = line->token(i);
		track = token->getTrack();
		if ((!m_stropheQ) && m_strophe[track]) {
			continue;
		}
		if (!m_flipState[track]) {
			continue;
		}
		if (m_kernQ) {
			if (!token->isKern()) {
				continue;
			}
		} else {
			if (!token->isDataType(m_interp)) {
				continue;
			}
		}
		if (lastInsertTrack != track) {
			flipees.resize(flipees.size() + 1);
			lastInsertTrack = track;
		}
		flipees.back().push_back(token);
	}
}



// END_MERGE

} // end namespace hum



