//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Fri Jun 17 14:31:58 PDT 2016
// Last Modified: Sat Jun 18 17:20:28 PDT 2016
// Filename:      HumdrumFileContent-accidental.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/HumdrumFileContent-accidental.cpp
// Syntax:        C++11
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Content analysis of accidentals.
//

#include "HumdrumFileContent.h"
#include "Convert.h"

using namespace std;

namespace hum {

// START_MERGE



//////////////////////////////
//
// HumdrumFileContent::analyzeKernAccidentals -- Identify accidentals that
//    should be printed (only in **kern spines) as well as cautionary
//    accidentals (accidentals which are forced to be displayed but otherwise
//    would not be printed.  Algorithm assumes that all secondary tied notes
//    will not display their accidental across a system break.  Consideration
//    about grace-note accidental display still needs to be done.
//

bool HumdrumFileContent::analyzeKernAccidentals(void) {
	HumdrumFileContent& infile = *this;
	int i, j, k;
	int kindex;
	int track;

	// ktracks == List of **kern spines in data.
	// rtracks == Reverse mapping from track to ktrack index (part/staff index).
	vector<HTp> ktracks = getKernSpineStartList();
	vector<int> rtracks(getMaxTrack()+1, -1);
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
			                        // may cause problems; fix later.
		std::fill(dstates[i].begin(), dstates[i].end(), 0);
	}

	// gdstates == grace note diatonic states for every pitch in a spine.
	vector<vector<int> > gdstates; // grace-note diatonic states
	gdstates.resize(kcount);
	for (i=0; i<kcount; i++) {
		gdstates[i].resize(70);
		std::fill(gdstates[i].begin(), gdstates[i].end(), 0);
	}

	// rhythmstart == keep track of first beat in measure.
	vector<int> firstinbar(kcount, 0);

	int lasttrack = -1;
	vector<int> concurrentstate(70, 0);
	
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
					resetDiatonicStatesWithKeySignature(gdstates[kindex],
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
				std::fill(firstinbar.begin(), firstinbar.end(), 1);
				track = infile[i].token(j)->getTrack();
				kindex = rtracks[track];
				// reset the accidental states in dstates to match keysigs.
				resetDiatonicStatesWithKeySignature(dstates[kindex],
						keysigs[kindex]);
				resetDiatonicStatesWithKeySignature(gdstates[kindex],
						keysigs[kindex]);
			}
		}

		if (!infile[i].isData()) {
			continue;
		}

		fill(concurrentstate.begin(), concurrentstate.end(), 0);
		lasttrack = -1;

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

			if (lasttrack != track) {
				fill(concurrentstate.begin(), concurrentstate.end(), 0);
			}
			lasttrack = track;

			int rindex = rtracks[track];
			for (k=0; k<subcount; k++) {
				string subtok = infile[i].token(j)->getSubtoken(k);
				int b40 = Convert::kernToBase40(subtok);
				int diatonic = Convert::kernToBase7(subtok);
				if (diatonic < 0) {
					// Deal with extra-low notes later.
					continue;
				}
				int graceQ = infile[i].token(j)->isGrace();
				int accid = Convert::kernToAccidentalCount(subtok);
				int hiddenQ = 0;
				if (subtok.find("yy") == string::npos) {
					if ((subtok.find("ny") != string::npos) ||
					    (subtok.find("#y") != string::npos) ||
					    (subtok.find("-y") != string::npos)) {
						hiddenQ = 1;
					}
				}

				if (((subtok.find("_") != string::npos) ||
						(subtok.find("]") != string::npos))) {
					// tied notes do not have slurs, so skip them
					if ((accid != keysigs[rindex][diatonic % 7]) &&
							firstinbar[rindex]) {
						// But first, prepare to force an accidental to be shown on
						// the note immediately following the end of a tied group
						// if the tied group crosses a barline.
						dstates[rindex][diatonic] = -1000 + accid;
						gdstates[rindex][diatonic] = -1000 + accid;
					}
					auto loc = subtok.find('X');
					if (loc == string::npos) {
						continue;
					} else if (loc == 0) {
						continue;
					} else {
						if (!((subtok[loc-1] == '#') || (subtok[loc-1] != '-') ||
								(subtok[loc-1] == 'n'))) {
							continue;
						} else {
							// an accident should be fored at end of tie
						}
					}
				}

				size_t loc;
				// check for accidentals on trills, mordents and turns.
				if (subtok.find("t") != string::npos) {
					// minor second trill
					int trillnote     = b40 + 5;
					int trilldiatonic = Convert::base40ToDiatonic(trillnote);
					int trillaccid    = Convert::base40ToAccidental(trillnote);
					if (dstates[rindex][trilldiatonic] != trillaccid) {
						infile[i].token(j)->setValue("auto", to_string(k),
								"trillAccidental", to_string(trillaccid));
						dstates[rindex][trilldiatonic] = -1000 + trillaccid;
					}
				} else if (subtok.find("T") != string::npos) {
					// major second trill
					int trillnote     = b40 + 6;
					int trilldiatonic = Convert::base40ToDiatonic(trillnote);
					int trillaccid    = Convert::base40ToAccidental(trillnote);
					if (dstates[rindex][trilldiatonic] != trillaccid) {
						infile[i].token(j)->setValue("auto", to_string(k),
								"trillAccidental", to_string(trillaccid));
						dstates[rindex][trilldiatonic] = -1000 + trillaccid;
					}
				} else if (subtok.find("M") != string::npos) {
					// major second upper mordent
					int auxnote     = b40 + 6;
					int auxdiatonic = Convert::base40ToDiatonic(auxnote);
					int auxaccid    = Convert::base40ToAccidental(auxnote);
					if (dstates[rindex][auxdiatonic] != auxaccid) {
						infile[i].token(j)->setValue("auto", to_string(k),
								"mordentUpperAccidental", to_string(auxaccid));
						dstates[rindex][auxdiatonic] = -1000 + auxaccid;
					}
				} else if (subtok.find("m") != string::npos) {
					// minor second upper mordent
					int auxnote     = b40 + 5;
					int auxdiatonic = Convert::base40ToDiatonic(auxnote);
					int auxaccid    = Convert::base40ToAccidental(auxnote);
					if (dstates[rindex][auxdiatonic] != auxaccid) {
						infile[i].token(j)->setValue("auto", to_string(k),
								"mordentUpperAccidental", to_string(auxaccid));
						dstates[rindex][auxdiatonic] = -1000 + auxaccid;
					}
				} else if (subtok.find("W") != string::npos) {
					// major second upper mordent
					int auxnote     = b40 - 6;
					int auxdiatonic = Convert::base40ToDiatonic(auxnote);
					int auxaccid    = Convert::base40ToAccidental(auxnote);
					if (dstates[rindex][auxdiatonic] != auxaccid) {
						infile[i].token(j)->setValue("auto", to_string(k),
								"mordentLowerAccidental", to_string(auxaccid));
						dstates[rindex][auxdiatonic] = -1000 + auxaccid;
					}
				} else if (subtok.find("w") != string::npos) {
					// minor second upper mordent
					int auxnote     = b40 - 5;
					int auxdiatonic = Convert::base40ToDiatonic(auxnote);
					int auxaccid    = Convert::base40ToAccidental(auxnote);
					if (dstates[rindex][auxdiatonic] != auxaccid) {
						infile[i].token(j)->setValue("auto", to_string(k),
								"mordentLowerAccidental", to_string(auxaccid));
						dstates[rindex][auxdiatonic] = -1000 + auxaccid;
					}

				} else if ((loc = subtok.find("$")) != string::npos) {

					int turndiatonic = Convert::base40ToDiatonic(b40);
					// int turnaccid = Convert::base40ToAccidental(b40);
					// inverted turn
					int lowerint = 0;
					int upperint = 0;
					if (loc < subtok.size()-1) {
						if (subtok[loc+1] == 's') {
							lowerint = -5;
						} else if (subtok[loc+1] == 'S') {
							lowerint = -6;
						}
					}
					if (loc < subtok.size()-2) {
						if (subtok[loc+2] == 's') {
							upperint = +5;
						} else if (subtok[loc+2] == 'S') {
							upperint = +6;
						}
					}
					int lowerdiatonic = turndiatonic - 1;
					// Maybe also need to check for forced accidental state...
					int loweraccid = dstates[rindex][lowerdiatonic];
					int lowerb40 = Convert::base7ToBase40(lowerdiatonic) + loweraccid;
					int upperdiatonic = turndiatonic + 1;
					// Maybe also need to check for forced accidental state...
					int upperaccid = dstates[rindex][upperdiatonic];
					int upperb40 = Convert::base7ToBase40(upperdiatonic) + upperaccid;
					if (lowerint == 0) {
						// need to calculate lower interval (but it will not appear
						// below the inverted turn, just calculating for performance
						// rendering.
						lowerint = lowerb40 - b40;
						lowerb40 = b40 + lowerint;
					}
					if (upperint == 0) {
						// need to calculate upper interval (but it will not appear
						// above the inverted turn, just calculating for performance
						// rendering.
						upperint = upperb40 - b40;
						upperb40 = b40 + upperint;
					}
					int uacc = Convert::base40ToAccidental(b40 + upperint);
					int bacc = Convert::base40ToAccidental(b40 + lowerint);
					if (uacc != upperaccid) {
						infile[i].token(j)->setValue("auto", to_string(k),
								"turnUpperAccidental", to_string(uacc));
						dstates[rindex][upperdiatonic] = -1000 + uacc;
					}
					if (bacc != loweraccid) {
						infile[i].token(j)->setValue("auto", to_string(k),
								"turnLowerAccidental", to_string(bacc));
						dstates[rindex][lowerdiatonic] = -1000 + bacc;
					}

				} else if ((loc = subtok.find("S")) != string::npos) {

					int turndiatonic = Convert::base40ToDiatonic(b40);
					// int turnaccid = Convert::base40ToAccidental(b40);
					// regular turn
					int lowerint = 0;
					int upperint = 0;
					if (loc < subtok.size()-1) {
						if (subtok[loc+1] == 's') {
							upperint = +5;
						} else if (subtok[loc+1] == 'S') {
							upperint = +6;
						}
					}
					if (loc < subtok.size()-2) {
						if (subtok[loc+2] == 's') {
							lowerint = -5;
						} else if (subtok[loc+2] == 'S') {
							lowerint = -6;
						}
					}
					int lowerdiatonic = turndiatonic - 1;
					// Maybe also need to check for forced accidental state...
					int loweraccid = dstates[rindex][lowerdiatonic];
					int lowerb40 = Convert::base7ToBase40(lowerdiatonic) + loweraccid;
					int upperdiatonic = turndiatonic + 1;
					// Maybe also need to check for forced accidental state...
					int upperaccid = dstates[rindex][upperdiatonic];
					int upperb40 = Convert::base7ToBase40(upperdiatonic) + upperaccid;
					if (lowerint == 0) {
						// need to calculate lower interval (but it will not appear
						// below the inverted turn, just calculating for performance
						// rendering.
						lowerint = lowerb40 - b40;
						lowerb40 = b40 + lowerint;
					}
					if (upperint == 0) {
						// need to calculate upper interval (but it will not appear
						// above the inverted turn, just calculating for performance
						// rendering.
						upperint = upperb40 - b40;
						upperb40 = b40 + upperint;
					}
					int uacc = Convert::base40ToAccidental(b40 + upperint);
					int bacc = Convert::base40ToAccidental(b40 + lowerint);
					if (uacc != upperaccid) {
						infile[i].token(j)->setValue("auto", to_string(k),
								"turnUpperAccidental", to_string(uacc));
						dstates[rindex][upperdiatonic] = -1000 + uacc;
					}
					if (bacc != loweraccid) {
						infile[i].token(j)->setValue("auto", to_string(k),
								"turnLowerAccidental", to_string(bacc));
						dstates[rindex][lowerdiatonic] = -1000 + bacc;
					}
				}

				if (graceQ && (accid != gdstates[rindex][diatonic])) {
					// accidental is different from the previous state so should be
					// printed
					if (!hiddenQ) {
						infile[i].token(j)->setValue("auto", to_string(k),
								"visualAccidental", "true");
						if (gdstates[rindex][diatonic] < -900) {
							// this is an obligatory cautionary accidental
							// or at least half the time it is (figure that out later)
							infile[i].token(j)->setValue("auto", to_string(k),
									"obligatoryAccidental", "true");
							infile[i].token(j)->setValue("auto", to_string(k),
									"cautionaryAccidental", "true");
						}
					}
					gdstates[rindex][diatonic] = accid;
					// regular notes are not affected by grace notes accidental
					// changes, but should have an obligatory cautionary accidental,
					// displayed for clarification.
					dstates[rindex][diatonic] = -1000 + accid;

				} else if (!graceQ && ((concurrentstate[diatonic] && (concurrentstate[diatonic] == accid)) 
						|| (accid != dstates[rindex][diatonic]))) {
					// accidental is different from the previous state so should be
					// printed, but only print if not supposed to be hidden.
					if (!hiddenQ) {
						infile[i].token(j)->setValue("auto", to_string(k),
								"visualAccidental", "true");
						concurrentstate[diatonic] = accid;
						if (dstates[rindex][diatonic] < -900) {
							// this is an obligatory cautionary accidental
							// or at least half the time it is (figure that out later)
							infile[i].token(j)->setValue("auto", to_string(k),
									"obligatoryAccidental", "true");
							infile[i].token(j)->setValue("auto", to_string(k),
									"cautionaryAccidental", "true");
						}
					}
					dstates[rindex][diatonic] = accid;
					gdstates[rindex][diatonic] = accid;

				} else if ((accid == 0) && (subtok.find("n") != string::npos) &&
							!hiddenQ) {
					infile[i].token(j)->setValue("auto", to_string(k),
							"cautionaryAccidental", "true");
					infile[i].token(j)->setValue("auto", to_string(k),
							"visualAccidental", "true");
				} else if (subtok.find("XX") == string::npos) {
					// The accidental is not necessary. See if there is a single "X"
					// immediately after the accidental which means to force it to
					// display.
					auto loc = subtok.find("X");
					if ((loc != string::npos) && (loc > 0)) {
						if (subtok[loc-1] == '#') {
							infile[i].token(j)->setValue("auto", to_string(k),
									"cautionaryAccidental", "true");
									infile[i].token(j)->setValue("auto", to_string(k),
											"visualAccidental", "true");
						} else if (subtok[loc-1] == '-') {
							infile[i].token(j)->setValue("auto", to_string(k),
									"cautionaryAccidental", "true");
									infile[i].token(j)->setValue("auto", to_string(k),
											"visualAccidental", "true");
						} else if (subtok[loc-1] == 'n') {
							infile[i].token(j)->setValue("auto", to_string(k),
									"cautionaryAccidental", "true");
							infile[i].token(j)->setValue("auto", to_string(k),
									"visualAccidental", "true");
						}
					}
				}
			}
		}
		std::fill(firstinbar.begin(), firstinbar.end(), 0);
	}

	// Indicate that the accidental analysis has been done:
	infile.setValue("auto", "accidentalAnalysis", "true");

	return true;
}



//////////////////////////////
//
// HumdrumFileContent::fillKeySignature -- Read key signature notes and
//    assign +1 to sharps, -1 to flats in the diatonic input array.  Used
//    only by HumdrumFileContent::analyzeKernAccidentals().
//

void HumdrumFileContent::fillKeySignature(vector<int>& states,
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
// HumdrumFileContent::resetDiatonicStatesWithKeySignature -- Only used in
//     HumdrumFileContent::analyzeKernAccidentals().  Resets the accidental
//     states for notes
//

void HumdrumFileContent::resetDiatonicStatesWithKeySignature(vector<int>&
		states, vector<int>& signature) {
	for (int i=0; i<(int)states.size(); i++) {
		states[i] = signature[i % 7];
	}
}


// END_MERGE

} // end namespace hum



