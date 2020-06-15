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

#include <sstream>
#include <cctype>

#include "Convert.h"

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



// END_MERGE

} // end namespace hum


