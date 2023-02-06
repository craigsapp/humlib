//
// Programmer:    Wolfgang Drescher <drescher.wolfgang@gmail.com>
// Creation Date: Sa  4 Feb 2023 16:27:58 CET
// Filename:      tool-worex.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/tool-worex.cpp
// Syntax:        C++11; humlib
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Add melismatic underlines at ending syllables of words
//

#include "tool-worex.h"
#include "Convert.h"
#include "HumRegex.h"
#include <regex>
#include <cmath>

using namespace std;

namespace hum {

// START_MERGE

//////////////////////////////
//
// Tool_worex::Tool_worex -- Set the recognized options for the tool.
//

Tool_worex::Tool_worex(void) {
	define("r|remove=b", "Remove melismatic underlines at ending syllables of words");
}



//////////////////////////////
//
// Tool_worex::run -- Do the main work of the tool.
//

bool Tool_worex::run(HumdrumFileSet &infiles) {
	bool status = true;
	for (int i = 0; i < infiles.getCount(); i++) {
		status &= run(infiles[i]);
	}
	return status;
}

bool Tool_worex::run(const string &indata, ostream &out) {
	HumdrumFile infile(indata);
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}

bool Tool_worex::run(HumdrumFile &infile, ostream &out) {
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}

bool Tool_worex::run(HumdrumFile &infile) {
	initialize();
	processFile(infile);
	return true;
}



//////////////////////////////
//
// Tool_worex::initialize -- 
//

void Tool_worex::initialize(void) {
	m_removeWordExtenderQ = getBoolean("remove");
}



//////////////////////////////
//
// Tool_worex::processFile -- 
//

void Tool_worex::processFile(HumdrumFile& infile) {

	vector<HTp> textSpinesStartList;
	infile.getSpineStartList(textSpinesStartList, "**text");	

	if (m_removeWordExtenderQ) {
		removeWordExtenders(textSpinesStartList);
	} else {
		addWordExtenders(textSpinesStartList);
	}

	infile.createLinesFromTokens();

	m_humdrum_text << infile;
}



//////////////////////////////
//
// Tool_worex::addWordExtenders -- 
//

void Tool_worex::addWordExtenders(vector<HTp> spineStartList) {
	for (HTp token : spineStartList) {
		if (!token->isDataType("**text")) {
			continue;
		}
		HTp currentToken = token;
		while (currentToken) {
			HTp nextToken = currentToken->getNextToken();
			while (nextToken != NULL && !nextToken->isData()) {
				nextToken = nextToken->getNextToken();
			}
			if (nextToken == NULL) {
				break;
			}
			string syllableExtender = "-";
			bool isWordEnd = equal(syllableExtender.rbegin(), syllableExtender.rend(), currentToken->getText().rbegin()) == false;
			string wordEndExtender = "_";
			bool endsWithUnderscore = equal(wordEndExtender.rbegin(), wordEndExtender.rend(), currentToken->getText().rbegin()) == true;
			if (nextToken->isNull() && currentToken->isNonNullData() && isWordEnd && !endsWithUnderscore) {
				currentToken->setText(currentToken->getText() + "_");
			}
			currentToken = nextToken;
		}
	}
}



//////////////////////////////
//
// Tool_worex::removeWordExtenders -- 
//

void Tool_worex::removeWordExtenders(vector<HTp> spineStartList) {
	for (HTp token : spineStartList) {
		if (!token->isDataType("**text")) {
			continue;
		}
		HTp currentToken = token;
		while (currentToken) {
			if (currentToken->isNonNullData()) {
				string wordEndExtender = "_";
				bool endsWithUnderscore = equal(wordEndExtender.rbegin(), wordEndExtender.rend(), currentToken->getText().rbegin()) == true;
				if (endsWithUnderscore) {
					string text = currentToken->getText();
					text.pop_back();
					currentToken->setText(text);
				}
			}
			currentToken = currentToken->getNextToken();
		}
	}
}

// END_MERGE

} // end namespace hum
