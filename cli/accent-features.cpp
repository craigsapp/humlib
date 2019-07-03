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
#include <ctype.h>

using namespace hum;
using namespace std;

class AccentFeatures {
	public:
		HTp  token      = NULL;
		int  subtoken   = 0;
		string text;
		HTp  nextNote   = NULL;
		HTp  prevNote   = NULL;

		int staff_num    = 0;
		int chord_num   = 0;

		bool accent     = false;
		bool marcato    = false;
		bool sforzando  = false;
		bool tenuto     = false;
		bool staccato   = false;
		bool slur_start = false;
		bool slur_end   = false;
		bool trill      = false;

		double metpos   = 0;
};


class MeasureNumberInfo {
	public:
		int number = -1;
		double start_time = -1.0;
		double stop_time = -1.0;
};


class PartInfo {
	public:
		int track     = -1;
		int staff_num = -1;
		HTp name      = NULL;            // name of instrument such as *I"Clarinet in Bb
		HTp abbr      = NULL;            // abbreviation of such as *I'cl
		HTp code      = NULL;            // instrument code such as *Iclars (soprano clarinet)
};



void   extractNotes             (vector<AccentFeatures>& data, HumdrumFile& infile, Options& options);
void   extractFeatures          (vector<AccentFeatures>& data);
void   printData                (vector<AccentFeatures>& data, Options& options);
HTp    getPreviousNote          (HTp starting);
HTp    getNextNote              (HTp starting);
void   printMeasureNumberInfo   (vector<MeasureNumberInfo>& measures);
void   extractMeasureNumberInfo (vector<MeasureNumberInfo>& measures, HumdrumFile& infile);
void   printPartInfo            (vector<PartInfo>& parts, HumdrumFile& infile);
void   extractPartInfo          (vector<PartInfo>& parts, HumdrumFile& infile);


///////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv) {
	Options options;
	options.define("A|all=b", "extract all features");
	options.define("k|kern=b", "include original kern notes in output feature list");
	options.define("m|measures=b", "extract measure timings");
	options.define("p|part-list=b", "extract part list");
	options.process(argc, argv);

	bool measuresQ = options.getBoolean("measures");
	bool partsQ    = options.getBoolean("part-list");

	HumdrumFileStream instream(options);
	HumdrumFile infile;
	vector<AccentFeatures> data;
	vector<MeasureNumberInfo> measures;
	vector<PartInfo> parts;
	while (instream.read(infile)) {
		if (measuresQ) {
			extractMeasureNumberInfo(measures, infile);
			printMeasureNumberInfo(measures);
		} if (partsQ) {
			extractPartInfo(parts, infile);
			printPartInfo(parts, infile);
		} else {
			extractNotes(data, infile, options);
			extractFeatures(data);
			printData(data, options);
		}
	}
	return 0;
}



//////////////////////////////
//
// extractPartInfo --
//

void extractPartInfo(vector<PartInfo>& parts, HumdrumFile& infile) {

	// partNumber: Part number (actually staff number), with "1" being the top
	// staff in the system.
	vector<HTp> kernspines = infile.getKernSpineStartList();
	int trackcount = infile.getTrackCount();
	vector<int> partNumber(trackcount + 1, 0);
	for (int i=0; i<(int)kernspines.size(); i++) {
		int track = kernspines[i]->getTrack();
		int ktrack = (int)kernspines.size() - i;
		partNumber[track] = ktrack;
	}

	parts.clear();
	parts.reserve(kernspines.size());

	HTp current;
	for (int i=(int)kernspines.size() - 1; i>=0; i--) {
		PartInfo pinfo;
		pinfo.track = kernspines[i]->getTrack();
		pinfo.staff_num = partNumber[pinfo.track];
		pinfo.code = NULL;
		pinfo.abbr = NULL;
		pinfo.name = NULL;
		current = kernspines[i]->getNextToken();
		while (current) {
			if (current->isData()) {
				break;
			}
			if (!current->isInterpretation()) {
				current = current->getNextToken();	
				continue;
			}
			if (current->compare(0, 3, "*I\"") == 0) {
				pinfo.name = current;
			} else if (current->compare(0, 3, "*I\'") == 0) {
				pinfo.abbr = current;
			} else if (current->compare(0, 2, "*I") == 0) {
				string value = current->substr(2);
				if (value.size() > 0) {
					if (islower(value[0])) {
						pinfo.code = current;
					}
				}
			}
			current = current->getNextToken();
		}
		parts.push_back(pinfo);
	}
}



//////////////////////////////
//
// printPartInfo --
//


void printPartInfo(vector<PartInfo>& parts, HumdrumFile& infile) {

	// partNumber: Part number (actually staff number), with "1" being the top
	// staff in the system.
	vector<HTp> kernspines = infile.getKernSpineStartList();
	int trackcount = infile.getTrackCount();
	vector<int> partNumber(trackcount + 1, 0);
	for (int i=0; i<(int)kernspines.size(); i++) {
		int track = kernspines[i]->getTrack();
		int ktrack = (int)kernspines.size() - i;
		partNumber[track] = ktrack;
	}

	int pfeatures = 5;
	cout << "**track";
	cout << "\t**staff";
	cout << "\t**code";
	cout << "\t**abbr";
	cout << "\t**name";
	cout << endl;

	for (int i=(int)kernspines.size() - 1; i>=0; i--) {
		int track = kernspines[i]->getTrack();
		cout << track;
		cout << "\t";

		cout << partNumber[track];
		cout << "\t";

		int partindex = partNumber[track] - 1;
		if (parts[partindex].code) {
			cout << parts[partindex].code->substr(2);
		} else {
			cout << ".";
		}
		cout << "\t";

		if (parts[partindex].abbr) {
			cout << parts[partindex].abbr->substr(3);
		} else {
			cout << ".";
		}
		cout << "\t";

		if (parts[partindex].name) {
			cout << parts[partindex].name->substr(3);
		} else {
			cout << ".";
		}

		cout << endl;
	}

	cout << "*-";
	for (int i=1; i<pfeatures; i++) {
		cout << "\t*-";
	}
	cout << endl;
}



//////////////////////////////
//
// printMeasureNumberInfo -- Prints a list of start times and stop times for 
//    measures in the data (in units of quarter notes).
//

void printMeasureNumberInfo(vector<MeasureNumberInfo>& measures) {
	int fcount = 3;
	cout << "**num";
	cout << "\t**start";
	cout << "\t**stop";
	cout << endl;
	for (int i=0; i<(int)measures.size(); i++) {
		cout << measures[i].number;
		cout << "\t" << measures[i].start_time;
		cout << "\t" << measures[i].stop_time;
		cout << endl;
	}
	cout << "*-";
	for (int i=0; i<fcount-1; i++) {
		cout << "\t*-";
	}
	cout << endl;
}



//////////////////////////////
//
// extractMeasureNumberInfo --
//

void extractMeasureNumberInfo(vector<MeasureNumberInfo>& measures, HumdrumFile& infile) {
	measures.clear();
	measures.reserve(infile.getLineCount());
	int number;
	double start_time;
	double stop_time;
	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isBarline()) {
			continue;
		}
		number = infile.getMeasureNumber(i);
		if (number < 0) {
			continue;
		}
		start_time = infile[i].getDurationFromStart().getFloat();
		stop_time = start_time + infile[i].getDurationToBarline().getFloat();
		if (measures.size() > 0) {
			// there could be barlines without numbers (such as at repeats),
			// so revise the duration to skip unnumbered barlines.
			measures.back().stop_time = start_time;
		}
		MeasureNumberInfo minfo;

		if (measures.empty() && (start_time > 0.0)) {
			// there is a pickup measure, so store information
			// about it, using one less that the current measure number
			minfo.number = number - 1;
			minfo.start_time = 0.0;
			minfo.stop_time = start_time;
			measures.push_back(minfo);
		}

		minfo.number = number;
		minfo.start_time = start_time;
		minfo.stop_time = stop_time;
		measures.push_back(minfo);
	}
}



//////////////////////////////
//
// printData --
//

void printData(vector<AccentFeatures>& data, Options& options) {
	int kernQ = options.getBoolean("kern");

	int fcount = 21;          // Number of features in output data.
	if (kernQ) {
		fcount += 3;
	}

	if (kernQ) {
		cout << "**kern";	  	  // Humdrum **kern pitch extracted from score.
		cout << "\t";
	}
	cout << "**line";         // Line number in score (offset from 1).
	cout << "\t**field";	     // Field number on line (offset from 1).
	cout << "\t**track";      // Spine number on line (offset from 1).
	cout << "\t**staff";      // Staff number on system (kern spine). 1 = top staff on system.
	cout << "\t**subtrack";   // Subtrack information (currently a string, convert to integer later).
	cout << "\t**start";      // Starting time of note in score as quarter notes from start of music.
	cout << "\t**dur";        // Duration of the note in quarter notes.
	cout << "\t**pitch";      // MIDI pitch number of note (60 = middle C)
	cout << "\t**chord";      // Is note in chord; if so, then which note in chord?
	cout << "\t**accent";     // Does note have an accent (^).
	cout << "\t**marcato";    // Does note have a marcato accent (^^).
	cout << "\t**sforzando";  // Does note have a sforzando accent (z) (also check **dynam).
	cout << "\t**tenuto";     // Does note have a tenuto (~).
	cout << "\t**staccato";   // Does note have a staccato (', or `).
	cout << "\t**trill";      // Does note have a trill (or mordent)
	cout << "\t**slur_start"; // Does note have a slur start?
	cout << "\t**slur_end";   // Does note have a slur end?
	if (kernQ) {
		cout << "\t**pkern";   // **kern data for previous note (or chord)
	}
	cout << "\t**ppitch";     // MIDI note number of note that precedes.
	cout << "\t**pstart";     // Starting time of the previous note in score (-1 means no previous note)
	if (kernQ) {
		cout << "\t**nkern";   // **kern data for next note (or chord)
	}
	cout << "\t**npitch";     // MIDI note number of note that follows.
	cout << "\t**nstart";     // Starting time of the next note in score (-1 means no next note)
	cout << endl;

	for (int i=0; i<(int)data.size(); i++) {

		// **kern
		if (kernQ) {
			cout << data[i].text;
			cout << "\t";
		}

		// **line
		cout << data[i].token->getLineNumber();

		// **field
		cout << "\t" << data[i].token->getFieldNumber();

		// **track
		cout << "\t" << data[i].token->getTrack();

		// **staff
		cout << "\t" << data[i].staff_num;

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

		// **marcato
		cout << "\t" << (data[i].marcato ? 1 : 0);

		// **sforzando
		cout << "\t" << (data[i].sforzando ? 1 : 0);

		// **tenuto
		cout << "\t" << (data[i].tenuto ? 1 : 0);

		// **staccato
		cout << "\t" << (data[i].staccato ? 1 : 0);

		// **trill
		cout << "\t" << (data[i].trill ? 1 : 0);

		// **slur_start
		cout << "\t" << (data[i].slur_start ? 1 : 0);

		// **slur_end
		cout << "\t" << (data[i].slur_end ? 1 : 0);

		// **pkern
		if (kernQ) {
			cout << "\t" << (data[i].prevNote ? data[i].prevNote : 0);
		}

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
		if (kernQ) {
			cout << "\t" << (data[i].nextNote ? data[i].nextNote : 0);
		}

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

		if (data[i].text.find("^^") != string::npos) {
			data[i].marcato = true;
		} else if (data[i].text.find("^") != string::npos) {
			data[i].accent = true;
		}

		if (data[i].text.find("z") != string::npos) {
			data[i].sforzando = true;
		}

		if (data[i].text.find("~") != string::npos) {
			data[i].tenuto = true;
		}

		if (data[i].text.find("'") != string::npos) {
			data[i].staccato = true;
		} else if (data[i].text.find("`") != string::npos) {
			// staccatissimo
			data[i].staccato = true;
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

	// partNumber: Part number (actually staff number), with "1" being the top
	// staff in the system.
	vector<HTp> kernspines = infile.getKernSpineStartList();
	int trackcount = infile.getTrackCount();
	vector<int> partNumber(trackcount + 1, 0);
	for (int i=0; i<(int)kernspines.size(); i++) {
		int track = kernspines[i]->getTrack();
		int ktrack = (int)kernspines.size() - i;
		partNumber[track] = ktrack;
	}

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
				af.staff_num = partNumber[token->getTrack()];
				data.push_back(af);
			}
		}
	}
}



