//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Wed Nov 30 22:27:57 PST 2016
// Last Modified: Wed Nov 30 22:28:03 PST 2016
// Filename:      HumdrumFileContent-timesig.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/HumdrumFileContent-timesig.cpp
// Syntax:        C++11; humlib
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:
//   Extracts time signature top and bottoms for a specific part.
//

#include "HumdrumFileContent.h"
#include "Convert.h"

#include <algorithm>
#include <cstring>

using namespace std;

namespace hum {

// START_MERGE

//////////////////////////////
//
// HumdrumFileStructure::getTimeSigs -- Return the prevailing time signature
//     top and bottom for a particular spine for each line in the HumdrumFile.
//     This version does not handle mulimeters such as 2+3/4 or 3/4+6/8.
//     Only checks the primary strand of a spine/track for time signatures.
//
//     default value: track = 0: 0 means use the time signature
//         of the first **kern spine in the file; otherwise, use the
//         time signatures found in the given track (indexed from 1
//         for the first spine on a line).  A value of <0, 0> is used for
//         unassigned time signature lines.
//

void HumdrumFileContent::getTimeSigs(vector<pair<int, HumNum> >& output,
		int track) {
	HumdrumFileStructure& infile = *this;
	int lineCount = infile.getLineCount();
	output.resize(lineCount);
	pair<int, HumNum> current(0, 0);
	fill(output.begin(), output.end(), current);
	if (track == 0) {
		vector<HTp> kernspines = infile.getKernLikeSpineStartList();
		if (kernspines.size() > 0) {
			track = kernspines[0]->getTrack();
		}
	}
	if (track == 0) {
		track = 1;
	}

	int top  = 0;   // top number of time signature (0 for no meter)
	int bot  = 0;   // bottom number of time signature
	int bot2 = 0;   // such as the 2 in 3%2.

	int firstsig  = -1;
	int firstdata = -1;

	HTp token = getTrackStart(track);
	while (token) {
		if (token->isData()) {
			if (firstdata < 0) {
				firstdata = token->getLineIndex();
			}
			token = token->getNextToken();
			continue;
		}
		if (!token->isInterpretation()) {
			token = token->getNextToken();
			continue;
		}
		// check for time signature:
		if (sscanf(token->c_str(), "*M%d/%d%%%d", &top, &bot, &bot2) == 3) {
			current.first = top;
			current.second.setValue(bot, bot2);
			if (firstsig < 0) {
				firstsig = token->getLineIndex();
			}
		} else if (sscanf(token->c_str(), "*M%d/%d", &top, &bot) == 2) {
			current.first = top;
			current.second = bot;
			if (firstsig < 0) {
				firstsig = token->getLineIndex();
			}
		}
		output[token->getLineIndex()] = current;
		token = token->getNextToken();
	}

	// Back-fill the list if the first time signature occurs before
	// the start of the data:
	if ((firstsig > 0) && (firstdata >= firstsig)) {
		current = output[firstsig];
		for (int i=0; i<firstsig; i++) {
			output[i] = current;
		}
	}

	// In-fill the list:
	int starti = firstsig;
	if (starti < 0) {
		starti = 0;
	}
	current = output[starti];
	for (int i=starti+1; i<(int)output.size(); i++) {
		if (output[i].first == 0) {
			output[i] = current;
		} else {
			current = output[i];
		}
	}
}


// END_MERGE

} // end namespace hum



