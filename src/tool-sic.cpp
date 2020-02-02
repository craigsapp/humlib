//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Thu Jan 30 22:26:33 PST 2020
// Last Modified: Thu Jan 30 22:26:35 PST 2020
// Filename:      tool-sic.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/tool-sic.cpp
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Switch between sic encoding and corrected encoding.
//

#include "tool-sic.h"
#include "Convert.h"
#include "HumRegex.h"

using namespace std;

namespace hum {

// START_MERGE


/////////////////////////////////
//
// Tool_sic::Tool_sic -- Set the recognized options for the tool.
//

Tool_sic::Tool_sic(void) {
	define("s|substitution=b", "insert substitutions into music");
	define("o|original=b", "insert originals into music");
	define("r|remove=b", "remove sic layout tokens");
	define("v|verbose=b", "add verbose parameter");
	define("q|quiet=b", "remove verbose parameter");
}



/////////////////////////////////
//
// Tool_sic::run -- Do the main work of the tool.
//

bool Tool_sic::run(HumdrumFileSet& infiles) {
	bool status = true;
	for (int i=0; i<infiles.getCount(); i++) {
		status &= run(infiles[i]);
	}
	return status;
}


bool Tool_sic::run(const string& indata, ostream& out) {
	HumdrumFile infile(indata);
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_sic::run(HumdrumFile& infile, ostream& out) {
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_sic::run(HumdrumFile& infile) {
	initialize();
	if (!(m_substituteQ || m_originalQ || m_removeQ || m_verboseQ || m_quietQ)) {
		return true;
	}
	processFile(infile);
	return true;
}



//////////////////////////////
//
// Tool_sic::initialize --  Initializations that only have to be done once
//    for all HumdrumFile segments.
//

void Tool_sic::initialize(void) {
	m_substituteQ = getBoolean("substitution");
	m_originalQ   = getBoolean("original");
	m_removeQ     = getBoolean("remove");
	m_verboseQ    = getBoolean("verbose");
	m_quietQ      = getBoolean("quiet");
}



//////////////////////////////
//
// Tool_sic::processFile --
//

void Tool_sic::processFile(HumdrumFile& infile) {
	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isLocalComment()) {
			continue;
		}
		for (int j=0; j<infile[i].getFieldCount(); j++) {
			HTp token = infile[i].token(j);
			if (token->compare(0, 8, "!LO:SIC:") != 0) {
				continue;
			}
			if (m_verboseQ) {
				addVerboseParameter(token);
			} else if (m_quietQ) {
				removeVerboseParameter(token);
			}
			if (m_removeQ) {
				token->setText("!");
				m_modifiedQ = true;
			} else if (m_substituteQ) {
				insertSubstitutionToken(token);
			} else if (m_originalQ) {
				insertOriginalToken(token);
			}
		}
	}
	if (m_modifiedQ) {
		infile.createLinesFromTokens();
	}
}



//////////////////////////////
//
// Tool_sic::addVerboseParameter --
//

void Tool_sic::addVerboseParameter(HTp token) {
	HumRegex hre;
	string value = token->getText();
	if (hre.search(value, "(:v:)|(:v$)")) {
		return;
	}
	string newvalue = value + ":v";
	token->setText(newvalue);
	m_modifiedQ = true;
}



//////////////////////////////
//
// Tool_sic::removeVerboseParameter --
//

void Tool_sic::removeVerboseParameter(HTp token) {
	HumRegex hre;
	string value = token->getText();
	string newvalue = value;
	hre.replaceDestructive(newvalue, ":", ":v:", "g");
	hre.replaceDestructive(newvalue, "", ":v$", "");
	if (value == newvalue) {
		return;
	}
	token->setText(newvalue);
	m_modifiedQ = true;
}



//////////////////////////////
//
// Tool_sic::getTargetToken -- Get the token that the layout command
//    applies to.
//

HTp Tool_sic::getTargetToken(HTp stok) {
	HTp current = stok->getNextToken();
	while (current) {
		if (current->isNull()) {
			current = current->getNextToken();
			continue;
		}
		if (current->isManipulator()) {
			// Layout commands should not apply to manipulators nor be split
			// from their associated token.
			current = NULL;
			break;
		}
		if (current->isCommentLocal()) {
			current = current->getNextToken();
			continue;
		}
		break;
	}
	if (!current) {
		return NULL;
	}
	return current;
}



//////////////////////////////
//
// Tool_sic::insertSubstitutionToken --
//

void Tool_sic::insertSubstitutionToken(HTp sictok) {
	HTp target = getTargetToken(sictok);
	if (!target) {
		return;
	}
	HumRegex hre;
	vector<string> pieces;
	hre.split(pieces, *sictok, ":");
	string tstring = target->getText();
	string sstring;
	for (int i=2; i<(int)pieces.size(); i++) {
		if (pieces[i].compare(0, 2, "s=") == 0) {
			sstring = pieces[i].substr(2);
		}
	}
	if (sstring.empty()) {
		return;
	}
	target->setText(sstring);
	m_modifiedQ = true;
	string newsic = "!LO:SIC";
	for (int i=2; i<(int)pieces.size(); i++) {
		if (pieces[i].compare(0, 2, "s=") == 0) {
			newsic += ":o=" + tstring;
		} else {
			newsic += ":" + pieces[i];
		}
	}
	sictok->setText(newsic);
	m_modifiedQ = true;
}



//////////////////////////////
//
// Tool_sic::insertOriginalToken --
//

void Tool_sic::insertOriginalToken(HTp sictok) {
	HTp target = getTargetToken(sictok);
	if (!target) {
		return;
	}
	HumRegex hre;
	vector<string> pieces;
	hre.split(pieces, *sictok, ":");
	string tstring = target->getText();
	string sstring;
	for (int i=2; i<(int)pieces.size(); i++) {
		if (pieces[i].compare(0, 2, "o=") == 0) {
			sstring = pieces[i].substr(2);
		}
	}
	if (sstring.empty()) {
		return;
	}
	target->setText(sstring);
	m_modifiedQ = true;
	string newsic = "!LO:SIC";
	for (int i=2; i<(int)pieces.size(); i++) {
		if (pieces[i].compare(0, 2, "o=") == 0) {
			newsic += ":s=" + tstring;
		} else {
			newsic += ":" + pieces[i];
		}
	}
	sictok->setText(newsic);
	m_modifiedQ = true;
}



// END_MERGE

} // end namespace hum



