//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Dec  3 10:09:19 PST 2016
// Last Modified: Sat Dec  3 10:09:23 PST 2016
// Filename:      HumRegex.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/HumRegex.h
// Syntax:        C++11
// vim:           syntax=cpp ts=3 noexpandtab nowrap
// note:          Requires GCC v4.9 or higher
//
// Description:   Interface to C++11 regular expressions.
//

#ifndef _HUMREGEX_H_INCLUDED
#define _HUMREGEX_H_INCLUDED

#include <regex>

using namespace std;

namespace hum {

// START_MERGE

class HumRegex {
	public:
		            HumRegex           (void);
		            HumRegex           (const string& exp);
		           ~HumRegex           ();

		// replacing

		string&     replaceDestructive (string& input, const string& exp,
		                                const string& replacement);
		string&     replaceDestructive (string& input, const string& exp,
		                                const string& replacement,
		                                const string& options);
		string      replaceCopy        (const string& input, const string& exp,
		                                const string& replacement);
		string      replaceCopy        (const string& input, const string& exp,
		                                const string& replacement,
		                                const string& options);

		string&     replaceDestructive (string* input, const string& exp,
		                                const string& replacement);
		string&     replaceDestructive (string* input, const string& exp,
		                                const string& replacement,
		                                const string& options);
		string      replaceCopy        (string* input, const string& exp,
		                                const string& replacement);
		string      replaceCopy        (string* input, const string& exp,
		                                const string& replacement,
		                                const string& options);

		// searching
		// http://www.cplusplus.com/reference/regex/regex_search
		bool        search             (const string& input, const string& exp);
		bool        search             (const string& input, const string& exp,
		                                const string& options);
		bool        search             (const string& input, int startindex,
		                                const string& exp);
		bool        search             (const string& input, int startindex,
		                                const string& exp,
		                                const string& options);

		bool        search             (string* input, const string& exp);
		bool        search             (string* input, const string& exp,
		                                const string& options);
		bool        search             (string* input, int startindex,
		                                const string& exp);
		bool        search             (string* input, int startindex,
		                                const string& exp,
		                                const string& options);

		int         getMatchCount      (void);
		string      getMatch           (int index);
		int         getMatchInt        (int index);
		string      getPrefix          (void);
		string      getSuffix          (void);
		int         getMatchStartIndex (int index = 0);
		int         getMatchEndIndex   (int index = 0);
		int         getMatchLength     (int index = 0);

		// token lists:
		bool        split              (vector<string>& entries,
		                                const string& buffer,
		                                const string& separator);

		// setting option flags:
		void        setIgnoreCase      (void);
		void        setNoIgnoreCase    (void);

	protected:
		std::regex_constants::match_flag_type
				getTemporaryFlags(const string& sflags);


	private:

		// m_regex: stores the regular expression to use as a default.
		//
		// http://en.cppreference.com/w/cpp/regex/basic_regex
		// .assign(string) == set the regular expression.
		// operator=       == set the regular expression.
		std::regex m_regex;

		// m_matches: stores the matches from a search:
		//
		// http://en.cppreference.com/w/cpp/regex/match_results
		// .empty()     == check if match was successful.
		// .size()      == number of matches.
		// .length(i)   == return length of a submatch.
		// .position(i) == return start index of submatch in search string.
		// .str(i)      == return string of submatch.
		// operator[i]  == return submatch.
		// .prefix
		// .suffix
		// .begin()     == start of submatch list.
		// .end()       == end of submatch list.
		std::smatch m_matches;

		// m_flags: stores default settings for regex processing
		// http://en.cppreference.com/w/cpp/regex/syntax_option_type
		// http://en.cppreference.com/w/cpp/regex/basic_regex
		//
		// Options:
		//    regex::icase      == Ignore case.
		//    regex::nosubs     == Don't collect submatches.
		//    regex::optimize   == Make matching faster, but
		//                                   construction slower.
		//    regex::collate    == locale character ranges.
		//    regex::multiline  == C++17 only.
		//
		// only one of the following can be given.  EMCAScript will be
		// used by default if non specified.
		//    regex::EMCAScript == Use EMCAScript regex syntax.
		//    regex::basic      == Use basic POSIX syntax.
		//    regex::extended   == Use extended POSIX syntax.
		//    regex::awk        == Use awk POSIX syntax.
		//    regex::grep       == Use grep POSIX syntax.
		//    regex::extended   == Use egrep POSIX syntax.
		// or:
		// http://www.cplusplus.com/reference/regex/regex_search/
		//    match_default     == clear all options
		//    match_not_bol     == not beginning of line
		//    match_not_eol     == not end of line
		//    match_not_bow     == not beginning of word for \b
		//    match_not_eow     == not end of word for \b
		//    match_any         == any match acceptable if more than 1 possible.
		//    match_not_null    == empty sequence does note match
		//    match_continuous  ==
		//    match_prev_avail  ==
		//    format_default    == same as match_default.
		std::regex_constants::match_flag_type m_flags;

};


// END_MERGE

} // end namespace hum

#endif  /* _HUMREGEX_H_INCLUDED */



