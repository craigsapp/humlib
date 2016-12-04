//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Dec  3 10:09:19 PST 2016
// Last Modified: Sat Dec  3 10:09:23 PST 2016
// Filename:      HumRegex.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/HumRegex.h
// Syntax:        C++11
// vim:           ts=3 noexpandtab
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

		// searching
		bool          search             (const string& input,
		                                  const string& exp);
		int           getMatchCount      (void);
		string        getMatch           (int index);
		string        getPrefix          (void);
		string        getSuffix          (void);
		int           getMatchStartIndex (int index = 0);
		int           getMatchEndIndex   (int index = 0);
		int           getMatchLength     (int index = 0);
   
   protected:

		// m_regex: store the regular expression to use as a default.
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

};


// END_MERGE

} // end namespace hum

#endif  /* _HUMREGEX_H_INCLUDED */



