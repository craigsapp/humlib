//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Fri Nov 25 19:41:43 PST 2016
// Last Modified: Fri Nov 25 19:41:49 PST 2016
// Filename:      NoteCell.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/NoteCell.cpp
// Syntax:        C++11
// vim:           ts=3 noexpandtab
//
// Description:   Keep track of variables related to a single note
//                within a time slice (could be a note attack, note
//                sustain, or rest.

#include "NoteCell.h"
#include "NoteGrid.h"
#include "Convert.h"

using namespace std;

namespace hum {

// START_MERGE

//////////////////////////////
//
// NoteCell::NoteCell -- Constructor.
//

NoteCell::NoteCell(NoteGrid* owner, HTp token) {
	clear();
	m_owner = owner;
	m_token = token;
	calculateNumericPitches();
}



//////////////////////////////
//
// NoteCell::clear -- Clear the contents of the object.
//

void NoteCell::clear(void) {
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
// NoteCell::calculateNumericPitches -- Fills in b7, b12, and b40 variables.
//    0 = rest, negative values for sustained notes.
//

void NoteCell::calculateNumericPitches(void) {
	if (!m_token) {
		m_b40        = NAN;
		m_b12        = NAN;
		m_b7         = NAN;
		m_accidental = NAN;
		return;
	}

	bool sustain = m_token->isNull() || m_token->isSecondaryTiedNote();
	if (m_token->isRest()) {
		m_b40 = NAN;
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
		m_b7         = NAN;
		m_b12        = NAN;
		m_accidental = NAN;
	}
}



//////////////////////////////
//
// NoteCell::getSgnKernPitch -- Return the **kern representation of the pitch.
//   Parentheses are placed around the pitch name if the NoteCell is a
//   sustain.
//

string NoteCell::getSgnKernPitch(void) {
	if (isRest()) {
		return "r";
	}
	string pitch;
	pitch = Convert::base40ToKern(getAbsBase40Pitch());
	if (isSustained()) {
		pitch.insert(0, "(");
		pitch += ")";
	}
	return pitch;
}



//////////////////////////////
//
// NoteCell::getAbsKernPitch -- Return the **kern representation of the pitch.
//

string NoteCell::getAbsKernPitch(void) {
	if (isRest()) {
		return "r";
	}
	return Convert::base40ToKern(getAbsBase40Pitch());
}



//////////////////////////////
//
// NoteCell::isSustained --
//

bool NoteCell::isSustained(void) {
	if (m_b40 < 0) {
		return true;
	} else if (m_b40 > 0) {
		return false;
	}
	// calculate if rest is a "sustain" or an "attack"
	if (m_currAttackIndex == m_timeslice) {
		return false;
	} else {
		return true;
	}
}



//////////////////////////////
//
// NoteCell::operator- -- Calculate the diatonic interval between
//   two notes.  Maybe layter allow subtraction operator to deal
//   with base-12 and base-40 represnetation.
//

double NoteCell::operator-(NoteCell& B) {
	NoteCell& A = *this;
	return A.getAbsDiatonicPitch() - B.getAbsDiatonicPitch();
}


double NoteCell::operator-(int B) {
	NoteCell& A = *this;
	return A.getAbsDiatonicPitch() - B;
}



//////////////////////////////
//
// NoteCell::getLineIndex -- Returns the line index of the note in
//    the original file.
//

int NoteCell::getLineIndex(void) {
	if (!m_token) {
		return -1;
	}
	return m_token->getLineIndex();
}



//////////////////////////////
//
// NoteCell:printNoteInfo --
//

ostream& NoteCell::printNoteInfo(ostream& out) {
	out << getSliceIndex()       << "\t";
	out << getSgnKernPitch()     << "\t";
	out << getPrevAttackIndex()  << "\t";
	out << getCurrAttackIndex()  << "\t";
	out << getNextAttackIndex()  << "\t";
	out << getSgnDiatonicPitch() << "\t";
	out << getSgnMidiPitch()     << "\t";
	out << getSgnBase40Pitch();
	out << endl;
	return out;
}



//////////////////////////////
//
// NoteCell::getDiatonicIntervalFromPreviousAttack --
//

double NoteCell::getDiatonicIntervalFromPreviousAttack(void) {
	int previ = getPrevAttackIndex();
	if (previ < 0) {
		return NAN;
	}
	if (!m_owner) {
		return NAN;
	}
	return getAbsDiatonicPitch()
			- m_owner->cell(m_voice,previ)->getAbsDiatonicPitch();
}



//////////////////////////////
//
// NoteCell::getDiatonicIntervalFromNextAttack --
//

double NoteCell::getDiatonicIntervalToNextAttack(void) {
	int nexti = getNextAttackIndex();
	if (nexti < 0) {
		return NAN;
	}
	if (!m_owner) {
		return NAN;
	}
	return m_owner->cell(m_voice,nexti)->getAbsDiatonicPitch()
			- getAbsDiatonicPitch();
}



//////////////////////////////
//
// NoteCell::isRest --
//

bool NoteCell::isRest(void) {
	// bug in GCC requires :: prefix to resolve two different isnan defs.
	return ::isnan(m_b40);
}



//////////////////////////////
//
// NoteCell::getMetricLevel --
//

double NoteCell::getMetricLevel(void) {
	if (!m_owner) {
		return NAN;
	}
	return m_owner->getMetricLevel(getLineIndex());
}


// END_MERGE

} // end namespace hum



