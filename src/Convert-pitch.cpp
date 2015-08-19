//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Aug  8 12:24:49 PDT 2015
// Last Modified: Wed Aug 19 00:06:39 PDT 2015
// Filename:      Convert-pitch.h
// URL:           https://github.com/craigsapp/minHumdrum/blob/master/src/Convert-pitch.h
// Syntax:        C++11
// vim:           ts=3 noexpandtab
//
// Description:   Conversions related to pitch.
//

#include <math.h>

#include <vector>

#include "Convert.h"

namespace minHumdrum {

// START_MERGE


//////////////////////////////
//
// Convert::kernToScientificPitch -- Convert a **kern pitch to
//   ScientificPitch notation, which is the diatonic letter name,
//   followed by a possible accidental, then an optional separator
//   string, and finally the octave number.  A string representing a
//   chord can be given to this function, and the output will return
//   a list of the pitches in the chord, separated by a space.
//		default values:
//			flat      = "b"
//			sharp     = "#"
//			separator = ""
//

string Convert::kernToScientificPitch(const string& kerndata,
		string flat, string sharp, string separator) {
	vector<string> subtokens = Convert::splitString(kerndata);
	string output;
	char   diatonic;
	int    accidental;
	int    octave;

	for (int i=0; i<subtokens.size(); i++) {
		diatonic   = Convert::kernToDiatonicUC(subtokens[i]);
		accidental = Convert::kernToAccidentalCount(subtokens[i]);
		octave     = Convert::kernToOctaveNumber(subtokens[i]);
		if ((i > 0) && (i < subtokens.size()-1)) {
			output += " ";
		}
		output += diatonic;
		for (int j=0; j<abs(accidental); j++) {
			output += (accidental < 0 ? flat : sharp);
		}
		output += separator;
		output += to_string(octave);
	}

	return output;
}



//////////////////////////////
//
// Convert::kernToDiatonicPC -- Convert a kern token into a diatonic
//    note pitch-class where 0="C", 1="D", ..., 6="B".  -1000 is returned 
//    if the note is rest, and -2000 if there is no pitch information in the
//    input string. Only the first subtoken in the string is considered.
//

int Convert::kernToDiatonicPC(const string& kerndata) {
	for (int i=0; i<kerndata.size(); i++) {
		if (kerndata[i] == ' ') {
			break;
		}
		if (kerndata[i] == 'r') {
			return -1000;
		}
		switch (kerndata[i]) {
			case 'A': case 'a': return 5;
			case 'B': case 'b': return 6;
			case 'C': case 'c': return 0;
			case 'D': case 'd': return 1;
			case 'E': case 'e': return 2;
			case 'F': case 'f': return 3;
			case 'G': case 'g': return 4;
		}
	}
	return -2000;
}



//////////////////////////////
//
// Convert::kernToDiatonicUC -- Convert a kern token into a diatonic
//    note pitch-class.  "R" is returned if the note is rest, and 
//    "X" is returned if there is no pitch name in the string.
//    Only the first subtoken in the string is considered.
//

char Convert::kernToDiatonicUC(const string& kerndata) {
	for (int i=0; i<kerndata.size(); i++) {
		if (kerndata[i] == ' ') {
			break;
		}
		if (kerndata[i] == 'r') {
			return 'R';
		}
		if (('A' <= kerndata[i]) && (kerndata[i] <= 'G')) {
			return kerndata[i];
		}
		if (('a' <= kerndata[i]) && (kerndata[i] <= 'g')) {
			return toupper(kerndata[i]);
		}
	}
	return 'X';
}



//////////////////////////////
//
// Convert::kernToDiatonicLC -- Similar to kernToDiatonicUC, but
//    the returned pitch name is lower case.
//

char Convert::kernToDiatonicLC(const string& kerndata) {
	return tolower(Convert::kernToDiatonicUC(kerndata));
}



//////////////////////////////
//
// Convert::kernToAccidentalCount -- Convert a kern token into a count
//    of accidentals in the first subtoken.  Sharps are assigned to the
//    value +1 and flats to -1.  So a double sharp is +2 and a double
//    flat is -2.  Only the first subtoken in the string is considered.
//    Cases such as "#-" should not exist, but in this case the return
//    value will be 0.
//

int Convert::kernToAccidentalCount(const string& kerndata) {
	int output = 0;
	for (int i=0; i<kerndata.size(); i++) {
		if (kerndata[i] == ' ') {
			break;
		}
		if (kerndata[i] == '-') {
			output--;
		}
		if (kerndata[i] == '#') {
			output++;
		}
	}
	return output;
}



//////////////////////////////
//
// Convert::kernToOctaveNumber -- Convert a kern token into an octave number.
//    Middle C is the start of the 4th octave. -1000 is returned if there
//    is not pitch in the string.  Only the first subtoken in the string is
//    considered.
//

int Convert::kernToOctaveNumber(const string& kerndata) {
	int uc = 0;
	int lc = 0;
	if (kerndata == ".") {
		return -1000;
	}
	for (int i=0; i<kerndata.size(); i++) {
		if (kerndata[i] == ' ') {
			break;
		}
		if (kerndata[i] == 'r') {
			return -1000;
		}
		uc += ('A' <= kerndata[i]) && (kerndata[i] <= 'G') ? 1 : 0;
		lc += ('a' <= kerndata[i]) && (kerndata[i] <= 'g') ? 1 : 0;
	}
	if ((uc > 0) && (lc > 0)) {
		// invalid pitch description
		return -1000;
	}
	if (uc > 0) {
		return 4 - uc;
	} else if (lc > 0) {
		return 3 + lc;
	} else {
		return -1000;
	}
}



// END_MERGE

} // end namespace std;



