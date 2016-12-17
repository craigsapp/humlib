// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Dec  3 16:21:22 PST 2016
// Last Modified: Tue Dec 13 03:09:32 PST 2016
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
//     /usr/local/Cellar/gcc49/4.9.3/include/c++/4.9.3/bits/regex_constants.h
//

#include "HumRegex.h"
#include <iostream>

using namespace std;

namespace hum {

// START_MERGE


//////////////////////////////
//
// HumRegex::HumRegex -- Constructor.
//

HumRegex::HumRegex(void) {
	// by default use ECMAScript regular expression syntax:
	m_regexflags  = std::regex_constants::ECMAScript;

	m_searchflags = std::regex_constants::format_first_only;
}


HumRegex::HumRegex(const string& exp, const string& options) {
	// initialize a regular expression for the object
	m_regexflags = (std::regex_constants::syntax_option_type)0;
	m_regexflags = getTemporaryRegexFlags(options);
	if (m_regexflags == 0) {
		// explicitly set the default syntax
		m_regexflags = std::regex_constants::ECMAScript;
	}
	m_regex = regex(exp, getTemporaryRegexFlags(options));
	m_searchflags = (std::regex_constants::match_flag_type)0;
	m_searchflags = getTemporarySearchFlags(options);
}



//////////////////////////////
//
// HumRegex::HumRegex -- Destructor.
//

HumRegex::~HumRegex() {
	// do nothing
}


///////////////////////////////////////////////////////////////////////////
//
// option setting
//

//////////////////////////////
//
// HumRegex::setIgnoreCase --
//

void HumRegex::setIgnoreCase(void) {
	m_regexflags |= std::regex_constants::icase;
}



//////////////////////////////
//
// HumRegex::getIgnoreCase --
//

bool HumRegex::getIgnoreCase(void) {
	return m_regexflags & std::regex_constants::icase;
}



//////////////////////////////
//
// HumRegex::unsetIgnoreCase --
//

void HumRegex::unsetIgnoreCase(void) {
	m_regexflags &= ~std::regex_constants::icase;
}



//////////////////////////////
//
// HumRegex::setGlobal --
//

void HumRegex::setGlobal(void) {
	m_searchflags &= ~std::regex_constants::format_first_only;
}



//////////////////////////////
//
// HumRegex::getGlobal --
//

bool HumRegex::getGlobal(void) {
	return !(m_searchflags & std::regex_constants::format_first_only);
}



//////////////////////////////
//
// HumRegex::unsetGlobal --
//

void HumRegex::unsetGlobal(void) {
	m_searchflags |= std::regex_constants::format_first_only;
}


///////////////////////////////////////////////////////////////////////////
//
// Searching functions
//

//////////////////////////////
//
// HumRegex::search -- Search for the regular expression in the
//    input string.  Returns true if any matches were found.  Search
//    results can be accessed with .getSubmatchCount() and .getSubmatch(index).
//

bool HumRegex::search(const string& input, const string& exp) {
	m_regex = regex(exp, m_regexflags);
	return regex_search(input, m_matches, m_regex, m_searchflags);
}


bool HumRegex::search(const string& input, int startindex,
		const string& exp) {
	m_regex = regex(exp, m_regexflags);
	auto startit = input.begin() + startindex;
	auto endit   = input.end();
	return regex_search(startit, endit, m_matches, m_regex, m_searchflags);
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
	m_regex = regex(exp, getTemporaryRegexFlags(options));
	return regex_search(input, m_matches, m_regex, getTemporarySearchFlags(options));
}


bool HumRegex::search(const string& input, int startindex, const string& exp,
		const string& options) {
	m_regex = regex(exp, getTemporaryRegexFlags(options));
	auto startit = input.begin() + startindex;
	auto endit   = input.end();
	return regex_search(startit, endit, m_matches, m_regex, getTemporarySearchFlags(options));
}


bool HumRegex::search(string* input, const string& exp,
		const string& options) {
	return HumRegex::search(*input, exp, options);
}


bool HumRegex::search(string* input, int startindex, const string& exp,
		const string& options) {
	return HumRegex::search(*input, startindex, exp, options);
}


///////////////////////////////////////////////////////////////////////////
//
// match-related functions
//

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


///////////////////////////////////////////////////////////////////////////
//
// match functions (a "match" is a search that matches a regular 
//    expression to the entire string").
//

//////////////////////////////
//
// HumRegex::match --
//

bool HumRegex::match(const string& input, const string& exp) {
	m_regex = regex(exp, m_regexflags);
	return regex_match(input, m_regex, m_searchflags);
}


bool HumRegex::match(const string& input, const string& exp,
		const string& options) {
	m_regex = regex(exp, getTemporaryRegexFlags(options));
	return regex_match(input, m_regex, getTemporarySearchFlags(options));
}


bool HumRegex::match(const string* input, const string& exp) {
	return HumRegex::match(*input, exp);

}


bool HumRegex::match(const string* input, const string& exp,
		const string& options) {
	return HumRegex::match(*input, exp, options);
}



///////////////////////////////////////////////////////////////////////////
//
// search and replace functions.  Default behavior is to only match
// the first match.  use the "g" option or .setGlobal() to do global
// replacing.
//

//////////////////////////////
//
// HumRegex::replaceDestructive -- Replace in input string.
//

string& HumRegex::replaceDestructive(string& input, const string& replacement,
		const string& exp) {
	m_regex = regex(exp, m_regexflags);
	regex_replace(input, m_regex, replacement, m_searchflags);
	return input;
}


string& HumRegex::replaceDestructive(string* input, const string& replacement,
		const string& exp) {
	return HumRegex::replaceDestructive(*input, replacement, exp);
}

//
// This version allows for temporary match flag options.
//

string& HumRegex::replaceDestructive(string& input, const string& replacement,
		const string& exp, const string& options) {
	m_regex = regex(exp, getTemporaryRegexFlags(options));
	regex_replace(input, m_regex, replacement,
			getTemporarySearchFlags(options));
	return input;
}


string& HumRegex::replaceDestructive (string* input, const string& replacement,
		const string& exp, const string& options) {
	return HumRegex::replaceDestructive(*input, replacement, exp, options);
}



//////////////////////////////
//
// HumRegex::replaceCopy --  Keep input string the same, return replacement
//    string as output
//

string HumRegex::replaceCopy(const string& input, const string& replacement,
		const string& exp) {
	m_regex = regex(exp, m_regexflags);
	string output;
	regex_replace(std::back_inserter(output), input.begin(), input.end(),
			m_regex, replacement);
	return output;
}


string HumRegex::replaceCopy(string* input, const string& replacement,
		const string& exp) {
	return HumRegex::replaceCopy(*input, replacement, exp);
}

//
// This version allows for temporary match flag options.
//

string HumRegex::replaceCopy(const string& input, const string& exp,
		const string& replacement, const string& options) {
	m_regex = regex(exp, getTemporaryRegexFlags(options));
	string output;
	regex_replace(std::back_inserter(output), input.begin(), input.end(),
			m_regex, replacement, getTemporarySearchFlags(options));
	return output;
}


string HumRegex::replaceCopy(string* input, const string& exp,
		const string& replacement, const string& options) {
	return HumRegex::replaceCopy(*input, replacement, exp, options);
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
		if (buffer.size() == 0) {
			return false;
		} else {
			entries.push_back(buffer);
			return true;
		}
	}
	int start = 0;
	while (status) {
		entries.push_back(getPrefix());
		start += getMatchEndIndex(1);
		status = search(buffer, start, newsep);
	}
	// add last token:
	entries.push_back(buffer.substr(start));
	return true;
}



//////////////////////////////
//
// HumRegex::getTemporaryRegexFlags --
//

std::regex_constants::syntax_option_type HumRegex::getTemporaryRegexFlags(
		const string& sflags) {
	if (sflags.empty()) {
		return m_regexflags;
	}
	std::regex_constants::syntax_option_type temp_flags = m_regexflags;
	for (auto it : sflags) {
		switch (it) {
			case 'i':
				temp_flags = (std::regex_constants::syntax_option_type)
						(temp_flags | std::regex_constants::icase);
				break;
			case 'I':
				temp_flags = (std::regex_constants::syntax_option_type)
						(temp_flags & ~std::regex_constants::icase);
				break;
		}
	}
	return temp_flags;
}



//////////////////////////////
//
// HumRegex::getTemporarySearchFlags --
//

std::regex_constants::match_flag_type HumRegex::getTemporarySearchFlags(
		const string& sflags) {
	if (sflags.empty()) {
		return m_searchflags;
	}
	std::regex_constants::match_flag_type temp_flags = m_searchflags;
	for (auto it : sflags) {
		switch (it) {
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

// END_MERGE

} // end namespace hum



