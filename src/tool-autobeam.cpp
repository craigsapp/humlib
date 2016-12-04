//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Wed Nov 30 01:02:57 PST 2016
// Last Modified: Fri Dec  2 03:45:34 PST 2016
// Filename:      tool-autobeam.cpp
// URL:           https://github.com/craigsapp/minHumdrum/blob/master/src/tool-autobeam.cpp
// Syntax:        C++11
// vim:           ts=3 noexpandtab
//
// Description:   Example of extracting a 2D pitch grid from
//                a score for dissonance analysis.
//

#include "tool-autobeam.h"

#include <algorithm>
#include <cmath>

using namespace std;

namespace hum {

// START_MERGE


/////////////////////////////////
//
// Tool_autobeam::Tool_autobeam -- Set the recognized options for the tool.
//

Tool_autobeam::Tool_autobeam(void) {
	define("k|kern=i:0",    "process specific kern spine number");
	define("t|track=i:0",   "process specific track number");
	define("r|remove=b",    "remove all beams");
	define("o|overwrite=b", "over-write existing stems");
}



/////////////////////////////////
//
// Tool_autobeam::run -- Do the main work of the tool.
//

bool Tool_autobeam::run(const string& indata, ostream& out) {
	HumdrumFile infile(indata);
	return run(infile, out);
}


bool Tool_autobeam::run(HumdrumFile& infile, ostream& out) {
	initialize(infile);
	if (getBoolean("remove")) {
		removeBeams(infile);
	} else {
		addBeams(infile);
	}
	infile.createLinesFromTokens();
	out << infile;
	return 1;
}


//////////////////////////////
//
// Tool_autobeam::removeBeams --
//

void Tool_autobeam::removeBeams(HumdrumFile& infile) {
	int strands = infile.getStrandCount();
	HTp endtok;
	HTp starttok;
	HTp token;
	int track;
	bool bfound = false;
	string newstr;
	for (int i=0; i<strands; i++) {
		if (m_track > 0) {
			track = infile.getStrandStart(i)->getTrack();
			if (track != m_track) {
				continue;
			}
		}
		starttok = infile.getStrandStart(i);

		if (!starttok->isKern()) {
			continue;
		}
		endtok   = infile.getStrandEnd(i);
		token    = starttok;

		while (token && (token != endtok)) {
			if (!token->isData()) {
				token = token->getNextToken();
				continue;
			}
			if (token->isNull()) {
				token = token->getNextToken();
				continue;
			}

			bfound = false;
			newstr.clear();
			for (int i=0; i<(int)((string)(*token)).size(); i++) {
				switch (((string)(*token))[i]) {
					case 'L':
					case 'J':
					case 'K':
					case 'k':
						bfound = true;
						break;
					default:
						newstr += ((string)(*token))[i];
				}
			}
			if (bfound) {
				((string)(*token))[0] = 'X';
				token->swap(newstr);
			}
			token = token->getNextToken();
		}
	}
}



//////////////////////////////
//
// Tool_autobeam::addBeams --
//

void Tool_autobeam::addBeams(HumdrumFile& infile) {
	int strands = infile.getStrandCount();
	int track;
	for (int i=0; i<strands; i++) {
		if (m_track > 0) {
			track = infile.getStrandStart(i)->getTrack();
			if (track != m_track) {
				continue;
			}
		}
		processStrand(infile.getStrandStart(i), infile.getStrandEnd(i));
	}
}



//////////////////////////////
//
// Tool_autobeam::initialize -- extract time signature lines for
//    each **kern spine in file.
//

void Tool_autobeam::initialize(HumdrumFile& infile) {
	m_kernspines = infile.getKernSpineStartList();
	vector<HTp>& ks = m_kernspines;
	m_timesigs.resize(infile.getTrackCount() + 1);
	for (int i=0; i<(int)ks.size(); i++) {
		infile.getTimeSigs(m_timesigs[ks[i]->getTrack()], ks[i]->getTrack());
	}
	m_overwriteQ = getBoolean("overwrite");
	m_track = getInteger("track");
	if ((m_track == 0) && getBoolean("kern")) {
		int ks = getInteger("kern") - 1;
		vector<HTp> kernspines = infile.getKernSpineStartList();
		if ((ks >= 0) && (ks <(int)kernspines.size())) {
			m_track = kernspines[ks]->getTrack();
		}
	}
}



//////////////////////////////
//
// Tool_autobeam::processStrand --
//

void Tool_autobeam::processStrand(HTp strandstart, HTp strandend) {
	HTp token = strandstart;
	vector<HTp> measure;
	while (token && (token != strandend)) {
		if (token->isBarline()) {
			processMeasure(measure);
			measure.clear();
			token = token->getNextToken();
			continue;
		}
		if (!token->isData()) {
			token = token->getNextToken();
			continue;
		}
		if (token->isNull()) {
			token = token->getNextToken();
			continue;
		}
		measure.push_back(token);
		token = token->getNextToken();
	}

}



//////////////////////////////
//
// Tool_autobeam::processMeasure --
//

void Tool_autobeam::processMeasure(vector<HTp>& measure) {
	if (measure.empty()) {
		return;
	}

	vector<HumNum> beatsize;
	vector<HumNum> beatpos;
	vector<HumNum> notedurnodots;

	// default beat duration is a quarter note.
	pair<int, HumNum> current;
	current.first = 1;
	current.second = 4;
	HumNum beatdur(1);

	// First, get the beat positions of all notes in the measure:
	vector<pair<int, HumNum> >& timesig = m_timesigs[measure[0]->getTrack()];
	for (int i=0; i<(int)measure.size(); i++) {
		int line = measure[i]->getLineIndex();
		if ((current.first != timesig[line].first) ||
		    (current.second != timesig[line].second)) {
			current = timesig[line];
			beatdur = 1;
			beatdur /= current.second;
			beatdur *= 4; // convert to quarter-notes units from whole-notes.
			if ((current.first % 3 == 0) && (current.first != 3)) {
				// compound meter, so shift the beat to 3x the demoniator
				beatdur *= 3;
			} else if (current.first == 3 && (current.second > 4)) {
				// time signatures such as 3/8 and 3/16 which should
				// beam together at the measure level (3/4 not included).
				beatdur *= 3;
			}
		}
		beatsize.push_back(beatdur);
		notedurnodots.push_back(measure[i]->getDurationNoDots());
		beatpos.push_back(measure[i]->getDurationFromBarline() / beatdur);
	}

	// Now identify notes which should be beamed together
	// (using lazy beaming for now).
	HumNum eighthnote(1, 2);
	int beat1;
	int beat2;
	#define INVALID -1000000
	int beamstart = INVALID;
	for (int i=0; i<(int)measure.size(); i++) {
		if (notedurnodots[i] > eighthnote) {
			// note does not need a beam, but first check if an open
			// beam should be closed:
			if ((beamstart >= 0) && (i - beamstart > 1)) {
				addBeam(measure[beamstart], measure[i-1]);
				beamstart = INVALID;
				continue;
			}
			beamstart = INVALID;
			continue;
		}

		if (beamstart == INVALID) {
			// possible beam start
			beamstart = i;
			beat1 = (int)beatpos[i].getFloat();
			continue;
		}

		beat2 = (int)beatpos[i].getFloat();
		if (beat1 == beat2) {
			// note should be added to current beam, but could
			// be beams to notes after it, so continue on to next note.
			continue;
		} else {
			// check if previous beam should be ended.
			if ((beamstart >= 0) && (i - beamstart > 1)) {
				addBeam(measure[beamstart], measure[i-1]);
				beamstart = i;
				beat1 = (int)beatpos[i].getFloat();
				continue;
			}
			beamstart = i;
			beat1 = (int)beatpos[i].getFloat();
		}

		// Note should not be attached to previous beam.

		// The current note should not be beamed to current note,
		// but perhaps will be beams to the following notes.
		beamstart = i;
		beat1 = (int)beatpos[i].getFloat();
	}

	// close the last beam
	if ((beamstart >= 0) && ((int)measure.size() - beamstart > 1)) {
		addBeam(measure[beamstart], measure[measure.size()-1]);
		beamstart = INVALID;
	}
}



//////////////////////////////
//
// Tool_autobeam::addBeam -- Lazy beaming for now.
//

void Tool_autobeam::addBeam(HTp startnote, HTp endnote) {
	if (!m_overwriteQ) {
		HTp token = startnote;
		while (token && (token != endnote)) {
			if (token->hasBeam()) {
				return;
			}
			token = token->getNextToken();
		}
	}
	startnote->push_back('L');
	endnote->push_back('J');
}


// END_MERGE

} // end namespace hum



