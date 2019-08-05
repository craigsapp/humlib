//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Jul 29 11:38:01 CEST 2019
// Last Modified: Wed Jul 31 21:20:06 CEST 2019
// Filename:      humdiff.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/cli/humdiff.cpp
// Syntax:        C++11
// vim:           ts=3 noexpandtab nowrap
//
// Description:   Compare contents of two similar scores.
//

#include "humlib.h"
#include <iostream>

using namespace hum;
using namespace std;


// Function declarations:

void     compareFiles       (HumdrumFileSet& humset);
ostream& compareTimePoints  (ostream& out, vector<vector<TimePoint>>& timepoints, HumdrumFileSet& humset);
void     extractTimePoints  (vector<TimePoint>& points, HumdrumFile& infile);
ostream& printTimePoints    (ostream& out, vector<TimePoint>& timepoints);
void     compareLines       (HumNum minval, vector<int>& indexes, vector<vector<TimePoint>>& timepoints, HumdrumFileSet& humset);
void     getNoteList        (vector<NotePoint>& notelist, HumdrumFile& infile, int line, int measure, int sourceindex, int tpindex);
int      findNoteInList     (NotePoint& np, vector<NotePoint>& nps);
void     printNotePoints    (vector<NotePoint>& notelist);
void     markNote           (NotePoint& np);


///////////////////////////////////////////////////////////////////////////

Options options;
int Marked = 0;

int main(int argc, char** argv) {
	options.define("r|reference=i:0",     "sequence number of reference score");
	options.define("report=b",            "display report of differences");
	options.define("time-points|times=b", "display timepoint lists for each file");
	options.define("note-points|notes=b", "display notepoint lists for each file");
	options.define("c|color=s:red",       "color for markers");
	options.process(argc, argv);
	HumdrumFileSet humset(options);
	int reference = options.getInteger("reference");
	if (reference > 1) {
		if (reference > humset.getCount()) {
			cerr << "Error: work number is too large: " << reference << endl;
			cerr << "Maximum is " << humset.getCount() << endl;
			return 1;
		}
		reference--;
		humset.swap(0, reference);
	}

	if (humset.getSize() == 0) {
		cerr << "Usage: " << options.getCommand() << " files" << endl;
		return 1;
	} else if (humset.getSize() < 2) {
		cerr << "Error: requires two or more files" << endl;
		cerr << "Usage: " << options.getCommand() << " files" << endl;
		return 1;
	} else {
		HumNum targetdur = humset[0].getScoreDuration();
		for (int i=1; i<humset.getSize(); i++) {
			HumNum dur = humset[i].getScoreDuration();
			if (dur != targetdur) {
				cerr << "Error: all files must have the same duration" << endl;
				return 1;
			}
		}
		compareFiles(humset);
	}

	if (!options.getBoolean("report")) {
		humset[0].createLinesFromTokens();
		cout << humset[0];
		if (Marked) {
			cout << "!!!RDF**kern: @ = marked note";
			if (options.getBoolean("color")) {
				cout << "color=\"" << options.getString("color") << "\"";
			}
			cout << endl;
		}
	}

	return 0;
}



//////////////////////////////
//
// compareFiles --
//

void compareFiles(HumdrumFileSet& humset) {
	vector<vector<TimePoint>> timepoints(humset.getSize());;
	for (int i=0; i<humset.getSize(); i++) {
		extractTimePoints(timepoints.at(i), humset[i]);
	}

	if (options.getBoolean("time-points")) {
		for (int i=0; i<(int)timepoints.size(); i++) {
	 		printTimePoints(cout, timepoints[i]);
		}
	}

	compareTimePoints(cout, timepoints, humset);
}



//////////////////////////////
//
// printTimePoints --
//

ostream& printTimePoints(ostream& out, vector<TimePoint>& timepoints) {
	for (int i=0; i<(int)timepoints.size(); i++) {
		out << "TIMEPOINT " << i << ":" << endl;
		out << timepoints[i] << endl;
	}
	return out;
}



//////////////////////////////
//
// compareTimePoints --
//

ostream& compareTimePoints(ostream& out, vector<vector<TimePoint>>& timepoints, HumdrumFileSet& humset) {
	vector<int> indexes(timepoints.size(), 0);
	HumNum minval;
	HumNum value;
	int found;

	vector<int> increment(timepoints.size(), 0);

	while ((1)) {
		if (indexes.at(0) >= timepoints.at(0).size()) {
			// at the end of the list of notes for the first file.
			// break from the comparison for now and figure out how
			// to report differences of added notes in the other file(s)
			// later.
			break;
		}
		timepoints.at(0).at(indexes.at(0)).index.resize(timepoints.size());
		for (int i=1; i<timepoints.size(); i++) {
			timepoints.at(0).at(indexes.at(0)).index.at(i) = -1;
		}
		minval = timepoints.at(0).at(indexes.at(0)).timestamp;
		for (int i=1; i<(int)timepoints.size(); i++) {
			if (indexes.at(i) >= timepoints.at(i).size()) {
				continue;
			}
			value = timepoints.at(i).at(indexes.at(i)).timestamp;
			if (value < minval) {
				minval = value;
			}
		}
		found = 0;
		fill(increment.begin(), increment.end(), 0);
		
		for (int i=0; i<(int)timepoints.size(); i++) {
			if (indexes.at(i) >= timepoints.at(i).size()) {
				// index is too large for file, so skip checking it.
				continue;
			}
			found = 1;
			value = timepoints.at(i).at(indexes.at(i)).timestamp;

			if (value == minval) {
				timepoints.at(0).at(indexes.at(0)).index.at(i) = timepoints.at(i).at(indexes.at(i)).index.at(0);
				increment.at(i)++;
			}
		}
		if (!found) {
			break;
		} else {
			compareLines(minval, indexes, timepoints, humset);
		}
		for (int i=0; i<(int)increment.size(); i++) {
			indexes.at(i) += increment.at(i);
		}
	}
	return out;
}



//////////////////////////////
//
// printNotePoints --
//

void printNotePoints(vector<NotePoint>& notelist) {
	cout << "vvvvvvvvvvvvvvvvvvvvvvvvv" << endl;
	for (int i=0; i<(int)notelist.size(); i++) {
		cout << "NOTE " << i << endl;
		cout << notelist.at(i) << endl;
	}
	cout << "^^^^^^^^^^^^^^^^^^^^^^^^^" << endl;
	cout << endl;
}



//////////////////////////////
//
// markNote -- mark the note (since it does not have a match in other edition(s).
//

void markNote(NotePoint& np) {
	Marked = 1;
	HTp token = np.token;
	if (!token) {
		return;
	}
	if (!token->isChord()) {
		string contents = *token;
		contents += "@";
		token->setText(contents);
		return;
	}
	vector<string> tokens = token->getSubtokens();
	tokens[np.subindex] += "@";
	string output = tokens[0];
	for (int i=1; i<(int)tokens.size(); i++) {
		output += " ";
		output += tokens[i];
	}
	token->setText(output);
}



//////////////////////////////
//
// compareLines --
//

void compareLines(HumNum minval, vector<int>& indexes,
		vector<vector<TimePoint>>& timepoints, HumdrumFileSet& humset) {

	bool reportQ = options.getBoolean("report");

	// cerr << "COMPARING LINES ====================================" << endl;
	vector<vector<NotePoint>> notelist(indexes.size());
	for (int i=0; i<(int)timepoints.size(); i++) {
		if (indexes.at(i) >= (int)timepoints.at(i).size()) {
			continue;
		}
		if (timepoints.at(i).at(indexes.at(i)).timestamp != minval) {
			// not at the same time
			continue;
		}

		getNoteList(notelist.at(i), humset[i], 
			timepoints.at(i).at(indexes.at(i)).index[0], 
			timepoints.at(i).at(indexes.at(i)).measure, i, indexes.at(i));


	}
	for (int i=0; i<(int)notelist.at(0).size(); i++) {
		notelist.at(0).at(i).matched.resize(notelist.size());
		fill(notelist.at(0).at(i).matched.begin(), notelist.at(0).at(i).matched.end(), -1);
		notelist.at(0).at(i).matched.at(0) = i;
		for (int j=1; j<(int)notelist.size(); j++) {
			int status = findNoteInList(notelist.at(0).at(i), notelist.at(j));
			notelist.at(0).at(i).matched.at(j) = status;
			if ((status < 0) && !reportQ) {
				markNote(notelist.at(0).at(i));
			}
		}
	}

	if (options.getBoolean("notes")) {
		for (int i=0; i<notelist.size(); i++) {
			cerr << "========== NOTES FOR I=" << i << endl;
			printNotePoints(notelist.at(i));
			cerr << endl;
		}
	}

	if (!reportQ) {
		return;
	}

	// report
	for (int i=0; i<(int)notelist.at(0).size(); i++) {
		for (int j=1; j<(int)notelist.at(0).at(i).matched.size(); j++) {
			if (notelist.at(0).at(i).matched.at(j) < 0) {
				cout << "NOTE " << notelist.at(0).at(i).subtoken 
				     << " DOES NOT HAVE EXACT MATCH IN SOURCE " << j << endl;
				int humindex = notelist.at(0).at(i).token->getLineIndex();
				cout << "\tREFERENCE MEASURE\t: " << notelist.at(0).at(i).measure << endl;
				cout << "\tREFERENCE LINE NO.\t: " << humindex+1 << endl;
				cout << "\tREFERENCE LINE TEXT\t: " << humset[0][humindex] << endl;

				cout << "\tTARGET  " << j << " LINE NO. ";
				if (j < 10) {
					cout << " ";
				}
				cout << ":\t" << "X" << endl;

				cout << "\tTARGET  " << j << " LINE TEXT";
				if (j < 10) {
					cout << " ";
				}
				cout << ":\t" << "X" << endl;

				cout << endl;
			}
		}
	}

}



//////////////////////////////
//
// findNoteInList --
//

int findNoteInList(NotePoint& np, vector<NotePoint>& nps) {
	for (int i=0; i<(int)nps.size(); i++) {
		// cerr << "COMPARING " << np.token << " (" << np.b40 << ") TO " << nps.at(i).token << " (" << nps.at(i).b40 << ") " << endl;
		if (nps.at(i).processed) {
			continue;
		}
		if (nps.at(i).b40 != np.b40) {
			continue;
		}
		if (nps.at(i).duration != np.duration) {
			continue;
		}
		return i;
	}
	// cerr << "\tCannot find note " << np.token << " on line " << np.token->getLineIndex() << " in other work" << endl;
	return -1;
}




//////////////////////////////
//
// getNoteList --
//

void getNoteList(vector<NotePoint>& notelist, HumdrumFile& infile, int line, int measure, int sourceindex, int tpindex) {
	for (int i=0; i<infile[line].getFieldCount(); i++) {
		HTp token = infile.token(line, i);
		if (!token->isKern()) {
			continue;
		}
		if (token->isNull()) {
			continue;
		}
		if (token->isRest()) {
			continue;
		}
		int scount = token->getSubtokenCount();
		int track = token->getTrack();
		int layer = token->getSubtrack();
		for (int j=0; j<scount; j++) {
			string subtok = token->getSubtoken(j);
			if (subtok.find("]") != string::npos) {
				continue;
			}
			if (subtok.find("_") != string::npos) {
				continue;
			}
			// found a note to store;
			notelist.resize(notelist.size() + 1);
			notelist.back().token = token;
			notelist.back().subtoken = subtok;
			notelist.back().subindex = j;
			notelist.back().measurequarter = token->getDurationFromBarline();
			notelist.back().measure = 
			notelist.back().track = track;
			notelist.back().layer = layer;
			notelist.back().sourceindex = sourceindex;
			notelist.back().tpindex = tpindex;
			notelist.back().duration = token->getTiedDuration();
			notelist.back().b40 = Convert::kernToBase40(subtok);
		}
	}
}



//////////////////////////////
//
// extractTimePoints -- Extract a list of the timestamps in a file.
//

void extractTimePoints(vector<TimePoint>& points, HumdrumFile& infile) {
	TimePoint tp;
	points.clear();
	HumRegex hre;
	points.reserve(infile.getLineCount());
	int measure = -1;
	for (int i=0; i<infile.getLineCount(); i++) {
		if (infile[i].isBarline()) {
			if (hre.search(infile.token(i, 0), "(\\d+)")) {
				measure = hre.getMatchInt(1);
			}
		}
		if (!infile[i].isData()) {
			continue;
		}
		if (infile[i].getDuration() == 0) {
			// ignore grace notes for now
			continue;
		}
		tp.clear();
		tp.file.push_back(&infile);
		tp.index.push_back(i);
		tp.timestamp = infile[i].getDurationFromStart();
		tp.measure = measure;
		points.push_back(tp);
	}
}



