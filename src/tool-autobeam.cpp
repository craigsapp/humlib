//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Wed Nov 30 01:02:57 PST 2016
// Last Modified: Sun May 21 21:11:12 CEST 2017 Ignore non-kern spines when adding beams
// Filename:      tool-autobeam.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/tool-autobeam.cpp
// Syntax:        C++11; humlib
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Beam notes together by metric position.
//

#include "tool-autobeam.h"
#include "Convert.h"

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
	define("k|kern=i:0",           "process specific kern spine number");
	define("t|track=i:0",          "process specific track number");
	define("r|remove=b",           "remove all beams");
	define("o|overwrite=b",        "over-write existing beams");
	define("l|lyric|lyrics=b",     "break beam by lyric syllables");
	define("L|lyric-info=b",       "return the number of breaks needed");
	define("rest|include-rests=b", "include rests in beam edges");
}



/////////////////////////////////
//
// Tool_autobeam::run -- Primary interfaces to the tool.
//

bool Tool_autobeam::run(HumdrumFileSet& infiles) {
	bool status = true;
	for (int i=0; i<infiles.getCount(); i++) {
		status &= run(infiles[i]);
	}
	return status;
}


bool Tool_autobeam::run(const string& indata, ostream& out) {
	HumdrumFile infile(indata);
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_autobeam::run(HumdrumFile& infile, ostream& out) {
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

bool Tool_autobeam::run(HumdrumFile& infile) {
	initialize(infile);
	if (getBoolean("remove")) {
		removeBeams(infile);
	} else if (getBoolean("lyrics")) {
		breakBeamsByLyrics(infile);
	} else if (getBoolean("lyric-info")) {
		breakBeamsByLyrics(infile);
		m_free_text << m_splitcount << endl;
		return true;
	} else {
		addBeams(infile);
	}
	// Re-load the text for each line from their tokens.
	infile.createLinesFromTokens();
	return true;
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
// Tool_autobeam::breakBeamsByLyrics --
//


void Tool_autobeam::breakBeamsByLyrics(HumdrumFile& infile) {
	infile.analyzeNonNullDataTokens();
	int strands = infile.getStrandCount();
	int track;
	for (int i=0; i<strands; i++) {
		if (m_track > 0) {
			track = infile.getStrandStart(i)->getTrack();
			if (track != m_track) {
				continue;
			}
		}
		HTp starttok = infile.getStrandStart(i);
		if (!starttok->isKern()) {
			continue;
		}
		HTp curtok = starttok->getNextFieldToken();
		bool hastext = false;
		while (curtok && !curtok->isKern()) {
			if (curtok->isDataType("**text")) {
				hastext = true;
				break;
			}
			curtok = starttok->getNextFieldToken();
		}
		if (!hastext) {
			continue;
		}
		processStrandForLyrics(infile.getStrandStart(i), infile.getStrandEnd(i));
	}
}



//////////////////////////////
//
// Tool_autobeam::processStrandForLyrics --
//

void Tool_autobeam::processStrandForLyrics(HTp stok, HTp etok) {
	HTp current = stok;
	current = current->getNextNNDT();
	while (current && (current != etok)) {
		if (hasSyllable(current)) {
			splitBeam(current, stok, etok);
		}
		current = current->getNextNNDT();
	}
}



//////////////////////////////
//
// Tool_autobeam::splitBeam --
//

void Tool_autobeam::splitBeam(HTp tok, HTp stok, HTp etok) {
	HumNum duration = Convert::recipToDuration(tok);
	if (duration >= 1) {
		// Should not be beamed
		return;
	}
	vector<HTp> seq;
	getBeamedNotes(seq, tok, stok, etok);
	if (seq.size() == 0) {
		// note is larger than eighth note
		return;
	}
	if (seq.size() == 1) {
		// Single note with no beam possible
		return;
	}
	splitBeam2(seq, tok);
}



//////////////////////////////
//
// Tool_autobeam::splitBeam2 --
//

void Tool_autobeam::splitBeam2(vector<HTp>& group, HTp tok) {
	int target = -1;
	for (int i=0; i<group.size(); i++) {
		if (group[i] == tok) {
			target = i;
			break;
		}
	}

	if (target <= 0) {
		// problem or at start of beam, so do not modify beam.
		return;
	}

	m_splitcount++;
	if (group.size() <= 2) {
		// remove beam completely
		for (int i=0; i<(int)group.size(); i++) {
			string value = *group[i];
			string newvalue;
			for (int j=0; j<(int)value.size(); j++) {
				if ((value[j] == 'L') || (value[j] == 'J') || (toupper(value[j]) == 'K')) {
					continue;
				}
				newvalue += value[j];
			}
			group[i]->setText(newvalue);
		}
		return;
	}
	int lazyQ = isLazy(group);
	if (lazyQ) {
		splitBeamLazy(group, tok);
	} else {
		splitBeamNotLazy(group, tok);
	}
}



//////////////////////////////
//
// Tool_autobeam::splitBeamNotLazy --
//

void Tool_autobeam::splitBeamNotLazy(vector<HTp>& group, HTp tok) {
	int target = -1;

	for (int i=0; i<(int)group.size(); i++) {
		if (tok == group[i]) {
			target = i;
			break;
		}
	}
	if (target < 0) {
		return;
	}

	vector<int> sbeam(group.size(), 0);
	vector<int> ebeam(group.size(), 0);

	for (int i=0; i<(int)group.size(); i++) {
		string value = *group[i];
		int Lcount = 0;
		int Jcount = 0;
		for (int j=0; j<(int)value.size(); j++) {
			if (value[j] == 'L') {
				Lcount++;
			}
			if (value[j] == 'J') {
				Jcount++;
			}
		}
		sbeam[i] = Lcount;
		ebeam[i] = Jcount;
	}

	vector<int> sum(group.size(), 0);
	sum[0] = sbeam[0] - ebeam[0];
	for (int i=1; i<(int)sum.size(); i++) {
		sum[i] = sum[i-1] + sbeam[i] - ebeam[i];
	}
	vector<int> rsum(group.size(), 0);
	int rsize = (int)rsum.size();
	rsum[rsize - 1] = ebeam[rsize - 1] - sbeam[rsize - 1];
	for (int i=rsize-2; i>=0; i--) {
		rsum[i] = rsum[i+1] - sbeam[i] + ebeam[i];
	}

	if (target == 1) {
		// remove the first note from a beam group
		removeBeamCharacters(group[0]);
		string value = *group[1];
		for (int i=0; i<rsum[1]; i++) {
			value += 'L';
		}
		group[1]->setText(value);
	} else if (target == (int)group.size() - 1) {
		// remove the last note from the beam
		removeBeamCharacters(group[(int)group.size() - 1]);
		string value = *group[(int)group.size()-2];
		for (int i=0; i<sum[(int)group.size()-2]; i++) {
			value += 'J';
		}
		group[(int)group.size() - 2]->setText(value);
	} else {
		// split beam into two beams
		string value = *group[target];
		for (int i=0; i<rsum[target]; i++) {
			value += 'L';
		}
		group[target]->setText(value);

		value = *group[target-1];
		for (int i=0; i<sum[target-1]; i++) {
			value += 'J';
		}
		group[target-1]->setText(value);
	}
}



//////////////////////////////
//
// Tool_autobeam::splitBeamLazy -- Input will have more than two notes in the beam.
//

void Tool_autobeam::splitBeamLazy(vector<HTp>& group, HTp tok) {

	int target = -1;
	for (int i=0; i<(int)group.size(); i++) {
		if (tok == group[i]) {
			target = i;
			break;
		}
	}
	if (target < 0) {
		return;
	}
	if (target == 1) {
		// remove the first note from a beam group
		removeBeamCharacters(group[0]);
		string value = *group[1];
		value += 'L';
		group[1]->setText(value);
	} else if (target == (int)group.size() - 2) {
		// remove the last note from the beam
		removeBeamCharacters(group[(int)group.size() - 1]);
		string value = *group[(int)group.size()-2];
		value += 'J';
		group[(int)group.size() - 2]->setText(value);
	} else {
		// split beam into two beams
		string value = *group[target];
		value += 'L';
		group[target]->setText(value);
		value = *group[target-1];
		value += 'J';
		group[target-1]->setText(value);
	}

}



//////////////////////////////
//
// Tool_autobeam::removeBeamCharacters --
//

void Tool_autobeam::removeBeamCharacters(HTp token) {
	string value = *token;
	string newvalue;
	for (int i=0; i<(int)value.size(); i++) {
		if ((value[i] == 'L') || (value[i] == 'J') || (toupper(value[i]) == 'K')) {
			continue;
		}
		newvalue += value[i];
	}
	if (newvalue.size()) {
		token->setText(newvalue);
	} else {
		token->setText(".");
	}
}



//////////////////////////////
//
// Tool_autobeam::isLazy -- Return if just a single beam encoded
//   (even if should only have one beam).
//

bool Tool_autobeam::isLazy(vector<HTp>& group) {
	int Lcount = 0;
	int Jcount = 0;
	int Kcount = 0;
	for (int i=0; i<(int)group.size(); i++) {
		string value = *group[i];
		for (int j=0; j<(int)value.size(); j++) {
			if (value[j] == 'L') {
				Lcount++;
			} else if (value[j] == 'J') {
				Jcount++;
			} else if (toupper(value[j]) == 'K') {
				Kcount++;
			}
		}
	}
	if ((Lcount == 1) && (Jcount == 1) && (Kcount == 0)) {
		return true;
	}
	return false;
}



//////////////////////////////
//
// Tool_autobeam::getBeamedNotes --
//

void Tool_autobeam::getBeamedNotes(vector<HTp>& toks, HTp tok, HTp stok, HTp etok) {
	toks.resize(0);
	vector<HTp> backward;
	vector<HTp> forward;
	vector<HTp> seq;
	HTp current = tok;
	while (current && !current->isBarline()) {
		if (current->isNull()) {
			current = current->getNextToken();
			if (current && (current == etok)) {
				break;
			}
		}
		HumNum dur = Convert::recipToDuration(current);
		if (dur >= 1) {
			// No beams should be on this note
			break;
		}
		forward.push_back(current);
		current = current->getNextToken();
		if (current && (current == etok)) {
			break;
		}
	}

	current = tok->getPreviousToken();
	while (current && !current->isBarline()) {
		if (current->isNull()) {
			if (current == stok) {
				break;
			}
			current = current->getPreviousToken();
			if (current && (current == stok)) {
				break;
			}
			continue;
		}
		HumNum dur = Convert::recipToDuration(current);
		if (dur >= 1) {
			// No beams should be on this note
			break;
		}
		backward.push_back(current);
		if (current == stok) {
			break;
		}
		current = current->getPreviousToken();
	}

	seq.clear();
	for (int i=(int)backward.size() - 1; i>=0; i--) {
		seq.push_back(backward[i]);
	}
	for (int i=0; i<(int)forward.size(); i++) {
		seq.push_back(forward[i]);
	}

	if (seq.size() < 2) {
		// no beams possible
		return;
	}

	vector<int> sbeam(seq.size(), 0);
	vector<int> ebeam(seq.size(), 0);

	for (int i=0; i<(int)seq.size(); i++) {
		string value = *seq[i];
		int Lcount = 0;
		int Jcount = 0;
		for (int j=0; j<(int)value.size(); j++) {
			if (value[j] == 'L') {
				Lcount++;
			}
			if (value[j] == 'J') {
				Jcount++;
			}
		}
		sbeam[i] = Lcount;
		ebeam[i] = Jcount;
	}

	vector<int> sum(seq.size(), 0);
	sum[0] = sbeam[0] - ebeam[0];
	for (int i=1; i<(int)sum.size(); i++) {
		sum[i] = sum[i-1] + sbeam[i] - ebeam[i];
	}

	int target = -1;
	for (int i=0; i<(int)sum.size(); i++) {
		if (seq[i] == tok) {
			target = i;
			break;
		}
	}


	if ((target == 0) && (sum[0] == 0)) {
		// no beam on note
		return;
	}

	int sindex = -1;
	int eindex = -1;
	if (sum[target] == 0) {
		if (sum[target - 1] == 0) {
			// no beam on note
			return;
		} else {
			// There is a beam on target note and currently at
			// the end of the beam, so find the start:
			eindex = target;
			sindex = target;
			for (int i=target-1; i>=0; i--) {
				if (sum[i] != 0) {
					sindex = i;
				} else {
					break;
				}
			}
		}
	} else {
		// In the middle of a beam so expand outwards to find
		// the start and end of the beam group.
		for (int i=target; i>=0; i--) {
			if (sum[i] != 0) {
				sindex = i;
			} else {
				break;
			}
		}
		for (int i=target; i<(int)sum.size(); i++) {
			if (sum[i] == 0) {
				eindex = i;
				break;
			}
		}
	}

	if (eindex < 0) {
		// in case where beam is not closed properly, assume last note is beam end.
		eindex = (int)sum.size() - 1;
	}
	if (sindex < 0) {
		// in case where beam is not opened properly, assume last note is beam end.
		sindex = 0;
	}

	toks.clear();
	for (int i=sindex; i<=eindex; i++) {
		toks.push_back(seq[i]);
	}
}



//////////////////////////////
//
// Tool_autobeam::hasSyllable -- Only checking the first verse.
//

bool Tool_autobeam::hasSyllable(HTp token) {
	HTp current = token->getNextFieldToken();
	while (current && !current->isKern()) {
		if (current->isDataType("**text")) {
			if (current->isNull()) {
				return false;
			} else {
				return true;
			}
		}
		current = token->getNextFieldToken();
	}
	return false;
}



//////////////////////////////
//
// Tool_autobeam::addBeams --
//

void Tool_autobeam::addBeams(HumdrumFile& infile) {
	infile.analyzeNonNullDataTokens();
	int strands = infile.getStrandCount();
	int track;
	for (int i=0; i<strands; i++) {
		if (m_track > 0) {
			track = infile.getStrandStart(i)->getTrack();
			if (track != m_track) {
				continue;
			}
		}
		HTp starttok = infile.getStrandStart(i);
		if (!starttok->isKern()) {
			continue;
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
	m_splitcount = 0;
	m_kernspines = infile.getKernSpineStartList();
	vector<HTp>& ks = m_kernspines;
	m_timesigs.resize(infile.getTrackCount() + 1);
	for (int i=0; i<(int)ks.size(); i++) {
		infile.getTimeSigs(m_timesigs[ks[i]->getTrack()], ks[i]->getTrack());
	}
	m_overwriteQ = getBoolean("overwrite");
	m_track = getInteger("track");
	m_includerests = getBoolean("include-rests");
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
// Tool_autobeam::processStrand -- Add beams to a single strand.
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
// Tool_autobeam::processMeasure -- Need to deal with rests starting or ending
//    a beamed group.
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

	// Now identify notes that should be beamed together
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
	if (!startnote) {
		return;
	}
	if (!endnote) {
		return;
	}
	if (!m_includerests) {
		removeEdgeRests(startnote, endnote);
	}
	if (startnote == endnote) {
		// Nothing to do since only one note in beam.
		return;
	}
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



//////////////////////////////
//
// Tool_autobeam::removeEdgeRests -- If the regions to be beams contain
//     beams at the endpoints, shrink the beam until it finds notes.
//     If there are no notes, then set startnote and endnote both to NULL.
//

void Tool_autobeam::removeEdgeRests(HTp& startnote, HTp& endnote) {
	HTp current = startnote;

	int startindex = startnote->getLineIndex();
	int endindex = endnote->getLineIndex();


	if (startnote->isRest()) {
		current = current->getNextNNDT();
		while (current && current->isRest()) {
			if (current == endnote) {
				startnote = current;
				return;
			}
			current = current->getNextNNDT();
		}

		if (current->getLineIndex() >= endindex) {
			startnote = endnote;
			return;
		} else {
			startnote = current;
		}
	}

	if (endnote->isRest()) {
		HTp newcurrent = endnote;

		newcurrent = newcurrent->getPreviousNNDT();
		while (newcurrent && newcurrent->isRest()) {
			if (newcurrent == startnote) {
				endnote = newcurrent;
				return;
			}
			newcurrent = newcurrent->getPreviousNNDT();
		}

		if (newcurrent->getLineIndex() <= startindex) {
			endnote = startnote;
			return;
		} else {
			endnote = newcurrent;
		}
	}

}



// END_MERGE

} // end namespace hum



