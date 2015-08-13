//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Aug  8 12:24:49 PDT 2015
// Last Modified: Sun Aug  9 21:59:46 PDT 2015
// Filename:      Convert.h
// URL:           https://github.com/craigsapp/minHumdrum/blob/master/src/HumLine.h
// Syntax:        C++11
// vim:           ts=3
//
// Description:   Used to store Humdrum text lines and analytic markup
//                of the line.
//

#include "HumdrumLine.h"
#include "Convert.h"

// START_MERGE

//////////////////////////////
//
// HumdrumLine::HumdrumLine --
//

HumdrumLine::HumdrumLine(void) : string() { }

HumdrumLine::HumdrumLine(const string& aString) : string(aString) { }

HumdrumLine::HumdrumLine(const char* aString) : string(aString) { }



//////////////////////////////
//
// HumdrumLine::HumdrumLine --
//

HumdrumLine::~HumdrumLine() {
	// free stored HumdrumTokens
	clear();
}



//////////////////////////////
//
// HumdrumLine::clear -- remove stored tokens.
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
// HumdrumLine::setLineIndex --
//

void HumdrumLine::setLineIndex(int index) {
   lineindex = index;
}



//////////////////////////////
//
// HumdrumLine::getLineIndex --
//

int HumdrumLine::getLineIndex(void) const {
	return lineindex;
}



//////////////////////////////
//
// HumdrumLine::getLineNumber --
//

int HumdrumLine::getLineNumber(void) const {
	return lineindex + 1;
}



//////////////////////////////
//
// HumdrumLine::getLineNumber --
//

HumNum HumdrumLine::getDuration(void) const { 
	return duration;
}



//////////////////////////////
//
// HumdrumLine::setDurationFromStart --
//

void HumdrumLine::setDurationFromStart(HumNum dur) {
	 durationFromStart = dur;
}



//////////////////////////////
//
// HumdrumLine::getDurationFromStart --
//

HumNum HumdrumLine::getDurationFromStart(void) const {
	return durationFromStart;
}



//////////////////////////////
//
// HumdrumLine::getDurationFromBarline --
//

HumNum HumdrumLine::getDurationFromBarline(void) const { 
	return durationFromBarline;
}



//////////////////////////////
//
// HumdrumLine::setDurationFromBarline -- Time from the previous
//    barline to the current line.
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
//     of a beat.
//  Default value: beatrecip = "4".
//

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
// HumdrumLine::setDurationToBarline --
//

void HumdrumLine::setDurationToBarline(HumNum dur) { 
	durationToBarline = dur;
}



//////////////////////////////
//
// HumdrumLine::getLineNumber --
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
// HumdrumLine::hasSpines --
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
// HumdrumLine::isManipulator --
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
// HumdrumLine::isEmpty -- Returns true if no characters on line.
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
// HumdrumLine::getTokenCount --
//

int HumdrumLine::getTokenCount(void) const {
	return tokens.size();
}



//////////////////////////////
//
// HumdrumLine::token --
//

HumdrumToken& HumdrumLine::token(int index) {
	return *tokens[index];
}



//////////////////////////////
//
// HumdrumLine::getTokenString --
//

string HumdrumLine::getTokenString(int index) const {
	return (string(*tokens[index]));
}


//////////////////////////////
//
// HumdrumLine::createTokensFromLine --
//

int HumdrumLine::createTokensFromLine(void) {
	tokens.resize(0);
	HumdrumToken* token = new HumdrumToken();
	char ch;
	for (int i=0; i<size(); i++) {
		ch = getChar(i);
		if (ch == '\t') {
			tokens.push_back(token);
			token = new HumdrumToken();
		} else {
			*token += ch;
		}
	}
	tokens.push_back(token);
	return tokens.size();
}



//////////////////////////////
//
// HumdrumLine::createLineFromTokens --
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
// HumdrumLine::analyzeTracks --
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
// operator<< -- Print a HumdrumLine.
//

ostream& operator<<(ostream& out, HumdrumLine& line) {
	out << (string)line;
	return out;
}

// END_MERGE



