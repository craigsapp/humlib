//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Tue Dec 03 11:28:21 PDT 2019
// Last Modified: Thu Jun 18 17:51:55 PDT 2020
// Filename:      HumPitch.cpp
// URL:           https://github.com/craigsapp/hum2ly/blob/master/src/HumPitch.cpp
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Class for handling tranpositions and calculating musical intervals.
//

#include "HumPitch.h"
#include "HumRegex.h"

#include <cctype>
#include <iostream>
#include <regex>
#include <string>
#include <vector>

using namespace std;

namespace hum { 

// START_MERGE


const std::vector<char> HumPitch::m_diatonicPC2letterLC({ 'c', 'd', 'e', 'f', 'g', 'a', 'b' });
const std::vector<char> HumPitch::m_diatonicPC2letterUC({ 'C', 'D', 'E', 'F', 'G', 'A', 'B' });

////////////////////////////////////////////////////////////////////////////
//
// The HumPitch class is an interface for storing information about notes which
// will be used in the HumTransposer class.  The diatonic pitch class, chromatic alteration
// of the diatonic pitch and the octave are store in the class.  Names given to the
// parameters are analogous to MEI note attributes.  Note that note@accid can be also
// note/accid in MEI data, and other complications that need to be resolved into
// storing the correct pitch information in HumPitch.
//


//////////////////////////////
//
// HumPitch::HumPitch -- HumPitch constructor.
//

HumPitch::HumPitch(int aDiatonic, int anAccid, int anOct) {
	setPitch(aDiatonic, anAccid, anOct);
}


HumPitch::HumPitch(const HumPitch &pitch) {
	m_diatonicpc = pitch.m_diatonicpc;
	m_accid = pitch.m_accid;
	m_oct = pitch.m_oct;
}



//////////////////////////////
//
// operator= HumPitch -- copy operator for pitches.
//

HumPitch &HumPitch::operator=(const HumPitch &pitch) {
	if (this != &pitch) {
		m_diatonicpc = pitch.m_diatonicpc;
		m_accid = pitch.m_accid;
		m_oct = pitch.m_oct;
	}
	return *this;
}



//////////////////////////////
//
// HumPitch::isValid -- returns true if the absolute value of the accidental
//     is less than or equal to the max accidental value.

bool HumPitch::isValid(int maxAccid) {
	return abs(m_accid) <= abs(maxAccid);
}



//////////////////////////////
//
// HumPitch::setPitch -- Set the attributes for a pitch all at once.
//

void HumPitch::setPitch(int aDiatonic, int anAccid, int anOct) {
	m_diatonicpc = aDiatonic;
	m_accid = anAccid;
	m_oct = anOct;
}



//////////////////////////////
//
// HumPitch::isRest -- returns true if a rest, which means that m_diatonicpc is negative.
//

bool HumPitch::isRest(void) const {
	return m_diatonicpc < 0 ? true : false;
}



//////////////////////////////
//
// HumPitch::makeRest -- 
//

void HumPitch::makeRest(void) {
	m_diatonicpc = -1;
	m_accid = 0;
	m_oct = 0;
}



//////////////////////////////
//
// HumPitch::getOctave -- Middle C is the start of the 4th octave.
//

int HumPitch::getOctave(void) const {
	return m_oct;
}



//////////////////////////////
//
// HumPitch::getAccid -- +1=sharp, -1=flat, +2=double-sharp, etc.
//   Maybe expand to a double later for quarter tones, etc.
//

int HumPitch::getAccid(void) const {
	return m_accid;
}



//////////////////////////////
//
// HumPitch::getDiatonicPitchClass --  Return the diatonic pitch class:
//   0 = C, 1 = D, 2 = E, 3 = F, 4 = G, 5 = A, 6 = B, -1 = rest.
//

int HumPitch::getDiatonicPitchClass(void) const {
	return m_diatonicpc;
}


int HumPitch::getDiatonicPC(void) const {
	return getDiatonicPitchClass();
}



//////////////////////////////
//
// HumPitch::setOctave -- Set the octave number of the pitch, with 4 meaning
//   the middle-C octave.
//

void HumPitch::setOctave(int anOct) {
	m_oct = anOct;
}



//////////////////////////////
//
// HumPitch::setAccid -- +1 = sharp, -1 = flat, +2 = double-sharp.
//
void HumPitch::setAccid(int anAccid) {
	m_accid = anAccid;
}



//////////////////////////////
//
// HumPitch::makeSharp -- Set the accidental to +1.
//

void HumPitch::makeSharp(void) {
	m_accid = 1;
}



//////////////////////////////
//
// HumPitch::makeFlat -- Set the accidental to -1.
//

void HumPitch::makeFlat(void) {
	m_accid = -1;
}



//////////////////////////////
//
// HumPitch::makeNatural -- Set the accidental to -1.
//

void HumPitch::makeNatural(void) {
	m_accid = 0;
}



//////////////////////////////
//
// HumPitch::setDiatonicPitchClass --
//

void HumPitch::setDiatonicPitchClass(int aDiatonicPC) {
	if (aDiatonicPC < 0) {
		m_diatonicpc = -1;
	} else if (aDiatonicPC < 7) {
		m_diatonicpc = aDiatonicPC;
	} else if (aDiatonicPC >= 'A' && aDiatonicPC <= 'G') {
		m_diatonicpc = (aDiatonicPC - 'A' + 5) % 7;
	} else if (aDiatonicPC >= 'a' && aDiatonicPC <= 'g') {
		m_diatonicpc = (aDiatonicPC - 'a' + 5) % 7;
	} else {
		m_diatonicpc = -1;
	}
}



//////////////////////////////
//
// HumPitch::setDiatonicPC --
//

void HumPitch::setDiatonicPC(int aDiatonicPC) {
	setDiatonicPitchClass(aDiatonicPC);
}



//////////////////////////////
//
// operator<< HumPitch -- Print pitch data as string for debugging.
//

ostream &operator<<(ostream &out, const HumPitch &pitch) {
	switch (pitch.getDiatonicPC()) {
		case dpc_C: out << "C"; break;
		case dpc_D: out << "D"; break;
		case dpc_E: out << "E"; break;
		case dpc_F: out << "F"; break;
		case dpc_G: out << "G"; break;
		case dpc_A: out << "A"; break;
		case dpc_B: out << "B"; break;
		default: out << "R";
	}
	if (pitch.getAccid() > 0) {
		for (int i = 0; i < pitch.getAccid(); i++) {
			out << "#";
		}
	} else if (pitch.getAccid() < 0) {
		for (int i = 0; i < abs(pitch.getAccid()); i++) {
			out << "b";
		}
	}
	out << pitch.getOctave();
	return out;
}



//////////////////////////////
//
// HumPitch::getKernPitch -- Return the pitch as a **kern pitch name.
//

string HumPitch::getKernPitch(void) const {
	if (m_diatonicpc < 0) {
		return "r";
	}

	int count;
	char diatonic;
	if (m_oct < 4) {
		diatonic = m_diatonicPC2letterUC.at(m_diatonicpc);
		count = 4 - m_oct;
	} else {
		count = m_oct - 4 + 1;
		diatonic = m_diatonicPC2letterLC.at(m_diatonicpc);
	}
	string output;
	output = diatonic;
	for (int i=1; i<count; i++) {
		output += diatonic;
	}
	if (m_accid != 0) {
		if (m_accid < 0) {
			for (int i=0; i<-m_accid; i++) {
				output += '-';
			}
		} else {
			for (int i=0; i<m_accid; i++) {
				output += '#';
			}
		}
	}
	return output;
}



//////////////////////////////
//
// HumPitch::setKernPitch -- Set the pitch from a **kern pitch name.
//

bool HumPitch::setKernPitch(const string& kern) {
	makeRest();
	HumRegex hre;
	if (kern.find('r') != string::npos) {
		// rests can have pitch information, but ignore.
		return true;
	}
	if (!hre.search(kern, "(A+|B+|C+|D+|E+|F+|G+|a+|b+|c+|d+|e+|f+|g+)(-+|#+)?")) {
		return false;
	}
	string letters = hre.getMatch(1);
	string accidentals = hre.getMatch(2);

	if (!accidentals.empty()) {
		m_accid = (int)accidentals.size();
		if (accidentals[0] == '-') {
			m_accid = -m_accid;
		}
	}
	int lcount = (int)letters.size();
	m_oct = islower(letters[0]) ? 3 + lcount : 4 - lcount;
	m_diatonicpc = (tolower(letters[0]) - 'a' + 5) % 7;
	return true;
}



//////////////////////////////
//
// HumPitch::getScientificPitch -- Returns the **pitch representation of the pitch.
//    Examples: Eb4, F#3, C-2.
//    Format:    [A-G](b+|#+)?-?\d+
//

std::string HumPitch::getScientificPitch(void) const {
	if (m_diatonicpc < 0) {
		return "R";
	}
	string output;
	output = m_diatonicPC2letterUC.at(m_diatonicpc);
	if (m_accid < 0) {
		for (int i=0; i<-m_accid; i++) {
			output += 'b';
		}
	} else if (m_accid > 0) {
		for (int i=0; i<m_accid; i++) {
			output += '#';
		}
	}
	output = to_string(m_oct);
	return output;
}



//////////////////////////////
//
// HumPitch::setScientificPitch --
//

bool HumPitch::setScientificPitch(const std::string& pitch) {
	makeRest();

	HumRegex hre;
	if (!hre.search(pitch, "([A-Ga-g])(b+|#+)?(-?\\d+)")) {
		return false;
	}
	string diatonic = hre.getMatch(1);
	string accidental = hre.getMatch(2);
	m_oct = hre.getMatchInt(3);
	if (!accidental.empty()) {
		m_accid = (int)accidental.size();
		if (accidental[0] == 'f') {
			m_accid = -m_accid;
		}
	}
	m_diatonicpc = (toupper(diatonic[0]) - 'A' + 5) % 7;
	return true;
}



// END_MERGE

} // namespace hum
