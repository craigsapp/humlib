//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Fri May  6 19:30:42 PDT 2022
// Last Modified: Fri May  6 19:30:45 PDT 2022
// Filename:      tool-hands.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/tool-hands.cpp
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   
//

#include "HumRegex.h"

#include "tool-hands.h"

using namespace std;

namespace hum {

// START_MERGE


/////////////////////////////////
//
// Tool_hands::Tool_hands -- Set the recognized options for the tool.
//

Tool_hands::Tool_hands(void) {
	define("c|color=b", "color right-hand notes red and left-hand notes blue");
	define("lcolor|left-color=s:dodgerblue", "color of left-hand notes");
	define("rcolor|right-color=s:crimson",   "color of right-hand notes");
	define("l|left-only=b",                  "remove right-hand notes");
	define("r|right-only=b",                 "remove left-hand notes");
	define("m|mark=b",                       "mark left and right-hand notes");
	define("a|attacks-only=b",               "only mark note attacks and not note sustains");
}



//////////////////////////////
//
// Tool_hands::initialize --  Initializations that only have to be done once
//    for all HumdrumFile segments.
//

void Tool_hands::initialize(void) {
	m_colorQ       = getBoolean("color");
	m_leftColor    = getString("left-color");
	m_rightColor   = getString("right-color");
	m_leftOnlyQ    = getBoolean("left-only");
	m_rightOnlyQ   = getBoolean("right-only");
	m_markQ        = getBoolean("mark");
	m_attacksOnlyQ = getBoolean("attacks-only");
}



/////////////////////////////////
//
// Tool_hands::run -- Do the main work of the tool.
//

bool Tool_hands::run(HumdrumFileSet& infiles) {
	bool status = true;
	for (int i=0; i<infiles.getCount(); i++) {
		status &= run(infiles[i]);
	}
	return status;
}



bool Tool_hands::run(const string& indata, ostream& out) {
	HumdrumFile infile(indata);
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_hands::run(HumdrumFile& infile, ostream& out) {
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_hands::run(HumdrumFile& infile) {
	initialize();
	processFile(infile);
	return true;
}



//////////////////////////////
//
// Tool_hands::processFile --
//

void Tool_hands::processFile(HumdrumFile& infile) {
	if (m_markQ || m_leftOnlyQ || m_rightOnlyQ) {
		infile.doHandAnalysis(m_attacksOnlyQ);
	}
	if (m_leftOnlyQ) {
		removeNotes(infile, "RH");
	} else if (m_rightOnlyQ) {
		removeNotes(infile, "LH");
	}
	if (m_colorQ) {
		colorHands(infile);
	} else if (m_markQ) {
		markNotes(infile);
	}
	m_humdrum_text << infile;
}



//////////////////////////////
//
// Tool_hands::removeNotes --
//

void Tool_hands::removeNotes(HumdrumFile& infile, const string& htype) {
	int counter = 0;
	int scount = infile.getStrandCount();
	for (int i=0; i<scount; i++) {
		HTp sstart = infile.getStrandStart(i);
		HTp xtok = sstart->getExclusiveInterpretation();
		int hasHandMarkup = xtok->getValueInt("auto", "hand");
		if (!hasHandMarkup) {
			continue;
		}
		HTp send = infile.getStrandEnd(i);
		removeNotes(sstart, send, htype);
		counter++;
	}

	
	if (counter) {
		infile.createLinesFromTokens();
	}
}


void Tool_hands::removeNotes(HTp sstart, HTp send, const string& htype) {
	HTp current = sstart;
	while (current && (current != send)) {
		if (!current->isData() || current->isNull()) {
			current = current->getNextToken();
			continue;
		}

		HumRegex hre;
		string ttype = current->getValue("auto", "hand");
		if (ttype != htype) {
			current = current->getNextToken();
			continue;
		}
		string text = *current;
		hre.replaceDestructive(text, "", "[^0-9.%q ]", "g");
		hre.replaceDestructive(text, "ryy ", " ", "g");
		text += "ryy";
		current->setText(text);
		current = current->getNextToken();
	}
}



//////////////////////////////
//
// Tool_hands::markNotes --
//

void Tool_hands::markNotes(HumdrumFile& infile) {
	HumRegex hre;

	int counter = 0;
	int scount = infile.getStrandCount();
	for (int i=0; i<scount; i++) {
		HTp sstart = infile.getStrandStart(i);
		HTp xtok = sstart->getExclusiveInterpretation();
		int hasHandMarkup = xtok->getValueInt("auto", "hand");
		if (!hasHandMarkup) {
			continue;
		}
		HTp send   = infile.getStrandEnd(i);
		markNotes(sstart, send);
		counter++;
	}

	if (counter) {
		infile.appendLine("!!!RDF**kern: " + m_leftMarker + " = marked note, color=\"" + m_leftColor + "\", left-hand note");
		infile.appendLine("!!!RDF**kern: " + m_rightMarker + " = marked note, color=\"" + m_rightColor + "\", right-hand note");
		infile.createLinesFromTokens();
	}
}


void Tool_hands::markNotes(HTp sstart, HTp send) {
	HTp current = sstart;
	while (current && (current != send)) {
		if (!current->isData() || current->isNull() || current->isRest()) {
			current = current->getNextToken();
			continue;
		}

		HumRegex hre;
		string text = *current;
		string htype = current->getValue("auto", "hand");
		if (htype == "LH") {
			hre.replaceDestructive(text, " " + m_leftMarker, " +", "g");
			text = m_leftMarker + text;
		} else if (htype == "RH") {
			hre.replaceDestructive(text, " " + m_rightMarker, " +", "g");
			text = m_rightMarker + text;
		}
		current->setText(text);
		current = current->getNextToken();
	}
}



//////////////////////////////
//
// Tool_hands::colorHands -- Convert for example *LH into *color:dodgerblue.
//

void Tool_hands::colorHands(HumdrumFile& infile) {
	string left = "*color:" + m_leftColor;
	string right = "*color:" + m_rightColor;
	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isInterpretation()) {
			continue;
		}
		bool changed = false;
		for (int j=0; j<infile[i].getFieldCount(); j++) {
			HTp token = infile.token(i, j);
			if (!token->isKern()) {
				continue;
			}
			if (*token == "*LH") {
				token->setText(left);
				changed = true;
			}
			if (*token == "*RH") {
				token->setText(right);
				changed = true;
			}
		}
		if (changed) {
			infile[i].createLineFromTokens();
		}
	}
}


// END_MERGE

} // end namespace hum



