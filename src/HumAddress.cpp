//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Aug  8 12:24:49 PDT 2015
// Last Modified: Sun Aug  9 21:03:12 PDT 2015
// Filename:      HumAddress.h
// URL:           https://github.com/craigsapp/minHumdrum/blob/master/src/HumAddress.h
// Syntax:        C++11
// vim:           ts=3
//
// Description:   Used to store the location of a token in a HumdrumFile.
//

#include "HumAddress.h"
#include "HumdrumLine.h"

// START_MERGE

//////////////////////////////
//
// HumAddress::HumAddress --
//

HumAddress::HumAddress(void) {
	track      = -1;
	subtrack   = -1;
	fieldindex = -1;
	owner      = NULL;
}



//////////////////////////////
//
// HumAddress::~HumAddress --
//

HumAddress::~HumAddress() {
	track      = -1;
	subtrack   = -1;
	fieldindex = -1;
	owner      = NULL;
}



//////////////////////////////
//
// HumAddress::getLineIndex -- Returns -1 if not owned by a HumdrumLine.
//

int  HumAddress::getLineIndex(void) const {
	if (owner == NULL) {
		return -1;
	} else {
		return owner->getLineIndex();
	}
}



//////////////////////////////
//
// HumAddress::getLineNumber --
//

int  HumAddress::getLineNumber(void) const {
	return getLineIndex() + 1;
}



//////////////////////////////
//
// HumAddress::getFieldIndex --
//

int  HumAddress::getFieldIndex(void) const {
	return fieldindex;
}



//////////////////////////////
//
// HumAddress::getDataType --
//

const HumdrumToken& HumAddress::getDataType(void) const {
	static HumdrumToken null("");
	if (owner == NULL) {
		return null;
	}
	HumdrumToken* tok = owner->getSpineStart(getTrack());
	return *tok;
}



//////////////////////////////
//
// HumAddress::getSpineInfo --
//

const string& HumAddress::getSpineInfo(void) const {
	return spining;
}



//////////////////////////////
//
// HumAddress::getTrack --
//

int HumAddress::getTrack(void) const {
	return track;
}



//////////////////////////////
//
// HumAddress::getSubTrack --
//

int HumAddress::getSubtrack(void) const {
	return subtrack;
}



//////////////////////////////
//
// HumAddress::getTrackString --
//

string HumAddress::getTrackString(void) const {
	string output;
	int thetrack    = getTrack();
	int thesubtrack = getSubtrack();
	output += to_string(thetrack);
	if (thesubtrack > 0) {
		output += '.' + to_string(thesubtrack);
	}
	return output;
}



//////////////////////////////
//
// HumAddress::setOwner --
//

void HumAddress::setOwner(HumdrumLine* aLine) {
	owner = aLine;
}



//////////////////////////////
//
// HumAddress::getLine -- return the HumdrumLine which owns the token
//    associated with this address.
//

HumdrumLine* HumAddress::getLine(void) const {
	return owner;
}



//////////////////////////////
//
// HumAddress::hasOwner -- Returns true if a HumdrumLine owns the specified
//    token.
//

bool HumAddress::hasOwner(void) const {
	return owner == NULL ? 0 : 1;
}



//////////////////////////////
//
// HumAddress::setFieldIndex --
//

void HumAddress::setFieldIndex(int index) {
	fieldindex = index;
}


/*
//////////////////////////////
//
// HumAddress::setDataType --
//

void HumAddress::setDataType(const string& datatype) {
	switch (datatype.size()) {
		case 0:
			cerr << "Error: cannot have an empty data type." << endl;
			exit(1);
		case 1:
			if (datatype[0] == '*') {
				cerr << "Error: incorrect data type: " << datatype << endl;
				exit(1);
			} else {
				exinterp = "**" + datatype;
				return;
			}
		case 2:
			if ((datatype[0] == '*') && (datatype[1] == '*')) {
				cerr << "Error: incorrect data type: " << datatype << endl;
				exit(1);
			} else if (datatype[1] == '*') {
				exinterp = "**" + datatype;
				return;
			} else {
				exinterp = '*' + datatype;
				return;
			}
		default:
			if (datatype[0] != '*') {
				exinterp = "**" + datatype;
				return;
			} else if (datatype[1] == '*') {
				exinterp = datatype;
				return;
			} else {
				exinterp = '*' + datatype;
				return;
			}
	}
}

*/


//////////////////////////////
//
// HumAddress::setSpineInfo --
//

void HumAddress::setSpineInfo(const string& spineinfo) {
	spining = spineinfo;
}



//////////////////////////////
//
// HumAddress::setTrack --
//

void HumAddress::setTrack(int aTrack, int aSubtrack) {
	setTrack(aTrack);
	setSubtrack(aTrack);
}


void HumAddress::setTrack(int aTrack) {
	if (aTrack < 0) {
		aTrack = 0;
	}
	if (aTrack > 1000) {
		aTrack = 1000;
	}
	track = aTrack;
}



//////////////////////////////
//
// HumAddress::setSubtrack --
//

void HumAddress::setSubtrack(int aSubtrack) {
	if (aSubtrack < 0) {
		aSubtrack = 0;
	}
	if (aSubtrack > 1000) {
		aSubtrack = 1000;
	}
	subtrack = aSubtrack;
}

// END_MERGE



