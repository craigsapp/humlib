//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Tue Sep 24 10:40:19 PDT 2019
// Last Modified: Tue Sep 24 10:40:22 PDT 2019
// Filename:      Convert-musedata.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/Convert-musedata.cpp
// Syntax:        C++11; humlib
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Convert between musedata and various Humdrum representations.
//

#include "Convert.h"

using namespace std;

namespace hum {

// START_MERGE



//////////////////////////////
//
// Convert::museToBase40 -- Convert a MuseData pitch into base-40 representation.
//

int Convert::museToBase40(const string& pitchString) {
   string temp = pitchString;
   int octave;
   int i = (int)temp.size() - 1;
   while (i >= 0 && !isdigit(temp[i])) {
      i--;
   }

   if (i <= 0) {
      cerr << "Error: could not find octave in string: " << pitchString << endl;
		cerr << "Assigning to octave 4" << endl;
      octave = 4;
   } else {
   	octave = temp[i] - '0';
	}
   temp.resize(i);

	for (int i=0; i<(int)temp.size(); i++) {
		if (temp[i] == 'f') {
			temp[i] = '-';
		}
	}
	int kb40 = Convert::kernToBase40(temp);
	if (kb40 < 0) {
		return kb40;
	}
   return kb40 % 40 + 40 * octave;
}



//////////////////////////////
//
// Convert::musePitchToKernPitch -- 
//

string Convert::musePitchToKernPitch(const string& museInput) {
   return base40ToKern(museToBase40(museInput));
}



//////////////////////////////
//
// Convert::museClefToKernClef --
//

string Convert::museClefToKernClef(const string& mclef) {
	if (mclef == "4") {         // treble clef
		return "*clefG2";
	} else if (mclef == "22") { // bass clef
		return "*clefF4";
	} else if (mclef == "13") { // alto clef
		return "*clefC3";
	} else if (mclef == "12") { // tenor clef
		return "*clefC4";
	} else if (mclef == "15") { // soprano clef
		return "*clefC1";
	} else if (mclef == "14") { // mezzo-soprano clef
		return "*clefC2";
	} else if (mclef == "14") { // baritone clef
		return "*clefC5";
	} else if (mclef == "5") { // French violin clef
		return "*clefG1";
	} else if (mclef == "3") {
		return "*clefG3";
	} else if (mclef == "2") {
		return "*clefG4";
	} else if (mclef == "1") {
		return "*clefG5";
	} else if (mclef == "25") {
		return "*clefF1";
	} else if (mclef == "24") {
		return "*clefF2";
	} else if (mclef == "23") {
		return "*clefF3";
	} else if (mclef == "21") {
		return "*clefF5";
	} else if (mclef == "35") {
		return "*clefGv1";
	} else if (mclef == "34") {  // vocal tenor clef
		return "*clefGv2";
	} else if (mclef == "33") {
		return "*clefGv3";
	} else if (mclef == "32") {
		return "*clefGv3";
	} else if (mclef == "31") {
		return "*clefGv5";
	}
	// percussion clef?
	// return unknown clef:
	return "*";
}



//////////////////////////////
//
// Convert::museKeySigToKernKeySig --
//

string Convert::museKeySigToKernKeySig(const string& mkeysig) {
	if (mkeysig == "0") {
		return "*k[]";
	} else if (mkeysig == "1") {
		return "*k[f#]";
	} else if (mkeysig == "-1") {
		return "*k[b-]";
	} else if (mkeysig == "2") {
		return "*k[f#c#]";
	} else if (mkeysig == "-2") {
		return "*k[b-e-]";
	} else if (mkeysig == "3") {
		return "*k[f#c#g#]";
	} else if (mkeysig == "-3") {
		return "*k[b-e-a-]";
	} else if (mkeysig == "4") {
		return "*k[f#c#g#d#]";
	} else if (mkeysig == "-4") {
		return "*k[b-e-a-d-]";
	} else if (mkeysig == "5") {
		return "*k[f#c#g#d#a#]";
	} else if (mkeysig == "-5") {
		return "*k[b-e-a-d-g-]";
	} else if (mkeysig == "6") {
		return "*k[f#c#g#d#a#e#]";
	} else if (mkeysig == "-6") {
		return "*k[b-e-a-d-g-c-]";
	} else if (mkeysig == "7") {
		return "*k[f#c#g#d#a#e#b#]";
	} else if (mkeysig == "-7") {
		return "*k[b-e-a-d-g-c-f-]";
	}
	return "*";
}



//////////////////////////////
//
// Convert::museTimeSigToKernTimeSig --
//

string Convert::museTimeSigToKernTimeSig(const string& mtimesig) {
	if (mtimesig == "11/0") {
		return "*M3/1";    // "*met(O)
	} else if (mtimesig == "1/1") {
		return "*M4/4";    // "*met(c)
	} else if (mtimesig == "0/0") {
		return "*M2/2";    // "*met(c)
	} else if (mtimesig == "12/0") {
		return "";    // "*met(O:)
	} else if (mtimesig == "21/0") {
		return "";    // "*met(O.)
	} else if (mtimesig == "22/0") {
		return "";    // "*met(O;)
	} else if (mtimesig == "31/0") {
		return "*M2/1";    // "*met(C)
	} else if (mtimesig == "41/0") {
		return "";    // "*met(C.)
	} else if (mtimesig == "42/0") {
		return "";    // "*met(C.3/2)
	} else if (mtimesig == "43/0") {
		return "";    // "*met(C.3/8)
	} else if (mtimesig == "51/0") {
		return "";    // "*met(Cr)
	} else if (mtimesig == "52/0") {
		return "";    // "*met(Cr|)
	} else if (mtimesig == "61/0") {
		return "*M2/1"; // "*met(C|)
	} else if (mtimesig == "62/0") {
		return "";     // "*met(C|/2)
	} else if (mtimesig == "63/0") {
		return "";     // "*met(C|.)
	} else if (mtimesig == "71/0") {
		return "";     // "*met(C2)
	} else if (mtimesig == "72/0") {
		return "";     // "*met(C2/3)
	} else if (mtimesig == "81/0") {
		return "";     // "*met(O2)
	} else if (mtimesig == "82/0") {
		return "";     // "*met(O3/2)
	} else if (mtimesig == "91/0") {
		return "*M3/1"; // "*met(O|)
	} else if (mtimesig == "92/0") {
		return ""; // "*met(O|3)
	} else if (mtimesig == "93/0") {
		return ""; // "*met(O|3/2)
	} else if (mtimesig == "101/0") {
		return ""; // "*met(C|3)
	} else if (mtimesig == "102/0") {
		return ""; // "*met(3)
	} else if (mtimesig == "103/0") {
		return ""; // "*met(3/2)
	} else if (mtimesig == "104/0") {
		return ""; // "*met(C|/3)
	} else if (mtimesig == "105/0") {
		return ""; // "*met(C3)
	} else if (mtimesig == "106/0") {
		return ""; // "*met(O/3)
	} else if (mtimesig == "111/0") {
		return ""; // "*met(C|2)
	} else if (mtimesig == "112/0") {
		return ""; // "*met(2)
	} else if (mtimesig == "121/0") {
		return ""; // "*met(Oo)
	}
	string output = "*M" + mtimesig;
	return output;
}



//////////////////////////////
//
// Convert::museMeterSigToKernMeterSig --
//

string Convert::museMeterSigToKernMeterSig(const string& mtimesig) {
	if (mtimesig == "11/0") {
		return "*met(O)";
	} else if (mtimesig == "1/1") {
		return "*met(c)";
	} else if (mtimesig == "0/0") {
		return "*met(c)";
	} else if (mtimesig == "12/0") {
		return "*met(O:)";
	} else if (mtimesig == "21/0") {
		return "*met(O.)";
	} else if (mtimesig == "22/0") {
		return "*met(O;)";
	} else if (mtimesig == "31/0") {
		return "*met(C)";
	} else if (mtimesig == "41/0") {
		return "*met(C.)";
	} else if (mtimesig == "42/0") {
		return "*met(C.3/2)";
	} else if (mtimesig == "43/0") {
		return "*met(C.3/8)";
	} else if (mtimesig == "51/0") {
		return "*met(Cr)";
	} else if (mtimesig == "52/0") {
		return "*met(Cr|)";
	} else if (mtimesig == "61/0") {
		return "*met(C|)";
	} else if (mtimesig == "62/0") {
		return "*met(C|/2)";
	} else if (mtimesig == "63/0") {
		return "*met(C|.)";
	} else if (mtimesig == "71/0") {
		return "*met(C2)";
	} else if (mtimesig == "72/0") {
		return "*met(C2/3)";
	} else if (mtimesig == "81/0") {
		return "*met(O2)";
	} else if (mtimesig == "82/0") {
		return "*met(O3/2)";
	} else if (mtimesig == "91/0") {
		return "*met(O|)";
	} else if (mtimesig == "92/0") {
		return "*met(O|3)";
	} else if (mtimesig == "93/0") {
		return "*met(O|3/2)";
	} else if (mtimesig == "101/0") {
		return "*met(C|3)";
	} else if (mtimesig == "102/0") {
		return "*met(3)";
	} else if (mtimesig == "103/0") {
		return "*met(3/2)";
	} else if (mtimesig == "104/0") {
		return "*met(C|/3)";
	} else if (mtimesig == "105/0") {
		return "*met(C3)";
	} else if (mtimesig == "106/0") {
		return "*met(O/3)";
	} else if (mtimesig == "111/0") {
		return "*met(C|2)";
	} else if (mtimesig == "112/0") {
		return "*met(2)";
	} else if (mtimesig == "121/0") {
		return "*met(Oo)";
	}
	return "";
}



//////////////////////////////
//
// Convert::museFiguredBassToKernFiguredBass --
//

string Convert::museFiguredBassToKernFiguredBass(const string& mfb) {
	string output;
	for (int i=0; i<(int)mfb.size(); i++) {
		if (mfb[i] == 'b') { // blank spot in figure stack
			output += 'X';
		} else if (mfb[i] == 'f') { // flat
			output += '-';
		} else if ((mfb[i] == '&') && (i < (int)mfb.size()-1) && (mfb[i+1] == '0')) {
			output += ":";
			i++;
		} else if ((mfb[i] == '/')) {  // assuming slash means flat
			output += "-/";
		} else if ((mfb[i] == '\\')) { // assuming slash means sharp
			output += "#/";
		} else if ((mfb[i] == '+')) {  // assuming slash means sharp
			output += "#|";
		} else if (isdigit(mfb[i]) && (i < (int)mfb.size() - 1) && (mfb[i+1] == '#')) {
			output += mfb[i];
			output += mfb[i+1];
			output += 'r';
			i++;
		} else if (isdigit(mfb[i]) && (i < (int)mfb.size() - 1) && (mfb[i+1] == 'f')) {
			output += mfb[i];
			output += '-';
			output += 'r';
			i++;
		} else if (isdigit(mfb[i]) && (i < (int)mfb.size() - 1) && (mfb[i+1] == 'n')) {
			output += mfb[i];
			output += mfb[i+1];
			output += 'r';
			i++;
		} else {
			output += mfb[i];
		}
	}
	return output;
}



// END_MERGE

} // end namespace hum



