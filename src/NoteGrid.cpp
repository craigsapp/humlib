//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Fri Nov 25 19:41:43 PST 2016
// Last Modified: Fri Nov 25 19:41:49 PST 2016
// Filename:      NoteGrid.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/NoteGrid.cpp
// Syntax:        C++11
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Manages a 2D array of NoteCells for each timeslice
//                in the Humdrum file score.
//

#include "NoteGrid.h"
#include "HumRegex.h"

using namespace std;

namespace hum {

// START_MERGE

//////////////////////////////
//
// NoteGrid::NoteGrid -- Constructor.
//

NoteGrid::NoteGrid(HumdrumFile& infile) {
	m_infile = NULL;
	load(infile);
}



//////////////////////////////
//
// NoteGrid::~NoteGrid -- Deconstructor.
//

NoteGrid::~NoteGrid() {
	clear();
}



//////////////////////////////
//
// NoteGrid::clear -- Deallocate storage and make the object empty.
//

void NoteGrid::clear(void) {
	m_infile = NULL;
	m_kernspines.clear();

	vector<vector<NoteCell* > >& grid = m_grid;
	for (int i=0; i<(int)grid.size(); i++) {
		for (int j=0; j<(int)grid[i].size(); j++) {
			if (grid[i][j]) {
				grid[i][j]->clear();
				delete grid[i][j];
				grid[i][j] = NULL;
			}
		}
		grid[i].clear();
	}
	grid.clear();
}



//////////////////////////////
//
// NoteGrid::getVoiceCount -- Return the number of voices/parts in the grid.
//

int NoteGrid::getVoiceCount(void) {
	return (int)m_grid.size();
}



//////////////////////////////
//
// NoteGrid::getSliceCount -- Return the number of time slices in the grid.
//     the grid.
//

int NoteGrid::getSliceCount(void) {
	if (m_grid.size() == 0) {
		return 0;
	} else {
		return (int)m_grid[0].size();
	}
}



//////////////////////////////
//
// NoteGrid::load -- Generate a two-dimensional list of notes
//     in a score.  Each row has at least one note attack, or an
//     empty data line in the Humdrum file will be skipped.
//

bool NoteGrid::load(HumdrumFile& infile) {
	// remove any previous contents:
	clear();

	m_infile = &infile;

	m_kernspines = infile.getKernSpineStartList();
	vector<HTp>& kernspines = m_kernspines;

	vector<int> metertops(infile.getMaxTrack() + 1, 0);
	vector<HumNum> meterbots(infile.getMaxTrack() + 1, 0);

	if (kernspines.size() == 0) {
		cerr << "Warning: no **kern spines in file" << endl;
		return false;
	}

	vector<vector<NoteCell* > >& grid = m_grid;
	grid.resize(kernspines.size());
	for (int i=0; i<(int)grid.size(); i++) {
		grid[i].reserve(infile.getLineCount());
	}

	int attack = 0;
	int track, lasttrack;
	vector<HTp> current;
	HumRegex hre;
	for (int i=0; i<infile.getLineCount(); i++) {
		if (infile[i].isInterpretation()) {
			for (int j=0; j<infile[i].getFieldCount(); j++) {
				if (!infile[i].token(j)->isKern()) {
					continue;
				}
				track = infile.token(i, j)->getTrack();
				if (hre.search(*infile.token(i, j), "\\*M(\\d+)/(\\d+)%(\\d+)")) {
					metertops[track] = hre.getMatchInt(1);
					meterbots[track] = hre.getMatchInt(2);
					meterbots[track] /= hre.getMatchInt(3);
				} else if (hre.search(*infile.token(i, j), "\\*M(\\d+)/(\\d+)")) {
					metertops[track] = hre.getMatchInt(1);
					meterbots[track] = hre.getMatchInt(2);
				} else {
					continue;
				}

			}
		}
		if (!infile[i].isData()) {
			continue;
		}
		track = 0;
		attack = 0;
		current.clear();
		for (int j=0; j<infile[i].getFieldCount(); j++) {
			lasttrack = track;
			track = infile.token(i, j)->getTrack();
			if (!infile[i].token(j)->isDataType("**kern")) {
				continue;
			}
			if (track == lasttrack) {
				// secondary voice: ignore
				continue;
			}
			current.push_back(infile.token(i, j));
			if (!(current.back()->isRest()
					|| current.back()->isSecondaryTiedNote())) {
				attack++;
			}
		}
		if (current.size() != kernspines.size()) {
			cerr << "Error: Unequal vector sizes " << current.size()
			     << " compared to " << kernspines.size() << endl;
			return false;
		}
		for (int j=0; j<(int)current.size(); j++) {
			NoteCell* cell = new NoteCell(this, current[j]);
			track = current[j]->getTrack();
			cell->setVoiceIndex(j);
			cell->setSliceIndex((int)grid[j].size());
			cell->setMeter(metertops[track], meterbots[track]);
			grid[j].push_back(cell);
		}
	}

	buildAttackIndexes();

	return true;
}



//////////////////////////////
//
// NoteGrid::cell -- Return the given cell in the grid.
//

NoteCell* NoteGrid::cell(int voiceindex, int sliceindex) {
	return m_grid.at(voiceindex).at(sliceindex);
}




//////////////////////////////
//
// NoteGrid::printDiatonicGrid --
//

void NoteGrid::printDiatonicGrid(ostream& output) {
	for (int j=0; j<getSliceCount(); j++) {
		for (int i=0; i<(int)getVoiceCount(); i++) {
			output << cell(i, j)->getSgnDiatonicPitch();
			if (i < getVoiceCount() - 1) {
				output << "\t";
			}
		}
		output << endl;
	}
}



//////////////////////////////
//
// NoteGrid::printMidiGrid --
//

void NoteGrid::printMidiGrid(ostream& output) {
	for (int j=0; j<getSliceCount(); j++) {
		for (int i=0; i<(int)getVoiceCount(); i++) {
			output << cell(i, j)->getSgnMidiPitch();
			if (i < getVoiceCount() - 1) {
				output << "\t";
			}
		}
		output << endl;
	}
}



//////////////////////////////
//
// NoteGrid::printBase40Grid --
//

void NoteGrid::printBase40Grid(ostream& output) {
	for (int j=0; j<getSliceCount(); j++) {
		for (int i=0; i<(int)getVoiceCount(); i++) {
			output << cell(i, j)->getSgnBase40Pitch();
			if (i < getVoiceCount() - 1) {
				output << "\t";
			}
		}
		output << endl;
	}
}



//////////////////////////////
//
// NoteGrid::printRawGrid --
//

void NoteGrid::printRawGrid(ostream& output) {
	for (int j=0; j<getSliceCount(); j++) {
		for (int i=0; i<(int)getVoiceCount(); i++) {
			output << cell(i, j)->getToken();
			if (i < getVoiceCount() - 1) {
				output << "\t";
			}
		}
		output << endl;
	}
}



//////////////////////////////
//
// NoteGrid::printKernGrid --
//

void NoteGrid::printKernGrid(ostream& output) {
	for (int j=0; j<getSliceCount(); j++) {
		for (int i=0; i<(int)getVoiceCount(); i++) {
			output << cell(i, j)->getSgnKernPitch();
			if (i < getVoiceCount() - 1) {
				output << "\t";
			}
		}
		output << endl;
	}
}



//////////////////////////////
//
// NoteGrid::buildAttackIndexes -- create forward and backward
//     note attack indexes for each cell.
//

void NoteGrid::buildAttackIndexes(void) {
	for (int i=0; i<(int)m_grid.size(); i++) {
		buildAttackIndex(i);
	}
}



//////////////////////////////
//
// NoteGrid::buildAttackIndex -- create forward and backward
//     note attack indexes for each cell in a single voice.
//

void NoteGrid::buildAttackIndex(int vindex) {
	vector<NoteCell*>& part = m_grid[vindex];

	// Set the slice index for the attack of the current note.  This
	// will be the same as the current slice if the NoteCell is an attack.
	// Otherwise if the note is a sustain, thie index will be set
	// to the slice of the attack correspinding to this NoteCell.
	// For rests, the first rest in a continuous sequence of rests
	// will be marked as the "attack" of the rest.
	NoteCell* currentcell = NULL;
	for (int i=0; i<(int)part.size(); i++) {
		if (i == 0) {
			part[0]->setCurrAttackIndex(0);
			continue;
		}
		if (part[i]->isRest()) {
			// This is a rest, so check for a rest sustain or start
			// of a rest sequence.
			if (part[i-1]->isRest()) {
				// rest "sustain"
				if (currentcell && !part[i]->getToken()->isNull()) {
					currentcell->m_tiedtokens.push_back(part[i]->getToken());
				}
				part[i]->setCurrAttackIndex(part[i-1]->getCurrAttackIndex());
			} else {
				// rest "attack";
				part[i]->setCurrAttackIndex(i);
			}
		} else if (part[i]->isAttack()) {
			part[i]->setCurrAttackIndex(i);
			currentcell = part[i];
		} else {
			// This is a sustain, so get the attack index of the
			// note from the previous slice index.
			part[i]->setCurrAttackIndex(part[i-1]->getCurrAttackIndex());
			if (currentcell && !part[i]->getToken()->isNull()) {
				currentcell->m_tiedtokens.push_back(part[i]->getToken());
			}
		}
	}

	// start with note attacks marked in the previous and next note slots:
	for (int i=0; i<(int)part.size(); i++) {
		if (part[i]->isAttack()) {
			part[i]->setNextAttackIndex(i);
			part[i]->setPrevAttackIndex(i);
		} else if (part[i]->isRest()) {
			if (part[i]->getCurrAttackIndex() == i) {
				part[i]->setNextAttackIndex(i);
				part[i]->setPrevAttackIndex(i);
			}
		}
	}

	// Go back and adjust the next note attack index:
	int value = -1;
	int temp  = -1;
	for (int i=(int)part.size()-1; i>=0; i--) {
		if (!part[i]->isSustained()) {
			temp = part[i]->getNextAttackIndex();
			part[i]->setNextAttackIndex(value);
			value = temp;
		} else {
			part[i]->setNextAttackIndex(value);
		}
	}

	// Go back and adjust the previous note attack index:
	value = -1;
	temp  = -1;
	for (int i=0; i<(int)part.size(); i++) {
		if (!part[i]->isSustained()) {
			temp = part[i]->getPrevAttackIndex();
			part[i]->setPrevAttackIndex(value);
			value = temp;
		} else {
			if (i != 0) {
				part[i]->setPrevAttackIndex(part[i-1]->getPrevAttackIndex());
			}
		}
	}

}



//////////////////////////////
//
// NoteGrid::getAbsDiatonicPitch -- Return the diatonic pitch number for
//     the given cell.
//

double NoteGrid::getAbsDiatonicPitch(int vindex, int sindex) {
	return m_grid.at(vindex).at(sindex)->getAbsDiatonicPitch();
}



//////////////////////////////
//
// NoteGrid::getSgnDiatonicPitch -- Return the diatonic pitch number for
//     the given cell.
//

double NoteGrid::getSgnDiatonicPitch(int vindex, int sindex) {
	return m_grid.at(vindex).at(sindex)->getSgnDiatonicPitch();
}



//////////////////////////////
//
// NoteGrid::getAbsMidiPitch -- Return the MIDI pitch number for
//     the given cell.
//

double NoteGrid::getAbsMidiPitch(int vindex, int sindex) {
	return m_grid.at(vindex).at(sindex)->getAbsMidiPitch();
}



//////////////////////////////
//
// NoteGrid::getSgnMidiPitch -- Return the MIDI pitch number for
//     the given cell.
//

double NoteGrid::getSgnMidiPitch(int vindex, int sindex) {
	return m_grid.at(vindex).at(sindex)->getSgnMidiPitch();
}



//////////////////////////////
//
// NoteGrid::getAbsBase40Pitch -- Return the base-40 pitch number for
//     the given cell.
//

double NoteGrid::getAbsBase40Pitch(int vindex, int sindex) {
	return m_grid.at(vindex).at(sindex)->getAbsBase40Pitch();
}



//////////////////////////////
//
// NoteGrid::getSgnBase40Pitch -- Return the base-40 pitch number for
//     the given cell.
//

double NoteGrid::getSgnBase40Pitch(int vindex, int sindex) {
	return m_grid.at(vindex).at(sindex)->getSgnBase40Pitch();
}



//////////////////////////////
//
// NoteGrid::getAbsKernPitch -- Return the **kern pitch name for
//     the given cell.  Sustained notes are enclosed in parentheses.
//

string NoteGrid::getAbsKernPitch(int vindex, int sindex) {
	return m_grid.at(vindex).at(sindex)->getAbsKernPitch();
}



//////////////////////////////
//
// NoteGrid::getSgnKernPitch -- Return the **kern pitch name for
//     the given cell.  Sustained notes are enclosed in parentheses.
//

string NoteGrid::getSgnKernPitch(int vindex, int sindex) {
	return m_grid.at(vindex).at(sindex)->getSgnKernPitch();
}



//////////////////////////////
//
// NoteGrid::getToken -- Return the HumdrumToken pointer for
//     the given cell.
//

HTp NoteGrid::getToken(int vindex, int sindex) {
	return m_grid.at(vindex).at(sindex)->getToken();
}



//////////////////////////////
//
// NoteGrid::getPrevAttackDiatonic -- Return the Diatonic note number
//     of the previous attacked note for the given cell.  Return 0 if
//     the previous note attack is a rest or there is no previous note
//     in the voice.
//

int NoteGrid::getPrevAttackDiatonic(int vindex, int sindex) {
	NoteCell*& cell = m_grid.at(vindex).at(sindex);
	int index = cell->getPrevAttackIndex();
	if (index < 0) {
		return 0;
	} else {
		return (int)this->cell(vindex, index)->getAbsDiatonicPitch();
	}
}



//////////////////////////////
//
// NoteGrid::getNextAttackDiatonic -- Return the Diatonic note number
//     of the next attacked note for the given cell.  Return 0 if
//     the next note attack is a rest or there is no next note
//     in the voice.
//

int NoteGrid::getNextAttackDiatonic(int vindex, int sindex) {
	NoteCell*& cell = m_grid.at(vindex).at(sindex);
	int index = cell->getNextAttackIndex();
	if (index < 0) {
		return 0;
	} else {
		return this->cell(vindex, index)->getAbsDiatonicPitch();
	}
}



//////////////////////////////
//
// NoteGrid::getLineIndex -- return the line index in the original
//    Humdrum data for the given slice index.
//

int NoteGrid::getLineIndex(int sindex) {
	if (m_grid.size() == 0) {
		return -1;
	}
	return m_grid.at(0).at(sindex)->getToken()->getLineIndex();
}



//////////////////////////////
//
// NoteGrid::getFieldIndex -- return the field index in the original
//    Humdrum data for the given slice index.
//

int NoteGrid::getFieldIndex(int sindex) {
	if (m_grid.size() == 0) {
		return -1;
	}
	return m_grid.at(0).at(sindex)->getToken()->getFieldIndex();
}



//////////////////////////////
//
// NoteGrid::getNoteAndRestAttacks -- Return the note attacks,
//    and the first rest slice ("rest attack") for a particular voice.
//

void NoteGrid::getNoteAndRestAttacks(vector<NoteCell*>& attacks,
		int vindex) {
	attacks.resize(0);
	int max = getSliceCount();
	if (max == 0) {
		return;
	}
	attacks.reserve(max);
	NoteCell* note = cell(vindex, 0);
	attacks.push_back(note);
	while (attacks.back()->getNextAttackIndex() > 0) {
		note = cell(vindex, attacks.back()->getNextAttackIndex());
		if (note == attacks.back()) {
			cerr << "Strange duplicate: ";
			note->printNoteInfo(cerr);
			break;
		}
		attacks.push_back(note);
	}
}



//////////////////////////////
//
// NoteGrid::getMetricLevel --
//

double NoteGrid::getMetricLevel(int sindex) {
	if (!m_infile) {
		return NAN;
	}
	if ((getSliceCount() == 0) || (getVoiceCount() == 0)) {
		return NAN;
	}
	if (m_metriclevels.empty()) {
		int track = 0;
		if ((getVoiceCount() > 0) && (getSliceCount() > 0)) {
			track = cell(0, 0)->getToken()->getTrack();
		}
		m_infile->getMetricLevels(m_metriclevels, track, NAN);
	}
	return m_metriclevels[sindex];
}



//////////////////////////////
//
// NoteGrid::getNoteDuration --
//

HumNum NoteGrid::getNoteDuration(int vindex, int sindex) {
	NoteCell* curnote = cell(vindex, sindex);
	int attacki = curnote->getCurrAttackIndex();
	int nexti   = curnote->getNextAttackIndex();
	HumNum starttime = 0;
	if (attacki >= 0) {
		starttime = cell(vindex, attacki)->getDurationFromStart();
	}
	HumNum endtime = m_infile->getScoreDuration();;
	if (nexti >= 0) {
		endtime = cell(vindex, nexti)->getDurationFromStart();
	}
	return endtime - starttime;
}



//////////////////////////////
//
// NoteGrid::printGridInfo -- for debugging.
//

void NoteGrid::printGridInfo(ostream& out) {
	for (int i=0; i<getVoiceCount(); i++) {
		printVoiceInfo(out, i);
		out << endl;
	}

}



//////////////////////////////
//
// NoteGrid::printVoiceInfo -- for debugging.
//

void NoteGrid::printVoiceInfo(ostream& out, int vindex) {
	out << "============================================================";
	out << endl;
	out << "i\tnote\tprevi\tcurri\tnexti\tb7\tmidi\tb40\n";
	for (int i=0; i<getSliceCount(); i++) {
		this->cell(vindex, i)->printNoteInfo(out);
	}
}


// END_MERGE

} // end namespace hum



