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
	m_aboveQ = !getBoolean("above");
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
/*
	vector<HTp> sspines;
	infile.getSpineStartList(sspines);
	vector<vector<HTp>> twoarray;
	makeTextArray(twoarray);
	for (int i=(int)sspines.size()-1; i>0; i--) {
		if (sspines[i]->isKern()) {
			continue;
		}
		removePartText(sspines[i]);
	}
*/
}



//////////////////////////////
//
// Tool_text::makeTextArray -- Reverse order of **text;
//

/*
void Tool_text::makeTextArray(vector<vector<HTp>>& texts, vector<HTp> spines) {
	text.resize(1();
	for (i=90 i<(int)spine.size(); i++) {

}
*/




/////////////////////////////
//
// Tool_text:removePartText --
//

void Tool_text::removePartText(HTp startspine) {
	if (hasPline(startspine)) {
		processPlineSpine(startspine);
	} else {
		processTextSpine(startspine);
	}
}



//////////////////////////////
//
// Tool_text::hasPline -- True if *pline exists in spine
//

bool Tool_text::hasPline(HTp tspine) {
	HTp current = tspine;
	while (current) {
		if (!current->isInterpretation()) {
			current = current->getNextToken();
			continue;
		} else if (current->compare(0, 7, "*pline:") == 0) {
			return true;
		}
		current = current->getNextToken();
	}

	return false;
}



//////////////////////////////
//
// Tool_text::procesTextSpine -- Extract a verse/spine of text
//

void Tool_text::processPlineSpine(HTp tspine) {
	vector<vector<HTp>> plines;
	fillPlines(plines, tspine);
	m_output << "\n!! <table>";
	for (int i=1; i<(int)plines.size(); i++) {
		zprintPlineRow(plines[i]);
	}
	m_output << "\n!! </table>\n";
}



//////////////////////////////
//
// Tool_text::printPlineRow -- print text for a pline
//

void Tool_text::zprintPlineRow(vector<HTp>& pieces) {
	m_output << "\n!! <tr>";
	string plinelabel = getPlineLabel(pieces);
	if (!plinelabel.empty()) {
		m_output << "\n!! <td class=\"pline\">";
		m_output << plinelabel;
		m_output << "</td>";
	}
	m_output << "\n!! <td class=\"pline-text\">";
	printPlineSyllables(pieces);
	m_output << "</td>";
	m_output << "\n!! </tr>";
}



//////////////////////////////
//
// Tool_text::getPlineLabel -- print text for a pline
//

string Tool_text::getPlineLabel(vector<HTp>& pieces) {
	for (int i=0; i<(int)pieces.size(); i++) {
		if (pieces[i]->isInterpretation()) {
			if (pieces[i]->compare(0, 7, "*pline:") == 0) {
				return pieces[i]->substr(7);
			}
		}
	}
	string out = "NONE";
	return out;
}



//////////////////////////////
//
// Tool_text::printPlineSyllables -- print text for a pline
//

void Tool_text::printPlineSyllables(vector<HTp>& pieces) {
	stringstream out;
	vector<HTp> np;
	for (int i=0; i<(int)pieces.size(); i++) {
		if (pieces[i]->isNull()) {
			continue;
		}
		else if (pieces[i]->isData()) {
			np.push_back(pieces[i]);
		}
	}

	// strip dashes
	for (int i=0; i<(int)np.size(); i++) {
		string text = *np[i];
		if (!text.empty()) {
			if (text.back() == '-') {
				text.resize(text.size()-1);
			}
		}
		if (!text.empty()) {
			if (text[0] == '-') {
				text = text.substr(1);
			}
		}
		m_output << text << " ";
	}


	m_output << out.str();
	return;
}



//////////////////////////////
//
// Tool_text::fillPlines -- create vector for each pline.
//

void Tool_text::fillPlines(vector<vector<HTp>>& plines, HTp tspine) {
	plines.clear();
	plines.resize(1);
	HTp current = tspine;
	while (current) {
		if (current->isInterpretation() && current->compare(0, 7, "*pline:") == 0) {
			plines.resize(plines.size() + 1);
		}
		plines.back().push_back(current);
		current = current->getNextToken();
	}

}



//////////////////////////////
//
// Tool_text::procesTextSpine -- Extract a verse/spine of text
//

void Tool_text::processTextSpine(HTp tspine) {
	HumdrumFileStructure *infile = tspine->getOwner()->getOwner();
	string name = infile->getPartName(tspine);
	if (!name.empty()) {
		m_output << "!!\n!! <p>" << name << endl;
	} else {
		m_output << "!!\n!! <p>" << "empty" << endl;
	}

	m_output << "!! <p>";
	HTp current = tspine;
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



