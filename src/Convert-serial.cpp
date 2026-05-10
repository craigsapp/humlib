//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Aug  8 12:24:49 PDT 2015
// Last Modified: Wed Aug 19 00:06:39 PDT 2015
// Filename:      Convert-serial.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/Convert-serial.cpp
// Syntax:        C++11; humlib
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Conversions related to pitch.
//

#include "Convert.h"
#include "HumRegex.h"

#include <cctype>
#include <cmath>
#include <vector>

using namespace std;

namespace hum {

// START_MERGE


//////////////////////////////
//
// Convert::midiPitchToClass --
//

vector<int> Convert::pitchToClass(const vector<int>& notes, int octave, int modulo) {
    map<int, bool> seen;
    vector<int> output;

    for (int note : notes) {
	if (note <= 0) {
		continue; // pitch = 0 means rests (ignore).
	}
        int pc = note % modulo;

        seen[pc] = true;
    }

    // extract unique values (sorted because map is ordered)
    for (auto& kv : seen) {
        output.push_back(kv.first + modulo*octave);
    }

    return output;
}



//////////////////////////////
//
// Convert::getMidiPCTriadAbbr -- reurn M = major, m = minor, d = diminished, A = augmented
//      Input PC are MIDI pitch classes.  Input PCs are assumed sorted and unique.
//
//    pcs.size() == 0 → "R" (Rest)
//    pcs.size() == 1 → "U" (Unison)
//    pcs.size() == 2
//    interval 7      → "-5" (Open fifth)
//    interval 3      → "-m" (minor 3rd dyad)
//    interval 4      → "-M" (major 3rd dyad)
//    pcs.size() == 3
//    4,3 → "M"              (Major triad)
//    3,4 → "m"              (Minor triad)
//    3,3 → "d"              (Diminished triad)
//    4,4 → "A"              (Augmented triad)
//

string Convert::getMidiPCTriadAbbr(const vector<int>& pcs) {
    if (pcs.size() == 0) return "R"; // rest
    if (pcs.size() == 1) return "U"; // Unison

    if (pcs.size() == 2) {
        int a = pcs[0];
        int b = pcs[1];

        int d = (b - a + 12) % 12;  // assumes pcs already 0–11

        if (d == 7) return "-5";     // open fifth
        if (d == 3) return "-m";     // minor third dyad
        if (d == 4) return "-M";     // major third dyad
        return "??";
    }

    if (pcs.size() > 3) return "+";  // more than 3pcs (so not a triad)

    // Resolve triads:

    const vector<int>& p = pcs;

    for (int r = 0; r < 3; r++) {
        int a = p[r];
        int b = p[(r + 1) % 3];
        int c = p[(r + 2) % 3];

        if (b < a) b += 12;
        if (c < b) c += 12;

        int d1 = b - a;
        int d2 = c - b;

        if (d1 == 4 && d2 == 3) return "M";
        if (d1 == 3 && d2 == 4) return "m";
        if (d1 == 3 && d2 == 3) return "d";
        if (d1 == 4 && d2 == 4) return "A";
    }

    return "???";

}



// END_MERGE

} // end namespace hum



