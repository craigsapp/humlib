//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Fri Feb  9 21:05:41 PST 2018
// Last Modified: Fri Feb  9 21:05:45 PST 2018
// Filename:      tool-chord.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/tool-chord.cpp
// Syntax:        C++11
// vim:           ts=3 noexpandtab
//
// Description:   Interface for fixing adjusting chords (order of pitches,
//                rhythms, articulatios).
//

#include "tool-chord.h"
#include "Convert.h"
#include "HumRegex.h"


using namespace std;

namespace hum {

// START_MERGE


/////////////////////////////////
//
// Tool_chord::Tool_chord -- Set the recognized options for the tool.
//

Tool_chord::Tool_chord(void) {
	// add options here
	define("u|sort-upwards=b",    "sort notes by lowest first in chord");
	define("d|sort-downwards=b",  "sort notes by highest first in chord");
	define("t|top-note=b",        "extract top note of chords");
	define("b|bottom-note=b",     "extract bottom note of chords");
	define("f|first-note=b",      "extract first note of chords");
	define("p|primary=b",         "place prefix/suffix/beams on first note in chord");
	define("l|last-note=b",       "extract last note of chords");
	define("s|spine=i:-1",        "spine to process (indexed from 1)");
	define("m|minimize=b",        "minimize chords");
	define("M|maximize=b",        "maximize chords");
}



/////////////////////////////////
//
// Tool_chord::run -- Do the main work of the tool.
//

bool Tool_chord::run(const string& indata, ostream& out) {
	HumdrumFile infile(indata);
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_chord::run(HumdrumFile& infile, ostream& out) {
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_chord::run(HumdrumFile& infile) {
	initialize();
	processFile(infile, m_direction);
	infile.createLinesFromTokens();
	return true;
}



//////////////////////////////
//
// Tool_chord::initialize --
//

void Tool_chord::initialize(void) {
	m_direction = 1;
	if (getBoolean("sort-upwards")) {
		m_direction = -1;
	}
	if (getBoolean("sort-downwards")) {
		m_direction = +1;
	}
	m_spine = getInteger("spine");
	m_primary = getBoolean("primary");
	if (getBoolean("minimize")) {
		m_primary = true;
	}
}



//////////////////////////////
//
// Tool_chord::processFile --
//     direction:  1 = highest first
//     direction: -1 = lowest first
//

void Tool_chord::processFile(HumdrumFile& infile, int direction) {
	if (!(getBoolean("top-note") || getBoolean("bottom-note") ||
			getBoolean("sort-upwards") || getBoolean("sort-downwards") ||
			getBoolean("minimize") || getBoolean("maximize") ||
			getBoolean("first-note") || getBoolean("last-note"))) {
		// nothing to do
		return;
	}

	HumRegex hre;
	for (int i=0; i<infile.getStrandCount(); i++) {
		HTp stok = infile.getStrandStart(i);
		int track = stok->getTrack();
		if ((m_spine > 0) && (track != m_spine)) {	
			continue;
		}
		if (!stok->isKern()) {
			continue;
		}
		HTp etok = infile.getStrandEnd(i);
		HTp tok = stok;
		while (tok && (tok != etok)) {
			if (!tok->isData()) {
				tok = tok->getNextToken();
				continue;
			}
			if (tok->isNull()) {
				tok = tok->getNextToken();
				continue;
			}
			if (!tok->isChord()) {
				tok = tok->getNextToken();
				continue;
			}
			processChord(tok, direction);
			tok = tok->getNextToken();
		}
	}
}



//////////////////////////////
//
// Tool_chord::processChord --
//

void Tool_chord::processChord(HTp tok, int direction) {
	vector<string> notes;
	int count = (int)tok->getSubtokenCount();
	for (int i=0; i<count; i++) {
		notes.emplace_back(tok->getSubtoken(i));
	}

	if (notes.size() <= 1) {
		// nothing to do
		return;
	}

	bool ismin = false;
	HumRegex hre;
	if (!hre.search(notes[1], "[0-9]")) {
		ismin = true;
	}

	vector<pair<int, int>> pitches(count);
	for (int i=0; i<(int)pitches.size(); i++) {
		pitches[i].first = Convert::kernToBase40(notes[i]);
		pitches[i].second = i;
	}

	if (ismin && (getBoolean("top-note") || getBoolean("bottom-note") || getBoolean("last-note"))) {
		maximizeChordPitches(notes, pitches);
	}

	if (getBoolean("top-note")) {
		direction = -1;
	}
	if (getBoolean("bottom-note")) {
		direction = -1;
	}
	if (getBoolean("first-note")) {
		direction = 0;
	}
	if (getBoolean("last-note")) {
		direction = 0;
	}

	if (direction > 0) {
		sort(pitches.begin(), pitches.end(), 
			[](const pair<int, int>& a, const pair<int, int>& b) -> bool {
				return a.first > b.first;
			});
	} else if (direction < 0) {
		sort(pitches.begin(), pitches.end(), 
			[](const pair<int, int>& a, const pair<int, int>& b) -> bool {
				return a.first < b.first;
			});
	}

	bool same = true;
	for (int i=1; i<(int)pitches.size(); i++) {
		if (pitches[i].second != pitches[i-1].second + 1) {
			same = false;
			break;
		}
	}
	if ((getBoolean("sort-upwards") || getBoolean("sort-downwards")) && same) {
		return;
	}

	string prefix;
	string suffix;

	if (hre.search(notes[0], "(&*\\([<>]?)")) {
		prefix = hre.getMatch(1);
		hre.replaceDestructive(notes[0], "", "(&*\\([<>]?)");
	}

	string beam;
	int beamindex = -1;

	for (int i=0; i<(int)notes.size(); i++) {
		if (hre.search(notes[i], "([LJkK]+[<>]?)")) {
			beamindex = i;
			beam = hre.getMatch(1);
			hre.replaceDestructive(notes[i], "", "([LJkK]+[<>]?)");
		}
	}

	if (getBoolean("maximize") && (beamindex >= 0)) {
		beamindex = (int)notes.size() - 1;
	}
	
	if (hre.search(notes.back(), "(&*\\)[<>]?)")) {
		suffix += hre.getMatch(1);
		hre.replaceDestructive(notes.back(), "", "(&*\\)[<>]?)");
	} else if (ismin || getBoolean("maximize")) {
		if (hre.search(notes[0], "(&*\\)[<>]?)")) {
			suffix += hre.getMatch(1);
			hre.replaceDestructive(notes[0], "", "(&*\\)[<>]?)");
		}
	}

	if (getBoolean("minimize")) {
		minimizeChordPitches(notes, pitches);
	} else if (getBoolean("maximize")) {
		maximizeChordPitches(notes, pitches);
	}

	string output = prefix;
	if (getBoolean("top-note")) {
		output += notes[pitches.back().second];
		output += beam;
	} else if (getBoolean("bottom-note")) {
		output += notes[pitches[0].second];
		output += beam;
	} else if (getBoolean("first-note")) {
		output += notes[pitches[0].second];
		output += beam;
	} else if (getBoolean("last-note")) {
		output += notes[pitches.back().second];
		output += beam;
	} else {
		for (int i=0; i<(int)pitches.size(); i++) {
			output += notes[pitches[i].second];
			if (m_primary && (i==0)) {
				output += beam;
				output += suffix;
			} else if ((!m_primary) && (beamindex == i)) {
				output += beam;
			}
			if (i < (int)pitches.size() - 1) {
				output += " ";
			}
		}
	}

	if (!m_primary) {
		output += suffix;
	}
	tok->setText(output);
}


//////////////////////////////
//
// Tool_chord::minimizeChordPitches -- remove durations, articulations
//   and stem directions for secondary notes in chord.
//		pitches[x].first = base40 pitch.
//		pitches[x].second = index for pitch in notes vector.
//

void Tool_chord::minimizeChordPitches(vector<string>& notes, 
		vector<pair<int,int>>& pitches) {
	if (notes.empty()) {
		return;
	}
	HumRegex hre;
	string firstdur;
	string firstartic;
	string firststem;
	if (hre.search(notes[pitches[0].second], "([0-9%.]+)")) {
		firstdur = hre.getMatch(1);
	}
	if (hre.search(notes[pitches[0].second], "([\\\\/])")) {
		firststem = hre.getMatch(1);
		hre.replaceDestructive(firststem, "\\\\", "\\", "g");
	}

	for (int i=1; i<(int)pitches.size(); i++) {
		hre.replaceDestructive(notes[pitches[i].second], "", firstdur);
		hre.replaceDestructive(notes[pitches[i].second], "", firststem);

		// articulations:
		hre.replaceDestructive(notes[pitches[i].second], "", "'[<>]?");
		hre.replaceDestructive(notes[pitches[i].second], "", "~[<>]?");
		hre.replaceDestructive(notes[pitches[i].second], "", "\\^[<>]?");
	}
}



//////////////////////////////
//
// Tool_chord::maximizeChordPitches -- add durations, articulations
//   and stem directions to all secondary notes in chord.
//

void Tool_chord::maximizeChordPitches(vector<string>& notes, 
		vector<pair<int,int>>& pitches) {
	if (notes.empty()) {
		return;
	}
	HumRegex hre;

	string prefix;
	string suffix;

	if (hre.search(notes[0], "(.*?)(?=[A-Ga-g])")) {
		prefix = hre.getMatch(1);
	}
	if (hre.search(notes[0], "([A-Ga-g]+[#n-]*[<>]?)(.*)")) {
		suffix = hre.getMatch(2);
	}

	for (int i=1; i<(int)notes.size(); i++) {
		notes[i] = prefix + notes[i] + suffix;
	}
}



// END_MERGE

} // end namespace hum



