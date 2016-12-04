//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Aug  8 12:24:49 PDT 2015
// Last Modified: Wed Aug 19 00:06:39 PDT 2015
// Filename:      Convert-pitch.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/Convert-pitch.cpp
// Syntax:        C++11
// vim:           ts=3 noexpandtab
//
// Description:   Conversions related to pitch.
//

#include <math.h>

#include <vector>

#include "Convert.h"

namespace hum {

// START_MERGE


//////////////////////////////
//
// Convert::kernToScientificPitch -- Convert a **kern pitch to
//   ScientificPitch notation, which is the diatonic letter name,
//   followed by a possible accidental, then an optional separator
//   string, and finally the octave number.  A string representing a
//   chord can be given to this function, and the output will return
//   a list of the pitches in the chord, separated by a space.
// default value: flat      = "b"
// default value: sharp     = "#"
// default value: separator = ""
//

string Convert::kernToScientificPitch(const string& kerndata,
		string flat, string sharp, string separator) {
	vector<string> subtokens = Convert::splitString(kerndata);
	string output;
	char   diatonic;
	int    accidental;
	int    octave;

	for (int i=0; i<(int)subtokens.size(); i++) {
		diatonic   = Convert::kernToDiatonicUC(subtokens[i]);
		accidental = Convert::kernToAccidentalCount(subtokens[i]);
		octave     = Convert::kernToOctaveNumber(subtokens[i]);
		if ((i > 0) && (i < (int)subtokens.size()-1)) {
			output += " ";
		}
		output += diatonic;
		for (int j=0; j<abs(accidental); j++) {
			output += (accidental < 0 ? flat : sharp);
		}
		output += separator;
		output += to_string(octave);
	}

	return output;
}



//////////////////////////////
//
// Convert::kernToDiatonicPC -- Convert a kern token into a diatonic
//    note pitch-class where 0="C", 1="D", ..., 6="B".  -1000 is returned
//    if the note is rest, and -2000 if there is no pitch information in the
//    input string. Only the first subtoken in the string is considered.
//

int Convert::kernToDiatonicPC(const string& kerndata) {
	for (int i=0; i<(int)kerndata.size(); i++) {
		if (kerndata[i] == ' ') {
			break;
		}
		if (kerndata[i] == 'r') {
			return -1000;
		}
		switch (kerndata[i]) {
			case 'A': case 'a': return 5;
			case 'B': case 'b': return 6;
			case 'C': case 'c': return 0;
			case 'D': case 'd': return 1;
			case 'E': case 'e': return 2;
			case 'F': case 'f': return 3;
			case 'G': case 'g': return 4;
		}
	}
	return -2000;
}



//////////////////////////////
//
// Convert::kernToDiatonicUC -- Convert a kern token into a diatonic
//    note pitch-class.  "R" is returned if the note is rest, and
//    "X" is returned if there is no pitch name in the string.
//    Only the first subtoken in the string is considered.
//

char Convert::kernToDiatonicUC(const string& kerndata) {
	for (int i=0; i<(int)kerndata.size(); i++) {
		if (kerndata[i] == ' ') {
			break;
		}
		if (kerndata[i] == 'r') {
			return 'R';
		}
		if (('A' <= kerndata[i]) && (kerndata[i] <= 'G')) {
			return kerndata[i];
		}
		if (('a' <= kerndata[i]) && (kerndata[i] <= 'g')) {
			return toupper(kerndata[i]);
		}
	}
	return 'X';
}



//////////////////////////////
//
// Convert::kernToDiatonicLC -- Similar to kernToDiatonicUC, but
//    the returned pitch name is lower case.
//

char Convert::kernToDiatonicLC(const string& kerndata) {
	return tolower(Convert::kernToDiatonicUC(kerndata));
}



//////////////////////////////
//
// Convert::kernToAccidentalCount -- Convert a kern token into a count
//    of accidentals in the first subtoken.  Sharps are assigned to the
//    value +1 and flats to -1.  So a double sharp is +2 and a double
//    flat is -2.  Only the first subtoken in the string is considered.
//    Cases such as "#-" should not exist, but in this case the return
//    value will be 0.
//

int Convert::kernToAccidentalCount(const string& kerndata) {
	int output = 0;
	for (int i=0; i<(int)kerndata.size(); i++) {
		if (kerndata[i] == ' ') {
			break;
		}
		if (kerndata[i] == '-') {
			output--;
		}
		if (kerndata[i] == '#') {
			output++;
		}
	}
	return output;
}



//////////////////////////////
//
// Convert::kernToOctaveNumber -- Convert a kern token into an octave number.
//    Middle C is the start of the 4th octave. -1000 is returned if there
//    is not pitch in the string.  Only the first subtoken in the string is
//    considered.
//

int Convert::kernToOctaveNumber(const string& kerndata) {
	int uc = 0;
	int lc = 0;
	if (kerndata == ".") {
		return -1000;
	}
	for (int i=0; i<(int)kerndata.size(); i++) {
		if (kerndata[i] == ' ') {
			break;
		}
		if (kerndata[i] == 'r') {
			return -1000;
		}
		uc += ('A' <= kerndata[i]) && (kerndata[i] <= 'G') ? 1 : 0;
		lc += ('a' <= kerndata[i]) && (kerndata[i] <= 'g') ? 1 : 0;
	}
	if ((uc > 0) && (lc > 0)) {
		// invalid pitch description
		return -1000;
	}
	if (uc > 0) {
		return 4 - uc;
	} else if (lc > 0) {
		return 3 + lc;
	} else {
		return -1000;
	}
}



//////////////////////////////
//
// Convert::kernToBase40PC -- Convert **kern pitch to a base-40 pitch class.
//    Will ignore subsequent pitches in a chord.
//

int Convert::kernToBase40PC(const string& kerndata) {
	int diatonic = Convert::kernToDiatonicPC(kerndata);
	if (diatonic < 0) {
		return diatonic;
	}
	int accid  = Convert::kernToAccidentalCount(kerndata);
	int output = -1000;
	switch (diatonic) {
		case 0: output =  0; break;
		case 1: output =  6; break;
		case 2: output = 12; break;
		case 3: output = 17; break;
		case 4: output = 23; break;
		case 5: output = 29; break;
		case 6: output = 35; break;
	}
	output += accid;
	return output + 2;     // +2 to make c-flat-flat bottom of octave.
}



//////////////////////////////
//
// Convert::kernToBase40 -- Convert **kern pitch to a base-40 integer.
//    Will ignore subsequent pitches in a chord.
//

int Convert::kernToBase40(const string& kerndata) {
	int pc = Convert::kernToBase40PC(kerndata);
	if (pc < 0) {
		return pc;
	}
	int octave   = Convert::kernToOctaveNumber(kerndata);
	return pc + 40 * octave;
}



//////////////////////////////
//
// Convert::kernToBase12PC -- Convert **kern pitch to a base-12 pitch-class.
//   C=0, C#/D-flat=1, D=2, etc.
//

int Convert::kernToBase12PC(const string& kerndata) {
	int diatonic = Convert::kernToDiatonicPC(kerndata);
	if (diatonic < 0) {
		return diatonic;
	}
	int accid    = Convert::kernToAccidentalCount(kerndata);
	int output = -1000;
	switch (diatonic) {
		case 0: output =  0; break;
		case 1: output =  2; break;
		case 2: output =  4; break;
		case 3: output =  5; break;
		case 4: output =  7; break;
		case 5: output =  9; break;
		case 6: output = 11; break;
	}
	output += accid;
	return output;
}



//////////////////////////////
//
// Convert::kernToBase12 -- Convert **kern pitch to a base-12 integer.
//     (middle C = 48).
//

int Convert::kernToBase12(const string& kerndata) {
	int pc = Convert::kernToBase12PC(kerndata);
	if (pc < 0) {
		return pc;
	}
	int octave = Convert::kernToOctaveNumber(kerndata);
	return pc + 12 * octave;
}



//////////////////////////////
//
// Convert::base40ToKern -- Convert Base-40 integer pitches into
//   **kern pitch representation.
//

string Convert::base40ToKern(int b40) {
   int octave     = b40 / 40;
   int accidental = Convert::base40ToAccidental(b40);
   int diatonic   = Convert::base40ToDiatonic(b40) % 7;
   char base = 'a';
   switch (diatonic) {
      case 0: base = 'c'; break;
      case 1: base = 'd'; break;
      case 2: base = 'e'; break;
      case 3: base = 'f'; break;
      case 4: base = 'g'; break;
      case 5: base = 'a'; break;
      case 6: base = 'b'; break;
   }
   if (octave < 4) {
      base = std::toupper(base);
   }
   int repeat = 0;
   if (octave > 4) {
      repeat = octave - 4;
   } else if (octave < 3) {
      repeat = 3 - octave;
   }
   if (repeat > 12) {
      cerr << "Error: unreasonable octave value: " << octave << endl;
      exit(1);
   }
	string output;
	output += base;
   for (int i=0; i<repeat; i++) {
		output += base;
   }
   if (accidental == 0) {
      return output;
   }
   if (accidental > 0) {
		for (int i=0; i<accidental; i++) {
			output += '#';
		}
   } else if (accidental < 0) {
		for (int i=0; i<-accidental; i++) {
			output += '-';
		}
   }

   return output;
}



//////////////////////////////
//
// Convert::base40ToDiatonic -- find the diatonic pitch of the
//   given base-40 pitch.  Output pitch classes: 0=C, 1=D, 2=E,
//   3=F, 4=G, 5=A, 6=B.  To this the diatonic octave is added.
//   To get only the diatonic pitch class, mod by 7: (% 7).
//   Base-40 pitches are not allowed, and the algorithm will have
//   to be adjusted to allow them.  Currently any negative base-40
//   value is presumed to be a rest and not processed.
//

int Convert::base40ToDiatonic(int b40) {
   int chroma = b40 % 40;
   int octaveoffset = (b40 / 40) * 7;
   if (b40 < 0) { 
      return -1;   // rest;
   }
   switch (chroma) {
		case 0: case 1: case 2: case 3: case 4:      // C-- to C##
			return 0 + octaveoffset;
		case 6: case 7: case 8: case 9: case 10:     // D-- to D##
			return 1 + octaveoffset;
		case 12: case 13: case 14: case 15: case 16: // E-- to E##
			return 2 + octaveoffset;
		case 17: case 18: case 19: case 20: case 21: // F-- to F##
			return 3 + octaveoffset;
		case 23: case 24: case 25: case 26: case 27: // G-- to G##
			return 4 + octaveoffset;
		case 29: case 30: case 31: case 32: case 33: // A-- to A##
			return 5 + octaveoffset;
		case 35: case 36: case 37: case 38: case 39: // B-- to B##
			return 6 + octaveoffset;
   }

	// found an empty slot, so return rest:
   return -1;
}



//////////////////////////////
//
// Convert::base40ToMidiNoteNumber --
//

int Convert::base40ToMidiNoteNumber(int b40) {
	// +1 since middle-C octave is 5 in MIDI:
   int octave     = b40 / 40 + 1;
   int accidental = Convert::base40ToAccidental(b40);
   int diatonicpc = Convert::base40ToDiatonic(b40) % 7;
	switch (diatonicpc) {
		case 0: return octave * 12 +  0 + accidental;
		case 1: return octave * 12 +  2 + accidental;
		case 2: return octave * 12 +  4 + accidental;
		case 3: return octave * 12 +  5 + accidental;
		case 4: return octave * 12 +  7 + accidental;
		case 5: return octave * 12 +  9 + accidental;
		case 6: return octave * 12 + 11 + accidental;
		default: return -1000; // can't deal with negative pitches
	}
}



//////////////////////////////
//
// Convert::base40ToAccidental -- +1 = 1 sharp, +2 = double sharp, 0 = natural
//	-1 = 1 flat, -2 = double flat
//

int Convert::base40ToAccidental(int b40) {
   if (b40 < 0) {
		// not considering low pitches.  If so then the mod operator
		// below whould need fixing.
      return 0;
   }

   switch (b40 % 40) {
      case 0:	return -2;      // C-double-flat
      case 1:	return -1;      // C-flat
      case 2:	return  0;      // C
      case 3:	return  1;      // C-sharp
      case 4:	return  2;      // C-double-sharp
      case 5:	return 1000;
      case 6:	return -2;
      case 7:	return -1;
      case 8:	return  0;      // D
      case 9:	return  1;
      case 10:	return  2;
      case 11:	return 1000;
      case 12:	return -2;
      case 13:	return -1;
      case 14:	return  0;      // E
      case 15:	return  1;
      case 16:	return  2;
      case 17:	return -2;
      case 18:	return -1;
      case 19:	return  0;      // F
      case 20:	return  1;
      case 21:	return  2;
      case 22:	return 1000;
      case 23:	return -2;
      case 24:	return -1;
      case 25:	return  0;      // G
      case 26:	return  1;
      case 27:	return  2;
      case 28:	return 1000;
      case 29:	return -2;
      case 30:	return -1;
      case 31:	return  0;      // A
      case 32:	return  1;
      case 33:	return  2;
      case 34:	return 1000;
      case 35:	return -2;
      case 36:	return -1;
      case 37:	return  0;      // B
      case 38:	return  1;
      case 39:	return  2;
   }

   return 0;
}



///////////////////////////////
//
// Convert::kernToMidiNoteNumber -- Convert **kern to MIDI note number
//    (middle C = 60).  Middle C is assigned to octave 5 rather than
//    octave 4 for the kernToBase12() function.
//

int Convert::kernToMidiNoteNumber(const string& kerndata) {
	int pc = Convert::kernToBase12PC(kerndata);
	if (pc < 0) {
		return pc;
	}
	int octave = Convert::kernToOctaveNumber(kerndata);
	return pc + 12 * (octave + 1);
}



//////////////////////////////
//
// Convert::kernToBase7 -- Convert **kern pitch to a base-7 integer.
//    This is a diatonic pitch class with C=0, D=1, ..., B=6.
//

int Convert::kernToBase7(const string& kerndata) {
	int diatonic = Convert::kernToDiatonicPC(kerndata);
	if (diatonic < 0) {
		return diatonic;
	}
	int octave = Convert::kernToOctaveNumber(kerndata);
	return diatonic + 7 * octave;;
}



//////////////////////////////
//
// Convert::pitchToWbh -- Convert a given diatonic pitch class and
//   accidental adjustment into an integer.  The diatonic pitch class
//   is C=0, D=1, E=2, F=3, G=4, A=5, B=6. "acc" is the accidental
//   count: -2=double flat, -1=double flat, 0 natural, +1=sharp, etc.
//   "octave" is the octave number, with middle-C being the start of
//   octave 4.  //   "maxacc" is the maximum accidental which defines
//    the base:
//    maxacc = 2 -> Base-40.
//    maxacc = n -> Base (n*2+1)*7 + 5.
//

int Convert::pitchToWbh(int dpc, int acc, int octave, int maxacc) {
	if (dpc > 6) {
		// allow for pitch-classes expressed as ASCII characters:
		dpc = std::tolower(dpc) - 'a' + 5;
		dpc = dpc % 7;
	}
	int output = -1000;
	switch (dpc) {
		case 0: output = maxacc;            break;
		case 1: output =  3  * maxacc + 2;  break;
		case 2: output =  5  * maxacc + 4;  break;
		case 3: output =  7  * maxacc + 5;  break;
		case 4: output =  9  * maxacc + 7;  break;
		case 5: output =  11 * maxacc + 9;  break;
		case 6: output =  13 * maxacc + 11; break;
	}
	if (output < 0) {
		return output;
	}
	return (output + acc) + (7 * (maxacc * 2 + 1) + 5) * octave;
}



//////////////////////////////
//
// Convert::wbhToPitch -- Convert an integer-based pitch into
//    a diatonic pitch class, accidental alteration and octave number
//   The output diatonic pitch classes are 0=C, 1=D, 2=E, 3=F, 4=G, 5=A, 6=B.
//   "acc" is the accidental count: -2=double flat, -1=double flat, 
//   0 natural, +1=sharp, etc.
//   "octave" is the octave number, with middle-C being the start of
//   octave 4.  
//   "maxacc" is the maximum accidental which defines
//    the base:
//    maxacc = 2 -> Base-40.
//    maxacc = n -> Base (n*2+1)*7 + 5.
//    This valus must match the the analogous value used in PitchToWbh().
//

void Convert::wbhToPitch(int& dpc, int& acc, int& octave, int maxacc,
		int wbh) {
	int cwidth = maxacc * 2 + 1;
	int base = 7 * cwidth + 5;
	octave = wbh / base;
	int pc = wbh % base;

	// test for C diatonic pitch:
	int pctest = cwidth;
	if (pc < pctest) {
		dpc = 0;
		acc = pc - pctest + maxacc + 1;
		return;
	}

	// test for D diatonic pitch
	pctest += 1 + cwidth;
	if (pc < pctest) {
		dpc = 1;
		acc = pc - pctest + maxacc + 1;
		return;
	}

	// test for E diatonic pitch
	pctest += 1 + cwidth;
	if (pc < pctest) {
		dpc = 2;
		acc = pc - pctest + maxacc + 1;
		return;
	}

	// test for F diatonic pitch
	pctest += cwidth;
	if (pc < pctest) {
		dpc = 3;
		acc = pc - pctest + maxacc + 1;
		return;
	}

	// test for G diatonic pitch
	pctest += 1 + cwidth;
	if (pc < pctest) {
		dpc = 4;
		acc = pc - pctest + maxacc + 1;
		return;
	}

	// test for A diatonic pitch
	pctest += 1 + cwidth;
	if (pc < pctest) {
		dpc = 5;
		acc = pc - pctest + maxacc + 1;
		return;
	}

	// test for B diatonic pitch
	pctest += 1 + cwidth;
	if (pc < pctest) {
		dpc = 6;
		acc = pc - pctest + maxacc + 1;
		return;
	}

	// if acc in any of the above tests is +3/-3, then there was an
	// accidental overflow (overflow of the accidental).

}


// END_MERGE

} // end namespace hum



