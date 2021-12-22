//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Fri May  4 20:29:09 PDT 2018
// Last Modified: Fri May  4 21:12:53 PDT 2018
// Filename:      Convert-mens.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/Convert-mens.cpp
// Syntax:        C++11; humlib
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Conversions related to strings.
//

#include <sstream>
#include <cctype>
#include <string>

#include "Convert.h"
#include "HumRegex.h"

using namespace std;

namespace hum {

// START_MERGE


//////////////////////////////
//
// Convert::isMensRest -- Returns true if the input string represents
//   a **mens rest.
//

bool Convert::isMensRest(const string& mensdata) {
	return mensdata.find('r') != std::string::npos;
}



//////////////////////////////
//
// Convert::isMensNote -- Returns true if the input string represents
//   a **mens note (i.e., token with a pitch, not a null token or a rest).
//

bool Convert::isMensNote(const string& mensdata) {
	char ch;
	for (int i=0; i < (int)mensdata.size(); i++) {
		ch = std::tolower(mensdata[i]);
		if ((ch >= 'a') && (ch <= 'g')) {
			return true;
		}
	}
	return false;
}



//////////////////////////////
//
// Convert::hasLigatureBegin -- Returns true if the input string
//   has a '<' or '[' character.
//

bool Convert::hasLigatureBegin(const string& mensdata) {
	return hasRectaLigatureBegin(mensdata) || hasObliquaLigatureBegin(mensdata);
}



//////////////////////////////
//
// Convert::hasRectaLigatureBegin --
//

bool Convert::hasRectaLigatureBegin(const string& mensdata) {
	return mensdata.find('[') != std::string::npos;
}



//////////////////////////////
//
// Convert::hasObliquaLigatureBegin --
//

bool Convert::hasObliquaLigatureBegin(const string& mensdata) {
	return mensdata.find('<') != std::string::npos;
}



//////////////////////////////
//
// Convert::hasLigatureEnd --
//

bool Convert::hasLigatureEnd(const string& mensdata) {
	return hasRectaLigatureEnd(mensdata) || hasObliquaLigatureEnd(mensdata);
}



//////////////////////////////
//
// Convert::hasRectaLigatureEnd -- Returns true if the input string
//   has a ']'.
//

bool Convert::hasRectaLigatureEnd(const string& mensdata) {
	return mensdata.find(']') != std::string::npos;
}



//////////////////////////////
//
// Convert::hasObliquaLigatureEnd -- Returns true if the input string
//   has a '>'.
//

bool Convert::hasObliquaLigatureEnd(const string& mensdata) {
	return mensdata.find('>') != std::string::npos;
}



//////////////////////////////
//
// Convert::getMensStemDir -- Returns the stem direction of **mens
//   notes.  These are the same as **kern pitches.
//      / = stem up, return +1
//      \ = stemp down, return -1
//      otherwise return 0 for no stem information
//

bool Convert::getMensStemDirection(const string& mensdata) {
	if (mensdata.find('/') != std::string::npos) {
		return +1;
	} else if (mensdata.find('\\') != std::string::npos) {
		return +1;
	} else {
		return 0;
	}
}



//////////////////////////////
//
// Convert::mensToDuration --
//   X = maxima (octuple whole note)
//   L = long  (quadruple whole note)
//   S = breve (double whole note)
//   s = semi-breve (whole note)
//   M = minim (half note)
//   m = semi-minim (quarter note)
//   U = fusa (eighth note)
//   u = semifusa (sixteenth note)
//
//   p = perfect (dotted)
//   i = imperfect (not-dotted)
//
// Still have to deal with coloration (triplets)
//
// Default value: scale = 4 (convert to quarter note units)
//                separator = " " (space between chord notes)
//

HumNum Convert::mensToDuration(const string& mensdata, HumNum scale,
		const string& separator) {
	HumNum output(0);
   bool perfect = false;
   // bool imperfect = true;
	for (int i=0; i<(int)mensdata.size(); i++) {
		if (mensdata[i] == 'p') {
			perfect = true;
			// imperfect = false;
		}
		if (mensdata[i] == 'i') {
			perfect = false;
			// imperfect = true;
		}

		// units are in whole notes, but scaling will probably
		// convert to quarter notes (default
		switch (mensdata[i]) {
			case 'X': output = 8; break;              // octuple whole note
			case 'L': output = 4; break;              // quadruple whole note
			case 'S': output = 2; break;              // double whole note
			case 's': output = 1; break;              // whole note
			case 'M': output.setValue(1, 2);  break;  // half note
			case 'm': output.setValue(1, 4);  break;  // quarter note
			case 'U': output.setValue(1, 8);  break;  // eighth note
			case 'u': output.setValue(1, 16); break;  // sixteenth note
		}

		if (mensdata.compare(i, separator.size(), separator) == 0) {
			// only get duration of first note in chord
			break;
		}
	}

	if (perfect) {
		output *= 3;
		output /= 2;
	}
	output *= scale;
	return output;
}



//////////////////////////////
//
// Convert::mensToDurationNoDots -- The imperfect duration of the **mens rhythm.
//

HumNum Convert::mensToDurationNoDots(const string& mensdata, HumNum scale,
		const string& separator) {
	HumNum output(0);

	for (int i=0; i<(int)mensdata.size(); i++) {
		switch (mensdata[i]) {
			case 'X': output = 8; break;              // octuple whole note
			case 'L': output = 4; break;              // quadruple whole note
			case 'S': output = 2; break;              // double whole note
			case 's': output = 1; break;              // whole note
			case 'M': output.setValue(1, 2);  break;  // half note
			case 'm': output.setValue(1, 4);  break;  // quarter note
			case 'U': output.setValue(1, 8);  break;  // eighth note
			case 'u': output.setValue(1, 16); break;  // sixteenth note
		}
		if (mensdata.compare(i, separator.size(), separator) == 0) {
			// only get duration of first note in chord
			break;
		}
	}
	output *= scale;

	return output;
}



//////////////////////////////
//
// Convert::mensToRecip --
//

string Convert::mensToRecip(const string& mensdata, HumNum scale, const string& separator) {
	HumNum duration = Convert::mensToDuration(mensdata, scale, separator);
	return Convert::durationToRecip(duration);
}



//////////////////////////////
//
// Convert::mensToRecip -- Convert from **mens rhythmic levels according to alteration/perfections/imperfection and
//      divisions of rhythmic levels based on prevailing mensuration.
//

string Convert::mensToRecip(char rhythm, bool altera, bool perfecta, bool imperfecta,
		int maximodus, int modus, int tempus, int prolatio) {

	double minim_units;


	double maxima_def = 16;
	double longa_def = 8;
	double brevis_def = 4;
	double semibrevis_def = 2;
	double minima_def = 1;
	double semiminima_def = 0.5;
	double fusa_def = 0.25;
	double semifusa_def = 0.125;

	prolatio  = prolatio  == 2 ? 2 : 3;
	tempus    = tempus    == 2 ? 2 : 3;
	modus     = modus     == 2 ? 2 : 3;
	maximodus = maximodus == 2 ? 2 : 3;

	semibrevis_def = prolatio;
	brevis_def     = semibrevis_def * tempus;
	longa_def      = brevis_def     * modus;
	maxima_def     = longa_def      * maximodus;

	if (rhythm == 'X') {
		if (perfecta) { minim_units = longa_def * 3; }
		else if (imperfecta) { minim_units = longa_def * 2; }
		else { minim_units = maxima_def; }
	}
	else if (rhythm == 'L') {
		if (perfecta) { minim_units = brevis_def * 3; }
		else if (imperfecta) { minim_units = brevis_def * 2; }
		else if (altera) { minim_units = longa_def * 2; }
		else { minim_units = longa_def; }
	}
	else if (rhythm == 'S') {
		if (perfecta) { minim_units = semibrevis_def * 3; }
		else if (imperfecta) { minim_units = semibrevis_def * 2; }
		else if (altera) { minim_units = brevis_def * 2; }
		else { minim_units = brevis_def; }
	}
	else if (rhythm == 's') {
		if (perfecta) { minim_units = minima_def * 3; }
		else if (imperfecta) { minim_units = minima_def * 2; }
		else if (altera) { minim_units = semibrevis_def * 2; }
		else { minim_units = semibrevis_def; }
	}
	else if (rhythm == 'M') {
		if (perfecta) { minim_units = minima_def * 3 / 2; }
		else if (altera) { minim_units = minima_def * 2; }
		else { minim_units = minima_def; }
	}
	else if (rhythm == 'm') {
		if (perfecta) { minim_units = semiminima_def * 3 / 2; }
		else { minim_units = semiminima_def; }
	}
	else if (rhythm == 'U') {
		if (perfecta) { minim_units = fusa_def * 3 / 2; }
		else { minim_units = fusa_def; }
	}
	else if (rhythm == 'u') {
		if (perfecta) { minim_units = semifusa_def * 3 / 2; }
		else { minim_units = semifusa_def; }
	}
	else { cerr << "UNKNOWN RHYTHM: " << rhythm << endl; return ""; }

	switch ((int)(minim_units * 10000)) {
		case 1250:     return "16";   break;   // sixteenth note
		case 1875:     return "16.";  break;   // dotted sixteenth note
		case 2500:     return "8";    break;   // eighth note
		case 3750:     return "8.";   break;   // dotted eighth note
		case 5000:     return "4";    break;   // quarter note
		case 7500:     return "4.";   break;   // dotted quarter note
		case 10000:    return "2";    break;   // half note
		case 15000:    return "2.";   break;   // dotted half note
		case 20000:    return "1";    break;   // whole note
		case 30000:    return "1.";   break;   // dotted whole note
		case 40000:    return "0";    break;   // breve note
		case 60000:    return "0.";   break;   // dotted breve note
		case 90000:    return "2%9";  break;   // or ["0.", "1."];
		case 80000:    return "00";   break;   // long note
		case 120000:   return "00.";  break;   // dotted long note
		case 180000:   return "1%9";  break;   // or ["00.", "0."];
		case 270000:   return "2%27"; break;   // or ["0.", "1.", "0.", "1.", "0.", "1."];
		case 160000:   return "000";  break;   // maxima note
		case 240000:   return "000."; break;   // dotted maxima note
		case 360000:   return "1%18"; break;   // or ["000.", "00."];
		case 540000:   return "1%27"; break;   // or ["00.", "0.", "00.", "0.", "00.", "0."];
		case 810000:   return "2%81"; break;   // or ["00.", "0.", "00.", "0.", "00.", "0.", "0.", "1.", "0.", "1.", "0.", "1."];
		default:
			cerr << "Error: unknown minim_units: " << minim_units << endl;
	}

	return "";
}




//////////////////////////////
//
// Convert::mensToDuration -- Convert from **mens rhythmic levels according to alteration/perfections/imperfection and
//      divisions of rhythmic levels based on prevailing mensuration.
//

HumNum Convert::mensToDuration(HTp menstok) {
	int rlev = menstok->getValueInt("auto", "mensuration", "levels");
	if (rlev < 2222) {
		cerr << "Warning: cannot find mensuration levels for token " << menstok << endl;
		rlev = 2222;
	}
	return Convert::mensToDuration(*menstok, rlev);
}

HumNum Convert::mensToDuration(HTp menstok, const std::string& mettok) {
	int rlev = Convert::metToMensurationLevels(mettok);
	return Convert::mensToDuration(*menstok, rlev);
}


HumNum Convert::mensToDuration(const string& menstok, int rlev) {
	if (rlev < 2222) {
		rlev = 2222;
	}
	int maximodus = (rlev / 1000) % 10;
	int modus     = (rlev / 100)  % 10;
	int tempus    = (rlev / 10)   % 10;
	int prolation =  rlev         % 10;
	bool altera = false;
	bool perfecta = false;
	bool imperfecta = false;

	if (menstok.find("+") != string::npos) {
		altera = true;
	}
	if (menstok.find("p") != string::npos) {
		perfecta = true;
	}
	if (menstok.find("i") != string::npos) {
		imperfecta = true;
	}
	HumRegex hre;
	if (!hre.search(menstok, "([XLSsMmUu])")) {
		// invalid note/rest rhythm
		return 0;
	}
	string rhythm = hre.getMatch(1);
	char rchar = rhythm[0];

	// check for redundant perfection/imperfection?
	return Convert::mensToDuration(rchar, altera, perfecta, imperfecta, maximodus, modus, tempus, prolation);

}

HumNum Convert::mensToDuration(char rhythm, bool altera, bool perfecta, bool imperfecta,
		int maximodus, int modus, int tempus, int prolatio) {

	HumNum minim_units;

	HumNum maxima_def(16,1);
	HumNum longa_def(8,1);
	HumNum brevis_def(4,1);
	HumNum semibrevis_def(2,1);
	HumNum minima_def(1,1);
	HumNum semiminima_def(1,2);
	HumNum fusa_def(1, 4);
	HumNum semifusa_def(1, 8);

	prolatio  = prolatio  == 2 ? 2 : 3;
	tempus    = tempus    == 2 ? 2 : 3;
	modus     = modus     == 2 ? 2 : 3;
	maximodus = maximodus == 2 ? 2 : 3;

	semibrevis_def = prolatio;
	brevis_def     = semibrevis_def * tempus;
	longa_def      = brevis_def     * modus;
	maxima_def     = longa_def      * maximodus;

	if (rhythm == 'X') {
		if      (perfecta)   { minim_units = longa_def * 3; }
		else if (imperfecta) { minim_units = longa_def * 2; }
		else                 { minim_units = maxima_def; }
	}
	else if (rhythm == 'L') {
		if      (perfecta)   { minim_units = brevis_def * 3; }
		else if (imperfecta) { minim_units = brevis_def * 2; }
		else if (altera)     { minim_units = longa_def  * 2; }
		else                 { minim_units = longa_def; }
	}
	else if (rhythm == 'S') {
		if      (perfecta)   { minim_units = semibrevis_def * 3; }
		else if (imperfecta) { minim_units = semibrevis_def * 2; }
		else if (altera)     { minim_units = brevis_def     * 2; }
		else                 { minim_units = brevis_def; }
	}
	else if (rhythm == 's') {
		if      (perfecta)   { minim_units = minima_def     * 3; }
		else if (imperfecta) { minim_units = minima_def     * 2; }
		else if (altera)     { minim_units = semibrevis_def * 2; }
		else                 { minim_units = semibrevis_def; }
	}
	else if (rhythm == 'M') {
		if      (perfecta) { minim_units = minima_def * 3 / 2; }
		else if (altera)   { minim_units = minima_def * 2; }
		else               { minim_units = minima_def; }
	}
	else if (rhythm == 'm') {
		if (perfecta) { minim_units = semiminima_def * 3 / 2; }
		else          { minim_units = semiminima_def; }
	}
	else if (rhythm == 'U') {
		if (perfecta) { minim_units = fusa_def * 3 / 2; }
		else          { minim_units = fusa_def; }
	}
	else if (rhythm == 'u') {
		if (perfecta) { minim_units = semifusa_def * 3 / 2; }
		else          { minim_units = semifusa_def; }
	}
	else { cerr << "UNKNOWN RHYTHM: " << rhythm << endl; return 0; }

	HumNum quarter_units = minim_units * 2;
	return quarter_units;
}



//////////////////////////////
//
// HumdrumInput::metToMensurationLevels --
//		Return value has four digits that are either 2 or 3 (almost
//    always, but currently always as output from this function):
//    	WXYZ
//    W = number of longs in a maxima (maximodus)
//    X = number of breves in a long (modus)
//    Y = number of semibreves in a breve (tempus)
//    Z = number of minims in a semibreve (prolation)
//
// See page 17 of: https://digital.library.unt.edu/ark:/67531/metadc86/m1/40
// For levels for basic mensuration signs:
//
//    sign  maximodus modus tempus prolation
//    O3    3         3     3      2          (breve = tactus)
//    C3    2         2     3      2          (breve = tactus)
//    O2    2         3     2      2          (breve = tactus)
//    C2    2         2     2      2          (breve = tactus)
//    O     2         2     3      2          (semibreve = tactus)
//    C     2         2     2      2          (semibreve = tactus)
//    O.    2         2     3      3          (semibreve = tactus)
//    C.    2         2     2      3          (semibreve = tactus)
//

int Convert::metToMensurationLevels(const string& metsig) {
	// Default divisions are 2 (matching modern system):
	int maximodus = 2;
	int modus     = 2;
	int tempus    = 2;
	int prolation = 2;

	// Explicit values for rhythmic levels:
	int emaximodus = 0;
	int emodus     = 0;
	int etempus    = 0;
	int eprolation = 0;

	HumRegex hre;

	// Check for explicit rhythmic level divisions:
	if (hre.search(metsig, "^\\*?met\\(.*?\\)_(\\d)(\\d)(\\d)(\\d)")) {
		emaximodus = hre.getMatchInt(1);
		emodus     = hre.getMatchInt(2);
		etempus    = hre.getMatchInt(3);
		eprolation = hre.getMatchInt(4);
		// Limit to two or three divisions per level:
		emaximodus = emaximodus == 3 ? 3 : 2;
		emodus     = emodus     == 3 ? 3 : 2;
		etempus    = etempus    == 3 ? 3 : 2;
		eprolation = eprolation == 3 ? 3 : 2;
		return emaximodus * 1000 + emodus * 100 + etempus * 10 + eprolation;
	}

	// Check for incomplete rhythmic levels:
	if (hre.search(metsig, "^\\*?met\\(.*?\\)_(\\d)(\\d)(\\d)")) {
		emaximodus = hre.getMatchInt(1);
		emodus     = hre.getMatchInt(2);
		etempus    = hre.getMatchInt(3);
	} else if (hre.search(metsig, "^\\*?met\\(.*?\\)_(\\d)(\\d)")) {
		emaximodus = hre.getMatchInt(1);
		emodus     = hre.getMatchInt(2);
	} else if (hre.search(metsig, "^\\*?met\\(.*?\\)_(\\d)")) {
		emaximodus = hre.getMatchInt(1);
	}

	// Get implicit rhythmic levels based on mensuration sign:
	if (!hre.search(metsig, "^\\*?met\\((.+?)\\)")) {
		// Null mensuration sign or strange problem.
		maximodus = emaximodus == 3 ? 3 : 2;
		modus     = emodus     == 3 ? 3 : 2;
		tempus    = etempus    == 3 ? 3 : 2;
		prolation = eprolation == 3 ? 3 : 2;
		return maximodus * 1000 + modus * 100 + tempus * 10 + prolation;
	}
	string mensur = hre.getMatch(1);

	if (mensur == "C") {
		maximodus = 2; modus = 2; tempus = 2; prolation = 2;
	} else if (mensur == "C|") {
		maximodus = 2; modus = 2; tempus = 2; prolation = 2;
	} else if (mensur == "C.") {
		maximodus = 2; modus = 2; tempus = 2; prolation = 3;
	} else if (mensur == "C.|") {
		maximodus = 2; modus = 2; tempus = 2; prolation = 3;
	} else if (mensur == "C2") {
		maximodus = 2; modus = 2; tempus = 2; prolation = 2;
	} else if (mensur == "C3") {
		maximodus = 2; modus = 2; tempus = 3; prolation = 2;
	} else if (mensur == "O") {
		maximodus = 2; modus = 2; tempus = 3; prolation = 2;
	} else if (mensur == "O|") {
		maximodus = 2; modus = 2; tempus = 3; prolation = 2;
	} else if (mensur == "O.") {
		maximodus = 2; modus = 2; tempus = 3; prolation = 3;
	} else if (mensur == "O.|") {
		maximodus = 2; modus = 2; tempus = 3; prolation = 3;
	} else if (mensur == "O2") {
		maximodus = 2; modus = 3; tempus = 2; prolation = 2;
	} else if (mensur == "O3") {
		maximodus = 3; modus = 3; tempus = 3; prolation = 2;
	} else if (mensur == "O|3") {
		maximodus = 3; modus = 3; tempus = 3; prolation = 2;
	} else if (mensur == "C|3/2") {
		maximodus = 2; modus = 2; tempus = 2; prolation = 2;
		// rscale: 3/2
	} else {
		// Others to implement include:
		// *met(c) == this is a modern metric signature (common time)
		// *met(c|) == this is a modern metric signature (cut time)
		// *met(Cr)
      // *met(3) == this is problematic because it will depend on the
		// previous mensuration sign, such as *met(C|), which means that
		// *met(3) is *met(C|3/2) which is 50% faster than *met(C|), having
		// three semibreves in the time of two original ones.  Such
		// "sequialtera" mensurations are not dealt with yet.
		cerr << "Warning: do not understand mensuration " << mensur << endl;
		return maximodus * 1000 + modus * 100 + tempus * 10 + prolation;
	}

	maximodus = emaximodus != 0 ? emaximodus : maximodus;
	modus     = emodus     != 0 ? emodus     : modus;
	tempus    = etempus    != 0 ? etempus    : tempus;
	prolation = eprolation != 0 ? eprolation : prolation;

	maximodus = maximodus == 3 ? 3 : 2;
	modus     = modus     == 3 ? 3 : 2;
	tempus    = tempus    == 3 ? 3 : 2;
	prolation = prolation == 3 ? 3 : 2;

	return maximodus * 1000 + modus * 100 + tempus * 10 + prolation;
}



// END_MERGE

} // end namespace hum



