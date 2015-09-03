//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Aug  8 12:24:49 PDT 2015
// Last Modified: Sun Aug  9 21:03:12 PDT 2015
// Filename:      Convert-string.h
// URL:           https://github.com/craigsapp/minHumdrum/blob/master/src/Convert-string.h
// Syntax:        C++11
// vim:           ts=3 noexpandtab
//
// Description:   Conversions related to strings.
//

#include <sstream>

#include "Convert.h"

namespace minHumdrum {

// START_MERGE


//////////////////////////////
//
// Convert::isKernRest -- Returns true if the input string represents
//   a **kern rest.
//

bool Convert::isKernRest(const string& kerndata) {
	if (kerndata.find("r") != string::npos) {
		return true;
	} else {
		return false;
	}
}



//////////////////////////////
//
// Convert::isKernNote -- Returns true if the input string represents
//   a **kern note (i.e., token with a pitch, not a null token or a rest).
//

bool Convert::isKernNote(const string& kerndata) {
	char ch;
	for (int i = 0; i < kerndata.size(); i++) {
		ch = std::tolower(kerndata[i]);
		if ((ch >= 'a') && (ch <= 'g')) {
			return true;
		}
	}
	return false;
}



//////////////////////////////
//
// Convert::getKernPitchAttributes --
//    pc         = pitch class
//    numacc     = numeric accidental (-1=flat, 0=natural, 1=sharp)
//    explicit   = force showing of accidental
//    oct        = octave number (middle C = 4)
//    base40     = base-40 enumeration of pitch (valid if abs(numacc) <= 2)
//

string Convert::getKernPitchAttributes(const string& kerndata) {
	int accid = kernToAccidentalCount(kerndata);
	string output = "";

	output += " dpc=\"";
	output += kernToDiatonicUC(kerndata);
	output += "\"";

	output += " numacc=\"";
	output += to_string(accid);
	output += "\"";

	if (kerndata.find('n') != string::npos) {
		output += " explicit =\"true\"";
	}

	output += " oct=\"";
	output += to_string(kernToOctaveNumber(kerndata));
	output += "\"";

	if (abs(accid) <= 2) {
		output += " base40=\"";
		output += to_string(kernToBase40(kerndata));
		output += "\"";
	}

	return output;
}



// END_MERGE

} // end namespace std;



