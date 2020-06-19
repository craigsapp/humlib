//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Thu Jun 18 17:40:50 PDT 2020
// Last Modified: Thu Jun 18 17:40:53 PDT 2020
// Filename:      HumPitch.h
// URL:           https://github.com/craigsapp/hum2ly/blob/master/include/HumPitch.h
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Class for generic pitch description for use in transposition
//                and interval calculations.
//

#ifndef _HUMPITCH_H_INCLUDED
#define _HUMPITCH_H_INCLUDED

#include <iostream>
#include <string>
#include <vector>

namespace hum {

// START_MERGE

#define INVALID_INTERVAL_CLASS -123456789

// Diatonic pitch class integers:
// These could be converted into an enum provided
// that the same value are assigned to each class.
#define dpc_rest -1 /* Any negative value should be treated as a rest */
#define dpc_C 0 /* Integer for Diatonic pitch class for C */
#define dpc_D 1
#define dpc_E 2
#define dpc_F 3
#define dpc_G 4
#define dpc_A 5
#define dpc_B 6


////////////////////////////////////////////////////////////////////////////
//
// The HumPitch class is an interface for storing information about notes that will
// be used in the HumTransposer class.  The diatonic pitch class, chromatic alteration
// of the diatonic pitch and the octave are store in the class.  Names given to the
// parameters are analogous to MEI note attributes.  Note that note@accid can be also
// note/accid in MEI data, and other complications that need to be resolved into
// storing the correct pitch information in HumPitch.
//

class HumPitch {

	public:

		            HumPitch              (void)   {};
		            HumPitch              (int aDiatonic, int anAccid, int anOct);
		            HumPitch              (const HumPitch &pitch);
		            HumPitch &operator=   (const HumPitch &pitch);
		bool        isValid               (int maxAccid);
		void        setPitch              (int aDiatonic, int anAccid, int anOct);

		bool        isRest                (void) const;
		void        makeRest              (void);

		int         getOctave             (void) const;
		int         getAccid              (void) const;
		int         getDiatonicPitchClass (void) const;
		int         getDiatonicPC         (void) const;

		void        setOctave             (int anOct);
		void        setAccid              (int anAccid);
		void        makeSharp             (void);
		void        makeFlat              (void);
		void        makeNatural           (void);
		void        setDiatonicPitchClass (int aDiatonicPC);
		void        setDiatonicPC         (int aDiatonicPC);


		// conversions in/out of various representations:
		std::string getKernPitch          (void) const;
		bool        setKernPitch          (const std::string& kern);
		std::string getScientificPitch    (void) const;
		bool        setScientificPitch    (const std::string& pitch);

	protected:

		// diatonic pitch class name of pitch: C = 0, D = 1, ... B = 6.
		int m_diatonicpc;

		// chromatic alteration of pitch: 0 = natural, 1 = sharp, -2 = flat, +2 = double sharp
		int m_accid;

		// octave number of pitch: 4 = middle-C octave
		int m_oct;

		// used to convert to other formats:
		static const std::vector<char> m_diatonicPC2letterLC;
		static const std::vector<char> m_diatonicPC2letterUC;

};

std::ostream &operator<<(std::ostream &out, const HumPitch &pitch);



// END_MERGE

} // namespace hum

#endif /* _HUMPITCH_H_INCLUDED */
