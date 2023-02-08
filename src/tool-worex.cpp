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
			HTp parallelNoteToken = getParallelNote(currentToken);
			HTp nextToken = currentToken->getNextToken();
			// get token of next syllable
			while (nextToken != NULL && !nextToken->isNonNullData()) {
				nextToken = nextToken->getNextToken();
			}
			if (nextToken == NULL) {
				// ignore current token if there is no next
				break;
			}
			if (parallelNoteToken != NULL) {
				// if next syllable if not direclty after the duration of the current parallel note token
				if (nextToken->getDurationFromStart() > (parallelNoteToken->getDurationFromStart() + parallelNoteToken->getDuration())) {
					// check if parallel note of current track is not followed by a rest
					HTp nextNonNullDataToken = parallelNoteToken->getNextNonNullDataToken();
					if (!nextNonNullDataToken->isRest()) {
						string syllableExtender = "-";
						bool isWordEnd = equal(syllableExtender.rbegin(), syllableExtender.rend(), currentToken->getText().rbegin()) == false;
						string wordEndExtender = "_";
						bool endsWithUnderscore = equal(wordEndExtender.rbegin(), wordEndExtender.rend(), currentToken->getText().rbegin()) == true;
						// if current syllable is the end end of a word
						// and the token does not alredy end with a underscore
						if(isWordEnd && !endsWithUnderscore) {
							currentToken->setText(currentToken->getText() + "_");
						}
					}
				}
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



//////////////////////////////
//
// Tool_worex::getParallelNote -- Go backwards on the line and count any note
//   attack (or tied note) on the first staff-like spine (track) found to the
//   left. If there is a spine split in the text and or **kern data, then this
//   algorithm needs to be refined further.
//

HTp Tool_worex::getParallelNote(hum::HTp token) {
	hum::HTp current = token;
	int track = -1;
	while (current) {
		current = current->getPreviousField();
		if (!current) {
			break;
		}
		if (current->isStaff()) {
			int ctrack = current->getTrack();
			if (track < 0) {
				track = ctrack;
			}
			if (track != ctrack) {
				return NULL;
			}
			if (current->isNull()) {
				continue;
			}
			if (current->isNote()) {
				return current;
			}
		}
	}
	return NULL;
}

// END_MERGE

} // end namespace hum
