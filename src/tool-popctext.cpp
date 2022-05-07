//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Fri May  6 19:30:42 PDT 2022
// Last Modified: Fri May  6 19:30:45 PDT 2022
// Filename:      tool-popctext.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/tool-popctext.cpp
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Modernize POPC2 project text.
//

#include "tool-popctext.h"
#include "HumRegex.h"

using namespace std;

namespace hum {

// START_MERGE


/////////////////////////////////
//
// Tool_popctext::Tool_popctext -- Set the recognized options for the tool.
//

Tool_popctext::Tool_popctext(void) {
	define("T|no-text|no-lyrics=b", "Do not convert lyrics in **text spines.");
	define("L|no-local=b", "Do not convert local LO t parameters.");
	define("G|no-global=b", "Do not convert global LO t parameters.");
	define("R|no-reference=b", "Do not convert reference record values.");
}


/////////////////////////////////
//
// Tool_popctext::run -- Do the main work of the tool.
//

bool Tool_popctext::run(HumdrumFileSet& infiles) {
	bool status = true;
	for (int i=0; i<infiles.getCount(); i++) {
		status &= run(infiles[i]);
	}
	return status;
}



bool Tool_popctext::run(const string& indata, ostream& out) {
	HumdrumFile infile(indata);
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_popctext::run(HumdrumFile& infile, ostream& out) {
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_popctext::run(HumdrumFile& infile) {
	initialize();
	processFile(infile);
	infile.createLinesFromTokens();
	m_humdrum_text << infile;
	return true;
}



//////////////////////////////
//
// Tool_popctext::initialize --  Initializations that only have to be done once
//    for all HumdrumFile segments.
//

void Tool_popctext::initialize(void) {
	m_lyricsQ    = !getBoolean("T");
	m_localQ     = !getBoolean("L");
	m_globalQ    = !getBoolean("G");
	m_referenceQ = !getBoolean("R");
}



//////////////////////////////
//
// Tool_popctext::processFile --
//

void Tool_popctext::processFile(HumdrumFile& infile) {
	if (m_lyricsQ) {
		convertTextSpines(infile);
	}
	if (m_localQ) {
		convertLocalLayoutText(infile);
	}
	if (m_globalQ) {
		convertGlobalLayoutText(infile);
	}
	if (m_referenceQ) {
		convertReferenceText(infile);
	}
}



//////////////////////////////
//
// Tool_popctext::convertTextSpines --
//

void Tool_popctext::convertTextSpines(HumdrumFile& infile) {
	int scount = infile.getStrandCount();
	for (int i=0; i<scount; i++) {
		HTp stok = infile.getStrandStart(i);
		if (!stok->isDataType("**text")) {
			continue;
		}
		HTp etok = infile.getStrandEnd(i);
		processTextStrand(stok, etok);
	}
}



//////////////////////////////
//
// Tool_popctext::processTextStrand --
//

void Tool_popctext::processTextStrand(HTp stok, HTp etok) {
	HTp current = stok;
	while (current && (current != etok)) {
		if (!current->isData()) {
			current = current->getNextToken();
			continue;
		}
		if (current->isNull()) {
			current = current->getNextToken();
			continue;
		}

		string text = modernizeText(*current);
		if (text != *current) {
			current->setText(text);
		}

		current = current->getNextToken();
	}
}



//////////////////////////////
//
// Tool_popctext::convertReferenceText --
//

void Tool_popctext::convertReferenceText(HumdrumFile& infile) {
	HumRegex hre;
	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isGlobalReference()) {
			continue;
		}

		HTp token = infile.token(i, 0);
		if (!hre.search(token, "^!!![^:]+:(.*)$")) {
			continue;
		}
		string oldcontents = hre.getMatch(1);
		if (oldcontents == "") {
			return;
		}
		string newcontents = modernizeText(oldcontents);
		if (oldcontents != newcontents) {
			string text = *token;
			hre.replaceDestructive(text, ":" + newcontents, ":" + oldcontents);
			token->setText(text);
		}
	}
}



//////////////////////////////
//
// Tool_popctext::convertGlobalLayoutText --
//

void Tool_popctext::convertGlobalLayoutText(HumdrumFile& infile) {
	HumRegex hre;
	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isCommentGlobal()) {
			continue;
		}

		HTp token = infile.token(i, 0);
		if (!hre.search(token, "^!!LO:.*:t=([^:]+)")) {
			continue;
		}
		string oldcontents = hre.getMatch(1);
		string newcontents = modernizeText(oldcontents);
		if (oldcontents != newcontents) {
			string text = *token;
			hre.replaceDestructive(text, ":t=" + newcontents, ":t=" + oldcontents);
			token->setText(text);
		}
	}
}



//////////////////////////////
//
// Tool_popctext::convertLocalLayoutText --
//

void Tool_popctext::convertLocalLayoutText(HumdrumFile& infile) {
	HumRegex hre;
	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isCommentLocal()) {
			continue;
		}
		for (int j=0; j<infile[i].getFieldCount(); j++) {
			HTp token = infile.token(i, j);
			if (*token == "!") {
				continue;
			}
			if (!hre.search(token, "^!LO:.*:t=([^:]+)")) {
				continue;
			}
			string oldcontents = hre.getMatch(1);
			string newcontents = modernizeText(oldcontents);
			if (oldcontents != newcontents) {
				string text = *token;
				hre.replaceDestructive(text, ":t=" + newcontents, ":t=" + oldcontents);
				token->setText(text);
			}
		}
	}
}



//////////////////////////////
//
// Tool_popctext::modernizeText --
//

string Tool_popctext::modernizeText(const string& input) {
	string output = input;
	HumRegex hre;
	
	// Long s:
	hre.replaceDestructive(output, "s", "ſ", "g");
	hre.replaceDestructive(output, "s", "ʃ", "g");
	hre.replaceDestructive(output, "s", "&#383;", "g");

	// Greek borrowings:
	hre.replaceDestructive(output, "u", "ν", "g");
	hre.replaceDestructive(output, "í", "ί", "g");
	hre.replaceDestructive(output, "a", "α", "g");

	hre.replaceDestructive(output, "k", "ť", "g");

	// Zs:
	hre.replaceDestructive(output, "z", "ᴣ", "g");
	hre.replaceDestructive(output, "z̨", "ʓ", "g");
	hre.replaceDestructive(output, "ż", "ʒ̇", "g");
	hre.replaceDestructive(output, "ź", "ʒ́", "g");

	// Capital Zs:
	hre.replaceDestructive(output, "Ż", "Ʒ̇", "g");
	hre.replaceDestructive(output, "Ź", "Ʒ́", "g");

	// Ligatures
	hre.replaceDestructive(output, "æ", "ae", "g");

	return output;
}



// END_MERGE

} // end namespace hum



