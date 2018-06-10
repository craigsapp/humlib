//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Aug 17 02:39:28 PDT 2015
// Last Modified: Mon Oct  5 23:15:44 PDT 2015
// Filename:      HumdrumFileContent-slur.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/HumdrumFileContent-slur.cpp
// Syntax:        C++11
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Links slur starting/ending points to each other.
//

#include "HumdrumFileContent.h"

#include <algorithm>

using namespace std;

namespace hum {

// START_MERGE



//////////////////////////////
//
// HumdrumFileContent::analyzeKernSlurs -- Link start and ends of
//    slurs to each other.
//

bool HumdrumFileContent::analyzeKernSlurs(void) {
	vector<HTp> kernspines;
	getSpineStartList(kernspines, "**kern");
	bool output = true;
	for (int i=0; i<(int)kernspines.size(); i++) {
		output = output && analyzeKernSlurs(kernspines[i]);
	}
	return output;
}


bool HumdrumFileContent::analyzeKernSlurs(HTp spinestart) {
	// tracktokens == the 2-D data list for the track,
	// arranged in layers with the second dimension.
	vector<vector<HTp> > tracktokens;
	this->getTrackSeq(tracktokens, spinestart, OPT_DATA | OPT_NOEMPTY);
	// printSequence(tracktokens);

	// sluropens == list of slur openings for each track and elision level
	// first dimension: elision level
	// second dimension: track number
	vector<vector<vector<HTp> > > sluropens;

	sluropens.resize(4); // maximum of 4 elision levels
	for (int i=0; i<(int)sluropens.size(); i++) {
		sluropens[i].resize(8);  // maximum of 8 layers
	}

	int opencount = 0;
	int closecount = 0;
	int elision = 0;
	HTp token;
	for (int row=0; row<(int)tracktokens.size(); row++) {
		for (int track=0; track<(int)tracktokens[row].size(); track++) {
			token = tracktokens[row][track];
			if (!token->isData()) {
				continue;
			}
			if (token->isNull()) {
				continue;
			}
			opencount = (int)count(token->begin(), token->end(), '(');
			closecount = (int)count(token->begin(), token->end(), ')');

			for (int i=0; i<closecount; i++) {
				elision = token->getSlurEndElisionLevel(i);
				if (elision < 0) {
					continue;
				}
				if (sluropens[elision][track].size() > 0) {
					linkSlurEndpoints(sluropens[elision][track].back(), token);
					// remove slur opening from buffer
					sluropens[elision][track].pop_back();
				} else {
					// No starting slur marker to match to this slur end in the
					// given track.
					// search for an open slur in another track:
					bool found = false;
					for (int itrack=0; itrack<(int)sluropens[elision].size(); itrack++) {
						if (sluropens[elision][itrack].size() > 0) {
							linkSlurEndpoints(sluropens[elision][itrack].back(), token);
							// remove slur opening from buffer
							sluropens[elision][itrack].pop_back();
							found = true;
							break;
						}
					}
					if (!found) {
						token->setValue("auto", "hangingSlur", "true");
						token->setValue("auto", "slurDration",
							token->getDurationToEnd());
					}
				}
			}

			for (int i=0; i<opencount; i++) {
				elision = token->getSlurStartElisionLevel(i);
				if (elision < 0) {
					continue;
				}
				sluropens[elision][track].push_back(token);
			}
		}
	}

	// Mark un-closed slur starts:
	for (int i=0; i<(int)sluropens.size(); i++) {
		for (int j=0; j<(int)sluropens[i].size(); j++) {
			for (int k=0; k<(int)sluropens[i][j].size(); j++) {
				sluropens[i][j][k]->setValue("", "auto", "hangingSlur", "true");
				sluropens[i][j][k]->setValue("", "auto", "slurDuration",
					sluropens[i][j][k]->getDurationFromStart());
			}
		}
	}

	return true;
}



//////////////////////////////
//
// HumdrumFileContent::linkSlurEndpoints --  Allow up to two slur starts/ends
//      on a note.
//

void HumdrumFileContent::linkSlurEndpoints(HTp slurstart, HTp slurend) {
	string durtag = "slurDuration";
	string endtag = "slurEnd";
	int slurEndCount = slurstart->getValueInt("auto", "slurEndCount");
	slurEndCount++;
	if (slurEndCount > 1) {
		endtag += to_string(slurEndCount);
		durtag += to_string(slurEndCount);
	}
	string starttag = "slurStart";
	int slurStartCount = slurend->getValueInt("auto", "slurStartCount");
	slurStartCount++;
	if (slurStartCount > 1) {
		starttag += to_string(slurStartCount);
	}

	slurstart->setValue("auto", endtag, slurend);
	slurstart->setValue("auto", "id", slurstart);
	slurend->setValue("auto", starttag, slurstart);
	slurend->setValue("auto", "id", slurend);
	HumNum duration = slurend->getDurationFromStart()
			- slurstart->getDurationFromStart();
	slurstart->setValue("auto", durtag, duration);
	slurstart->setValue("auto", "slurEndCount", to_string(slurEndCount));
	slurend->setValue("auto", "slurStartCount", to_string(slurStartCount));
}


// END_MERGE

} // end namespace hum



