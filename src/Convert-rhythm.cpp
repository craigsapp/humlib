//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Aug  8 12:24:49 PDT 2015
// Last Modified: Sun Aug  9 21:03:12 PDT 2015
// Filename:      Convert-rhythm.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/Convert-rhythm.cpp
// Syntax:        C++11
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Conversions related to rhythm.
//

#include "Convert.h"
#include <algorithm>
#include <cmath>

namespace hum {

// START_MERGE


//////////////////////////////
//
// Convert::recipToDuration -- Convert **recip rhythmic values into
//     rational number durations in terms of quarter notes.  For example "4"
//     will be converted to 1, "4." to 3/2 (1+1/2).  The second parameter
//     is a scaling factor which can change the rhythmic value's base duration.
//     Giving a scale of 1 will return the duration in whole note units, so
//     "4" will return a value of 1/4 (one quarter of a whole note).  Using
//     3/2 will give the duration in terms of dotted-quarter note units.
//     The third parameter is the sub-token separate.  For example if the input
//     string contains a space, anything after the first space will be ignored
//     when extracting the string.  **kern data which also includes the pitch
//     along with the rhythm can also be given and will be ignored.
// default value: scale = 4 (duration in terms of quarter notes)
// default value: separator = " " (sub-token separator)
//

HumNum Convert::recipToDuration(string* recip, HumNum scale,
		const string& separator) {
	return Convert::recipToDuration(*recip, scale, separator);
}


HumNum Convert::recipToDuration(const string& recip, HumNum scale,
		const string& separator) {
	size_t loc;
	loc = recip.find(separator);
	string subtok;
	if (loc != string::npos) {
		subtok = recip.substr(0, loc);
	} else {
		subtok = recip;
	}

	loc = recip.find('q');
	if (loc != string::npos) {
		// grace note, ignore printed rhythm
		HumNum zero(0);
		return zero;
	}

	int dotcount = 0;
	int i;
	int numi = -1;
	for (i=0; i<(int)subtok.size(); i++) {
		if (subtok[i] == '.') {
			dotcount++;
		}
		if ((numi < 0) && isdigit(subtok[i])) {
			numi = i;
		}
	}
	loc = subtok.find("%");
	int numerator = 1;
	int denominator = 1;
	HumNum output;
	if (loc != string::npos) {
		// reciprocal rhythm
		numerator = 1;
		denominator = subtok[numi++] - '0';
		while ((numi<(int)subtok.size()) && isdigit(subtok[numi])) {
			denominator = denominator * 10 + (subtok[numi++] - '0');
		}
		if ((loc + 1 < subtok.size()) && isdigit(subtok[loc+1])) {
			int xi = (int)loc + 1;
			numerator = subtok[xi++] - '0';
			while ((xi<(int)subtok.size()) && isdigit(subtok[xi])) {
				numerator = numerator * 10 + (subtok[xi++] - '0');
			}
		}
		output.setValue(numerator, denominator);
	} else if (numi < 0) {
		// no rhythm found
		HumNum zero(0);
		return zero;
	} else if (subtok[numi] == '0') {
		// 0-symbol
		int zerocount = 1;
		for (i=numi+1; i<(int)subtok.size(); i++) {
			if (subtok[i] == '0') {
				zerocount++;
			} else {
				break;
			}
		}
		numerator = (int)pow(2, zerocount);
		output.setValue(numerator, 1);
	} else {
		// plain rhythm
		denominator = subtok[numi++] - '0';
		while ((numi<(int)subtok.size()) && isdigit(subtok[numi])) {
			denominator = denominator * 10 + (subtok[numi++] - '0');
		}
		output.setValue(1, denominator);
	}

	if (dotcount <= 0) {
		return output * scale;
	}

	int bot = (int)pow(2.0, dotcount);
	int top = (int)pow(2.0, dotcount + 1) - 1;
	HumNum factor(top, bot);
	return output * factor * scale;
}



//////////////////////////////
//
// Convert::recipToDurationNoDots -- Same as recipToDuration(), but ignore
//   any augmentation dots.
//

HumNum Convert::recipToDurationNoDots(string* recip, HumNum scale,
		const string& separator) {
	return Convert::recipToDurationNoDots(*recip, scale, separator);
}


HumNum Convert::recipToDurationNoDots(const string& recip, HumNum scale,
		const string& separator) {
	string temp = recip;
	std::replace(temp.begin(), temp.end(), '.', 'Z');
	return Convert::recipToDuration(temp, scale, separator);
}


//////////////////////////////
//
// Convert::durationToRecip -- Duration input is in units of quarter notes,
//     since the default value for scale is 1/4.
//

string Convert::durationToRecip(HumNum duration, HumNum scale) {
	duration *= scale;
	if (duration.getNumerator() == 1) {
		// simple rhythm (integer divisions of the whole note)
		return to_string(duration.getDenominator());
	}
	if (duration.getNumerator() == 0) {
		// grace note
		return "q";
	}

	// now decide if the rhythm can be represented simply with one dot.
	HumNum test1dot = (duration * 2) / 3;
	if (test1dot.getNumerator() == 1) {
		// single dot works
		string output = to_string(test1dot.getDenominator() * 2);
		output += ".";
		return output;
	}

	// now decide if the rhythm can be represented simply with two dots.
	HumNum test2dot = (duration * 4) / 7;
	if (test2dot.getNumerator() == 1) {
		// double dot works
		string output = to_string(test2dot.getDenominator() * 4);
		output += "..";
		return output;
	}

	// now decide if the rhythm can be represented simply with two dots.
	HumNum test3dot = (duration * 8) / 15;
	if (test3dot.getNumerator() == 1) {
		// single dot works
		string output = to_string(test3dot.getDenominator() * 8);
		output += "...";
		return output;
	}

	// duration required more than three dots or is not simple,
	// so assume that it is not simple:
	string output = to_string(duration.getDenominator());
	output += "%";
	output = to_string(duration.getNumerator());
	return output;
}



// END_MERGE

} // end namespace hum



