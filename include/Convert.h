//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Aug  8 12:24:49 PDT 2015
// Last Modified: Sun Aug  9 21:03:12 PDT 2015
// Filename:      Convert.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/Convert.h
// Syntax:        C++11; humlib
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Convert between various data representations.
//

#ifndef _CONVERT_H_INCLUDED
#define _CONVERT_H_INCLUDED

#include <vector>
#include <string>

#include "HumNum.h"
#include "HumdrumToken.h"

namespace hum {

// START_MERGE

class Convert {
	public:

		// Rhythm processing, defined in Convert-rhythm.cpp
		static HumNum  recipToDuration      (const std::string& recip,
		                                     HumNum scale = 4,
		                                     const std::string& separator = " ");
		static HumNum  recipToDurationNoDots(const std::string& recip,
		                                     HumNum scale = 4,
		                                     const std::string& separator = " ");
		static HumNum  recipToDuration      (std::string* recip,
		                                     HumNum scale = 4,
		                                     const std::string& separator = " ");
		static HumNum  recipToDurationNoDots(std::string* recip,
		                                     HumNum scale = 4,
		                                     const std::string& separator = " ");
		static std::string  durationToRecip      (HumNum duration,
		                                     HumNum scale = HumNum(1,4));
		static std::string  durationFloatToRecip (double duration,
		                                     HumNum scale = HumNum(1,4));
		static HumNum timeSigToDurationInQuarter(HTp token);

		// Pitch processing, defined in Convert-pitch.cpp
		static std::string  base40ToKern    (int b40);
		static int     base40ToAccidental   (int b40);
		static int     base40ToDiatonic     (int b40);
		static int     base40ToMidiNoteNumber(int b40);
		static std::string  base40ToIntervalAbbr (int b40);
		static int     kernToOctaveNumber   (const std::string& kerndata);
		static int     kernToOctaveNumber   (HTp token)
				{ return kernToOctaveNumber((std::string)*token); }
		static int     kernToAccidentalCount(const std::string& kerndata);
		static int     kernToAccidentalCount(HTp token)
				{ return kernToAccidentalCount((std::string)*token); }
		static int     kernToDiatonicPC     (const std::string& kerndata);
		static int     kernToDiatonicPC     (HTp token)
				{ return kernToDiatonicPC     ((std::string)*token); }
		static char    kernToDiatonicUC     (const std::string& kerndata);
		static int     kernToDiatonicUC     (HTp token)
				{ return kernToDiatonicUC     ((std::string)*token); }
		static char    kernToDiatonicLC     (const std::string& kerndata);
		static int     kernToDiatonicLC     (HTp token)
				{ return kernToDiatonicLC     ((std::string)*token); }
		static int     kernToBase40PC       (const std::string& kerndata);
		static int     kernToBase40PC       (HTp token)
				{ return kernToBase40PC       ((std::string)*token); }
		static int     kernToBase12PC       (const std::string& kerndata);
		static int     kernToBase12PC       (HTp token)
				{ return kernToBase12PC       ((std::string)*token); }
		static int     kernToBase7PC        (const std::string& kerndata) {
		                                     return kernToDiatonicPC(kerndata); }
		static int     kernToBase7PC        (HTp token)
				{ return kernToBase7PC        ((std::string)*token); }
		static int     kernToBase40         (const std::string& kerndata);
		static int     kernToBase40         (HTp token)
				{ return kernToBase40         ((std::string)*token); }
		static int     kernToBase12         (const std::string& kerndata);
		static int     kernToBase12         (HTp token)
				{ return kernToBase12         ((std::string)*token); }
		static int     kernToBase7          (const std::string& kerndata);
		static int     kernToBase7          (HTp token)
				{ return kernToBase7          ((std::string)*token); }
		static int     kernToMidiNoteNumber (const std::string& kerndata);
		static int     kernToMidiNoteNumber(HTp token)
				{ return kernToMidiNoteNumber((std::string)*token); }
		static std::string  kernToScientificPitch(const std::string& kerndata,
		                                     std::string flat = "b",
		                                     std::string sharp = "#",
		                                     std::string separator = "");
		static std::string  kernToSciPitch  (const std::string& kerndata,
		      										 std::string flat = "b",
		                                     std::string sharp = "#",
		                                     std::string separator = "")
	       { return kernToScientificPitch(kerndata, flat, sharp, separator); }
		static std::string  kernToSP        (const std::string& kerndata,
		                                     std::string flat = "b",
		                                     std::string sharp = "#",
		                                     std::string separator = "")
		      { return kernToScientificPitch(kerndata, flat, sharp, separator); }
		static int     pitchToWbh           (int dpc, int acc, int octave,
		                                     int maxacc);
		static void    wbhToPitch           (int& dpc, int& acc, int& octave,
		                                     int maxacc, int wbh);
		static int     kernClefToBaseline   (const std::string& input);
		static int     kernClefToBaseline   (HTp input);
		static std::string  base40ToTrans   (int base40);
		static int     transToBase40        (const std::string& input);
		static int     base40IntervalToLineOfFifths(int trans);
		static std::string  keyNumberToKern (int number);
		static int     base7ToBase40        (int base7);
		static int     base40IntervalToDiatonic(int base40interval);


		// **mens, white mensual notation, defiend in Convert-mens.cpp
		static bool    isMensRest           (const std::string& mensdata);
		static bool    isMensNote           (const std::string& mensdata);
		static bool    hasLigatureBegin     (const std::string& mensdata);
		static bool    hasRectaLigatureBegin(const std::string& mensdata);
		static bool    hasObliquaLigatureBegin(const std::string& mensdata);
		static bool    hasLigatureEnd       (const std::string& mensdata);
		static bool    hasRectaLigatureEnd  (const std::string& mensdata);
		static bool    hasObliquaLigatureEnd(const std::string& mensdata);
		static bool    getMensStemDirection (const std::string& mensdata);
		static HumNum  mensToDuration       (const std::string& mensdata,
		                                     HumNum scale = 4,
		                                     const std::string& separator = " ");
		static std::string  mensToRecip          (const std::string& mensdata,
		                                     HumNum scale = 4,
		                                     const std::string& separator = " ");
		static HumNum  mensToDurationNoDots(const std::string& mensdata,
		                                     HumNum scale = 4,
		                                     const std::string& separator = " ");

		// MuseData conversions in Convert-musedata.cpp
      static int       museToBase40        (const std::string& pitchString);
      static std::string musePitchToKernPitch(const std::string& museInput);
		static std::string museClefToKernClef(const std::string& mclef);
		static std::string museKeySigToKernKeySig(const std::string& mkeysig);
		static std::string museTimeSigToKernTimeSig(const std::string& mtimesig);
		static std::string museMeterSigToKernMeterSig(const std::string& mtimesig);
		static std::string museFiguredBassToKernFiguredBass(const std::string& mfb);

		// Harmony processing, defined in Convert-harmony.cpp
		static std::vector<int> minorHScaleBase40(void);
		static std::vector<int> majorScaleBase40 (void);
		static int         keyToInversion   (const std::string& harm);
		static int         keyToBase40      (const std::string& key);
		static std::vector<int> harmToBase40     (HTp harm, const std::string& key) {
		                                        return harmToBase40(*harm, key); }
		static std::vector<int> harmToBase40     (HTp harm, HTp key) {
		                                        return harmToBase40(*harm, *key); }
		static std::vector<int> harmToBase40     (const std::string& harm, const std::string& key);
		static std::vector<int> harmToBase40     (const std::string& harm, int keyroot, int keymode);
		static void        makeAdjustedKeyRootAndMode(const std::string& secondary,
		                                     int& keyroot, int& keymode);
		static int         chromaticAlteration(const std::string& content);

		// data-type specific (other than pitch/rhythm),
		// defined in Convert-kern.cpp
		static bool isKernRest              (const std::string& kerndata);
		static bool isKernNote              (const std::string& kerndata);
		static bool isKernNoteAttack        (const std::string& kerndata);
		static bool hasKernSlurStart        (const std::string& kerndata);
		static bool hasKernSlurEnd          (const std::string& kerndata);
		static int  getKernSlurStartElisionLevel(const std::string& kerndata, int index);
		static int  getKernSlurEndElisionLevel  (const std::string& kerndata, int index);
		static char hasKernStemDirection    (const std::string& kerndata);

		static bool isKernSecondaryTiedNote (const std::string& kerndata);
		static std::string getKernPitchAttributes(const std::string& kerndata);

		// String processing, defined in Convert-string.cpp
		static std::vector<std::string> splitString   (const std::string& data,
		                                     char separator = ' ');
		static void    replaceOccurrences   (std::string& source,
		                                     const std::string& search,
		                                     const std::string& replace);
		static std::string  repeatString         (const std::string& pattern, int count);
		static std::string  encodeXml            (const std::string& input);
		static std::string  getHumNumAttributes  (const HumNum& num);
		static std::string  trimWhiteSpace       (const std::string& input);
		static bool    startsWith           (const std::string& input,
		                                     const std::string& searchstring);
		static bool    contains(const std::string& input, const std::string& pattern);
		static bool    contains(const std::string& input, char pattern);
		static bool    contains(std::string* input, const std::string& pattern);
		static bool    contains(std::string* input, char pattern);
		static void    makeBooleanTrackList(std::vector<bool>& spinelist,
		                                     const std::string& spinestring,
		                                     int maxtrack);
		static std::vector<int> extractIntegerList(const std::string& input, int maximum);
		// private functions for extractIntegerList:
		static void processSegmentEntry(std::vector<int>& field, const std::string& astring, int maximum);
		static void removeDollarsFromString(std::string& buffer, int maximum);

		// Mathematical processing, defined in Convert-math.cpp
		static int     getLcm               (const std::vector<int>& numbers);
		static int     getGcd               (int a, int b);
		static void    primeFactors         (std::vector<int>& output, int n);
		static double  nearIntQuantize      (double value,
		                                    double delta = 0.00001);
		static double  significantDigits    (double value, int digits);
		static bool    isNaN                (double value);
		static double  pearsonCorrelation   (const std::vector<double> &x, const std::vector<double> &y);
		static double  standardDeviation    (const std::vector<double>& x);
		static double  standardDeviationSample(const std::vector<double>& x);
		static double  mean                 (const std::vector<double>& x);
		static int     romanNumeralToInteger(const std::string& roman);
		static double  coefficientOfVariationSample(const std::vector<double>& x);
		static double  coefficientOfVariationPopulation(const std::vector<double>& x);
		static double  nPvi                 (const std::vector<double>& x);
};


// END_MERGE

} // end namespace hum

#endif /* _CONVERT_H_INCLUDED */



