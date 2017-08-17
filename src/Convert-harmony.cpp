//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Aug  7 20:37:16 EDT 2017
// Last Modified: Mon Aug  7 20:37:19 EDT 2017
// Filename:      Convert-harmony.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/Convert-harmony.cpp
// Syntax:        C++11
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Conversions related to harmony.
//

#include "Convert.h"
#include "HumRegex.h"

namespace hum {

// START_MERGE


//////////////////////////////
//
// Convert::majorScaleBase40 -- Return the base-40 scale degree
//     tonic-intervals for each  note in a major scale.  The input is the
//     base-40 pitch-class of the root.  The default input is 0, which
//     will return a list of the intervals for each scale degree to the
//     tonic of the key.
//

vector<int> Convert::majorScaleBase40(void) {
	return {0, 6, 12, 17, 23, 29, 35};
}



//////////////////////////////
//
// Convert::minorHScaleBase40 -- Return the base-40 scale degree
//     tonic-intervals for each  note in a harmonic minor scale.  The input
//     is the base-40 pitch-class of the root.  The default input is 0, which
//     will return a list of the intervals for each scale degree to the
//     tonic of the key.
//

vector<int> Convert::minorHScaleBase40(void) {
	return {0, 6, 11, 17, 23, 28, 35};
}



//////////////////////////////
//
// Convert::keyToBase40 -- convert a Humdrum **kern key designation into
//    a base-40 integer.  Positive values are for major keys and negative
//    values are for minor keys.  (C-double-flat major is 40 rather than 0).
//    Returns 0 if no legitimate key was found.
//

int Convert::keyToBase40(const string& key) {
	string token;
	auto loc = key.find(":");
	if (loc != std::string::npos) {
		token = key.substr(0, loc);
	} else {
		token = key;
	}

	int base40 = Convert::kernToBase40(token);
	if (base40 < 0)  {
		return 0;
	}

	if (base40 >= 160) {
		base40 = -(base40 % 40);
		if (base40 == 0) {
			base40 = -40;
		}
	} else {
		base40 = base40 % 40;
		if (base40 == 0) {
			base40 = 40;
		}
	}
	return base40;
}



//////////////////////////////
//
// Convert::keyToInversion -- Extract the inversion from a **harm token.
//    Root position is 0, first inversion is 1, etc. up to 6th inversion
//    for 13th chords.
//

int Convert::keyToInversion(const string& harm) {
	for (char ch : harm) {
		if ((ch >= 'a') && (ch <= 'g')) {
			return ch - 'a';
		}
	}
	return 0;
}



//////////////////////////////
//
// Convert::makeRootInterval --
//

int Convert::makeRootInterval(const string& harm, int keyroot, int keymode) {
	int output = 0;

	string prefix;
	string postfix;
	auto loc = harm.find('/');
	if (loc != string::npos) {
		prefix = harm.substr(0, loc);
		postfix = harm.substr(loc+1, string::npos);
	} else {
		prefix = harm;
	}

	int offset = 0;
	if (!postfix.empty()) {
		offset = makeRootInterval(postfix, keyroot, keymode);
	} 
	if (prefix.empty()) {
		return offset;
	}

	if (keymode) {
		// minor mode (harmonic minor)
			  if (prefix.find("vii")   != string::npos) { output = 35; }
		else if (prefix.find( "VII")  != string::npos) { output = 35; }
		else if (prefix.find( "vi")   != string::npos) { output = 28; }
		else if (prefix.find( "VI")   != string::npos) { output = 28; }
		else if (prefix.find( "iv")   != string::npos) { output = 17; }
		else if (prefix.find( "IV")   != string::npos) { output = 17; }
		else if (prefix.find( "iii")  != string::npos) { output = 11; }
		else if (prefix.find( "III")  != string::npos) { output = 11; }
		else if (prefix.find( "ii")   != string::npos) { output =  6; }
		else if (prefix.find( "II")   != string::npos) { output =  6; }
		else if (prefix.find( "N")    != string::npos) { output =  5; }
		else if (prefix.find( "v")    != string::npos) { output = 23; }
		else if (prefix.find( "V")    != string::npos) { output = 23; }
		else if (prefix.find( "i")    != string::npos) { output =  0; }
		else if (prefix.find( "I")    != string::npos) { output =  0; }
		else if (prefix.find( "Lt")   != string::npos) { output = 18; }
		else if (prefix.find( "Gn")   != string::npos) { output = 18; }
		else if (prefix.find( "Fr")   != string::npos) { output = 18; }
		else { output = -1000; }
	} else {
		// major mode
			  if (prefix.find("vii")  != string::npos) { output = 35; }
		else if (prefix.find("VII")  != string::npos) { output = 35; }
		else if (prefix.find("vi")   != string::npos) { output = 29; }
		else if (prefix.find("VI")   != string::npos) { output = 29; }
		else if (prefix.find("iv")   != string::npos) { output = 17; }
		else if (prefix.find("IV")   != string::npos) { output = 17; }
		else if (prefix.find("iii")  != string::npos) { output = 12; }
		else if (prefix.find("III")  != string::npos) { output = 12; }
		else if (prefix.find("ii")   != string::npos) { output =  6; }
		else if (prefix.find("II")   != string::npos) { output =  6; }
		else if (prefix.find("N")    != string::npos) { output =  5; }
		else if (prefix.find("v")    != string::npos) { output = 23; }
		else if (prefix.find("V")    != string::npos) { output = 23; }
		else if (prefix.find("i")    != string::npos) { output =  0; }
		else if (prefix.find("I")    != string::npos) { output =  0; }
		else if (prefix.find("Lt")   != string::npos) { output = 18; }
		else if (prefix.find("Gn")   != string::npos) { output = 18; }
		else if (prefix.find("Fr")   != string::npos) { output = 18; }
		else { output = -1000; }
	}

	int flatcount = 0;
	int sharpcount = 0;

	for (int i=0; i<(int)prefix.size(); i++) {
		if (prefix[i] == '-') {
			flatcount++;
		} else if (prefix[i] == '#') {
			sharpcount++;
		}
	}

	if (output < 0) {
		return output;
	} else {
		output += sharpcount;
		output -= flatcount;
		return (output + offset) % 40;
	}
}



//////////////////////////////
//
// Convert::harmToBase40 -- Convert a **harm chord into a list of
//   pitch classes contained in the chord.  The output is a vector
//   that contains the root pitch class in the first slot, then
//   the successive chord tones after that.  If the vector is empty
//   then there was some sort of syntax error in the **harm token.
//   The bass note is placed in the 3rd octave and other pitch classes
//   in the chord are placed in the 4th octave.
//

vector<int> Convert::harmToBase40(const string& harm, const string& key) {
	int keyroot = Convert::keyToBase40(key);
	int keymode = 0; // major key
	if (keyroot < 0) {
		keyroot = -keyroot;
		keymode = 1; // minor key
	}
	return harmToBase40(harm, keyroot, keymode);
}


vector<int> Convert::harmToBase40(const string& harm, int keyroot, int keymode) {
	// Create a tonic-interval list of the scale-degrees:
	vector<int> degrees;
	if (keymode == 1) {
		degrees = Convert::minorHScaleBase40();
	} else {
		degrees = Convert::majorScaleBase40();
	}

	// Remove alternate chord labels:
	string single;
	auto loc = harm.find('[');
	if (loc != string::npos) {
		single = harm.substr(0, loc);
	} else {
		single = harm;
	}

	// Split off secondary dominant qualifications
	string cbase;     // base chord
	string secondary; // secondary chord qualifiers
	loc = single.find("/");
	if (loc != string::npos) {
		cbase = single.substr(0, loc);
		secondary = single.substr(loc+1, string::npos);
	} else {
		cbase = single;
	}

	// Calculate interval offset for secondary dominants:
	int secoffset = makeRootInterval(secondary, keyroot, keymode);

	int rootdeg = -1; // chord root scale degree in key
	int degalt = 0;   // degree alteration

	vector<char> chars(256, 0);
	for (auto ch : cbase) {
		chars[ch]++;
	}

	rootdeg = -1; // invalid scale degree
	degalt = chars['#'] - chars['-'];

	int vcount = chars['V'] + chars['v'];
	int icount = chars['I'] + chars['i'];

	if (vcount == 1) {
		switch (icount) {
			case 0: rootdeg = 4; break; // V
			case 1:
				if (cbase.find("IV") != string::npos) {
					rootdeg = 3; break; // IV
				} else if (cbase.find("iv") != string::npos) {
					rootdeg = 3; break; // iv
				} else {
					rootdeg = 5; break; // VI/vi
				}
			case 2: rootdeg = 6; break; // VII
			case 3: rootdeg = 0; break; // VIII (I)
		}
	} else {
		switch (icount) {
			case 0:  // N, Fr, Gn, Lt, Tr
				if (chars['N']) {
					// Neapolitan (flat-second scale degree)
					rootdeg = 1; // -II
					degalt += -1; // -II
				} else if (chars['L'] || chars['F'] || chars['G']) {
					// augmented 6th chord on -VII
					rootdeg = 5;
					// fixed to -VI of major scale:
					if (keymode == 0) { // major
						degalt += -1;
					} else { // minor
						// already at -VI in minor
						degalt += 0;
					}
				}
				break;
			case 1: rootdeg = 0; break; // I
			case 2: rootdeg = 1; break; // II
			case 3: rootdeg = 2; break; // III
		}
	}

	int inversion = Convert::keyToInversion(harm);
	vector<int> output;

	if (rootdeg < 0) {
		return output;
	}

	int root = degrees.at(rootdeg) + keyroot + secoffset;
	output.push_back(root);

	int int3  = -1;
	int int5  = 23;  // assume a perfect 5th
	int int7  = -1;
	int int9  = -1;
	// int int11 = -1;
	// int int13 = -1;

	// determine the third's interval
	if (chars['i'] || chars['v']) {
		// minor third
		int3 = 11;
	} else if (chars['I'] || chars['V']) {
		// major third
		int3 = 12;
	} else if (chars['N']) {
		// neapolitan (major triad)
		int3 = 12;
		int5 = 23;
	} else if (chars['G']) {
		// german aug. 6th chord
		int3 = 12;
		int5 = 23;
		int7 = 30; // technically on 6th
	} else if (chars['L']) {
		// Italian aug. 6th chord
		int3 = 12;
		int5 = -1;
		int7 = 30; // technically on 6th
	} else if (chars['F']) {
		// French aug. 6th chord
		int3 = 12;
		int5 = 18; // technically on 4th
		int7 = 30; // technically on 6th
	}

	// determine the fifth's interval
	if (chars['o']) { // diminished
		int5 = 22;
	}
	if (chars['+']) { // augmented
		int5 = 24;
	}

	if (int3 > 0) {
		output.push_back(int3 + output[0]);
	}
	if (int5 > 0) {
		output.push_back(int5 + output[0]);
	}


	///// determine higher chord notes

	// determine the seventh
	if (chars['7']) {
		HumRegex hre;
		int7 = degrees.at((rootdeg + 6) % 7) - degrees.at(rootdeg) + secoffset;
		if (int7 < 0) {
			int7 += 40;
		}
		if (hre.search(cbase, "(A+|D+|M|m)7")) {
			string quality = hre.getMatch(1);
			if (quality == "M") {
				int7 = 35;
			} else if (quality == "m") {
				int7 = 34;
			} else if (quality[0] == 'D') {
				int7 = 34 - (int)quality.size();
			} else if (quality[0] == 'A') {
				int7 = 35 + (int)quality.size();
			}
		}
		output.push_back(int7 % 40 + output[0]);
	}

	// determine the 9th
	if (chars['9']) {
		HumRegex hre;
		int9 = degrees.at((rootdeg + 1) % 7) - degrees.at(rootdeg) + secoffset;
		if (int9 < 0) {
			int9 += 40;
		}
		if (hre.search(cbase, "(A+|D+|M|m)9")) {
			string quality = hre.getMatch(1);
			if (quality == "M") {
				int9 = 46;
			} else if (quality == "m") {
				int9 = 45;
			} else if (quality[0] == 'D') {
				int9 = 45 - (int)quality.size();
			} else if (quality[0] == 'A') {
				int9 = 46 + (int)quality.size();
			}
		}
		output.push_back(int9 + output[0]);
	}


	// add inverion
	if (inversion < (int)output.size()) {
		output[inversion] = output[inversion] % 40 + 3 * 40;
	}

	int oct = 4;
	int lastvalue = -1;
	for (int i=0; i<(int)output.size(); i++) {
		if (i != inversion) {
			output[i] = output[i] % 40 + oct * 40;
			if (output[i] < lastvalue) {
				output[i] += 40;
			}
			if (output[i] < lastvalue) {
				output[i] += 40;
			}
			lastvalue = output[i];
		} else {
		}
	}

	return output;

}



// END_MERGE

} // end namespace hum



