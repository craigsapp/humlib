//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Aug 17 02:39:28 PDT 2015
// Last Modified: Mon Oct  5 23:15:44 PDT 2015
// Filename:      HumdrumFileContent-slur.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/HumdrumFileContent-midi.cpp
// Syntax:        C++11; humlib
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   HumdrumFileContent functions related to **kern data.
//

#include "HumdrumFileContent.h"

using namespace std;

namespace hum {

// START_MERGE



//////////////////////////////
//
// HumdrumFileContent::getTrackToKernIndex -- return a list indexed by file track
//     numbers (first entry not used), with non-zero values in the vector being
//     the **kern index with the given track number.
//

vector<int> HumdrumFileContent::getTrackToKernIndex(void) {
	HumdrumFileContent& infile = *this;
	vector<HTp> ktracks = infile.getKernSpineStartList();
	vector<int> trackToKernIndex(infile.getMaxTrack() + 1, -1);
	for (int i=0; i<(int)ktracks.size(); i++) {
		int track = ktracks[i]->getTrack();
		trackToKernIndex[track] = i;
	}
	return trackToKernIndex;
}


// END_MERGE

} // end namespace hum



