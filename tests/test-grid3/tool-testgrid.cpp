//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Programmer:    Alexander Morgan
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
	define("m|midi-pitch=b",      "print midie-pitch grid");
	define("b|base-40=b",         "print base-40 grid");
	define("l|metric-levels=b",   "use metric levels in analysis");
	define("k|kern=b",            "print kern pitch grid");
	define("debug=b",             "print grid cell information");
	define("e|exinterp=s:**data", "specify exinterp for **data spine");
	define("B=b",  		         "use second algorithm");
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
		if (getBoolean("B")) {
			doAnalysisB(results[i], grid, i, debug);
		} else {
			doAnalysisA(results[i], grid, i, debug);
		}
	}
}



//////////////////////////////
//
// Tool_testgrid::doAnalysisA -- do analysis for a single voice by subtracting
//     NoteCells to calculate the interval.
//

void Tool_testgrid::doAnalysisA(vector<string>& results, NoteGrid& grid,
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

	double interval1, interval2;
	for (int i=1; i<(int)attacks.size() - 1; i++) {
		HumNum durp = attacks[i-1]->getDuration();
		HumNum dur  = attacks[i]->getDuration();
		HumNum durn = attacks[i+1]->getDuration();
		HumNum duran = attacks[i+1]->getDuration();
		interval1 = *attacks[i] - *attacks[i-1];
		interval2 = *attacks[i+1] - *attacks[i];
		double levp = attacks[i-1]->getMetricLevel();
		double lev = attacks[i]->getMetricLevel();
		double levn = attacks[i+1]->getMetricLevel();

		int lineindex = attacks[i]->getLineIndex();

		if ((dur <= durp) && (lev >= levp) && (lev >= levn)) { // weak dissonances
			if (interval1 == -1) { // descending dissonances
				if (interval2 == -1) {
					results[lineindex] = "pd"; // downward passing tone
				} else if (interval2 == 1) {
					results[lineindex] = "nd"; // lower neighbor
				} else if (interval2 == 0) {
					results[lineindex] = "ad"; // descending anticipation
				} else if (interval2 > 1) {
					results[lineindex] = "ed"; // lower échappée
				} else if (interval2 == -2) {
					results[lineindex] = "scd"; // short descending nota cambiata
				} else if (interval2 < -2) {
					results[lineindex] = "ipd"; // incomplete posterior lower neighbor
				}
			} else if (interval1 == 1) { // ascending dissonances
				if (interval2 == 1) {
					results[lineindex] = "pu"; // rising passing tone
				} else if (interval2 == -1) {
					results[lineindex] = "nu"; // upper neighbor
				} else if (interval2 < -1) {
					results[lineindex] = "eu"; // upper échappée
				} else if (interval2 == 0) {
					results[lineindex] = "au"; // rising anticipation
				} else if (interval2 == 2) {
					results[lineindex] = "scu"; // short ascending nota cambiata
				} else if (interval2 > 2) {
					results[lineindex] = "ipu"; // incomplete posterior upper neighbor
				}
			} else if ((interval1 < -2) && (interval2 == 1)) {
				results[lineindex] = "iad"; // incomplete anterior lower neighbor
			} else if ((interval1 > 2) && (interval2 == -1)) {
				results[lineindex] = "iau"; // incomplete anterior upper neighbor
			}
		}
		// TODO: add check to see if results already has a result.
		if (i < ((int)attacks.size() - 2)) { // expand the analysis window
			double interval3 = *attacks[i+2] - *attacks[i+1];
			HumNum duran = attacks[i+2]->getDuration();	// dur of note after next
			double levan = attacks[i+2]->getMetricLevel(); // lev of note after next

			if ((dur == durn) && (lev == 1) && (levn == 2) && (levan == 0) &&
				(interval1 == -1) && (interval2 == -1) && (interval3 == 1)) {
				results[lineindex] = "ci"; // chanson idiom
			} else if ((durp >= 2) && (dur == 1) && (lev < levn) &&
				(interval1 == -1) && (interval2 == -1)) {
				results[lineindex] = "dq"; // dissonant third quarter
			} else if ((dur <= durp) && (lev >= levp) && (lev >= levn) &&
				(interval1 == -1) && (interval2 == -2) && (interval3 == 1)) {
				results[lineindex] = "lcd"; // long descending nota cambiata
			} else if ((dur <= durp) && (lev >= levp) && (lev >= levn) &&
				(interval1 == 1) && (interval2 == 2) && (interval3 == -1)) {
				results[lineindex] = "lcu"; // long ascending nota cambiata
			}
		}
	}
}



//////////////////////////////
//
// Tool_testgrid::doAnalysisB -- do analysis for a single voice by asking the
//     Note for the interval values instead of calculating them
//     directly.
//

void Tool_testgrid::doAnalysisB(vector<string>& results, NoteGrid& grid,
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

	int interval1, interval2;
	for (int i=1; i<(int)attacks.size() - 1; i++) {
		interval1 = attacks[i]->getDiatonicIntervalFromPreviousAttack();
		interval2 = attacks[i]->getDiatonicIntervalToNextAttack();

		int lineindex = attacks[i]->getLineIndex();

		if ((interval1 == 1) && (interval2 == 1)) {
			results[lineindex] = "pu";
		} else if ((interval1 == -1) && (interval2 == -1)) {
			results[lineindex] = "pd";
		} else if ((interval1 == 1) && (interval2 == -1)) {
			results[lineindex] = "nu";
		} else if ((interval1 == -1) && (interval2 == 1)) {
			results[lineindex] = "nd";
		}
		
	}
}


// END_MERGE

} // end namespace hum



