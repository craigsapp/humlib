//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Aug  8 12:24:49 PDT 2015
// Last Modified: Sun Aug  9 21:03:12 PDT 2015
// Filename:      Convert.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/Convert.h
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
#include "HumdrumToken.h"

using namespace std;

namespace hum {

// START_MERGE

class Convert {
	public:

		// Rhythm processing, defined in Convert-rhythm.cpp
		static HumNum  recipToDuration    (const string& recip, HumNum scale = 4,
		                                   string separator = " ");
		static HumNum  recipToDurationNoDots(const string& recip,
		                                   HumNum scale = 4,
		                                   string separator = " ");

		// Pitch processing, defined in Convert-pitch.cpp
		static string  base40ToKern         (int b40);
   	static int     base40ToAccidental   (int b40);
   	static int     base40ToDiatonic     (int b40);
		static int     base40ToMidiNoteNumber(int b40);
		static int     kernToOctaveNumber   (const string& kerndata);
		static int     kernToOctaveNumber   (HTp token) 
				{ return kernToOctaveNumber((string)*token); }
		static int     kernToAccidentalCount(const string& kerndata);
		static int     kernToAccidentalCount(HTp token) 
				{ return kernToAccidentalCount((string)*token); }
		static int     kernToDiatonicPC     (const string& kerndata);
		static int     kernToDiatonicPC     (HTp token) 
				{ return kernToDiatonicPC     ((string)*token); }
		static char    kernToDiatonicUC     (const string& kerndata);
		static int     kernToDiatonicUC     (HTp token) 
				{ return kernToDiatonicUC     ((string)*token); }
		static char    kernToDiatonicLC     (const string& kerndata);
		static int     kernToDiatonicLC     (HTp token) 
				{ return kernToDiatonicLC     ((string)*token); }
		static int     kernToBase40PC       (const string& kerndata);
		static int     kernToBase40PC       (HTp token) 
				{ return kernToBase40PC       ((string)*token); }
		static int     kernToBase12PC       (const string& kerndata);
		static int     kernToBase12PC       (HTp token) 
				{ return kernToBase12PC       ((string)*token); }
		static int     kernToBase7PC        (const string& kerndata) {
		                                     return kernToDiatonicPC(kerndata); }
		static int     kernToBase7PC        (HTp token) 
				{ return kernToBase7PC        ((string)*token); }
		static int     kernToBase40         (const string& kerndata);
		static int     kernToBase40         (HTp token) 
				{ return kernToBase40         ((string)*token); }
		static int     kernToBase12         (const string& kerndata);
		static int     kernToBase12         (HTp token) 
				{ return kernToBase12         ((string)*token); }
		static int     kernToBase7          (const string& kerndata);
		static int     kernToBase7         (HTp token) 
				{ return kernToBase7         ((string)*token); }
		static int     kernToMidiNoteNumber (const string& kerndata);
		static int     kernToMidiNoteNumber(HTp token) 
				{ return kernToMidiNoteNumber((string)*token); }
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
		static int     pitchToWbh         (int dpc, int acc, int octave,
		                                   int maxacc);
		static void    wbhToPitch         (int& dpc, int& acc, int& octave,
		                                   int maxacc, int wbh);

		// data-type specific (other than pitch/rhythm),
		// defined in Convert-kern.cpp
		static bool isKernRest              (const string& kerndata);
		static bool isKernNote              (const string& kerndata);
		static bool isKernNoteAttack        (const string& kerndata);
		static bool hasKernSlurStart        (const string& kerndata);
		static bool hasKernSlurEnd          (const string& kerndata);
		static int  getKernSlurStartElisionLevel(const string& kerndata);
		static int  getKernSlurEndElisionLevel  (const string& kerndata);

		static bool isKernSecondaryTiedNote (const string& kerndata);
		static string  getKernPitchAttributes  (const string& kerndata);

		// String processing, defined in Convert-string.cpp
		static vector<string> splitString   (const string& data,
		                                     char separator = ' ');
		static void    replaceOccurrences   (string& source,
		                                     const string& search,
		                                     const string& replace);
		static string  repeatString         (const string& pattern, int count);
		static string  encodeXml            (const string& input);
		static string  getHumNumAttributes  (const HumNum& num);
		static string  trimWhiteSpace       (const string& input);

		// Mathematical processing, defined in Convert-math.cpp
		static int     getLcm               (const vector<int>& numbers);
		static int     getGcd               (int a, int b);

};



// END_MERGE

} // end namespace hum

#endif /* _CONVERT */



