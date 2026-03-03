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
	define("a|above=b", "Show above the music");
	define("b|below=b", "Show below the music");
	define("j|join=b", "join syllables into single word");
	define("M|no-merge=b", "do not merge syllables");
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
	m_belowQ = getBoolean("below");
	m_joinQ  = getBoolean("join");
}



//////////////////////////////
//
// Tool_text::processFile --
//

void Tool_text::processFile(HumdrumFile& infile) {
	removeText(infile);
	m_humdrum_text << infile;
	m_humdrum_text << "!!@@BEGIN: ";
	if (m_aboveQ) {
		m_humdrum_text << "PREHTML" << endl;
	} else {
		m_humdrum_text << "POSTHTML" << endl;
	}
	m_humdrum_text << "!!@CONTENT:" << endl;
	m_humdrum_text << m_output.str();
	m_humdrum_text << "!!@@END: ";
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
	vector<HTp> sspines;
	infile.getSpineStartList(sspines);
	sspines.push_back(NULL);
	for (int i=0; i<(int)sspines.size()-1; i++) {
		if (sspines[i]->isKern()) {
			continue;
		}
		removePartText(sspines[i]);
	}
}



/////////////////////////////
//
// Tool_text:removePartText --
//

void Tool_text::removePartText(HTp& startspine) {
	processTextSpine(startspine);
}



//////////////////////////////
//
// Tool_text::procesTextSpine -- Extract a verse/spine of text
//

void Tool_text::processTextSpine(HTp tspine) {
	HTp current = tspine;
	m_output << "!!";
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
		string syllable = getSyllable(current);
		m_output << syllable;
		current = current->getNextToken();
	}
	m_output << endl;
}



//////////////////////////////
//
// Tool_text::getSyllable --
//

string Tool_text::getSyllable(HTp token) {
	string text = *token;
	HumRegex hre;
	if (m_mergeQ) {
		hre.replaceDestructive(text, "", "^-");
		if (!text.empty()) {
			if (text.back() == '-') {
				text.resize((int)text.size()-1);
			} else {
				text += " ";
			}
		} else {
			text += " ";
		}
	} else {
		text += " ";
	}

	return text;
}




// END_MERGE

} // end namespace hum



