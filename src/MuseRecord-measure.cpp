//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Tue Jun 30 22:41:24 PDT 1998
// Last Modified: Sun Apr 14 03:25:23 PDT 2024
// Filename:      humlib/src/MuseRecord-measure.cpp
// Web Address:   http://github.com/craigsapp/humlib/blob/master/src/MuseRecord-measure.cpp
// Syntax:        C++11
// vim:           ts=3
//
// Description:   Measure-related functions of the MuseRecord class.
//
//

#include "Convert.h"
#include "HumRegex.h"
#include "MuseData.h"
#include "MuseRecord.h"

#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sstream>

using namespace std;

namespace hum {

// START_MERGE


//////////////////////////////
//
// MuseRecord::getMeasureNumberField -- Columns 9-12 contain the measure number.
//

string MuseRecord::getMeasureNumberField(void) {
	if (!isBarline()) {
		return "";
	}
	return extract(9, 12);
}



//////////////////////////////
//
// MuseRecord::getMeasureNumber -- Remove spaces from field.
//

string MuseRecord::getMeasureNumber(void) {
	return trimSpaces(getMeasureNumberField());
}



//////////////////////////////
//
// MuseRecord::getMeasureType -- Columns 1-7.
//

string MuseRecord::getMeasureType(void) {
	if (!isBarline()) {
		return "";
	}
	return extract(1, 7);
}



//////////////////////////////
//
// MuseRecord::measureNumberQ -- Returns true if barline
//     has a measure number.
//

bool MuseRecord::measureNumberQ(void) {
	return (getMeasureNumber() != "");
}



//////////////////////////////
//
// MuseRecord::getMeasureFlags --  Columns 17 to 80.  This is
//    the styling of the barline.
//

string MuseRecord::getMeasureFlags(void) {
	if (m_recordString.size() < 17) {
		return "";
	} else {
		return trimSpaces(m_recordString.substr(16));
	}
}



//////////////////////////////
//
// MuseRecord::measureFermataQ -- returns true if there is a
//	fermata above or below the measure
//

int MuseRecord::measureFermataQ(void) {
	int output = 0;
	for (int i=17; i<=80 && i<= getLength(); i++) {
		if (getColumn(i) == 'F' || getColumn(i) == 'E') {
			output = 1;
			break;
		}
	}
	return output;
}



//////////////////////////////
//
// MuseRecord::measureFlagEqual -- Returns true if there are non-space
//     characters in columns 17 through 80.   A more smarter way of
//     doing this is checking the allocated length of the record, and
//     do not search non-allocated columns for non-space characters...
//

bool MuseRecord::measureFlagEqual(const string& key) {
	return (getMeasureFlags() == key);
}



//////////////////////////////
//
// MuseRecord::addMeasureFlag -- add the following characters to the
//    Flag region of the measure flag area (columns 17-80).  But only
//    add the flag if it is not already present in the region.  If it is
//    not present, then append it after the last non-space character
//    in that region.  A space will be added between the last item
//    and the newly added parameter.
//

void MuseRecord::addMeasureFlag(const string& strang) {
	string flags = getColumns(17, 80);
	string flag = strang;

	HumRegex hre;
	hre.replaceDestructive(flag, "\\*", "\\*", "g");
	hre.replaceDestructive(flag, "\\|", "\\|", "g");
	if (hre.search(flags, flag)) {
		// flag was already found in flags, so don't do anything
		return;
	}
	hre.replaceDestructive(flags, "", "\\s+$");
	flags += " ";
	flags += strang;
	setColumns(flags, 17, 80);
}


// END_MERGE

} // end namespace hum



