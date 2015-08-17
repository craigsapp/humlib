//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Aug  8 12:24:49 PDT 2015
// Last Modified: Sun Aug  9 21:03:12 PDT 2015
// Filename:      HumdrumToken.h
// URL:           https://github.com/craigsapp/minHumdrum/blob/master/src/HumdrumToken.h
// Syntax:        C++11
// vim:           ts=3
//
// Description:   Used to work with tokens on a Humdrum line.
//

#include "HumAddress.h"
#include "HumdrumToken.h"
#include "HumdrumLine.h"
#include "Convert.h"

namespace minHumdrum {

// START_MERGE

// spine mainipulators:
#define SPLIT_TOKEN       "*^"
#define MERGE_TOKEN       "*v"
#define EXCHANGE_TOKEN    "*x"
#define TERMINATE_TOKEN   "*-"
#define ADD_TOKEN         "*+"
// Also exclusive interpretations which start "**" followed by the data type.

// other special tokens:
#define NULL_DATA            "."
#define NULL_INTERPRETATION  "*"
#define NULL_COMMENT_LOCAL   "!"
#define NULL_COMMENT_GLOBAL  "!!"



//////////////////////////////
//
// HumdrumToken::HumdrumToken --
//

HumdrumToken::HumdrumToken(void) : string() {
	rhycheck = 0;
	setPrefix("!");
}

HumdrumToken::HumdrumToken(const string& aString) : string(aString) {
	rhycheck = 0;
	setPrefix("!");
}

HumdrumToken::HumdrumToken(const char* aString) : string(aString) {
	rhycheck = 0;
	setPrefix("!");
}



//////////////////////////////
//
// HumdrumToken::equalChar -- return true if the character at the given
//     index is the given char.
//

bool HumdrumToken::equalChar(int index, char ch) const {
	if (size() <= index) {
		return false;
	}
	if (index < 0) {
		return false;
	}
	if (((string)(*this))[index] == ch) {
		return true;
	} else {
		return false;
	}
}



//////////////////////////////
//
// HumdrumToken::getPreviousNullDataTokenCount --
//

int HumdrumToken::getPreviousNonNullDataTokenCount(void) {
	return previousNonNullTokens.size();
}



//////////////////////////////
//
// HumdrumToken::getPreviousNullDataTokenCount --
//


HumdrumToken* HumdrumToken::getPreviousNonNullDataToken(int index) {
	if (index < 0) {
		index += previousNonNullTokens.size();
	}
	if (index < 0) {
		return NULL;
	}
	if (index >= previousNonNullTokens.size()) {
		return NULL;
	}
	return previousNonNullTokens[index];
}



//////////////////////////////
//
// HumdrumToken::getNextNonNullDataTokenCount --
//

int HumdrumToken::getNextNonNullDataTokenCount(void) {
	return nextNonNullTokens.size();
}



//////////////////////////////
//
// HumdrumToken::getNextNonNullDataToken --
//

HumdrumToken* HumdrumToken::getNextNonNullDataToken(int index) {
	if (index < 0) {
		index += nextNonNullTokens.size();
	}
	if (index < 0) {
		return NULL;
	}
	if (index >= nextNonNullTokens.size()) {
		return NULL;
	}
	return nextNonNullTokens[index];
}



//////////////////////////////
//
// HumdrumToken::HumdrumToken --
//

HumdrumToken::~HumdrumToken() {
	// do nothing
}



//////////////////////////////
//
// HumdrumToken::getDataType -- Get the exclusive interpretation type.
//

const string& HumdrumToken::getDataType(void) const {
	return address.getDataType();
}



//////////////////////////////
//
// HumdrumToken::setSpineInfo -- Set the spine manipulation history string.
//

void HumdrumToken::setSpineInfo(const string& spineinfo) {
	address.setSpineInfo(spineinfo);
}



//////////////////////////////
//
// HumdrumToken::getSpineInfo --
//

string HumdrumToken::getSpineInfo(void) const {
	return address.getSpineInfo();
}



//////////////////////////////
//
// HumdrumToken::getLineIndex --
//

int HumdrumToken::getLineIndex(void) const {
	return address.getLineIndex();
}



//////////////////////////////
//
// HumdrumToken::getLineNumber --
//

int HumdrumToken::getLineNumber(void) const {
	return address.getLineNumber();
}



//////////////////////////////
//
// HumdrumToken::setFieldIndex --
//

void HumdrumToken::setFieldIndex(int index) {
	address.setFieldIndex(index);
}



//////////////////////////////
//
// HumdrumToken::setTrack -- Set the track (similar to a staff in MEI).
//

void HumdrumToken::setTrack(int aTrack) {
	address.setTrack(aTrack);
}



//////////////////////////////
//
// HumdrumToken::setTrack -- Set the track and subtrack (similar to a
//     staff and layer in MEI).
//

void HumdrumToken::setTrack(int aTrack, int aSubtrack) {
	setTrack(aTrack);
	setSubtrack(aSubtrack);
}



//////////////////////////////
//
// HumdrumToken::getTrack -- Get the track (similar to a staff in MEI).
//

int HumdrumToken::getTrack(void) const {
	return address.getTrack();
}



//////////////////////////////
//
// HumdrumToken::setSubtrack -- Set the subtrack (similar to a layer
//    in MEI).
//

void HumdrumToken::setSubtrack(int aSubtrack) {
	address.setSubtrack(aSubtrack);
}



//////////////////////////////
//
// HumdrumToken::setPreviousToken --
//

void HumdrumToken::setPreviousToken(HumdrumToken* aToken) {
	previousTokens.resize(1);
	previousTokens[0] = aToken;
}



//////////////////////////////
//
// HumdrumToken::setNextToken --
//

void HumdrumToken::setNextToken(HumdrumToken* aToken) {
	nextTokens.resize(1);
	nextTokens[0] = aToken;
}



//////////////////////////////
//
// HumdrumToken::getNextToken --
//    default value: index = 0
//

HumdrumToken* HumdrumToken::getNextToken(int index) const {
	if ((index >= 0) && (index < (int)nextTokens.size())) {
		return nextTokens[index];
	} else {
		return NULL;
	}
}



//////////////////////////////
//
// HumdrumToken::getNextTokens --
//

vector<HumdrumToken*> HumdrumToken::getNextTokens(void) const {
	return nextTokens;
}



//////////////////////////////
//
// HumdrumToken::getPreviousTokens --
//

vector<HumdrumToken*> HumdrumToken::getPreviousTokens(void) const {
	return previousTokens;
}



//////////////////////////////
//
// HumdrumToken::getPreviousToken --
//    default value: index = 0
//

HumdrumToken* HumdrumToken::getPreviousToken(int index) const {
	if ((index >= 0) && (index < (int)previousTokens.size())) {
		return previousTokens[index];
	} else {
		return NULL;
	}
}



//////////////////////////////
//
// HumdrumToken::analyzeDuration -- Currently reads the duration of
//   **kern and **recip data.  Add more data types here.
//

bool HumdrumToken::analyzeDuration(void) {
	if ((*this) == NULL_DATA) {
		duration.setValue(-1);
		return true;
	}
	if (equalChar(0 ,'!')) {
		duration.setValue(-1);
		return true;
	}
	if (equalChar(0 ,'*')) {
		duration.setValue(-1);
		return true;
	}
	if (equalChar(0 ,'=')) {
		duration.setValue(-1);
		return true;
	}
	string dtype = getDataType();
	if (hasRhythm()) {
		if (isData()) {
			if (!isNull()) {
				duration = Convert::recipToDuration((string)(*this));
			} else {
				duration.setValue(-1);
			}
		} else {
			duration.setValue(-1);
		}
	} else {
		duration.setValue(-1);
	}
	return true;
}



///////////////////////////////
//
// HumdrumToken::isManipulator -- returns true if token is one of:
//    SPLIT_TOKEN     = "*^"  == spine splitter
//    MERGE_TOKEN     = "*v"  == spine merger
//    EXCHANGE_TOKEN  = "*x"  == spine exchanger
//    ADD_TOKEN       = "*+"  == spine adder
//    TERMINATE_TOKEN = "*-"  == spine terminator
//    **...  == exclusive interpretation
//

bool HumdrumToken::isManipulator(void) const {
	if (isSplitInterpretation())     { return true; }
	if (isMergeInterpretation())     { return true; }
	if (isExchangeInterpretation())  { return true; }
	if (isAddInterpretation())       { return true; }
	if (isTerminateInterpretation()) { return true; }
	if (isExclusiveInterpretation()) { return true; }
	return false;
}



//////////////////////////////
//
// HumdrumToken::getDuration --
//

HumNum HumdrumToken::getDuration(void) const {
	return duration;
}



//////////////////////////////
//
// HumdrumToken::setDuration --
//

void HumdrumToken::setDuration(const HumNum& dur) {
	duration = dur;
}



//////////////////////////////
//
// HumdrumToken::getDurationFromStart --
//

HumNum HumdrumToken::getDurationFromStart(void) const {
	return getLine()->getDurationFromStart();
}



//////////////////////////////
//
// HumdrumToken::hasRhythm -- Returns true if the exclusive interpretation
//    contains rhythmic data which will be used for analyzing the
//    duration of a HumdrumFile, for example.
//

bool HumdrumToken::hasRhythm(void) const {
	string type = getDataType();
	if (type == "**kern") {
		return true;
	}
	if (type == "**recip") {
		return true;
	}
	return false;
}



//////////////////////////////
//
// HumdrumToken::isBarlines --
//

bool HumdrumToken::isBarline(void) const {
	if (size() == 0) {
		return false;
	}
	if ((*this)[0] == '=') {
		return true;
	} else {
		return false;
	}
}



//////////////////////////////
//
// HumdrumToken::isCommentLocal -- Presumed to be in a spine where
//   only local comments are allowed.
//

bool HumdrumToken::isCommentLocal(void) const {
	if (size() == 0) {
		return false;
	}
	if ((*this)[0] == '!') {
		if (size() > 1) {
			if ((*this)[1] == '!') {
				// global comment
				return false;
			}
		}
		return true;
	} else {
		return false;
	}
}



//////////////////////////////
//
// HumdrumToken::isData -- Returns true if not an interpretation, barline
//      or local comment;
//

bool HumdrumToken::isData(void) const {
	if (size() == 0) {
		return false;
	}
	int firstchar = (*this)[0];
	if ((firstchar == '*') || (firstchar == '!') || (firstchar == '=')) {
		return false;
	}
	return true;
}



//////////////////////////////
//
// HumdrumToken::isExclusive -- Returns true if first two characters
//     are "**".
//

bool HumdrumToken::isExclusive(void) const {
	const string& tok = (string)(*this);
	return tok.substr(0, 2) == "**";
}



//////////////////////////////
//
// HumdrumToken::isSplitInterpretation --
//

bool HumdrumToken::isSplitInterpretation(void) const {
	return ((string)(*this)) == SPLIT_TOKEN;
}



//////////////////////////////
//
// HumdrumToken::isMergeInterpretation --
//

bool HumdrumToken::isMergeInterpretation(void) const {
	return ((string)(*this)) == MERGE_TOKEN;
}



//////////////////////////////
//
// HumdrumToken::isExchangeInterpretation --
//

bool HumdrumToken::isExchangeInterpretation(void) const {
	return ((string)(*this)) == EXCHANGE_TOKEN;
}



//////////////////////////////
//
// HumdrumToken::isTerminateInterpretation --
//

bool HumdrumToken::isTerminateInterpretation(void) const {
	return ((string)(*this)) == TERMINATE_TOKEN;
}



//////////////////////////////
//
// HumdrumToken::isAddInterpretation --
//

bool HumdrumToken::isAddInterpretation(void) const {
	return ((string)(*this)) == ADD_TOKEN;
}



//////////////////////////////
//
// HumdrumToken::isNull -- Returns true if the token is a null token,
//   either for data, comments, or interpretations.  Does not consider
//   null global comments since they are not part of the spine structure.
//

bool HumdrumToken::isNull(void) const {
	const string& tok = (string)(*this);
	if (tok == NULL_DATA)           { return true; }
	if (tok == NULL_INTERPRETATION) { return true; }
	if (tok == NULL_COMMENT_LOCAL)  { return true; }
	return false;
}



//////////////////////////////
//
// HumdrumToken::getSubtrack -- Get the subtrack (similar to a layer
//    in MEI).
//

int HumdrumToken::getSubtrack(void) const {
	return address.getSubtrack();
}



//////////////////////////////
//
// HumdrumToken::getTrackString -- Get "track.subtrack" as a string.
//

string HumdrumToken::getTrackString(void) const {
	return address.getTrackString();
}



/////////////////////////////
//
// HumdrumToken::getSubtokenCount --
//   default value: separator = " "
//

int HumdrumToken::getSubtokenCount(const string& separator) const {
	int count = 0;
	string::size_type start = 0;
	while ((start = string::find(separator, start)) != string::npos) {
		count++;
		start += separator.size();
	}
	return count;
}



/////////////////////////////
//
// HumdrumToken::getSubtoken --
//   default value: separator = " "
//

string HumdrumToken::getSubtoken(int index, const string& separator) const {
	if (index < 0) {
		return "";
	}
	int count = 0;
	int start = 0;
	int end   = 0;
	while ((end = string::find(separator, start)) != string::npos) {
		count++;
		if (count == index) {
			return string::substr(start, end-start);
		}
		start += separator.size();
	}
	if (count == index) {
		return string::substr(start, string::size()-start);
	}
	return "";
}



//////////////////////////////
//
// HumdrumToken::setParameters -- Process a local comment with
//     the structure:
//        !NS1:NS2:key1=value1:key2=value2:key3=value3
//

void HumdrumToken::setParameters(HumdrumToken* ptok) {
	HumdrumToken& pl = *ptok;
	if (pl.size() <= 1) {
		return;
	}
	string pdata = pl.substr(1, pl.size()-1);
	setParameters(pdata);
}


void HumdrumToken::setParameters(const string& pdata) {
	vector<string> pieces = Convert::splitString(pdata, ':');
	if (pieces.size() < 3) {
		return;
	}
	string ns1 = pieces[0];
	string ns2 = pieces[1];
	string key;
	string value;
	int loc;
	for (int i=2; i<pieces.size(); i++) {
		Convert::replaceOccurrences(pieces[i], "&colon;", ":");
		loc = pieces[i].find("=");
		if (loc != string::npos) {
			key   = pieces[i].substr(0, loc);
			value = pieces[i].substr(loc+1, pieces[i].size());
		} else {
			key   = pieces[i];
			value = "true";
		}
		setValue(ns1, ns2, key, value);
	}
}



//////////////////////////////
//
// HumdrumToken::makeForwardLink --
//

void HumdrumToken::makeForwardLink(HumdrumToken& nextToken) {
	nextTokens.push_back(&nextToken);
	nextToken.previousTokens.push_back(this);
}



//////////////////////////////
//
// HumdrumToken::makeBackwarddLink --
//

void HumdrumToken::makeBackwardLink(HumdrumToken& previousToken) {
	previousTokens.push_back(&previousToken);
	previousToken.nextTokens.push_back(this);
}



//////////////////////////////
//
// HumdrumToken::setOwner --
//

void HumdrumToken::setOwner(HumdrumLine* aLine) {
	address.setOwner(aLine);
}



//////////////////////////////
//
// HumdrumToken::getOwner --
//

HumdrumLine* HumdrumToken::getOwner(void) const {
	return address.getOwner();
}



//////////////////////////////
//
// HumdrumToken::getState --
//

int HumdrumToken::getState(void) const {
	return rhycheck;
}



//////////////////////////////
//
// HumdrumToken::getState --
//

void HumdrumToken::incrementState(void) {
	rhycheck++;
}



//////////////////////////////
//
// HumdrumToken::getNextTokenCount -- Return the number of tokens in the
//   spine/subspine which follow this token.  Typically this will be 1,
//   but will be zero for a terminator interpretation (*-), and will be
//   2 for a split interpretation (*^).
//

int HumdrumToken::getNextTokenCount(void) const {
	return nextTokens.size();
}



//////////////////////////////
//
// HumdrumToken::getPreviousTokenCount -- Return the number of tokens
//   in the spine/subspine which precede this token.  Typically this will
//   be 1, but will be zero for an exclusive interpretation (starting with
//   "**"), and will be greater than one for a token which follows a
//   spine merger (using *v interpretations).
//

int HumdrumToken::getPreviousTokenCount(void) const {
	return previousTokens.size();
}



//////////////////////////////
//
// operator<< -- Needed to avoid interaction with HumHash.
//

ostream& operator<<(ostream& out, const HumdrumToken& token) {
	out << (string)token;
	return out;
}



// END_MERGE

} // end namespace std;



