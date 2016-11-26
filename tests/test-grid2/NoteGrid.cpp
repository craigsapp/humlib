//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Fri Nov 25 19:41:43 PST 2016
// Last Modified: Fri Nov 25 19:41:49 PST 2016
// Filename:      NoteGrid.cpp
// URL:           TBD
// Syntax:        C++11
// vim:           ts=3 noexpandtab
//
// Description:   Manages a 2D array of NoteCells for each timeslice
//                in the Humdrum file score.
//

#include "NoteGrid.h"

namespace hum {

//////////////////////////////
//
// NoteGrid::NoteGrid -- Constructor.
//

NoteGrid::NoteGrid(HumdrumFile& infile) {
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
	m_kernspines.clear();

	vector<vector<GridCell* > >& grid = m_grid;
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

	m_kernspines = infile.getKernSpineStartList();
	vector<HTp>& kernspines = m_kernspines;

	if (kernspines.size() == 0) {
		cerr << "Warning: no **kern spines in file" << endl;
		return false;
	}

	vector<vector<GridCell* > >& grid = m_grid;
	grid.resize(kernspines.size());
	for (int i=0; i<(int)grid.size(); i++) {
		grid[i].reserve(infile.getLineCount());
	}

	int attack = 0;
	int track, lasttrack;
	vector<HTp> current;
	for (int i=0; i<infile.getLineCount(); i++) {
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
		if (attack == 0) {
			continue;
		}
		if (current.size() != kernspines.size()) {
			cerr << "Error: Unequal vector sizes " << current.size()
			     << " compared to " << kernspines.size() << endl;
			return false;
		}
		for (int j=0; j<(int)current.size(); j++) {
			GridCell* cell = new GridCell(this, current[j]);
			cell->setVoiceIndex(j);
			cell->setSliceIndex((int)grid[j].size());
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

GridCell* NoteGrid::cell(int voiceindex, int sliceindex) {
	return m_grid.at(voiceindex).at(sliceindex);
}




//////////////////////////////
//
// NoteGrid::printDiatonicGrid --
//

void NoteGrid::printDiatonicGrid(ostream& output) {
	for (int j=0; j<getSliceCount(); j++) {
		for (int i=0; i<(int)getVoiceCount(); i++) {
			output << cell(i, j)->getDiatonicPitch();
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
			output << cell(i, j)->getMidiPitch();
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
			output << cell(i, j)->getBase40Pitch();
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
			output << cell(i, j)->getKernPitch();
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
	vector<GridCell*>& part = m_grid[vindex];

	// Set the slice index for the attack of the current note.  This
	// will be the same as the current slice if the NoteCell is an attack.
	// Otherwise if the note is a sustain, thie index will be set
	// to the slice of the attack correspinding to this NoteCell.
	// For rests, the first rest in a continuous sequence of rests
	// will be marked as the "attack" of the rest.
	for (int i=0; i<(int)part.size(); i++) {
		if (i == 0) {
			part[0]->setCurrAttackIndex(0);
			continue;
		}
		if (part[i]->getBase40Pitch() > 0) {
			part[i]->setCurrAttackIndex(i);
		} else if (part[i]->getBase40Pitch() < 0) {
			// This is a sustain, so get the attack index of the
			// note from the previous slice index.
			part[i]->setCurrAttackIndex(part[i-1]->getCurrAttackIndex());
		} else {
			// This is a rest, so check for a rest sustain or start
			// of a rest sequence.
			if (part[i-1]->getBase40Pitch() == 0) {
				// rest "sustain"
				part[i]->setCurrAttackIndex(part[i-1]->getCurrAttackIndex());
			} else {
				// rest "attack";
				part[i]->getCurrAttackIndex();
			}
		}
	}

	for (int i=0; i<(int)part.size(); i++) {
		if (part[i]->getBase40Pitch() > 0) {
			part[i]->setNextAttackIndex(i);
			part[i]->setPrevAttackIndex(i);
		} else if (part[i]->getBase40Pitch() == 0) {
			if (part[i]->getCurrAttackIndex() == i) {
				part[i]->setNextAttackIndex(i);
				part[i]->setPrevAttackIndex(i);
			}
		}
	}

	int value = -1;
	int temp  = -1;
	for (int i=(int)part.size()-1; i>=0; i--) {
		if (part[i]->getNextAttackIndex() >= 0) {
			temp = part[i]->getNextAttackIndex();
			part[i]->setNextAttackIndex(value);
			value = temp;
		} else {
			part[i]->setNextAttackIndex(value);
		}
	}

	value = -1;
	temp  = -1;
	for (int i=0; i<(int)part.size(); i++) {
		if (part[i]->getPrevAttackIndex() >= 0) {
			temp = part[i]->getPrevAttackIndex();
			part[i]->setPrevAttackIndex(value);
			value = temp;
		} else {
			part[i]->setPrevAttackIndex(value);
		}
	}

}



//////////////////////////////
//
// NoteGrid::getDiatonicPitch -- Return the diatonic pitch number for
//     the given cell.
//

int NoteGrid::getDiatonicPitch(int vindex, int sindex) {
	return m_grid.at(vindex).at(sindex)->getDiatonicPitch();
}



//////////////////////////////
//
// NoteGrid::getMidiPitch -- Return the MIDI pitch number for
//     the given cell.
//

int NoteGrid::getMidiPitch(int vindex, int sindex) {
	return m_grid.at(vindex).at(sindex)->getMidiPitch();
}



//////////////////////////////
//
// NoteGrid::getDiatonicPitch -- Return the base-40 pitch number for
//     the given cell.
//

int NoteGrid::getBase40Pitch(int vindex, int sindex) {
	return m_grid.at(vindex).at(sindex)->getBase40Pitch();
}



//////////////////////////////
//
// NoteGrid::getKernPitch -- Return the **kern pitch name for
//     the given cell.  Sustained notes are enclosed in parentheses.
//

string NoteGrid::getKernPitch(int vindex, int sindex) {
	return m_grid.at(vindex).at(sindex)->getKernPitch();
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
	GridCell*& cell = m_grid.at(vindex).at(sindex);
	int index = cell->getPrevAttackIndex();
	if (index < 0) {
		return 0;
	} else {
		return this->cell(vindex, index)->getDiatonicPitch();
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
	GridCell*& cell = m_grid.at(vindex).at(sindex);
	int index = cell->getNextAttackIndex();
	if (index < 0) {
		return 0;
	} else {
		return this->cell(vindex, index)->getDiatonicPitch();
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
// NoteGrid::printCellInfo -- for debugging.
//

void NoteGrid::printCellInfo(ostream& out) {
	for (int i=0; i<getVoiceCount(); i++) {
		printCellInfo(out, i);
		out << endl;
	}

}


void NoteGrid::printCellInfo(ostream& out, int vindex) {
	out << "============================================================";
	out << endl;
	out << "i\tnote\tprevi\tcurri\tnexti\tb7\tmidi\tb40\n";
	for (int i=0; i<getSliceCount(); i++) {
		out << cell(vindex, i);
	}
}


} // end namespace hum

