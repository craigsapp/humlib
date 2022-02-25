//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Programmer:    Kiana Hu
// Creation Date: Fri Feb 18 22:00:48 PST 2022
// Last Modified: Fri Feb 18 22:00:51 PST 2022
// Filename:      peak.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/cli/peak.cpp
// Syntax:        C++11
// vim:           ts=3 noexpandtab nowrap
//
// Description:   Analyze high-points in melodies
//

#include "humlib.h"
#include <iostream>

using namespace hum;
using namespace std;

Options options;  // used to process command-line options.

// Function declarations:
void                processFile        (HumdrumFile& infile, Options& options);
void                processSpine       (HTp startok);
void                identifyLocalPeaks (vector<bool>& peaknotes, vector<int>& notelist);
vector<int>         getMidiNumbers     (vector<vector<HTp>>& notelist);
vector<vector<HTp>> getNoteList        (HTp starting);
void                printData          (vector<vector<HTp>>& notelist,
                                        vector<int>& midinums,
                                        vector<bool>& peaknotes);


///////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv) {
	options.define("d|data=b",       "print input/output data");    // used later
	options.define("m|mark=s:@",     "symbol to mark peak notes");  // used later
	options.define("c|color=s:red",  "color of marked notes");      // used later
	options.process(argc, argv);
	HumdrumFileStream instream(options);
	HumdrumFile infile;
	while (instream.read(infile)) {
		processFile(infile, options);
	}
	return 0;
}



//////////////////////////////
//
// processFile --
//

void processFile(HumdrumFile& infile, Options& options) {
	// get list of music spines (columns):
	vector<HTp> starts = infile.getKernSpineStartList();

	// The first "spine" is the lowest part on the system.
	// The last "spine" is the highest part on the system.
	for (int i=0; i<(int)starts.size(); i++) {
		processSpine(starts[i]);
	}
}



//////////////////////////////
//
// processSpine -- Process one line of music.
//

void processSpine(HTp startok) {
	// notelist is a two dimensional array of notes.   The
	// first dimension is a list of the note attacks in time
	// (plus rests), and the second dimension is for a list of the
	// tied notes after the first one (this is so that we can
	// highlight both the starting note and any tied notes to that
	// starting note later).
	vector<vector<HTp>> notelist = getNoteList(startok);

	// MIDI note numbers for each note (with rests being 0).
	vector<int> midinums = getMidiNumbers(notelist);

	// True = the note is a local high pitch.
	vector<bool> peaknotes(midinums.size(), false);

	identifyLocalPeaks(peaknotes, midinums);

	printData(notelist, midinums, peaknotes);
}



//////////////////////////////
//
// identifyLocalPeaks -- Identify notes that are higher than their
//    adjacent neighbors.  The midinumbs are MIDI note numbers (integers)
//    for the pitch, with higher number meaning higher pitches.  Rests are
//    the value 0.  Do not assign a note as a peak note if one of the
//    adjacent notes is a rest. (This could be refined later, such as ignoring
//    short rests).
//

void identifyLocalPeaks(vector<bool>& peaknotes, vector<int>& midinums) { //changed to midinums from 'notelist'
  for (int i=1; i<(int)midinums.size() - 1; i++) {
    if ((midinums[i - 1] <= 0) || (midinums[i + 1] <= 0)) { //not next to a rest
      continue;
    } else if (midinums[i] <= 0) {
      continue;
    }
    if ((midinums[i] > midinums[i - 1]) && (midinums[i] > midinums[i + 1])) { //check neighboring notes
      peaknotes[i] = 1;
    }
  }
}



//////////////////////////////
//
// printData -- Print input and output data.  First column is the MIDI note
//      number, second one is the peak analysis (true=local maximum note)
//

void printData(vector<vector<HTp>>& notelist, vector<int>& midinums, vector<bool>& peaknotes) {
	cout << "MIDI\tPEAK\tKERN" << endl;
	for (int i=0; i<(int)notelist.size(); i++) {
		cout << midinums.at(i) << "\t";
		cout << peaknotes.at(i);
		for (int j=0; j<(int)notelist[i].size(); j++) {
			cout << "\t" << notelist[i][j];
		}
		cout << endl;
	}
	cout << "******************************************" << endl;
	cout << endl;
}



//////////////////////////////
//
// getMidiNumbers -- convert note tokens into MIDI note numbers.
//    60 = middle C (C4), 62 = D4, 72 = C5, 48 = C3.
//

vector<int> getMidiNumbers(vector<vector<HTp>>& notelist) {
	vector<int> output(notelist.size(), 0);  // fill with rests by default
	for (int i=0; i<(int)notelist.size(); i++) {
		output[i] = Convert::kernToMidiNoteNumber(notelist.at(i).at(0));
		if (output[i] < 0) {
			// Set rests to be 0
			output[i] = 0;
		}
	}
	return output;
}



//////////////////////////////
//
// getNoteList --
//

vector<vector<HTp>> getNoteList(HTp starting) {
	vector<vector<HTp>> output;
	output.reserve(2000);

	HTp current = starting;
	while (current) {
		if (!current->isData()) {
			current = current->getNextToken();
			continue;
		}
		if (current->isNull()) {
			current = current->getNextToken();
			continue;
		}
		if (current->isNoteSustain()) {
			if (output.size() > 0) {
				output.back().push_back(current);
			}
			current = current->getNextToken();
			continue;
		}
		output.resize(output.size() + 1);
		output.back().push_back(current);
		current = current->getNextToken();
	}
	return output;
}
