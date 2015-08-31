//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Aug  8 12:24:49 PDT 2015
// Last Modified: Sun Aug  9 21:03:12 PDT 2015
// Filename:      HumAddress.h
// URL:           https://github.com/craigsapp/minHumdrum/blob/master/src/HumAddress.h
// Syntax:        C++11
// vim:           ts=3 noexpandtab
//
// Description:   Used to store the location of a token in a HumdrumFile.
//

#include "HumAddress.h"
#include "HumdrumLine.h"

namespace minHumdrum {

// START_MERGE


//////////////////////////////
//
// HumAddress::HumAddress -- HumAddress constructor.
//

HumAddress::HumAddress(void) {
	track         = -1;
	subtrack      = -1;
	subtrackcount = 0;
	fieldindex    = -1;
	owner         = NULL;
}



//////////////////////////////
//
// HumAddress::~HumAddress -- HumAddress deconstructor.
//

HumAddress::~HumAddress() {
	track         = -1;
	subtrack      = -1;
	fieldindex    = -1;
	subtrackcount = 0;
	owner         = NULL;
}



//////////////////////////////
//
// HumAddress::getLineIndex -- Returns the line index in the owning HumdrumFile
//    for the token associated with the address.  Returns -1 if not owned by a
//    HumdrumLine (or line assignments have not been made for tokens in the
//    file).
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
// HumAddress::getLineNumber --  Similar to getLineIndex() but adds one.
//

int HumAddress::getLineNumber(void) const {
	return getLineIndex() + 1;
}



//////////////////////////////
//
// HumAddress::getFieldIndex -- Returns the field index on the line of the
//     token associated with the address.
//

int HumAddress::getFieldIndex(void) const {
	return fieldindex;
}



//////////////////////////////
//
// HumAddress::getDataType -- Return the exclusive interpretation string of the
//    token associated with the address.
//

const HumdrumToken& HumAddress::getDataType(void) const {
	static HumdrumToken null("");
	if (owner == NULL) {
		return null;
	}
	HumdrumToken* tok = owner->getTrackStart(getTrack());
	return *tok;
}



//////////////////////////////
//
// HumAddress::getSpineInfo -- Return the spine information for the token
//     associated with the address.  Examples: "1" the token is in the first
//     (left-most) spine, and there are no active sub-spines for the spine.
//     "(1)a"/"(1)b" are the spine descriptions of the two sub-spines after
//     a split manipulator (*^).  "((1)a)b" is the second sub-spines of the
//     first sub-spine for spine 1.
//
//

const string& HumAddress::getSpineInfo(void) const {
	return spining;
}



//////////////////////////////
//
// HumAddress::getTrack -- The track number of the given spine.  This is the
//   first number in the spine info string.  The track number is the same
//   as a spine number.
//

int HumAddress::getTrack(void) const {
	return track;
}



//////////////////////////////
//
// HumAddress::getSubtrack -- The subtrack number of the given spine.  This
//   functions in a similar manner to layer numbers in MEI data.  The first
//   sub-spine of a spine is always subtrack 1, regardless of whether or not
//   an exchange manipulator (*x) was used to switch the left-to-right ordering
//   of the spines in the file.  All sub-spines regardless of their splitting
//   origin are given sequential subtrack numbers.  For example if the spine
//   info is "(1)a"/"((1)b)a"/"((1)b)b" -- the spine is split, then the second
//   sub-spine only is split--then the sub-spines are labeled as sub-tracks "1",
//   "2", "3" respectively.  When a track has only one sub-spine (i.e., it has
//   been split), the subtrack value will be "0".
//

int HumAddress::getSubtrack(void) const {
	return subtrack;
}



//////////////////////////////
//
// HumAddress::getSubtrackCount -- The number of subtrack spines for a 
//   given spine on the owning HumdurmLine.  Returns 0 if spine analysis
//   has not been done, or if the line does not have spines (i.e., reference
//   records, global comments and empty lines).
//

int HumAddress::getSubtrackCount(void) const {
	return subtrackcount;
}



//////////////////////////////
//
// HumAddress::getTrackString --  Return the track and subtrack as a string.
//      The returned string will have the track number if the sub-spine value
//      is zero.  The optional separator parameter is used to separate the
//      track number from the subtrack number.
// default value: separator = "."
//

string HumAddress::getTrackString(string separator) const {
	string output;
	int thetrack    = getTrack();
	int thesubtrack = getSubtrack();
	output += to_string(thetrack);
	if (thesubtrack > 0) {
		output += separator + to_string(thesubtrack);
	}
	return output;
}



//////////////////////////////
//
// HumAddress::setOwner -- Stores a pointer to the HumdrumLine on which
//   the token associated with this address belongs.  When not owned by
//   a HumdrumLine, the parameter's value should be NULL.
//

void HumAddress::setOwner(HumdrumLine* aLine) {
	owner = aLine;
}



//////////////////////////////
//
// HumAddress::getLine -- return the HumdrumLine which owns the token
//    associated with this address.  Returns NULL if it does not belong
//    to a HumdrumLine object.
//

HumdrumLine* HumAddress::getLine(void) const {
	return owner;
}



//////////////////////////////
//
// HumAddress::hasOwner -- Returns true if a HumdrumLine owns the token
//    associated with the address.
//

bool HumAddress::hasOwner(void) const {
	return owner == NULL ? 0 : 1;
}



//////////////////////////////
//
// HumAddress::setFieldIndex -- Set the field index of associated token
//   in the HumdrumLine owner.  If the token is now owned by a HumdrumLine,
//   then the input parameter should be -1.
//

void HumAddress::setFieldIndex(int index) {
	fieldindex = index;
}



//////////////////////////////
//
// HumAddress::setSpineInfo -- Set the spine description of the associated
//     token.  For example "2" for the second spine (from the left), or
//     "((2)a)b" for a sub-spine created as the left sub-spine of the main
//     spine and then as the right sub-spine of that sub-spine.  This function
//     is used by the HumdrumFileStructure class.
//

void HumAddress::setSpineInfo(const string& spineinfo) {
	spining = spineinfo;
}



//////////////////////////////
//
// HumAddress::setTrack -- Set the track number of the associated token.
//   This should always be the first number in the spine information string,
//   or -1 if the spine info is empty.  Tracks are limited to an arbitrary
//   count of 1000 (could be increased in the future if needed).  This function
//   is used by the HumdrumFileStructure class.
//

void HumAddress::setTrack(int aTrack, int aSubtrack) {
	setTrack(aTrack);
	setSubtrack(aTrack);
}


void HumAddress::setTrack(int aTrack) {
	if (aTrack < 0) {
		aTrack = -1;
	}
	if (aTrack > 1000) {
		aTrack = 1000;
	}
	track = aTrack;
}



//////////////////////////////
//
// HumAddress::setSubtrack -- Set the subtrack of the spine.
//   If the token is the only one active for a spine, the subtrack should
//   be set to zero.  If there are more than one sub-tracks for the spine, this
//   is the one-offset index of the spine (be careful if a sub-spine column
//   is exchanged with another spine other than the one from which it was
//   created.  In this case the subtrack number is not useful to calculate
//   the field index of other sub-tracks for the given track.
//   This function is used by the HumdrumFileStructure class.
//

void HumAddress::setSubtrack(int aSubtrack) {
	if (aSubtrack < 0) {
		aSubtrack = -1;
	}
	if (aSubtrack > 1000) {
		aSubtrack = 1000;
	}
	subtrack = aSubtrack;
}



//////////////////////////////
//
// HumAddress::setSubtrackCount --
//

void HumAddress::setSubtrackCount(int count) {
	subtrackcount = count;
}



// END_MERGE

} // end namespace std;



