//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Thu Jan 30 22:26:33 PST 2020
// Last Modified: Thu Jan 30 22:26:35 PST 2020
// Filename:      tool-autoaccid.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/tool-autoaccid.cpp
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Add/remove visual/hidden accidental states on notes in the score
//

#include "tool-autoaccid.h"
#include "Convert.h"
#include "HumRegex.h"

using namespace std;

namespace hum {

// START_MERGE


/////////////////////////////////
//
// Tool_autoaccid::Tool_autoaccid -- Set the recognized options for the tool.
//

Tool_autoaccid::Tool_autoaccid(void) {
	define("x|visual=b",                        "mark visual accidentals only");
	define("y|suppressed=b",                    "mark hidden accidentals only");
	define("r|remove=b",                        "remove accidental qualifications");
	define("c|keep-cautionary|keep-courtesy=b", "keep cautionary accidentals when removing markers");
}



/////////////////////////////////
//
// Tool_autoaccid::run -- Do the main work of the tool.
//

bool Tool_autoaccid::run(HumdrumFileSet& infiles) {
	bool status = true;
	for (int i=0; i<infiles.getCount(); i++) {
		status &= run(infiles[i]);
	}
	return status;
}


bool Tool_autoaccid::run(const string& indata, ostream& out) {
	HumdrumFile infile(indata);
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_autoaccid::run(HumdrumFile& infile, ostream& out) {
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_autoaccid::run(HumdrumFile& infile) {
	processFile(infile);
	return true;
}



//////////////////////////////
//
// Tool_autoaccid::initialize --  Initializations that only have to be done once
//    for all HumdrumFile segments.
//

void Tool_autoaccid::initialize(void) {
	m_visualQ   = getBoolean("visual");
	m_hiddenQ   = getBoolean("suppressed");
	m_removeQ   = getBoolean("remove");
	m_cautionQ  = getBoolean("keep-cautionary");
}



//////////////////////////////
//
// Tool_autoaccid::processFile --
//

void Tool_autoaccid::processFile(HumdrumFile& infile) {
	initialize();
	if (m_removeQ) {
		removeAccidentalQualifications(infile);
	} else {
		infile.analyzeKernAccidentals();
		addAccidentalQualifications(infile);
	}
	infile.createLinesFromTokens();
}


//////////////////////////////
//
// Tool_autoaccid::addAccidentalQualifications --
//
//

void Tool_autoaccid::addAccidentalQualifications(HumdrumFile& infile) {
	int scount = infile.getStrandCount();
	HumRegex hre;
	for (int i=0; i<scount; i++) {
		HTp sbegin = infile.getStrandBegin(i);
		if (!sbegin->isKern()) {
			continue;
		}
		HTp send   = infile.getStrandEnd(i);
		HTp current = sbegin;
		while (current && (current != send)) {
			if (!current->isData()) {
				current = current->getNextToken();
				continue;
			}
			if (current->isNull()) {
				current = current->getNextToken();
				continue;
			}
			if (current->isRest()) {
				current = current->getNextToken();
				continue;
			}
			addAccidentalInfo(current);
			current = current->getNextToken();
		}
	}
}



//////////////////////////////
//
// Tool_autoaccid::addAccidentalInfo --
//
// Analysis have several parameters:
//      visualAccidental = true :: need to add an X after the accidental
//      visualAccidental = false :: need to add an y after the accidental
// also:
//      cautionaryAccidental = "true" ::
//      obligatoryAccidental = "true" ::
//

void Tool_autoaccid::addAccidentalInfo(HTp token) {
	vector<string> subtokens;
	subtokens = token->getSubtokens();
	if (subtokens.size() == 1) {
		bool visual = token->getValueBool("auto", "0", "visualAccidental");
		subtokens[0] = setVisualState(subtokens[0], visual);
	} else {
		for (int i=0; i<(int)subtokens.size(); i++) {
			bool visual = token->getValueBool("auto", to_string(i+1), "visualAccidental");
			subtokens[i] = setVisualState(subtokens[i], visual);
		}
	}
	string text;
	for (int i=0; i<(int)subtokens.size(); i++) {
		text += subtokens[i];
		if (i < (int)subtokens.size() - 1) {
			text += ' ';
		}
	}
	token->setText(text);
}



//////////////////////////////
//
// Tool_autoaccid::setVisualState --
//

string Tool_autoaccid::setVisualState(const string& input, bool state) {
	HumRegex hre;
	if (hre.search(input, "[-#n][Xy]")) {
		// do not remark accidental
		return input;
	}
	bool hasNatural = hre.search(input, "n");
	bool hasFlat    = hre.search(input, "-");
	bool hasSharp   = hre.search(input, "#");
	bool accidental = hasNatural || hasFlat || hasSharp;
	string output;
	if (m_visualQ) {
		if (state) {
			if (!accidental) {
				// need to show a natural accidental (and add the natural sign)
				output = hre.replaceCopy(input, "$1nX", "([A-Ga-g]+)");
			} else {
				// force accidental to display
				output = hre.replaceCopy(input, "$1X", "([-#n]+)");
			}
		} else {
			// do nothing
		}
	} else if (m_hiddenQ) {
		if (!state) {
			if (accidental) {
				// force accidental to be hidden
				output = hre.replaceCopy(input, "$1y", "([-#n]+)");
			} else {
				output = input;
			}
		}
	} else {
		// force display/hide state for accidental
		if (state) {
			if (!accidental) {
				// need to show a natural accidental (and add the natural sign)
				output = hre.replaceCopy(input, "$1nX", "([A-Ga-g]+)");
			} else {
				// force accidental to display
				output = hre.replaceCopy(input, "$1X", "([-#n]+)");
			}
		} else {
			if (accidental) {
				// force accidental to be hidden
				output = hre.replaceCopy(input, "$1y", "([-#n]+)");
			} else {
				output = input;
			}
		}
	}
	return output;
}



//////////////////////////////
//
// Tool_autoaccid::removeAccidentalQualifications --
//
//

void Tool_autoaccid::removeAccidentalQualifications(HumdrumFile& infile) {
	int scount = infile.getStrandCount();
	HumRegex hre;
	for (int i=0; i<scount; i++) {
		HTp sbegin = infile.getStrandStart(i);
		if (!sbegin->isKern()) {
			continue;
		}
		HTp send   = infile.getStrandEnd(i);
		HTp current = sbegin;
		while (current && (current != send)) {
			if (!current->isData()) {
				current = current->getNextToken();
				continue;
			}
			if (current->isNull()) {
				current = current->getNextToken();
				continue;
			}
			if (current->isRest()) {
				current = current->getNextToken();
				continue;
			}
			string text = current->getText();
			if (m_visualQ) {
				hre.replaceDestructive(text, "$1", "([-#n]+)X(?!X)", "g");
			} else if (m_hiddenQ) {
				hre.replaceDestructive(text, "$1", "([-#n]+)y(?!y)", "g");
			} else {
				hre.replaceDestructive(text, "$1", "([-#n]+)X(?!X)", "g");
				hre.replaceDestructive(text, "$1", "([-#n]+)y(?!y)", "g");
			}
		}
		current = current->getNextToken();
	}
}




// END_MERGE

} // end namespace hum



