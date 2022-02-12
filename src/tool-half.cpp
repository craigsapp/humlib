//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Fri Jul 30 07:07:48 CEST 2021
// Last Modified: Sat Feb 12 10:53:23 PST 2022
// Filename:      tool-half.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/tool-half.cpp
// Syntax:        C++11; humlib
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Double note/rest/time signature durations.
//

#include "tool-half.h"

#include "Convert.h"
#include "HumRegex.h"
#include "tool-autobeam.h"

#include <algorithm>

using namespace std;

namespace hum {

// START_MERGE


/////////////////////////////////
//
// Tool_half::Tool_half -- Set the recognized options for the tool.
//

Tool_half::Tool_half(void) {
	define("l|lyric-beam-break=b", "Break beams at syllable starts");
}



/////////////////////////////////
//
// Tool_half::run -- Primary interfaces to the tool.
//

bool Tool_half::run(HumdrumFileSet& infiles) {
	bool status = true;
	for (int i=0; i<infiles.getCount(); i++) {
		status &= run(infiles[i]);
	}
	return status;
}


bool Tool_half::run(const string& indata, ostream& out) {
	HumdrumFile infile(indata);
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_half::run(HumdrumFile& infile, ostream& out) {
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

bool Tool_half::run(HumdrumFile& infile) {
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
// Tool_half::processFile --
//

void Tool_half::processFile(HumdrumFile& infile) {
	m_lyricBreakQ = getBoolean("lyric-beam-break");
	terminalLongToTerminalBreve(infile);
	halfRhythms(infile);
	adjustBeams(infile);
}



//////////////////////////////
//
// Tool_half::adjustBeams --
//

void Tool_half::adjustBeams(HumdrumFile& infile) {
	Tool_autobeam autobeam;
	vector<string> argv;
	argv.push_back("autobeam");
	if (m_lyricBreakQ) {
		argv.push_back("-l");
	}
	autobeam.process(argv);
	autobeam.run(infile);
}



//////////////////////////////
//
// Tool_half::halfRhythms --
//

void Tool_half::halfRhythms(HumdrumFile& infile) {
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

				string text = *token;
				// extract duration without dot
				HumNum durnodot = Convert::recipToDurationNoDots(text);
				durnodot /= 2;
				string newrhythm = Convert::durationToRecip(durnodot);
				hre.replaceDestructive(text, newrhythm, "\\d+%?\\d*");
				token->setText(text);
			}
		} else if (infile[i].isInterpretation()) {
			// half time signatures
			for (int j=0; j<infile[i].getFieldCount(); j++) {
				HTp token = infile.token(i, j);
				if (hre.search(token, "^\\*M(\\d+)/(\\d+)%(\\d+)")) {
					int bot1 = hre.getMatchInt(2);
					int bot2 = hre.getMatchInt(3);
					if (bot2 % 2) {
						cerr << "Cannot handle conversion of time signature " << token << endl;
						continue;
					}
					bot2 /= 2;
					if (bot2 == 1) {
						string text = *token;
						string replacement = "/" + to_string(bot1);
						hre.replaceDestructive(text, replacement, "/\\d+%\\d+");
						token->setText(text);
					} else {
						string text = *token;
						string replacement = "/" + to_string(bot1);
						replacement += "%" + to_string(bot2);
						hre.replaceDestructive(text, replacement, "/\\d+");
						token->setText(text);
					}
				} else if (hre.search(token, "^\\*M(\\d+)/(\\d+)")) {
					int bot = hre.getMatchInt(2);
					if (bot == 4) {
						bot = 8;
					} else if (bot == 2) {
						bot = 4;
					} else if (bot == 3) {
						bot = 6;
					} else if (bot == 1) {
						bot = 2;
					} else if (bot == 0) {
						bot = 1;
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



//////////////////////////////
//
// Tool_half::terminalLongToTerminalBreve --
//

void Tool_half::terminalLongToTerminalBreve(HumdrumFile& infile) {
	HumRegex hre;
	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isReference()) {
			continue;
		}
		HTp token = infile.token(i, 0);
		if (token->find("terminal long") == string::npos) {
			continue;
		}
		string text = *token;
		hre.replaceDestructive(text, "terminal breve", "terminal long", "g");
		token->setText(text);
	}
}


// END_MERGE

} // end namespace hum



