//
// class GridCell -- keep track of variables related to a single note
//    within a time slice (could be a note attack, note sustain, or rest.
//
// vim: ts=3

#ifndef _GRIDCELL_H
#define _GRIDCELL_H

#include "humlib.h"


namespace hum {

class NoteGrid;


class GridCell {
	public:
		       GridCell           (NoteGrid* owner, HTp token);
		      ~GridCell           (void) { clear();        }
		int    getDiatonicPitch   (void) { return m_b7;    }
		int    getMidiPitch       (void) { return m_b12;   }
		int    getBase40Pitch     (void) { return m_b40;   }
		string getKernPitch       (void);
		HTp    getToken           (void) { return m_token; }
		int    getNextAttackIndex (void) { return m_nextAttackIndex; }
		int    getPrevAttackIndex (void) { return m_prevAttackIndex; }
		int    getCurrAttackIndex (void) { return m_currAttackIndex; }
		int    getSliceIndex      (void) { return m_timeslice; }

	protected:
		void clear                  (void);
		void calculateNumericPitches(void);
		void setVoiceIndex          (int index) { m_voice = index; }
		void setSliceIndex          (int index) { m_timeslice = index; }
		void setNextAttackIndex     (int index) { m_nextAttackIndex = index; }
		void setPrevAttackIndex     (int index) { m_prevAttackIndex = index; }
		void setCurrAttackIndex     (int index) { m_currAttackIndex = index; }

	private:
		NoteGrid* m_owner; // the NoteGrid to which this cell belongs.
		HTp m_token;       // pointer to the note in the origina Humdrum file.
		int m_voice;       // index of the voice in the score the note belongs
		                   // 0=bottom voice (HumdrumFile ordering of parts)
		                   // column in NoteGrid.
		int m_timeslice;   // index for the row in NoteGrid.

		int m_b7;          // diatonic note number; 0=rest; negative=sustain.
		int m_b12;         // MIDI note number; 0=rest; negative=sustain.
		int m_b40;         // base-40 note number; 0=rest; negative=sustain.
		int m_accidental;  // chromatic alteration of a diatonic pitch.
		int m_nextAttackIndex; // index to next note attack (or rest),
		                       // -1 for undefined (interpred as rest).
		int m_prevAttackIndex; // index to previous note attack.
		int m_currAttackIndex; // index to current note attack (useful for
		                       // finding the start of a sustained note.

	friend NoteGrid;
};


} // end namespace hum


ostream& operator<<(ostream& out, hum::GridCell* cell);
ostream& operator<<(ostream& out, hum::GridCell& cell);


#endif /* _GRIDCELL_H */


