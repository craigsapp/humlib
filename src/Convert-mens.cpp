//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Fri May  4 20:29:09 PDT 2018
// Last Modified: Fri May  4 21:12:53 PDT 2018
// Filename:      Convert-mens.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/Convert-mens.cpp
// Syntax:        C++11
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Conversions related to strings.
//

#include <sstream>
#include <cctype>
#include <string>

#include "Convert.h"

using namespace std;

namespace hum {

// START_MERGE


//////////////////////////////
//
// Convert::isMensRest -- Returns true if the input string represents
//   a **mens rest.
//

bool Convert::isMensRest(const string& mensdata) {
	return mensdata.find('r') != std::string::npos;
}



//////////////////////////////
//
// Convert::isMensNote -- Returns true if the input string represents
//   a **mens note (i.e., token with a pitch, not a null token or a rest).
//

bool Convert::isMensNote(const string& mensdata) {
	char ch;
	for (int i=0; i < (int)mensdata.size(); i++) {
		ch = std::tolower(mensdata[i]);
		if ((ch >= 'a') && (ch <= 'g')) {
			return true;
		}
	}
	return false;
}



//////////////////////////////
//
// Convert::hasLigatureBegin -- Returns true if the input string
//   has a '<' character.
//

bool Convert::hasLigatureBegin(const string& mensdata) {
	return mensdata.find('<') != std::string::npos;
}



//////////////////////////////
//
// Convert::hasLigatureEnd -- Returns true if the input string
//   has a '>'.
//

bool Convert::hasLigatureEnd(const string& mensdata) {
	return mensdata.find('>') != std::string::npos;
}



//////////////////////////////
//
// Convert::getMensStemDir -- Returns the stem direction of **mens
//   notes.  These are the same as **kern pitches.
//      / = stem up, return +1
//      \ = stemp down, return -1
//      otherwise return 0 for no stem information
//

bool Convert::getMensStemDirection(const string& mensdata) {
	if (mensdata.find('/') != std::string::npos) {
		return +1;
	} else if (mensdata.find('\\') != std::string::npos) {
		return +1;
	} else {
		return 0;
	}
}



//////////////////////////////
//
// Convert::mensToDuration --
//   X = maxima (octuple whole note)
//   L = long  (quadruple whole note)
//   S = breve (double whole note)
//   s = semi-breve (whole note)
//   M = minim (half note)
//   m = semi-minim (quarter note)
//   U = fusa (eighth note)
//   u = semifusa (sixteenth note)
//
//   p = perfect (dotted)
//   i = imperfect (not-dotted)
//
// Still have to deal with coloration (triplets)
//
// Default value: scale = 4 (convert to quarter note units)
//                separator = " " (space between chord notes)
//

HumNum Convert::mensToDuration(const string& mensdata, HumNum scale,
		const string& separator) {
	HumNum output(0);
   bool perfect = false;
   // bool imperfect = true;
	for (int i=0; i<(int)mensdata.size(); i++) {
		if (mensdata[i] == 'p') {
			perfect = true;
			// imperfect = false;
		}
		if (mensdata[i] == 'i') {
			perfect = false;
			// imperfect = true;
		}

		// units are in whole notes, but scaling will probably
		// convert to quarter notes (default
		switch (mensdata[i]) {
			case 'X': output = 8; break;              // octuple whole note
			case 'L': output = 4; break;              // quadruple whole note
			case 'S': output = 2; break;              // double whole note
			case 's': output = 1; break;              // whole note
			case 'M': output.setValue(1, 2);  break;  // half note
			case 'm': output.setValue(1, 4);  break;  // quarter note
			case 'U': output.setValue(1, 8);  break;  // eighth note
			case 'u': output.setValue(1, 16); break;  // sixteenth note
		}

		if (mensdata.compare(i, separator.size(), separator) == 0) {
			// only get duration of first note in chord
			break;
		}
	}

	if (perfect) {
		output *= 3;
		output /= 2;
	}
	output *= scale;
	return output;
}



//////////////////////////////
//
// Convert::mensToDurationNoDots -- The imperfect duration of the **mens rhythm.
//

HumNum Convert::mensToDurationNoDots(const string& mensdata, HumNum scale,
		const string& separator) {
	HumNum output(0);

	for (int i=0; i<(int)mensdata.size(); i++) {
		switch (mensdata[i]) {
			case 'X': output = 8; break;              // octuple whole note
			case 'L': output = 4; break;              // quadruple whole note
			case 'S': output = 2; break;              // double whole note
			case 's': output = 1; break;              // whole note
			case 'M': output.setValue(1, 2);  break;  // half note
			case 'm': output.setValue(1, 4);  break;  // quarter note
			case 'U': output.setValue(1, 8);  break;  // eighth note
			case 'u': output.setValue(1, 16); break;  // sixteenth note
		}
		if (mensdata.compare(i, separator.size(), separator) == 0) {
			// only get duration of first note in chord
			break;
		}
	}
	output *= scale;

	return output;
}



//////////////////////////////
//
// Convert::mensToRecip --
//

string Convert::mensToRecip(const string& mensdata, HumNum scale,
		const string& separator) {
	HumNum duration = Convert::mensToDuration(mensdata, scale, separator);
	return Convert::durationToRecip(duration);
}



// END_MERGE

} // end namespace hum



