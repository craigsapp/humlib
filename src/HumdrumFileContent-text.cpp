//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Dec 16 12:20:21 PST 2019
// Last Modified: Mon Dec 16 12:20:24 PST 2019
// Filename:      HumdrumFileContent-text.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/HumdrumFileContent-text.cpp
// Syntax:        C++11; humlib
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Content analysis related to **text/**silbe spines.
//

#include "HumdrumFileContent.h"

using namespace std;

namespace hum {

// START_MERGE


//////////////////////////////
//
// HumdrumFileContent::analyzeTextRepetition -- Look for *ij and *Xij markers
//     that indicate repetition marks.  values added to text:
//      	auto/ij=true: the syllable is in an ij region.
//      	auto/ij-begin=true: the syllable is the first in an ij region.
//      	auto/ij-end=true: the syllable is the last in an ij region.
//
// Returns true if there are any *ij/*Xij markers in the data.
//
// Also consider *edit/*Xedit and *italic/*Xitalic as *ij/*Xij for printing.
//

bool HumdrumFileContent::analyzeTextRepetition(void) {
	HumdrumFileContent& infile = *this;
	vector<HTp> sstarts;
	infile.getSpineStartList(sstarts);

	bool output = false;
	bool ijstate;
	bool startij;  // true if at the first note in IJ
	HTp lastword;   // non-null if last syllable before *Xij

	for (int i=0; i<(int)sstarts.size(); i++) {
		ijstate = false;
		startij = false;
		lastword = NULL;
		HTp start = sstarts[i];
		if (!(start->isDataType("**text") || start->isDataType("**sylb"))) {
			continue;
		}
		HTp current = start;
		while (current) {
			if (current->isNull()) {
				current = current->getNextToken();
				continue;
			}
			if (current->isInterpretation()) {
				if ((*current == "*ij") || (*current == "*edit") || (*current == "*italic")) {
					output = true;
					startij = true;
					ijstate = true;
				} else if ((*current == "*Xij") || (*current == "*Xedit") || (*current == "*Xitalic")) {
					output = true;
					startij = false;
					ijstate = false;
					if (lastword) {
						lastword->setValue("auto", "ij-end", "true");
						lastword = NULL;
					}
				}
				current = current->getNextToken();
				continue;
			}
			if (current->isData()) {
				if (ijstate == true) {
					current->setValue("auto", "ij", "true");
					if (startij) {
						current->setValue("auto", "ij-begin", "true");
						startij = false;
					}
					lastword = current;
				}
			}
			current = current->getNextToken();
			continue;
		}
	}

	return output;
}


// END_MERGE

} // end namespace hum



