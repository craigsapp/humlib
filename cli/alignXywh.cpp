//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Apr 29 16:34:41 PDT 2023
// Last Modified: Sat Apr 29 16:34:44 PDT 2023
// Filename:      cli/alignXywh.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/cli/alignXywh.cpp
// Syntax:        C++11
// vim:           ts=3 noexpandtab nowrap
//
// Description:   Align xywh interpretations (for IIIF bounding boxes)
//                on single lines if they are in adjacent tandem
//                interpretations with nothing else in other spines.
//

#include "humlib.h"
#include <iostream>

using namespace hum;
using namespace std;

void   processFile  (HumdrumFile& infile, Options& options);
void   mergeLines   (HumdrumFile& infile, int index, int count);

bool listQ = false;

///////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv) {
	Options options;
	options.define("l|list=b", "Only list files that will be processed.");
	options.process(argc, argv);

	listQ = options.getBoolean("list");

	HumdrumFileStream instream(options);
	HumdrumFile infile;
	while (instream.read(infile)) {
		processFile(infile, options);
	}
	return 0;
}



//////////////////////////////
//
// processFile --
//

void processFile(HumdrumFile& infile, Options& options) {
	vector<bool> xywh(infile.getLineCount(), false);
	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isInterpretation()) {
			continue;
		}
		HumRegex hre;
		bool hasXywh = false;
		bool hasOther = false;
		for (int j=0; j<infile[i].getFieldCount(); j++) {
			HTp token = infile.token(i, j);
			if (*token == "*") {
				continue;
			}
			if (hre.search(token, R"(^\*xywh-)")) {
				hasXywh = true;
			} else {
				hasOther = true;
				break;
			}
		}
		if (hasOther) {
			continue;
		}
		if (hasXywh) {
			xywh[i] = true;
		}
	}

	bool hasMerger = false;
	vector<int> mergers(infile.getLineCount(), 0);
	for (int i=0; i<infile.getLineCount(); i++) {
		if (!xywh[i]) {
			continue;
		}
		if (mergers[i] < 0) {
			continue;
		}
		mergers[i] = 1;
		for (int j=i+1; j<infile.getLineCount(); j++) {
			if (!xywh[j]) {
				break;
			}
			mergers[i]++;
			hasMerger = true;
			mergers[j] = -1;
		}
	}

	if (listQ && hasMerger) {
		cout << "MERGER " << infile.getFilename() << endl;
		return;
	}

	for (int i=0; i<(int)mergers.size(); i++) {
		if (mergers[i] > 1) {
			mergeLines(infile, i, mergers[i]);
		}
	}
	infile.generateLinesFromTokens();

	for (int i=0; i<infile.getLineCount(); i++) {
		if (mergers[i] >= 0) {
			cout << infile[i] << endl;
		}
	}

}



//////////////////////////////
//
// mergeLines --
//

void mergeLines(HumdrumFile& infile, int index, int count) {
	for (int i=1; i<count; i++) {
		for (int j=0; j<infile[index+i].getFieldCount(); j++) {
			HTp stok = infile.token(index+i, j);
			if (*stok == "*") {
				continue;
			}
			HTp ttok = infile.token(index, j);
			ttok->setText(stok->getText());
		}
	}
}



