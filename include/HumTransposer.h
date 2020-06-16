//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Tue Dec 03 11:28:21 PDT 2019
// Last Modified: Mon Jun 15 16:35:52 PDT 2020
// Filename:      HumHumTransposer.h
// URL:           https://github.com/craigsapp/hum2ly/blob/master/include/HumHumTransposer.h
// Related:       https://github.com/rism-ch/verovio/blob/develop/include/vrv/transposition.h
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Class for handling tranpositions and calculating musical intervals.
//

#ifndef _HUMTRANSPOSER_H_INCLUDED
#define _HUMTRANSPOSER_H_INCLUDED

#include <iostream>
#include <string>
#include <vector>

#define INVALID_INTERVAL_CLASS -123456789

namespace hum {

class HumHumTransPitch;
class HumHumTransposer;

////////////////////////////////////////////////////////////////////////////
//
// The HumTransPitch class is an interface for storing information about notes that will
// be used in the HumTransposer class.  The diatonic pitch class, chromatic alteration
// of the diatonic pitch and the octave are store in the class.  Names given to the
// parameters are analogous to MEI note attributes.  Note that note@accid can be also
// note/accid in MEI data, and other complications that need to be resolved into
// storing the correct pitch information in HumTransPitch.
//

class HumTransPitch {

	public:
		// diatonic pitch class name of pitch: C = 0, D = 1, ... B = 6.
		int m_diatonic;

		// chromatic alteration of pitch: 0 = natural, 1 = sharp, -2 = flat, +2 = double sharp
		int m_accid;

		// octave number of pitch: 4 = middle-C octave
		int m_oct;

		            HumTransPitch            (void)   {};
		            HumTransPitch            (int aDiatonic, int anAccid, int anOct);
		            HumTransPitch            (const HumTransPitch &pitch);
		            HumTransPitch &operator= (const HumTransPitch &pitch);
		bool        isValid                  (int maxAccid);
		void        setPitch                 (int aDiatonic, int anAccid, int anOct);

};

std::ostream &operator<<(std::ostream &out, const HumTransPitch &pitch);



////////////////////////////////////////////////////////////////////////////
//
// The HumTransposer class is an interface for transposing notes represented in the
// HumTransPitch class format.
//

class HumTransposer {

	public:
		             HumTransposer       (void);
		            ~HumTransposer       (void);

		// Set the interval class for an octave (default is 40, +/- two sharps/flats).
		void         setMaxAccid         (int maxAccid);
		int          getMaxAccid         (void);
		void         setBase40           (void);
		void         setBase600          (void);
		int          getBase             (void);

		// Set the transposition amount for use with Transpose(void) functions.  These functions
		// need to be rerun after SetMaxAccid(void) or SetBase*(void) are called; otherwise, the
		// transposition will be 0/P1/unison.
		bool         setTransposition    (int transVal);
		bool         setTransposition    (const std::string &transString);
		bool         setTransposition    (const HumTransPitch &fromPitch, const std::string &toString);
		bool         setTransposition    (int keyFifths, int semitones);
		bool         setTransposition    (int keyFifths, const std::string &semitones);

		// Accessor functions for retrieving stored transposition interval.
		int         getTranspositionIntervalClass  (void);
		std::string getTranspositionIntervalName   (void);

		// Transpostion based on stored transposition interval.
		void        transpose            (HumTransPitch &pitch);
		int         transpose            (int iPitch);

		// Transpose based on second input parameter (not with stored transposition interval).
		void        transpose            (HumTransPitch &pitch, int transVal);
		void        transpose            (HumTransPitch &pitch, const std::string &transString);

		// Convert between integer intervals and interval name strings:
		std::string getIntervalName      (const HumTransPitch &p1, const HumTransPitch &p2);
		std::string getIntervalName      (int intervalClass);
		int         getInterval          (const std::string &intervalName);

		// Convert between HumTransPitch class and integer pitch and interval representations.
		int humTransPitchToIntegerPitch  (const HumTransPitch &pitch);
		HumTransPitch integerPitchToHumTransPitch(int ipitch);
		int         getInterval          (const HumTransPitch &p1, const HumTransPitch &p2);

		// Convert between Semitones and integer interval representation.
		std::string semitonesToIntervalName  (int keyFifths, int semitones);
		int         semitonesToIntervalClass (int keyFifths, int semitones);
		int         intervalToSemitones      (int intervalClass);
		int         intervalToSemitones      (const std::string &intervalName);

		// Circle-of-fifths related functions.
		int         intervalToCircleOfFifths      (const std::string &transString);
		int         intervalToCircleOfFifths      (int transval);
		std::string circleOfFifthsToIntervalName  (int fifths);
		int         circleOfFifthsToIntervalClass (int fifths);

		// Key-signature related functions.
		bool          getKeyTonic                     (const std::string &keyTonic,
		                                               HumTransPitch &tonic);
		HumTransPitch circleOfFifthsToMajorTonic      (int fifths);
		HumTransPitch circleOfFifthsToMinorTonic      (int fifths);
		HumTransPitch circleOfFifthsToDorianTonic     (int fifths);
		HumTransPitch circleOfFifthsToPhrygianTonic   (int fifths);
		HumTransPitch circleOfFifthsToLydianTonic     (int fifths);
		HumTransPitch circleOfFifthsToMixolydianTonic (int fifths);
		HumTransPitch circleOfFifthsToLocrianTonic    (int fifths);

		// Conversions between diatonic/chromatic system and integer system of intervals.
		std::string diatonicChromaticToIntervalName(int diatonic, int chromatic);
		int  diatonicChromaticToIntervalClass (int diatonic, int chromatic);
		void intervalToDiatonicChromatic      (int &diatonic, int &chromatic, int intervalClass);
		void intervalToDiatonicChromatic      (int &diatonic, int &chromatic,
		                                       const std::string &intervalName);

		// Convenience functions for calculating common interval classes.  Augmented classes
		// can be calculated by adding 1 to perfect/major classes, and diminished classes can be
		// calcualted by subtracting 1 from perfect/minor classes.
		int    perfectUnisonClass   (void);
		int    minorSecondClass     (void);
		int    majorSecondClass     (void);
		int    minorThirdClass      (void);
		int    majorThirdClass      (void);
		int    perfectFourthClass   (void);
		int    perfectFifthClass    (void);
		int    minorSixthClass      (void);
		int    majorSixthClass      (void);
		int    minorSeventhClass    (void);
		int    majorSeventhClass    (void);
		int    perfectOctaveClass   (void);

		// Convenience functions for acessing m_diatonicMapping.
		int getCPitchClass(void) { return m_diatonicMapping[0]; }
		int getDPitchClass(void) { return m_diatonicMapping[1]; }
		int getEPitchClass(void) { return m_diatonicMapping[2]; }
		int getFPitchClass(void) { return m_diatonicMapping[3]; }
		int getGPitchClass(void) { return m_diatonicMapping[4]; }
		int getAPitchClass(void) { return m_diatonicMapping[5]; }
		int getBPitchClass(void) { return m_diatonicMapping[6]; }

		// Input string validity helper functions.
		static bool isValidIntervalName (const std::string &name);
		static bool isValidKeyTonic     (const std::string &name);
		static bool isValidSemitones    (const std::string &name);

	protected:
		// integer representation for perfect octave:
		int m_base;

		// maximum allowable sharp/flats for transposing:
		int m_maxAccid;

		// integer interval class for transposing:
		int m_transpose;

		// pitch integers for each natural diatonic pitch class:
		std::vector<int> m_diatonicMapping;

		// used to calculate semitones between diatonic pitch classes:
		const std::vector<int> m_diatonic2semitone{ 0, 2, 4, 5, 7, 9, 11 };

	private:
		void calculateDiatonicMapping(void);
};

} // namespace hum

#endif
