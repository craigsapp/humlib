//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Jul  1 11:49:28 CEST 2019
// Last Modified: Mon Mar 16 20:45:00 PDT 2020
// Filename:      cli/accent-features.cpp
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

		int staff_num   = 0;
		int measure     = -1;
		int chord_num   = 0;
		int group       = 0;

		// Accent and articulation features:
		bool accent     = false;
		bool marcato    = false;
		bool sforzando  = false;
		bool tenuto     = false;
		bool staccato   = false;

		// Slur features:
		bool slur_start = false;
		bool slur_end   = false;

		// Trill and ornament features:
		bool trill      = false;
		bool mordent    = false;
		bool turn       = false;

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
void   printData                (vector<AccentFeatures>& data, HumdrumFile& infile, Options& options);
HTp    getPreviousNote          (HTp starting);
HTp    getNextNote              (HTp starting);
void   printMeasureNumberInfo   (vector<MeasureNumberInfo>& measures);
void   extractMeasureNumberInfo (vector<MeasureNumberInfo>& measures, HumdrumFile& infile);
void   printPartInfo            (vector<PartInfo>& parts, HumdrumFile& infile);
void   extractPartInfo          (vector<PartInfo>& parts, HumdrumFile& infile);
void   printLegend              (ostream& output);


///////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv) {
	Options options;
	options.define("A|all=b", "extract all features");
	options.define("k|kern=b", "include original kern notes in output feature list");
	options.define("m|measures=b", "extract measure timings");
	options.define("p|part-list=b", "extract part list");
	options.define("t|tsv|TSV=b", "output in TSV format");
	options.define("l|legend=b", "output description of each column");
	options.process(argc, argv);

	bool measuresQ = options.getBoolean("measures");
	bool partsQ    = options.getBoolean("part-list");

	if (options.getBoolean("legend")) {
		printLegend(cout);
		return 0;
	}

	HumdrumFileStream instream(options);
	HumdrumFile infile;
	vector<AccentFeatures> data;
	vector<MeasureNumberInfo> measures;
	vector<PartInfo> parts;
	while (instream.read(infile)) {
		if (measuresQ) {
			extractMeasureNumberInfo(measures, infile);
			printMeasureNumberInfo(measures);
		} else if (partsQ) {
			extractPartInfo(parts, infile);
			printPartInfo(parts, infile);
		} else {
			extractNotes(data, infile, options);
			extractFeatures(data);
			printData(data, infile, options);
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
		// 1=top: int ktrack = (int)kernspines.size() - i;
		int ktrack = i + 1;
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

void printData(vector<AccentFeatures>& data, HumdrumFile& infile, Options& options) {
	int kernQ = options.getBoolean("kern");

	HumNum tickbase = 1;
	tickbase /= infile.tpq();

	int fcount = 24;          // Number of features in output data.
	if (kernQ) {
		fcount += 3;
	}

	string prefix = "**";
	bool tsvQ = options.getBoolean("tsv");
	if (tsvQ) {
		prefix = "";
	}

	if (kernQ) {
		cout << prefix << "kern";	  	       // Humdrum **kern pitch extracted from score.
		cout << "\t";
	}
	cout << prefix << "line";               // Line number in score (offset from 1).
	cout << "\t" << prefix << "field";	    // Field number on line (offset from 1).
	cout << "\t" << prefix << "track";      // Spine number on line (offset from 1).
	cout << "\t" << prefix << "subtrack";   // Subtrack (i.e. layer/voice on staff).
	cout << "\t" << prefix << "group";      // Rhythmic group (1 = Group A, 2 = Group B).
	cout << "\t" << prefix << "staff";      // Staff number on system (kern spine). 1 = top staff on system.
	cout << "\t" << prefix << "measure";    // Measure number that note is in.
	cout << "\t" << prefix << "qstart";     // Starting time of note in score as quarter notes from start of music.
	cout << "\t" << prefix << "tstart";     // Starting time of note in score as quarter notes from start of music.
	cout << "\t" << prefix << "qdur";       // Duration of the note in quarter notes.
	cout << "\t" << prefix << "tdur";       // Duration of the note in minimum integral ticks.
	cout << "\t" << prefix << "pitch";      // MIDI pitch number of note (60 = middle C)
	cout << "\t" << prefix << "chord";      // Is note in chord; if so, then which note in chord?

	// note accents and articulations:
	cout << "\t" << prefix << "accent";     // Does note have an accent (^)?
	cout << "\t" << prefix << "marcato";    // Does note have a marcato accent (^^)?
	cout << "\t" << prefix << "sforzando";  // Does note have a sforzando accent (z) (also check **dynam)?
	cout << "\t" << prefix << "tenuto";     // Does note have a tenuto (~)?
	cout << "\t" << prefix << "staccato";   // Does note have a staccato (', or `)?

	// note ornaments:
	cout << "\t" << prefix << "trill";      // Does note have a trill?
	cout << "\t" << prefix << "mordent";    // Does note have a mordent?
	cout << "\t" << prefix << "turn";       // Does note have a turn?

	// slurs:
	cout << "\t" << prefix << "sslur";      // Does note have a slur start?
	cout << "\t" << prefix << "eslur";      // Does note have a slur end?

	// previous note information:
	if (kernQ) {
		cout << "\t" << prefix << "pkern";   // **kern data for previous note (or chord)
	}
	cout << "\t" << prefix << "ppitch";     // MIDI note number of note that precedes.
	cout << "\t" << prefix << "pqstart";    // Starting time of the previous note in score (-1 means no previous note)
	cout << "\t" << prefix << "ptstart";    // Starting time of the previous note in score (-1 means no previous note)
	cout << "\t" << prefix << "pqdur";      // Duration of the previous note in score (-1 means no previous note)
	cout << "\t" << prefix << "ptdur";      // Duration of the previous note in score (-1 means no previous note)

	// next note information:
	if (kernQ) {
		cout << "\t" << prefix << "nkern";   // **kern data for next note (or chord)
	}
	cout << "\t" << prefix << "npitch";     // MIDI note number of note that follows.
	cout << "\t" << prefix << "nqstart";    // Starting time of the next note in score (-1 means no next note)
	cout << "\t" << prefix << "ntstart";    // Starting time of the next note in score (-1 means no next note)
	cout << "\t" << prefix << "nqdur";      // Duration of the next note in score (-1 means no next note)
	cout << "\t" << prefix << "ntdur";      // Duration of the next note in score (-1 means no next note)
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

		// **subtrack
		cout << "\t" << data[i].token->getSubtrack();

		// **group
		cout << "\t" << data[i].group;

		// **staff
		cout << "\t" << data[i].staff_num;

		// **measure
		cout << "\t" << data[i].measure;

		// **qstart
		HumNum currPos = data[i].token->getDurationFromStart();
		cout << "\t" << currPos.getFloat();

		// **tstart
		cout << "\t" << currPos / tickbase;

		// **qdur
		cout << "\t" << data[i].token->getTiedDuration().getFloat();

		// **tdur
		cout << "\t" << (data[i].token->getTiedDuration() / tickbase);

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

		// **mordent
		cout << "\t" << (data[i].mordent ? 1 : 0);

		// **turn
		cout << "\t" << (data[i].turn ? 1 : 0);

		// **sslur
		cout << "\t" << (data[i].slur_start ? 1 : 0);

		// **eslur
		cout << "\t" << (data[i].slur_end ? 1 : 0);

		// **pkern
		if (kernQ) {
			if (data[i].prevNote) {
				cout << "\t" << data[i].prevNote;
			} else {
				cout << "\t" << ".";
			}
		}

		// **ppitch
		cout << "\t" << (data[i].prevNote ? Convert::kernToMidiNoteNumber(data[i].prevNote) : -1);

		// **pqstart
		if (data[i].prevNote) {
			HumNum prevPos = data[i].prevNote->getDurationFromStart();
			cout << "\t" << prevPos.getFloat();
		} else {
			cout << "\t" << -1;
		}

		// **ptstart
		if (data[i].prevNote) {
			HumNum prevPos = data[i].prevNote->getDurationFromStart();
			cout << "\t" << prevPos / tickbase;
		} else {
			cout << "\t" << -1;
		}

		// **pqdir
		if (data[i].prevNote) {
			cout << "\t" << data[i].prevNote->getTiedDuration().getFloat();
		} else {
			cout << "\t" << -1;
		}

		// **pqdir
		if (data[i].prevNote) {
			cout << "\t" << data[i].prevNote->getTiedDuration() / tickbase;
		} else {
			cout << "\t" << -1;
		}

		// **nkern
		if (kernQ) {
			if (data[i].nextNote) {
				cout << "\t" << data[i].nextNote;
			} else {
				cout << "\t" << ".";
			}
		}

		// **npitch
		cout << "\t" << (data[i].nextNote ? Convert::kernToMidiNoteNumber(data[i].nextNote) : -1);

		// **nqstart
		if (data[i].nextNote) {
			HumNum nextPos = data[i].nextNote->getDurationFromStart();
			cout << "\t" << nextPos.getFloat();
		} else {
			cout << "\t" << -1;
		}

		// **ntstart
		if (data[i].nextNote) {
			HumNum nextPos = data[i].nextNote->getDurationFromStart();
			cout << "\t" << nextPos / tickbase;
		} else {
			cout << "\t" << -1;
		}

		// **nqdir
		if (data[i].nextNote) {
			cout << "\t" << data[i].nextNote->getTiedDuration().getFloat();
		} else {
			cout << "\t" << -1;
		}

		// **ntdir
		if (data[i].nextNote) {
			cout << "\t" << data[i].nextNote->getTiedDuration() / tickbase;
		} else {
			cout << "\t" << -1;
		}

		cout << endl;
	}

	if (!tsvQ) {
		// Print Humdrum data terminators:
		cout << "*-";
		for (int i=1; i<fcount; i++) {
			cout << "\t*-";
		}
		cout << endl;
	}
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

		if (data[i].text.find("M") != string::npos) {
			data[i].mordent = true;
		}
		if (data[i].text.find("m") != string::npos) {
			data[i].mordent = true;
		}

		if (data[i].text.find("W") != string::npos) {
			data[i].mordent = true;
		}
		if (data[i].text.find("w") != string::npos) {
			data[i].mordent = true;
		}

		if (data[i].text.find("S") != string::npos) {
			data[i].turn = true;
		}
		if (data[i].text.find("$") != string::npos) {
			data[i].turn = true;
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


	int maxtrack = infile.getMaxTrack();
	vector<int> cgroup(maxtrack+1, 0);
	HumRegex hre;
	int cmeasure = 0;
	data.clear();
	data.reserve(infile.getLineCount() + infile.getMaxTrack() * 4);
	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].hasSpines()) {
			continue;
		}
		if (infile[i].isBarline()) {
			if (hre.search(infile.token(i, 0), "(\\d+)")) {
				cmeasure = hre.getMatchInt(1);
			}
		}
		if (infile[i].isInterpretation()) {
			for (int j=0; j<infile[i].getFieldCount(); j++) {
				HTp token = infile.token(i, j);
				int track = token->getTrack();
				if (*token == "*grp:A") {
					cgroup[track] = 1;
				} else if (*token == "*grp:B") {
					cgroup[track] = 2;
				} else if (*token == "*grp:C") {
					cgroup[track] = 3;
				} else if (*token == "*grp:D") {
					cgroup[track] = 4;
				} else if (*token == "*grp:E") {
					cgroup[track] = 5;
				}
			}
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
			int track = token->getTrack();
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
				af.group = cgroup[track];
				af.measure = cmeasure;
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



///////////////////////////////
//
// printLegend --
//

void printLegend(ostream& output) {
	int i = 1;
	output << i++ << ":\tline       == The line number of the note in original Humdrum file.\n";
	output << i++ << ":\tfield      == The column number of the note in original Humdrum file.\n";
	output << i++ << ":\ttrack      == The track number of the note in the original Humdrum file (similar to field).\n";
	output << i++ << ":\tsubtrack   == The subtrack number of the note.  This is the voice/layer: 0=monophonic in staff; 1=polyphonic instaff and in top layer; 2,3=second, third, etc. layer.\n";
	output << i++ << ":\tgroup      == This is the rhythmic group number (0=undefined group; 1=group A; 2=group B).\n";
	output << i++ << ":\tstaff      == This is the staff number (1 = bottom staff).\n";
	output << i++ << ":\tmeasure    == The measure number the note attack occurs in.\n";
	output << i++ << ":\tqstart     == The absolute quarter-note start time of the note.\n";
	output << i++ << ":\ttstart     == The absolute tick start time of the note.\n";
	output << i++ << ":\tqdur       == The quarter-note duration of the note.\n";
	output << i++ << ":\ttdur       == The tick duration of the note.\n";
	output << i++ << ":\tpitch      == The pitch of the note as a MIDI key number (60 = middle C).\n";
	output << i++ << ":\tchord      == Is the note in a chord? 0=no, 1=yes.\n";
	output << i++ << ":\taccent     == Does the note have a regular accent?\n";
	output << i++ << ":\tmarcato    == Does the note have a strong accent?\n";
	output << i++ << ":\tsforzando  == Does the note have a sforzando?\n";
	output << i++ << ":\ttenuto     == Does the note have a tenuto?\n";
	output << i++ << ":\tstaccato   == Does the note have a staccato?\n";
	output << i++ << ":\ttrill      == Does the note have a trill?\n";
	output << i++ << ":\tmordent    == Does the note have a mordent?\n";
	output << i++ << ":\tturn       == Does the note have a turn?\n";
	output << i++ << ":\tsslur      == Does the note have a slur beginning?\n";
	output << i++ << ":\teslur      == Does the note have a slur ending?\n";
	output << i++ << ":\tppitch     == Previous MIDI key number (-1 = no previous note).\n";
	output << i++ << ":\tpqstart    == Previous note absolute quarter-note start time (-1 = no previous note).\n";
	output << i++ << ":\tptstart    == Previous note absolute tick start time (-1 = no previous note).\n";
	output << i++ << ":\tpqdur      == Previous note quarter-note duration (-1 = no previous note).\n";
	output << i++ << ":\tptdur      == Previous note tick duration (-1 = no previous note).\n";
	output << i++ << ":\tnpitch     == Next MIDI key number (-1 = no next note).\n";
	output << i++ << ":\tnqstart    == Next note absolute quarter-note start time (-1 = no next note).\n";
	output << i++ << ":\tntstart    == Next note absolute tick start time (-1 = no next note).\n";
	output << i++ << ":\tnqdur      == Next note quarter-note duration (-1 = no next note).\n";
	output << i++ << ":\tntdur      == Next note tick duration (-1 = no next note).\n";
}



