//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Tue Jun 30 21:44:58 PDT 1998
// Last Modified: Mon Sep 23 22:49:43 PDT 2019 Convert to STL
// Filename:      humlib/src/MuseRecord.cpp
// URL:           http://github.com/craigsapp/humlib/blob/master/src/MuseRecord-header.cpp
// Syntax:        C++11
// vim:           ts=3
//
// Description:   Functions related to MuseData file headers.
//

#include "MuseRecord.h"
#include "HumRegex.h"

#include <string>

using namespace std;

namespace hum {

// START_MERGE


//////////////////////////////
//
// MuseRecord::getPartName -- return the name line in the 
//     MuseData part's header.  This is the 9th non-comment
//     line in a part file.
//

string MuseRecord::getPartName(void) {
	if (isPartName()) {
		HumRegex hre;
		string raw = this->getLine();
		hre.replaceDestructive(raw, "", "^\\s+");
		hre.replaceDestructive(raw, "", "\\s+$");
		return raw;
	} else {
		return "";
	}
}


// END_MERGE

} // end namespace hum



