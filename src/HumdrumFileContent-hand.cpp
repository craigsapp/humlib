//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Aug 17 02:39:28 PDT 2015
// Last Modified: Mon Oct  5 23:15:44 PDT 2015
// Filename:      HumdrumFileContent-slur.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/HumdrumFileContent-hand.cpp
// Syntax:        C++11; humlib
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Add auto/hand = "LH" or "RH" parameters to notes if there are *LH and *RH
//                interpretations at the start of the file.
//

#include "HumdrumFileContent.h"

using namespace std;

namespace hum {

// START_MERGE


//////////////////////////////
//
// HumdrumFileContent::doHandAnalysis -- Returns true if any **kern spine has hand markup.
//    default value:
//         attacksOnlyQ = false;
//

bool HumdrumFileContent::doHandAnalysis(bool attacksOnlyQ) {
	HumdrumFileContent& infile = *this;
	vector<HTp> kstarts = infile.getKernSpineStartList();
	bool status = 0;
	for (int i=0; i<(int)kstarts.size(); i++) {
		status |= doHandAnalysis(kstarts[i], attacksOnlyQ);
	}
	return status;
}


bool HumdrumFileContent::doHandAnalysis(HTp startSpine, bool attacksOnlyQ) {
	if (!startSpine->isKern()) {
		return false;
	}
	bool output = false;
	vector<string> states(20);
	states[0] = "none";
	HTp current = startSpine->getNextToken();
	while (current) {
		int subtrack = current->getSubtrack();
		if (subtrack == 0) {
			for (int i=1; i<(int)states.size(); i++) {
				states[i] = states[0];
			}
		}

		if (current->isInterpretation()) {
			if (subtrack == 0) {
				if (*current == "*LH") {
					states[0] = "LH";
					output = true;
					for (int i=1; i<(int)states.size(); i++) {
						states[i] = states[0];
					}
				} else if (*current == "*RH") {
					states[0] = "RH";
					output = true;
					for (int i=1; i<(int)states.size(); i++) {
						states[i] = states[0];
					}
				}
			} else {
				int ttrack = current->getTrack();
				HTp c2 = current;
				while (c2) {
					int track = c2->getTrack();
					if (track != ttrack) {
						break;
					}
					int sub = c2->getSubtrack();
					if (*c2 == "*LH") {
						states.at(sub) = "LH";
						if (sub == 1) {
							states.at(0) = "LH";
						}
					} else if (*c2 == "*RH") {
						states.at(sub) = "RH";
						if (sub == 1) {
							states.at(0) = "RH";
						}
					}
					c2 = c2->getNextFieldToken();
				}
			}
		}

		if (!current->isData()) {
			current = current->getNextToken();
			continue;
		}

		if (subtrack == 0) {
			// no subspines
			if (attacksOnlyQ && current->isNoteAttack()) {
				current->setValue("auto", "hand", states[0]);
			} else {
				current->setValue("auto", "hand", states[0]);
			}
		} else {
			int ttrack = current->getTrack();
			HTp c2 = current;
			while (c2) {
				int track = c2->getTrack();
				if (track != ttrack) {
					break;
				}
				if (attacksOnlyQ && !c2->isNoteAttack()) {
					c2 = c2->getNextFieldToken();
					continue;
				}
				int sub = c2->getSubtrack();
				if (states.at(sub).empty()) {
					c2->setValue("auto", "hand", states.at(0));
				} else {
					c2->setValue("auto", "hand", states.at(sub));
				}
				c2 = c2->getNextFieldToken();
			}
		}
		current = current->getNextToken();
		continue;
	}
	if (output) {
		startSpine->setValue("auto", "hand", 1);
	}
	return output;
}


// END_MERGE

} // end namespace hum



