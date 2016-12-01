//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Programmer:    Alex Morgan
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
	define("r|raw=b",           "print raw grid");
	define("d|diatonic=b",      "print diatonic grid");
	define("m|midi-pitch=b",    "print midie-pitch grid");
	define("b|base-40=b",       "print base-40 grid");
	define("l|metric-levels=b", "use metric levels in analysis");
	define("k|kern=b",          "print kern pitch grid");
	define("debug=b",           "print grid cell information");
	define("B=b",  		       "use second algorithm");
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

	vector<HTp> kernspines = infile.getKernSpineStartList();
	infile.appendDataSpine(results.back());
	for (int i = (int)results.size()-1; i>0; i--) {
		int track = kernspines[i]->getTrack();
		infile.insertDataSpineBefore(track, results[i-1]);
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
		for (int i=0; i<attacks.size(); i++) {
			attacks[i]->printNoteInfo(cerr);
		}
	}

	bool levelsQ = getBoolean("metric-levels");

	double interval1, interval2;
	for (int i=1; i<(int)attacks.size() - 1; i++) {
		if (levelsQ) {
			double lev1 = attacks[i-1]->getMetricLevel();
			double lev2 = attacks[i]->getMetricLevel();
			// double lev3 = attacks[i+1]->getMetricLevel();
			if (lev2 < lev1) {
				// if the starting note is at a higher metric position
				// than the second note, then don't mark.
				continue;
			}
		}
		interval1 = *attacks[i] - *attacks[i-1];
		interval2 = *attacks[i+1] - *attacks[i];
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
		for (int i=0; i<attacks.size(); i++) {
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



