//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Aug 17 02:39:28 PDT 2015
// Last Modified: Mon Aug 17 02:39:32 PDT 2015
// Filename:      HumdrumFileContent.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/HumdrumFileContent.cpp
// Syntax:        C++11
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Used to add content analysis to HumdrumFileStructure class.
//

#include "HumdrumFileContent.h"
#include "HumRegex.h"
#include "Convert.h"

using namespace std;

namespace hum {

// START_MERGE

//////////////////////////////
//
// HumdrumFileContent::HumdrumFileContent --
//

HumdrumFileContent::HumdrumFileContent(void) {
	// do nothing
}


HumdrumFileContent::HumdrumFileContent(const string& filename) :
		HumdrumFileStructure() {
	read(filename);
}


HumdrumFileContent::HumdrumFileContent(istream& contents) :
		HumdrumFileStructure() {
	read(contents);
}



//////////////////////////////
//
// HumdrumFileContent::~HumdrumFileContent --
//

HumdrumFileContent::~HumdrumFileContent() {
	// do nothing
}



//////////////////////////////
//
// HumdrumFileContent::analyzeRScale --
//

bool HumdrumFileContent::analyzeRScale(void) {
	int active = 0; // number of tracks currently having an active rscale parameter
	HumdrumFileBase& infile = *this;
	vector<HumNum> rscales(infile.getMaxTrack() + 1, 1);
	HumRegex hre;
	for (int i=0; i<infile.getLineCount(); i++) {
		if (infile[i].isInterpretation()) {
			int fieldcount = infile[i].getFieldCount();
			for (int j=0; j<fieldcount; j++) {
				HTp token = infile[i].token(j);
				if (token->compare(0, 8, "*rscale:") != 0) {
					continue;
				}
				if (!token->isKern()) {
					continue;
				}
				int track = token->getTrack();
				HumNum value = 1;
				if (hre.search(*token, "\\*rscale:(\\d+)/(\\d+)")) {
					int top = hre.getMatchInt(1);
					int bot = hre.getMatchInt(2);
					value.setValue(top, bot);
				} else if (hre.search(*token, "\\*rscale:(\\d+)")) {
					int top = hre.getMatchInt(1);
					value.setValue(top, 1);
				}
				if (value == 1) {
					if (rscales[track] != 1) {
						rscales[track] = 1;
						active--;
					}
				} else {
					if (rscales[track] == 1) {
						active++;
					}
					rscales[track] = value;
				}
			}
			continue;
		}
		if (!active) {
			continue;
		}
		if (!infile[i].isData()) {
			continue;
		}
		int fieldcount = infile[i].getFieldCount();
		for (int j=0; j<fieldcount; j++) {
			HTp token = infile.token(i, j);
			int track = token->getTrack();
			if (rscales[track] == 1) {
				continue;
			}
			if (!token->isKern()) {
				continue;
			}
			if (token->isNull()) {
				continue;
			}

			int dots = token->getDots();
			HumNum dur = token->getDurationNoDots();
			dur *= rscales[track];
			string vis = Convert::durationToRecip(dur);
			for (int k=0; k<dots; k++) {
				vis += '.';
			}
			token->setValue("LO", "N", "vis", vis);
		}
	}

	return true;
}



// END_MERGE

} // end namespace hum



