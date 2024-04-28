//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Apr 27 10:23:51 PDT 2024
// Last Modified: Sat Apr 27 10:23:54 PDT 2024
// Filename:      cli/midiphrases.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/cli/midiphrases.cpp
// Syntax:        C++11
// vim:           ts=3 noexpandtab nowrap
//
// Description:   Extract phrases of MIDI notes (for Essen Folksong Collection)
//

#include "humlib.h"
#include <iostream>
#include <map>

using namespace hum;
using namespace std;

void  processFile     (HumdrumFile& infile);
void  processSpine    (HTp start, int voice);
void  transposeInput  (HumdrumFile& infile);

bool beatQ      = false; // used with -b option
bool durationQ  = false; // used with -d option
bool fileQ      = false; // used with -f option
bool keyQ       = false; // used with -k option
bool measureQ   = false; // used with -m option
bool pitchQ     = false; // used with -p option
bool timesigQ   = false; // used with -s option
bool transposeQ = false; // used with -t option
bool voiceQ     = false; // used with -v option

string filename;


///////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv) {
	Options options;
	options.define("b|beat=b",           "print beat position before MIDI note");
	options.define("d|duration=b",       "print note duration before MIDI note");
	options.define("f|filename=b",       "print filename source of MIDI notes");
	options.define("k|key=b",            "print note metric position before MIDI note");
	options.define("m|measure=b",        "print measure position before MIDI note");
	options.define("s|time-signature=b", "print note metric position before MIDI note");
	options.define("p|pitch=b",          "print input token before MIDI note");
	options.define("t|transpose=b",      "transpose input to tonic on C");
	options.define("v|voice=b",          "print voice number of note");
	options.process(argc, argv);

	beatQ      = options.getBoolean("beat");
	durationQ  = options.getBoolean("duration");
	fileQ      = options.getBoolean("filename");
	keyQ       = options.getBoolean("key");
	measureQ   = options.getBoolean("measure");
	timesigQ   = options.getBoolean("time-signature");
	pitchQ     = options.getBoolean("pitch");
	transposeQ = options.getBoolean("transpose");
	voiceQ     = options.getBoolean("voice");

	HumdrumFileStream instream(options);
	HumdrumFile infile;
	while (instream.read(infile)) {
		filename = infile.getFilenameBase();
		if (transposeQ) {
			transposeInput(infile);
		}
		processFile(infile);
	}

	return 0;
}


//////////////////////////////
//
// transposeInput --
//

void transposeInput(HumdrumFile& infile) {
	Tool_transpose transpose;
	vector<string> argv = {"transpose", "-k c"}; // transpose to C tonic.
	transpose.process(argv);
	stringstream sstream;
	sstream << infile;
	HumdrumFile outfile2;
	outfile2.readString(sstream.str());
	transpose.run(outfile2);
	if (transpose.hasHumdrumText()) {
		stringstream ss;
		transpose.getHumdrumText(ss);
		infile.readString(ss.str());
	}
}



//////////////////////////////
//
// processFile --
//

void processFile(HumdrumFile& infile) {
	vector<HTp> starts;
	infile.getKernSpineStartList(starts);
	for (int i=0; i<(int)starts.size(); i++) {
		processSpine(starts[i], i+1);
	}
}



///////////////////////////////
//
// processSpine -- Extract notes from spine (voice).
//

void processSpine(HTp start, int voice) {
	HTp current = start->getNextToken();
	string key = "*:";
	string timesig = "*M";
	int measure = 0;
	bool newlineQ = false;
	bool noteQ = false;
	HumNum beatUnit = 1;
	HumRegex hre;
	while (current) {
		if (current->isKeyDesignation()) {
			key = *current;
		}
		if (current->isBarline()) {
			if (hre.search(current, "(\\d+)")) {
				measure = hre.getMatchInt(1);
			}
		}
		if (current->isTimeSignature()) {
			timesig = *current;
			if (hre.search(timesig, "^\\*M(\\d+)/(\\d+)")) {
				int top = hre.getMatchInt(1);
				int bot = hre.getMatchInt(2);
				beatUnit = bot;
				// compound meters:
				// maybe 3/8
				if ((top == 6) && (bot == 8)) {
					beatUnit = 3;
					beatUnit /= 2;
				} else if ((top == 9) && (bot == 8)) {
					beatUnit = 3;
					beatUnit /= 2;
				} else if ((top == 12) && (bot == 8)) {
					beatUnit = 3;
					beatUnit /= 2;
				}
			}
		}
		if (!current->isData() || current->isNull()) {
			current = current->getNextToken();
			continue;
		}
		if (current->find("{") != string::npos) {
			if (newlineQ) {
				cout << endl;
				newlineQ = false;
				noteQ = false;
			}
			if (fileQ) {
				cout << filename << " ";
			}
			if (keyQ) {
				cout << key << " ";
			}
			if (timesigQ) {
				cout << timesig << " ";
			}
			newlineQ = true;
			noteQ = false;
		}
		if (current->isSecondaryTiedNote()) {
			current = current->getNextToken();
			continue;
		}
		if (noteQ) {
			cout << " ";
		} else {
			noteQ = true;
		}
		if (current->isRest()) {
			if (pitchQ) {
				cout << current << ":";
			}
			if (voiceQ) {
				cout << "v" << voice << ":";
			}
			if (measureQ) {
				cout << "m" << measure << ":";
			}
			if (durationQ) {
				cout << current->getTiedDuration().toFloat() << ":";
			}
			if (beatQ) {
				cout << current->getBeat(beatUnit).toFloat() << ":";
			}
			cout << 0;
			current = current->getNextToken();
			continue;
		}
		if (pitchQ) {
			cout << current << ":";
		}
		if (voiceQ) {
			cout << "v" << voice << ":";
		}
		if (measureQ) {
			cout << "m" << measure << ":";
		}
		if (durationQ) {
			cout << current->getTiedDuration().toFloat() << ":";
		}
		if (beatQ) {
			cout << current->getBeat(beatUnit).toFloat() << ":";
		}
		int midi = current->getMidiPitch();
		if (pitchQ) {
			cout << current << ":";
		}
		cout << midi;
		current = current->getNextToken();
	}
	cout << endl;
}



