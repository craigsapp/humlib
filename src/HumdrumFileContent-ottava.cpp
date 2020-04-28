//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Thu Sep 20 08:45:02 PDT 2018
// Last Modified: Thu Sep 20 08:45:05 PDT 2018
// Filename:      HumdrumFileContent-octave.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/HumdrumFileContent-octave.cpp
// Syntax:        C++11; humlib
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Marks octave shifts necessary for printing notes
//                under ottava lines in music notation.
//                *8va   --> *X8va     == octave up
//                *15ma  --> *X15ma    == 2 octaves up
//                *8ba   --> *X8ba     == octave down
//                *15ba  --> *X15ba    == 2 octaves down
//

#include "HumdrumFileContent.h"

using namespace std;

namespace hum {

// START_MERGE


//////////////////////////////
//
// HumdrumFileContent::analyzeOttavas --
//

void HumdrumFileContent::analyzeOttavas(void) {
	int tcount = getTrackCount();
	vector<int> activeOttava(tcount+1, 0);
	vector<int> octavestate(tcount+1, 0);
	for (int i=0; i<getLineCount(); i++) {
		HLp line = getLine(i);
		if (line->isInterpretation()) {
			int fcount = getLine(i)->getFieldCount();
			for (int j=0; j<fcount; j++) {
				HTp token = line->token(j);
				if (!token->isKern()) {
					continue;
				}
				int track = token->getTrack();
				if (*token == "*8va") {
					octavestate[track] = +1;
					activeOttava[track]++;
				} else if (*token == "*X8va") {
					octavestate[track] = 0;
					activeOttava[track]--;
				} else if (*token == "*8ba") {
					octavestate[track] = -1;
					activeOttava[track]++;
				} else if (*token == "*X8ba") {
					octavestate[track] = 0;
					activeOttava[track]--;
				} else if (*token == "*15ma") {
					octavestate[track] = +2;
					activeOttava[track]++;
				} else if (*token == "*X15ma") {
					octavestate[track] = 0;
					activeOttava[track]--;
				} else if (*token == "*15ba") {
					octavestate[track] = -2;
					activeOttava[track]++;
				} else if (*token == "*X15ba") {
					octavestate[track] = 0;
					activeOttava[track]--;
				}
			}
		}
		else if (line->isData()) {
			int fcount = getLine(i)->getFieldCount();
			for (int j=0; j<fcount; j++) {
				HTp token = line->token(j);
				if (!token->isKern()) {
					continue;
				}
				int track = token->getTrack();
				if (!activeOttava[track]) {
					continue;
				}
				if (octavestate[track] == 0) {
					continue;
				}
				if (token->isNull()) {
					continue;
				}
				if (token->isRest()) {
					// do not exclude rests, since the vertical placement
					// of the staff may need to be updated by the ottava mark.
				}
				token->setValue("auto", "ottava", to_string(octavestate[track]));
			}
		}
	}
}


// END_MERGE

} // end namespace hum



