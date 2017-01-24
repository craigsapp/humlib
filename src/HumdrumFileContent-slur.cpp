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

	int elision;
	for (int row=0; row<(int)tracktokens.size(); row++) {
		for (int track=0; track<(int)tracktokens[row].size(); track++) {
			if (tracktokens[row][track]->hasSlurStart() &&
					tracktokens[row][track]->hasSlurEnd()) {

				// If note has slur start and stop on the same note,
				// then this means to end the previous slur and start
				// a new one.  This is a special case of an elided slur
				// where the elision is not explicitly marked.

				// slur ending code:
				elision = tracktokens[row][track]->getSlurEndElisionLevel();
				if (elision >= 0) {

					if (sluropens[elision][track].size() > 0) {
						sluropens[elision][track].back()->setValue("auto",
								"slurEnd", tracktokens[row][track]);
						sluropens[elision][track].back()->setValue("auto",
								"id", sluropens[elision][track].back());
						tracktokens[row][track]->setValue("auto", "slurStart",
								sluropens[elision][track].back());
						tracktokens[row][track]->setValue("auto", "id",
								tracktokens[row][track]);
						sluropens[elision][track].back()->setValue("auto", "slurDuration",
							tracktokens[row][track]->getDurationFromStart() -
							sluropens[elision][track].back()->getDurationFromStart());
						sluropens[elision][track].pop_back();

					} else {
						// no starting slur marker to match to this slur end.
						tracktokens[row][track]->setValue("auto", "hangingSlur", "true");
						tracktokens[row][track]->setValue("auto", "slurDration",
							tracktokens[row][track]->getDurationToEnd());
					}
				}

				// slur starting code:
				elision = tracktokens[row][track]->getSlurStartElisionLevel();
				if (elision >= 0) {
					sluropens[elision][track].push_back(tracktokens[row][track]);
				}

			} else {
				// not a single-note elided slur

				if (tracktokens[row][track]->hasSlurStart()) {
					elision = tracktokens[row][track]->getSlurStartElisionLevel();
					if (elision >= 0) {
						sluropens[elision][track].push_back(tracktokens[row][track]);
					}
				}

				if (tracktokens[row][track]->hasSlurEnd()) {

					// elision = tracktokens[row][track]->getSlurEndElisionLevel();
					elision = tracktokens[row][track]->getSlurEndElisionLevel();
					if (elision >= 0) {

						if (sluropens[elision][track].size() > 0) {

							sluropens[elision][track].back()->setValue("auto",
									"slurEnd", tracktokens[row][track]);
							sluropens[elision][track].back()->setValue("auto",
									"id", sluropens[elision][track].back());
							tracktokens[row][track]->setValue("auto", "slurStart",
									sluropens[elision][track].back());
							tracktokens[row][track]->setValue("auto", "id",
									tracktokens[row][track]);
							sluropens[elision][track].back()->setValue("auto", "slurDuration",
								tracktokens[row][track]->getDurationFromStart() -
								sluropens[elision][track].back()->getDurationFromStart());
							sluropens[elision][track].pop_back();

						} else {
							// No starting slur marker to match to this slur end in the
							// given track.

							// search for an open slur in another track:
							
							bool found = false;
							for (int itrack=0; itrack<(int)sluropens[elision].size(); itrack++) {
								if (sluropens[elision][itrack].size() > 0) {

cerr << "LNKING " << tracktokens[row][track]  << " to " << sluropens[elision][itrack].back() << endl;
									// link to this slur start in another layer instead
									sluropens[elision][itrack].back()->setValue("auto",
											"slurEnd", tracktokens[row][track]);
									sluropens[elision][itrack].back()->setValue("auto",
											"id", sluropens[elision][itrack].back());
									tracktokens[row][track]->setValue("auto", "slurStart",
											sluropens[elision][itrack].back());
									tracktokens[row][track]->setValue("auto", "id",
											tracktokens[row][track]);
									sluropens[elision][itrack].back()->setValue("auto", "slurDuration",
										tracktokens[row][track]->getDurationFromStart() -
										sluropens[elision][itrack].back()->getDurationFromStart());
									sluropens[elision][itrack].pop_back();

									found = true;
									break;
								}
							}
							if (!found) {
cerr << "HANGING SLUR" << endl;
								tracktokens[row][track]->setValue("auto", "hangingSlur", "true");
								tracktokens[row][track]->setValue("auto", "slurDration",
									tracktokens[row][track]->getDurationToEnd());
							}
						}
					}
					// if elision level is less than 0 something strange happened.

				}
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


// END_MERGE

} // end namespace hum



