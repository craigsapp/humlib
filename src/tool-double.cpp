//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Fri Jul 30 04:59:25 CEST 2021
// Last Modified: Fri Jul 30 04:59:28 CEST 2021
// Filename:      tool-double.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/tool-double.cpp
// Syntax:        C++11; humlib
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Double note/rest/time signature durations.
//

#include "tool-double.h"

#include "Convert.h"
#include "HumRegex.h"

#include <algorithm>

using namespace std;

namespace hum {

// START_MERGE


/////////////////////////////////
//
// Tool_double::Tool_double -- Set the recognized options for the tool.
//

Tool_double::Tool_double(void) {
}



/////////////////////////////////
//
// Tool_double::run -- Primary interfaces to the tool.
//

bool Tool_double::run(HumdrumFileSet& infiles) {
	bool status = true;
	for (int i=0; i<infiles.getCount(); i++) {
		status &= run(infiles[i]);
	}
	return status;
}


bool Tool_double::run(const string& indata, ostream& out) {
	HumdrumFile infile(indata);
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_double::run(HumdrumFile& infile, ostream& out) {
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}

//
// In-place processing of file:
//

bool Tool_double::run(HumdrumFile& infile) {
	processFile(infile);

	// Re-load the text for each line from their tokens.
	infile.createLinesFromTokens();

	// Need to adjust the line numbers for tokens for later
	// processing.
	m_humdrum_text << infile;
	return true;
}



//////////////////////////////
//
// Tool_double::processFile --
//

void Tool_double::processFile(HumdrumFile& infile) {
	terminalBreveToTerminalLong(infile);
	doubleRhythms(infile);
	adjustBeams(infile);
}



//////////////////////////////
//
// Tool_double::terminalBreveToTerminalLong --
//

void Tool_double::terminalBreveToTerminalLong(HumdrumFile& infile) {
	HumRegex hre;
	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isReference()) {
			continue;
		}
		HTp token = infile.token(i, 0);
		if (token->find("terminal breve") == string::npos) {
			continue;
		}
		string text = *token;
		hre.replaceDestructive(text, "terminal long", "terminal breve", "g");
		token->setText(text);
	}
}


//////////////////////////////
//
// Tool_double::adjustBeams -- Assuming non-lazy beams.
//

void Tool_double::adjustBeams(HumdrumFile& infile) {
	for (int i=0; i<infile.getStrandCount(); i++) {
		HTp sstart = infile.getStrandStart(i);
		if (!sstart->isKern()) {
			continue;
		}
		HTp send   = infile.getStrandEnd(i);
		adjustBeams(sstart, send);
	}
}


void Tool_double::adjustBeams(HTp sstart, HTp send) {
	// Remove one level of beaming from notes.  This method
	// requires non-lazy beaming.
	HTp current = sstart;
	vector<HTp> notes;
	current = current->getNextToken();
	while (current) {
		if (current->isBarline()) {
			processBeamsForMeasure(notes);
			notes.clear();
			current = current->getNextToken();
			continue;
		}
		if (!current->isData()) {
			current = current->getNextToken();
			continue;
		}
		if (current->isNull()) {
			current = current->getNextToken();
			continue;
		}
		notes.push_back(current);
		current = current->getNextToken();
	}
}



//////////////////////////////
//
// Tool_double::processBeamsForMeasure --
//

void Tool_double::processBeamsForMeasure(vector<HTp>& notes) {
	int lastlevel = 0;
	int level = 0;
	HumRegex hre;
	for (int i=0; i<(int)notes.size(); i++) {
		int Lcount = 0;
		int Jcount = 0;
		for (int j=0; j<(int)notes[i]->size(); j++) {
			if (notes[i]->at(j) == 'L') {
				Lcount++;
			} else if (notes[i]->at(j) == 'J') {
				Jcount++;
			}
		}
		level += Lcount - Jcount;
		if ((lastlevel == 0) && (level > 0)) {
			// remove one L:
			string text = *notes[i];
			hre.replaceDestructive(text, "", "L");
			notes[i]->setText(text);
		} else if ((level == 0) && (lastlevel > 0)) {
			// remove one J:
			string text = *notes[i];
			hre.replaceDestructive(text, "", "J");
			notes[i]->setText(text);
		}

		if (notes[i]->find("k") != string::npos) {
			if ((level == 0) && (lastlevel == 1)) {
				// remove k:
				string text = *notes[i];
				hre.replaceDestructive(text, "", "k");
				notes[i]->setText(text);
			}
		}

		if (notes[i]->find("K") != string::npos) {
			if ((level == 1) && (lastlevel == 0)) {
				// remove K:
				string text = *notes[i];
				hre.replaceDestructive(text, "", "K");
				notes[i]->setText(text);
			}
		}

		lastlevel = level;
	}
}



//////////////////////////////
//
// Tool_double::doubleRhythms --
//

void Tool_double::doubleRhythms(HumdrumFile& infile) {
	HumRegex hre;
	for (int i=0; i<infile.getLineCount(); i++) {
		if (infile[i].isData()) {
			for (int j=0; j<infile[i].getFieldCount(); j++) {
				HTp token = infile.token(i, j);
				if (!token->isKern()) {
					continue;
				}
				if (token->isNull()) {
					continue;
				}

				// extract duration without dot
				string text = token->getText();
				HumNum durnodot = Convert::recipToDurationNoDots(text);
				durnodot *= 2;
				string newrhythm = Convert::durationToRecip(durnodot);
				hre.replaceDestructive(text, newrhythm, "\\d+%?\\d*");
				token->setText(text);
			}
		} else if (infile[i].isInterpretation()) {
			// Double time signature bottom numbers:
			for (int j=0; j<infile[i].getFieldCount(); j++) {
				HTp token = infile.token(i, j);
				if (hre.search(token, "^\\*M(\\d+)/(\\d+)")) {
					int bot = hre.getMatchInt(2);
					if (bot == 4) {
						bot = 2;
					} else if (bot == 2) {
						bot = 1;
					} else if (bot == 1) {
						bot = 0;
					} else {
						cerr << "Warning: ignored time signature: " << token << endl;
					}
					string text = *token;
					string replacement = "/" + to_string(bot);
					hre.replaceDestructive(text, replacement, "/\\d+");
					token->setText(text);
				}
			}
		}
	}
}


// END_MERGE

} // end namespace hum



