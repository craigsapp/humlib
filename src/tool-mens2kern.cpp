//
// Programmer:    Martha Thomae
// Creation Date: Mon Sep 28 12:08:25 PDT 2020
// Last Modified: Sun Mar  7 06:53:52 PST 2021
// Filename:      tool-mens2kern.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/tool-mens2kern.cpp
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Convert from **mens to **kern representations.
//

#include "tool-mens2kern.h"
#include "HumRegex.h"

using namespace std;

namespace hum {

// START_MERGE


/////////////////////////////////
//
// Tool_mens2kern::Tool_mens2kern -- Set the recognized options for the tool.
//

Tool_mens2kern::Tool_mens2kern(void) {
	define("debug=b",    "print debugging statements");
}



/////////////////////////////////
//
// Tool_mens2kern::run -- Do the main work of the tool.
//

bool Tool_mens2kern::run(HumdrumFileSet& infiles) {
	bool status = true;
	for (int i=0; i<infiles.getCount(); i++) {
		status &= run(infiles[i]);
	}
	return status;
}


bool Tool_mens2kern::run(const string& indata, ostream& out) {
	HumdrumFile infile(indata);
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_mens2kern::run(HumdrumFile& infile, ostream& out) {
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_mens2kern::run(HumdrumFile& infile) {
	initialize();
	processFile(infile);
	return true;
}



//////////////////////////////
//
// Tool_mens2kern::initialize --  Initializations that only have to be done once
//    for all HumdrumFile segments.
//

void Tool_mens2kern::initialize(void) {
	m_debugQ = getBoolean("debug");
}



//////////////////////////////
//
// Tool_mens2kern::processFile --
//

void Tool_mens2kern::processFile(HumdrumFile& infile) {
	vector<HTp> melody;
	int scount = infile.getStrandCount();
	for (int i=0; i<scount; i++) {
		HTp sstart = infile.getStrandBegin(i);
		if (!sstart->isDataType("**mens")) {
			continue;
		}
		HTp sstop = infile.getStrandEnd(i);
		HTp current = sstart;
		while (current && (current != sstop)) {
			if (current->isNull()) {
				// ignore null data tokens
				current = current->getNextToken();
				continue;
			}
			melody.push_back(current);
			current = current->getNextToken();
		}
		processMelody(melody);
		melody.clear();
	}

	infile.createLinesFromTokens();
}



//////////////////////////////
//
// Tool_mens2kern::processMelody --
//

void Tool_mens2kern::processMelody(vector<HTp>& melody) {
	int maximodus = 0;
	int modus = 0;
	int tempus = 0;
	int prolatio = 0;
	int semibrevis_def = 0;
	int brevis_def     = 0;
	int longa_def      = 0;
	int maxima_def     = 0;
	string regexopts;
	HumRegex hre;
	string rhythm;
	bool imperfecta;
	bool perfecta;
	bool altera;

	for (int i=0; i<(int)melody.size(); i++) {
		if (*melody[i] == "**mens") {
			// convert spine to **kern data:
			melody[i]->setText("**kern");
		}

		if (melody[i]->isMensuration()) {
			getMensuralInfo(melody[i], maximodus, modus, tempus, prolatio);

			// Default value of notes from maxima to semibrevis in minims:
			semibrevis_def = prolatio;
			brevis_def     = tempus    * semibrevis_def;
			longa_def      = modus     * brevis_def;
			maxima_def     = maximodus * longa_def;
			if (m_debugQ) {
				cerr << "LEVELS X_def = "    << maxima_def
					  << " | L_def = " << longa_def
					  << " | S_def = " << brevis_def
					  << " | s_def = " << semibrevis_def << endl;
			}
		}

		if (!melody[i]->isData()) {
			continue;
		}
		string text = melody[i]->getText();
		imperfecta = hre.search(text, "i") ? true : false;
		perfecta = hre.search(text, "p") ? true : false;
		altera = hre.search(text, "\\+") ? true : false;
		if (hre.search(text, "([XLSsMmUu])")) {
			rhythm = hre.getMatch(1);
		} else {
			cerr << "Error: token " << melody[i] << " has no rhythm" << endl;
			cerr << "   ON LINE: "  << melody[i]->getLineNumber()    << endl;
			continue;
		}

		string kernRhythm = mens2kernRhythm(rhythm, altera, perfecta, imperfecta, maxima_def, longa_def, brevis_def, semibrevis_def);

		hre.replaceDestructive(text, kernRhythm, rhythm);
		// Remove any dot of division/augmentation
		hre.replaceDestructive(text, "", ":");
		// remove perfection/imperfection/alteration markers
		hre.replaceDestructive(text, "", "[pi\\+]");
		if (text.empty()) {
			text = ".";
		}
		melody[i]->setText(text);
	}
}


//////////////////////////////
//
// Tool_mens2kern::getMensuralInfo --
//

void Tool_mens2kern::getMensuralInfo(HTp token, int& maximodus, int& modus,
		int& tempus, int& prolatio) {
	HumRegex hre;
	if (!hre.search(token, "^\\*met\\(.*?\\)_(\\d+)")) {
		// need to interpret symbols without underscores.
		if (token->getText() == "*met(C)") {
			maximodus = 2;
			modus = 2;
			tempus = 2;
			prolatio = 2;
		} else if (token->getText() == "*met(O)") {
			maximodus = 2;
			modus = 2;
			tempus = 3;
			prolatio = 2;
		} else if (token->getText() == "*met(C.)") {
			maximodus = 2;
			modus = 2;
			tempus = 2;
			prolatio = 3;
		} else if (token->getText() == "*met(O.)") {
			maximodus = 2;
			modus = 2;
			tempus = 3;
			prolatio = 3;
		} else if (token->getText() == "*met(C|)") {
			maximodus = 2;
			modus = 2;
			tempus = 2;
			prolatio = 2;
		} else if (token->getText() == "*met(O|)") {
			maximodus = 2;
			modus = 2;
			tempus = 3;
			prolatio = 2;
		} else if (token->getText() == "*met(C.|)") {
			maximodus = 2;
			modus = 2;
			tempus = 2;
			prolatio = 3;
		} else if (token->getText() == "*met(O.|)") {
			maximodus = 2;
			modus = 2;
			tempus = 3;
			prolatio = 3;
		} else if (token->getText() == "*met(C2)") {
			maximodus = 2;
			modus = 2;
			tempus = 2;
			prolatio = 2;
		} else if (token->getText() == "*met(C3)") {
			maximodus = 2;
			modus = 2;
			tempus = 3;
			prolatio = 2;
		} else if (token->getText() == "*met(O2)") {
			maximodus = 2;
			modus = 3;
			tempus = 2;
			prolatio = 2;
		} else if (token->getText() == "*met(O3)") {
			maximodus = 3;
			modus = 3;
			tempus = 3;
			prolatio = 2;
		} else if (token->getText() == "*met(C3/2)") {
			maximodus = 2;
			modus = 2;
			tempus = 2;
			prolatio = 3;
		} else if (token->getText() == "*met(C|3/2)") {
			maximodus = 2;
			modus = 2;
			tempus = 3;
			prolatio = 2;
		}
	} else {
		string levels = hre.getMatch(1);
		if (levels.size() >= 1) {
			maximodus = levels[0] - '0';
		}
		if (levels.size() >= 2) {
			modus = levels[1] - '0';
		}
		if (levels.size() >= 3) {
			tempus = levels[2] - '0';
		}
		if (levels.size() >= 4) {
			prolatio = levels[3] - '0';
		}
	}

	if (m_debugQ) {
		cerr << "MENSURAL INFO: maximodus = "   << maximodus
			  << " | modus = "    << modus
			  << " | tempus = "   << tempus
			  << " | prolatio = " << prolatio << endl;
	}
}



//////////////////////////////
//
// Tool_mens2kern::mens2kernRhythm --
//

string Tool_mens2kern::mens2kernRhythm(const string& rhythm, bool altera, bool perfecta, bool imperfecta, int maxima_def, int longa_def, int brevis_def, int semibrevis_def) {
	double val_note;
	double minima_def = 1;
	double semiminima_def = 0.5;
	double fusa_def = 0.25;
	double semifusa_def = 0.125;

	if (rhythm == "X") {
		if (perfecta) { val_note = 3 * longa_def; }
		else if (imperfecta) { val_note = 2 * longa_def; }
		else { val_note = maxima_def; }
	}
	else if (rhythm == "L") {
		if (perfecta) { val_note = 3 * brevis_def; }
		else if (imperfecta) { val_note = 2 * brevis_def; }
		else if (altera) { val_note = 2 * longa_def; }
		else { val_note = longa_def; }
	}
	else if (rhythm == "S") {
		if (perfecta) { val_note = 3 * semibrevis_def; }
		else if (imperfecta) { val_note = 2 * semibrevis_def; }
		else if (altera) { val_note = 2 * brevis_def; }
		else { val_note = brevis_def; }
	}
	else if (rhythm == "s") {
		if (perfecta) { val_note = 3 * minima_def; }
		else if (imperfecta) { val_note = 2 * minima_def; }
		else if (altera) { val_note = 2 * semibrevis_def; }
		else { val_note = semibrevis_def; }
	}
	else if (rhythm == "M") {
		if (perfecta) { val_note = 1.5 * minima_def; }
		else if (altera) { val_note = 2 * minima_def; }
		else { val_note = minima_def; }
	}
	else if (rhythm == "m") {
		if (perfecta) { val_note = 1.5 * semiminima_def; }
		else { val_note = semiminima_def; }
	}
	else if (rhythm == "U") {
		if (perfecta) { val_note = 1.5 * fusa_def; }
		else { val_note = fusa_def; }
	}
	else if (rhythm == "u") {
		if (perfecta) { val_note = 1.5 * semifusa_def; }
		else { val_note = semifusa_def; }
	}
	else { cerr << "UNKNOWN RHYTHM: " << rhythm << endl; return ""; }

	switch ((int)(val_note * 10000)) {
		case 1250:     return "16";   break;   // sixteenth note
		case 1875:     return "16.";  break;   // dotted sixteenth note
		case 2500:     return "8";    break;   // eighth note
		case 3750:     return "8.";   break;   // dotted eighth note
		case 5000:     return "4";    break;   // quarter note
		case 7500:     return "4.";   break;   // dotted quarter note
		case 10000:    return "2";    break;   // half note
		case 15000:    return "2.";   break;   // dotted half note
		case 20000:    return "1";    break;   // whole note
		case 30000:    return "1.";   break;   // dotted whole note
		case 40000:    return "0";    break;   // breve note
		case 60000:    return "0.";   break;   // dotted breve note
		case 90000:    return "2%9";  break;   // or ["0.", "1."];
		case 80000:    return "00";   break;   // long note
		case 120000:   return "00.";  break;   // dotted long note
		case 180000:   return "1%9";  break;   // or ["00.", "0."];
		case 270000:   return "2%27"; break;   // or ["0.", "1.", "0.", "1.", "0.", "1."];
		case 160000:   return "000";  break;   // maxima note
		case 240000:   return "000."; break;   // dotted maxima note
		case 360000:   return "1%18"; break;   // or ["000.", "00."];
		case 540000:   return "1%27"; break;   // or ["00.", "0.", "00.", "0.", "00.", "0."];
		case 810000:   return "2%81"; break;   // or ["00.", "0.", "00.", "0.", "00.", "0.", "0.", "1.", "0.", "1.", "0.", "1."];
		default:
			cerr << "Error: unknown val_note: " << val_note << endl;
	}

	return "";
}


// END_MERGE

} // end namespace hum



