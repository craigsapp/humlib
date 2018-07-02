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
#include <string>
#include <vector>

namespace hum {

// START_MERGE

class HumRegex {
	public:
		            HumRegex           (void);
		            HumRegex           (const std::string& exp,
		                                const std::string& options = "");
		           ~HumRegex           ();

		// setting persistent options for regular expression contruction
		void        setIgnoreCase      (void);
		bool        getIgnoreCase      (void);
		void        unsetIgnoreCase    (void);

		// setting persistent search/match options
		void        setGlobal          (void);
		bool        getGlobal          (void);
		void        unsetGlobal        (void);

		// replacing
		std::string&     replaceDestructive (std::string& input, const std::string& replacement,
		                                const std::string& exp);
		std::string&     replaceDestructive (std::string& input, const std::string& replacement,
		                                const std::string& exp,
		                                const std::string& options);
		std::string      replaceCopy        (const std::string& input,
		                                const std::string& replacement,
		                                const std::string& exp);
		std::string      replaceCopy        (const std::string& input,
		                                const std::string& replacement,
		                                const std::string& exp,
		                                const std::string& options);

		std::string&     replaceDestructive (std::string* input, const std::string& replacement,
		                                const std::string& exp);
		std::string&     replaceDestructive (std::string* input, const std::string& replacement,
		                                const std::string& exp,
		                                const std::string& options);
		std::string      replaceCopy        (std::string* input, const std::string& replacement,
		                                const std::string& exp);
		std::string      replaceCopy        (std::string* input, const std::string& replacement,
		                                const std::string& exp,
		                                const std::string& options);
		std::string&      tr                 (std::string& input, const std::string& from,
		                                const std::string& to);

		// matching (full-string match)
		bool        match              (const std::string& input, const std::string& exp);
		bool        match              (const std::string& input, const std::string& exp,
		                                const std::string& options);
		bool        match              (const std::string* input, const std::string& exp);
		bool        match              (const std::string* input, const std::string& exp,
		                                const std::string& options);


		// searching
		// http://www.cplusplus.com/reference/regex/regex_search
		int         search             (const std::string& input, const std::string& exp);
		int         search             (const std::string& input, const std::string& exp,
		                                const std::string& options);
		int         search             (const std::string& input, int startindex,
		                                const std::string& exp);
		int         search             (const std::string& input, int startindex,
		                                const std::string& exp,
		                                const std::string& options);

		int         search             (std::string* input, const std::string& exp);
		int         search             (std::string* input, const std::string& exp,
		                                const std::string& options);
		int         search             (std::string* input, int startindex,
		                                const std::string& exp);
		int         search             (std::string* input, int startindex,
		                                const std::string& exp,
		                                const std::string& options);

		int         getMatchCount      (void);
		std::string getMatch           (int index);
		int         getMatchInt        (int index);
		double      getMatchDouble     (int index);
		std::string getPrefix          (void);
		std::string getSuffix          (void);
		int         getMatchStartIndex (int index = 0);
		int         getMatchEndIndex   (int index = 0);
		int         getMatchLength     (int index = 0);

		// token lists:
		bool        split              (std::vector<std::string>& entries,
		                                const std::string& buffer,
		                                const std::string& separator);

	protected:
		std::regex_constants::syntax_option_type
				getTemporaryRegexFlags(const std::string& sflags);
		std::regex_constants::match_flag_type
				getTemporarySearchFlags(const std::string& sflags);


	private:

		// m_regex: stores the regular expression to use as a default.
		//
		// http://en.cppreference.com/w/cpp/regex/basic_regex
		// .assign(string) == set the regular expression.
		// operator=       == set the regular expression.
		// .flags()        == return syntax_option_type used to construct.
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

		// m_regexflags: store default settings for regex processing
		// http://en.cppreference.com/w/cpp/regex/syntax_option_type
		// http://en.cppreference.com/w/cpp/regex/basic_regex
		// /usr/local/Cellar/gcc49/4.9.3/include/c++/4.9.3/bits/regex_constants.h
		//
		// Options (in the namespace std::regex_constants):
		//    icase      == Ignore case.
		//    nosubs     == Don't collect submatches.
		//    optimize   == Make matching faster, but construction slower.
		//    collate    == locale character ranges.
		//    multiline  == C++17 only.
		//
		// Only one of the following can be given.  EMCAScript will be
		// used by default if none specified.
		//    EMCAScript == Use EMCAScript regex syntax.
		//    basic      == Use basic POSIX syntax.
		//    extended   == Use extended POSIX syntax.
		//    awk        == Use awk POSIX syntax.
		//    grep       == Use grep POSIX syntax.
		//    egrep      == Use egrep POSIX syntax.
		std::regex_constants::syntax_option_type m_regexflags;

		// m_flags: store default settings for regex processing
		// http://www.cplusplus.com/reference/regex/regex_search
		//    match_default     == clear all options.
		//    match_not_bol     == not beginning of line.
		//    match_not_eol     == not end of line.
		//    match_not_bow     == not beginning of word for \b.
		//    match_not_eow     == not end of word for \b.
		//    match_any         == any match acceptable if more than 1 possible..
		//    match_not_null    == empty sequence does note match.
		//    match_continuous  ==
		//    match_prev_avail  ==
		//    format_default    == same as match_default.
		std::regex_constants::match_flag_type m_searchflags;

};


// END_MERGE

} // end namespace hum

#endif  /* _HUMREGEX_H_INCLUDED */



