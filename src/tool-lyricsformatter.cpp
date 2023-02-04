//
// Programmer:    Wolfgang Drescher <drescher.wolfgang@gmail.com>
// Creation Date: Sa  4 Feb 2023 16:27:58 CET
// Filename:      tool-lyricsformatter.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/tool-lyricsformatter.cpp
// Syntax:        C++11; humlib
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Format the lyrics of **text spines
//

#include "tool-lyricsformatter.h"
#include "Convert.h"
#include "HumRegex.h"
#include <regex>
#include <cmath>

using namespace std;

namespace hum {

// START_MERGE

//////////////////////////////
//
// Tool_lyricsformatter::Tool_lyricsformatter -- Set the recognized options for the tool.
//

Tool_lyricsformatter::Tool_lyricsformatter(void) {
	define("ul|aul|add-ul|add-underline|underline=b", "Add melismatic underlines at ending syllables of words");
	define("xul|rul|remove-ul|remove-underline=b", "Remove melismatic underlines at ending syllables of words");
}



//////////////////////////////
//
// Tool_lyricsformatter::run -- Do the main work of the tool.
//

bool Tool_lyricsformatter::run(HumdrumFileSet &infiles) {
	bool status = true;
	for (int i = 0; i < infiles.getCount(); i++) {
		status &= run(infiles[i]);
	}
	return status;
}

bool Tool_lyricsformatter::run(const string &indata, ostream &out) {
	HumdrumFile infile(indata);
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}

bool Tool_lyricsformatter::run(HumdrumFile &infile, ostream &out) {
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}

bool Tool_lyricsformatter::run(HumdrumFile &infile) {
	initialize();
	processFile(infile);
	return true;
}



//////////////////////////////
//
// Tool_lyricsformatter::initialize -- 
//

void Tool_lyricsformatter::initialize(void) {
	m_addUnderlineQ = getBoolean("add-underline");
	m_removeUnderlineQ = getBoolean("remove-underline");
}



//////////////////////////////
//
// Tool_lyricsformatter::processFile -- 
//

void Tool_lyricsformatter::processFile(HumdrumFile& infile) {

	vector<HTp> textSpinesStartList;
	infile.getSpineStartList(textSpinesStartList, "**text");	

	if (m_addUnderlineQ) {
		addUnderlines(textSpinesStartList);
	} else if (m_removeUnderlineQ) {
		removeUnderlines(textSpinesStartList);
	}

	infile.createLinesFromTokens();

	m_humdrum_text << infile;
}



//////////////////////////////
//
// Tool_lyricsformatter::addUnderlines -- 
//

void Tool_lyricsformatter::addUnderlines(vector<HTp> spineStartList) {
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
// Tool_lyricsformatter::removeUnderlines -- 
//

void Tool_lyricsformatter::removeUnderlines(vector<HTp> spineStartList) {
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
