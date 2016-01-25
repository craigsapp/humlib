//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Aug  8 12:24:49 PDT 2015
// Last Modified: Sun Aug  9 21:03:12 PDT 2015
// Filename:      Convert-string.h
// URL:           https://github.com/craigsapp/humlib/blob/master/src/Convert-string.h
// Syntax:        C++11
// vim:           ts=3 noexpandtab
//
// Description:   Conversions related to strings.
//

#include <sstream>

#include "Convert.h"

namespace humlib {

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
	for (int i=0; i < kerndata.size(); i++) {
		ch = std::tolower(kerndata[i]);
		if ((ch >= 'a') && (ch <= 'g')) {
			return true;
		}
	}
	return false;
}



//////////////////////////////
//
// Convert::isKernSecondaryTiedNote -- Returns true if the input string
//   represents a **kern note (i.e., token with a pitch,
//   not a null token or a rest) and has a '_' or ']' character.
//

bool Convert::isKernSecondaryTiedNote(const string& kerndata) {
	char ch;
	if (!Convert::isKernNote(kerndata)) {
		return false;
	}
	for (int i=0; i < kerndata.size(); i++) {
		ch = std::tolower(kerndata[i]);
		if ((ch == '_') || (ch == ']')) {
			return true;
		}
	}
	return false;
}



//////////////////////////////
//
// Convert::isKernNoteAttack -- Returns true if the input string
//   represents a **kern note (not null or rest) and is not a 
//   secondary tied note.
//

bool Convert::isKernNoteAttack(const string& kerndata) {
	char ch;
	if (!Convert::isKernNote(kerndata)) {
		return false;
	}
	for (int i=0; i < kerndata.size(); i++) {
		ch = std::tolower(kerndata[i]);
		if ((ch == '_') || (ch == ']')) {
			return false;
		}
	}
	return true;
}



//////////////////////////////
//
// Convert::hasKernSlurStart -- Returns true if the input string
//   has a '('.
//

bool Convert::hasKernSlurStart(const string& kerndata) {
	for (int i=0; i < kerndata.size(); i++) {
		char ch = kerndata[i];
		if (ch == '(') {
			return true;
		}
	}
	return false;
}



//////////////////////////////
//
// Convert::hasKernSlurEnd -- Returns true if the input string
//   has a '('.
//

bool Convert::hasKernSlurEnd(const string& kerndata) {
	for (int i=0; i < kerndata.size(); i++) {
		char ch = kerndata[i];
		if (ch == ')') {
			return true;
		}
	}
	return false;
}



//////////////////////////////
//
// Convert::getKernSlurStartElisionLevel -- Returns the number of
//   '&' characters before the last '(' character in a kern token.
//   Returns -1 if no '(' character in string.
//

int Convert::getKernSlurStartElisionLevel(const string& kerndata) { 
	bool foundSlurStart = false;
	int output = 0;
	for (int i=kerndata.size()-1; i >=0; i--) {
		char ch = kerndata[i];
		if (ch == '(') {
			foundSlurStart = true;
			continue;
		}
		if (!foundSlurStart) {
			continue;
		}
		if (ch == '&') {
			output++;
		} else {
			return output;
		}
	}
	if (foundSlurStart) {
		return output;
	} else {
		return -1;
	}
}



//////////////////////////////
//
// Convert::getKernSlurEndElisionLevel -- Returns the number of 
//   '&' characters before the last ')' character in a kern token.
//   Returns -1 if no ')' character in string.
//

int Convert::getKernSlurEndElisionLevel(const string& kerndata) { 
	bool foundSlurEnd = false;
	int output = 0;
	for (int i=kerndata.size()-1; i >=0; i--) {
		char ch = kerndata[i];
		if (ch == ')') {
			foundSlurEnd = true;
			continue;
		}
		if (!foundSlurEnd) {
			continue;
		}
		if (ch == '&') {
			output++;
		} else {
			return output;
		}
	}
	if (foundSlurEnd) {
		return output;
	} else {
		return -1;
	}
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



