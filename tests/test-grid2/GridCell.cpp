//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Fri Nov 25 19:41:43 PST 2016
// Last Modified: Fri Nov 25 19:41:49 PST 2016
// Filename:      GridCell.cpp
// URL:           TBD
// Syntax:        C++11
// vim:           ts=3 noexpandtab
//
// Description:   Keep track of variables related to a single note
//                within a time slice (could be a note attack, note
//                sustain, or rest.

#include "humlib.h"
#include "GridCell.h"

using namespace std;


namespace hum {


//////////////////////////////
//
// GridCell::GridCell -- Constructor.
//

GridCell::GridCell(NoteGrid* owner, HTp token) {
	clear();
	m_owner = owner;
	m_token = token;
	calculateNumericPitches();
}



//////////////////////////////
//
// GridCell::clear -- Clear the contents of the object.
//

void GridCell::clear(void) {
	m_owner = NULL;
	m_token = NULL;
	m_b7  = 0;
	m_b12 = 0;
	m_b40 = 0;
	m_accidental = 0;
	m_nextAttackIndex = -1;
	m_prevAttackIndex = -1;
	m_currAttackIndex = -1;
	m_timeslice = -1;
	m_voice = -1;
}



//////////////////////////////
//
// GridCell::calculateNumericPitches -- Fills in b7, b12, and b40 variables.
//    0 = rest, negative values for sustained notes.
//

void GridCell::calculateNumericPitches(void) {
	if (!m_token) {
		m_b40        = 0;
		m_b12        = 0;
		m_b7         = 0;
		m_accidental = 0;
		return;
	}

	bool sustain = m_token->isNull() || m_token->isSecondaryTiedNote();
	if (m_token->isRest()) {
		m_b40 = 0;
	} else {
		m_b40 = Convert::kernToBase40(m_token->resolveNull());
		m_b40 = (sustain ? -m_b40 : m_b40);
	}

	// convert to base-7 (diatonic pitch numbers)
	if (m_b40 > 0) {
		m_b7         = Convert::base40ToDiatonic(m_b40);
		m_b12        = Convert::base40ToMidiNoteNumber(m_b40);
		m_accidental = Convert::base40ToAccidental(m_b40);
	} else if (m_b40 < 0) {
		m_b7         = -Convert::base40ToDiatonic(-m_b40);
		m_b12        = -Convert::base40ToMidiNoteNumber(-m_b40);
		m_accidental = -Convert::base40ToAccidental(-m_b40);
	} else {
		m_b7         = 0;
		m_b12        = 0;
		m_accidental = 0;
	}
}



//////////////////////////////
//
// GridCell::getKernPitch -- Return the **kern representation of the pitch.
//   Parentheses are placed around the pitch name if the GridCell is a
//   sustain.
//

string GridCell::getKernPitch(void) {
	if (m_b40 == 0) {
		return "r";
	}
	string pitch;
	if (m_b40 > 0) {
		pitch = Convert::base40ToKern(m_b40);
	} else {
		pitch = Convert::base40ToKern(-m_b40);
	}
	bool sustain = m_b40 < 0 ? true : false;
	if (sustain) {
		pitch.insert(0, "(");
		pitch += ")";
	}
	return pitch;
}



//////////////////////////////
//
// ostream<< -- print a GridCell (for debugging).
//

ostream& operator<<(ostream& out, GridCell* cell) {
	out << cell->getSliceIndex()      << "\t";
	out << cell->getKernPitch()       << "\t";
	out << cell->getPrevAttackIndex() << "\t";
	out << cell->getCurrAttackIndex() << "\t";
	out << cell->getNextAttackIndex() << "\t";
	out << cell->getDiatonicPitch()   << "\t";
	out << cell->getMidiPitch()       << "\t";
	out << cell->getBase40Pitch();
	out << endl;
	return out;
}


ostream& operator<<(ostream& out, GridCell& cell) {
	out << &cell;
	return out;
}


} // end namespace hum


