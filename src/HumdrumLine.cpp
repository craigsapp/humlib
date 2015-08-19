//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Aug  8 12:24:49 PDT 2015
// Last Modified: Sun Aug  9 21:59:46 PDT 2015
// Filename:      Convert.h
// URL:           https://github.com/craigsapp/minHumdrum/blob/master/src/HumLine.h
// Syntax:        C++11
// vim:           ts=3 noexpandtab
//
// Description:   Used to store Humdrum text lines and analytic markup
//                of the line.
//

#include <sstream>

#include "HumdrumLine.h"
#include "HumdrumFile.h"
#include "Convert.h"

namespace minHumdrum {

// START_MERGE

//////////////////////////////
//
// HumdrumLine::HumdrumLine -- HumdrumLine constructor.
//

HumdrumLine::HumdrumLine(void) : string() {
	owner = NULL;
	duration = -1;
	durationFromStart = -1;
	setPrefix("!!");
}

HumdrumLine::HumdrumLine(const string& aString) : string(aString) {
	owner = NULL;
	duration = -1;
	durationFromStart = -1;
	setPrefix("!!");
}

HumdrumLine::HumdrumLine(const char* aString) : string(aString) {
	owner = NULL;
	duration = -1;
	durationFromStart = -1;
	setPrefix("!!");
}



//////////////////////////////
//
// HumdrumLine::~HumdrumLine -- HumdrumLine deconstructor.
//

HumdrumLine::~HumdrumLine() {
	// free stored HumdrumTokens:
	clear();
}



//////////////////////////////
//
// HumdrumLine::clear -- Remove stored tokens.
//

void HumdrumLine::clear(void) {
	for (int i=0; i<tokens.size(); i++) {
		delete tokens[i];
		tokens[i] = NULL;
	}
}



//////////////////////////////
//
// HumdrumLine::equalChar -- return true if the character at the given
//     index is the given char.
//

bool HumdrumLine::equalChar(int index, char ch) const {
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
// HumdrumLine::isComment -- Returns true if the first character
//   in the string is '!'. Could be local, global, or a reference record.
//

bool HumdrumLine::isComment(void) const {
	return equalChar(0, '!');
}

bool HumdrumLine::isCommentLocal(void) const {
	return equalChar(0, '!') && !equalChar(1, '!');

}

bool HumdrumLine::isCommentGlobal(void) const {
	return equalChar(0, '!') && equalChar(1, '!');
}



//////////////////////////////
//
// HumdrumLine::isExclusive -- Returns true if the first two characters
//     are "**".
//

bool HumdrumLine::isExclusive(void) const {
	return equalChar(1, '*') && equalChar(0, '*');
}



//////////////////////////////
//
// HumdrumLine::isTerminator -- Returns true if first two characters
//    are "*-" and the next character is undefined or a tab character.
//    Not a very sophisticated function and may need adjustment later.
//    Better to search for a terminator in each token.
//

bool HumdrumLine::isTerminator(void) const {
	return equalChar(1, '!') && equalChar(0, '*');
}



//////////////////////////////
//
// HumdrumLine::isInterp -- Returns true if starts with '*' character.
//

bool HumdrumLine::isInterp(void) const {
	return equalChar(0, '*');
}



//////////////////////////////
//
// HumdrumLine::isBarline -- Returns true if starts with '=' character.
//

bool HumdrumLine::isBarline(void) const {
	return equalChar(0, '=');
}



//////////////////////////////
//
// HumdrumLine::isData -- Returns true if data (but not measure).
//

bool HumdrumLine::isData(void) const {
	if (isComment() || isInterp() || isBarline() || isEmpty()) {
		return false;
	} else {
		return true;
	}
}



//////////////////////////////
//
// HumdrumLine::isAllNull -- Returns true if all tokens on the line
//		are null ("." if a data line, "*" if an interpretation line, "!"
//		if a local comment line).
//

bool HumdrumLine::isAllNull(void) const {
	if (!hasSpines()) {
		return false;
	}
	for (int i=0; i<getTokenCount(); i++) {
		if (!token(i).isNull()) {
			return false;
		}
	}
	return true;
}



//////////////////////////////
//
// HumdrumLine::isAllRhythmicNull -- Returns true if all rhythmic
//    data-type tokens on the line are null ("." if a data line,
//    "*" if an interpretation line, "!" if a local comment line).
//

bool HumdrumLine::isAllRhythmicNull(void) const {
	if (!hasSpines()) {
		return false;
	}
	for (int i=0; i<getTokenCount(); i++) {
		if (!token(i).hasRhythm()) {
			continue;
		}
		if (!token(i).isNull()) {
			return false;
		}
	}
	return true;
}



//////////////////////////////
//
// HumdrumLine::setLineIndex -- Used by the HumdrumFileBase class to set the
//   index number of the line in the data storage for the file.
//

void HumdrumLine::setLineIndex(int index) {
	lineindex = index;
}



//////////////////////////////
//
// HumdrumLine::getLineIndex -- Returns the index number of the line in the
//    HumdrumFileBase storage for the lines.
//

int HumdrumLine::getLineIndex(void) const {
	return lineindex;
}



//////////////////////////////
//
// HumdrumLine::getLineNumber -- Return the line index plus one.
//

int HumdrumLine::getLineNumber(void) const {
	return lineindex + 1;
}



//////////////////////////////
//
// HumdrumLine::getDuration -- Get the duration of the line.  The duration will
//    be negative one if rhythmic analysis in HumdrumFileStructure has not been
//    done on the owning HumdrumFile object.  Otherwise this is the duration of
//    the current line in the file.
//

HumNum HumdrumLine::getDuration(void) const {
	return duration;
}



//////////////////////////////
//
// HumdrumLine::setDurationFromStart -- Set the duration from the start of the
//    file to the start of the current line.  This is used in rhythmic
//    analysis done in the HumdrumFileStructure class.
//

void HumdrumLine::setDurationFromStart(HumNum dur) {
	durationFromStart = dur;
}



//////////////////////////////
//
// HumdrumLine::getDurationFromStart -- Get the duration from the start of the
//    file to the start of the current line.  This will be -1 if rhythmic
//    analysis has not been done in the HumdrumFileStructure class.
//

HumNum HumdrumLine::getDurationFromStart(void) const {
	return durationFromStart;
}



//////////////////////////////
//
// HumdrumLine::getDurationToEnd -- Return the duration from the start of the
//    line to the end of the HumdrumFile which owns this HumdrumLine.  The
//    rhythm of the HumdrumFile must be analyze before using this function;
//    otherwise a 0 will probably be returned.
//

HumNum HumdrumLine::getDurationToEnd(void) const {
	if (owner == NULL) {
		return 0;
	}
	return ((HumdrumFile*)owner)->getScoreDuration() -  durationFromStart;
}



//////////////////////////////
//
// HumdrumLine::getDurationFromBarline -- Return the duration from the start
//    of the given line to the first barline occurring before the given line.
//    Analysis of this data is found in HumdrumFileStructure::metricAnalysis.
//

HumNum HumdrumLine::getDurationFromBarline(void) const {
	return durationFromBarline;
}



//////////////////////////////
//
// HumdrumLine::getTrackStart --  Return the starting exclusive interpretation
//    for the given spine/track.
//

HumdrumToken* HumdrumLine::getTrackStart(int track) const {
	if (owner == NULL) {
		return NULL;
	} else {
		return ((HumdrumFile*)owner)->getTrackStart(track);
	}
}



//////////////////////////////
//
// HumdrumLine::setDurationFromBarline -- Time from the previous
//    barline to the current line.  This function is used in analyzeMeter in
//    the HumdrumFileStructure class.
//

void HumdrumLine::setDurationFromBarline(HumNum dur) {
	durationFromBarline = dur;
}



//////////////////////////////
//
// HumdrumLine::getDurationToBarline -- Time from the starting of the
//     current note to the next barline.
//

HumNum HumdrumLine::getDurationToBarline(void) const {
	return durationToBarline;
}



//////////////////////////////
//
// HumdrumLine::getBeat -- return the beat number for the data on the
//     current line given the input **recip representation for the duration
//     of a beat.  The beat in a measure is offset from 1 (first beat is
//     1 rather than 0).
//  Default value: beatrecip = "4".
//  Default value: beatdur   = 1.
//

HumNum HumdrumLine::getBeat(HumNum beatdur) const {
	if (beatdur.isZero()) {
		return beatdur;
	}
	HumNum beat = (getDurationFromBarline() / beatdur) + 1;
	return beat;
}


HumNum HumdrumLine::getBeat(string beatrecip) const {
	HumNum beatdur = Convert::recipToDuration(beatrecip);
	if (beatdur.isZero()) {
		return beatdur;
	}
	HumNum beat = (getDurationFromBarline() / beatdur) + 1;
	return beat;
}



//////////////////////////////
//
// HumdrumLine::setDurationToBarline -- Set the duration from the current
//     line to the next barline in the score.  This function is used by
//     analyzeMeter in the HumdrumFileStructure class.
//

void HumdrumLine::setDurationToBarline(HumNum dur) {
	durationToBarline = dur;
}



//////////////////////////////
//
// HumdrumLine::setDuration -- Set the duration of the line.  This is done
//   in the rhythmic analysis for the HumdurmFileStructure class.
//

void HumdrumLine::setDuration(HumNum aDur) {
	if (aDur.isNonNegative()) {
		duration = aDur;
	} else {
		duration = 0;
	}
}



//////////////////////////////
//
// HumdrumLine::hasSpines -- Returns true if the line contains spines.  This
//   means the the line is not empty or a global comment (which can include
//   reference records.
//

bool HumdrumLine::hasSpines(void) const {
	if (isEmpty() || isCommentGlobal()) {
		return false;
	} else {
		return true;
	}
}



//////////////////////////////
//
// HumdrumLine::isManipulator -- Returns true if any tokens on the line are
//   manipulator interpretations.  Only null interpretations are allowed on
//   lines which contain manipulators, but the parser currently does not
//   enforce this rule.
//

bool HumdrumLine::isManipulator(void) const {
	for (int i=0; i<tokens.size(); i++) {
		if (tokens[i]->isManipulator()) {
			return true;
		}
	}
	return false;
}



//////////////////////////////
//
// HumdrumLine::isEmpty -- Returns true if no characters on line.  A blank line
//   is technically disallowed in the classic Humdrum Toolkit programs, but it
//   is usually tolerated.  In minHumdrum (and HumdrumExtras) empty lines with
//   no content (not even space characters) are allowed and treated as a
//   special class of line.
//

bool HumdrumLine::isEmpty(void) const {
	if (size() == 0) {
		return true;
	} else {
		return false;
	}
}



//////////////////////////////
//
// HumdrumLine::getTokenCount --  Return the number of tokens on the line.  This
//     value is set by HumdrumFileBase in analyzeTokens.
//

int HumdrumLine::getTokenCount(void) const {
	return tokens.size();
}



//////////////////////////////
//
// HumdrumLine::token -- Return a reference to the given token on the line.
//    An invalid token index would be bad to give to this function as it
//    returns a reference rather than a pointer (which could be set to
//    NULL if invalid).  Perhaps this function will eventually throw an
//    error if the index is out of bounds.
//

HumdrumToken& HumdrumLine::token(int index) const {
	return *tokens[index];
}



//////////////////////////////
//
// HumdrumLine::getTokenString -- Return a copy of the string component of
//     a token.  This code will return a segmentation fault if index is out of
//     range...
//

string HumdrumLine::getTokenString(int index) const {
	return (string(*tokens[index]));
}


//////////////////////////////
//
// HumdrumLine::createTokensFromLine -- Chop up a HumdrumLine string into
//     individual tokens.
//

int HumdrumLine::createTokensFromLine(void) {
	tokens.resize(0);
	HumdrumToken* token = new HumdrumToken();
	token->setOwner(this);
	char ch;
	for (int i=0; i<size(); i++) {
		ch = getChar(i);
		if (ch == '\t') {
			tokens.push_back(token);
			token = new HumdrumToken();
			token->setOwner(this);
		} else {
			*token += ch;
		}
	}
	tokens.push_back(token);
	return tokens.size();
}



//////////////////////////////
//
// HumdrumLine::createLineFromTokens --  Re-generate a HumdrumLine string from
//    individual tokens on the line.  This function will be necessary to
//    run before printing a HumdrumFile if you have changed any tokens on the
//    line.  Otherwise, changes in the tokens will not be passed on to the
///   printing of the line.
//

void HumdrumLine::createLineFromTokens(void) {
	string& iline = *this;
	iline.resize(0);
	for (int i=0; i<tokens.size(); i++) {
		iline += (string)(*tokens[i]);
		if (i < tokens.size() - 1) {
			iline += '\t';
		}
	}
}



//////////////////////////////
//
// HumdrumLine::getTokens -- Returns an array of tokens pointers for a
//   Humdrum line.  This function should not be called on global comments,
//   reference records (which are a sub-cateogry of global comments).  This
//   is because a line's type may contain tabs which are not representing
//   token separators.  Empty lines are ok to input: the output token
//   list will contain one empty string.
//

void HumdrumLine::getTokens(vector<HumdrumToken*>& list) {
	if (tokens.size() == 0) {
		createTokensFromLine();
	}
	list = tokens;
}



//////////////////////////////
//
// HumdrumLine::getChar -- Return character at given index in string, or
//    null if out of range.
//

char HumdrumLine::getChar(int index) const {
	if (index < 0) {
		return '\0';
	}
	if (index >= size()) {
		return '\0';
	}
	return (((string)(*this))[index]);
}



//////////////////////////////
//
// HumdrumLine::printSpineInfo -- Print the spine state information of
//    each token in a file.  Useful for debugging.  The spine info
//    is the track number, such as "1".  When the track splits into
//    subtracks, then there will be two subtracks: "(1)a" and "(1)b".
//    If the second of those subtracks splits again, then its subtracks
//    will be "((1)b)a" and "((1)b)b". If two different tracks merge, such
//    as "1" and "(2)a", then the spine info will be "1 (2)a".
//
//    default value: out = cout
//

ostream& HumdrumLine::printSpineInfo(ostream& out) {
	if (isManipulator()) {
		out << *this;
	} else {
		for (int i=0; i<tokens.size(); i++) {
			out << tokens[i]->getSpineInfo();
			if (i < tokens.size() - 1) {
				out << '\t';
			}
		}
	}
	return out;
}



//////////////////////////////
//
// HumdrumLine::printDataTypeInfo -- Print the datatype of each token in
//     the file.  Useful for debugging.  The datatype prefix "**" is removed;
//     otherwise, it is given when you call HumdrumToken::getDataType().
//
//     default value: out = cout
//

ostream& HumdrumLine::printDataTypeInfo(ostream& out) {
	if (isManipulator()) {
		out << *this;
	} else {
		for (int i=0; i<tokens.size(); i++) {
			out << tokens[i]->getDataType().substr(2, string::npos);
			if (i < tokens.size() - 1) {
				out << '\t';
			}
		}
	}
	return out;
}



//////////////////////////////
//
// HumdrumLine::analyzeTokenDurations -- Calculate the duration of
//    all tokens on a line.
//

bool HumdrumLine::analyzeTokenDurations(void) {
	if (!hasSpines()) {
		return true;
	}
	for (int i=0; i<tokens.size(); i++) {
		if (!tokens[i]->analyzeDuration()) {
			return false;
		}
	}
	return true;
}



//////////////////////////////
//
// HumdrumLine::analyzeTracks -- Calculate the subtrack info for subspines.
//   Subtracks index subspines strictly from left to right on the line.
//   Subspines can be exchanged and be represented left to right out of
//   original order.
//

bool HumdrumLine::analyzeTracks(void) {
	if (!hasSpines()) {
		return true;
	}

	string info;
	int track;
	int maxtrack = 0;
	int i, j, k;

	for (i=0; i<tokens.size(); i++) {
		info = tokens[i]->getSpineInfo();
		track = 0;
		for (j=0; j<info.size(); j++) {
			if (!isdigit(info[j])) {
				continue;
			}
			track = info[j] - '0';
			for (k=j+1; k<info.size(); k++) {
				if (isdigit(info[k])) {
					track = track * 10 + (info[k] - '0');
				} else {
					break;
				}
			}
			break;
		}
		if (maxtrack < track) {
			maxtrack = track;
		}
		tokens[i]->setTrack(track);
	}

	int subtrack;
	vector<int> subtracks;
	vector<int> cursub;

	subtracks.resize(maxtrack+1);
	cursub.resize(maxtrack+1);
	fill(subtracks.begin(), subtracks.end(), 0);
	fill(cursub.begin(), cursub.end(), 0);

	for (i=0; i<tokens.size(); i++) {
		subtracks[tokens[i]->getTrack()]++;
	}
	for (i=0; i<tokens.size(); i++) {
		subtrack = subtracks[tokens[i]->getTrack()];
		if (subtrack > 1) {
			tokens[i]->setSubtrack(++cursub[tokens[i]->getTrack()]);
		} else {
			tokens[i]->setSubtrack(0);
		}
	}
	return true;
}



//////////////////////////////
//
// HumdrumLine::printDurationInfo -- Print the analyzed duration of each
//     token in a file (for debugging).  If a token has an undefined
//     duration, then its duration is -1.  If a token represents
//     a grace note, then its duration is 0 (regardless of whether it
//     includes a visual duration).
//     default value: out = cout
//

ostream& HumdrumLine::printDurationInfo(ostream& out) {
	if (isManipulator()) {
		out << *this;
	} else {
		for (int i=0; i<tokens.size(); i++) {
			tokens[i]->getDuration().printMixedFraction(out);
			if (i < tokens.size() - 1) {
				out << '\t';
			}
		}
	}
	return out;
}



//////////////////////////////
//
// HumdrumLine::printTrackInfo -- Print the analyzed track information.
//     The first (left-most) spine in a Humdrum file is track 1, the
//     next is track 2, etc.  The track value is shared by all subspines,
//     so there may be duplicate track numbers on a line if the spine
//     has split.  When the spine splits, a subtrack number is given
//     after a "." character in the printed output from this function.
//     Subtrack==0 means that there is only one subtrack.
//     Examples:
//         "1"  == Track 1, subtrack 1 (and there are no more subtracks)
//	        "1.1" == Track 1, subtrack 1 (and there are more subtracks)
//	        "1.2" == Track 1, subtrack 2 (and there may be more subtracks)
//	        "1.10" == Track 1, subtrack 10 (and there may be subtracks)
//     Each starting exclusive interpretation is assigned to a unique
//     track number.  When a *+ manipulator is given, the new exclusive
//     interpretation on the next line is give the next higher track
//     number.
//
//     default value: out = cout
//

ostream& HumdrumLine::printTrackInfo(ostream& out) {
	if (isManipulator()) {
		out << *this;
	} else {
		for (int i=0; i<tokens.size(); i++) {
			out << tokens[i]->getTrackString();
			if (i < tokens.size() - 1) {
				out << '\t';
			}
		}
	}
	return out;
}



//////////////////////////////
//
// HumdrumLine::setOwner -- store a pointer to the HumdrumFile which
//    manages (owns) this object.
//

void HumdrumLine::setOwner(void* hfile) {
	owner = hfile;
}



//////////////////////////////
//
// HumdrumLine::setParameters -- Takes a global comment with
//     the structure:
//        !!NS1:NS2:key1=value1:key2=value2:key3=value3
//     and stores it in the HumHash parent class of the line.
//

void HumdrumLine::setParameters(HumdrumLine* pLine) {
	HumdrumLine& pl = *pLine;
	if (pl.size() <= 2) {
		return;
	}
	string pdata = pLine->substr(2, pl.size()-2);
	setParameters(pdata);
}


void HumdrumLine::setParameters(const string& pdata) {
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
// operator<< -- Print a HumdrumLine. Needed to avoid interaction with
//     HumHash parent class.
//

ostream& operator<<(ostream& out, HumdrumLine& line) {
	out << (string)line;
	return out;
}



// END_MERGE

} // end namespace std;



