//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Aug 25 14:12:42 PDT 2018
// Last Modified: Sat Aug 25 19:47:08 PDT 2018
// Filename:      tool-trillspell.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-trillspell.cpp
// Syntax:        C++11; humlib
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Interface for trill tool, which assigns intervals to
//                trill, mordent and turn ornaments based on the key
//                signature and previous notes in a measure.
//

#include "tool-trillspell.h"
#include "Convert.h"
#include "HumRegex.h"

#include <algorithm>
#include <cmath>

using namespace std;

namespace hum {

// START_MERGE


/////////////////////////////////
//
// Tool_trillspell::Tool_trillspell -- Set the recognized options for the tool.
//

Tool_trillspell::Tool_trillspell(void) {
	define("x=b", "mark trills with x (interpretation)");
}



///////////////////////////////
//
// Tool_trillspell::run -- Primary interfaces to the tool.
//

bool Tool_trillspell::run(HumdrumFileSet& infiles) {
	bool status = true;
	for (int i=0; i<infiles.getCount(); i++) {
		status &= run(infiles[i]);
	}
	return status;
}

bool Tool_trillspell::run(const string& indata, ostream& out) {
	HumdrumFile infile(indata);
	return run(infile, out);
}


bool Tool_trillspell::run(HumdrumFile& infile, ostream& out) {
	bool status = run(infile);
	return status;
}


bool Tool_trillspell::run(HumdrumFile& infile) {
	processFile(infile);
	infile.createLinesFromTokens();
	return true;
}



///////////////////////////////
//
// Tool_trillspell::processFile -- Adjust intervals of ornaments.
//

void Tool_trillspell::processFile(HumdrumFile& infile) {
	m_xmark = getBoolean("x");
	analyzeOrnamentAccidentals(infile);
}



//////////////////////////////
//
// Tool_trillspell::analyzeOrnamentAccidentals --
//

bool Tool_trillspell::analyzeOrnamentAccidentals(HumdrumFile& infile) {
	int i, j, k;
	int kindex;
	int track;

	// ktracks == List of **kern spines in data.
	// rtracks == Reverse mapping from track to ktrack index (part/staff index).
	vector<HTp> ktracks = infile.getKernSpineStartList();
	vector<int> rtracks(infile.getMaxTrack()+1, -1);
	for (i=0; i<(int)ktracks.size(); i++) {
		track = ktracks[i]->getTrack();
		rtracks[track] = i;
	}
	int kcount = (int)ktracks.size();

	// keysigs == key signature spellings of diatonic pitch classes.  This array
	// is duplicated into dstates after each barline.
	vector<vector<int> > keysigs;
	keysigs.resize(kcount);
	for (i=0; i<kcount; i++) {
		keysigs[i].resize(7);
		std::fill(keysigs[i].begin(), keysigs[i].end(), 0);
	}

	// dstates == diatonic states for every pitch in a spine.
	// sub-spines are considered as a single unit, although there are
	// score conventions which would keep a separate voices on a staff
	// with different accidental states (i.e., two parts superimposed
	// on the same staff, but treated as if on separate staves).
	// Eventually this algorithm should be adjusted for dealing with
	// cross-staff notes, where the cross-staff notes should be following
	// the accidentals of a different spine...
	vector<vector<int> > dstates; // diatonic states
	dstates.resize(kcount);
	for (i=0; i<kcount; i++) {
		dstates[i].resize(70);     // 10 octave limit for analysis
			                        // may cause problems; maybe fix later.
		std::fill(dstates[i].begin(), dstates[i].end(), 0);
	}

	for (i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].hasSpines()) {
			continue;
		}
		if (infile[i].isInterpretation()) {
			for (j=0; j<infile[i].getFieldCount(); j++) {
				if (!infile[i].token(j)->isKern()) {
					continue;
				}
				if (infile[i].token(j)->compare(0, 3, "*k[") == 0) {
					track = infile[i].token(j)->getTrack();
					kindex = rtracks[track];
					fillKeySignature(keysigs[kindex], *infile[i].token(j));
					// resetting key states of current measure.  What to do if this
					// key signature is in the middle of a measure?
					resetDiatonicStatesWithKeySignature(dstates[kindex],
							keysigs[kindex]);
				}
			}
		} else if (infile[i].isBarline()) {
			for (j=0; j<infile[i].getFieldCount(); j++) {
				if (!infile[i].token(j)->isKern()) {
					continue;
				}
				if (infile[i].token(j)->isInvisible()) {
					continue;
				}
				track = infile[i].token(j)->getTrack();
				kindex = rtracks[track];
				// reset the accidental states in dstates to match keysigs.
				resetDiatonicStatesWithKeySignature(dstates[kindex],
						keysigs[kindex]);
			}
		}

		if (!infile[i].isData()) {
			continue;
		}

		for (j=0; j<infile[i].getFieldCount(); j++) {
			if (!infile[i].token(j)->isKern()) {
				continue;
			}
			if (infile[i].token(j)->isNull()) {
				continue;
			}
			if (infile[i].token(j)->isRest()) {
				continue;
			}

			int subcount = infile[i].token(j)->getSubtokenCount();
			track = infile[i].token(j)->getTrack();

			HumRegex hre;
			int rindex = rtracks[track];
			for (k=0; k<subcount; k++) {
				string subtok = infile[i].token(j)->getSubtoken(k);
				int b40 = Convert::kernToBase40(subtok);
				int diatonic = Convert::kernToBase7(subtok);
				if (diatonic < 0) {
					// Deal with extra-low notes later.
					continue;
				}
				int accid = Convert::kernToAccidentalCount(subtok);
				dstates.at(rindex).at(diatonic) = accid;

				// check for accidentals on trills, mordents and turns.
				// N.B.: augmented-second intervals are not considered.

				if ((subtok.find("t") != string::npos) && !hre.search(subtok, "[tT]x")) {
					int nextup = getBase40(diatonic + 1, dstates[rindex][diatonic+1]);
					int interval = nextup - b40;
					if (interval == 6) {
						// Set to major-second trill
						hre.replaceDestructive(subtok, "T", "t", "g");
						if (m_xmark) {
							hre.replaceDestructive(subtok, "$1x", "([Tt]+)", "g");
						}
						infile[i].token(j)->replaceSubtoken(k, subtok);
					} else {
						if (m_xmark) {
							hre.replaceDestructive(subtok, "$1x", "([Tt]+)", "g");
						}
						infile[i].token(j)->replaceSubtoken(k, subtok);
					}
				} else if ((subtok.find("T") != string::npos) && !hre.search(subtok, "[tT]x")) {
					int nextup = getBase40(diatonic + 1, dstates[rindex][diatonic+1]);
					int interval = nextup - b40;
					if (interval == 5) {
						// Set to minor-second trill
						hre.replaceDestructive(subtok, "t", "T", "g");
						if (m_xmark) {
							hre.replaceDestructive(subtok, "$1x", "([Tt]+)", "g");
						}
						infile[i].token(j)->replaceSubtoken(k, subtok);
					} else {
						if (m_xmark) {
							hre.replaceDestructive(subtok, "$1x", "([Tt]+)", "g");
						}
						infile[i].token(j)->replaceSubtoken(k, subtok);
					}
				} else if ((subtok.find("M") != string::npos) && !hre.search(subtok, "[Mm]x")) {
					// major-second upper mordent
					int nextup = getBase40(diatonic + 1, dstates[rindex][diatonic+1]);
					int interval = nextup - b40;
					if (interval == 5) {
						// Set to minor-second upper mordent
						hre.replaceDestructive(subtok, "m", "M", "g");
						if (m_xmark) {
							hre.replaceDestructive(subtok, "$1x", "([Mm]+)", "g");
						}
						infile[i].token(j)->replaceSubtoken(k, subtok);
					} else {
						if (m_xmark) {
							hre.replaceDestructive(subtok, "$1x", "([Mm]+)", "g");
						}
						infile[i].token(j)->replaceSubtoken(k, subtok);
					}
				} else if ((subtok.find("m") != string::npos) && !hre.search(subtok, "[Mm]x")) {
					// minor-second upper mordent
					int nextup = getBase40(diatonic + 1, dstates[rindex][diatonic+1]);
					int interval = nextup - b40;
					if (interval == 6) {
						// Set to major-second upper mordent
						hre.replaceDestructive(subtok, "M", "m", "g");
						if (m_xmark) {
							hre.replaceDestructive(subtok, "$1x", "([Mm]+)", "g");
						}
						infile[i].token(j)->replaceSubtoken(k, subtok);
					} else {
						if (m_xmark) {
							hre.replaceDestructive(subtok, "$1x", "([Mm]+)", "g");
						}
						infile[i].token(j)->replaceSubtoken(k, subtok);
					}
				} else if ((subtok.find("W") != string::npos) && !hre.search(subtok, "[Ww]x")) {
					// major-second lower mordent
					int nextdn = getBase40(diatonic - 1, dstates[rindex][diatonic-1]);
					int interval = b40 - nextdn;
					if (interval == 5) {
						// Set to minor-second lower mordent
						hre.replaceDestructive(subtok, "w", "W", "g");
						if (m_xmark) {
							hre.replaceDestructive(subtok, "$1x", "([Ww]+)", "g");
						}
						infile[i].token(j)->replaceSubtoken(k, subtok);
					} else {
						if (m_xmark) {
							hre.replaceDestructive(subtok, "$1x", "([Ww]+)", "g");
						}
						infile[i].token(j)->replaceSubtoken(k, subtok);
					}
				} else if ((subtok.find("w") != string::npos) && !hre.search(subtok, "[Ww]x")) {
					// minor-second lower mordent
					int nextdn = getBase40(diatonic - 1, dstates[rindex][diatonic-1]);
					int interval = b40 - nextdn;
					if (interval == 6) {
						// Set to major-second lower mordent
						hre.replaceDestructive(subtok, "W", "w", "g");
						if (m_xmark) {
							hre.replaceDestructive(subtok, "$1x", "([Ww]+)", "g");
						}
						infile[i].token(j)->replaceSubtoken(k, subtok);
					} else {
						if (m_xmark) {
							hre.replaceDestructive(subtok, "$1x", "([Ww]+)", "g");
						}
						infile[i].token(j)->replaceSubtoken(k, subtok);
					}
				}
				// deal with turns and inverted turns here.

			}
		}
	}

	return true;
}



//////////////////////////////
//
// Tool_trillspell::resetDiatonicStatesWithKeySignature -- Only used in
//     Tool_trillspell::analyzeKernAccidentals().  Resets the accidental
//     states for notes
//

void Tool_trillspell::resetDiatonicStatesWithKeySignature(vector<int>&
		states, vector<int>& signature) {
	for (int i=0; i<(int)states.size(); i++) {
		states[i] = signature[i % 7];
	}
}



//////////////////////////////
//
// Tool_trillspell::fillKeySignature -- Read key signature notes and
//    assign +1 to sharps, -1 to flats in the diatonic input array.  Used
//    only by Tool_trillspell::analyzeOrnamentAccidentals().
//

void Tool_trillspell::fillKeySignature(vector<int>& states,
		const string& keysig) {
	std::fill(states.begin(), states.end(), 0);
	if (keysig.find("f#") != string::npos) { states[3] = +1; }
	if (keysig.find("c#") != string::npos) { states[0] = +1; }
	if (keysig.find("g#") != string::npos) { states[4] = +1; }
	if (keysig.find("d#") != string::npos) { states[1] = +1; }
	if (keysig.find("a#") != string::npos) { states[5] = +1; }
	if (keysig.find("e#") != string::npos) { states[2] = +1; }
	if (keysig.find("b#") != string::npos) { states[6] = +1; }
	if (keysig.find("b-") != string::npos) { states[6] = -1; }
	if (keysig.find("e-") != string::npos) { states[2] = -1; }
	if (keysig.find("a-") != string::npos) { states[5] = -1; }
	if (keysig.find("d-") != string::npos) { states[1] = -1; }
	if (keysig.find("g-") != string::npos) { states[4] = -1; }
	if (keysig.find("c-") != string::npos) { states[0] = -1; }
	if (keysig.find("f-") != string::npos) { states[3] = -1; }
}



//////////////////////////////
//
// Tool_trillspell::getBase40 --
//

int Tool_trillspell::getBase40(int diatonic, int accidental) {
	return Convert::base7ToBase40(diatonic) + accidental;
}


// END_MERGE

} // end namespace hum



