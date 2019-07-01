//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Jul  1 11:49:28 CEST 2019
// Last Modified: Mon Jul  1 13:24:07 CEST 2019
// Filename:      accent-features.cpp
// Syntax:        C++11
// vim:           ts=3 noexpandtab nowrap
//
// Description:   Extracts potential accentual features from Humdrum scores.
//

#include "humlib.h"
#include <iostream>

using namespace hum;
using namespace std;

class AccentFeatures {
	public:
		HTp  token      = NULL;
		int  subtoken   = 0;
		string text;
		HTp  nextNote   = NULL;
		HTp  prevNote   = NULL;

		int chord_num   = 0;

		bool accent     = false;
		bool slur_start = false;
		bool slur_end   = false;
		bool trill      = false;

		double metpos   = 0;
};

void   extractNotes       (vector<AccentFeatures>& data, HumdrumFile& infile, Options& options);
void   extractFeatures    (vector<AccentFeatures>& data);
void   printData          (vector<AccentFeatures>& data);
HTp    getPreviousNote    (HTp starting);
HTp    getNextNote        (HTp starting);


///////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv) {
	Options options;
	options.define("A|all=b", "extract all features");
	options.process(argc, argv);
	HumdrumFileStream instream(options);
	HumdrumFile infile;
	vector<AccentFeatures> data;
	while (instream.read(infile)) {
		extractNotes(data, infile, options);
		extractFeatures(data);
		printData(data);
	}
	return 0;
}



//////////////////////////////
//
// printData --
//

void printData(vector<AccentFeatures>& data) {
	int fcount = 19;          // Number of features in output data.
	cout << "**kern";	  	  	  // Humdrum **kern pitch extracted from score.
	cout << "\t**line";       // Line number in score (offset from 1).
	cout << "\t**field";	     // Field number on line (offset from 1).
	cout << "\t**track";      // Spine number on line (offset from 1).
	cout << "\t**subtrack";   // Subtrack information (currently a string, convert to integer later).
	cout << "\t**start";      // Starting time of note in score as quarter notes from start of music.
	cout << "\t**dur";        // Duration of the note in quarter notes.
	cout << "\t**pitch";      // MIDI pitch number of note (60 = middle C)
	cout << "\t**chord";      // Is note in chord; if so, then which note in chord?
	cout << "\t**accent";     // Does note have an accent (^, ^^, z).
	cout << "\t**trill";      // Does note have a trill (or mordent)
	cout << "\t**slur_start"; // Does note have a slur start?
	cout << "\t**slur_end";   // Does note have a slur end?
	cout << "\t**pkern";      // **kern data for previous note (or chord)
	cout << "\t**ppitch";     // MIDI note number of note that precedes.
	cout << "\t**pstart";     // Starting time of the previous note in score (-1 means no previous note)
	cout << "\t**nkern";      // **kern data for next note (or chord)
	cout << "\t**npitch";     // MIDI note number of note that follows.
	cout << "\t**nstart";     // Starting time of the next note in score (-1 means no next note)
	cout << endl;

	for (int i=0; i<(int)data.size(); i++) {

		// **kern
		cout << data[i].text;

		// **line
		cout << "\t" << data[i].token->getLineNumber();

		// **field
		cout << "\t" << data[i].token->getFieldNumber();

		// **track
		cout << "\t" << data[i].token->getTrack();

		// **subtrack
		cout << "\t" << data[i].token->getSpineInfo();

		// **start
		HumNum currPos = data[i].token->getDurationFromStart();
		cout << "\t" << currPos.getFloat();

		// **dur
		cout << "\t" << data[i].token->getTiedDuration().getFloat();

		// **pitch
		cout << "\t" << Convert::kernToMidiNoteNumber(data[i].text);

		// **chord
		cout << "\t" << data[i].chord_num;

		// **accent
		cout << "\t" << (data[i].accent ? 1 : 0);

		// **trill
		cout << "\t" << (data[i].trill ? 1 : 0);

		// **slur_start
		cout << "\t" << (data[i].slur_start ? 1 : 0);

		// **slur_end
		cout << "\t" << (data[i].slur_end ? 1 : 0);

		// **pkern
		cout << "\t" << (data[i].prevNote ? data[i].prevNote : 0);

		// **ppitch
		cout << "\t" << (data[i].prevNote ? Convert::kernToMidiNoteNumber(data[i].prevNote) : 0);

		// **pstart
		if (data[i].prevNote) {
			HumNum prevPos = data[i].prevNote->getDurationFromStart();
			cout << "\t" << prevPos.getFloat();
		} else {
			cout << "\t" << -1;
		}

		// **nkern
		cout << "\t" << (data[i].nextNote ? data[i].nextNote : 0);

		// **npitch
		cout << "\t" << (data[i].nextNote ? Convert::kernToMidiNoteNumber(data[i].nextNote) : 0);

		// **nstart
		if (data[i].nextNote) {
			HumNum nextPos = data[i].nextNote->getDurationFromStart();
			cout << "\t" << nextPos.getFloat();
		} else {
			cout << "\t" << 0;
		}

		cout << endl;
	}

	// print data terminators:
	cout << "*-";
	for (int i=1; i<fcount; i++) {
		cout << "\t*-";
	}
	cout << endl;
}



//////////////////////////////
//
// extractFeatures --
//

void extractFeatures(vector<AccentFeatures>& data) {
	for (int i=0; i<(int)data.size(); i++) {

		if (data[i].text.find("t") != string::npos) {
			data[i].trill = true;
		}
		if (data[i].text.find("T") != string::npos) {
			data[i].trill = true;
		}

		if (data[i].text.find("^") != string::npos) {
			data[i].accent = true;
		}
		if (data[i].text.find("z") != string::npos) {
			// sforzando
			data[i].accent = true;
		}

		if (data[i].token->find("(") != string::npos) {
			data[i].slur_start = true;
		}
		if (data[i].token->find(")") != string::npos) {
			data[i].slur_end = true;
		}

		data[i].nextNote = getNextNote(data[i].token);
		data[i].prevNote = getPreviousNote(data[i].token);
	}
}


//////////////////////////////
//
// getNextNote -- Return the next note after the given token. 
//    Returns NULL if there is no following note.  Skips over rests
//    and only follows the track.
//

HTp getNextNote(HTp starting) {
	HTp current = starting->getNextToken();
	while (current) {
		if (!current->isData()) {
			current = current->getNextToken();
			continue;
		}
		if ((!current->isNull()) && (current->isNoteAttack())) {
			return current;
		}
		current = current->getNextToken();
	}
	return NULL;
}



//////////////////////////////
//
// getPreviousNote -- Return the previous note before the given token. 
//    Returns NULL if there is no preceding note.  Skips over rests
//    and only follows the track.
//

HTp getPreviousNote(HTp starting) {
	HTp current = starting->getPreviousToken();
	while (current) {
		if (!current->isData()) {
			current = current->getPreviousToken();
			continue;
		}
		if ((!current->isNull()) && (current->isNoteAttack())) {
			return current;
		}
		current = current->getPreviousToken();
	}
	return NULL;
}



//////////////////////////////
//
// extractNotes --
//

void extractNotes(vector<AccentFeatures>& data, HumdrumFile& infile, 
		Options& options) {
	data.clear();
	data.reserve(infile.getLineCount() + infile.getMaxTrack() * 4);
	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].hasSpines()) {
			continue;
		}
		if (!infile[i].isData()) {
			continue;
		}
		for (int j=0; j<infile[i].getFieldCount(); j++) {
			HTp token = infile.token(i, j);
			if (!token->isKern()) {
				continue;
			}
			if (token->isNull()) {
				continue;
			}
			if (!token->isNoteAttack()) {
				continue;
			}
			int subcount = token->getSubtokenCount();
			vector<string> subtoks;
			for (int k=0; k<subcount; k++) {
				string sub = token->getSubtoken(k);
				if (sub.find("]") != string::npos) {
					continue;
				}
				if (sub.find("_") != string::npos) {
					continue;
				}
				if (sub.find("r") != string::npos) {
					continue;
				}
				AccentFeatures af;
				af.token = token;
				af.subtoken = k;
				af.text = sub;
				if (subcount > 1) {
					af.chord_num++;
				}
				data.push_back(af);
			}
		}
	}
}



