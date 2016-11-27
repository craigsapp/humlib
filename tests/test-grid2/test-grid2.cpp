// Description: Example of extracting a 2D pitch grid from
// a score for dissonance analysis.  This version creates a
// class for storing grid data.
// vim: ts=3

#include "humlib.h"
#include "NoteCell.h"
#include "NoteGrid.h"

// remove the following lines when NoteCell and NoteGrid are in humlib library.
#include "NoteCell.cpp"
#include "NoteGrid.cpp"

using namespace std;
using namespace hum;

// function declarations:
void    doAnalysis  (vector<vector<string> >& results, NoteGrid& grid,
                     bool debug);
void    doAnalysisA (vector<string>& results, NoteGrid& grid, int vindex,
                     bool debug);
void    doAnalysisB (vector<string>& results, NoteGrid& grid, int vindex,
                     bool debug);


char Algorithm = 'A';

int main(int argc, char** argv) {

	// handle command-line options:
	Options opts;
	opts.define("r|raw=b",        "print raw grid");
	opts.define("d|diatonic=b",   "print diatonic grid");
	opts.define("m|midi-pitch=b", "print midie-pitch grid");
	opts.define("b|base-40=b",    "print base-40 grid");
	opts.define("k|kern=b",       "print kern pitch grid");
	opts.define("debug=b",        "print grid cell information");
	opts.define("B=b",  		      "use second algorithm");
	opts.process(argc, argv);
	if (opts.getBoolean("B")) {
		Algorithm = 'B';
	}

	// read an inputfile from the first filename argument, or standard input
	HumdrumFile infile;
	if (opts.getArgCount() > 0) {
		infile.read(opts.getArgument(1));
	} else {
		infile.read(cin);
	}

	NoteGrid grid(infile);

	if (opts.getBoolean("debug")) {
		grid.printGridInfo(cerr);
		// return 0;
	} else if (opts.getBoolean("raw")) {
		grid.printRawGrid(cout);
		return 0;
	} else if (opts.getBoolean("diatonic")) {
		grid.printDiatonicGrid(cout);
		return 0;
	} else if (opts.getBoolean("midi-pitch")) {
		grid.printMidiGrid(cout);
		return 0;
	} else if (opts.getBoolean("base-40")) {
		grid.printBase40Grid(cout);
		return 0;
	} else if (opts.getBoolean("kern")) {
		grid.printKernGrid(cout);
		return 0;
	}

	vector<vector<string> > results;

	results.resize(grid.getVoiceCount());
	for (int i=0; i<(int)results.size(); i++) {
		results[i].resize(infile.getLineCount());
	}
	doAnalysis(results, grid, opts.getBoolean("debug"));

	vector<HTp> kernspines = infile.getKernSpineStartList();
	infile.appendDataSpine(results.back());
	for (int i = (int)results.size()-1; i>0; i--) {
		int track = kernspines[i]->getTrack();
		infile.insertDataSpineBefore(track, results[i-1]);
	}
	cout << infile;

	return 0;
}



//////////////////////////////
//
// doAnalysis -- do a basic melodic analysis of all parts.
//

void doAnalysis(vector<vector<string> >& results, NoteGrid& grid, bool debug) {
	for (int i=0; i<grid.getVoiceCount(); i++) {
		if (Algorithm == 'A') {
			doAnalysisA(results[i], grid, i, debug);
		} else {
			doAnalysisB(results[i], grid, i, debug);
		}
	}
}



//////////////////////////////
//
// doAnalysisA -- do analysis for a single voice by subtracting
//     NoteCells to calculate the interval.
//

void doAnalysisA(vector<string>& results, NoteGrid& grid, int vindex,
		bool debug) {
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

	double interval1, interval2;
	for (int i=1; i<(int)attacks.size() - 1; i++) {
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
// doAnalysisB -- do analysis for a single voice by asking the
//     Note for the interval values instead of calculating them
//     directly.
//

void doAnalysisB(vector<string>& results, NoteGrid& grid, int vindex,
		bool debug) {
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




/*

//////////////////////////////
//
// getAttack --
//

int getAttack(int n, vector<int>& data, int index, vector<int> attacks) {
	int ordinal = 0;
	index = attacks[index];
	while ((index >= 0) && (ordinal < n)) {
		if (index < 0) {
			return 0;
		}
		ordinal++;
		if (ordinal == n) {
			return data[index];
		}
		index = attacks[index];
	}
	return 0;
}



//////////////////////////////
//
// getAttacks --
//

vector<int> getAttacks(int n, vector<int>& data, int index,
		vector<int> attacks) {
	vector<int> output(n, 0);
	int ordinal = 0;
	index = attacks[index];
	while ((index >= 0) && (ordinal < n)) {
		if (index < 0) {
			break;
		}
		ordinal++;
		if (ordinal <= n) {
			output[ordinal-1] = data[index];
		}
		if (ordinal == n) {
			break;
		}
		index = attacks[index];
	}
	return output;
}

*/

