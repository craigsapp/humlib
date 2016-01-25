//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Oct  5 23:16:26 PDT 2015
// Last Modified: Mon Oct  5 23:16:29 PDT 2015
// Filename:      HumdrumFileContent-tie.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/HumdrumFileContent-tie.cpp
// Syntax:        C++11
// vim:           ts=3 noexpandtab
//
// Description:   Links tie starting/continuing/ending points to each other.
//

#include "HumdrumFileContent.h"

using namespace std;

namespace Humdrum {

// START_MERGE



//////////////////////////////
//
// HumdrumFileContent::analyzeKernTies -- Link start and ends of
//    ties to each other.
//

bool HumdrumFileContent::analyzeKernTies(void) {
	vector<HTp> kernspines;
	getSpineStartList(kernspines, "**kern");
	bool output = true;
	for (int i=0; i<kernspines.size(); i++) {
		output = output && analyzeKernTies(kernspines[i]);
	}
	return output;
}


bool HumdrumFileContent::analyzeKernTies(HTp spinestart) {
	vector<vector<HTp> > tracktokens;
	this->getTrackSeq(tracktokens, spinestart, OPT_DATA | OPT_NOEMPTY);


	return true;
}


// END_MERGE

} // end namespace Humdrum



