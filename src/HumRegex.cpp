// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Dec  3 16:21:22 PST 2016
// Last Modified: Sat Dec  3 16:21:25 PST 2016
// Filename:      HumRegex.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/HumRegex.cpp
// Syntax:        C++11
// vim:           syntax=cpp ts=3 noexpandtab nowrap
// note:          Requires GCC v4.9 or higher
//
// Description:   Regular expression handling.
//
// References:
//     http://www-01.ibm.com/support/docview.wss?uid=swg27041858&aid=1
//     https://msdn.microsoft.com/en-us/library/bb982382.aspx
//     https://msdn.microsoft.com/en-us/library/bb982727.aspx
//

#include "HumRegex.h"

using namespace std;

namespace hum {

// START_MERGE


//////////////////////////////
//
// HumRegex::HumRegex -- Constructor.
//

HumRegex::HumRegex(void) {
	m_flags = std::regex_constants::match_default
				| std::regex_constants::format_first_only;
}


HumRegex::HumRegex(const string& exp) {
	// initialize a regular expression for the object
	m_regex = exp;
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
	return regex_search(input, m_matches, m_regex, m_flags);
}


bool HumRegex::search(const string& input, int startindex,
		const string& exp) {
	m_regex = exp;
	auto startit = input.begin() + startindex;
	auto endit   = input.end();
	return regex_search(startit, endit, m_matches, m_regex, m_flags);
}


bool HumRegex::search(string* input, const string& exp) {
	return HumRegex::search(*input, exp);
}


bool HumRegex::search(string* input, int startindex, const string& exp) {
	return HumRegex::search(*input, startindex, exp);
}

//
// This version of HumRegex allows for setting the options temporarily.
//

bool HumRegex::search(const string& input, const string& exp,
		const string& options) {
	m_regex = exp;
	return regex_search(input, m_matches, m_regex, getTemporaryFlags(options));
}


bool HumRegex::search(const string& input, int startindex, const string& exp,
		const string& options) {
	m_regex = exp;
	auto startit = input.begin() + startindex;
	auto endit   = input.end();
	return regex_search(startit, endit, m_matches, m_regex,
		getTemporaryFlags(options));
}


bool HumRegex::search(string* input, const string& exp,
		const string& options) {
	return HumRegex::search(*input, exp, options);
}


bool HumRegex::search(string* input, int startindex, const string& exp,
		const string& options) {
	return HumRegex::search(*input, startindex, exp, options);
}



//////////////////////////////
//
// HumRegex::getTemporaryFlags --
//

std::regex_constants::match_flag_type HumRegex::getTemporaryFlags(
		const string& sflags) {
	std::regex_constants::match_flag_type temp_flags;
	for (auto it : sflags) {
		switch (it) {
			case 'i':
				temp_flags = (std::regex_constants::match_flag_type)
						(temp_flags | std::regex_constants::icase);
				break;
			case 'I':
				temp_flags = (std::regex_constants::match_flag_type)
						(temp_flags & ~std::regex_constants::icase);
				break;
			case 'g':
				temp_flags = (std::regex_constants::match_flag_type)
						(temp_flags & ~std::regex_constants::format_first_only);
			case 'G':
				temp_flags = (std::regex_constants::match_flag_type)
						(temp_flags | std::regex_constants::format_first_only);
		}
	}
	return temp_flags;
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
// HumRegex::getMatchInt -- Get the match interpreted as a integer.
//

int HumRegex::getMatchInt(int index) {
	return stoi(m_matches.str(index));
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



//////////////////////////////
//
// HumRegex::setIgnoreCase -- Turn on ignore case to be persistent until
//     it is turned off with HumRegex::setNoIgnoreCase(), or if a function
//     call sets it off temporarily.
//

void HumRegex::setIgnoreCase(void) {
	m_flags = (std::regex_constants::match_flag_type)
			(m_flags | std::regex_constants::icase);
}



//////////////////////////////
//
// HumRegex::setNoIgnoreCase -- Turn off persistent ignore case flag.
//

void HumRegex::setNoIgnoreCase(void) {
	m_flags = (std::regex_constants::match_flag_type)
			(m_flags & ~std::regex_constants::icase);
}



//////////////////////////////
//
// HumRegex::replaceDestructive -- Replace in input string.
//

string& HumRegex::replaceDestructive(string& input, const string& exp,
		const string& replacement) {
	m_regex = exp;
	input = regex_replace(input, m_regex, replacement, m_flags);
	return input;
}


string& HumRegex::replaceDestructive(string* input, const string& exp,
		const string& replacement) {
	return HumRegex::replaceDestructive(*input, exp, replacement);
}

//
// This version allows for temporary match flag options.
//

string& HumRegex::replaceDestructive(string& input, const string& exp,
		const string& replacement, const string& options) {
	m_regex = exp;
	input = regex_replace(input, m_regex, replacement,
			getTemporaryFlags(options));
	return input;
}


string& HumRegex::replaceDestructive (string* input, const string& exp,
		const string& replacement, const string& options) {
	return HumRegex::replaceDestructive(*input, exp, replacement, options);
}



//////////////////////////////
//
// HumRegex::replaceCopy --  Keep input string the same, return replacement
//    string as output
//

string HumRegex::replaceCopy(const string& input, const string& exp,
		const string& replacement) {
	m_regex = exp;
	return regex_replace(input, m_regex, replacement, m_flags);
}


string HumRegex::replaceCopy(string* input, const string& exp,
		const string& replacement) {
	return HumRegex::replaceCopy(*input, exp, replacement);
}

//
// This version allows for temporary match flag options.
//

string HumRegex::replaceCopy(const string& input, const string& exp,
		const string& replacement, const string& options) {
	m_regex = exp;
	return regex_replace(input, m_regex, replacement,
			getTemporaryFlags(options));
}


string HumRegex::replaceCopy(string* input, const string& exp,
		const string& replacement, const string& options) {
	return HumRegex::replaceCopy(*input, exp, replacement, options);
}



//////////////////////////////
//
// HumRegex::split --
//

bool HumRegex::split(vector<string>& entries, const string& buffer,
		const string& separator) {
	entries.clear();
	string newsep = "(";
	newsep += separator;
	newsep += ")";
	int status = search(buffer, newsep);
	if (!status) {
		return false;
	}
	int start = 0;
	while (status) {
		entries.push_back(getPrefix());
		start += getMatchEndIndex(1);
		status = search(buffer, start, newsep);
	}
	// add last token:
	entries.push_back(getSuffix());
	return true;
}



// END_MERGE

} // end namespace hum



