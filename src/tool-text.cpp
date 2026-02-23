//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Thu Jan 30 22:26:33 PST 2020
// Last Modified: Thu Jan 30 22:26:35 PST 2020
// Filename:      tool-text.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/tool-text.cpp
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Switch between text encoding and corrected encoding.
//

#include "tool-text.h"
#include "Convert.h"
#include "HumRegex.h"

using namespace std;

namespace hum {

// START_MERGE


/////////////////////////////////
//
// Tool_text::Tool_text -- Set the recognized options for the tool.
//

Tool_text::Tool_text(void) {
	define("1|first=b", "Display only first verse for each part");
}



/////////////////////////////////
//
// Tool_text::run -- Do the main work of the tool.
//

bool Tool_text::run(HumdrumFileSet& infiles) {
	bool status = true;
	for (int i=0; i<infiles.getCount(); i++) {
		status &= run(infiles[i]);
	}
	return status;
}


bool Tool_text::run(const string& indata, ostream& out) {
	HumdrumFile infile(indata);
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_text::run(HumdrumFile& infile, ostream& out) {
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_text::run(HumdrumFile& infile) {
	initialize();
	processFile(infile);
	return true;
}



//////////////////////////////
//
// Tool_text::initialize --  Initializations that only have to be done once
//    for all HumdrumFile segments.
//

void Tool_text::initialize(void) {
	m_onlyQ  = getBoolean("first");
	m_aboveQ = getBoolean("above");
}



//////////////////////////////
//
// Tool_text::processFile --
//

void Tool_text::processFile(HumdrumFile& infile) {
	removeText(infile);
	m_humdrum_text << "!!@@BEGIN: ";
	if (m_aboveQ) {
		m_humdrum_text << "PREHTML" << endl;
	} else {
		m_humdrum_text << "POSTHTML" << endl;
	}
	m_humdrum_text << infile;
	m_humdrum_text << "!!@CONTENT:" << endl;
	m_humdrum_text << m_text;
	m_humdrum_text << "!!@@END: "<< endl;
	if (m_aboveQ) {
		m_humdrum_text << "PREHTML" << endl;
	} else {
		m_humdrum_text << "POSTHTML" << endl;
	}
}


/////////////////////////////
//
// Tool_text:removeText -- move **text above/below music.
//

void Tool_text::removeText(HumdrumFile& infile) {
	vector<HTp> staffspines;
	infile.getStaffLikeSpineStartList(staffspines);
	staffspines.push_back(NULL);
	for (int i=0; i<(int)staffspines.size()-1; i++) {
		removePartText(staffspines[i], staffspines[i+1], infile);
	}

}

/////////////////////////////
//
// Tool_text:removePartText --
//

void Tool_text::removePartText(HTp& startspine, HTp& endspine, HumdrumFile& infile) {
	int endcol;
	if (endspine) {
		endcol = endspine->getSpineIndex();
	} else {
		endcol = infile.getMaxTracks() - 1;
	}
	HTp current = startspine;
	while (current && current->getSpineIndex() != endcol) {
		processTextSpine(startspine);
		current = current->getNextFieldToken();
	}
}



//////////////////////////////
//
// Tool_text::procesTextSpine -- Extract a verse/spine of text
//

void Tool_text::processTextSpine(HTp tspine) {
	HTp current = tspine;
	m_output << "!!@<p>";
	while (current) {
		if (!current->isData()) {
			current = current->getNextToken();
			continue;
		}
		if (current->isNull()) {
			current = current->getNextToken();
			continue;
		}
		m_output << current->getText();
	}
	m_output << endl;
}



// END_MERGE

} // end namespace hum



