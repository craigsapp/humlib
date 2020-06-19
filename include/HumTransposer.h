//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Tue Dec 03 11:28:21 PDT 2019
// Last Modified: Thu Jun 18 17:47:07 PDT 2020
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

#include "HumPitch.h"

#include <iostream>
#include <string>
#include <vector>

namespace hum {

// START_MERGE



////////////////////////////////////////////////////////////////////////////
//
// The HumTransposer class is an interface for transposing notes represented in the
// HumPitch class format.
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
		bool         setTransposition    (const HumPitch &fromPitch, const std::string &toString);
		bool         setTransposition    (int keyFifths, int semitones);
		bool         setTransposition    (int keyFifths, const std::string &semitones);
		bool         setTranspositionDC  (int diatonic, int chromatic);

		// Accessor functions for retrieving stored transposition interval.
		int         getTranspositionIntervalClass  (void);
		std::string getTranspositionIntervalName   (void);

		// Transpostion based on stored transposition interval.
		void        transpose            (HumPitch &pitch);
		int         transpose            (int iPitch);

		// Transpose based on second input parameter (not with stored transposition interval).
		void        transpose            (HumPitch &pitch, int transVal);
		void        transpose            (HumPitch &pitch, const std::string &transString);

		// Convert between integer intervals and interval name strings:
		std::string getIntervalName      (const HumPitch &p1, const HumPitch &p2);
		std::string getIntervalName      (int intervalClass);
		int         getInterval          (const std::string &intervalName);

		// Convert between HumPitch class and integer pitch and interval representations.
		int humHumPitchToIntegerPitch    (const HumPitch &pitch);
		HumPitch integerPitchToHumPitch  (int ipitch);
		int         getInterval          (const HumPitch &p1, const HumPitch &p2);

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
		bool       getKeyTonic                     (const std::string &keyTonic,
		                                            HumPitch &tonic);
		HumPitch   circleOfFifthsToMajorTonic      (int fifths);
		HumPitch   circleOfFifthsToMinorTonic      (int fifths);
		HumPitch   circleOfFifthsToDorianTonic     (int fifths);
		HumPitch   circleOfFifthsToPhrygianTonic   (int fifths);
		HumPitch   circleOfFifthsToLydianTonic     (int fifths);
		HumPitch   circleOfFifthsToMixolydianTonic (int fifths);
		HumPitch   circleOfFifthsToLocrianTonic    (int fifths);

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
		static const std::vector<int> m_diatonic2semitone;

	private:
		void calculateDiatonicMapping(void);
};



// END_MERGE

} // namespace hum

#endif /* _HUMTRANSPOSER_H_INCLUDED */
