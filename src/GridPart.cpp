//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sun Oct 16 16:08:05 PDT 2016
// Last Modified: Sun Oct 16 16:08:08 PDT 2016
// Filename:      GridPart.cpp
// URL:           https://github.com/craigsapp/hum2ly/blob/master/src/GridPart.cpp
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   GridPart is an intermediate container for converting from
//                MusicXML syntax into Humdrum syntax.
//
//

#include "GridPart.h"

using namespace std;

namespace hum {

// START_MERGE

//////////////////////////////
//
// GridPart::GridPart -- Constructor.
//

GridPart::GridPart(void) : GridSide() {
	// do nothing;
}

//////////////////////////////
//
// GridPart::~GridPart -- Deconstructor: delete any GridStaff items
//     being stored.
//

GridPart::~GridPart(void) {
	for (int i=0; i<(int)this->size(); i++) {
		if (this->at(i)) {
			delete this->at(i);
			this->at(i) = NULL;
		}
	}
}


//////////////////////////////
//
// operator<< -- print the contents of a GridPart data structure --
//

ostream& operator<<(ostream& output, GridPart* part) {
	if (part == NULL) {
		output << "{n}";
		return output;
	}
	for (int s=0; s<(int)part->size(); s++) {
		GridStaff* staff = part->at(s);
		output << "(s" << s << ":)";
		if (staff == NULL) {
			output << "{n}";
			continue;
		}
		for (int t=0; t<(int)staff->size(); t++) {
			GridVoice* gt = staff->at(t);
			output << "(v" << t << ":)";
			if (gt == NULL) {
				output << "{n}";
				continue;
			} else {
				HTp token = gt->getToken();
				if (token == NULL) {
					output << "{n}";
				} else {
					output << " \"" << *token << "\" ";
				}
			}
		}
	}
	output << " ppp " << (GridSide*) part;
	return output;
}


ostream& operator<<(ostream& output, GridPart& part) {
	output << &part;
	return output;
}


// END_MERGE

} // end namespace hum



