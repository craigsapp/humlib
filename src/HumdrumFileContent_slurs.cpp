//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Aug 17 02:39:28 PDT 2015
// Last Modified: Mon Aug 17 02:39:32 PDT 2015
// Filename:      HumdrumFileContent_slurs.cpp
// URL:           https://github.com/craigsapp/minHumdrum/blob/master/src/HumdrumFileContent_slurs.cpp
// Syntax:        C++11
// vim:           ts=3 noexpandtab
//
// Description:   Links slur starting/ending points to each other.
//

#include "HumdrumFileContent.h"

using namespace std;

namespace minHumdrum {

// START_MERGE



//////////////////////////////
//
// HumdrumFileContent::analyzeKernSlurs -- Link start and ends of
//    slurs to each other.
//

bool HumdrumFileContent::analyzeKernSlurs(void) {
	vector<HumdrumToken*> kernspines;
cout << "GOT HERE ZZZ" << endl;
	getSpineStartList(kernspines, "**kern");
cout << "GOT kern spines" << endl;
	bool output = true;
	for (int i=0; i<kernspines.size(); i++) {
cout << "GOT HERE AAA" << i << endl;
		output = output && analyzeKernSlurs(kernspines[i]);
	}
	return output;
}


bool HumdrumFileContent::analyzeKernSlurs(HumdrumToken* spinestart) {
	vector<HumdrumToken*> sluropen;
	vector<HumdrumToken*> slurclose;
	
	vector<vector<HumdrumToken*> > tracktokens;
	this->getTrackSeq(tracktokens, spinestart, OPT_NONULLS | OPT_NOGLOBAL);
	for (int i=0; i<tracktokens.size(); i++) {
		for (int j=0; j<tracktokens[i].size(); j++) {
			cout << tracktokens[i][j];
			if (i < tracktokens[i].size() -1) {
				cout << "\t";
			}
		}
		cout << "\n";
	}

	return true;
}



// END_MERGE

} // end namespace std;



