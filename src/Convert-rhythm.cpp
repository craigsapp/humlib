//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Aug  8 12:24:49 PDT 2015
// Last Modified: Sun Aug  9 21:03:12 PDT 2015
// Filename:      Convert-rhythm.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/Convert-rhythm.cpp
// Syntax:        C++11; humlib
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Conversions related to rhythm.
//

#include "Convert.h"
#include "HumRegex.h"

#include <algorithm>
#include <cmath>

using namespace std;

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
// Convert::recipToDurationIgnoreGrace -- Similar to recipToDuration(), but 
//     do not set grace notes to a zero duration, but rather give their
//     visual duration.
// default value: scale = 4 (duration in terms of quarter notes)
// default value: separator = " " (sub-token separator)
//

HumNum Convert::recipToDurationIgnoreGrace(string* recip, HumNum scale,
		const string& separator) {
	return Convert::recipToDurationIgnoreGrace(*recip, scale, separator);
}


HumNum Convert::recipToDurationIgnoreGrace(const string& recip, HumNum scale,
		const string& separator) {
	size_t loc;
	loc = recip.find(separator);
	string subtok;
	if (loc != string::npos) {
		subtok = recip.substr(0, loc);
	} else {
		subtok = recip;
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
	if (duration.getDenominator() == 1) {
		if (duration.getNumerator() == 2) {
			return "0";  // breve
		} else if (duration.getNumerator() == 3) {
			return "0."; // dotted breve
		} else if (duration.getNumerator() == 4) {
			return "00";  // long
		} else if (duration.getNumerator() == 6) {
			return "00."; // dotted long
		} else if (duration.getNumerator() == 8) {
			return "000";  // maxima
		} else if (duration.getNumerator() == 12) {
			return "000."; // dotted maxima
		}
	}
	if (duration.getNumerator() == 0) {
		// grace note
		return "q";
	}

	// now decide if the rhythm can be represented simply with one dot.
	HumNum test1dot = (duration * 2) / 3;
	if (test1dot.getNumerator() == 1) {
		// single dot works
		// string output = to_string(test1dot.getDenominator() * 2);
		string output = to_string(test1dot.getDenominator());
		output += ".";
		return output;
	}

	// now decide if the rhythm can be represented simply with two dots.
	HumNum test2dot = (duration * 4) / 7;
	if (test2dot.getNumerator() == 1) {
		// double dot works
		string output = to_string(test2dot.getDenominator() * 2);
		output += "..";
		return output;
	}

	// now decide if the rhythm can be represented simply with three dots.
	HumNum test3dot = (duration * 8) / 15;
	if (test3dot.getNumerator() == 1) {
		// single dot works
		string output = to_string(test3dot.getDenominator() * 4);
		output += "...";
		return output;
	}

	// duration required more than three dots or is not simple,
	// so assume that it is not simple:
	string output = to_string(duration.getDenominator());
	output += "%";
	output += to_string(duration.getNumerator());
	return output;
}



//////////////////////////////
//
// Convert::durationFloatToRecip -- not allowed to have more than
//	three rhythmic dots
//	default value: timebase = 1;
//

string Convert::durationFloatToRecip(double input, HumNum timebase) {
	string output;

   double testinput = input;
   double basic = 4.0 / input * timebase.getFloat();
   double diff = basic - (int)basic;

   if (diff > 0.998) {
      diff = 1.0 - diff;
      basic += diff;
   }

	// do power of two checks instead
   if (input == 0.0625)  { output = "64"; return output; }
   if (input == 0.125)   { output = "32"; return output; }
   if (input == 0.25)    { output = "16"; return output; }
   if (input == 0.5)  { output = "8";    return output; }
   if (input == 1.0)  { output = "4";    return output; }
   if (input == 2.0)  { output = "2";    return output; }
   if (input == 4.0)  { output = "1";    return output; }
   if (input == 8.0)  { output = "0";    return output; }
   if (input == 12.0) { output = "0.";   return output; }
   if (input == 16.0) { output = "00";   return output; }
   if (input == 24.0) { output = "00.";  return output; }
   if (input == 32.0) { output = "000";  return output; }
   if (input == 48.0) { output = "000."; return output; }

   // special case for triplet whole notes:
   if (fabs(input - (4.0 * 2.0 / 3.0)) < 0.0001) {
		return "3%2";
   }

   // special case for triplet breve notes:
   if (fabs(input - (4.0 * 4.0 / 3.0)) < 0.0001) {
		return "3%4";
   }

   // special case for 9/8 full rests
   if (fabs(input - (4.0 * 9.0 / 8.0)) < 0.0001) {
		return "8%9";
   }

   // special case for 9/2 full-measure rest
   if (fabs(input - 18.0) < 0.0001) {
		return "2%9";
   }

   // handle special rounding cases primarily for SCORE which
   // only stores 4 digits for a duration
   if (input == 0.0833) {
      // triplet 32nd note, which has a real duration of 0.0833333 etc.
		return "48";
   }

   if (diff < 0.002) {
		output += to_string((int)basic);
   } else {
      testinput = input / 3.0 * 2.0;
      basic = 4.0 / testinput;
      diff = basic - (int)basic;
      if (diff < 0.002) {
			output += to_string((int)basic);
			output += ".";
      } else {
         testinput = input / 7.0 * 4.0;
         basic = 4.0 / testinput;
         diff = basic - (int)basic;
         if (diff < 0.002) {
				output += to_string((int)basic);
            output += "..";
         } else {
            testinput = input / 15.0 * 4.0;
            basic = 2.0 / testinput;
            diff = basic - (int)basic;
            if (diff < 0.002) {
					output += to_string((int)basic);
               output += "...";
            } else {
					// Don't know what it could be so echo as a grace note.
					output += "q";
					output += to_string(input);
            }
         }
      }
   }

   return output;
}



//////////////////////////////
//
// Convert::timeSigToDurationInQuarters -- Convert a **kern time signature 
//   into the duration of the measure for that time signature.
//   output units are in quarter notes.
//   Example: 6/8 => 3 quarters
//   Example: 3/4 => 3 quarters
//   Example: 3/8 => 3/2 quarters
//

HumNum Convert::timeSigToDurationInQuarter(HTp token) {
	HumRegex hre;
	if (!token->isTimeSignature()) {
		return 0;
	}
	// Handle extended **recip for denominator later...
	if (!hre.search(token, "^\\*M(\\d+)/(\\d+)")) {
		return 0;
	}
	int top = hre.getMatchInt(1);
	int bot = hre.getMatchInt(2);
	HumNum output = 4;
	output /= bot;
	output *= top;
	return output;
}



// END_MERGE

} // end namespace hum



