//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Aug  8 12:24:49 PDT 2015
// Last Modified: Sun Aug  9 21:03:12 PDT 2015
// Filename:      Convert.h
// URL:           https://github.com/craigsapp/minHumdrum/blob/master/include/Convert.h
// Syntax:        C++11
// vim:           ts=3 noexpandtab
//
// Description:   Convert between various data representations.
//

#ifndef _CONVERT_H
#define _CONVERT_H

#include <iostream>
#include <vector>

#include "HumNum.h"

using namespace std;

namespace minHumdrum {

// START_MERGE

class Convert {
	public:

      // Rhythm processing, defined in Convert-rhythm.cpp
		static HumNum  recipToDuration    (const string& recip, HumNum scale = 4,
		                                   string separator = " ");

      // Pitch processing, defined in Convert-pitch.cpp
      static int     kernToOctaveNumber   (const string& kerndata);
		static int     kernToAccidentalCount(const string& kerndata);
		static int     kernToDiatonicPC     (const string& kerndata);
		static char    kernToDiatonicUC     (const string& kerndata);
		static char    kernToDiatonicLC     (const string& kerndata);
		static string  kernToScientificPitch(const string& kerndata, 
		                                     string flat = "b",
		                                     string sharp = "#", 
		                                     string separator = "");
		static string  kernToSciPitch      (const string& kerndata, 
		      string flat = "b", string sharp = "#", string separator = "") {
			return kernToScientificPitch(kerndata, flat, sharp, separator);
		}
		static string  kernToSP            (const string& kerndata, 
		      string flat = "b", string sharp = "#", string separator = "") {
			return kernToScientificPitch(kerndata, flat, sharp, separator);
		}

		// String processing, defined in Convert-string.cpp
		static vector<string> splitString   (const string& data,
		                                     char separator = ' ');
		static void    replaceOccurrences   (string& source,
		                                     const string& search,
		                                     const string& replace);
		static string  repeatString         (const string& pattern, int count);
		static string  encodeXml            (const string& input);
		static string  getHumNumAttributes  (const HumNum& num);

		// Mathematical processing, defined in Convert-math.cpp
		static int     getLcm               (const vector<int>& numbers);
      static int     getGcd               (int a, int b);

};



// END_MERGE

} // end namespace std;

#endif /* _CONVERT */



