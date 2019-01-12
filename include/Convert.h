//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Aug  8 12:24:49 PDT 2015
// Last Modified: Sun Aug  9 21:03:12 PDT 2015
// Filename:      Convert.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/Convert.h
// Syntax:        C++11
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Convert between various data representations.
//

#ifndef _CONVERT_H_INCLUDED
#define _CONVERT_H_INCLUDED

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
		static HumNum  recipToDuration      (const string& recip,
		                                     HumNum scale = 4,
		                                     const string& separator = " ");
		static HumNum  recipToDurationNoDots(const string& recip,
		                                     HumNum scale = 4,
		                                     const string& separator = " ");
		static HumNum  recipToDuration      (string* recip,
		                                     HumNum scale = 4,
		                                     const string& separator = " ");
		static HumNum  recipToDurationNoDots(string* recip,
		                                     HumNum scale = 4,
		                                     const string& separator = " ");
		static string  durationToRecip      (HumNum duration,
		                                     HumNum scale = HumNum(1,4));
		static string  durationFloatToRecip (double duration,
		                                     HumNum scale = HumNum(1,4));

		// Pitch processing, defined in Convert-pitch.cpp
		static string  base40ToKern         (int b40);
		static int     base40ToAccidental   (int b40);
		static int     base40ToDiatonic     (int b40);
		static int     base40ToMidiNoteNumber(int b40);
		static string  base40ToIntervalAbbr (int b40);
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
		static int     kernToBase7          (HTp token)
				{ return kernToBase7          ((string)*token); }
		static int     kernToMidiNoteNumber (const string& kerndata);
		static int     kernToMidiNoteNumber(HTp token)
				{ return kernToMidiNoteNumber((string)*token); }
		static string  kernToScientificPitch(const string& kerndata,
		                                     string flat = "b",
		                                     string sharp = "#",
		                                     string separator = "");
		static string  kernToSciPitch       (const string& kerndata,
		      										 string flat = "b",
		                                     string sharp = "#",
		                                     string separator = "")
	       { return kernToScientificPitch(kerndata, flat, sharp, separator); }
		static string  kernToSP             (const string& kerndata,
		                                     string flat = "b",
		                                     string sharp = "#",
		                                     string separator = "")
		      { return kernToScientificPitch(kerndata, flat, sharp, separator); }
		static int     pitchToWbh           (int dpc, int acc, int octave,
		                                     int maxacc);
		static void    wbhToPitch           (int& dpc, int& acc, int& octave,
		                                     int maxacc, int wbh);
		static int     kernClefToBaseline   (const string& input);
		static string  base40ToTrans        (int base40);
		static int     transToBase40        (const string& input);
		static int     base40IntervalToLineOfFifths(int trans);
		static string  keyNumberToKern      (int number);
		static int     base7ToBase40        (int base7);
		static int     base40IntervalToDiatonic(int base40interval);


		// **mens, white mensual notation, defiend in Convert-mens.cpp
		static bool    isMensRest           (const string& mensdata);
		static bool    isMensNote           (const string& mensdata);
		static bool    hasLigatureBegin     (const string& mensdata);
		static bool    hasLigatureEnd       (const string& mensdata);
		static bool    getMensStemDirection (const string& mensdata);
		static HumNum  mensToDuration       (const string& mensdata,
		                                     HumNum scale = 4,
		                                     const string& separator = " ");
		static string  mensToRecip          (const string& mensdata,
		                                     HumNum scale = 4,
		                                     const string& separator = " ");
		static HumNum  mensToDurationNoDots(const string& mensdata,
		                                     HumNum scale = 4,
		                                     const string& separator = " ");

		// Harmony processing, defined in Convert-harmony.cpp
		static vector<int> minorHScaleBase40(void);
		static vector<int> majorScaleBase40 (void);
		static int         keyToInversion   (const string& harm);
		static int         keyToBase40      (const string& key);
		static vector<int> harmToBase40     (HTp harm, const string& key) {
		                                        return harmToBase40(*harm, key); }
		static vector<int> harmToBase40     (HTp harm, HTp key) {
		                                        return harmToBase40(*harm, *key); }
		static vector<int> harmToBase40     (const string& harm, const string& key);
		static vector<int> harmToBase40     (const string& harm, int keyroot, int keymode);
		static void        makeAdjustedKeyRootAndMode(const string& secondary,
		                                     int& keyroot, int& keymode);
		static int         chromaticAlteration(const string& content);

		// data-type specific (other than pitch/rhythm),
		// defined in Convert-kern.cpp
		static bool isKernRest              (const string& kerndata);
		static bool isKernNote              (const string& kerndata);
		static bool isKernNoteAttack        (const string& kerndata);
		static bool hasKernSlurStart        (const string& kerndata);
		static bool hasKernSlurEnd          (const string& kerndata);
		static int  getKernSlurStartElisionLevel(const string& kerndata, int index);
		static int  getKernSlurEndElisionLevel  (const string& kerndata, int index);
		static char hasKernStemDirection    (const string& kerndata);

		static bool isKernSecondaryTiedNote (const string& kerndata);
		static string getKernPitchAttributes(const string& kerndata);

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
		static bool    startsWith           (const string& input,
		                                     const string& searchstring);
		static bool    contains(const string& input, const string& pattern);
		static bool    contains(const string& input, char pattern);
		static bool    contains(string* input, const string& pattern);
		static bool    contains(string* input, char pattern);
		static void    makeBooleanTrackList(vector<bool>& spinelist,
		                                     const string& spinestring,
		                                     int maxtrack);

		// Mathematical processing, defined in Convert-math.cpp
		static int     getLcm               (const vector<int>& numbers);
		static int     getGcd               (int a, int b);
		static void    primeFactors         (vector<int>& output, int n);
		static double  nearIntQuantize      (double value,
		                                    double delta = 0.00001);
		static double  significantDigits    (double value, int digits);
		static bool    isNaN                (double value);
		static double  pearsonCorrelation   (vector<double> x, vector<double> y);
		static int     romanNumeralToInteger(const string& roman);

};


// END_MERGE

} // end namespace hum

#endif /* _CONVERT_H_INCLUDED */



