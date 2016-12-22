//
// Programmer:    Alexander Morgan
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Aug  8 12:24:49 PDT 2015
// Last Modified: Wed Nov 30 01:02:57 PST 2016
// Filename:      tool-testgrid.h
// URL:           https://github.com/craigsapp/minHumdrum/blob/master/src/tool-testgrid.cpp
// Syntax:        C++11
// vim:           ts=3 noexpandtab
//
// Description:   Example of extracting a 2D pitch grid from
//                a score for dissonance analysis.
//

#include "tool-testgrid.h"

#include <algorithm>
#include <cmath>

using namespace std;

namespace hum {

// START_MERGE


/////////////////////////////////
//
// Tool_testgrid::Tool_testgrid -- Set the recognized options for the tool.
//

Tool_testgrid::Tool_testgrid(void) {
	define("r|raw=b",             "print raw grid");
	define("d|diatonic=b",        "print diatonic grid");
	define("D|no-dissonant=b",    "don't do dissonance anaysis");
	define("m|midi-pitch=b",      "print midi-pitch grid");
	define("b|base-40=b",         "print base-40 grid");
	define("l|metric-levels=b",   "use metric levels in analysis");
	define("k|kern=b",            "print kern pitch grid");
	define("debug=b",             "print grid cell information");
	define("e|exinterp=s:**data", "specify exinterp for **data spine");
}



/////////////////////////////////
//
// Tool_testgrid::run -- Do the main work of the tool.
//

bool Tool_testgrid::run(HumdrumFile& infile, ostream& out) {

	NoteGrid grid(infile);

	if (getBoolean("debug")) {
		grid.printGridInfo(cerr);
		// return 1;
	} else if (getBoolean("raw")) {
		grid.printRawGrid(out);
		return 1;
	} else if (getBoolean("diatonic")) {
		grid.printDiatonicGrid(out);
		return 1;
	} else if (getBoolean("midi-pitch")) {
		grid.printMidiGrid(out);
		return 1;
	} else if (getBoolean("base-40")) {
		grid.printBase40Grid(out);
		return 1;
	} else if (getBoolean("kern")) {
		grid.printKernGrid(out);
		return 1;
	}

	vector<vector<string> > results;

	results.resize(grid.getVoiceCount());
	for (int i=0; i<(int)results.size(); i++) {
		results[i].resize(infile.getLineCount());
	}
	doAnalysis(results, grid, getBoolean("debug"));

	string exinterp = getString("exinterp");
	vector<HTp> kernspines = infile.getKernSpineStartList();
	infile.appendDataSpine(results.back(), "", exinterp);
	for (int i = (int)results.size()-1; i>0; i--) {
		int track = kernspines[i]->getTrack();
		infile.insertDataSpineBefore(track, results[i-1], "", exinterp);
	}
	out << infile;

	return 1;
}



//////////////////////////////
//
// Tool_testgrid::doAnalysis -- do a basic melodic analysis of all parts.
//

void Tool_testgrid::doAnalysis(vector<vector<string> >& results,
		NoteGrid& grid, bool debug) {
	for (int i=0; i<grid.getVoiceCount(); i++) {
		doAnalysisForVoice(results[i], grid, i, debug);
	}
}



//////////////////////////////
//
// Tool_testgrid::doAnalysisForVoice -- do analysis for a single voice by
//     subtracting NoteCells to calculate the diatonic intervals.
//

void Tool_testgrid::doAnalysisForVoice(vector<string>& results, NoteGrid& grid,
		int vindex, bool debug) {
	vector<NoteCell*> attacks;
	grid.getNoteAndRestAttacks(attacks, vindex);

	if (debug) {
		cerr << "=======================================================";
		cerr << endl;
		cerr << "Note attacks for voice number "
		     << grid.getVoiceCount()-vindex << ":" << endl;
		for (int i=0; i<(int)attacks.size(); i++) {
			attacks[i]->printNoteInfo(cerr);
		}
	}
	bool nodissonanceQ = getBoolean("no-dissonant");

	HumNum durp;     // duration of previous melodic note;
	HumNum dur;      // duration of current note;
	HumNum durn;     // duration of next melodic note;
	HumNum durnn;    // duration of next next melodic note;
	double intp;     // diatonic interval from previous melodic note
	double intn;     // diatonic interval to next melodic note
	double levp;     // metric level of the previous melodic note
	double lev;      // metric level of the current note
	double levn;     // metric level of the next melodic note
	int lineindex;   // line in original Humdrum file content that contains note
	int sliceindex;  // current timepoint in NoteGrid.
	vector<double> harmint(grid.getVoiceCount());  // harmonic intervals;
	bool dissonant;  // true if  note is dissonant with other sounding notes.

	for (int i=1; i<(int)attacks.size() - 1; i++) {
		sliceindex = attacks[i]->getSliceIndex();
		lineindex = attacks[i]->getLineIndex();

		// calculate harmonic intervals:
		for (int j=0; j<(int)harmint.size(); j++) {
			if (j == vindex) {
				harmint[j] = 0;
			}
			if (j < vindex) {
				harmint[j] = *grid.cell(vindex, sliceindex) -
						*grid.cell(j, sliceindex);
			} else {
				harmint[j] = *grid.cell(j, sliceindex) -
						*grid.cell(vindex, sliceindex);
			}
		}
		// check if current note is dissonant to another sounding note:
		dissonant = false;
		for (int j=0; j<(int)harmint.size(); j++) {
			if (j == vindex) {
				// don't compare to self
				continue;
			}
			if (harmint[j] == NAN) {
				// rest, so ignore
				continue;
			}
			int value = (int)harmint[j];
			if (value > 7) {
				value = value % 7; // remove octaves from interval
			} else if (value < -7) {
				value = -(-value % 7); // remove octaves from interval
			}

			if ((value == 1) || (value == -1)) {
				// forms a second with another sounding note
				dissonant = true;
				results[lineindex] = "d2";
				break;
			} else if ((value == 6) || (value == -6)) {
				// forms a seventh with another sounding note
				dissonant = true;
				results[lineindex] = "d7";
				break;
			}
		}
	
		// Don't label current note if not dissonant with other sounding notes.
		if (!dissonant) {
			if (!nodissonanceQ) {
				continue;
			}
		}

		durp = attacks[i-1]->getDuration();
		dur  = attacks[i]->getDuration();
		durn = attacks[i+1]->getDuration();
		intp = *attacks[i] - *attacks[i-1];
		intn = *attacks[i+1] - *attacks[i];
		levp = attacks[i-1]->getMetricLevel();
		lev  = attacks[i]->getMetricLevel();
		levn = attacks[i+1]->getMetricLevel();

		if ((dur <= durp) && (lev >= levp) && (lev >= levn)) { // weak dissonances
			if (intp == -1) { // descending dissonances
				if (intn == -1) {
					results[lineindex] = "pd"; // downward passing tone
				} else if (intn == 1) {
					results[lineindex] = "nd"; // lower neighbor
				} else if (intn == 0) {
					results[lineindex] = "ad"; // descending anticipation
				} else if (intn > 1) {
					results[lineindex] = "ed"; // lower échappée
				} else if (intn == -2) {
					results[lineindex] = "scd"; // short descending nota cambiata
				} else if (intn < -2) {
					results[lineindex] = "ipd"; // incomplete posterior lower neighbor
				}
			} else if (intp == 1) { // ascending dissonances
				if (intn == 1) {
					results[lineindex] = "pu"; // rising passing tone
				} else if (intn == -1) {
					results[lineindex] = "nu"; // upper neighbor
				} else if (intn < -1) {
					results[lineindex] = "eu"; // upper échappée
				} else if (intn == 0) {
					results[lineindex] = "au"; // rising anticipation
				} else if (intn == 2) {
					results[lineindex] = "scu"; // short ascending nota cambiata
				} else if (intn > 2) {
					results[lineindex] = "ipu"; // incomplete posterior upper neighbor
				}
			} else if ((intp < -2) && (intn == 1)) {
				results[lineindex] = "iad"; // incomplete anterior lower neighbor
			} else if ((intp > 2) && (intn == -1)) {
				results[lineindex] = "iau"; // incomplete anterior upper neighbor
			}
		}
		// TODO: add check to see if results already has a result.
		if (i < ((int)attacks.size() - 2)) { // expand the analysis window
			double interval3 = *attacks[i+2] - *attacks[i+1];
			HumNum durnn = attacks[i+2]->getDuration();	// dur of note after next
			double levnn = attacks[i+2]->getMetricLevel(); // lev of note after next

			if ((dur == durn) && (lev == 1) && (levn == 2) && (levnn == 0) &&
				(intp == -1) && (intn == -1) && (interval3 == 1)) {
				results[lineindex] = "ci"; // chanson idiom
			} else if ((durp >= 2) && (dur == 1) && (lev < levn) &&
				(intp == -1) && (intn == -1)) {
				results[lineindex] = "dq"; // dissonant third quarter
			} else if ((dur <= durp) && (lev >= levp) && (lev >= levn) &&
				(intp == -1) && (intn == -2) && (interval3 == 1)) {
				results[lineindex] = "lcd"; // long descending nota cambiata
			} else if ((dur <= durp) && (lev >= levp) && (lev >= levn) &&
				(intp == 1) && (intn == 2) && (interval3 == -1)) {
				results[lineindex] = "lcu"; // long ascending nota cambiata
			}
		}
	}
}



// END_MERGE

} // end namespace hum



