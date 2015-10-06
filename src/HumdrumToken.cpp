//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Aug  8 12:24:49 PDT 2015
// Last Modified: Sun Aug  9 21:03:12 PDT 2015
// Filename:      HumdrumToken.h
// URL:           https://github.com/craigsapp/minHumdrum/blob/master/src/HumdrumToken.h
// Syntax:        C++11
// vim:           ts=3 noexpandtab
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
// HumdrumToken::HumdrumToken -- Constructor for HumdrumToken.
//

HumdrumToken::HumdrumToken(void) : string() {
	rhycheck = 0;
	setPrefix("!");
	strand = -1;
}

HumdrumToken::HumdrumToken(const string& aString) : string(aString) {
	rhycheck = 0;
	setPrefix("!");
	strand = -1;
}

HumdrumToken::HumdrumToken(const char* aString) : string(aString) {
	rhycheck = 0;
	setPrefix("!");
	strand = -1;
}


//////////////////////////////
//
// HumdrumToken::~HumdrumToken -- Deconstructor for HumdrumToken.
//

HumdrumToken::~HumdrumToken() {
	// do nothing
}


//////////////////////////////
//
// HumdrumToken::equalChar -- Returns true if the character at the given
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
// HumdrumToken::getPreviousNonNullDataTokenCount -- Returns the number of
//   previous tokens in the spine which is not a null token.  For null
//   tokens, this will be a count of the number of non-null tokens which
//   the null represents.
// @SEEALSO: getPreviousNonNullDataToken
//

int HumdrumToken::getPreviousNonNullDataTokenCount(void) {
	return previousNonNullTokens.size();
}



//////////////////////////////
//
// HumdrumToken::getPreviousNonNullDataToken -- Returns the non-null
//    data token which occurs before this token in the data in the same
//    spine.  The default value is index 0, since mostly there will only
//    be one previous token.
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
// HumdrumToken::getNextNonNullDataTokenCount -- Returns the number of non-null
//     data tokens which follow this token in the spine.
//

int HumdrumToken::getNextNonNullDataTokenCount(void) {
	return nextNonNullTokens.size();
}



//////////////////////////////
//
// HumdrumToken::getNextNonNullDataToken -- Returns the given next non-null token
//    following this one in the spine.  The default value for index is 0 since
//    the next non-null data token count will typically be 1.
// default value: index = 0
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
// HumdrumToken::getSlurDuration -- If the note has a slur start, then
//    returns the duration until the endpoint; otherwise, returns 0;
//    Expand later to handle slur ends and elided slurs.  The function
//    HumdrumFileContent::analyzeSlurs() should be called before accessing
//    this function.  If the slur duruation was already calculated, return
//    thave value; otherwise, calculate from the location of a matching
//    slur end.
//

HumNum HumdrumToken::getSlurDuration(HumNum scale) {
	if (!isDataType("**kern")) {
		return 0;
	}
	if (isDefined("auto", "slurDuration")) {
		return getValueFraction("auto", "slurDuration");
	} else if (isDefined("auto", "slurEnd")) {
		HTp slurend = getValueHTp("auto", "slurEnd");
		return slurend->getDurationFromStart(scale) - 
				getDurationFromStart(scale);
	} else {
		return 0;
	}
}



//////////////////////////////
//
// HumdrumToken::getDataType -- Get the exclusive interpretation type for
//     the token.
// @SEEALSO: isDataType
//

const string& HumdrumToken::getDataType(void) const {
	return address.getDataType();
}



//////////////////////////////
//
// HumdrumToken::isDataType -- Returns true if the data type of the token
//   matches the test data type.
// @SEEALSO: getDataType
//

bool HumdrumToken::isDataType(string dtype) const {
	if (dtype.compare(0, 2, "**") == 0) {
		return dtype == getDataType();
	} else {
		return getDataType().compare(2, string::npos, dtype) == 0;
	}
}



//////////////////////////////
//
// HumdrumToken::setSpineInfo -- Sets the spine manipulation history string.
// @SEEALTO: getSpineInfo
//

void HumdrumToken::setSpineInfo(const string& spineinfo) {
	address.setSpineInfo(spineinfo);
}



//////////////////////////////
//
// HumdrumToken::getSpineInfo -- Returns the spine split/merge history
//    for the token.
// @SEEALTO: setSpineInfo
//

string HumdrumToken::getSpineInfo(void) const {
	return address.getSpineInfo();
}



//////////////////////////////
//
// HumdrumToken::getLineIndex -- Returns the line index of the owning
//    HumdrumLine for this token.
// @SEEALTO: getLineNumber
//

int HumdrumToken::getLineIndex(void) const {
	return address.getLineIndex();
}



//////////////////////////////
//
// HumdrumToken::getFieldIndex -- Returns the index of the token the line.
// @SEEALSO: getFieldIndex
//

int HumdrumToken::getFieldIndex(void) const {
	return address.getFieldIndex();
}



//////////////////////////////
//
// HumdrumToken::getLineNumber -- Returns the line index plus 1.
// @SEEALTO: getLineIndex
//

int HumdrumToken::getLineNumber(void) const {
	return address.getLineNumber();
}



//////////////////////////////
//
// HumdrumToken::setFieldIndex -- Sets the field index of the token on the
//   owning HumdrumLine object.
// @SEEALSO: getFieldIndex
//

void HumdrumToken::setFieldIndex(int index) {
	address.setFieldIndex(index);
}



//////////////////////////////
//
// HumdrumToken::setTrack -- Sets the track number (similar to a staff in MEI).
//     The two-parameter version will set the track and sub-track at the same
//     time (subtrack is similar to a staff and layer in MEI).
//

void HumdrumToken::setTrack(int aTrack) {
	address.setTrack(aTrack);
}


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
// HumdrumToken::setSubtrack -- Sets the subtrack (similar to a layer
//    in MEI).
//

void HumdrumToken::setSubtrack(int aSubtrack) {
	address.setSubtrack(aSubtrack);
}



//////////////////////////////
//
// HumdrumToken::setSubtrackCount -- Sets the subtrack count in the
//    HumdrumLine for all tokens in the same track as the current
//    token.
//

void HumdrumToken::setSubtrackCount(int count) {
	address.setSubtrackCount(count);
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
// HumdrumToken::getNextToken -- Returns the next token in the
//    spine.  Since the next token count is usually one, the default
//    index value is zero.  When there is no next token (when the current
//    token is a spine terminaor), then NULL will be returned.
// default value: index = 0
// @SEEALSO: getNextTokens, getPreviousToken
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
// HumdrumToken::getNextTokens -- Returns a list of the next
//   tokens in the spine after this token.
// @SEEALSO: getNextToken
//

vector<HumdrumToken*> HumdrumToken::getNextTokens(void) const {
	return nextTokens;
}



//////////////////////////////
//
// HumdrumToken::getPreviousTokens -- Returns a list of the previous
//    tokens in the spine before this token.
//

vector<HumdrumToken*> HumdrumToken::getPreviousTokens(void) const {
	return previousTokens;
}



//////////////////////////////
//
// HumdrumToken::getPreviousToken -- Returns the previous token in the
//    spine.  Since the previous token count is usually one, the default
//    index value is zero.
// default value: index = 0
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
//   **kern and **recip data.  Add more data types here such as **koto.
//

bool HumdrumToken::analyzeDuration(string& err) {
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
// HumdrumToken::isManipulator -- Returns true if token is one of:
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
// HumdrumToken::getDuration -- Returns the duration of the token.  The token
//    does not necessarily need to have any explicit duration, as the returned
//    value will also include implicit duration calculated in analyzeRhythm
//    in the HumdrumFileStructure class.
//

HumNum HumdrumToken::getDuration(void) const {
	return duration;
}


HumNum HumdrumToken::getDuration(HumNum scale) const {
	return duration * scale;
}


//////////////////////////////
//
// HumdrumToken::setDuration -- Sets the duration of the token.  This is done in
//    HumdrumFileStructure::analyzeTokenDurations().
//

void HumdrumToken::setDuration(const HumNum& dur) {
	duration = dur;
}



//////////////////////////////
//
// HumdrumToken::getDurationFromStart -- Returns the duration from the
//   start of the owning HumdrumFile to the starting time of the
//   owning HumdrumLine for the token.  The durationFromStart is
//   in reference to the start of the token, not the end of the token,
//   which may be on another HumdrumLine.
//

HumNum HumdrumToken::getDurationFromStart(void) const {
	return getLine()->getDurationFromStart();
}


HumNum HumdrumToken::getDurationFromStart(HumNum scale) const {
	return getLine()->getDurationFromStart() * scale;
}



//////////////////////////////
//
// HumdrumToken::getDurationToEnd -- Returns the duration from the
//   start of the current line to the start of the last line
//   (the duration of the last line is always zero, so the duration
//   to end is always the duration to the end of the last non-zero
//   duration line.
//

HumNum HumdrumToken::getDurationToEnd(void) const {
	return getLine()->getDurationToEnd();
}


HumNum HumdrumToken::getDurationToEnd(HumNum scale) const {
	return getLine()->getDurationToEnd() * scale;
}



//////////////////////////////
//
// HumdrumToken::getBarlineDuration -- Returns the duration between
//   the next and previous barline.  If the token is a barline token,
//   then return the duration to the next barline.  The barline duration data 
//   is filled in automatically when reading a file with the 
//   HumdrumFileStructure::analyzeMeter() function.  The duration
//   will always be non-positive if the file is read with HumdrumFileBase and 
//   analyzeMeter() is not run to analyze the data.
//

HumNum HumdrumToken::getBarlineDuration(void) const {
	HumdrumLine* own = getOwner();
	if (own == NULL) {
		return 0;
	}
	return own->getBarlineDuration();
}


HumNum HumdrumToken::getBarlineDuration(HumNum scale) const {
	HumdrumLine* own = getOwner();
	if (own == NULL) {
		return 0;
	}
	return own->getBarlineDuration(scale);
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
// HumdrumToken::isRest -- Returns true if the token is a (kern) rest.
//

bool HumdrumToken::isRest(void) const {
	if (isDataType("**kern")) {
		if (Convert::isKernRest((string)(*this))) {
			return true;
		}
	}
	return false;
}



//////////////////////////////
//
// HumdrumToken::isNote -- Returns true if the token is a (kern) note
//     (possessing a pitch).
//

bool HumdrumToken::isNote(void) const {
	if (isDataType("**kern")) {
		if (Convert::isKernNote((string)(*this))) {
			return true;
		}
	}
	return false;
}



//////////////////////////////
//
// HumdrumToken::hasSlurStart -- Returns true if the **kern token has 
//     a '(' character.
//

bool HumdrumToken::hasSlurStart(void) const {
	if (isDataType("**kern")) {
		if (Convert::hasKernSlurStart((string)(*this))) {
			return true;
		}
	}
	return false;
}



//////////////////////////////
//
// HumdrumToken::hasSlurEnd -- Returns true if the **kern token has
//     a ')' character.
//

bool HumdrumToken::hasSlurEnd(void) const {
	if (isDataType("**kern")) {
		if (Convert::hasKernSlurEnd((string)(*this))) {
			return true;
		}
	}
	return false;
}




//////////////////////////////
//
// HumdrumToken::isSecondaryTiedNote -- Returns true if the token 
//     is a (kern) note (possessing a pitch) and has '_' or ']' characters.
//

bool HumdrumToken::isSecondaryTiedNote(void) const {
	if (isDataType("**kern")) {
		if (Convert::isKernSecondaryTiedNote((string)(*this))) {
			return true;
		}
	}
	return false;
}



//////////////////////////////
//
// HumdrumToken::isBarline -- Returns true if the first character is an
//   equals sign.
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
// HumdrumToken::isCommentLocal -- Returns true of the token start with "!",
//   but not "!!" which is for global comments.
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
// HumdrumToken::isComment -- Returns true of the token start with "!".
//

bool HumdrumToken::isComment(void) const {
	if (size() == 0) {
		return false;
	}
	if ((*this)[0] == '!') {
		return true;
	} else {
		return false;
	}
}



//////////////////////////////
//
// HumdrumToken::isData -- Returns true if not an interpretation, barline
//      or local comment.  This will not work on synthetic tokens generated
//      from an empty line.  So this function should be called only on tokens
//      in lines which pass the HumdrumLine::hasSpines() test.
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
// HumdrumToken::isNonNullData -- Returns true if the token is a data token
//    that is not a null token.
//

bool HumdrumToken::isNonNullData(void) const {
	return isData() && !isNull();
}



//////////////////////////////
//
// HumdrumToken::isNullData -- Returns true if the token is a null
//     data token.
//

bool HumdrumToken::isNullData(void) const {
	return isData() && isNull();
}



//////////////////////////////
//
// HumdrumToken::isExclusiveInterpretation -- Returns true if first two
//     characters are "**".
//

bool HumdrumToken::isExclusiveInterpretation(void) const {
	const string& tok = (string)(*this);
	return tok.substr(0, 2) == "**";
}



//////////////////////////////
//
// HumdrumToken::isSplitInterpretation -- True if the token is "*^".
//

bool HumdrumToken::isSplitInterpretation(void) const {
	return ((string)(*this)) == SPLIT_TOKEN;
}



//////////////////////////////
//
// HumdrumToken::isMergeInterpretation -- True if the token is "*v".
//

bool HumdrumToken::isMergeInterpretation(void) const {
	return ((string)(*this)) == MERGE_TOKEN;
}



//////////////////////////////
//
// HumdrumToken::isExchangeInterpretation -- True if the token is "*x".
//

bool HumdrumToken::isExchangeInterpretation(void) const {
	return ((string)(*this)) == EXCHANGE_TOKEN;
}



//////////////////////////////
//
// HumdrumToken::isTerminateInterpretation -- True if the token is "*-".
//

bool HumdrumToken::isTerminateInterpretation(void) const {
	return ((string)(*this)) == TERMINATE_TOKEN;
}



//////////////////////////////
//
// HumdrumToken::isAddInterpretation -- True if the token is "*+".
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
// HumdrumToken::getTrackString -- Gets "track.subtrack" as a string.  The
//     track and subtrack are integers.  The getTrackString function will
//     return a string with the track and subtrack separated by an dot.  The
//     Dot is not a decimal point, but if the subtrack count does not exceed
//     9, then the returned string can be treated as a floating-point number
//     where the subtrack is the fractional part.
// @SEEALSO: getTrack, getSubtrack
//

string HumdrumToken::getTrackString(void) const {
	return address.getTrackString();
}



/////////////////////////////
//
// HumdrumToken::getSubtokenCount -- Returns the number of sub-tokens in
//     a token.  The input parameter is the sub-token separator.  If the
//     separator comes at the start or end of the token, then there will
//     be empty sub-token(s) included in the count.
// default value: separator = " "
// @SEEALSO: getSubtoken
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
// HumdrumToken::getSubtoken -- Extract the specified sub-token from the token.
//    Tokens usually are separated by spaces in Humdrum files, but this will
//    depened on the data type (so therefore, the tokens are not presplit into
//    sub-tokens when reading in the file).
// default value: separator = " "
// @SEEALSO: getSubtokenCount, getTrackString
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
//     and store the parameter in the HumHash parent class component of the
//     HumdrumToken object.
// default value for 2-parameter version: ptok = NULL
//

void HumdrumToken::setParameters(HumdrumToken* ptok) {
	HumdrumToken& pl = *ptok;
	if (pl.size() <= 1) {
		return;
	}
	string pdata = pl.substr(1, pl.size()-1);
	setParameters(pdata, ptok);
}


void HumdrumToken::setParameters(const string& pdata, HumdrumToken* ptok) {
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
		setOrigin(ns1, ns2, key, ptok);
	}
}



//////////////////////////////
//
// HumdrumToken::makeForwardLink -- Line a following spine token to this one.
//    Used by the HumdrumFileBase::analyzeLinks function.
//

void HumdrumToken::makeForwardLink(HumdrumToken& nextToken) {
	nextTokens.push_back(&nextToken);
	nextToken.previousTokens.push_back(this);
}



//////////////////////////////
//
// HumdrumToken::makeBackwarddLink -- Link a previous spine token to this one.
//    Used by the HumdrumFileBase::analyzeLinks function.
//

void HumdrumToken::makeBackwardLink(HumdrumToken& previousToken) {
	previousTokens.push_back(&previousToken);
	previousToken.nextTokens.push_back(this);
}



//////////////////////////////
//
// HumdrumToken::setOwner -- Sets the HumdrumLine owner of this token.
//

void HumdrumToken::setOwner(HumdrumLine* aLine) {
	address.setOwner(aLine);
}



//////////////////////////////
//
// HumdrumToken::getOwner -- Returns a pointer to the HumdrumLine that
//    owns this token.
//

HumdrumLine* HumdrumToken::getOwner(void) const {
	return address.getOwner();
}



//////////////////////////////
//
// HumdrumToken::getState -- Returns the rhythm state variable.
//

int HumdrumToken::getState(void) const {
	return rhycheck;
}



//////////////////////////////
//
// HumdrumToken::getStrandIndex -- Returns the 1-D strand index
//    that the token belongs to in the owning HumdrumFile.
//    Returns -1 if there is no strand assignment.
//

int  HumdrumToken::getStrandIndex(void) const {
	return strand;
}



//////////////////////////////
//
// HumdrumToken::getSlurStartElisionLevel -- Returns the count of
//   elision marks ('&') preceding a slur start character '('.
//   Returns -1 if there is no slur start character.
//

int HumdrumToken::getSlurStartElisionLevel(void) const { 
	if (isDataType("**kern")) {
		return Convert::getKernSlurStartElisionLevel((string)(*this));
	} else {
		return -1;
	}
}



//////////////////////////////
//
// HumdrumToken::getSlurEndElisionLevel -- Returns the count of
//   elision marks ('&') preceding a slur end character ')'.
//   Returns -1 if there is no slur end character.
//

int HumdrumToken::getSlurEndElisionLevel(void) const { 
	if (isDataType("**kern")) {
		return Convert::getKernSlurEndElisionLevel((string)(*this));
	} else {
		return -1;
	}
}



//////////////////////////////
//
// HumdrumToken::setStrandIndex -- Sets the 1-D strand index
//    that the token belongs to in the owning HumdrumFile.
//    By default the strand index is set to -1 when a HumdrumToken
//    is created.
//

void  HumdrumToken::setStrandIndex(int index) {
	strand = index;
}



//////////////////////////////
//
// HumdrumToken::incrementState -- update the rhythm analysis state variable.
//    This will prevent redundant recursive analysis in analyzeRhythm of
//    the HumdrumFileStructure class.
//

void HumdrumToken::incrementState(void) {
	rhycheck++;
}



//////////////////////////////
//
// HumdrumToken::getNextTokenCount -- Returns the number of tokens in the
//   spine/sub spine which follow this token.  Typically this will be 1,
//   but will be zero for a terminator interpretation (*-), and will be
//   2 for a split interpretation (*^).
//

int HumdrumToken::getNextTokenCount(void) const {
	return nextTokens.size();
}



//////////////////////////////
//
// HumdrumToken::getPreviousTokenCount -- Returns the number of tokens
//   in the spine/sub-spine which precede this token.  Typically this will
//   be 1, but will be zero for an exclusive interpretation (starting with
//   "**"), and will be greater than one for a token which follows a
//   spine merger (using *v interpretations).
//

int HumdrumToken::getPreviousTokenCount(void) const {
	return previousTokens.size();
}



//////////////////////////////
//
// HumdrumToken::printCsv -- print token in CSV format.
// default value: out = std::cout
//

ostream& HumdrumToken::printCsv(ostream& out) {
	string& value = *this;
	int loc = this->find(",");
	if (loc == string::npos) {
		out << value;
	} else {
		out << '"';
		for (int i=0; i<value.size(); i++) {
		   if (value[i] == '"') {
				out << '"' << '"';
			} else {
				out << value[i];
			}
		}
		out << '"';
	}
	return out;
}



//////////////////////////////
//
// HumdrumToken::printXml -- Print a HumdrumToken in XML format.
// default value: out = cout
// default value: level = 0
// default value: indent = "\t"
//
//

ostream& HumdrumToken::printXml(ostream& out, int level, const string& indent) {
	out << Convert::repeatString(indent, level);
	out << "<field";
	out << " n=\"" << getTokenIndex() << "\"";

	out << " track=\"" << getTrack() << "\"";
	if (getSubtrack() > 0) {
		out << " subtrack=\"" << getSubtrack() << "\"";
	}
	out << " token=\"" << Convert::encodeXml(((string)(*this))) << "\"";
	out << " xml:id=\"" << getXmlId() << "\"";
	out << ">\n";

	printXmlBaseInfo(out, level+1, indent);
	printXmlStructureInfo(out, level+1, indent);

	if (isData()) {
		if (isNote()) {
			out << Convert::repeatString(indent, level+1) << "<pitch";
			out << Convert::getKernPitchAttributes(((string)(*this)));
			out << "/>\n";
		}
	}

	printXmlContentInfo(out, level+1, indent);
	printXmlParameterInfo(out, level+1, indent);
	out << Convert::repeatString(indent, level) << "</field>\n";
	return out;
}



//////////////////////////////
//
// HumdrumToken::printXmlBaseInfo -- print data type and spine info.
// default value: out = cout
// default value: level = 0
// default value: indent = "\t"
//

ostream& HumdrumToken::printXmlBaseInfo(ostream& out, int level,
		const string& indent) {

	// <dataType> redundant with
	// sequence/sequenceInfo/trackInfo/track@dataType
	out << Convert::repeatString(indent, level);
	out << "<dataType>" << getDataType().substr(2) << "</dataType>\n";

	out << Convert::repeatString(indent, level) << "<tokenType>";
	if (isNull()) {
		out << "null";
	} else if (isManipulator()) {
		out << "manipulator";
	} else if (isCommentLocal()) {
		out << "local-comment";
	} else if (isBarline()) {
		out << "barline";
	} else if (isData()) {
		out << "data";
	} else {
		out << "interpretation";
	}
	out << "</tokenType>\n";

	// <tokenFunction>
	if (isDataType("**kern")) {
		if (isNote()) {
			out << Convert::repeatString(indent, level) << "<tokenFunction>";
			out << "note" << "</tokenFunction>\n";
		} else if (isRest()) {
			out << Convert::repeatString(indent, level) << "<tokenFunction>";
			out << "note" << "</tokenFunction>\n";
		}
	}

	if (isNull()) {
		HumdrumToken* previous = getPreviousNonNullDataToken(0);
		if (previous != NULL) {
			out << Convert::repeatString(indent, level) << "<nullResolve";
			out << " text=\"";
			out << Convert::encodeXml(((string)(*previous))) << "\"";
			out << " idref=\"";
			out << previous->getXmlId();
			out << "\"/>\n";
		}
	}

	return out;
}



//////////////////////////////
//
// HumdrumToken::printXmlStructureInfo -- Prints structural information
//    other than spine analysis.
// default value: out = cout
// default value: level = 0
// default value: indent = "\t"
//

ostream& HumdrumToken::printXmlStructureInfo(ostream& out, int level,
		const string& indent) {

	if (getDuration().isNonNegative()) {
		out << Convert::repeatString(indent, level);
		out << "<duration" << Convert::getHumNumAttributes(getDuration());
		out << "/>\n";
	}

	return out;
}



//////////////////////////////
//
// HumdrumToken::printXmlContentInfo -- Print content analysis information.
// default value: out = cout
// default value: level = 0
// default value: indent = "\t"
//

ostream& HumdrumToken::printXmlContentInfo(ostream& out, int level,
		const string& indent) {
	if (hasSlurStart()) {
		out << Convert::repeatString(indent, level) << "<slur";
		if (isDefined("auto", "hangingSlur")) {
			out << " hanging=\"" << getValue("auto", "hangingSlur") << "\"";
		}
		out << ">" << endl; 
		out << Convert::repeatString(indent, level+1);
		out << "<duration" << Convert::getHumNumAttributes(getSlurDuration());
		out << "/>\n";
		out << Convert::repeatString(indent, level) << "</slur>" << endl; 
	}
	return out;
}



//////////////////////////////
//
// HumdrumToken::printXmlParameterInfo -- Print contents of HumHash for token.
// default value: out = cout
// default value: level = 0
// default value: indent = "\t"
//

ostream& HumdrumToken::printXmlParameterInfo(ostream& out, int level,
		const string& indent) {
	((HumHash)(*this)).printXml(out, level, indent);
	return out;
}



//////////////////////////////
//
// HumdrumToken::getXmlId -- Returns an XML id attribute based on the line
//     and field index for the location of the token in the HumdrumFile.
//     An optional parameter for a prefix can be given.  If this parameter
//     is an empty string, then the prefix set in the owning HumdrumFile
//     will instead be used.  The prefix cannot start with a digit, and
//     should not include a space charcter.
//

string HumdrumToken::getXmlId(const string& prefix) const {
	string output;
	if (prefix.size() > 0) {
		output = prefix;
	} else {
		output = getXmlIdPrefix();
	}
	output += "loc" + to_string(getLineIndex()) + "_";
	output += to_string(getFieldIndex());
	// subtoken IDS could be added here.
	return output;
}



//////////////////////////////
//
// HumdrumToken::getXmlIdPrefix -- Returns the XML ID prefix from the HumdrumFile
//   structure via the HumdrumLine on which the token resides.
//

string HumdrumToken::getXmlIdPrefix(void) const {
	auto own = getOwner();
	if (own == NULL) {
		return "";
	}
	return own->getXmlIdPrefix();
}



//////////////////////////////
//
// operator<< -- Needed to avoid interaction with the HumHash parent class.
//

ostream& operator<<(ostream& out, const HumdrumToken& token) {
	out << token.c_str();
	return out;
}


ostream& operator<<(ostream& out, HumdrumToken* token) {
	out << token->c_str();
	return out;
}



//////////////////////////////
//
// printSequence --
//    default value: out = cout;
//

ostream& printSequence(vector<vector<HTp> >& sequence, ostream& out) {
	for (int i=0; i<sequence.size(); i++) {
		for (int j=0; j<sequence[i].size(); j++) {
			out << sequence[i][j];
			if (j < sequence[i].size() - 1) {
				out << '\t';
			}
		}
		out << endl;
	}
	return out;
}


ostream& printSequence(vector<HTp>& sequence, ostream& out) {
	for (int i=0; i<sequence.size(); i++) {
		out << sequence[i] << endl;
	}
	return out;
}



// END_MERGE

} // end namespace std;



