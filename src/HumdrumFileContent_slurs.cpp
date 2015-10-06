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
	vector<HTp> kernspines;
	getSpineStartList(kernspines, "**kern");
	bool output = true;
	for (int i=0; i<kernspines.size(); i++) {
		output = output && analyzeKernSlurs(kernspines[i]);
	}
	return output;
}


bool HumdrumFileContent::analyzeKernSlurs(HTp spinestart) {
	vector<HTp> sluropen;
	vector<HTp> slurclose;
	
	vector<vector<HTp> > tracktokens;
	this->getTrackSeq(tracktokens, spinestart, OPT_DATA | OPT_NOEMPTY);

	int i;
	for (i=0; i<tracktokens.size(); i++) {
		for (int j=0; j<tracktokens[i].size(); j++) {
			if (j < tracktokens[i].size() - 1) {
				if (tracktokens[i][j]->hasSlurStart()) {
					sluropen.push_back(tracktokens[i][j]);
				}
				if (tracktokens[i][j]->hasSlurEnd()) {
					slurclose.push_back(tracktokens[i][j]);
				}
			}
		}
	}

	for (i=0; i<sluropen.size(); i++) {
		cout << "open\t" << sluropen[i] << "\t" << sluropen[i]->getStrandIndex() << endl;
	}
	for (i=0; i<slurclose.size(); i++) {
		cout << "close\t" << slurclose[i] << "\t" << slurclose[i]->getStrandIndex() << endl;
	}

	return true;
}



// END_MERGE

} // end namespace std;



