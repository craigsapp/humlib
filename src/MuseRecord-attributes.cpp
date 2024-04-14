//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Tue Jun 30 22:41:24 PDT 1998
// Last Modified: Sun Apr 14 03:37:22 PDT 2024
// Filename:      humlib/src/MuseRecord-attributes.cpp
// Web Address:   http://github.com/craigsapp/humlib/blob/master/src/MuseRecord-attributes.cpp
// Syntax:        C++11
// vim:           ts=3
//
// Description:   Musical attribute fuctions for the MuseRecord class.
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

#define E_unknown   (0x7fff)

//////////////////////////////
//
// MuseRecord::getAttributeMap --
//

void MuseRecord::getAttributeMap(map<string, string>& amap) {
	amap.clear();
	// Should be "3" on the next line, but "1" or "2" might catch poorly formatted data.
	string contents = getLine().substr(2);
	if (contents.empty()) {
		return;
	}
	int i = 0;
	string key;
	string value;
	int state = 0;  // 0 outside, 1 = in key, 2 = in value
	while (i < (int)contents.size()) {
		switch (state) {
			case 0: // outside of key or value
				if (!isspace(contents[i])) {
					if (contents[i] == ':') {
						// Strange: should not happen
						key.clear();
						state = 2;
					} else {
						state = 1;
						key += contents[i];
					}
				}
				break;
			case 1: // in key
				if (!isspace(contents[i])) {
					if (contents[i] == ':') {
						value.clear();
						state = 2;
					} else {
						// Add to key, such as "C2" for second staff clef.
						key += contents[i];
					}
				}
				break;
			case 2: // in value
				if (key == "D") {
					value += contents[i];
				} else if (isspace(contents[i])) {
					// store parameter and clear variables
					amap[key] = value;
					state = 0;
					key.clear();
					value.clear();
				} else {
					value += contents[i];
				}
				break;
		}
		i++;
	}

	if ((!key.empty()) && (!value.empty())) {
		amap[key] = value;
	}
}



//////////////////////////////
//
// MuseRecord::getAttributes --
//

string MuseRecord::getAttributes(void) {
	string output;
	switch (getType()) {
		case E_muserec_musical_attributes:
			break;
		default:
			cerr << "Error: cannot use getAttributes function on line: "
				  << getLine() << endl;
			return "";
	}

	int ending = 0;
	int tempcol;
	for (int column=4; column <= getLength(); column++) {
		if (getColumn(column) == ':') {
			tempcol = column - 1;
			while (tempcol > 0 && getColumn(tempcol) != ' ') {
				tempcol--;
			}
			tempcol++;
			while (tempcol <= column) {
				output += getColumn(tempcol);
				if (output.back() == 'D') {
					ending = 1;
				}
				tempcol++;
			}
		}
		if (ending) {
			break;
		}
	}
	return output;
}



//////////////////////////////
//
// MuseRecord::attributeQ --
//

int MuseRecord::attributeQ(const string& attribute) {
	switch (getType()) {
		case E_muserec_musical_attributes:
			break;
		default:
			cerr << "Error: cannot use getAttributes function on line: "
				  << getLine() << endl;
			return 0;
	}


	string attributelist = getAttributes();

	int output = 0;
	int attstrlength = (int)attributelist.size();
	int attlength = (int)attribute.size();

	for (int i=0; i<attstrlength-attlength+1; i++) {
		if (attributelist[i] == attribute[0]) {
			output = 1;
			for (int j=0; j<attlength; j++) {
				if (attributelist[i] != attribute[j]) {
					output = 0;
					break;
				}
			}
			if (output == 1) {
				break;
			}
		}
	}
	return output;
}



//////////////////////////////
//
// MuseRecord::getAttributeInt --
//

int MuseRecord::getAttributeInt(char attribute) {
	switch (getType()) {
		case E_muserec_musical_attributes:
			break;
		default:
			cerr << "Error: cannot use getAttributeInt function on line: "
				  << getLine() << endl;
			return 0;
	}

	int output = E_unknown;
	int ending = 0;
	// int index = 0;
	int tempcol;
	int column;
	for (column=4; column <= getLength(); column++) {
		if (getColumn(column) == ':') {
			tempcol = column - 1;
			while (tempcol > 0 && getColumn(tempcol) != ' ') {
				tempcol--;
			}
			tempcol++;
			while (tempcol <= column) {
				if (getColumn(tempcol) == attribute) {
					ending = 2;
				} else if (getColumn(tempcol) == 'D') {
					ending = 1;
				}
				tempcol++;
				// index++;
			}
		}
		if (ending) {
			break;
		}
	}

	if (ending == 0 || ending == 1) {
		return output;
	} else {
		string value = &getColumn(column+1);
		if (value.empty()) {
			output = std::stoi(value);
			return output;
		} else {
			return 0;
		}
	}
}



//////////////////////////////
//
// MuseRecord::getAttributeField -- returns true if found attribute
//

int MuseRecord::getAttributeField(string& value, const string& key) {
	switch (getType()) {
		case E_muserec_musical_attributes:
			break;
		default:
			cerr << "Error: cannot use getAttributeInt function on line: "
				  << getLine() << endl;
			return 0;
	}

	int returnValue = 0;
	int ending = 0;
	// int index = 0;
	int tempcol;
	int column;
	for (column=4; column <= getLength(); column++) {
		if (getColumn(column) == ':') {
			tempcol = column - 1;
			while (tempcol > 0 && getColumn(tempcol) != ' ') {
				tempcol--;
			}
			tempcol++;
			while (tempcol <= column) {
				if (getColumn(tempcol) == key[0]) {
					ending = 2;
				} else if (getColumn(tempcol) == 'D') {
					ending = 1;
				}
				tempcol++;
				// index++;
			}
		}
		if (ending) {
			break;
		}
	}

	value.clear();
	if (ending == 0 || ending == 1) {
		return returnValue;
	} else {
		returnValue = 1;
		column++;
		while (getColumn(column) != ' ') {
			value += getColumn(column++);
		}
		return returnValue;
	}
}


// END_MERGE

} // end namespace hum



