//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Jun 16 06:37:31 PDT 2018
// Last Modified: Sat Jun 16 06:37:34 PDT 2018
// Filename:      HumdrumFileContent-stem.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/HumdrumFileContent-stem.cpp
// Syntax:        C++11; humlib
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Calculate stem lengths, particular for stems off of staff.
//

#include "HumdrumFileContent.h"
#include "Convert.h"

#include <vector>

using namespace std;

namespace hum {

// START_MERGE



//////////////////////////////
//
// HumdrumFileContent::analyzeKernStemLengths --
//

bool HumdrumFileContent::analyzeKernStemLengths(void) {
	int scount = this->getStrandCount();
	bool output = true;

	vector<vector<int>> centerlines;
	getBaselines(centerlines);
	for (int i=0; i<scount; i++) {
		HTp sstart = this->getStrandStart(i);
		if (!sstart->isKern()) {
			continue;
		}
		HTp send = this->getStrandEnd(i);
		output = output && analyzeKernStemLengths(sstart, send, centerlines);
	}
	return output;
}


bool HumdrumFileContent::analyzeKernStemLengths(HTp stok, HTp etok, vector<vector<int>>& centerlines) {
	HTp tok = stok;
	while (tok && (tok != etok)) {
		if (!tok->isData()) {
			tok = tok->getNextToken();
			continue;
		}
		if (tok->isNull()) {
			tok = tok->getNextToken();
			continue;
		}
		if (tok->isChord()) {
			// don't deal with chords yet
			tok = tok->getNextToken();
			continue;
		}
		if (!tok->isNote()) {
			tok = tok->getNextToken();
			continue;
		}
		int subtrack = tok->getSubtrack();
		if (subtrack == 0) {
			// single voice on staff, so don't process unless it has a stem direction
			// deal with explicit stem direction later.
			tok = tok->getNextToken();
			continue;
		}
		if (subtrack > 2) {
			// 3rd and higher voices will not be processed without stem direction
			// deal with explicit stem direction later.
			tok = tok->getNextToken();
			continue;
		}
		HumNum dur = Convert::recipToDurationNoDots(tok, 8);
		// dur is in units of eighth notes
		if (dur <= 1) {
			// eighth-note or less (could be in beam, so deal with it later)
			tok = tok->getNextToken();
			continue;
		}
		if (dur > 4) {
			// greater than a half-note (no stem)
			tok = tok->getNextToken();
			continue;
		}
		int track = tok->getTrack();
		int b7 = Convert::kernToBase7(tok);
		int diff = b7 - centerlines[track][tok->getLineIndex()];
		if (subtrack == 1) {
			if (diff == 1) { // 0.5 stem length adjustment
				tok->setValue("auto", "stemlen", "6.5");
			} else if (diff == 2) { // 1.0 stem length adjustment
				tok->setValue("auto", "stemlen", "6");
			} else if (diff >= 3) { // 1.5 stem length adjustment
				tok->setValue("auto", "stemlen", "5.5");
			}
		} else if (subtrack == 2) {
			if (diff == -1) { // 0.5 stem length adjustment
				tok->setValue("auto", "stemlen", "6.5");
			} else if (diff == -2) { // 1.0 stem length adjustment
				tok->setValue("auto", "stemlen", "6");
			} else if (diff <= -3) { // 1.5 stem length adjustment
				tok->setValue("auto", "stemlen", "5.5");
			}

		}
		tok = tok->getNextToken();
	}

	return true;
}


//////////////////////////////
//
// HumdrumFileContent::getCenterlines --
//

void HumdrumFileContent::getBaselines(vector<vector<int>>& centerlines) {
	centerlines.resize(this->getTrackCount()+1);

	vector<HTp> kernspines;
	getSpineStartList(kernspines, "**kern");
	int treble = Convert::kernClefToBaseline("*clefG2") + 4;
	int track;

	for (int i=0; i<(int)kernspines.size(); i++) {
		track = kernspines[i]->getTrack();
		centerlines[track].resize(getLineCount());
		for (int j=0; j<getLineCount(); j++) {
			centerlines[track][j] = treble;
		}
	}

	for (int i=0; i<(int)kernspines.size(); i++) {
		HTp tok = kernspines[i];
		int clefcenter = treble;
		while (tok) {
			track = tok->getTrack();
			centerlines[track][tok->getLineIndex()] = clefcenter;
			if (!tok->isClef()) {
				tok = tok->getNextToken();
				continue;
			}
			int centerline = Convert::kernClefToBaseline(tok) + 4;
			centerlines[track][tok->getLineIndex()] = centerline;
			clefcenter = centerline;
			tok = tok->getNextToken();
		}
	}
}


// END_MERGE

} // end namespace hum



