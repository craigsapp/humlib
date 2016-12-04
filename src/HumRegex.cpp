//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Dec  3 16:21:22 PST 2016
// Last Modified: Sat Dec  3 16:21:25 PST 2016
// Filename:      HumRegex.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/HumRegex.cpp
// Syntax:        C++11
// vim:           ts=3 noexpandtab
//
// Description:   Regular expression handling.
//

#include "HumRegex.h"

using namespace std;

namespace hum {

// START_MERGE

using namespace std::regex_constants;


//////////////////////////////
//
// HumRegex::HumRegex -- Constructor.
//

HumRegex::HumRegex(void) {
  // do nothing
}


HumRegex::HumRegex(const string& exp) {
  // initialize a regular expression for the object
  m_regex = (exp);
}



//////////////////////////////
//
// HumRegex::HumRegex -- Destructor.
//

HumRegex::~HumRegex() {
  // do nothing
}



//////////////////////////////
//
// HumRegex::search -- Search for the regular expression in the
//    input string.  Returns true if any matches were found.  Search
//    results can be accessed with .getSubmatchCount() and .getSubmatch(index).
//

bool HumRegex::search(const string& input, const string& exp) {
	m_regex = exp;
	return regex_search(input, m_matches, m_regex);

}



/////////////////////////////
//
// HumRegex::getMatchCount -- Return the number of submatches that a 
//   previous call to HumRegex::search generated.
//

int HumRegex::getMatchCount(void) {
	return (int)m_matches.size();
}



//////////////////////////////
//
// HumRegex::getMatch -- Returns the given match.  The first match
//   at "0" is the complete match.  The matches with a larger index
//   are the submatches.
//

string HumRegex::getMatch(int index) {
	return m_matches.str(index);
}


//////////////////////////////
//
// HumRegex::getPrefix -- Return the input string text which
//    occurs before the match;
//

string HumRegex::getPrefix(void) {
	return m_matches.prefix().str();
}



//////////////////////////////
//
// HumRegex::getSuffix -- Return the input string text which
//    occurs after the match;
//

string HumRegex::getSuffix(void) {
	return m_matches.suffix().str();
}



//////////////////////////////
//
// HumRegex::getMatchStartIndex -- Get starting index of match in input
//     search string.
//

int HumRegex::getMatchStartIndex(int index) {
	return m_matches.position(index);
}



//////////////////////////////
//
// HumRegex::getMatchEndIndex -- Get ending index of match in input
//     search string.  The index is one larger than the index of the 
//     end of the matched position.
//

int HumRegex::getMatchEndIndex(int index) {
	return getMatchStartIndex(index) + getMatchLength(index);
}



//////////////////////////////
//
// HumRegex::getMatchLength -- Get starting character length of match.
//

int HumRegex::getMatchLength(int index) {
	return m_matches.length(index);
}


// END_MERGE

} // end namespace hum



