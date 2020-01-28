//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Aug  8 12:24:49 PDT 2015
// Last Modified: Sun Aug  9 21:03:12 PDT 2015
// Filename:      HumdrumToken.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/HumdrumToken.cpp
// Syntax:        C++11; humlib
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Used to work with tokens on a Humdrum line.
//

#include "HumAddress.h"
#include "HumdrumToken.h"
#include "HumdrumLine.h"
#include "HumdrumFile.h"
#include "Convert.h"
#include "HumRegex.h"

#include "string.h"

using namespace std;

namespace hum {

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
	m_rhycheck = 0;
	setPrefix("!");
	m_strand = -1;
	m_nullresolve = NULL;
}


HumdrumToken::HumdrumToken(const string& aString) : string(aString) {
	m_rhycheck = 0;
	setPrefix("!");
	m_strand = -1;
	m_nullresolve = NULL;
}


HumdrumToken::HumdrumToken(const char* aString) : string(aString) {
	m_rhycheck = 0;
	setPrefix("!");
	m_strand = -1;
	m_nullresolve = NULL;
}


HumdrumToken::HumdrumToken(const HumdrumToken& token) :
		string((string)token), HumHash((HumHash)token) {
	m_address         = token.m_address;
	m_address.m_owner = NULL;
	m_duration        = token.m_duration;
	m_nextTokens      = token.m_nextTokens;
	m_previousTokens.clear();
	m_nextNonNullTokens.clear();
	m_previousNonNullTokens.clear();
	m_rhycheck        = token.m_rhycheck;
	m_strand          = -1;
	m_nullresolve     = NULL;
	setPrefix(token.getPrefix());
}


HumdrumToken::HumdrumToken(HumdrumToken* token) :
		string((string)(*token)), HumHash((HumHash)(*token)) {
	m_address         = token->m_address;
	m_address.m_owner = NULL;
	m_duration        = token->m_duration;
	m_nextTokens      = token->m_nextTokens;
	m_previousTokens.clear();
	m_nextNonNullTokens.clear();
	m_previousNonNullTokens.clear();
	m_rhycheck        = token->m_rhycheck;
	m_strand          = -1;
	m_nullresolve     = NULL;
	setPrefix(token->getPrefix());
}



HumdrumToken::HumdrumToken(const HumdrumToken& token, HumdrumLine* owner) :
		string((string)token), HumHash((HumHash)token) {
	m_address         = token.m_address;
	m_address.m_owner = owner;
	m_duration        = token.m_duration;
	m_nextTokens      = token.m_nextTokens;
	m_previousTokens.clear();
	m_nextNonNullTokens.clear();
	m_previousNonNullTokens.clear();
	m_rhycheck        = token.m_rhycheck;
	m_strand          = -1;
	m_nullresolve     = NULL;
	setPrefix(token.getPrefix());
}


HumdrumToken::HumdrumToken(HumdrumToken* token, HumdrumLine* owner) :
		string((string)(*token)), HumHash((HumHash)(*token)) {
	m_address         = token->m_address;
	m_address.m_owner = owner;
	m_duration        = token->m_duration;
	m_nextTokens      = token->m_nextTokens;
	m_previousTokens.clear();
	m_nextNonNullTokens.clear();
	m_previousNonNullTokens.clear();
	m_rhycheck        = token->m_rhycheck;
	m_strand          = -1;
	m_nullresolve     = NULL;
	setPrefix(token->getPrefix());
}



//////////////////////////////
//
// HumdrumToken::operator= -- Copy operator.
//

HumdrumToken& HumdrumToken::operator=(HumdrumToken& token) {
	if (this == &token) {
		return *this;
	}
	(string)(*this)   = (string)token;
	(HumHash)(*this)  = (HumHash)token;

	m_address         = token.m_address;
	m_address.m_owner = NULL;
	m_duration        = token.m_duration;
	m_nextTokens      = token.m_nextTokens;
	m_previousTokens.clear();
	m_nextNonNullTokens.clear();
	m_previousNonNullTokens.clear();
	m_rhycheck        = token.m_rhycheck;
	m_strand          = -1;
	m_nullresolve     = NULL;
	setPrefix(token.getPrefix());

	return *this;
}


HumdrumToken& HumdrumToken::operator=(const string& token) {
	(string)(*this) = token;

	m_address.m_owner = NULL;
	m_duration        = 0;
	m_nextTokens.clear();
	m_previousTokens.clear();
	m_nextNonNullTokens.clear();
	m_previousNonNullTokens.clear();
	m_rhycheck        = -1;
	m_strand          = -1;
	m_nullresolve     = NULL;
	setPrefix("!");

	return *this;
}


HumdrumToken& HumdrumToken::operator=(const char* token) {
	(string)(*this) = token;

	m_address.m_owner = NULL;
	m_duration        = 0;
	m_nextTokens.clear();
	m_previousTokens.clear();
	m_nextNonNullTokens.clear();
	m_previousNonNullTokens.clear();
	m_rhycheck        = -1;
	m_strand          = -1;
	m_nullresolve     = NULL;
	setPrefix("!");

	return *this;
}



//////////////////////////////
//
// HumdrumToken::~HumdrumToken -- Deconstructor for HumdrumToken.
//

HumdrumToken::~HumdrumToken() {
	if (m_linkedParameter) {
		delete m_linkedParameter;
		m_linkedParameter = NULL;
	}
}


//////////////////////////////
//
// HumdrumToken::equalChar -- Returns true if the character at the given
//     index is the given char.
//

bool HumdrumToken::equalChar(int index, char ch) const {
	if ((int)size() <= index) {
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
	return (int)m_previousNonNullTokens.size();
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
		index += (int)m_previousNonNullTokens.size();
	}
	if (index < 0) {
		return NULL;
	}
	if (index >= (int)m_previousNonNullTokens.size()) {
		return NULL;
	}
	return m_previousNonNullTokens[index];
}



//////////////////////////////
//
// HumdrumToken::getNextNonNullDataTokenCount -- Returns the number of non-null
//     data tokens which follow this token in the spine.
//

int HumdrumToken::getNextNonNullDataTokenCount(void) {
	return (int)m_nextNonNullTokens.size();
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
		index += (int)m_nextNonNullTokens.size();
	}
	if (index < 0) {
		return NULL;
	}
	if (index >= (int)m_nextNonNullTokens.size()) {
		return NULL;
	}
	return m_nextNonNullTokens[index];
}



//////////////////////////////
//
// HumdrumToken::getSlurDuration -- If the note has a slur start, then
//    returns the duration until the endpoint; otherwise, returns 0;
//    Expand later to handle slur ends and elided slurs.  The function
//    HumdrumFileContent::analyzeSlurs() should be called before accessing
//    this function.  If the slur duruation was already calculated, return
//    that value; otherwise, calculate from the location of a matching
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
	return m_address.getDataType();
}



//////////////////////////////
//
// HumdrumToken::isDataType -- Returns true if the data type of the token
//   matches the test data type.
// @SEEALSO: getDataType getKern
//

bool HumdrumToken::isDataType(const string& dtype) const {
	if (dtype.compare(0, 2, "**") == 0) {
		return dtype == getDataType();
	} else {
		return getDataType().compare(2, string::npos, dtype) == 0;
	}
}



//////////////////////////////
//
// HumdrumToken::isKern -- Returns true if the data type of the token
//    is **kern.
// @SEEALSO: isDataType
//

bool HumdrumToken::isKern(void) const {
	return isDataType("**kern");
}



//////////////////////////////
//
// HumdrumToken::isMens -- Returns true if the data type of the token
//    is **mens.
// @SEEALSO: isDataType
//

bool HumdrumToken::isMens(void) const {
	return isDataType("**mens");
}



//////////////////////////////
//
// HumdrumToken::setSpineInfo -- Sets the spine manipulation history string.
// @SEEALTO: getSpineInfo
//

void HumdrumToken::setSpineInfo(const string& spineinfo) {
	m_address.setSpineInfo(spineinfo);
}



//////////////////////////////
//
// HumdrumToken::getSpineInfo -- Returns the spine split/merge history
//    for the token.
// @SEEALTO: setSpineInfo
//

string HumdrumToken::getSpineInfo(void) const {
	return m_address.getSpineInfo();
}



//////////////////////////////
//
// HumdrumToken::getLineIndex -- Returns the line index of the owning
//    HumdrumLine for this token.
// @SEEALTO: getLineNumber
//

int HumdrumToken::getLineIndex(void) const {
	return m_address.getLineIndex();
}



//////////////////////////////
//
// HumdrumToken::getFieldIndex -- Returns the index of the token the line.
// @SEEALSO: getFieldIndex
//

int HumdrumToken::getFieldIndex(void) const {
	return m_address.getFieldIndex();
}



//////////////////////////////
//
// HumdrumToken::getFieldNumber -- Returns the index of the token the line.
// @SEEALSO: getFieldNumber
//

int HumdrumToken::getFieldNumber(void) const {
	return m_address.getFieldIndex() + 1;
}



//////////////////////////////
//
// HumdrumToken::getTokenIndex -- Returns the index of the token the line.
// @SEEALSO: getTokenIndex
//

int HumdrumToken::getTokenIndex(void) const {
	return m_address.getFieldIndex();
}



//////////////////////////////
//
// HumdrumToken::getTokenNumber -- Returns the index of the token the line.
// @SEEALSO: getFieldNumber
//

int HumdrumToken::getTokenNumber(void) const {
	return m_address.getFieldIndex() + 1;
}



//////////////////////////////
//
// HumdrumToken::getLineNumber -- Returns the line index plus 1.
// @SEEALTO: getLineIndex
//

int HumdrumToken::getLineNumber(void) const {
	return m_address.getLineNumber();
}



//////////////////////////////
//
// HumdrumToken::setFieldIndex -- Sets the field index of the token on the
//   owning HumdrumLine object.
// @SEEALSO: getFieldIndex
//

void HumdrumToken::setFieldIndex(int index) {
	m_address.setFieldIndex(index);
}



//////////////////////////////
//
// HumdrumToken::setTrack -- Sets the track number (similar to a staff in MEI).
//     The two-parameter version will set the track and sub-track at the same
//     time (subtrack is similar to a staff and layer in MEI).
//

void HumdrumToken::setTrack(int aTrack) {
	m_address.setTrack(aTrack);
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
	return m_address.getTrack();
}



//////////////////////////////
//
// HumdrumToken::setSubtrack -- Sets the subtrack (similar to a layer
//    in MEI).
//

void HumdrumToken::setSubtrack(int aSubtrack) {
	m_address.setSubtrack(aSubtrack);
}



//////////////////////////////
//
// HumdrumToken::setSubtrackCount -- Sets the subtrack count in the
//    HumdrumLine for all tokens in the same track as the current
//    token.
//

void HumdrumToken::setSubtrackCount(int count) {
	m_address.setSubtrackCount(count);
}



//////////////////////////////
//
// HumdrumToken::setPreviousToken --
//

void HumdrumToken::setPreviousToken(HumdrumToken* token) {
	m_previousTokens.resize(1);
	m_previousTokens[0] = token;
}



//////////////////////////////
//
// HumdrumToken::setNextToken --
//

void HumdrumToken::setNextToken(HumdrumToken* token) {
	m_nextTokens.resize(1);
	m_nextTokens[0] = token;
}



//////////////////////////////
//
// HumdrumToken::addNextNonNullToken --
//

void HumdrumToken::addNextNonNullToken(HTp token) {
	if (token == NULL) {
		return;
	}
	for (int i=0; i<(int)m_nextNonNullTokens.size(); i++) {
		if (token == m_nextNonNullTokens[i]) {
			return;
		}
	}
	m_nextNonNullTokens.push_back(token);
	// maybe should sort by track/subspine order...
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

HTp HumdrumToken::getNextToken(int index) const {
	if ((index >= 0) && (index < (int)m_nextTokens.size())) {
		return m_nextTokens[index];
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
	return m_nextTokens;
}



//////////////////////////////
//
// HumdrumToken::getPreviousTokens -- Returns a list of the previous
//    tokens in the spine before this token.
//

vector<HumdrumToken*> HumdrumToken::getPreviousTokens(void) const {
	return m_previousTokens;
}



//////////////////////////////
//
// HumdrumToken::getPreviousToken -- Returns the previous token in the
//    spine.  Since the previous token count is usually one, the default
//    index value is zero.
// default value: index = 0
//

HumdrumToken* HumdrumToken::getPreviousToken(int index) const {
	if ((index >= 0) && (index < (int)m_previousTokens.size())) {
		return m_previousTokens[index];
	} else {
		return NULL;
	}
}


//////////////////////////////
//
// HumdrumToken::getNextFieldToken --
//

HTp HumdrumToken::getNextFieldToken(void) const {
	HumdrumLine* line = getLine();
	if (!line) {
		return NULL;
	}
	int field = getFieldIndex();
	if (field >= line->getFieldCount()  - 1) {
		return NULL;
	}
	return line->token(field+1);
}



//////////////////////////////
//
// HumdrumToken::getPreviousFieldToken --
//

HTp HumdrumToken::getPreviousFieldToken(void) const {
	HumdrumLine* line = getLine();
	if (!line) {
		return NULL;
	}
	int field = getFieldIndex();
	if (field < 1) {
		return NULL;
	}
	return line->token(field-1);
}



//////////////////////////////
//
// HumdrumToken::analyzeDuration -- Currently reads the duration of
//   **kern and **recip data.  Add more data types here such as **koto.
//

bool HumdrumToken::analyzeDuration(void) {
	m_rhythm_analyzed = true;
	if ((*this) == NULL_DATA) {
		m_duration.setValue(-1);
		return true;
	}
	if (equalChar(0 ,'!')) {
		m_duration.setValue(-1);
		return true;
	}
	if (equalChar(0 ,'*')) {
		m_duration.setValue(-1);
		return true;
	}
	if (equalChar(0 ,'=')) {
		m_duration.setValue(-1);
		return true;
	}
	string dtype = getDataType();
	if (hasRhythm()) {
		if (isData()) {
			if (!isNull()) {
				if (isKern()) {
					if (strchr(this->c_str(), 'q') != NULL) {
						m_duration = 0;
					} else {
						m_duration = Convert::recipToDuration((string)(*this));
					}
				} else if (isMens()) {
					m_duration = Convert::mensToDuration((string)(*this));
				}
			} else {
				m_duration.setValue(-1);
			}
		} else {
			m_duration.setValue(-1);
		}

	} else {
		m_duration.setValue(-1);
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

HumNum HumdrumToken::getDuration(void) {
	if (!m_rhythm_analyzed) {
		analyzeDuration();
	}
	return m_duration;
}


HumNum HumdrumToken::getDuration(HumNum scale) {
	return m_duration * scale;
}



//////////////////////////////
//
// HumdrumToken::getTiedDuration -- Returns the duration of the token and any
//    tied notes attached to it.  Does not work well which chords.  Does
//    not work well with secondary spine splits.
//

HumNum HumdrumToken::getTiedDuration(void) {
	HumNum output = m_duration;
	// start of a tied group so add the durations of the other notes.
	int b40 = Convert::kernToBase40(this);
	HTp note = this;
	HTp nnote = NULL;
	while (note) {
		nnote = note->getNextNNDT();
		if (!nnote) {
			break;
		}
		if (!nnote->isSecondaryTiedNote()) {
			break;
		}
		int nb40 = Convert::kernToBase40(this);
		if (nb40 != b40) {
			break;
		}
		// note is tied to previous one, so add its curation to output.
		output += nnote->getDuration();
		note = nnote;
	}

	return output;
}


HumNum HumdrumToken::getTiedDuration(HumNum scale) {
	return getTiedDuration() * scale;
}



//////////////////////////////
//
// HumdrumToken::getDots -- Count the number of '.' characters in token string.
//    Terminating the count at the first occurrence of the separator character,
//    which is by default a space character.
//

int HumdrumToken::getDots(char separator) const {
	int count = 0;
	for (int i=0; i<(int)this->size()-1; i++) {
		if (this->at(i) == '.') {
			count++;
		}
		if (this->at(i) == separator) {
			break;
		}
	}
	return count;
}



//////////////////////////////
//
// HumdrumToken::getDurationNoDots -- Return the duration of the
//   note excluding any dots.
//

HumNum HumdrumToken::getDurationNoDots(void) {

	int dots = getDots();
	if (dots == 0) {
		return getDuration();
	}
	int bot = (int)pow(2.0, dots + 1) - 1;
	int top = (int)pow(2.0, dots);
	HumNum factor(top, bot);
	return getDuration() * factor;

}


HumNum HumdrumToken::getDurationNoDots(HumNum scale) {
	int dots = getDots();
	if (dots == 0) {
		return getDuration(scale);
	}
	int top = (int)pow(2.0, dots + 1) - 1;
	int bot = (int)pow(2.0, dots);
	HumNum factor(top, bot);
	return getDuration(scale) * factor;
}



//////////////////////////////
//
// HumdrumToken::setDuration -- Sets the duration of the token.  This is done in
//    HumdrumFileStructure::analyzeTokenDurations().
//

void HumdrumToken::setDuration(const HumNum& dur) {
	m_duration = dur;
}



//////////////////////////////
//
// HumdrumToken::getDurationFromStart -- Returns the duration from the
//   start of the owning HumdrumFile to the starting time of the
//   owning HumdrumLine for the token.  The durationFromStart is
//   in reference to the start of the token, not the end of the token,
//   which may be on another HumdrumLine.
//

HumNum HumdrumToken::getDurationFromStart(void) {
	return getLine()->getDurationFromStart();
}


HumNum HumdrumToken::getDurationFromStart(HumNum scale) {
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

HumNum HumdrumToken::getDurationToEnd(void) {
	return getLine()->getDurationToEnd();
}


HumNum HumdrumToken::getDurationToEnd(HumNum scale) {
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

HumNum HumdrumToken::getBarlineDuration(void) {
	HumdrumLine* own = getOwner();
	if (own == NULL) {
		return 0;
	}
	return own->getBarlineDuration();
}


HumNum HumdrumToken::getBarlineDuration(HumNum scale) {
	HumdrumLine* own = getOwner();
	if (own == NULL) {
		return 0;
	}
	return own->getBarlineDuration(scale);
}



//////////////////////////////
//
// HumdrumToken::getDurationToBarline -- Get duration from start of token to
//      the start of the next barline. Units are quarter notes, unless scale
//      is set to a value other than 1.
//

HumNum HumdrumToken::getDurationToBarline(void) {
	HumdrumLine* own = getOwner();
	if (own == NULL) {
		return 0;
	}
	return own->getDurationToBarline();
}

HumNum HumdrumToken::getDurationToBarline(HumNum scale) {
	HumdrumLine* own = getOwner();
	if (own == NULL) {
		return 0;
	}
	return own->getDurationToBarline(scale);
}



//////////////////////////////
//
// HumdrumToken::getDurationFromBarline -- Get duration from start of token to
//      the previous barline. Units are quarter notes, unless scale
//      is set to a value other than 1.
//

HumNum HumdrumToken::getDurationFromBarline(void) {
	HumdrumLine* own = getOwner();
	if (own == NULL) {
		return 0;
	}
	return own->getDurationFromBarline();
}

HumNum HumdrumToken::getDurationFromBarline(HumNum scale) {
	HumdrumLine* own = getOwner();
	if (own == NULL) {
		return 0;
	}
	return own->getDurationFromBarline(scale);
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
	if (type == "**mens") {
		return true;
	}
	return false;
}



//////////////////////////////
//
// HumdrumToken::hasBeam -- True if **kern has L, J, K, or k.
//

bool HumdrumToken::hasBeam(void) const {
	for (int i=0; i<(int)this->size(); i++) {
		switch (this->at(i)) {
			case 'L':
			case 'J':
			case 'k':
			case 'K':
				return true;
		}
	}
	return false;
}



//////////////////////////////
//
// HumdrumToken::hasFermata --
//

bool HumdrumToken::hasFermata(void) const {
	if (this->find(';') != string::npos) {
		return true;
	} else {
		return false;
	}
}



//////////////////////////////
//
// HumdrumToken::equalTo --
//

bool HumdrumToken::equalTo(const string& pattern) {
	if ((string)(*this) == pattern) {
		return true;
	} else {
		return false;
	}
}



//////////////////////////////
//
// HumdrumToken::isRest -- Returns true if the token is a (kern) rest.
//

bool HumdrumToken::isRest(void) {
	if (isKern()) {
		if (isNull() && Convert::isKernRest((string)(*resolveNull()))) {
			return true;
		} else if (Convert::isKernRest((string)(*this))) {
			return true;
		}
	} else if (isMens()) {
		if (isNull() && Convert::isMensRest((string)(*resolveNull()))) {
			return true;
		} else if (Convert::isMensRest((string)(*this))) {
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

bool HumdrumToken::isNote(void) {
	if (isKern()) {
		if (Convert::isKernNote((string)(*this))) {
			return true;
		}
	} else if (isMens()) {
		if (Convert::isMensNote((string)(*this))) {
			return true;
		}
	}
	return false;
}



//////////////////////////////
//
// HumdrumToken::isSustainedNote -- Returns true if the token represents
//     a sounding note, but not the attack portion.  Should only be
//     applied to **kern data.
//

bool HumdrumToken::isSustainedNote(void) {
	HTp token = this;
	if (isNull()) {
		token = resolveNull();
	}
	return token->isSecondaryTiedNote();
}



//////////////////////////////
//
// HumdrumToken::isNoteAttack -- Returns true if the token represents
//     the attack of a note.  Should only be applied to **kern data.
//

bool HumdrumToken::isNoteAttack(void) {
	HTp token = this;
	if (isNull()) {
		token = resolveNull();
	}
	if (token->isRest()) {
		return false;
	}
	return !token->isSecondaryTiedNote();
}



//////////////////////////////
//
// HumdrumToken::isInvisible -- True if a barline and is invisible (contains
//     a "-" styling), or a note/rest contains the string "yy" which is
//     interpreted as meaning make it invisible.
//
//

bool HumdrumToken::isInvisible(void) {
	if (!isDataType("**kern")) {
			return false;
	}
	if (isBarline()) {
		if (find("-") != string::npos) {
			return true;
		}
	} else if (isData()) {
		if (find("yy") != string::npos) {
			return true;
		}
	}

	return false;
}



//////////////////////////////
//
// HumdrumToken::isGrace -- True if a **kern note has no duration.
//

bool HumdrumToken::isGrace(void) {
	if (!isDataType("**kern")) {
			return false;
	}
	if (!isData()) {
		return false;
	} else if (this->find("q") != string::npos) {
		return true;
	}

	return false;
}



//////////////////////////////
//
// HumdrumToken::isClef -- True if a **kern clef.
//

bool HumdrumToken::isClef(void) {
	if (!(isDataType("**kern") || isDataType("**mens"))) {
			return false;
	}
	if (!isInterpretation()) {
		return false;
	} else if (this->compare(0, 5, "*clef") == 0) {
		return true;
	}

	return false;
}



//////////////////////////////
//
// HumdrumToken::isKeySignature -- True if a **kern key signature.
//

bool HumdrumToken::isKeySignature(void) {
	if (this->compare(0, 3, "*k[") != 0) {
		return false;
	}
	if (this->back() != ']') {
		return false;
	}
	return true;
}



//////////////////////////////
//
// HumdrumToken::isKeyDesignation -- True if a **kern key designation.
//   *C:
//   *A-:
//   *c#:
//   *d:dor
//

bool HumdrumToken::isKeyDesignation(void) {
	if (this->size() < 3) {
		return false;
	}
	if (this->find(":") == string::npos) {
		return false;
	}
	char diatonic = (*this)[1];

	if ((diatonic >= 'A') && (diatonic <= 'G')) {
		return true;
	}
	if ((diatonic >= 'a') && (diatonic <= 'g')) {
		return true;
	}
	return false;
}



//////////////////////////////
//
// HumdrumToken::isTimeSignature -- True if a **kern time signature.
//

bool HumdrumToken::isTimeSignature(void) {
	if (this->size() < 3) {
		return false;
	}
	if (this->compare(0, 2, "*M") != 0) {
		return false;
	}
	if (!isdigit((*this)[2])) {
		return false;
	}
	if (this->find("/") == string::npos) {
		return false;
	}
	return true;
}



//////////////////////////////
//
// HumdrumToken::isTempo -- True if a **kern tempo.
//

bool HumdrumToken::isTempo(void) {
	if (this->size() < 4) {
		return false;
	}
	if (this->compare(0, 3, "*MM") != 0) {
		return false;
	}
	if (!isdigit((*this)[3])) {
		return false;
	}
	return true;
}



//////////////////////////////
//
// HumdrumToken::isMensurationSymbol -- True if a **kern mensuration Symbol.
//

bool HumdrumToken::isMensurationSymbol(void) {
	if (this->compare(0, 5, "*met(") != 0) {
		return false;
	}
	if ((*this)[this->size()-1] != ')') {
		return false;
	}
	return true;
}



//////////////////////////////
//
// HumdrumToken::isInstrumentDesignation -- Such as *Iclars for B-flat clarinet.
//

bool HumdrumToken::isInstrumentDesignation(void) {
	if (this->compare(0, 2, "*I") != 0) {
		return false;
	}
	for (int i=2; i<(int)this->size(); i++) {
		if (!isalpha(this->at(i))) {
			return false;
		}
		if (!islower(this->at(i))) {
			return false;
		}
	}

	return true;
}



//////////////////////////////
//
// HumdrumToken::isInstrumentName -- True if an instrument name token.
//

bool HumdrumToken::isInstrumentName(void) {
	if (this->compare(0, 3, "*I\"") != 0) {
		return false;
	} else {
		return true;
	}
}



//////////////////////////////
//
// HumdrumToken::isInstrumentAbbreviation -- True if an instrument abbreviation token.
//

bool HumdrumToken::isInstrumentAbbreviation(void) {
	if (this->compare(0, 3, "*I'") != 0) {
		return false;
	} else {
		return true;
	}
}



//////////////////////////////
//
// HumdrumToken::hasSlurStart -- Returns true if the **kern token has
//     a '(' character.
//

bool HumdrumToken::hasSlurStart(void) {
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

bool HumdrumToken::hasSlurEnd(void) {
	if (isDataType("**kern")) {
		if (Convert::hasKernSlurEnd((string)(*this))) {
			return true;
		}
	}
	return false;
}



//////////////////////////////
//
// HumdrumToken::hasVisibleAccidental -- Returns true if the accidental
//    of a **kern note is viewable if rendered to graphical notation.
// 	return values:
//      0  = false;
//      1  = true;
//      -1 = undefined;
//

int HumdrumToken::hasVisibleAccidental(int subtokenIndex) const {
	HumdrumLine* humrec = getOwner();
	if (humrec == NULL) {
		return -1;
	}
	HumdrumFile* humfile = humrec->getOwner();
	if (humfile == NULL) {
		return -1;
	}
	if (!humfile->getValueBool("auto", "accidentalAnalysis")) {
		int status = humfile->analyzeKernAccidentals();
		if (!status) {
			return -1;
		}
	}
	return getValueBool("auto", to_string(subtokenIndex), "visualAccidental");
}



//////////////////////////////
//
// HumdrumToken::hasVisibleAccidental -- Returns true if the accidental
//    of a **kern note is viewable if rendered to graphical notation.
// 	return values:
//      0  = false;
//      1  = true;
//      -1 = undefined;
//

int HumdrumToken::hasCautionaryAccidental(int subtokenIndex) const {
	HumdrumLine* humrec = getOwner();
	if (humrec == NULL) {
		return -1;
	}
	HumdrumFile* humfile = humrec->getOwner();
	if (humfile == NULL) {
		return -1;
	}
	if (!humfile->getValueBool("auto", "accidentalAnalysis")) {
		int status = humfile->analyzeKernAccidentals();
		if (!status) {
			return -1;
		}
	}
	return getValueBool("auto", to_string(subtokenIndex), "cautionaryAccidental");
}



//////////////////////////////
//
// HumdrumToken::hasLigatureBegin --
//

bool HumdrumToken::hasLigatureBegin(void) {
	if (isMens()) {
		return Convert::hasLigatureBegin(*this);
	} else {
		return false;
	}
}



//////////////////////////////
//
// HumdrumToken::hasRectaLigatureBegin --
//

bool HumdrumToken::hasRectaLigatureBegin(void) {
	if (isMens()) {
		return Convert::hasRectaLigatureBegin(*this);
	} else {
		return false;
	}
}



//////////////////////////////
//
// HumdrumToken::hasObliquaLigatureBegin --
//

bool HumdrumToken::hasObliquaLigatureBegin(void) {
	if (isMens()) {
		return Convert::hasObliquaLigatureBegin(*this);
	} else {
		return false;
	}
}



//////////////////////////////
//
// HumdrumToken::hasStemDirection --
//

char HumdrumToken::hasStemDirection(void) {
	if (isKern()) {
		return Convert::hasKernStemDirection(*this);
	} else {
		// don't know what a stem in this datatype is
		return '\0';
	}
}


//////////////////////////////
//
// HumdrumToken::hasLigatureEnd --
//

bool HumdrumToken::hasLigatureEnd(void) {
	if (isMens()) {
		return Convert::hasLigatureEnd(*this);
	} else {
		return false;
	}
}



//////////////////////////////
//
// HumdrumToken::hasRectaLigatureEnd --
//

bool HumdrumToken::hasRectaLigatureEnd(void) {
	if (isMens()) {
		return Convert::hasRectaLigatureEnd(*this);
	} else {
		return false;
	}
}



//////////////////////////////
//
// HumdrumToken::hasObliquaLigatureEnd --
//

bool HumdrumToken::hasObliquaLigatureEnd(void) {
	if (isMens()) {
		return Convert::hasObliquaLigatureEnd(*this);
	} else {
		return false;
	}
}



//////////////////////////////
//
// HumdrumToken::isSecondaryTiedNote -- Returns true if the token
//     is a (kern) note (possessing a pitch) and has '_' or ']' characters.
//

bool HumdrumToken::isSecondaryTiedNote(void) {
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
// HumdrumToken::isCommentGlobal -- Returns true of the token starts with "!!".
//    Currently confused with reference records.
//

bool HumdrumToken::isCommentGlobal(void) const {
	if (size() == 0) {
		return false;
	}
	if ((*this)[0] == '!') {
		if (size() > 1) {
			if ((*this)[1] == '!') {
				// global comment
				return true;
			}
		}
	}
	return false;
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
// HumdrumToken::isInterpretation -- Returns true if an interpretation.
//

bool HumdrumToken::isInterpretation(void) const {
	if (size() == 0) {
		return false;
	}
	int firstchar = (*this)[0];
	if (firstchar == '*') {
		return true;
	}
	return false;
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
// HumdrumToken::isLabel -- Returns true if a thru label (such as *>A).
//

bool HumdrumToken::isLabel(void) const {
	if (string::compare(0, 2, "*>") != 0) {
		return false;
	}
	if (string::find("[") != string::npos) {
		return false;
	}
	return true;
}



/////////////////////////////
//
// HumdrumToken::isChord -- True if is a chord.  Presuming you know what
//     data type you are accessing.
//     Default value:
//          separate = " "   (**kern note separator)
//

bool HumdrumToken::isChord(const string& separator) {
	return (this->find(separator) != string::npos) ? true : false;
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
	if ((void*)this == NULL) {
		// This was added perhaps due to a new bug [20100125] that is checking a null pointer
		return false;
	}
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
	return m_address.getSubtrack();
}



//////////////////////////////
//
// HumdrumToken::noteInLowerSubtrack -- Return true if the note
//     is attacked or sustained with another note in a lower layer.
//     This is for using in hum2mei conversion to avoid a bug in
//     verovio related to lyrics in layers where the notes are a
//     second apart.
//

bool HumdrumToken::noteInLowerSubtrack(void) {
	int subtrack = this->getSubtrack();
	if (subtrack <= 1) {
		return false;
	}
	int field = this->getFieldIndex();
	int track = this->getTrack();

	HumdrumLine* owner = this->getOwner();
	if (owner == NULL) {
		return false;
	}

	for (int i=field-1; i>=0; i--) {
		HTp xtoken = owner->token(i);
		int xtrack = xtoken->getTrack();
		if (xtrack != track) {
			return false;
		}
		if (xtoken->isNull()) {
			continue;
		}
		if (xtoken->find("r") != string::npos) {
			continue;
		}
		return true;
	}

	return false;
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
	return m_address.getTrackString();
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
	return count+1;
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

	string output;
	const string& token = *this;
	if (separator.size() == 0) {
		output = token[index];
		return output;
	}

	int count = 0;
	for (int i=0; i<(int)size(); i++) {
		if (string::compare(i, separator.size(), separator) == 0) {
			count++;
			if (count > index) {
				break;
			}
			i += (int)separator.size() - 1;
		} else if (count == index) {
			output += token[i];
		}
	}
	return output;
}



//////////////////////////////
//
// HumdrumToken::getSubtokens -- Return the list of subtokens as an array
//     of strings.
//     default value: separator = " "
//

std::vector<std::string> HumdrumToken::getSubtokens (const std::string& separator) const {
	std::vector<std::string> output;
	const string& token = *this;
	HumRegex hre;
	hre.split(output, token, separator);
	return output;
}



//////////////////////////////
//
// HumdrumToken::replaceSubtoken --
//     default value: separator = " "
//

void HumdrumToken::replaceSubtoken(int index, const std::string& newsubtok,
		const std::string& separator) {
	if (index < 0) {
		return;
	}
	std::vector<std::string> subtokens = getSubtokens(separator);
	if (index >= (int)subtokens.size()) {
		return;
	}
	subtokens[index] = newsubtok;
	string output;
	for (int i=0; i<(int)subtokens.size(); i++) {
		output += subtokens[i];
		if (i < (int)subtokens.size() - 1) {
			output += separator;
		}
	}
	this->setText(output);
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
	for (int i=2; i<(int)pieces.size(); i++) {
		Convert::replaceOccurrences(pieces[i], "&colon;", ":");
		loc = (int)pieces[i].find("=");
		if (loc != (int)string::npos) {
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
// HumdrumToken::setText --
//

void HumdrumToken::setText(const string& text) {
	string::assign(text);
}



//////////////////////////////
//
// HumdrumToken::getText --
//

string HumdrumToken::getText(void) const {
	return string(*this);
}



//////////////////////////////
//
// HumdrumToken::addLinkedParamter --
//

int HumdrumToken::addLinkedParameter(HTp token) {
	if (token->find(":ignore") != string::npos) {
		// Ignore layout command (store layout command but
		// do not use it.  This is particularly for adding
		// layout parameters for notation, but the parameters
		// currently cause problems in verovio (so they should
		// be unignored at a future date when the layout
		// parameter is handled better).  Note that any
		// parameter starting with "ignore" such as "ignored"
		// will also be suppressed by this if statement.
		return -1;
	}
	for (int i=0; i<(int)m_linkedParameters.size(); i++) {
		if (m_linkedParameters[i] == token) {
			return i;
		}
	}

	if (m_linkedParameters.empty()) {
		m_linkedParameters.push_back(token);
	} else {
		int lineindex = token->getLineIndex();
		if (lineindex >= m_linkedParameters.back()->getLineIndex()) {
			m_linkedParameters.push_back(token);
		} else {
			// Store sorted by line number
			for (auto it = m_linkedParameters.begin(); it != m_linkedParameters.end(); it++) {
				if (lineindex < (*it)->getLineIndex()) {
					m_linkedParameters.insert(it, token);
					break;
				}
			}
		}

	}

	return (int)m_linkedParameters.size() - 1;
}



//////////////////////////////
//
// HumdrumToken::linkedParameterIsGlobal --
//

bool HumdrumToken::linkedParameterIsGlobal(int index) {
	return m_linkedParameters.at(index)->isCommentGlobal();
}



//////////////////////////////
//
// HumdrumToken::getLinkedParameterCount --
//

int HumdrumToken::getLinkedParameterCount(void) {
	return (int)m_linkedParameters.size();
}



//////////////////////////////
//
// HumdrumToken::getLinkedParameter--
//

HumParamSet* HumdrumToken::getLinkedParameter(void) {
	return m_linkedParameter;
}


HumParamSet* HumdrumToken::getLinkedParameter(int index) {
	return m_linkedParameters.at(index)->getLinkedParameter();
}



//////////////////////////////
//
// HumdrumToken::storeLinkedParameters -- Store the contents of the token
//    in the linked parameter storage.  Used for layout parameters.
//

void HumdrumToken::storeLinkedParameters(void) {
	if (m_linkedParameter) {
		delete m_linkedParameter;
	}
	m_linkedParameter = new HumParamSet(*((string*)this));
}



//////////////////////////////
//
// HumdrumToken::makeForwardLink -- Line a following spine token to this one.
//    Used by the HumdrumFileBase::analyzeLinks function.
//

void HumdrumToken::makeForwardLink(HumdrumToken& nextToken) {
	m_nextTokens.push_back(&nextToken);
	nextToken.m_previousTokens.push_back(this);
}



//////////////////////////////
//
// HumdrumToken::makeBackwarddLink -- Link a previous spine token to this one.
//    Used by the HumdrumFileBase::analyzeLinks function.
//

void HumdrumToken::makeBackwardLink(HumdrumToken& previousToken) {
	m_previousTokens.push_back(&previousToken);
	previousToken.m_nextTokens.push_back(this);
}



//////////////////////////////
//
// HumdrumToken::getVisualDuration -- Returns LO:N:vis parameter if it is attached
//    to a token directly or indirectly through a linked parameter.  Returns empty string
//    if no explicit visual durtation (so the visual duration is same as the logical duration).
//

string HumdrumToken::getVisualDuration(int subtokenindex) {
	// direct storage of the layout parameter is possible, but currently disabled:
	//string parameter = this->getValue("LO", "N", "vis");
	//if (!parameter.empty()) {
	//	return parameter;
	//}
	return this->getLayoutParameter("N", "vis", subtokenindex);
}



//////////////////////////////
//
// HumdrumToken::getVisualDurationChord -- only return the chord-level visual duration
//    parameter (not if it is specific to certain note(s) in the chord).
//

string HumdrumToken::getVisualDurationChord(void) {
	return this->getLayoutParameterChord("N", "vis");
}



//////////////////////////////
//
// HumdrumToken::getVisualDurationNote -- only return the note-level visual duration
//    parameter (not if it is general to the entire chord.
//

string HumdrumToken::getVisualDurationNote(int subtokenindex) {
	return this->getLayoutParameterNote("N", "vis", subtokenindex);
}



//////////////////////////////
//
// HumdrumToken::getLayoutParameter -- Returns requested layout parameter
//     if it is attached to a token directly or indirectly through a linked
//     parameter.  Returns empty string if no explicit visual durtation (so
//     the visual duration is same as the logical duration).  If subtokenindex
//     is less than -1 (the default value for the paramter), then ignore the
//     @n parameter control for indexing the layout parameter to chord notes.
//     The subtokenindex (0 indexed) is converted to note number (1 indexed)
//     for checking @n.  @n is currently only allowed to be a single integer
//     (eventually allow ranges and multiple values).
//

std::string HumdrumToken::getLayoutParameter(const std::string& category,
		const std::string& keyname, int subtokenindex) {

	// First check for any local layout parameter:
	std::string testoutput = this->getValue("LO", category, keyname);
	if (!testoutput.empty()) {
		if (subtokenindex >= 0) {
			int n = this->getValueInt("LO", category, "n");
			if (n == subtokenindex + 1) {
				return testoutput;
			}
		} else {
			return testoutput;
		}
	}

	std::string output;
	int lcount = this->getLinkedParameterCount();
	if (lcount == 0) {
		return output;
	}

	std::string nparam;
	for (int p = 0; p < this->getLinkedParameterCount(); ++p) {
		hum::HumParamSet *hps = this->getLinkedParameter(p);
		if (hps == NULL) {
			continue;
		}
		if (hps->getNamespace1() != "LO") {
			continue;
		}
		if (hps->getNamespace2() != category) {
			continue;
		}

		output = "";
		for (int q = 0; q < hps->getCount(); ++q) {
			string key = hps->getParameterName(q);
			if (key == keyname) {
				output = hps->getParameterValue(q);
				if (subtokenindex < 0) {
					return output;
				}
			}
			if (key == "n") {
				nparam = hps->getParameterValue(q);
			}
		}
		if (nparam.empty()) {
			// No subtoken selection for this parameter,
			// so return if not empty:
			if (!output.empty()) {
				return output;
			}
		} else if (subtokenindex < 0) {
			// No subtoken selection so return output if not empty
			if (!output.empty()) {
				return output;
			}
		} else {
			// There is a subtoken selection number, so
			// return output if n matches it (minus one)

			// currently @n requires a single value
			// (should allow a range or multiple values
			// later).  Also not checking validity of
			// string first (needs to start with a digit);

			int n = stoi(nparam);
			if (n == subtokenindex + 1) {
				return output;
			} else {
				// not the output that is required,
				// so suppress for end of loop:
				output = "";
			}
		}
	}

	return output;
}



//////////////////////////////
//
// HumdrumToken::getSlurLayoutParameter --
//

std::string HumdrumToken::getSlurLayoutParameter(const std::string& keyname,
		int subtokenindex) {
	std::string category = "S";
	std::string output;

	// First check for any local layout parameter:
	std::string testoutput = this->getValue("LO", category, keyname);
	if (!testoutput.empty()) {
		if (subtokenindex >= 0) {
			int s = this->getValueInt("LO", category, "s");
			if (s == subtokenindex + 1) {
				return testoutput;
			}
		} else {
			return testoutput;
		}
	}

	int lcount = this->getLinkedParameterCount();
	if (lcount == 0) {
		return output;
	}

	std::string sparam;
	for (int p = 0; p < this->getLinkedParameterCount(); ++p) {
		hum::HumParamSet *hps = this->getLinkedParameter(p);
		if (hps == NULL) {
			continue;
		}
		if (hps->getNamespace1() != "LO") {
			continue;
		}
		if (hps->getNamespace2() != category) {
			continue;
		}
		for (int q = 0; q < hps->getCount(); ++q) {
			string key = hps->getParameterName(q);
			if (key == "s") {
				sparam = hps->getParameterValue(q);
			}
			if (key == keyname) {
				output = hps->getParameterValue(q);
			}
		}
	}
	if (subtokenindex < 0) {
		// do not filter by s parameter
		return output;
	} else if (sparam.empty()) {
		// parameter is not qualified by a note number, so applies to whole token
		return output;
	}

	// currently @s requires a single value (should allow a range or multiple values later)
	// also not checking validity of string first (needs to start with a digit);
	int s = stoi(sparam);
	if (s == subtokenindex + 1) {
		return output;
	} else {
		return "";
	}
}



//////////////////////////////
//
// HumdrumToken::getPhraseLayoutParameter --
//

std::string HumdrumToken::getPhraseLayoutParameter(const std::string& keyname,
		int subtokenindex) {
	std::string category = "P";
	std::string output;

	// First check for any local layout parameter:
	std::string testoutput = this->getValue("LO", category, keyname);
	if (!testoutput.empty()) {
		if (subtokenindex >= 0) {
			int s = this->getValueInt("LO", category, "s");
			if (s == subtokenindex + 1) {
				return testoutput;
			}
		} else {
			return testoutput;
		}
	}

	int lcount = this->getLinkedParameterCount();
	if (lcount == 0) {
		return output;
	}

	std::string sparam;
	for (int p = 0; p < this->getLinkedParameterCount(); ++p) {
		hum::HumParamSet *hps = this->getLinkedParameter(p);
		if (hps == NULL) {
			continue;
		}
		if (hps->getNamespace1() != "LO") {
			continue;
		}
		if (hps->getNamespace2() != category) {
			continue;
		}
		for (int q = 0; q < hps->getCount(); ++q) {
			string key = hps->getParameterName(q);
			if (key == "s") {
				sparam = hps->getParameterValue(q);
			}
			if (key == keyname) {
				output = hps->getParameterValue(q);
			}
		}
	}
	if (subtokenindex < 0) {
		// do not filter by s parameter
		return output;
	} else if (sparam.empty()) {
		// parameter is not qualified by a note number, so applies to whole token
		return output;
	}

	// currently @s requires a single value (should allow a range or multiple values later)
	// also not checking validity of string first (needs to start with a digit);
	int s = stoi(sparam);
	if (s == subtokenindex + 1) {
		return output;
	} else {
		return "";
	}
}



//////////////////////////////
//
// HumdrumToken::getLayoutParameterChord -- Returns requested layout
// parameter if it is attached to a token directly or indirectly through
// a linked parameter.  The parameter must apply to the entire chord, so
// no @n qualification parameters can be given (even if they include all
// notes in the chord).

std::string HumdrumToken::getLayoutParameterChord(const std::string& category,
		const std::string& keyname) {

	// First check for any local layout parameter:
	std::string testoutput = this->getValue("LO", category, keyname);
	if (!testoutput.empty()) {
		std::string n = this->getValue("LO", category, "n");
		if (n.empty()) {
			return testoutput;
		}
	}

	std::string output;
	int lcount = this->getLinkedParameterCount();
	if (lcount == 0) {
		return output;
	}

	std::string nparam;
	for (int p = 0; p < this->getLinkedParameterCount(); ++p) {
		hum::HumParamSet *hps = this->getLinkedParameter(p);
		if (hps == NULL) {
			continue;
		}
		if (hps->getNamespace1() != "LO") {
			continue;
		}
		if (hps->getNamespace2() != category) {
			continue;
		}
		for (int q = 0; q < hps->getCount(); ++q) {
			string key = hps->getParameterName(q);
			if (key == "n") {
				nparam = hps->getParameterValue(q);
			}
			if (key == keyname) {
				output = hps->getParameterValue(q);
			}
		}
	}

	if (!nparam.empty()) {
		// parameter is qualified by a note number, so does not apply to whole token
		return "";
	} else {
		return output;
	}
}



//////////////////////////////
//
// HumdrumToken::getLayoutParameterNote -- Returns requested layout
//     parameter if it is attached to a token directly or indirectly through a
//     linked parameter.  The parameter must apply to a single note or specific
//     note in a chord.


std::string HumdrumToken::getLayoutParameterNote(const std::string& category,
		const std::string& keyname, int subtokenindex) {

	// First check for any local layout parameter:
	std::string testoutput = this->getValue("LO", category, keyname);
	if (!testoutput.empty()) {
		if (subtokenindex >= 0) {
			int n = this->getValueInt("LO", category, "n");
			if (n == subtokenindex + 1) {
				return testoutput;
			}
		}
	}

	std::string output;
	int lcount = this->getLinkedParameterCount();
	if (lcount == 0) {
		return output;
	}

	std::string nparam;
	for (int p = 0; p < this->getLinkedParameterCount(); ++p) {
		hum::HumParamSet *hps = this->getLinkedParameter(p);
		if (hps == NULL) {
			continue;
		}
		if (hps->getNamespace1() != "LO") {
			continue;
		}
		if (hps->getNamespace2() != category) {
			continue;
		}
		for (int q = 0; q < hps->getCount(); ++q) {
			string key = hps->getParameterName(q);
			if (key == "n") {
				nparam = hps->getParameterValue(q);
			}
			if (key == keyname) {
				output = hps->getParameterValue(q);
			}
		}
	}

	if (!nparam.empty()) {
		// a number number is specified from the parameter(s)
		int n = stoi(nparam);
		if (n == subtokenindex + 1) {
			return output;
		} else {
			// wrong note
			return "";
		}
	}

	if ((subtokenindex < 0) && isChord()) {
		// in chord, and no specific note is selected by @n.
		return "";
	} else {
		// single note, so return parameter:
		return output;
	}
}



//////////////////////////////
//
// HumdrumToken::setOwner -- Sets the HumdrumLine owner of this token.
//

void HumdrumToken::setOwner(HumdrumLine* aLine) {
	m_address.setOwner(aLine);
}



//////////////////////////////
//
// HumdrumToken::getOwner -- Returns a pointer to the HumdrumLine that
//    owns this token.
//

HumdrumLine* HumdrumToken::getOwner(void) const {
	return m_address.getOwner();
}



//////////////////////////////
//
// HumdrumToken::getState -- Returns the rhythm state variable.
//

int HumdrumToken::getState(void) const {
	return m_rhycheck;
}



//////////////////////////////
//
// HumdrumToken::getStrandIndex -- Returns the 1-D strand index
//    that the token belongs to in the owning HumdrumFile.
//    Returns -1 if there is no strand assignment.
//

int  HumdrumToken::getStrandIndex(void) const {
	return m_strand;
}



//////////////////////////////
//
// HumdrumToken::getSlurStartElisionLevel -- Returns the count of
//   elision marks ('&') preceding a slur start character '('.
//   Returns -1 if there is no slur start character.
//   Default value: index = 0
//

int HumdrumToken::getSlurStartElisionLevel(int index) const {
	if (isDataType("**kern") || isDataType("**mens")) {
		return Convert::getKernSlurStartElisionLevel((string)(*this), index);
	} else {
		return -1;
	}
}



//////////////////////////////
//
// HumdrumToken::getPhraseStartElisionLevel -- Returns the count of
//   elision marks ('&') preceding a phrase start character '{'.
//   Returns -1 if there is no phrase start character.
//   Default value: index = 0
//

int HumdrumToken::getPhraseStartElisionLevel(int index) const {
	if (isDataType("**kern") || isDataType("**mens")) {
		return Convert::getKernPhraseStartElisionLevel((string)(*this), index);
	} else {
		return -1;
	}
}



//////////////////////////////
//
// HumdrumToken::getSlurEndElisionLevel -- Returns the count of
//   elision marks ('&') preceding a slur end character ')'.
//   Returns -1 if there is no slur end character.
//   Default value: index = 0
//

int HumdrumToken::getSlurEndElisionLevel(int index) const {
	if (isDataType("**kern") || isDataType("**mens")) {
		return Convert::getKernSlurEndElisionLevel((string)(*this), index);
	} else {
		return -1;
	}
}



//////////////////////////////
//
// HumdrumToken::getPhraseEndElisionLevel -- Returns the count of
//   elision marks ('&') preceding a slur end character '}'.
//   Returns -1 if there is no phrase end character.
//   Default value: index = 0
//

int HumdrumToken::getPhraseEndElisionLevel(int index) const {
	if (isDataType("**kern")) {
		return Convert::getKernPhraseEndElisionLevel((string)(*this), index);
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
	m_strand = index;
}



//////////////////////////////
//
// HumdrumToken::incrementState -- update the rhythm analysis state variable.
//    This will prevent redundant recursive analysis in analyzeRhythm of
//    the HumdrumFileStructure class.
//

void HumdrumToken::incrementState(void) {
	m_rhycheck++;
}



//////////////////////////////
//
// HumdrumToken::getNextTokenCount -- Returns the number of tokens in the
//   spine/sub spine which follow this token.  Typically this will be 1,
//   but will be zero for a terminator interpretation (*-), and will be
//   2 for a split interpretation (*^).
//

int HumdrumToken::getNextTokenCount(void) const {
	return (int)m_nextTokens.size();
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
	return (int)m_previousTokens.size();
}



//////////////////////////////
//
// HumdrumToken::printCsv -- print token in CSV format.
// default value: out = std::cout
//

ostream& HumdrumToken::printCsv(ostream& out) {
	string& value = *this;
	int loc = (int)this->find(",");
	if (loc == (int)string::npos) {
		out << value;
	} else {
		out << '"';
		for (int i=0; i<(int)value.size(); i++) {
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
	printXmlLinkedParameterInfo(out, level+1, indent);
	printXmlLinkedParameters(out, level+1, indent);

	out << Convert::repeatString(indent, level) << "</field>\n";
	return out;
}



//////////////////////////////
//
// HumdrumToken::printXmlLinkedParameters --
//

ostream&	HumdrumToken::printXmlLinkedParameters(ostream& out, int level, const string& indent) {
	if (m_linkedParameter) {
		m_linkedParameter->printXml(out, level, indent);
	}
	return out;
}



//////////////////////////////
//
// HumdrumToken::printXmlLinkedParameterInfo --
//

ostream& HumdrumToken::printXmlLinkedParameterInfo(ostream& out, int level, const string& indent) {
	if (m_linkedParameters.empty()) {
		return out;
	}

	out << Convert::repeatString(indent, level);
	out << "<parameters-linked>\n";

	level++;
	for (int i=0; i<(int)m_linkedParameters.size(); i++) {
		out << Convert::repeatString(indent, level);
		out << "<linked-parameter";
		out << " idref=\"";
		HumdrumLine* owner = m_linkedParameters[i]->getOwner();
		if (owner && owner->isGlobalComment()) {
			out << owner->getXmlId();
		} else {
			out << m_linkedParameters[i]->getXmlId();
		}
		out << "\"";
		out << ">\n";
	}
	level--;

	out << Convert::repeatString(indent, level);
	out << "</parameters-linked>\n";

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
// HumdrumToken::printGlobalXmlParameterInfo --
//

ostream& HumdrumToken::printGlobalXmlParameterInfo(ostream& out, int level, const string& indent) {
	((HumHash*)this)->printXmlAsGlobal(out, level, indent);
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
	((HumHash*)this)->printXml(out, level, indent);
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
	if (token) {
		out << token->c_str();
	} else {
		out << "{NULL}";
	}
	return out;
}



//////////////////////////////
//
// printSequence --
//    default value: out = cout;
//

ostream& printSequence(vector<vector<HTp> >& sequence, ostream& out) {
	for (int i=0; i<(int)sequence.size(); i++) {
		for (int j=0; j<(int)sequence[i].size(); j++) {
			out << sequence[i][j];
			if (j < (int)sequence[i].size() - 1) {
				out << '\t';
			}
		}
		out << endl;
	}
	return out;
}


ostream& printSequence(vector<HTp>& sequence, ostream& out) {
	for (int i=0; i<(int)sequence.size(); i++) {
		out << sequence[i] << endl;
	}
	return out;
}



//////////////////////////////
//
// HumdrumToken::getSlurStartToken -- Return a pointer to the token
//     which starts the given slur.  Returns NULL if no start.  Assumes that
//     HumdrumFileContent::analyzeKernSlurs() has already been run.
//				<parameter key="slurEnd" value="HT_140366146702320" idref=""/>
//

HTp HumdrumToken::getSlurStartToken(int number) {
	string tag = "slurStart";
	if (number > 1) {
		tag += to_string(number);
	}
	return getValueHTp("auto", tag);
}



//////////////////////////////
//
// HumdrumToken::getSlurEndToken -- Return a pointer to the token
//     which ends the given slur.  Returns NULL if no end.  Assumes that
//     HumdrumFileContent::analyzeKernSlurs() has already been run.
//				<parameter key="slurStart" value="HT_140366146702320" idref=""/>
//

HTp HumdrumToken::getSlurEndToken(int number) {
	string tag = "slurEnd";
	if (number > 1) {
		tag += to_string(number);
	}
	return getValueHTp("auto", tag);
}




//////////////////////////////
//
// HumdrumToken::getPhraseStartToken -- Return a pointer to the token
//     which starts the given phrase.  Returns NULL if no start.  Assumes that
//     HumdrumFileContent::analyzeKernPhrasings() has already been run.
//				<parameter key="phraseEnd" value="HT_140366146702320" idref=""/>
//

HTp HumdrumToken::getPhraseStartToken(int number) {
	string tag = "phraseStart";
	if (number > 1) {
		tag += to_string(number);
	}
	return getValueHTp("auto", tag);
}



//////////////////////////////
//
// HumdrumToken::getPhraseEndToken -- Return a pointer to the token
//     which ends the given phrase.  Returns NULL if no end.  Assumes that
//     HumdrumFileContent::analyzeKernPhrasings() has already been run.
//				<parameter key="phraseStart" value="HT_140366146702320" idref=""/>
//

HTp HumdrumToken::getPhraseEndToken(int number) {
	string tag = "phraseEnd";
	if (number > 1) {
		tag += to_string(number);
	}
	return getValueHTp("auto", tag);
}



//////////////////////////////
//
// HumdrumToken::resolveNull --
//

HTp HumdrumToken::resolveNull(void) {
	if (m_nullresolve == NULL) {
		HumdrumLine* hline = getOwner();
		if (hline) {
			HumdrumFile* infile = hline->getOwner();
			infile->resolveNullTokens();
		}
		if (m_nullresolve == NULL) {
			return this;
		} else {
			return m_nullresolve;
		}
		return this;
	} else {
		return m_nullresolve;
	}
}



//////////////////////////////
//
// HumdrumToken::setNullResolution --
//

void HumdrumToken::setNullResolution(HTp resolution) {
	m_nullresolve = resolution;
}



// END_MERGE

} // end namespace hum



