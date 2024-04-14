//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Tue Jun 30 22:41:24 PDT 1998
// Last Modified: Tue Sep 24 06:47:55 PDT 2019
// Filename:      humlib/src/MuseRecord.cpp
// Web Address:   http://github.com/craigsapp/humlib/blob/master/src/MuseRecord.cpp
// Syntax:        C++11
// vim:           ts=3
//
// Description:   Functions related to figured bass for the MuseRecord class.
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
// MuseRecord::getFigureCountField -- column 2.
//

string MuseRecord::getFigureCountField(void) {
	allowFigurationOnly("getFigureCountField");
	return extract(2, 2);
}



//////////////////////////////
//
// MuseRecord::getFigurationCountString --
//

string MuseRecord::getFigureCountString(void) {
	allowFigurationOnly("getFigureCount");
	string output = extract(2, 2);
	if (output[0] == ' ') {
		output = "";
	}
	return output;
}



//////////////////////////////
//
// MuseRecord::getFigurationCount --
//

int MuseRecord::getFigureCount(void) {
	allowFigurationOnly("getFigureCount");
	string temp = getFigureCountString();
	int output = (int)strtol(temp.c_str(), NULL, 36);
	return output;
}



//////////////////////////////
//
// getFigurePointerField -- columns 6 -- 8.
//

string MuseRecord::getFigurePointerField(void) {
	allowFigurationOnly("getFigurePointerField");
	return extract(6, 8);
}


//////////////////////////////
//
// figurePointerQ --
//

int MuseRecord::figurePointerQ(void) {
	allowFigurationOnly("figurePointerQ");
	int output = 0;
	for (int i=6; i<=8; i++) {
		if (getColumn(i) != ' ') {
			output = 1;
			break;
		}
	}
	return output;
}



//////////////////////////////
//
// MuseRecord::getFigureString --
//

string MuseRecord::getFigureString(void) {
	string output = getFigureFields();
	for (int i=(int)output.size()-1; i>= 0; i--) {
		if (isspace(output[i])) {
			output.resize((int)output.size() - 1);
		} else {
			break;
		}
	}
	return output;
}



//////////////////////////////
//
// MuseRecord::getFigureFields -- columns 17 -- 80
//

string MuseRecord::getFigureFields(void) {
	allowFigurationOnly("getFigureFields");
	return extract(17, 80);
}


//////////////////////////////
//
// MuseRecord::figureFieldsQ --
//

int MuseRecord::figureFieldsQ(void) {
	allowFigurationOnly("figureFieldsQ");
	int output = 0;
	if (getLength() < 17) {
		output = 0;
	} else {
		for (int i=17; i<=80; i++) {
			if (getColumn(i) != ' ') {
				output = 1;
				break;
			}
		}
	}
	return output;
}



//////////////////////////////
//
// getFigure --
//

string MuseRecord::getFigure(int index) {
	string output;
	allowFigurationOnly("getFigure");
	if (index >= getFigureCount()) {
		return output;
	}
	string temp = getFigureString();
	if (index == 0) {
		return temp;
	}
	HumRegex hre;
	vector<string> pieces;
	hre.split(pieces, temp, " +");
	if (index < (int)pieces.size()) {
	output = pieces[index];
	}
	return output;
}


// END_MERGE

} // end namespace hum



