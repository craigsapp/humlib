//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Tue Dec 03 11:28:21 PDT 2019
// Last Modified: Mon Jun 15 16:55:07 PDT 2020
// Filename:      HumTransposer.cpp
// URL:           https://github.com/craigsapp/hum2ly/blob/master/src/HumTransposer.cpp
// Related:       https://github.com/rism-ch/verovio/blob/develop/src/transposition.cpp
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Class for handling tranpositions and calculating musical intervals.
//

//
// References:
//            http://www.ccarh.org/publications/reprints/base40
//            https://github.com/craigsapp/humlib/blob/master/src/tool-transpose.cpp
//
// Description:   Draft implementation of a transposition system for verovio.
//                There is a main() function at the bottom of the file for demo/testing.
//                There are two classes in this file:
//                   HumPitch: pitch representation as three integers:
//                            m_diatonicpc: diatonic pitch class integer from C=0 to B=6.
//                            m_accid: chromatic alterations in semitones (0=natural, -1=flat).
//                            m_oct: octave number (4 = middle-C octave).
//                   HumTransposer: transposition system which uses HumPitch as a user interface.
//                   (Add MEI to HumPitch conversions in HumPitch class, or use external
//                    code to interface to verovio attributes for <note>).
//                   The default maximum accidental handling is +/- two sharps/flats (base-40).
//                   Use the HumTransposer::setMaxAccid() to set the maximum allowed accidental
//                   count.  HumTransposer::setBase40() is equivalent to HumTransposer::setMaxAccid(2),
//                   and HumTransposer::setBase600() is equivalent to HumTransposer::setMaxAccid(42).
//
// Todo: Probably useful to add an autowrap feature to force unrepresentable pitches to be
//     moved to enharmonic equivalent pitches (better than leaving a pitch undefined).
//     For example, F#### in a system that cannot represent more than two or three
//     sharps would be converted to G##, probably with a warning message.  From F####
//     to G## is up a diminished second ("d2").
//

#include "HumTransposer.h"

#include <cctype>
#include <iostream>
#include <regex>
#include <string>
#include <vector>

using namespace std;

namespace hum {

// START_MERGE


const std::vector<int> HumTransposer::m_diatonic2semitone({ 0, 2, 4, 5, 7, 9, 11 });


//////////////////////////////
//
// HumTransposer::HumTransposer -- HumTransposer constructor.
//

HumTransposer::HumTransposer() {
	// Initialize with base-600 system by default:
	setMaxAccid(42);
}



//////////////////////////////
//
// HumTransposer::~HumTransposer -- HumTransposer deconstructor.
//

HumTransposer::~HumTransposer() {
	// do nothing;
}



//////////////////////////////
//
// HumTransposer::setTransposition -- Set the transposition value which is an
//   interval class in the current base system.  When HumTransposer::setMaxAccid()
//   or HumTransposer::setBase*() are called, the transposition value will be set
//   to 0 (a perfect unison).  The integer is a base-40 class of number.  If you
//   want to transpose by semitone, do not use this option but rather the
//   setHumTransposer(int keyFifths, string semitones) function or the
//   setHumTransposer(int keyFifths, int semitones) function that are defined
//   further below.
//

bool HumTransposer::setTransposition(int transVal) {
	m_transpose = transVal;
	return true;
}

// Use a string to set the interval class in the current base system.  For example,
//  "+M2" means up a major second, which is the integer 6 in base-40.

bool HumTransposer::setTransposition(const string &transString) {
	m_transpose = getInterval(transString);
	return m_transpose != INVALID_INTERVAL_CLASS;
}

// Set transposition interval based on two pitches that represent the source data
// key tonic and the target key tonic.

bool HumTransposer::setTransposition(const HumPitch &fromPitch, const string &toString) {
	HumPitch toPitch;
	if (getKeyTonic(toString, toPitch)) {
		// Determine proper octave offset.
		int numSigns = toPitch.getOctave();
		m_transpose = getInterval(fromPitch, toPitch);
		// A transposition with n plus or minus signs should never be more than n octaves away.
		if (numSigns > 0 && m_transpose > perfectOctaveClass() * numSigns) {
			m_transpose -= perfectOctaveClass();
		}
		else if (numSigns < 0 && m_transpose < perfectOctaveClass() * numSigns) {
			m_transpose += perfectOctaveClass();
		}
		// A transposition with 0 plus or minus signs should never be more than 1/2 an octave away.
		else if (numSigns == 0 && m_transpose > perfectOctaveClass() / 2) {
			m_transpose -= perfectOctaveClass();
		}
		else if (numSigns == 0 && m_transpose < -1 * perfectOctaveClass() / 2) {
			m_transpose += perfectOctaveClass();
		}
		return true;
	}
	return false;
}

// Set the transposition based on the key signature (or inferred key signature coming from
// the keySig@diatonic/keySig@accid/keySig@mode information) and a string containing the
// semitone transposition.

bool HumTransposer::setTransposition(int keyFifths, const string &semitones) {
	if (!isValidSemitones(semitones)) {
		return false;
	}
	int semis = stoi(semitones);
	return setTransposition(keyFifths, semis);
}

// Note the order of the variables (key signature information is first in all
// cases where there are two input parametrs to setTransposition().

bool HumTransposer::setTransposition(int keyFifths, int semitones) {
	int intervalClass = semitonesToIntervalClass(keyFifths, semitones);
	return setTransposition(intervalClass);
}



//////////////////////////////
//
// HumTransposer::setTranspositionDC --
//

bool HumTransposer::setTranspositionDC(int diatonic, int chromatic) {
	int interval = HumTransposer::diatonicChromaticToIntervalClass(diatonic, chromatic);
	return setTransposition(interval);
}



//////////////////////////////
//
// HumTransposer::semitonesToIntervalClass -- convert semitones plus key
//     signature information into an integer interval class.
//

int HumTransposer::semitonesToIntervalClass(int keyFifths, int semitones) {
	int sign = semitones < 0 ? -1 : +1;
	semitones = semitones < 0 ? -semitones : semitones;
	int octave = semitones / 12;
	semitones = semitones - octave * 12;
	int sum1, sum2;
	string interval = "P1";
	switch (semitones) {
		case 0: interval = "P1"; break;

		case 1:
			sum1 = keyFifths - 5 * sign;
			sum2 = keyFifths + 7 * sign;
			interval = abs(sum1) < abs(sum2) ? "m2" : "A1";
			break;

		case 2:
			sum1 = keyFifths + 2 * sign;
			sum2 = keyFifths - 10 * sign;
			interval = abs(sum1) < abs(sum2) ? "M2" : "d3";
			break;

		case 3:
			sum1 = keyFifths - 3 * sign;
			sum2 = keyFifths + 9 * sign;
			interval = abs(sum1) < abs(sum2) ? "m3" : "A2";
			break;

		case 4:
			sum1 = keyFifths + 4 * sign;
			sum2 = keyFifths - 8 * sign;
			interval = abs(sum1) < abs(sum2) ? "M3" : "d4";
			break;

		case 5:
			sum1 = keyFifths - 1 * sign;
			sum2 = keyFifths + 11 * sign;
			interval = abs(sum1) < abs(sum2) ? "P4" : "A3";
			break;

		case 6:
			sum1 = keyFifths + 6 * sign;
			sum2 = keyFifths - 6 * sign;
			interval = abs(sum1) < abs(sum2) ? "A4" : "d5";
			break;

		case 7:
			sum1 = keyFifths + 1 * sign;
			sum2 = keyFifths - 11 * sign;
			interval = abs(sum1) < abs(sum2) ? "P5" : "d6";
			break;

		case 8:
			sum1 = keyFifths - 4 * sign;
			sum2 = keyFifths + 8 * sign;
			interval = abs(sum1) < abs(sum2) ? "m6" : "A5";
			break;

		case 9:
			sum1 = keyFifths + 3 * sign;
			sum2 = keyFifths - 9 * sign;
			interval = abs(sum1) < abs(sum2) ? "M6" : "d7";
			break;

		case 10:
			sum1 = keyFifths - 2 * sign;
			sum2 = keyFifths + 10 * sign;
			interval = abs(sum1) < abs(sum2) ? "m7" : "A6";
			break;

		case 11:
			sum1 = keyFifths + 5 * sign;
			sum2 = keyFifths - 7 * sign;
			interval = abs(sum1) < abs(sum2) ? "M7" : "d8";
			break;
	}

	interval = sign < 0 ? "-" + interval : "+" + interval;
	int intint = getInterval(interval);
	intint += sign * octave * m_base;
	return intint;
}



//////////////////////////////
//
// HumTransposer::semitonesToIntervalName -- convert semitones plus key
//     signature information into an interval name string.
//

string HumTransposer::semitonesToIntervalName(int keyFifths, int semitones) {
	int intervalClass = semitonesToIntervalClass(keyFifths, semitones);
	return getIntervalName(intervalClass);
}



//////////////////////////////
//
// HumTransposer::intervalToSemitones --  Convert a base interval class into
//   semitones.  Multiple enharmonic equivalent interval classes will collapse into
//   a single semitone value, so the process is not completely reversable
//   by calling HumTransposer::semitonesToIntervalClass(), but for simple
//   intervals it will be reversable.
//

int HumTransposer::intervalToSemitones(int interval) {
	int sign = interval < 0 ? -1 : +1;
	interval = interval < 0 ? -interval : interval;
	int octave = interval / m_base;
	int intervalClass = interval - octave * m_base;
	int diatonic = 0;
	int chromatic = 0;
	intervalToDiatonicChromatic(diatonic, chromatic, intervalClass);
	if ((diatonic != INVALID_INTERVAL_CLASS) && (chromatic != INVALID_INTERVAL_CLASS)) {
		return (m_diatonic2semitone.at(diatonic) + chromatic) * sign + 12 * octave;
	}
	else {
		return INVALID_INTERVAL_CLASS;
	}
}

//  Conversion from an interval name string into semitones:

int HumTransposer::intervalToSemitones(const string &intervalName) {
	int interval = getInterval(intervalName);
	return intervalToSemitones(interval);
}



//////////////////////////////
//
// HumTransposer::getTranspositionIntervalClass -- return the interval class integer
//   that was set for use with HumTransposer::HumTransposer.
//

int HumTransposer::getTranspositionIntervalClass() {
	return m_transpose;
}



//////////////////////////////
//
// HumTransposer::getTranspositionIntervalClass -- return the interval integer
//   as a string name that was set for use with HumTransposer::HumTransposer.
//
string HumTransposer::getTranspositionIntervalName() {
	return getIntervalName(m_transpose);
}



//////////////////////////////
//
// HumTransposer::transpose -- Do a transposition at the stored transposition interval, or
//   with a temporary provided integer interval class, or a temporary interval name.
//

void HumTransposer::transpose(HumPitch &pitch) {
	int ipitch = humHumPitchToIntegerPitch(pitch);
	ipitch += m_transpose;
	pitch = integerPitchToHumPitch(ipitch);
}

int HumTransposer::transpose(int ipitch) {
	return ipitch + m_transpose;
}

// Use a temporary transposition value in the following
// two functions. To save for later use of HumTransposer::HumTransposer
// without specifying the transposition interval, store
// transposition value with HumTransposer::setTransposition() first.

void HumTransposer::transpose(HumPitch &pitch, int transVal) {
	int ipitch = humHumPitchToIntegerPitch(pitch);
	ipitch += transVal;
	pitch = integerPitchToHumPitch(ipitch);
}

void HumTransposer::transpose(HumPitch &pitch, const string &transString) {
	int transVal = getInterval(transString);
	int ipitch = humHumPitchToIntegerPitch(pitch);
	ipitch += transVal;
	pitch = integerPitchToHumPitch(ipitch);
}



//////////////////////////////
//
// HumTransposer::getBase -- Return the integer interval class representing an octave.
//

int HumTransposer::getBase() {
	return m_base;
}



//////////////////////////////
//
// HumTransposer::getMaxAccid -- Return the maximum possible absolute accidental value
//     that can be represented by the current transposition base.
//

int HumTransposer::getMaxAccid() {
	return m_maxAccid;
}



//////////////////////////////
//
// HumTransposer::setMaxAccid -- Calculate variables related to a specific base system.
//

void HumTransposer::setMaxAccid(int maxAccid) {
	m_maxAccid = abs(maxAccid);
	m_base = 7 * (2 * m_maxAccid + 1) + 5;
	calculateDiatonicMapping();
	m_transpose = 0;
}



//////////////////////////////
//
// HumTransposer::calculateDiatonicMaping -- Calculate the integer values for the
//    natural diatonic pitch classes: C, D, E, F, G, A, and B in the current
//    base system.
//

void HumTransposer::calculateDiatonicMapping() {
	int m2 = m_maxAccid * 2 + 1;
	int M2 = m2 + 1;
	m_diatonicMapping.resize(7);
	m_diatonicMapping[dpc_C] = m_maxAccid;
	m_diatonicMapping[dpc_D] = m_diatonicMapping[dpc_C] + M2;
	m_diatonicMapping[dpc_E] = m_diatonicMapping[dpc_D] + M2;
	m_diatonicMapping[dpc_F] = m_diatonicMapping[dpc_E] + m2;
	m_diatonicMapping[dpc_G] = m_diatonicMapping[dpc_F] + M2;
	m_diatonicMapping[dpc_A] = m_diatonicMapping[dpc_G] + M2;
	m_diatonicMapping[dpc_B] = m_diatonicMapping[dpc_A] + M2;
}



//////////////////////////////
//
// HumTransposer::getKeyTonic -- Convert a key tonic string into a HumPitch
//      where the octave is the direction it should go.
//      Should conform to the following regular expression:
//          ([+]*|[-]*)([A-Ga-g])([Ss#]*|[Ffb]*)

bool HumTransposer::getKeyTonic(const string &keyTonic, HumPitch &tonic) {
	int octave = 0;
	int pitch = 0;
	int accid = 0;
	int state = 0;
	for (unsigned int i = 0; i < (unsigned int)keyTonic.size(); i++) {
		switch (state) {
			case 0:
				switch (keyTonic[i]) {
					case '-': octave--; break;
					case '+': octave++; break;
					default:
						state++;
						i--;
						break;
				}
				break;
			case 1:
				state++;
				switch (keyTonic[i]) {
					case 'C':
					case 'c': pitch = 0; break;
					case 'D':
					case 'd': pitch = 1; break;
					case 'E':
					case 'e': pitch = 2; break;
					case 'F':
					case 'f': pitch = 3; break;
					case 'G':
					case 'g': pitch = 4; break;
					case 'A':
					case 'a': pitch = 5; break;
					case 'B':
					case 'b': pitch = 6; break;
					default:
						cerr << "Invalid keytonic pitch character: " << keyTonic[i] << endl;
						return false;
				}
				break;
			case 2:
				switch (keyTonic[i]) {
					case 'F':
					case 'f':
					case 'b': accid--; break;
					case 'S':
					case 's':
					case '#': accid++; break;
					default:
						cerr << "Invalid keytonic accid character: " << keyTonic[i] << endl;
						return false;
				}
				break;
		}
	}

	tonic = HumPitch(pitch, accid, octave);
	return true;
}



//////////////////////////////
//
// HumTransposer::getInterval -- Convert a diatonic interval with chromatic
//     quality and direction into an integer interval class.   Input string
//     is in the format: direction + quality + diatonic interval.
//     Such as +M2 for up a major second, -P5 is down a perfect fifth.
//     Regular expression that the string should conform to:
//            (-|\+?)([Pp]|M|m|[aA]+|[dD]+)(\d+)
//

int HumTransposer::getInterval(const string &intervalName) {
	string direction;
	string quality;
	string number;
	int state = 0;

	for (int i = 0; i < (int)intervalName.size(); i++) {
		switch (state) {
			case 0: // direction or quality expected
				switch (intervalName[i]) {
					case '-': // interval is down
						direction = "-";
						state++;
						break;
					case '+': // interval is up
						direction += "";
						state++;
						break;
					default: // interval is up by default
						direction += "";
						state++;
						i--;
						break;
				}
				break;

			case 1: // quality expected
				if (isdigit(intervalName[i])) {
					state++;
					i--;
				}
				else {
					switch (intervalName[i]) {
						case 'M': // major
							quality = "M";
							break;
						case 'm': // minor
							quality = "m";
							break;
						case 'P': // perfect
						case 'p': quality = "P"; break;
						case 'D': // diminished
						case 'd': quality += "d"; break;
						case 'A': // augmented
						case 'a': quality += "A"; break;
					}
				}
				break;

			case 2: // digit expected
				if (isdigit(intervalName[i])) {
					number += intervalName[i];
				}
				break;
		}
	}

	if (quality.empty()) {
		cerr << "Interval name requires a chromatic quality: " << intervalName << endl;
		return INVALID_INTERVAL_CLASS;
	}

	if (number.empty()) {
		cerr << "Interval name requires a diatonic interval number: " << intervalName << endl;
		return INVALID_INTERVAL_CLASS;
	}

	int dnum = stoi(number);
	if (dnum == 0) {
		cerr << "Integer interval number cannot be zero: " << intervalName << endl;
		return INVALID_INTERVAL_CLASS;
	}
	dnum--;
	int octave = dnum / 7;
	dnum = dnum - octave * 7;

	int base = 0;
	int adjust = 0;

	switch (dnum) {
		case 0: // unison
			base = perfectUnisonClass();
			if (quality[0] == 'A') {
				adjust = (int)quality.size();
			}
			else if (quality[0] == 'd') {
				adjust = -(int)quality.size();
			}
			else if (quality != "P") {
				cerr << "Error in interval quality: " << intervalName << endl;
				return INVALID_INTERVAL_CLASS;
			}
			break;
		case 1: // second
			if (quality == "M") {
				base = majorSecondClass();
			}
			else if (quality == "m") {
				base = minorSecondClass();
			}
			else if (quality[0] == 'A') {
				base = majorSecondClass();
				adjust = (int)quality.size();
			}
			else if (quality[0] == 'd') {
				base = minorSecondClass();
				adjust = -(int)quality.size();
			}
			else {
				cerr << "Error in interval quality: " << intervalName << endl;
				return INVALID_INTERVAL_CLASS;
			}
			break;
		case 2: // third
			if (quality == "M") {
				base = majorThirdClass();
			}
			else if (quality == "m") {
				base = minorThirdClass();
			}
			else if (quality[0] == 'A') {
				base = majorThirdClass();
				adjust = (int)quality.size();
			}
			else if (quality[0] == 'd') {
				base = minorThirdClass();
				adjust = -(int)quality.size();
			}
			else {
				cerr << "Error in interval quality: " << intervalName << endl;
				return INVALID_INTERVAL_CLASS;
			}
			break;
		case 3: // fourth
			base = perfectFourthClass();
			if (quality[0] == 'A') {
				adjust = (int)quality.size();
			}
			else if (quality[0] == 'd') {
				adjust = -(int)quality.size();
			}
			else if (quality != "P") {
				cerr << "Error in interval quality: " << intervalName << endl;
				return INVALID_INTERVAL_CLASS;
			}
			break;
		case 4: // fifth
			base = perfectFifthClass();
			if (quality[0] == 'A') {
				adjust = (int)quality.size();
			}
			else if (quality[0] == 'd') {
				adjust = -(int)quality.size();
			}
			else if (quality != "P") {
				cerr << "Error in interval quality: " << intervalName << endl;
				return INVALID_INTERVAL_CLASS;
			}
			break;
		case 5: // sixth
			if (quality == "M") {
				base = majorSixthClass();
			}
			else if (quality == "m") {
				base = minorSixthClass();
			}
			else if (quality[0] == 'A') {
				base = majorSixthClass();
				adjust = (int)quality.size();
			}
			else if (quality[0] == 'd') {
				base = minorSixthClass();
				adjust = -(int)quality.size();
			}
			else {
				cerr << "Error in interval quality: " << intervalName << endl;
				return INVALID_INTERVAL_CLASS;
			}
			break;
		case 6: // seventh
			if (quality == "M") {
				base = majorSeventhClass();
			}
			else if (quality == "m") {
				base = minorSeventhClass();
			}
			else if (quality[0] == 'A') {
				base = majorSeventhClass();
				adjust = (int)quality.size();
			}
			else if (quality[0] == 'd') {
				base = minorSeventhClass();
				adjust = -(int)quality.size();
			}
			else {
				cerr << "Error in interval quality: " << intervalName << endl;
				return INVALID_INTERVAL_CLASS;
			}
			break;
	}

	if (direction == "-") {
		return -((octave * m_base) + base + adjust);
	}
	else {
		return (octave * m_base) + base + adjust;
	}
}



//////////////////////////////
//
// HumTransposer::perfectUnisonClass -- Return the integer interval class
//     representing a perfect unison.
//

int HumTransposer::perfectUnisonClass() {
	return 0;
}



//////////////////////////////
//
// HumTransposer::minorSecondClass -- Return the integer interval class
//     representing a minor second.
//

int HumTransposer::minorSecondClass() {
	return m_diatonicMapping[3] - m_diatonicMapping[2]; // F - E
}



//////////////////////////////
//
// HumTransposer::majorSecondClass -- Return the integer interval class
//    representing a major second.
//

int HumTransposer::majorSecondClass() {
	return m_diatonicMapping[1] - m_diatonicMapping[0]; // D - C
}



//////////////////////////////
//
// HumTransposer::minorThirdClass -- Return the integer interval class
//    representing a minor third.
//

int HumTransposer::minorThirdClass() {
	return m_diatonicMapping[3] - m_diatonicMapping[1]; // F - D
}



//////////////////////////////
//
// HumTransposer::majorThirdClass -- Return the integer interval class
//    representing a major third.
//

int HumTransposer::majorThirdClass() {
	return m_diatonicMapping[2] - m_diatonicMapping[0]; // E - C
}



//////////////////////////////
//
// HumTransposer::perfectFourthClass -- Return the integer interval class
//    representing a perfect fourth.
//

int HumTransposer::perfectFourthClass() {
	return m_diatonicMapping[3] - m_diatonicMapping[0]; // F - C
}



//////////////////////////////
//
// HumTransposer::perfectFifthClass -- Return the integer interval class
//    representing a perfect fifth.
//

int HumTransposer::perfectFifthClass() {
	return m_diatonicMapping[4] - m_diatonicMapping[0]; // G - C
}



//////////////////////////////
//
// HumTransposer::minorSixthClass -- Return the integer interval class
//    representing a minor sixth.
//

int HumTransposer::minorSixthClass() {
	return m_diatonicMapping[5] - m_diatonicMapping[0] - 1; // A - C - 1;
}



//////////////////////////////
//
// HumTransposer::majorSixthClass -- Return the integer interval class
//    representing a major sixth.
//

int HumTransposer::majorSixthClass() {
	return m_diatonicMapping[5] - m_diatonicMapping[0]; // A - C
}



//////////////////////////////
//
// HumTransposer::minorSeventhClass -- Return the integer interval class
//    representing a minor sixth.
//

int HumTransposer::minorSeventhClass() {
	return m_diatonicMapping[6] - m_diatonicMapping[0] - 1; // B - C - 1
}



//////////////////////////////
//
// HumTransposer::majorSeventhClass -- Return the integer interval class
//    representing a major sixth.
//

int HumTransposer::majorSeventhClass() {
	return m_diatonicMapping[6] - m_diatonicMapping[0]; // B - C
}



//////////////////////////////
//
// HumTransposer::octaveClass -- Return the integer interval class
//    representing a major second.
//

int HumTransposer::perfectOctaveClass() {
	return m_base;
}



//////////////////////////////
//
// HumTransposer::humHumPitchToIntegerPitch -- Convert a pitch (octave/diatonic pitch class/chromatic
//     alteration) into an integer value according to the current base.
//

int HumTransposer::humHumPitchToIntegerPitch(const HumPitch &pitch) {
	return pitch.getOctave() * m_base + m_diatonicMapping[pitch.getDiatonicPC()] + pitch.getAccid();
}



//////////////////////////////
//
// HumTransposer::integerPitchToHumPitch -- Convert an integer within the current base
//    into a pitch (octave/diatonic pitch class/chromatic alteration).  Pitches
//    with negative octaves will have to be tested.
//

HumPitch HumTransposer::integerPitchToHumPitch(int ipitch) {
	HumPitch pitch;
	pitch.setOctave(ipitch / m_base);
	int chroma = ipitch - pitch.getOctave() * m_base;
	int mindiff = -1000;
	int mini = -1;

	int targetdiff = m_maxAccid;

	if (chroma > m_base / 2) {
		// search from B downwards
		mindiff = chroma - m_diatonicMapping.back();
		mini = (int)m_diatonicMapping.size() - 1;
		for (int i = (int)m_diatonicMapping.size() - 2; i >= 0; i--) {
			int diff = chroma - m_diatonicMapping[i];
			if (abs(diff) < abs(mindiff)) {
				mindiff = diff;
				mini = i;
			}
			if (abs(mindiff) <= targetdiff) {
				break;
			}
		}
	}
	else {
		// search from C upwards
		mindiff = chroma - m_diatonicMapping[0];
		mini = 0;
		for (int i = 1; i < (int)m_diatonicMapping.size(); i++) {
			int diff = chroma - m_diatonicMapping[i];
			if (abs(diff) < abs(mindiff)) {
				mindiff = diff;
				mini = i;
			}
			if (abs(mindiff) <= targetdiff) {
				break;
			}
		}
	}
	pitch.setDiatonicPC(mini);
	pitch.setAccid(mindiff);
	return pitch;
}



//////////////////////////////
//
// HumTransposer::setBase40 -- Standard chromatic alteration mode, allowing up to double sharp/flats.
//

void HumTransposer::setBase40() {
	setMaxAccid(2);
}



//////////////////////////////
//
// HumTransposer::setBase600 -- Extended chromatic alteration mode, allowing up to 42 sharp/flats.
//

void HumTransposer::setBase600() {
	setMaxAccid(42);
}



//////////////////////////////
//
// HumTransposer::getInterval -- Return the interval between two pitches.
//    If the second pitch is higher than the first, then the interval will be
//    positive; otherwise, the interval will be negative.
//

int HumTransposer::getInterval(const HumPitch &p1, const HumPitch &p2) {
	return humHumPitchToIntegerPitch(p2) - humHumPitchToIntegerPitch(p1);
}

// Similar function to getInterval, but the integer interval class is converted
// into a string that is not dependent on a base:

string HumTransposer::getIntervalName(const HumPitch &p1, const HumPitch &p2) {
	int iclass = getInterval(p1, p2);
	return getIntervalName(iclass);
}

string HumTransposer::getIntervalName(int intervalClass) {
	string direction;
	if (intervalClass < 0) {
		direction = "-";
		intervalClass = -intervalClass;
	}

	int octave = intervalClass / m_base;
	int chroma = intervalClass - octave * m_base;

	int mindiff = chroma;
	int mini = 0;
	for (int i = 1; i < (int)m_diatonicMapping.size(); i++) {
		int diff = chroma - (m_diatonicMapping[i] - m_diatonicMapping[0]);
		if (abs(diff) < abs(mindiff)) {
			mindiff = diff;
			mini = i;
		}
		if (abs(mindiff) <= m_maxAccid) {
			break;
		}
	}

	int number = INVALID_INTERVAL_CLASS;
	int diminished = 0;
	int augmented = 0;
	string quality;

	switch (mini) {
		case 0: // unison
			number = 1;
			if (mindiff == 0) {
				quality = "P";
			}
			else if (mindiff < 0) {
				diminished = -mindiff;
			}
			else if (mindiff > 0) {
				augmented = mindiff;
			}
			break;
		case 1: // second
			number = 2;
			if (mindiff == 0) {
				quality = "M";
			}
			else if (mindiff == -1) {
				quality = "m";
			}
			else if (mindiff < 0) {
				diminished = -mindiff - 1;
			}
			else if (mindiff > 0) {
				augmented = mindiff;
			}
			break;
		case 2: // third
			number = 3;
			if (mindiff == 0) {
				quality = "M";
			}
			else if (mindiff == -1) {
				quality = "m";
			}
			else if (mindiff < 0) {
				diminished = -mindiff - 1;
			}
			else if (mindiff > 0) {
				augmented = mindiff;
			}
			break;
		case 3: // fourth
			number = 4;
			if (mindiff == 0) {
				quality = "P";
			}
			else if (mindiff < 0) {
				diminished = -mindiff;
			}
			else if (mindiff > 0) {
				augmented = mindiff;
			}
			break;
		case 4: // fifth
			number = 5;
			if (mindiff == 0) {
				quality = "P";
			}
			else if (mindiff < 0) {
				diminished = -mindiff;
			}
			else if (mindiff > 0) {
				augmented = mindiff;
			}
			break;
		case 5: // sixth
			number = 6;
			if (mindiff == 0) {
				quality = "M";
			}
			else if (mindiff == -1) {
				quality = "m";
			}
			else if (mindiff < 0) {
				diminished = -mindiff - 1;
			}
			else if (mindiff > 0) {
				augmented = mindiff;
			}
			break;
		case 6: // seventh
			number = 7;
			if (mindiff == 0) {
				quality = "M";
			}
			else if (mindiff == -1) {
				quality = "m";
			}
			else if (mindiff < 0) {
				diminished = -mindiff - 1;
			}
			else if (mindiff > 0) {
				augmented = mindiff;
			}
			break;
	}

	if (quality.empty()) {
		if (augmented) {
			for (int i = 0; i < augmented; i++) {
				quality += "A";
			}
		}
		else if (diminished) {
			for (int i = 0; i < diminished; i++) {
				quality += "d";
			}
		}
		else {
			quality = "?";
		}
	}

	number += octave * 7;

	string output = direction;
	output += quality;
	output += to_string(number);

	return output;
}



//////////////////////////////
//
// HumTransposer::intervalToCircleOfFifths -- Returns the circle-of-fiths count
//    that is represented by the given interval class or interval string.
//    Examples:  "P5"  => +1      "-P5" => -1
//               "P4"  => -1      "-P4" => +1
//               "M2"  => +2      "m7"  => -2
//               "M6"  => +3      "m3"  => -3
//               "M3"  => +4      "m6"  => -4
//               "M7"  => +5      "m2"  => -5
//               "A4"  => +6      "d5"  => -6
//               "A1"  => +7      "d1"  => -7
//
// If a key-signature plus the transposition interval in circle-of-fifths format
// is greater than +/-7, Then the -/+ 7 should be added to the key signature to
// avoid double sharp/flats in the key signature (and the transposition interval
// should be adjusted accordingly).
//

int HumTransposer::intervalToCircleOfFifths(const string &transstring) {
	int intervalClass = getInterval(transstring);
	return intervalToCircleOfFifths(intervalClass);
}

int HumTransposer::intervalToCircleOfFifths(int transval) {
	if (transval < 0) {
		transval = (m_base * 100 + transval) % m_base;
	}
	else if (transval == 0) {
		return 0;
	}
	else {
		transval %= m_base;
	}

	int p5 = perfectFifthClass();
	int p4 = perfectFourthClass();
	for (int i = 1; i < m_base; i++) {
		if ((p5 * i) % m_base == transval) {
			return i;
		}
		if ((p4 * i) % m_base == transval) {
			return -i;
		}
	}
	return INVALID_INTERVAL_CLASS;
}



//////////////////////////////
//
// HumTransposer::circleOfFifthsToIntervalClass -- Inputs a circle-of-fifths value and
//   returns the interval class as an integer in the current base.
//

int HumTransposer::circleOfFifthsToIntervalClass(int fifths) {
	if (fifths == 0) {
		return 0;
	}
	else if (fifths > 0) {
		return (perfectFifthClass() * fifths) % m_base;
	}
	else {
		return (perfectFourthClass() * (-fifths)) % m_base;
	}
}



//////////////////////////////
//
// HumTransposer::circleOfFifthsToIntervalName -- Convert a circle-of-fifths position
//    into an interval string.
//

string HumTransposer::circleOfFifthsToIntervalName(int fifths) {
	int intervalClass = circleOfFifthsToIntervalClass(fifths);
	return getIntervalName(intervalClass);
}



//////////////////////////////
//
// HumTransposer::circleOfFifthsTomajorTonic -- Return the tonic
//    of the major key that has the given key signature.  Return
//    value is in the 0th octave.
//

HumPitch HumTransposer::circleOfFifthsToMajorTonic(int fifths) {
	int intervalClass = circleOfFifthsToIntervalClass(fifths);
	return integerPitchToHumPitch((getCPitchClass() + intervalClass) % getBase());
}



//////////////////////////////
//
// HumTransposer::circleOfFifthsToMinorTonic -- Return the tonic
//    of the minor key that has the given key signature.  Return
//    value is in the 0th octave.
//

HumPitch HumTransposer::circleOfFifthsToMinorTonic(int fifths) {
	int intervalClass = circleOfFifthsToIntervalClass(fifths);
	return integerPitchToHumPitch((getAPitchClass() + intervalClass) % getBase());
}



//////////////////////////////
//
// HumTransposer::circleOfFifthsToDorianTonic -- Return the tonic
//    of the dorian key that has the given key signature.  Return
//    value is in the 0th octave.
//

HumPitch HumTransposer::circleOfFifthsToDorianTonic(int fifths) {
	int intervalClass = circleOfFifthsToIntervalClass(fifths);
	return integerPitchToHumPitch((getDPitchClass() + intervalClass) % getBase());
}



//////////////////////////////
//
// HumTransposer::circleOfFifthsToPhrygianTonic -- Return the tonic
//    of the phrygian key that has the given key signature.  Return
//    value is in the 0th octave.
//

HumPitch HumTransposer::circleOfFifthsToPhrygianTonic(int fifths) {
	int intervalClass = circleOfFifthsToIntervalClass(fifths);
	return integerPitchToHumPitch((getEPitchClass() + intervalClass) % getBase());
}



//////////////////////////////
//
// HumTransposer::circleOfFifthsToLydianTonic -- Return the tonic
//    of the lydian key that has the given key signature.  Return
//    value is in the 0th octave.
//

HumPitch HumTransposer::circleOfFifthsToLydianTonic(int fifths) {
	int intervalClass = circleOfFifthsToIntervalClass(fifths);
	return integerPitchToHumPitch((getFPitchClass() + intervalClass) % getBase());
}



//////////////////////////////
//
// HumTransposer::circleOfFifthsToMixolydianTonic -- Return the tonic
//    of the mixolydian key that has the given key signature.  Return
//    value is in the 0th octave.
//

HumPitch HumTransposer::circleOfFifthsToMixolydianTonic(int fifths) {
	int intervalClass = circleOfFifthsToIntervalClass(fifths);
	return integerPitchToHumPitch((getGPitchClass() + intervalClass) % getBase());
}



//////////////////////////////
//
// HumTransposer::circleOfFifthsToLocrianTonic -- Return the tonic
//    of the locrian key that has the given key signature.  Return
//    value is in the 0th octave.
//

HumPitch HumTransposer::circleOfFifthsToLocrianTonic(int fifths) {
	int intervalClass = circleOfFifthsToIntervalClass(fifths);
	return integerPitchToHumPitch((getBPitchClass() + intervalClass) % getBase());
}



//////////////////////////////
//
// HumTransposer::diatonicChromaticToIntervalClass -- Convert a diatonic/chromatic interval
//    into a base-n interval class integer.
//      +1D +1C = m2
//      +1D +2C = M2
//      +1D +3C = A2
//      +2D +4C = M3
//      +2D +3C = m3
//      +2D +2C = m3
//      +2D +1C = d3
//      +3D +5C = P4
//      +3D +6C = A4
//      +3D +4C = d4
//
//

string HumTransposer::diatonicChromaticToIntervalName(int diatonic, int chromatic) {
	if (diatonic == 0) {
		string output;
		if (chromatic == 0) {
			output += "P";
		}
		else if (chromatic > 0) {
			for (int i = 0; i < chromatic; i++) {
				output += "A";
			}
		}
		else {
			for (int i = 0; i < -chromatic; i++) {
				output += "d";
			}
		}
		output += "1";
		return output;
	}

	int octave = 0;
	string direction;
	if (diatonic < 0) {
		direction = "-";
		octave = -diatonic / 7;
		diatonic = (-diatonic - octave * 7);
		chromatic = -chromatic;
	}
	else {
		octave = diatonic / 7;
		diatonic = diatonic - octave * 7;
	}

	int augmented = 0;
	int diminished = 0;
	string quality;

	switch (abs(diatonic)) {
		case 0: // unsion
			if (chromatic == 0) {
				quality = "P";
			}
			else if (chromatic > 0) {
				augmented = chromatic;
			}
			else {
				diminished = chromatic;
			}
			break;
		case 1: // second
			if (chromatic == 2) {
				quality = "M";
			}
			else if (chromatic == 1) {
				quality = "m";
			}
			else if (chromatic > 2) {
				augmented = chromatic - 2;
			}
			else {
				diminished = chromatic - 1;
			}
			break;
		case 2: // third
			if (chromatic == 4) {
				quality = "M";
			}
			else if (chromatic == 3) {
				quality = "m";
			}
			else if (chromatic > 4) {
				augmented = chromatic - 4;
			}
			else {
				diminished = chromatic - 3;
			}
			break;
		case 3: // fourth
			if (chromatic == 5) {
				quality = "P";
			}
			else if (chromatic > 5) {
				augmented = chromatic - 5;
			}
			else {
				diminished = chromatic - 5;
			}
			break;
		case 4: // fifth
			if (chromatic == 7) {
				quality = "P";
			}
			else if (chromatic > 7) {
				augmented = chromatic - 7;
			}
			else {
				diminished = chromatic - 7;
			}
			break;
		case 5: // sixth
			if (chromatic == 9) {
				quality = "M";
			}
			else if (chromatic == 8) {
				quality = "m";
			}
			else if (chromatic > 9) {
				augmented = chromatic - 9;
			}
			else {
				diminished = chromatic - 8;
			}
			break;
		case 6: // seventh
			if (chromatic == 11) {
				quality = "M";
			}
			else if (chromatic == 10) {
				quality = "m";
			}
			else if (chromatic > 11) {
				augmented = chromatic - 11;
			}
			else {
				diminished = chromatic - 10;
			}
			break;
	}

	augmented = abs(augmented);
	diminished = abs(diminished);

	if (quality.empty()) {
		if (augmented) {
			for (int i = 0; i < augmented; i++) {
				quality += "A";
			}
		}
		else if (diminished) {
			for (int i = 0; i < diminished; i++) {
				quality += "d";
			}
		}
	}

	return direction + quality + to_string(octave * 7 + diatonic + 1);
}



//////////////////////////////
//
// HumTransposer::diatonicChromaticToIntervalClass --
//

int HumTransposer::diatonicChromaticToIntervalClass(int diatonic, int chromatic) {
	string intervalName = diatonicChromaticToIntervalName(diatonic, chromatic);
	return getInterval(intervalName);
}



//////////////////////////////
//
// HumTransposer::intervalToDiatonicChromatic --
//

void HumTransposer::intervalToDiatonicChromatic(int &diatonic, int &chromatic, int intervalClass) {
	string intervalName = getIntervalName(intervalClass);
	intervalToDiatonicChromatic(diatonic, chromatic, intervalName);
}

void HumTransposer::intervalToDiatonicChromatic(int &diatonic, int &chromatic, const string &intervalName) {
	int direction = 1;
	string quality;
	string number;
	int state = 0;

	for (int i = 0; i < (int)intervalName.size(); i++) {
		switch (state) {
			case 0: // direction or quality expected
				switch (intervalName[i]) {
					case '-': // interval is down
						direction = -1;
						state++;
						break;
					case '+': // interval is up
						direction = 1;
						state++;
						break;
					default: // interval is up by default
						direction = 1;
						state++;
						i--;
						break;
				}
				break;

			case 1: // quality expected
				if (isdigit(intervalName[i])) {
					state++;
					i--;
				}
				else {
					switch (intervalName[i]) {
						case 'M': // major
							quality = "M";
							break;
						case 'm': // minor
							quality = "m";
							break;
						case 'P': // perfect
						case 'p': quality = "P"; break;
						case 'D': // diminished
						case 'd': quality += "d"; break;
						case 'A': // augmented
						case 'a': quality += "A"; break;
					}
				}
				break;

			case 2: // digit expected
				if (isdigit(intervalName[i])) {
					number += intervalName[i];
				}
				break;
		}
	}

	if (quality.empty()) {
		cerr << "Interval requires a chromatic quality: " << intervalName << endl;
		chromatic = INVALID_INTERVAL_CLASS;
		diatonic = INVALID_INTERVAL_CLASS;
		return;
	}

	if (number.empty()) {
		cerr << "Interval requires a diatonic interval number: " << intervalName << endl;
		chromatic = INVALID_INTERVAL_CLASS;
		diatonic = INVALID_INTERVAL_CLASS;
		return;
	}

	int dnum = stoi(number);
	if (dnum == 0) {
		cerr << "Integer interval number cannot be zero: " << intervalName << endl;
		chromatic = INVALID_INTERVAL_CLASS;
		diatonic = INVALID_INTERVAL_CLASS;
		return;
	}
	dnum--;
	int octave = dnum / 7;
	dnum = dnum - octave * 7;

	diatonic = direction * (octave * 7 + dnum);
	chromatic = 0;

	switch (dnum) {
		case 0: // unison
			if (quality[0] == 'A') {
				chromatic = (int)quality.size();
			}
			else if (quality[0] == 'd') {
				chromatic = -(int)quality.size();
			}
			else if (quality == "P") {
				chromatic = 0;
			}
			else {
				cerr << "Error in Interval quality: " << intervalName << endl;
				chromatic = INVALID_INTERVAL_CLASS;
				diatonic = INVALID_INTERVAL_CLASS;
				return;
			}
			break;
		case 1: // second
			if (quality == "M") {
				chromatic = 2;
			}
			else if (quality == "m") {
				chromatic = 1;
			}
			else if (quality[0] == 'A') {
				chromatic = 2 + (int)quality.size();
			}
			else if (quality[0] == 'd') {
				chromatic = 1 - (int)quality.size();
			}
			else {
				cerr << "Error in Interval quality: " << intervalName << endl;
				chromatic = INVALID_INTERVAL_CLASS;
				diatonic = INVALID_INTERVAL_CLASS;
				return;
			}
			break;
		case 2: // third
			if (quality == "M") {
				chromatic = 4;
			}
			else if (quality == "m") {
				chromatic = 3;
			}
			else if (quality[0] == 'A') {
				chromatic = 4 + (int)quality.size();
			}
			else if (quality[0] == 'd') {
				chromatic = 3 - (int)quality.size();
			}
			else {
				cerr << "Error in Interval quality: " << intervalName << endl;
				chromatic = INVALID_INTERVAL_CLASS;
				diatonic = INVALID_INTERVAL_CLASS;
				return;
			}
			break;
		case 3: // fourth
			if (quality[0] == 'A') {
				chromatic = 5 + (int)quality.size();
			}
			else if (quality[0] == 'd') {
				chromatic = 5 - (int)quality.size();
			}
			else if (quality == "P") {
				chromatic = 5;
			}
			else {
				cerr << "Error in Interval quality: " << intervalName << endl;
				chromatic = INVALID_INTERVAL_CLASS;
				diatonic = INVALID_INTERVAL_CLASS;
				return;
			}
			break;
		case 4: // fifth
			if (quality[0] == 'A') {
				chromatic = 7 + (int)quality.size();
			}
			else if (quality[0] == 'd') {
				chromatic = 7 - (int)quality.size();
			}
			else if (quality == "P") {
				chromatic = 7;
			}
			else {
				cerr << "Error in Interval quality: " << intervalName << endl;
				chromatic = INVALID_INTERVAL_CLASS;
				diatonic = INVALID_INTERVAL_CLASS;
				return;
			}
			break;
		case 5: // sixth
			if (quality == "M") {
				chromatic = 9;
			}
			else if (quality == "m") {
				chromatic = 8;
			}
			else if (quality[0] == 'A') {
				chromatic = 9 + (int)quality.size();
			}
			else if (quality[0] == 'd') {
				chromatic = 8 - (int)quality.size();
			}
			else {
				cerr << "Error in Interval quality: " << intervalName << endl;
				chromatic = INVALID_INTERVAL_CLASS;
				diatonic = INVALID_INTERVAL_CLASS;
				return;
			}
			break;
		case 6: // seventh
			if (quality == "M") {
				chromatic = 11;
			}
			else if (quality == "m") {
				chromatic = 10;
			}
			else if (quality[0] == 'A') {
				chromatic = 11 + (int)quality.size();
			}
			else if (quality[0] == 'd') {
				chromatic = 10 - (int)quality.size();
			}
			else {
				cerr << "Error in Interval quality: " << intervalName << endl;
				chromatic = INVALID_INTERVAL_CLASS;
				diatonic = INVALID_INTERVAL_CLASS;
				return;
			}
			break;
	}
	chromatic *= direction;
}



//////////////////////////////
//
// HumTransposer::isValidIntervalName -- Returns true if the input string
//    is a valid chromatic interval string.  A valid interval name will match
//    this regular expression:
//          (-|\+?)([Pp]|M|m|[aA]+|[dD]+)([1-9][0-9]*)
//
//    Components of the regular expression:
//
//    1:  (-|\+?) == an optional direction for the interval.  When there
//                   is no sign, a + sign is implied.  A sign on a unison (P1)
//                   will be ignored.
//    2:  ([Pp]|M|m|[aA]+|[dD]+) == The chromatic quality of the following
//                   diatonic interval number.  Meanings of the letters:
//                      P or p = perfect
//                      M      = major
//                      m      = minor
//                      A or a = augmented
//                      d or D = diminished
//                   unisons (1), fourths, fifths and octaves (8) and octave multiples
//                   of these intervals can be prefixed by P but not by M or m.  Seconds,
//                   thirds, sixths, sevenths and octave transpositions of those intervals
//                   can be prefixed by M and m but not by P.  All intervals can be prefixed
//                   with A or d (or AA/dd for doubly augmented/diminished, etc.).  M and m
//                   are case sensitive, but P, A, and d are case insensitive.  This function
//                   does not check the correct pairing of M/m and P for the diatonic intervals
//                   (such as the invalid interval construct P3 for a perfect third).
//                   HumTransposer::getInterval(const string &intervalName) will do a
//                   more thorough check for invalid pairings.  This function is used mainly to
//                   determine whether an interval or a key tonic is being used in the --transpose
//                   option for verovio.
//     3: ([1-9][0-9]*) == a positive integer representing the diatonic interval.  1 = unison,
//                   2 = second, 3 = third, and so on.  Compound intervals are allowed, such as
//                   9 for a nineth (2nd plus a perfect octave), 15 for two perfect octaves.
//
//

bool HumTransposer::isValidIntervalName(const string &name) {
	string pattern = "(-|\\+?)([Pp]|M|m|[aA]+|[dD]+)([1-9][0-9]*)";
	if (regex_search(name, regex(pattern))) {
		return true;
	}
	else {
		return false;
	}
}



//////////////////////////////
//
// HumTransposer::isValidSemitones -- Returns true if the input string
//    is a valid semitone interval string.  A valid interval name will match
//    this regular expression:
//          ^(-|\+?)(\d+)$
//
//    Components of the regular expression:
//
//    1:  (-|\+?) == an optional direction for the interval.  When there
//                   is no sign, a + sign is implied.  A sign on 0
//                   will be ignored.
//    2:  (\d+)   == The number of semitones.  0 means transpose at the
//                   unison (i.e., transpose to same pitch as input).
//                   12 means an octave, 14 means a second plus an octave,
//                   24 means two octaves.
//

bool HumTransposer::isValidSemitones(const string &name) {
	string pattern = "^(-|\\+?)(\\d+)$";
	if (regex_search(name, regex(pattern))) {
		return true;
	}
	else {
		return false;
	}
}



//////////////////////////////
//
// HumTransposer::isValidKeyTonicName -- Returns true if the input string
//    is a valid key tonic which can be used to calculate a transposition
//    interval based on the current key.  A valid key tonic will match
//    this regular expression:
//          ([+]*|[-]*)([A-Ga-g])([Ss#]*|[Ffb]*)
//
//    Components of the regular expression:
//
//    1: ([+]*|[-]*) == An optional sign for the direction of the
//                      transposition.  If there is no sign, then
//                      the closest tonic pitch class to the tonic
//                      of the data will be selected.  When The
//                      sign is double/tripled/etc., additional
//                      octaves will be added to the transposition.
//    2: ([A-Ga-g]) ==  The diatonic letter of the tonic key.  The letter
//                      is case insensitive, so "g" and "G" have the
//                      same meaning.
//    3: ([Ss#]*|[Ffb]*) == An optional accidental alteration of the
//                      diatonic letter, such as eF, ef, eb, EF, Ef, or Eb,
//                      all meaning e-flat, and aS, as, a#, AS, As, or
//                      A# all meaning a-sharp.
//

bool HumTransposer::isValidKeyTonic(const string &name) {
	string pattern = "([+]*|[-]*)([A-Ga-g])([Ss#]*|[Ffb]*)";
	if (regex_search(name, regex(pattern))) {
		return true;
	}
	else {
		return false;
	}
}

/*



/////////////////////////////////////////////////////
//
// Test program for HumTransposer class:
//

int main(void) {
	HumPitch pitch(dpc_C, 0, 4); // middle C

	HumTransposer transpose;

	// transpose.setBase40() is the default system.
	transpose.setTransposition(transpose.perfectFifthClass());
	cout << "Starting pitch:\t\t\t\t" << pitch << endl;
	transpose.HumTransposer(pitch);
	cout << "HumTransposerd up a perfect fifth:\t\t" << pitch << endl;

	// testing use of a different base for transposition:
	transpose.setBase600(); // allows up to 42 sharps or flats
	// Note that transpose value is cleared when setAccid() or setBase*() is called.
	transpose.setTransposition(-transpose.perfectFifthClass());
	transpose.HumTransposer(pitch);
	cout << "HumTransposerd back down a perfect fifth:\t" << pitch << endl;

	// testing use of interval string
	transpose.setTransposition("-m3");
	transpose.HumTransposer(pitch);
	cout << "HumTransposerd down a minor third:\t\t" << pitch << endl;

	// testing validation system for under/overflows:
	cout << endl;
	pitch.setPitch(dpc_C, 2, 4); // C##4
	cout << "Initial pitch:\t\t" << pitch << endl;
	transpose.HumTransposer(pitch, "A4"); // now F###4
	bool valid = pitch.isValid(2);
	cout << "Up an aug. 4th:\t\t" << pitch;
	if (!valid) {
		cout << "\t(not valid in base-40 system)";
	}
	cout << endl;

	// calculate interval between two pitches:
	cout << endl;
	cout << "TESTING INTERVAL NAMES IN BASE-40:" << endl;
	transpose.setBase40();
	HumPitch p1(dpc_C, 0, 4);
	HumPitch p2(dpc_F, 2, 4);
	cout << "\tInterval between " << p1 << " and " << p2;
	cout << " is " << transpose.getIntervalName(p1, p2) << endl;
	HumPitch p3(dpc_G, -2, 3);
	cout << "\tInterval between " << p1 << " and " << p3;
	cout << " is " << transpose.getIntervalName(p1, p3) << endl;

	cout << "TESTING INTERVAL NAMES IN BASE-600:" << endl;
	transpose.setBase600();
	cout << "\tInterval between " << p1 << " and " << p2;
	cout << " is " << transpose.getIntervalName(p1, p2) << endl;
	cout << "\tInterval between " << p1 << " and " << p3;
	cout << " is " << transpose.getIntervalName(p1, p3) << endl;
	cout << endl;

	cout << "TESTING INTERVAL NAME TO CIRCLE-OF-FIFTHS:" << endl;
	cout << "\tM6 should be 3:  " << transpose.intervalTocircleOfFifths("M6") << endl;
	cout << "\tm6 should be -4: " << transpose.intervalTocircleOfFifths("m6") << endl;

	cout << "TESTING CIRCLE-OF-FIFTHS TO INTERVAL NAME:" << endl;
	cout << "\t3 should be M6:  " << transpose.circleOfFifthsToIntervalName(3) << endl;
	cout << "\t-4 should be m6: " << transpose.circleOfFifthsToIntervalName(-4) << endl;
	cout << endl;

	cout << "TESTING INTERVAL NAME TO DIATONIC/CHROMATIC:" << endl;
	cout << "\tD-1,C-2 should be -M2:  " << transpose.diatonicChromaticToIntervalName(-1, -2) << endl;
	cout << "\tD3,C6 should be A4:     " << transpose.diatonicChromaticToIntervalName(3, 6) << endl;

	int chromatic;
	int diatonic;

	cout << "TESTING DIATONIC/CHROMATIC TO INTERVAL NAME:" << endl;
	cout << "\t-M2 should be D-1,C-2:  ";
	transpose.intervalToDiatonicChromatic(diatonic, chromatic, "-M2");
	cout << "D" << diatonic << ",C" << chromatic << endl;
	cout << "\tA4 should be D3,C6:     ";
	transpose.intervalToDiatonicChromatic(diatonic, chromatic, "A4");
	cout << "D" << diatonic << ",C" << chromatic << endl;

	return 0;
}

*/

/* Example output from test program:

	Starting pitch:                        C4
	Transposed up a perfect fifth:         G4
	Transposed back down a perfect fifth:  C4
	Transposed down a minor third:         A3

	Initial pitch:   C##4
	Up an aug. 4th:  F###4 (not valid in base-40 system)

	TESTING INTERVAL NAMES IN BASE-40:
		Interval between C4 and F##4 is AA4
		Interval between C4 and Gbb3 is -AA4
	TESTING INTERVAL NAMES IN BASE-600:
		Interval between C4 and F##4 is AA4
		Interval between C4 and Gbb3 is -AA4

	TESTING INTERVAL NAME TO CIRCLE-OF-FIFTHS:
		M6 should be 3:  3
		m6 should be -4: -4
	TESTING CIRCLE-OF-FIFTHS TO INTERVAL NAME:
		3 should be M6:  M6
		-4 should be m6: m6

	TESTING INTERVAL NAME TO DIATONIC/CHROMATIC:
		D-1,C-2 should be -M2:  -M2
		D3,C6 should be A4:     A4
	TESTING DIATONIC/CHROMATIC TO INTERVAL NAME:
		-M2 should be D-1,C-2:  D-1,C-2
		A4 should be D3,C6:     D3,C6

 */

// END_MERGE

} // namespace hum
