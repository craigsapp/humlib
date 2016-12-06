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
	vector<vector<HTp> > tracktokens;
	this->getTrackSeq(tracktokens, spinestart, OPT_DATA | OPT_NOEMPTY);
	// printSequence(tracktokens);

	vector<vector<HTp> > sluropens;
	sluropens.resize(32);

	int elisionlevel;
	int i, j;
	for (i=0; i<(int)tracktokens.size(); i++) {
		for (j=0; j<(int)tracktokens[i].size(); j++) {
			if (tracktokens[i][j]->hasSlurStart() &&
					tracktokens[i][j]->hasSlurEnd()) {

				// If note has slur start and stop on the same note,
				// then this means to end the previous slur and start
				// a new one.  This is a special case of an elided slur
				// where the elision is not explicitly marked.

				// slur ending code:
				elisionlevel = tracktokens[i][j]->getSlurEndElisionLevel();
				if (elisionlevel >= 0) {
					if (sluropens[elisionlevel].size() > 0) {
						sluropens[elisionlevel].back()->setValue("auto",
								"slurEnd", tracktokens[i][j]);
						sluropens[elisionlevel].back()->setValue("auto",
								"id", sluropens[elisionlevel].back());
						tracktokens[i][j]->setValue("auto", "slurStart",
								sluropens[elisionlevel].back());
						tracktokens[i][j]->setValue("auto", "id",
								tracktokens[i][j]);
						sluropens[elisionlevel].back()->setValue("auto", "slurDuration",
							tracktokens[i][j]->getDurationFromStart() -
							sluropens[elisionlevel].back()->getDurationFromStart());
						sluropens[elisionlevel].pop_back();
					} else {
						// no starting slur marker to match to this slur end.
						tracktokens[i][j]->setValue("auto", "hangingSlur", "true");
						tracktokens[i][j]->setValue("auto", "slurDration",
							tracktokens[i][j]->getDurationToEnd());
					}
				}

				// slur starting code:
				elisionlevel = tracktokens[i][j]->getSlurStartElisionLevel();
				if (elisionlevel >= 0) {
					sluropens[elisionlevel].push_back(tracktokens[i][j]);
				}

			} else {

				if (tracktokens[i][j]->hasSlurStart()) {
					elisionlevel = tracktokens[i][j]->getSlurStartElisionLevel();
					if (elisionlevel >= 0) {
						sluropens[elisionlevel].push_back(tracktokens[i][j]);
					}
				}

				if (tracktokens[i][j]->hasSlurEnd()) {

					elisionlevel = tracktokens[i][j]->getSlurEndElisionLevel();
					if (elisionlevel >= 0) {
						if (sluropens[elisionlevel].size() > 0) {
							sluropens[elisionlevel].back()->setValue("auto",
									"slurEnd", tracktokens[i][j]);
							sluropens[elisionlevel].back()->setValue("auto",
									"id", sluropens[elisionlevel].back());
							tracktokens[i][j]->setValue("auto", "slurStart",
									sluropens[elisionlevel].back());
							tracktokens[i][j]->setValue("auto", "id",
									tracktokens[i][j]);
							sluropens[elisionlevel].back()->setValue("auto", "slurDuration",
								tracktokens[i][j]->getDurationFromStart() -
								sluropens[elisionlevel].back()->getDurationFromStart());
							sluropens[elisionlevel].pop_back();
						} else {
							// no starting slur marker to match to this slur end.
							tracktokens[i][j]->setValue("auto", "hangingSlur", "true");
							tracktokens[i][j]->setValue("auto", "slurDration",
								tracktokens[i][j]->getDurationToEnd());
						}
					}
				}
			}
		}
	}
	// Mark un-closed slur starts:
	for (i=0; i<(int)sluropens.size(); i++) {
		for (j=0; j<(int)sluropens[i].size(); j++) {
			sluropens[i][j]->setValue("", "auto", "hangingSlur", "true");
			sluropens[i][j]->setValue("", "auto", "slurDuration",
				sluropens[i][j]->getDurationFromStart());
		}
	}

	return true;
}


// END_MERGE

} // end namespace hum



