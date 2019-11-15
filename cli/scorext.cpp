//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Tue Oct 29 22:38:35 PDT 2019
// Last Modified: Thu Nov 14 23:56:42 WET 2019
// Filename:      scorext.cpp
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab nowrap
//
// Description:   Extract musical features for performance-based machine
//                learning analysis of the score.
//
// I. Note-wise features
//
// For each note in the score (N notes in total), I need to extract
// 12+ features, each feature is indicated by a number. So the function
// could return and store a matrix of N by 12 given a kern score.
//
// 1.  Pitch number ( from 0-127)
//     [Column 2 of note data, labeled "MIDI"]
// 2.  Which voice it belongs to
//     [Column 3 of note data, labeled "VOICE"]
//        0 = Voice 1, but only when there is no voice 2 on staff.
//        1 = Voice 1, and there is a voice 2 active on the staff at the same time.
//        2, 3, 4 = Other possible voice numbers on staff.
//        The difference between 0 and 1/2 might be useful for interactions
//        with the pedal (such as more pedal may be needed when there is
//        more than one voice at a time).
// 3.  Duration in beats (i.e. 1.0 for whole beat, 0.5 for half beat,
//     0.25 for quarter beat)
//     [Column 7 of note data, labeled "DUR"]
// 4.  Onset in beat
//     [Column 9 in the note data, labeled "METER"]
// 5.  Offset in beat
//     [Not directly indicated yet]
// 6.  Which measure it belongs to
//     [Column 8 in the note data, labeled "BAR"]
// 7.  The beat phase in the measure (i.e 0-3 if it's a 4/4, or a ratio 
//     between 0-1 if it's easier.)
//     [Maybe this is column 9?, what is the difference compared to #4?]
// 8.  Whether it has a staccato marking (0 or 1)
//     [Column 10 in the note data, labeled "STAC"]
// 9.  Whether it has a fermata marking (0 or 1)
//     [Column 11 in the note data, labeled "FERM"]
// 10. Whether it has an accent marking (0 or 1)
//     [Column 12 in the note data, labeled "ACNT"]
// 11. If it's an accidental in a score. 
//     [Column 13 in the note data, labeled "ACCID"]
//        0 = no accidental
//        1 = double flat
//        2 = flat
//        3 = natural
//        4 = sharp
//        5 = double sharp
// 12. Which staff it belongs to (0 for bass and 1 for treble)
//     [Column 3 in the note data, labeled STAFF]
// 
// II. Slur information
//    Column 1: (starting) staff number of slur
//    Column 2: (starting) voice number of slur, 0 means that it
//              there is only one active voice at the starting
//              position.
//    Column 3: Duration of the slur (in quarter notes since start)
//
// And I would want all the slur information including the start
// beat, and end beat of a slur
// 
// III. Dynamic Information
//
// I'd like to know the beat position(start and end) of each
// crescendo, decrescendo, p, f, pp, ff, .... that related to dynamics
// 
// IV. Pedal Information
//
//    Column 1: Pedal type (1 = pedal down, 0 = pedal up).
//    Column 2: Starting time of the pedal event in quarter notes 
//    Column 2: Measure fraction position of pedal event.
//    Column 3: Duration to previous pedal down or next pedal up.
//
// I'd like to know which beat includes a pedal marking.
// 
// V. Others
// 
// Other information such as key and metric information, music
// terms, or expression text, repeats, or markings in the score.
//
//
// To do:
//
// * Deal with pickup beat (downbeat of next measure will be defined as 0 point in absbeat.
//

#include "humlib.h"
#include <iostream>

using namespace std;
using namespace hum;


void        processFile          (HumdrumFile& infile);
vector<int> getTrack2StaffMapping(HumdrumFile& infile);
bool        hasStaccato          (HTp token);
bool        hasAccent            (HTp token);
bool        hasFermata           (HTp token);
int         hasVisualAccidental  (HTp token, int subtoken);
void        printNoteData        (HumdrumFile& infile);
void        printSlurData        (HumdrumFile& infile);
void        printDynamicsData    (HumdrumFile& infile);
void        printPedalData       (HumdrumFile& infile);
void        getPedalTokens       (vector<HTp>& pedalseq, HumdrumFile& infile);
void        printTextData        (HumdrumFile& infile);
HumNum      getAbsBeat           (HTp token);
void        fillAnalysisInfo     (HumdrumFile& infile);
HumNum      getMeasureFraction   (HTp token);
void        getSlurList          (vector<HTp>& slurlist, HumdrumFile& infile);
HumNum      getSlurDuration      (HTp token, int index);

// By default extract all types of data; otherwise, specify
// specific features to extract based on the following options:
bool NotesQ = true;    // used with -n option
bool SlursQ = true;    // used with -s option
bool DynamQ = true;    // used with -d option
bool PedalQ = true;    // used with -p option
bool TextQ  = true;    // used with -t option

bool LegendQ = true;   // used with -L option

vector<int> Barnum;    // indexed by Humdrum file line
vector<HumNum> Meterdur;  // duration of meter

///////////////////////////////////////////////////////////////////////////


int main(int argc, char** argv) {
	Options options;
	options.define("n|notes=b",    "Extract note data"    );
	options.define("s|slurs=b",    "Extract slur data"    );
	options.define("d|dynamics=b", "Extract dynamics data");
	options.define("p|pedal=b",    "Extract pedal data"   );
	options.define("t|text=b",     "Extract text data"    );
	options.define("L|no-legend=b", "don't give parameter legend");
	options.process(argc, argv);

	// If any options are specified, then only display the
	// data that is requested:
	NotesQ  = !options.getBoolean("notes");
	SlursQ  = !options.getBoolean("slurs");
	DynamQ  = !options.getBoolean("dynamics");
	PedalQ  = !options.getBoolean("pedal");
	TextQ   = !options.getBoolean("text");
	if (!(NotesQ && SlursQ && DynamQ && PedalQ && TextQ)) {
		NotesQ = !NotesQ;
		SlursQ = !SlursQ;
		DynamQ = !DynamQ;
		PedalQ = !PedalQ;
		TextQ  = !TextQ;
	}
	LegendQ = !options.getBoolean("no-legend");

	HumdrumFile infile;
	if (options.getArgCount() == 0) {
		infile.read(cin);
	} else {
		infile.read(options.getArg(1));
	}
	processFile(infile);
	return 0;
}


///////////////////////////////////////////////////////////////////////////


//////////////////////////////
//
// processFile -- Extract data from a score.
//

void processFile(HumdrumFile& infile) {
	infile.analyzeSlurs();
	fillAnalysisInfo(infile);
	if (NotesQ) { printNoteData(infile);     }
	if (SlursQ) { printSlurData(infile);     }
	if (DynamQ) { printDynamicsData(infile); }
	if (PedalQ) { printPedalData(infile);    }
	if (TextQ ) { printTextData(infile);     }
}



//////////////////////////////
//
// printNoteData -- Extract articulation information for notes in score.
// 
// Todo: also add visible rests.
//

void printNoteData(HumdrumFile& infile) {
	// cout << "NOTES:\n";

	vector<int> track2staff = getTrack2StaffMapping(infile);

	if (LegendQ) {
		cout << "\n";
		cout << "########################################################\n";
		cout << "## LIST OF NOTES                                      ##\n";
		cout << "##                                                    ##\n";
		cout << "## Meaning of columns:                                ##\n";
		cout << "## 1  NOTE : note enumeration number.                 ##\n";
		cout << "## 2  MIDI:  MIDI number number (60=middle C).        ##\n";
		cout << "## 3  STAFF: staff on which the note belongs.         ##\n";
		cout << "## 4  VOICE: voice number on staff:                   ##\n";
		cout << "##      0 = layer 1, but no other layers              ##\n";
		cout << "##      1 = layer 1 (highest layer on staff)          ##\n";
		cout << "##      2 = layer 2 (lowest layer on staff)           ##\n";
		cout << "##      3 = layer 3, etc.                             ##\n";
		cout << "## 5  START: the starting time in quarter notes from  ##\n";
		cout << "##         start of music.                            ##\n";
		cout << "## 6  END:   the ending time in quarter notes from    ##\n";
		cout << "##         start of music.                            ##\n";
		cout << "## 7  DUR:   The duration of the note in quarters.    ##\n";
		cout << "## 8  BAR:   The measure that the note is in.         ##\n";
		cout << "## 9  STAC:  Boolean for staccato on note.            ##\n";
		cout << "## 10 STAC:  Boolean for staccato on note.            ##\n";
		cout << "## 11 FERM:  Boolean for fermata on note.             ##\n";
		cout << "## 12 ACNT:  Boolean for accent on note.              ##\n";
		cout << "## 13 ACCID: Visual accidental:                       ##\n";
		cout << "##          0 = no accidental                         ##\n";
		cout << "##          1 = double flat                           ##\n";
		cout << "##          2 = double flat                           ##\n";
		cout << "##          3 = natural sign                          ##\n";
		cout << "##          4 = sharp                                 ##\n";
		cout << "##          5 = double sharp                          ##\n";
		cout << "########################################################\n";
	}

	cout << "#NOTE\t"    // Note number key number of note
	     << "MIDI\t"     // MIDI key number of note
	     << "STAFF\t"    // staff number of note
	     << "VOICE\t"    // voice/layer number of note (0 = layer 1 when no layer 2)
	     << "START\t"    // start time in quarter notes from start of music
	     << "END\t"      // end time in quarter notes from start of music
	     << "DUR\t"      // duration of note in quarter notes
	     << "BAR\t"     // measure number that the note is in
	     << "METER\t"    // duration from start of metric cycle in quarter notes
	     << "STAC\t"     // 1 = staccato, 0 = not staccato
	     << "FERM\t"     // 1 = fermata, 0 = no fermata
	     << "ACNT\t"     // 1 = accent/marcato, 0 = no accent
	     << "ACCID\t"    // Visual accidental 0=no accent, 1=double flat, 2=flat, 3=natural, 4=sharp, 5=double sharp
	     << endl;

	int notecounter = 0;
	int barnum = 0;
	for (int i=0; i<infile.getLineCount(); i++) {
		if (infile[i].isBarline()) {
			HumRegex hre;
			HTp token = infile.token(i, 0);
			if (hre.search(token, "^=(\\d+)")) {
				barnum = hre.getMatchInt(1);
			}
		}
		if (!infile[i].isData()) {
			continue;
		}
		for (int j=0; j<infile[i].getFieldCount(); j++) {
			HTp token = infile.token(i, j);
			if (!token->isKern()) {
				// ignore non-music
				continue;
			}
			if (token->isNull()) {
				// ignore empty placeholders
				continue;
			}
			if (token->isRest()) {
				// ignore rests
				continue;
			}
			int notecount = token->getSubtokenCount();
			for (int k=0; k<notecount; k++) {
				string subtoken = token->getSubtoken(k);
				if (subtoken.find("_") != string::npos) {
					// sustained note so skip
					continue;
				}
				if (subtoken.find("]") != string::npos) {
					// sustained note so skip
					continue;
				}
				int midi  = Convert::kernToMidiNoteNumber(subtoken);
				int track = token->getTrack();
				int staff = track2staff[track];
				int voice = token->getSubtrack();
				HumNum nstart = token->getDurationFromStart();
				HumNum dur    = token->getTiedDuration();
				HumNum nend   = nstart + dur;
				HumNum nmeter = token->getDurationFromBarline();
				int staccato = 0;
				if (hasStaccato(token)) {
					staccato = 1;
				}
				int accent = 0;
				if (hasAccent(token)) {
					accent = 1;
				}
				int fermata = 0;
				if (hasFermata(token)) {
					fermata = 1;
				}
				int accidental = hasVisualAccidental(token, k);
				notecounter++;

				cout << "NOTE-" << notecounter << "\t"     // NOTE
				     << midi << "\t"                       // MIDI
				     << staff << "\t"                      // STAFF
				     << voice << "\t"                      // VOICE
				     << nstart.getFloat() << "\t"          // START
				     << nend.getFloat() << "\t"            // END
				     << dur.getFloat() << "\t"             // DUR
				     << barnum << "\t"                     // BAR
				     << nmeter.getFloat() << "\t"          // METER
				     << staccato << "\t"                   // STAC
				     << fermata << "\t"                    // FERM
				     << accent << "\t"                     // ACNT
				     << accidental << "\t"                 // ACCID
				     << endl;
			}
		}
	}
}



//////////////////////////////
//
// printSlurData -- Print information about slurs.
//

void printSlurData(HumdrumFile& infile) {
	// cout << "\nSLURS:\n";

	if (LegendQ) {
		cout << "\n";
		cout << "########################################################\n";
		cout << "## LIST OF SLURS                                      ##\n";
		cout << "##                                                    ##\n";
		cout << "## Meaning of columns:                                ##\n";
		cout << "## 1  SLUR:  Slur enumeration number.                 ##\n";
		cout << "## 2  BAR:   The measure that the slur is in.         ##\n";
		cout << "## 3  ABSQ:  The starting time in quarter notes from  ##\n";
		cout << "##           start of music.                          ##\n";
		cout << "## 4  SDUR:  the ending time in quarter notes from    ##\n";
		cout << "##           start of music.                          ##\n";
		cout << "## 5  MFRAC: Fractional position of the slur in bar.  ##\n";
		cout << "########################################################\n";
	}

	// Data column meanings:
	cout << "#SLUR";
	cout << "\tBAR";
	cout << "\tABSQ";
	cout << "\tSDUR";
	cout << "\tMFRAC";
	cout << "\n";

	vector<HTp> slurlist;
	getSlurList(slurlist, infile);

	for (int i=0; i<(int)slurlist.size(); i++) {
		HTp token = slurlist[i];

		// @@SLUR-#: The enumeration of the slur
		cout << "SLUR-" << (i+1);

		// @@BAR: The measure/bar number in which the pedal is locaed.
		cout << "\t" << Barnum[token->getLineIndex()];

		// @@ABSQ: the position in the score since the first barline
		HumNum start = getAbsBeat(token);
		cout << "\t" << start.getFloat();

		// @@SDUR: the duration to the slur end.  
		cout << "\t" << getSlurDuration(token, 0).getFloat();

		// @@MFRAC: the position in the score since the first barline
		cout << "\t" << getMeasureFraction(token).getFloat();

		cout << endl;
	}
}



//////////////////////////////
//
// printDynamicsData -- Print information about dynamics.
//

void printDynamicsData(HumdrumFile& infile) {
	cout << "\n#DYNAMICS: list of dynamics goes here\n";
}



//////////////////////////////
//
// printPedalData -- Print information about pedalling.
//

void printPedalData(HumdrumFile& infile) { 
	if (LegendQ) {
		cout << "\n";
		cout << "########################################################\n";
		cout << "## LIST OF PEDALING                                   ##\n";
		cout << "##                                                    ##\n";
		cout << "## Meaning of columns:                                ##\n";
		cout << "## 1  PEDAL: Pedal enumeration number.                ##\n";
		cout << "## 2  PTYPE: Type of pedal (0 = off; 1 = on).         ##\n";
		cout << "## 3  BAR:   The measure that the pedal is in.        ##\n";
		cout << "## 4  ABSQ:  The starting time in quarter notes from  ##\n";
		cout << "##           start of music.                          ##\n";
		cout << "## 5  PDUR:  the ending time in quarter notes from    ##\n";
		cout << "##           start of music.                          ##\n";
		cout << "## 6  MFRAC: Fractional position of the pedal in bar. ##\n";
		cout << "########################################################\n";
	}

	// Data column meanings:
	cout << "#PEDAL";
	cout << "\tPTYPE";
	cout << "\tBAR";
	cout << "\tABSQ";
	cout << "\tPDUR";
	cout << "\tMFRAC";
	cout << "\n";

	vector<HTp> pedalseq;
	getPedalTokens(pedalseq, infile);
	for (int i=0; i<(int)pedalseq.size(); i++) {
		HTp pedal = pedalseq[i];
		HTp nextpedal = NULL;
		if (i < (int)pedalseq.size() - 1) {
			nextpedal = pedalseq[i+1];
		}

		// @@PEDAL-# and enumeration of the pedal
		cout << "PEDAL-" << (i+1) << "\t";

		// @@PTYPE: the pedal message type (0 = pedal up, 1 = pedal down)
		if (*pedal == "*ped") {
			cout << "1";
		} else if (*pedal == "*Xped") {
			cout << "0";
		} else {
			cout << "-1";
		}

		// @@BAR: The measure/bar number in which the pedal is locaed.
		cout << "\t" << Barnum[pedal->getLineIndex()];

		// @@ABSQ: the position in the score since the first barline
		HumNum start = getAbsBeat(pedal);
		cout << "\t" << start.getFloat();

		// @@PDUR: the duration to the next pedal mark.  If the current
		// pedal is a pedal down, then the PDUR value is the duration
		// of the sustain.
		if (nextpedal) {
			HumNum duration = nextpedal->getDurationFromStart()
					- pedal->getDurationFromStart();
			cout << "\t" << duration.getFloat();
		} else {
			cout << "\t-1";
		}

		// @@MFRAC: the position in the score since the first barline
		cout << "\t" << getMeasureFraction(pedal).getFloat();

		cout << endl;
	}
}



//////////////////////////////
//
// printTextData -- Print information about text written in score.
//

void printTextData(HumdrumFile& infile) { 
	cout << "\n#TEXT: list of text goes here\n";
}



//////////////////////////////
//
// getTrack2StaffMapping -- Generate a lookup table for spine column and staff number,
//    with staff = 1 being the bottom staff of the system. Does not work with
//    dynamics for piano music, though (which will have one spine apply to a single
//    staff.
//

vector<int> getTrack2StaffMapping(HumdrumFile& infile) {
	vector<HTp> starts;
	infile.getSpineStartList(starts);
	vector<int> output(starts.size()+1, 0);
	int counter = 0;
	for (int i=0; i<(int)starts.size(); i++) {
		if (starts[i]->isKern()) {
			counter++;
		}
		output[i+1] = counter;
	}
	return output;
}



//////////////////////////////
//
// hasStaccato -- returns true if the Humdrum **kern token has a staccato.
//    ' = regular staccato
//    ` = staccatissimo (NB: sometimes '' is used to mean staccatisimo)
//

bool hasStaccato(HTp token) {
	if (token->find("'") != std::string::npos) {
		return true;
	}
	if (token->find("`") != std::string::npos) {
		return true;
	}
	return false;
}



//////////////////////////////
//
// hasAccent -- returns true if the Humdrum **kern token has an accent.
//    If one note in a chord is accented, all notes are considered accented.
// 
//    ^  = regular accent
//    ^^ = marcato accent
//    z  = sforzando
//

bool hasAccent(HTp token) {
	if (token->find("^") != std::string::npos) {
		return true;
	}
	if (token->find("z") != std::string::npos) {
		return true;
	}
	return false;
}



//////////////////////////////
//
// hasFermata -- returns true if the Humdrum **kern token has a fermata.
//    ; = fermata
//

bool hasFermata(HTp token) {
	if (token->find(";") != std::string::npos) {
		return true;
	}
	return false;
}



//////////////////////////////
//
// hasVisualAccidental -- returns true if the Humdrum **kern token has
//    a visual accidental.
//       0 = no visual accidental
//       1 = double flat
//       2 = flat
//       3 = natural
//       4 = sharp
//       5 = double sharp
//

int hasVisualAccidental(HTp token, int subtoken) {
	int acc = token->hasVisibleAccidental(subtoken);
	if (!acc) {
		return 0;
	}
	string tstring = token->getSubtoken(subtoken);
	if (tstring.find("--") != std::string::npos) {
		return 1;
	}
	if (tstring.find("-") != std::string::npos) {
		return 2;
	}
	if (tstring.find("##") != std::string::npos) {
		return 5;
	}
	if (tstring.find("#") != std::string::npos) {
		return 4;
	}
	return 3;
}



//////////////////////////////
//
// getAbsBeat -- return the duration from the first barline to
//   the given token.  Elements before the first complete barline
//   are given negative values.
//

HumNum getAbsBeat(HTp token) {
	return token->getDurationFromStart();
}



//////////////////////////////
//
// getMeasureFraction --
//

HumNum getMeasureFraction(HTp token) {
	HumNum tostart = token->getDurationFromBarline();
	HumNum toend = token->getDurationToBarline();
	return tostart / (tostart + toend);
}



/////////////////////////////
//
// getSlurList --
//

void getSlurList(vector<HTp>& slurlist, HumdrumFile& infile) {
	slurlist.clear();
	slurlist.reserve(10000);
	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isData()) {
			continue;
		}
		for (int j=0; j<infile[i].getFieldCount(); j++) {
			HTp token = infile.token(i, j);
			if (!token->isKern()) {
				continue;
			}
			if (token->find("(") != std::string::npos) {
				slurlist.push_back(token);
			}
		}
	}
}



//////////////////////////////
//
// getSlurDuration --
//

HumNum getSlurDuration(HTp token, int index) {
	if (token->find('(') == std::string::npos) {
		return 0;
	}
	// need to add index to getSlurDuration in case there are 
	// multiple slurs attached to the same note/chord.
	return token->getSlurDuration();
}



//////////////////////////////
//
// getPedalTokens -- Return a list of the pedal markings in the score.
//

void getPedalTokens(vector<HTp>& pedalseq, HumdrumFile& infile) {
	pedalseq.clear();
	pedalseq.reserve(1000);
	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isInterpretation()) {
			continue;
		}
		for (int j=0; j<infile[i].getFieldCount(); j++) {
			HTp token = infile.token(i, j);
			if (*token == "*ped") {
				pedalseq.push_back(token);
			} else if (*token == "*Xped") {
				pedalseq.push_back(token);
			}
		}
	}
}



/////////////////////////////////
//
// fillAnalysisInfo -- Extract information about the measure numbera dn
//    time signatures for the file.  This information is stored in
//    vectors indexd by line in the Humdrum data for access when creating
//    fractional positions in scores and barline numbers for data.
//

void fillAnalysisInfo(HumdrumFile& infile) {
	Barnum.resize(infile.getLineCount());
	Meterdur.resize(infile.getLineCount());
	int bar = -1;
	HumNum meter = -1;
	for (int i=0; i<infile.getLineCount(); i++) {
		Barnum[i] = bar;
		Meterdur[i] = meter;
		if (infile[i].isBarline()) {
			int value = infile[i].getBarNumber();
			if (value >= 0) {
				bar = value;
			}
			Barnum[i] = bar;
		}
		if (!infile[i].isInterpretation()) {
			continue;
		}
		for (int j=0; i<infile[i].getFieldCount(); j++) {
			HTp tok = infile.token(i, j);
			if (!tok->isTimeSignature()) {
				continue;
			}
			HumNum value = Convert::timeSigToDurationInQuarter(tok);
			meter = value;
		}
		Meterdur[i] = meter;
	}
}


