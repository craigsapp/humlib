//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Aug  8 12:24:49 PDT 2015
// Last Modified: Thu Nov 24 08:31:41 PST 2016 Added null token resolving
// Filename:      HumdrumToken.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/HumdrumToken.h
// Syntax:        C++11
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Keep track of variables related to a single note
//                within a time slice (could be a note attack, note
//                sustain, or rest.
//

#ifndef _NOTECELL_H_INCLUDED
#define _NOTECELL_H_INCLUDED

#include "HumdrumFile.h"

#include <cmath>
#include <iostream>
#include <string>
#include <vector>

namespace hum {

// START_MERGE

#define GRIDREST NAN

class NoteGrid;


class NoteCell {
	public:
		       NoteCell             (NoteGrid* owner, HTp token);
		      ~NoteCell             (void) { clear();                    }

		double getSgnDiatonicPitch  (void) { return m_b7;                }
		double getSgnMidiPitch      (void) { return m_b12;               }
		double getSgnBase40Pitch    (void) { return m_b40;               }
		double getSgnAccidental     (void) { return m_accidental;        }

		double getSgnDiatonicPitchClass(void);
		double getAbsDiatonicPitchClass(void);

		double getSgnBase40PitchClass(void);
		double getAbsBase40PitchClass(void);

		double getAbsDiatonicPitch  (void) { return fabs(m_b7);          }
		double getAbsMidiPitch      (void) { return fabs(m_b12);         }
		double getAbsBase40Pitch    (void) { return fabs(m_b40);         }
		double getAbsAccidental     (void) { return fabs(m_accidental);  }

		HTp    getToken             (void) { return m_token;             }
		int    getNextAttackIndex   (void) { return m_nextAttackIndex;   }
		int    getPrevAttackIndex   (void) { return m_prevAttackIndex;   }
		int    getCurrAttackIndex   (void) { return m_currAttackIndex;   }
		int    getSliceIndex        (void) { return m_timeslice;         }
		int    getVoiceIndex        (void) { return m_voice;             }

		bool   isAttack             (void) { return m_b40>0? true:false; }
		bool   isRest               (void);
		bool   isSustained          (void);

		std::string getAbsKernPitch (void);
		std::string getSgnKernPitch (void);

		double operator-            (NoteCell& B);
		double operator-            (int B);

		int    getLineIndex         (void);
		int    getFieldIndex        (void);
		std::ostream& printNoteInfo (std::ostream& out);
		double getDiatonicIntervalToNextAttack      (void);
		double getDiatonicIntervalFromPreviousAttack(void);
		double getMetricLevel       (void);
		HumNum getDurationFromStart (void);
		HumNum getDuration          (void);
		void   setMeter             (int topval, HumNum botval);
		int    getMeterTop          (void);
		HumNum getMeterBottom       (void);

		std::vector<HTp> m_tiedtokens;  // list of tied notes/rests after note attack

	protected:
		void clear                  (void);
		void calculateNumericPitches(void);
		void setVoiceIndex          (int index) { m_voice = index;           }
		void setSliceIndex          (int index) { m_timeslice = index;       }
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

		double m_b7;         // diatonic note number; NaN=rest; negative=sustain.
		double m_b12;        // MIDI note number; NaN=rest; negative=sustain.
		double m_b40;        // base-40 note number; NaN=rest; negative=sustain.
		double m_accidental; // chromatic alteration of a diatonic pitch.
		                     // NaN=no accidental.
		int m_nextAttackIndex; // index to next note attack (or rest),
		                       // -1 for undefined (interpred as rest).
		int m_prevAttackIndex; // index to previous note attack.
		int m_currAttackIndex; // index to current note attack (useful for
		                       // finding the start of a sustained note.
		int m_metertop = 0;    // top number of prevailing meter signature
		HumNum m_meterbot = 0; // bottom number of prevailing meter signature

	friend NoteGrid;
};


// END_MERGE

} // end namespace hum

#endif /* _NOTECELL_H_INCLUDED */



