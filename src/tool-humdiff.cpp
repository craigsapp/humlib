//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Jul 29 11:38:01 CEST 2019
// Last Modified: Sat Aug  3 17:48:04 EDT 2019
// Filename:      humdiff.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/cli/humdiff.cpp
// Syntax:        C++11
// vim:           ts=3 noexpandtab nowrap
//
// Description:   Compare contents of two similar scores.
//

#include "tool-humdiff.h"
#include "HumRegex.h"
#include "Convert.h"
#include <iostream>

using namespace std;

namespace hum {

// START_MERGE


/////////////////////////////////
//
// Tool_humdiff::Tool_humdiff -- Set the recognized options for the tool.
//

Tool_humdiff::Tool_humdiff(void) {
	define("r|reference=i:1",     "sequence number of reference score");
	define("report=b",            "display report of differences");
	define("time-points|times=b", "display timepoint lists for each file");
	define("note-points|notes=b", "display notepoint lists for each file");
	define("c|color=s:red",       "color for difference markers");
}



//////////////////////////////
//
// Tool_humdiff::run --
//

bool Tool_humdiff::run(HumdrumFileSet& infiles) {
	int reference = getInteger("reference") - 1;
	if (reference < 0) {
		cerr << "Error: reference has to be 1 or higher" << endl;
		return false;
	}
	if (reference > infiles.getCount()) {
		cerr << "Error: reference number is too large: " << reference << endl;
		cerr << "Maximum is " << infiles.getCount() << endl;
		return false;
	}

	if (infiles.getSize() == 0) {
		cerr << "Usage: " << getCommand() << " files" << endl;
		return false;
	} else if (infiles.getSize() < 2) {
		cerr << "Error: requires two or more files" << endl;
		cerr << "Usage: " << getCommand() << " files" << endl;
		return false;
	} else {
		HumNum targetdur = infiles[0].getScoreDuration();
		for (int i=1; i<infiles.getSize(); i++) {
			HumNum dur = infiles[i].getScoreDuration();
			if (dur != targetdur) {
				cerr << "Error: all files must have the same duration" << endl;
				return false;
			}
		}

		for (int i=0; i<infiles.getCount(); i++) {
			if (i == reference) {
				continue;
			}
			compareFiles(infiles[reference], infiles[i]);
		}

		if (!getBoolean("report")) {
			infiles[reference].createLinesFromTokens();
			m_humdrum_text << infiles[reference];
			if (m_marked) {
				m_humdrum_text << "!!!RDF**kern: @ = marked note";
				if (getBoolean("color")) {
					m_humdrum_text << "color=\"" << getString("color") << "\"";
				}
				m_humdrum_text << endl;
			}
		}
	}

	return true;
}



//////////////////////////////
//
// Tool_humdiff::compareFiles --
//

void Tool_humdiff::compareFiles(HumdrumFile& reference, HumdrumFile& alternate) {
	vector<vector<TimePoint>> timepoints(2);
	extractTimePoints(timepoints.at(0), reference);
	extractTimePoints(timepoints.at(1), alternate);

	if (getBoolean("time-points")) {
		printTimePoints(timepoints[0]);
		printTimePoints(timepoints[1]);
	}

	compareTimePoints(timepoints, reference, alternate);
}



//////////////////////////////
//
// Tool_humdiff::printTimePoints --
//

void Tool_humdiff::printTimePoints(vector<TimePoint>& timepoints) {
	for (int i=0; i<(int)timepoints.size(); i++) {
		m_free_text << "TIMEPOINT " << i << ":" << endl;
		m_free_text << timepoints[i] << endl;
	}
}



//////////////////////////////
//
// Tool_humdiff::compareTimePoints --
//

void Tool_humdiff::compareTimePoints(vector<vector<TimePoint>>& timepoints,
		HumdrumFile& reference, HumdrumFile& alternate) {
	vector<int> indexes(timepoints.size(), 0);
	HumNum minval;
	HumNum value;
	int found;

	vector<HumdrumFile*> infiles(2, NULL);
	infiles[0] = &reference;
	infiles[1] = &alternate;

	vector<int> increment(timepoints.size(), 0);

	while ((1)) {
		if (indexes.at(0) >= (int)timepoints.at(0).size()) {
			// at the end of the list of notes for the first file.
			// break from the comparison for now and figure out how
			// to report differences of added notes in the other file(s)
			// later.
			break;
		}
		timepoints.at(0).at(indexes.at(0)).index.resize(timepoints.size());
		for (int i=1; i<(int)timepoints.size(); i++) {
			timepoints.at(0).at(indexes.at(0)).index.at(i) = -1;
		}
		minval = timepoints.at(0).at(indexes.at(0)).timestamp;
		for (int i=1; i<(int)timepoints.size(); i++) {
			if (indexes.at(i) >= (int)timepoints.at(i).size()) {
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
			if (indexes.at(i) >= (int)timepoints.at(i).size()) {
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
			compareLines(minval, indexes, timepoints, infiles);
		}
		for (int i=0; i<(int)increment.size(); i++) {
			indexes.at(i) += increment.at(i);
		}
	}
}



//////////////////////////////
//
// Tool_humdiff::printNotePoints --
//

void Tool_humdiff::printNotePoints(vector<NotePoint>& notelist) {
	m_free_text << "vvvvvvvvvvvvvvvvvvvvvvvvv" << endl;
	for (int i=0; i<(int)notelist.size(); i++) {
		m_free_text << "NOTE " << i << endl;
		m_free_text << notelist.at(i) << endl;
	}
	m_free_text << "^^^^^^^^^^^^^^^^^^^^^^^^^" << endl;
	m_free_text << endl;
}



//////////////////////////////
//
// Tool_humdiff::markNote -- mark the note (since it does not have a match in other edition(s).
//

void Tool_humdiff::markNote(NotePoint& np) {
	m_marked = 1;
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
// Tool_humdiff::compareLines --
//

void Tool_humdiff::compareLines(HumNum minval, vector<int>& indexes,
		vector<vector<TimePoint>>& timepoints, vector<HumdrumFile*> infiles) {

	bool reportQ = getBoolean("report");

	// cerr << "COMPARING LINES ====================================" << endl;
	vector<vector<NotePoint>> notelist(indexes.size());

	// Note: timepoints size must be 2
	// and infiles size must be 2
	for (int i=0; i<(int)timepoints.size(); i++) {
		if (indexes.at(i) >= (int)timepoints.at(i).size()) {
			continue;
		}
		if (timepoints.at(i).at(indexes.at(i)).timestamp != minval) {
			// not at the same time
			continue;
		}

		getNoteList(notelist.at(i), *infiles[i],
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

	if (getBoolean("notes")) {
		for (int i=0; i<(int)notelist.size(); i++) {
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
				cout << "\tREFERENCE LINE TEXT\t: " << (*infiles[0])[humindex] << endl;

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
// Tool_humdiff::findNoteInList --
//

int Tool_humdiff::findNoteInList(NotePoint& np, vector<NotePoint>& nps) {
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
// Tool_humdiff::getNoteList --
//

void Tool_humdiff::getNoteList(vector<NotePoint>& notelist, HumdrumFile& infile, int line, int measure, int sourceindex, int tpindex) {
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
// Tool_humdiff::extractTimePoints -- Extract a list of the timestamps in a file.
//

void Tool_humdiff::extractTimePoints(vector<TimePoint>& points, HumdrumFile& infile) {
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



//////////////////////////////
//
// operator<< == print a TimePoint
//

ostream& operator<<(ostream& out, TimePoint& tp) {
	out << "\ttimestamp:\t" << tp.timestamp.getFloat() << endl;
	out << "\tmeasure:\t" << tp.measure << endl;
	out << "\tindexes:\t" << endl;
	for (int i=0; i<(int)tp.index.size(); i++) {
		out << "\t\tindex " << i << " is:\t" << tp.index[i] << "\t" << (*tp.file[i])[tp.index[i]] << endl;
	}
	return out;
}



//////////////////////////////
//
// operator<< == print a NotePoint
//

ostream& operator<<(ostream& out, NotePoint& np) {
	if (np.token) {
		out << "\ttoken:\t\t" << np.token << endl;
	}
	out << "\ttoken index:\t" << np.subindex << endl;
	if (!np.subtoken.empty()) {
		out << "\tsubtoken:\t" << np.subtoken << endl;
	}
	out << "\tmeasure:\t" << np.measure << endl;
	out << "\tmquarter:\t" << np.measurequarter << endl;
	out << "\ttrack:\t\t" << np.track << endl;
	out << "\tlayer:\t\t" << np.layer << endl;
	out << "\tduration:\t" << np.duration << endl;
	out << "\tb40:\t\t" << np.b40 << endl;
	out << "\tprocessed:\t" << np.processed << endl;
	out << "\tsourceindex:\t" << np.sourceindex << endl;
	out << "\ttpindex:\t" << np.tpindex << endl;
	out << "\tmatched:\t" << endl;
	for (int i=0; i<(int)np.matched.size(); i++) {
		out << "\t\tindex " << i << " is:\t" << np.matched[i] << endl;
	}
	return out;
}



// END_MERGE

} // end namespace hum


