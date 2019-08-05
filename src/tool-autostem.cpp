//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sun Dec 26 03:28:25 PST 2010
// Last Modified: Sun Dec  4 16:15:36 PST 2016 Ported from humextras
// Filename:      tool-autostem.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/tool-autostem.cpp
// Syntax:        C++11; humlib
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Add or remove stems.
//

#include "tool-autostem.h"
#include "HumRegex.h"
#include "Convert.h"

#include "string.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <sstream>

using namespace std;

namespace hum {

// START_MERGE


/////////////////////////////////
//
// Tool_autostem::Tool_autostem -- Set the recognized options for the tool.
//

Tool_autostem::Tool_autostem(void) {
	define("d|debug=b",       "Debugging information");
	define("r|remove=b",      "Remove stems");
	define("R|removeall=b",   "Remove all stems including explicit beams");
	define("o|overwrite|replace=b","Overwrite non-explicit stems in input");
	define("O|overwriteall|replaceall=b",  "Overwrite all stems in input");
	define("L|no-long|not-long|not-longs=b",
	       "Do not put stems one whole notes or breves");
	define("u|up=b",          "Middle note on staff has stem up");
	define("p|pos=b",         "Display only note vertical positions on staves");
	define("v|voice=b",       "Display only voice/layer information");
	define("author=b",        "Program author");
	define("version=b",       "Program version");
	define("example=b",       "Program examples");
	define("h|help=b",        "Short description");
}



/////////////////////////////////
//
// Tool_autostem::run -- Primary interfaces to the tool.
//

bool Tool_autostem::run(HumdrumFileSet& infiles) {
	bool status = true;
	for (int i=0; i<infiles.getCount(); i++) {
		status &= run(infiles[i]);
	}
	return status;
}


bool Tool_autostem::run(const string& indata, ostream& out) {
	HumdrumFile infile(indata);
	bool status = run(infile, out);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_autostem::run(HumdrumFile& infile, ostream& out) {
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}

//
// In-place processing of file:
//

bool Tool_autostem::run(HumdrumFile& infile) {
	initialize(infile);
	if (m_quit) {
		return true;
	}
	if (removeQ || overwriteQ) {
		removeStems(infile);
		if (removeQ) {
			infile.createLinesFromTokens();
			return true;
		}
	}
	autostem(infile);
	// Re-load the text for each line from their tokens.
	infile.createLinesFromTokens();
	return true;
}



//////////////////////////////
//
// Tool_autostem::initialize --
//

void Tool_autostem::initialize(HumdrumFile& infile) {
	// handle basic options:

	if (getBoolean("author")) {
		m_free_text << "Written by Craig Stuart Sapp, "
			  << "craig@ccrma.stanford.edu, December 2010" << endl;
		m_quit = true;
	} else if (getBoolean("version")) {
		m_free_text << getCommand() << ", version: 17 June 2019" << endl;
		m_free_text << "compiled: " << __DATE__ << endl;
		m_quit = true;
	} else if (getBoolean("help")) {
		usage();
		m_quit = true;
	} else if (getBoolean("example")) {
		example();
		m_quit = true;
	}

	debugQ        = getBoolean("debug");
	removeQ       = getBoolean("remove");
	removeallQ    = getBoolean("removeall");
	noteposQ      = getBoolean("pos");
	voiceQ        = getBoolean("voice");
	overwriteQ    = getBoolean("overwrite");
	overwriteallQ = getBoolean("overwriteall");
	notlongQ      = getBoolean("no-long");

	if (getBoolean("up")) {
		Middle = 4;
		Borderline = 1;
	}
	removeallQ = getBoolean("removeall");
	if (removeallQ) {
		removeQ = 1;
	}
	if (overwriteallQ) {
		overwriteQ = 1;
	}
}



//////////////////////////////////////////////////////////////////////////


//////////////////////////////
//
// Tool_autostem::removeStems --
//

void Tool_autostem::removeStems(HumdrumFile& infile) {
	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isData()) {
			continue;
		}
		for (int j=0; j<infile[i].getFieldCount(); j++) {
			if (!infile.token(i, j)->isKern()) {
				continue;
			}
			HumRegex hre;
			// string& buffer = static_cast<string&>(*infile.token(i, j));
			if (removeallQ || overwriteallQ) {
				hre.replaceDestructive(infile.token(i, j), "", "[\\\\/]x(?!x)", "g");
				hre.replaceDestructive(infile.token(i, j), "", "[\\\\/](?!x)", "g");
			} else {
				hre.replaceDestructive(infile.token(i, j), "", "[\\\\/](?!x)", "g");
			}
		}
	}
}



//////////////////////////////
//
// Tool_autostem::autostem -- add an up/down stem on notes in **kern data
//     that do not already have stem information.
//

void Tool_autostem::autostem(HumdrumFile& infile) {
	vector<vector<int> > baseline;
	getClefInfo(baseline, infile);

	// get staff-line position of all notes:
	vector<vector<vector<int> > > notepos;
	getNotePositions(notepos, baseline, infile);
	if (noteposQ) {
		printNotePositions(infile, notepos);
		return;
	}

	// get voice/layer number in track:
	vector<vector<int> > voice;
	getVoiceInfo(voice, infile);
	if (voiceQ) {
		printVoiceInfo(infile, voice);
		return;
	}

	// get stem directions:
	vector<vector<int> > stemdir;
	assignStemDirections(stemdir, voice, notepos, infile);
	insertStems(infile, stemdir);
}



//////////////////////////////
//
// Tool_autostem::insertStems -- put stem directions into the data.
//

void Tool_autostem::insertStems(HumdrumFile& infile,
		vector<vector<int> >& stemdir) {
	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isData()) {
			continue;
		}
		for (int j=0; j<infile[i].getFieldCount(); j++) {
			if (!infile.token(i, j)->isKern()) {
				continue;
			}
			setStemDirection(infile, i, j, stemdir[i][j]);
		}
	}
}



//////////////////////////////
//
// Tool_autostem::setStemDirection -- don't change stem direction if
//    there is already a stem on the note.
//

void Tool_autostem::setStemDirection(HumdrumFile& infile, int row, int col,
			int direction) {
	int& i = row;
	int& j = col;

	if (infile.token(i, j)->isNull()) {
		return;
	}
	if (infile.token(i, j)->isRest()) {
		return;
	}

	if (notlongQ) {
		// Don't print stems on whole notes and breves.
		// Duration units are in quarter notes.
		HumNum value = Convert::recipToDuration(*infile.token(row,col));
		double duration = value.getFloat();
		if ((duration >= 4.0) && (duration < 16.0)) {
			return;
		}
	}

	string buffer;
	string output;
	int tokencount = infile.token(i, j)->getSubtokenCount();
	for (int k=0; k<tokencount; k++) {
		buffer = infile.token(i, j, k);
		if ((!Convert::contains(buffer, '/')) &&
		    (!Convert::contains(buffer, '\\'))) {
			if (direction > 0) {
				addStem(buffer, "/");
			} else if (direction < 0) {
				addStem(buffer, "\\");
			}
		}
		output += buffer;
		output += ' ';
	}
	if (output.back() == ' ') {
		output.pop_back();
	}
	infile.token(i, j)->setText(output);
}



//////////////////////////////
//
// Tool_autostem::assignStemDirections --
//

void Tool_autostem::assignStemDirections(vector<vector<int> >& stemdir,
		vector<vector<int> > & voice,
		vector<vector<vector<int> > >& notepos, HumdrumFile& infile) {

	// the dimensions are not correct:
	stemdir.resize(infile.getLineCount());
	for (int i=0; i<(int)stemdir.size(); i++) {
		stemdir[i].resize(infile[i].getFieldCount());
		fill(stemdir[i].begin(), stemdir[i].end(), 0);
	}

	vector<int> maxlayer;
	getMaxLayers(maxlayer, voice, infile);

	assignBasicStemDirections(stemdir, voice, notepos, infile);

	vector<vector<string > > beamstates;
	getBeamState(beamstates, infile);

	vector<vector<Coord> > beamednotes;
	getBeamSegments(beamednotes, beamstates, infile, maxlayer);

	// print notes which are beamed together for debugging:

	if (debugQ) {
		for (int i=0; i<(int)beamednotes.size(); i++) {
			m_humdrum_text << "!! ";
			for (int j=0; j<(int)beamednotes[i].size(); j++) {
				m_humdrum_text << infile[beamednotes[i][j].i][beamednotes[i][j].j] << "\t";
			}
			m_humdrum_text << "\n";
		}
	}

	int direction;
	for (int i=0; i<(int)beamednotes.size(); i++) {
		direction = getBeamDirection(beamednotes[i], voice, notepos);
		setBeamDirection(stemdir, beamednotes[i], direction);
	}

	if (debugQ) {
		cerr << "STEM DIRECTION ASSIGNMENTS ==================" << endl;
		for (int i=0; i<(int)stemdir.size(); i++) {
			for (int j=0; j<(int)stemdir[i].size(); i++) {
				cerr << stemdir[i][j] << "\t";
			}
			cerr << endl;
		}
	}
}



//////////////////////////////
//
// Tool_autostem::setBeamDirection --
//

void Tool_autostem::setBeamDirection(vector<vector<int> >& stemdir,
		vector<Coord>& bnote, int direction) {
	int x;
	int i, j;
	for (x=0; x<(int)bnote.size(); x++) {
		i = bnote[x].i;
		j = bnote[x].j;
		stemdir[i][j] = direction;
	}
}




//////////////////////////////
//
// Tool_autostem::getBeamDirection -- return a consensus stem direction
//     for beamed notes.
//

int Tool_autostem::getBeamDirection(vector<Coord>& coords,
		vector<vector<int> >& voice, vector<vector<vector<int> > >& notepos) {

	// voice values are presumed to be 0 at the moment.

	int minn = 1000;
	int maxx = -1000;

	int x;
	int i, j, k;
	for (x=0; x<(int)coords.size(); x++) {
		i = coords[x].i;
		j = coords[x].j;
		if (voice[i][j] == 1) {
			return +1;
		}
		if (voice[i][j] == 2) {
			return -1;
		}
		for (k=0; k<(int)notepos[i][j].size(); k++) {
			if (minn > notepos[i][j][k]) {
				minn = notepos[i][j][k];
			}
			if (maxx < notepos[i][j][k]) {
				maxx = notepos[i][j][k];
			}
		}
	}

	if (maxx < 0 + Borderline) {
		// both minn and maxx are less than zero, so place stems up
		return +1;
	}
	if (minn > 0) {
		// both minn and maxx are greater than zero, so place stems down
		return -1;
	}

	if (abs(maxx) > abs(minn)) {
		// highest note is higher than lower note is lower, so place
		// stems down
		return -1;
	}
	if (abs(maxx) > abs(minn)) {
		// highest note is lower than lower note is lower, so place
		// stems up
		return +1;
	}

	// its a draw, so place stem up.
	return +1;
}



//////////////////////////////
//
// Tool_autostem::getBeamSegments -- arrange the beamed notes into
//     a long list with each entry being a list of notes containing one beam.
//     Each beamed note set should have their beams all pointing in the same
//     direction.
//

void Tool_autostem::getBeamSegments(vector<vector<Coord> >& beamednotes,
		vector<vector<string > >& beamstates, HumdrumFile& infile,
		vector<int> maxlayer) {
	beamednotes.clear();
	beamednotes.reserve(10000);
	vector<vector<vector<Coord> > > beambuffer;
	beambuffer.resize(infile.getMaxTrack() + 1);
	int i, j;
	for (i=0; i<(int)beambuffer.size(); i++) {
		beambuffer[i].resize(10); // layer  max 10, all more later if needed
		for (j=0; j<(int)beambuffer[i].size(); j++) {
			beambuffer[i][j].reserve(1000);
		}
	}

	Coord tcoord;
	char beamchar;
	int track, oldtrack, layer;
	for (i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isData()) {
			continue;
		}
		oldtrack = 0;
		layer = 0;
		for (j=0; j<infile[i].getFieldCount(); j++) {
			if (!infile.token(i, j)->isKern()) {
				continue;
			}
			track = infile.token(i, j)->getTrack();
			if (track == oldtrack) {
				layer++;
			} else {
				layer = 0;
			}
			oldtrack = track;
			if (infile.token(i, j)->isNull()) {
				continue;
			}
			if (infile.token(i, j)->isRest()) {
				continue;
			}

			if (beamstates[i][j].empty()) {
				beambuffer[track][layer].resize(0);  // possible unter. beam
				continue;
			}
			beamchar = beamstates[i][j][0];

			if ((beamchar == '[') || (beamchar == '=')) {
				// add a beam to the buffer and wait for more
				tcoord.i = i;
				tcoord.j = j;
				beambuffer[track][layer].push_back(tcoord);
				continue;
			}
			if (beamchar == ']') {
				// ending of a beam so store in permanent storage
				tcoord.i = i;
				tcoord.j = j;
				beambuffer[track][layer].push_back(tcoord);
				beamednotes.push_back(beambuffer[track][layer]);
				beambuffer[track][layer].resize(0);
			}
		}
	}
}



//////////////////////////////
//
// Tool_autostem::getMaxLayers --
//

void Tool_autostem::getMaxLayers(vector<int>& maxlayer,
		vector<vector<int> >& voice, HumdrumFile& infile) {

	int track;
	maxlayer.resize(infile.getMaxTrack() + 1);
	fill(maxlayer.begin(), maxlayer.end(), 0);
	int i, j;
	for (i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isData()) {
			continue;
		}
		for (j=0; j<infile[i].getFieldCount(); j++) {
			if (!infile.token(i, j)->isKern()) {
				continue;
			}
			if (infile.token(i, j)->isNull()) {
				continue;
			}
			if (infile.token(i, j)->isRest()) {
				continue;
			}
			track = infile.token(i, j)->getTrack();
			if (voice[i][j] + 1 > maxlayer[track]) {
				maxlayer[track] = voice[i][j] + 1;
			}
		}
	}
}



//////////////////////////////
//
// Tool_autostem::getVoiceInfo -- 0 = only voice in track, 1 = layer 1,
//     2 = layer 2, etc.
//
// 0 voices will be stemmed up or down based on vertical positions of notes
// 1 voices will be stemmed up always
// 2 voices will be stemmed down always.
// 3 and higher are still to be determined.
//
// Future enhancement of this algorithm: if one voice contains an invisible
// rest, then it will be ignored in the voice calculation.
//

void Tool_autostem::getVoiceInfo(vector<vector<int> >& voice,
		HumdrumFile& infile) {

	voice.resize(infile.getLineCount());

	int i, j, v;
	for (i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isData()) {
			continue;
		}
		voice[i].resize(infile[i].getFieldCount());
		fill(voice[i].begin(), voice[i].end(), -1);
		for (j=0; j<infile[i].getFieldCount(); j++) {
			if (!infile.token(i, j)->isKern()) {
				continue;
			}
			if (infile.token(i, j)->isNull()) {
				continue;
			}
			v = getVoice(infile, i, j);
			voice[i][j] = v;
		}
	}
}



//////////////////////////////
//
// Tool_autostem::printVoiceInfo --
//

void Tool_autostem::printVoiceInfo(HumdrumFile& infile,
		vector<vector<int> >& voice) {
	vector<string> voiceinfo(infile.getLineCount());
	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isData()) {
			continue;
		}
		for (int j=0; j<infile[i].getFieldCount(); j++) {
			if (!infile.token(i, j)->isKern()) {
				continue;
			}
			if (infile.token(i, j)->isNull()) {
				continue;
			}
			if (infile.token(i, j)->isRest()) {
				continue;
			}
			voiceinfo[i] += voice[i][j];
		}
		if (voiceinfo[i].back() == ' ') {
			voiceinfo[i].pop_back();
		}
	}
	infile.appendDataSpine(voiceinfo, "", "**voice");
}




//////////////////////////////
//
// Tool_autostem::printNotePositions -- prints the vertical position of notes on the
//    staves.  Mostly for debugging purposes.  A spine at the end of the
//    data will be added containing all positions for notes on the line
//    in the sequence in which the notes occur from left to right.
//
//    The middle line of a 5-line staff is the zero position, and
//    position values are diatonic steps above or below that level:
//
//    ===== +4
//          +3
//    ===== +2
//          +1
//    =====  0
//          -1
//    ===== -2
//          -3
//    ===== -4
//

void Tool_autostem::printNotePositions(HumdrumFile& infile,
		vector<vector<vector<int> > >& notepos) {
	vector<string> posinfo(infile.getLineCount());
	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isData()) {
			continue;
		}
		for (int j=0; j<infile[i].getFieldCount(); j++) {
			if (!infile.token(i, j)->isKern()) {
				continue;
			}
			if (infile.token(i, j)->isNull()) {
				// ignore null tokens
				continue;
			}
			if (infile.token(i, j)->isRest()) {
				// ignore rests
				continue;
			}
			for (int k=0; k<(int)notepos[i][j].size(); k++) {
				posinfo[i] += notepos[i][j][k];
			}
			if (posinfo[i].back() == ' ') {
				posinfo[i].pop_back();
			}
		}
	}
	infile.appendDataSpine(posinfo, "", "**vpos");
}



//////////////////////////////
//
// Tool_autostem::getNotePositions -- Extract the vertical position of the notes
// on the staves, with the centerline of the staff being the 0 position
// and each diatonic step equal to 1, so that lines of 5-lined staff are
// at positions from bottom to top: -4, -2, 0, +2, +4.
//

void Tool_autostem::getNotePositions(vector<vector<vector<int> > >& notepos,
		vector<vector<int> >& baseline, HumdrumFile& infile) {

	notepos.resize(infile.getLineCount());

	int location;
	string buffer;
	int i, j, k;
	int tokencount;

	for (i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isData()) {
			continue;
		}
		notepos[i].resize(infile[i].getFieldCount());
		for (j=0; j<infile[i].getFieldCount(); j++) {
			if (!infile.token(i, j)->isKern()) {
				continue;
			}
			if (infile.token(i, j)->isNull()) {
				// ignore null-tokens
				continue;
			}
			if (infile.token(i, j)->isRest()) {
				// ignore rests
				continue;
			}

			tokencount = infile.token(i, j)->getSubtokenCount();
			notepos[i][j].resize(tokencount);
			for (k=0; k<tokencount; k++) {
				buffer = infile.token(i, j)->getSubtoken(k);
				location = Convert::kernToBase7(buffer) -
						baseline[i][j] - 4;
				notepos[i][j][k] = location;
			}
		}
	}
}



//////////////////////////////
//
// Tool_autostem::processKernTokenStems --
//

void Tool_autostem::processKernTokenStems(HumdrumFile& infile,
		vector<vector<int> >& baseline, int row, int col) {
	exit(1);
}



//////////////////////////////
//
// Tool_autostem::assignBasicStemDirections -- don't take beams into
//     consideration.
//

void Tool_autostem::assignBasicStemDirections(vector<vector<int> >& stemdir,
		vector<vector<int> >& voice, vector<vector<vector<int> > >& notepos,
		HumdrumFile& infile) {

	int i, j;
	for (i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isData()) {
			continue;
		}
		for (j=0; j<infile[i].getFieldCount(); j++) {
			if (!infile.token(i, j)->isKern()) {
				continue;
			}
			if (infile.token(i, j)->isNull()) {
				continue;
			}
			if (infile.token(i, j)->isRest()) {
				continue;
			}
			if (removeQ) {
				removeStem2(infile, i, j);
			}

			if (Convert::contains(infile.token(i, j), '/')) {
				stemdir[i][j] = +1;
			} else if (Convert::contains(infile.token(i, j), '\\')) {
				stemdir[i][j] = -1;
			} else {
				stemdir[i][j] = determineChordStem(voice, notepos, infile, i, j);
			}
		}
	}
}



//////////////////////////////
//
// Tool_autostem::determineChordStem --
//

int Tool_autostem::determineChordStem(vector<vector<int> >& voice,
		vector<vector<vector<int> > >& notepos, HumdrumFile& infile, int row,
		int col) {

	if (notepos[row][col].size() == 0) {
		return 0;
	}

	if (voice[row][col] == 1) {
		return +1;
	}
	if (voice[row][col] == 2) {
		return -1;
	}
	if (voice[row][col] == 3) {
		return +1;
	}
	// voice == 0 means determine by vertical position

	if (notepos[row][col].size() == 1) {
		int location = notepos[row][col][0];
		if (location >= 0 + Borderline) {
			return -1;
		} else {
			return +1;
		}
	}

	// chord with more than one note so choose the extreme note as the
	// one which decides the direction

	int i;
	int minn = notepos[row][col][0];
	int maxx = notepos[row][col][0];
	for (i=1; i<(int)notepos[row][col].size(); i++) {
		if (minn > notepos[row][col][i]) {
			minn = notepos[row][col][i];
		}
		if (maxx < notepos[row][col][i]) {
			maxx = notepos[row][col][i];
		}
	}

	if (maxx < 0 + Borderline) {
		// all stems want to point upwards:
		return +1;
	}
	if (minn > 0) {
		// all stems want to point downwards:
		return -1;
	}

	if (abs(maxx) > abs(minn)) {
		return -1;
	}
	if (abs(minn) > abs(maxx)) {
		return +1;
	}

	return +1;
}



//////////////////////////////
//
// Tool_autostem::processKernTokenStemsSimpleModel --
//

void Tool_autostem::processKernTokenStemsSimpleModel(HumdrumFile& infile,
		vector<vector<int> >& baseline, int row, int col) {
	int& i = row;
	int& j = col;
	int tokencount = infile.token(i, j)->getSubtokenCount();

	HumNum duration;
	if (tokencount == 1) {
		duration = Convert::recipToDuration(*infile.token(i, j));
		if (duration >= 4) {
			// whole note or larger for note/chord, to not append a stem
			return;
		}
		if (Convert::contains(infile.token(i, j), '/')) {
			if (removeallQ || overwriteallQ) {
				if (Convert::contains(infile.token(i, j), "/x")) {
					if (Convert::contains(infile.token(i, j), "/xx")) {
						return;
					}
				} else if (Convert::contains(infile.token(i, j), "\\x")) {
					if (Convert::contains(infile.token(i, j), "\\xx")) {
						return;
					}
				}
			} else if (removeallQ || overwriteallQ) {
				removeStem2(infile, i, j);
			} else {
				// nothing to do
				return;
			}
		}
		if (infile.token(i, j)->isRest()) {
			// rest which does not have a stem
			return;
		}
	}

	if (removeQ) {
		removeStem2(infile, i, j);
	}

	int voice = getVoice(infile, row, col);
	int location;
	string buffer;
	string output;
	for (int k=0; k<tokencount; k++) {
		buffer = infile.token(i, j, k);
		if (i == 0) {
			duration = Convert::recipToDuration(buffer);
			// if (duration >= 4) {
			//    // whole note or larger for note/chord, do not append a stem
			//    return;
			// }
		}
		if (!(Convert::contains(infile.token(i, j), '/') ||
				Convert::contains(infile.token(i, j), '\\'))) {
			location = Convert::kernToBase7(buffer) -
					baseline[row][col] - Middle;
		  if (voice == 1) {
			  addStem(buffer, "/");
			} else if (voice == 2) {
				addStem(buffer, "\\");
			} else {
				addStem(buffer, location > 0 ? "\\" : "/");
			}
			output += buffer;
			output += ' ';
		} else {
			output += buffer;
			output += ' ';
		}
	}
	if (output.back() == ' ') {
		output.pop_back();
	}
	infile.token(i, j)->setText(output);
}



//////////////////////////////
//
// Tool_autostem::getVoice -- return 0 if the only spine in primary track, otherwise, return
// the nth column offset from 1 in the primary track.
//

int Tool_autostem::getVoice(HumdrumFile& infile, int row, int col) {
	int output = 0;
	int tcount = 0;
	int track = infile.token(row, col)->getTrack();
	int j;
	int testtrack;
	for (j=0; j<infile[row].getFieldCount(); j++) {
		testtrack = infile.token(row, j)->getTrack();
		if (testtrack == track) {
			tcount++;
		}
		if (col == j) {
			output = tcount;
		}
	}
	if (tcount == 1) {
		output = 0;
	}
	return output;
}



//////////////////////////////
//
// Tool_autostem::removeStem2 -- remove stem and any single x after the stem.
//

void Tool_autostem::removeStem2(HumdrumFile& infile, int row, int col) {
	HumRegex hre;
	hre.replaceDestructive(infile.token(row, col), "", "[\\\\/]x(?!x)", "g");
	hre.replaceDestructive(infile.token(row, col), "", "[\\\\/](?!x)", "g");
}



//////////////////////////////
//
// Tool_autostem::addStem --
//

void Tool_autostem::addStem(string& input, const string& piece) {
	string output;
	HumRegex hre;
	if (hre.search(input, "(.*[ABCDEFG][n#-]*)(.*)$", "i")) {
		output = hre.getMatch(1);
		output += piece;
		output += hre.getMatch(2);
	} else {
		output = input;
		output += piece;
	}
	input = output;
}



///////////////////////////////
//
// Tool_autostem::getClefInfo -- Identify the clef of each note in the score.
//     Does not consider the case where a primary track contains more
//     than one clef at a time (but that should not reasonable happen
//     in more scores).
//

void Tool_autostem::getClefInfo(vector<vector<int> >& baseline,
		HumdrumFile& infile) {
	vector<int> states(infile.getMaxTrack()+1,
			Convert::kernClefToBaseline("*clefG2"));
	baseline.resize(infile.getLineCount());

	int track;
	for (int i=0; i<infile.getLineCount(); i++) {
		if (infile[i].isInterpretation()) {
			for (int j=0; j<infile[i].getFieldCount(); j++) {
				if (!infile.token(i, j)->isKern()) {
					continue;
				}
				if (infile.token(i, j)->compare(0, 5, "*clef") == 0) {
					track = infile.token(i, j)->getTrack();
					states[track] = Convert::kernClefToBaseline(*infile.token(i, j));
				}
			}
		}
		if (!infile[i].isData()) {
			continue;
		}
		baseline[i].resize(infile[i].getFieldCount());
		for (int j=0; j<infile[i].getFieldCount(); j++) {
			if (!infile.token(i, j)->isKern()) {
				continue;
			}
			track = infile.token(i, j)->getTrack();
			baseline[i][j] = states[track];
		}
	}
}



//////////////////////////////
//
// Tool_autostem::example -- example function calls to the program.
//

void Tool_autostem::example(void) {
	m_error_text << getCommand() << " file.krn" << endl;
}



//////////////////////////////
//
// Tool_autostem::usage -- command-line usage description and brief summary
//

void Tool_autostem::usage(void) {
	m_error_text << "Usage: " << getCommand() << " [file(s)] " << endl;
}



//////////////////////////////
//
// Tool_autostem::getBeamState -- Analyze structure of beams and store note layout
//      directives at the same time.
//
// Type          Humdrum     MuseData
// start          L           [
// continue                   =
// end            J           ]
// forward hook   K           /
// backward hook  k           \  x
//

void Tool_autostem::getBeamState(vector<vector<string > >& beams,
		HumdrumFile& infile) {
	int len;
	int contin;
	int start;
	int stop;
	int flagr;
	int flagl;
	int track;
	HumNum rn;

	vector<vector<int> > beamstate;   // state of beams in tracks/layers
	vector<vector<int> > gracestate;  // independents state for grace notes

	string gbinfo;

	beamstate.resize(infile.getMaxTrack() + 1);
	gracestate.resize(infile.getMaxTrack() + 1);
	for (int i=0; i<(int)beamstate.size(); i++) {
		beamstate[i].resize(100);     // maximum of 100 layers in each track...
		gracestate[i].resize(100);    // maximum of 100 layers in each track...
		fill(beamstate[i].begin(), beamstate[i].end(), 0);
		fill(gracestate[i].begin(), gracestate[i].end(), 0);
	}

	beams.resize(infile.getLineCount());
	vector<int> curlayer;
	curlayer.resize(infile.getMaxTrack() + 1);
	vector<int> laycounter;

	for (int i=0; i<infile.getLineCount(); i++) {
		if (infile[i].isBarline()) {
			// don't allow beams across barlines.  Mostly for
			// preventing buggy beams from propagating...
			for (int t=1; t<=infile.getMaxTrack(); t++) {
				fill(beamstate[t].begin(), beamstate[t].end(), 0);
				fill(gracestate[t].begin(), gracestate[t].end(), 0);
			}
		}

		if (!infile[i].isData() && !infile[i].isBarline()) {
			continue;
		}

		if (!infile[i].isData()) {
			continue;
		}

		beams[i].resize(infile[i].getFieldCount());
		for (int j=0; j<(int)beams[i].size(); j++) {
			beams[i][j].resize(1);
			beams[i][j] = "";
		}

		fill(curlayer.begin(), curlayer.end(), 0);
		for (int j=0; j<infile[i].getFieldCount(); j++) {
			HTp token = infile.token(i, j);
			if (!token->isKern()) {
				continue;
			}
			track = token->getTrack();
			curlayer[track]++;
			if (token->isNull()) {
				continue;
			}
			if (infile.token(i, j)->isRest()) {
				// ignore rests.  Might be useful to not ignore
				// rests if beams extend over rests...
				continue;
			}
			rn = Convert::recipToDuration(*infile.token(i, j));
			if (rn >= 1) {
				beamstate[track][curlayer[track]] = 0;
				continue;
			}
			if (rn == 0) {
				// grace notes;
				countBeamStuff(infile.token(i, j)->c_str(), start, stop, flagr, flagl);
				if ((start != 0) && (stop != 0)) {
					cerr << "Funny error in grace note beam calculation" << endl;
					exit(1);
				}
				if (start > 7) {
					cerr << "Too many beam starts" << endl;
				}
				if (stop > 7) {
					cerr << "Too many beam ends" << endl;
				}
				if (flagr > 7) {
					cerr << "Too many beam flagright" << endl;
				}
				if (flagl > 7) {
					cerr << "Too many beam flagleft" << endl;
				}
				contin = gracestate[track][curlayer[track]];
				contin -= stop;
				gbinfo.clear();
				if (contin > 0) {
					gbinfo.resize(contin);
				}
				for (int ii=0; ii<contin; ii++) {
					gbinfo[ii] = '=';
				}
				if (start > 0) {
					for (int ii=0; ii<start; ii++) {
						gbinfo += "[";
					}
				} else if (stop > 0) {
					for (int ii=0; ii<stop; ii++) {
						gbinfo += "]";
					}
				}
				for (int ii=0; ii<flagr; ii++) {
					gbinfo += "/";
				}
				for (int ii=0; ii<flagl; ii++) {
					gbinfo += "\\";
				}
				len = (int)gbinfo.size();
				if (len > 6) {
					cerr << "Error too many grace note beams" << endl;
					exit(1);
				}
				beams[i][j] = gbinfo;
				gracestate[track][curlayer[track]] = contin;
				gracestate[track][curlayer[track]] += start;

			} else {
				// regular notes which are shorter than a quarter note
				// (including tuplet quarter notes which should be removed):

				countBeamStuff(infile.token(i, j)->c_str(), start, stop, flagr, flagl);
				if ((start != 0) && (stop != 0)) {
					cerr << "Funny error in note beam calculation" << endl;
					exit(1);
				}
				if (start > 7) {
					cerr << "Too many beam starts" << endl;
				}
				if (stop > 7) {
					cerr << "Too many beam ends" << endl;
				}
				if (flagr > 7) {
					cerr << "Too many beam flagright" << endl;
				}
				if (flagl > 7) {
					cerr << "Too many beam flagleft" << endl;
				}
				contin = beamstate[track][curlayer[track]];
				contin -= stop;
				gbinfo.resize(contin);
				for (int ii=0; ii<contin; ii++) {
					gbinfo[ii] = '=';
				}
				if (start > 0) {
					for (int ii=0; ii<start; ii++) {
						gbinfo += "[";
					}
				} else if (stop > 0) {
					for (int ii=0; ii<stop; ii++) {
						gbinfo += "[";
					}
				}
				for (int ii=0; ii<flagr; ii++) {
					gbinfo += "/";
				}
				for (int ii=0; ii<flagl; ii++) {
					gbinfo += "\\";
				}
				len = (int)gbinfo.size();
				if (len > 6) {
					cerr << "Error too many grace note beams" << endl;
					exit(1);
				}
				beams[i][j] = gbinfo;
				beamstate[track][curlayer[track]] = contin;
				beamstate[track][curlayer[track]] += start;
			}
		}
	}
}



//////////////////////////////
//
// Tool_autostem::countBeamStuff --
//

void Tool_autostem::countBeamStuff(const string& token, int& start, int& stop,
		int& flagr, int& flagl) {
	start = stop = flagr = flagl = 0;
	for (int i=0; i<(int)token.size(); i++) {
		switch (token[i]) {
			case 'L': start++;  break;
			case 'J': stop++;   break;
			case 'K': flagr++;  break;
			case 'k': flagl++;  break;
		}
	}
}


// END_MERGE

} // end namespace hum



