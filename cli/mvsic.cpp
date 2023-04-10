//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sun Apr  9 22:43:34 PDT 2023
// Last Modified: Sun Apr  9 22:43:37 PDT 2023
// Filename:      mvsic.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/cli/mvsic.cpp
// Syntax:        C++11
// vim:           ts=3 noexpandtab nowrap
//
// Description:   Move !SIC:l to the proper **text columns.
//                Related to issue https://github.com/craigsapp/website-polish-scores/issues/170
//

#include "humlib.h"
#include <iostream>

using namespace hum;
using namespace std;

void processFile         (HumdrumFile& infile, Options& options);
void processLocalComment (HumdrumFile& infile, int line);
void moveSic             (HumdrumFile& infile, int line, int field, int verse);


///////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv) {
	Options options;
	options.process(argc, argv);
	HumdrumFileStream instream(options);
	HumdrumFile infile;
	while (instream.read(infile)) {
		processFile(infile, options);
		infile.generateLinesFromTokens();
		cout << infile;
	}
	return 0;
}



//////////////////////////////
//
// processFile --
//

void processFile(HumdrumFile& infile, Options& options) {
	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isCommentLocal()) {
			continue;
		}
		processLocalComment(infile, i);
	}
}



//////////////////////////////
//
// processLocalComment --
//

void processLocalComment(HumdrumFile& infile, int line) {
	if (!infile[line].isCommentLocal()) {
		return;
	}
	HumRegex hre;
	for (int j=0; j<infile[line].getFieldCount(); j++) {
		HTp token = infile[line].token(j);
		if (!token->isKern()) {
			continue;
		}
		if (!hre.search(token, "!LO:SIC(?=:).*:l(:|$)")) {
			continue;
		}
		int verse = 1;
		if (hre.search(token, ":verse (\\d+)")) {
			verse = hre.getMatchInt(1);
		}
		moveSic(infile, line, j, verse);
	}
}



//////////////////////////////
//
// moveSic --
//

void moveSic(HumdrumFile& infile, int line, int field, int verse) {
	HTp kerntok = infile.token(line, field);
	cerr << "ADJUSTING: >>" << kerntok << "<< ON LINE " << (line+1) << " FIELD " << (field+1) << " MOVE TO VERSE " << verse << endl;
	int ktrack = kerntok->getTrack();
	HTp texttok = NULL;
	int verseNum = 0;
	for (int j=field+1; j<infile[line].getFieldCount(); j++) {
		HTp token = infile.token(line, j);
		if (token->isKern()) {
			int track = token->getTrack();
			if (track == ktrack) {
				continue;
			} else {
				cerr << "\tCOULD NOT FIND TARGET VERSE" << endl;
				return;
			}
		}
		if (!token->isDataType("**text")) {
			continue;
		}
		verseNum++;
		if (verseNum != verse) {
			continue;
		}
		texttok = token;
		if (*texttok != "!") {
			cerr << "\tTARGET TEXT TOKEN IS NOT EMPTY: " << texttok << endl;
			return;
		}
		break;
	}

	if (!texttok) {
		cerr << "\tCOULD NOT FIND VERSE TO MOVE SIC TO" << endl;
		return;
	}

	HumRegex hre;
	string text = *kerntok;
	hre.replaceDestructive(text, "", ":l(?=:|$)", "g");
	hre.replaceDestructive(text, "", ":verse \\d+[^:]*:?", "g");

	// fix parameter error:
	hre.replaceDestructive(text, ":s=", ":s:t=", "g");

	cerr << "\tUPDATING TEXT SIC TO: " << text << endl;
	kerntok->setText("!");
	texttok->setText(text);
}



