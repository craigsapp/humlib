//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Aug  8 12:24:49 PDT 2015
// Last Modified: Sun Aug  9 21:03:12 PDT 2015
// Filename:      Convert-kern.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/Convert-string.cpp
// Syntax:        C++11; humlib
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Conversions related to strings.
//

#include "Convert.h"
#include "HumRegex.h"

#include <sstream>
#include <cctype>
#include <cmath>

using namespace std;

namespace hum {

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
	if (isKernRest(kerndata)) {
		return false;
	}
	char ch;
	for (int i=0; i < (int)kerndata.size(); i++) {
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
	for (int i=0; i < (int)kerndata.size(); i++) {
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
	for (int i=0; i < (int)kerndata.size(); i++) {
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
	return kerndata.find('(') != string::npos;
}



//////////////////////////////
//
// Convert::hasKernSlurEnd -- Returns true if the input string
//   has a ')'.
//

bool Convert::hasKernSlurEnd(const string& kerndata) {
	return kerndata.find(')') != string::npos;
}



//////////////////////////////
//
// Convert::hasKernPhraseStart -- Returns true if the input string
//   has a '{'.
//

bool Convert::hasKernPhraseStart(const string& kerndata) {
	return kerndata.find('{') != string::npos;
}



//////////////////////////////
//
// Convert::hasKernPhraseEnd -- Returns true if the input string
//   has a '}'.
//

bool Convert::hasKernPhraseEnd(const string& kerndata) {
	return kerndata.find('}') != string::npos;
}



//////////////////////////////
//
// Convert::getKernBeamStartElisionLevel -- Returns the number of
//   '&' characters before the given 'L' character in a kern token.
//   Returns -1 if no 'L' character in string.
//

int Convert::getKernBeamStartElisionLevel(const string& kerndata, int index) {
	bool foundBeamStart = false;
	int output = 0;
	int count = 0;
	int target = index + 1;
	for (int i=0; i<(int)kerndata.size(); i++) {
		char ch = kerndata[i];
		if (ch == 'L') {
			count++;
		}
		if (count == target) {
			foundBeamStart = true;
			for (int j=i-1; j>=0; j--) {
				ch = kerndata[j];
				if (ch == '&') {
					output++;
				} else {
					break;
				}
			}
			break;
		}
	}
	if (!foundBeamStart) {
		return -1;
	} else {
		return output;
	}
}



//////////////////////////////
//
// Convert::getKernBeamEndElisionLevel -- Returns the number of
//   '&' characters before the last 'J' character in a kern token.
//   Returns -1 if no 'J' character in string.
//

int Convert::getKernBeamEndElisionLevel(const string& kerndata, int index) {
	bool foundBeamEnd = false;
	int output = 0;
	int count = 0;
	int target = index + 1;
	for (int i=0; i<(int)kerndata.size(); i++) {
		char ch = kerndata[i];
		if (ch == 'J') {
			count++;
		}
		if (count == target) {
			foundBeamEnd = true;
			for (int j=i-1; j>=0; j--) {
				ch = kerndata[j];
				if (ch == '&') {
					output++;
				} else {
					break;
				}
			}
			break;
		}
	}
	if (!foundBeamEnd) {
		return -1;
	} else {
		return output;
	}
}



//////////////////////////////
//
// Convert::getKernSlurStartElisionLevel -- Returns the number of
//   '&' characters before the given '(' character in a kern token.
//   Returns -1 if no '(' character in string.
//

int Convert::getKernSlurStartElisionLevel(const string& kerndata, int index) {
	bool foundSlurStart = false;
	int output = 0;
	int count = 0;
	int target = index + 1;
	for (int i=0; i<(int)kerndata.size(); i++) {
		char ch = kerndata[i];
		if (ch == '(') {
			count++;
		}
		if (count == target) {
			foundSlurStart = true;
			for (int j=i-1; j>=0; j--) {
				ch = kerndata[j];
				if (ch == '&') {
					output++;
				} else {
					break;
				}
			}
			break;
		}
	}
	if (!foundSlurStart) {
		return -1;
	} else {
		return output;
	}
}



//////////////////////////////
//
// Convert::getKernSlurEndElisionLevel -- Returns the number of
//   '&' characters before the last ')' character in a kern token.
//   Returns -1 if no ')' character in string.
//

int Convert::getKernSlurEndElisionLevel(const string& kerndata, int index) {
	bool foundSlurEnd = false;
	int output = 0;
	int count = 0;
	int target = index + 1;
	for (int i=0; i<(int)kerndata.size(); i++) {
		char ch = kerndata[i];
		if (ch == ')') {
			count++;
		}
		if (count == target) {
			foundSlurEnd = true;
			for (int j=i-1; j>=0; j--) {
				ch = kerndata[j];
				if (ch == '&') {
					output++;
				} else {
					break;
				}
			}
			break;
		}
	}
	if (!foundSlurEnd) {
		return -1;
	} else {
		return output;
	}
}



//////////////////////////////
//
// Convert::getKernPhraseStartElisionLevel -- Returns the number of
//   '&' characters before the given '{' character in a kern token.
//   Returns -1 if no '{' character in string.
//

int Convert::getKernPhraseStartElisionLevel(const string& kerndata, int index) {
	bool foundPhraseStart = false;
	int output = 0;
	int count = 0;
	int target = index + 1;
	for (int i=0; i<(int)kerndata.size(); i++) {
		char ch = kerndata[i];
		if (ch == '{') {
			count++;
		}
		if (count == target) {
			foundPhraseStart = true;
			for (int j=i-1; j>=0; j--) {
				ch = kerndata[j];
				if (ch == '&') {
					output++;
				} else {
					break;
				}
			}
			break;
		}
	}
	if (!foundPhraseStart) {
		return -1;
	} else {
		return output;
	}
}



//////////////////////////////
//
// Convert::getKernPhraseEndElisionLevel -- Returns the number of
//   '&' characters before the last '}' character in a kern token.
//   Returns -1 if no '}' character in string.
//

int Convert::getKernPhraseEndElisionLevel(const string& kerndata, int index) {
	bool foundPhraseEnd = false;
	int output = 0;
	int count = 0;
	int target = index + 1;
	for (int i=0; i<(int)kerndata.size(); i++) {
		char ch = kerndata[i];
		if (ch == '}') {
			count++;
		}
		if (count == target) {
			foundPhraseEnd = true;
			for (int j=i-1; j>=0; j--) {
				ch = kerndata[j];
				if (ch == '&') {
					output++;
				} else {
					break;
				}
			}
			break;
		}
	}
	if (!foundPhraseEnd) {
		return -1;
	} else {
		return output;
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



//////////////////////////////
//
// Convert::hasKernStemDirection -- Returns true if a stem direction in data; otherwise,
//    return false.  If true, then '/' means stem up, and '\\' means stem down.
//

char Convert::hasKernStemDirection(const string& kerndata) {
	for (int i=0; i<(int)kerndata.size(); i++) {
		if (kerndata[i] == '/') {
			return '/';
		}
		if (kerndata[i] == '\\') {
			return '\\';
		}
	}
	return '\0';
}



//////////////////////////////
//
// Convert::kernToRecip -- Extract only the **recip data from **kern data.
//

string Convert::kernToRecip(const std::string& kerndata) {
	string output;
	output.reserve(kerndata.size());
	for (int i=0; i<(int)kerndata.size(); i++) {
		if (kerndata.at(i) == ' ') {
			// only process the first subtoken
			break;
		}
		switch (kerndata.at(i)) {
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
			case '.':   // augmentation dot
			case '%':   // rational rhythms
			case 'q':   // grace note (zero duration)
				output += kerndata.at(i);
		}
	}
	return output;
}


string Convert::kernToRecip(HTp token) {
	return Convert::kernToRecip((string)*token);
}



//////////////////////////////
//
// Convert::kernKeyToNumber -- convert a kern key signature into an integer.
//      For example: *k[f#] == +1, *k[b-e-] == -2, *k[] == 0
//      Input string is expected to be in the form *k[] with the
//      accidentals inside the brackets with no spaces.
//

int Convert::kernKeyToNumber(const string& aKernString) {
	int count = 0;
	int length = (int)aKernString.size();
	int start = 0;
	int sign = 1;

	if ((length == 0) || (aKernString.find("[]") != std::string::npos)) {
		return 0;
	}

	for (int i=0; i<length; i++) {
		if (start) {
			if (aKernString[i] == ']') {
				break;
			} else if (aKernString[i] == '-') {
				sign = -1;
			}
			count++;
		}
		else if (aKernString[i] == '[') {
			start = 1;
		}
	}

	return sign * count/2;
}



//////////////////////////////
//
// Convert::kernToDuration -- returns the kern rhythm's duration, using
//	1.0 as the duration of a quarter note (rhythm=4).
// if the kern token has a "q" then assume that it is a grace note
// and return a duration of zero.
//

HumNum Convert::kernToDuration(const string& aKernString) {
	HumNum zero(0,1);
	HumRegex hre;

	// check for grace notes
	if ((aKernString.find('q') != std::string::npos) ||
			(aKernString.find('Q') != std::string::npos)) {
		return zero;
	}

	// check for dots to modify rhythm
	// also add an exit if a space is found, so that dots
	// from multiple notes in chord notes do not get accidentally
	// get counted together (input to this function should be a
	// single note, but chords may accidentally be sent to this
	// function instead).
	int dotcount = 0;
	for (int i=0; i<(int)aKernString.size(); i++) {
		if (aKernString[i] == '.') {
			dotcount++;
		}
		if (aKernString[i] == ' ') {
			break;
		}
	}

	// parse special rhythms which can't be represented in
	// classical **kern definition.  A non-standard rhythm
	// consists of two numbers separated by any character.
	if (hre.search(aKernString, "(\\d+)[^\\d](\\d+)")) {
		int rtop = stoi(hre.getMatch(1));
		int rbot = stoi(hre.getMatch(2));
		HumNum original(rbot, rtop);  // duration is inverse
		HumNum output(rbot, rtop);    // duration is inverse
		original *= 4;  // adjust to quarter note count;
		output *= 4;    // adjust to quarter note count;
		for (int i=0; i<dotcount; i++) {
			output += original / (int)(pow(2.0, (double)(i+1)));
		}
		return output;
	}

	int index = 0;
	while ((index < (int)aKernString.size()) && !isdigit(aKernString[index])) {
		index++;
	}
	if (index >= (int)aKernString.size()) {
		// no rhythm data found
		return zero;
	}

	// should now be at start of kern rhythm
	int orhythm = 0;
	while ((index < (int)aKernString.size()) && isdigit(aKernString[index])) {
		orhythm *= 10;
		orhythm += aKernString[index] - '0';
		index++;
	}

	HumNum oduration(0,1);
	if ((aKernString.find('0') != std::string::npos) &&
		 (aKernString.find('1') == std::string::npos) &&
		 (aKernString.find('2') == std::string::npos) &&
		 (aKernString.find('3') == std::string::npos) &&
		 (aKernString.find('4') == std::string::npos) &&
		 (aKernString.find('5') == std::string::npos) &&
		 (aKernString.find('6') == std::string::npos) &&
		 (aKernString.find('7') == std::string::npos) &&
		 (aKernString.find('8') == std::string::npos) &&
		 (aKernString.find('9') == std::string::npos)    ) {
		if (aKernString.find("0000000000") != std::string::npos) { // exotic rhythm
			oduration = 4096;
		} else if (aKernString.find("000000000") != std::string::npos) { // exotic rhythm
			oduration = 2048;
		} else if (aKernString.find("00000000") != std::string::npos) { // exotic rhythm
			oduration = 1024;
		} else if (aKernString.find("0000000") != std::string::npos) { // exotic rhythm
			oduration = 512;
		} else if (aKernString.find("000000") != std::string::npos) { // exotic rhythm
			oduration = 256;
		} else if (aKernString.find("00000") != std::string::npos) { // exotic rhythm
			oduration = 128;
		} else if (aKernString.find("0000") != std::string::npos) { // exotic rhythm
			oduration = 64;
		} else if (aKernString.find("000") != std::string::npos) { // 000 = maxima
			oduration = 32;
		} else if (aKernString.find("00") != std::string::npos) {  // 00 = long
			oduration = 16;
		} else { // 0 == breve
			oduration = 8;
		}

	} else {
		// now know everything to create a duration
		if (orhythm == 0) {
			oduration = 8;
		} else {
			oduration = 4;
			oduration /= orhythm;
		}
	}

	HumNum duration = oduration;
	for (int i=0; i<dotcount; i++) {
		duration += oduration / (int)(pow(2.0, (double)(i+1)));
	}

	return duration;
}


// END_MERGE

} // end namespace hum


