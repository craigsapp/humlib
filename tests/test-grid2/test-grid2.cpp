// Description: Example of extracting a 2D pitch grid from
// a score for dissonance analysis.  This version creates a
// class for storing grid data.
// vim: ts=3

#include "humlib.h"
#include "GridCell.h"
#include "NoteGrid.h"

// remove the following lines when GridCell and NoteGrid are in humlib library.
#include "GridCell.cpp"
#include "NoteGrid.cpp"

using namespace std;
using namespace hum;

// function declarations:
void    doAnalysis  (vector<vector<string> >& results, NoteGrid& grid);
void    doAnalysis  (vector<string>& results, NoteGrid& grid, int vindex);


int main(int argc, char** argv) {

	// handle command-line options:
	Options opts;
	opts.define("r|raw=b",        "print raw grid");
	opts.define("d|diatonic=b",   "print diatonic grid");
	opts.define("m|midi-pitch=b", "print midie-pitch grid");
	opts.define("b|base-40=b",    "print base-40 grid");
	opts.define("k|kern=b",       "print kern pitch grid");
	opts.define("debug=b",        "print grid cell information");
	opts.process(argc, argv);

	// read an inputfile from the first filename argument, or standard input
	HumdrumFile infile;
	if (opts.getArgCount() > 0) {
		infile.read(opts.getArgument(1));
	} else {
		infile.read(cin);
	}

	NoteGrid grid(infile);

	if (opts.getBoolean("debug")) {
		grid.printCellInfo(cout);
		return 0;
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
	doAnalysis(results, grid);

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

void doAnalysis(vector<vector<string> >& results, NoteGrid& grid) {
	for (int i=0; i<grid.getVoiceCount(); i++) {
		doAnalysis(results[i], grid, i);
	}
}



//////////////////////////////
//
// doAnalysis -- do analysis for a single voice.
//

void doAnalysis(vector<string>& results, NoteGrid& grid, int vindex) {
	int previous, next, current;
	int interval1, interval2;

	for (int i=1; i<(int)grid.getSliceCount() - 1; i++) {
		current = grid.getDiatonicPitch(vindex, i);
		if (current <= 0) {
			continue;
		}
		previous = grid.getPrevAttackDiatonic(vindex, i);
		if (previous <= 0) {
			continue;
		}
		next = grid.getNextAttackDiatonic(vindex, i);
		if (next <= 0) {
			continue;
		}
		interval1 = current - previous;
		interval2 = next - current;
		int lineindex = grid.getLineIndex(i);
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

