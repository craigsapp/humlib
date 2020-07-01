//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sun Oct 16 16:08:05 PDT 2016
// Last Modified: Sat Oct 21 21:59:45 PDT 2017
// Filename:      GridSlice.h
// URL:           https://github.com/craigsapp/hum2ly/blob/master/include/GridSlice.h
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   HumGrid is an intermediate container for converting from
//                MusicXML syntax into Humdrum syntax. GridSlice is a
//                time instance which contains all notes in all parts
//                that should be played at that time.
//

#ifndef _GRIDSLICE_H
#define _GRIDSLICE_H

#include "GridCommon.h"
#include "MxmlPart.h"
#include "GridPart.h"
#include "GridMeasure.h"

#include <iostream>
#include <list>
#include <string>
#include <vector>

namespace hum {

// START_MERGE

class HumGrid;


class GridSlice : public std::vector<GridPart*> {
	public:
		GridSlice(GridMeasure* measure, HumNum timestamp, SliceType type,
		          int partcount = 0);
		GridSlice(GridMeasure* measure, HumNum timestamp, SliceType type,
		          const GridSlice& slice);
		GridSlice(GridMeasure* measure, HumNum timestamp, SliceType type,
		          GridSlice* slice);
		~GridSlice();

		bool isNoteSlice(void)          { return m_type == SliceType::Notes;            }
		bool isGraceSlice(void)         { return m_type == SliceType::GraceNotes;       }
		bool isMeasureSlice(void)       { return m_type == SliceType::Measures;         }
		bool isClefSlice(void)          { return m_type == SliceType::Clefs;            }
		bool isLabelSlice(void)         { return m_type == SliceType::Labels;           }
		bool isLabelAbbrSlice(void)     { return m_type == SliceType::LabelAbbrs;       }
		bool isTransposeSlice(void)     { return m_type == SliceType::Transpositions;   }
		bool isKeySigSlice(void)        { return m_type == SliceType::KeySigs;          }
		bool isKeyDesignationSlice(void){ return m_type == SliceType::KeyDesignations;  }
		bool isTimeSigSlice(void)       { return m_type == SliceType::TimeSigs;         }
		bool isTempoSlice(void)         { return m_type == SliceType::Tempos;           }
		bool isMeterSigSlice(void)      { return m_type == SliceType::MeterSigs;        }
		bool isManipulatorSlice(void)   { return m_type == SliceType::Manipulators;     }
		bool isLayoutSlice(void)        { return m_type == SliceType::Layouts;          }
		bool isLocalLayoutSlice(void)   { return m_type == SliceType::Layouts;          }
		bool isInvalidSlice(void)       { return m_type == SliceType::Invalid;          }
		bool isGlobalComment(void)      { return m_type == SliceType::GlobalComments;   }
		bool isGlobalLayout(void)       { return m_type == SliceType::GlobalLayouts;    }
		bool isReferenceRecord(void)    { return m_type == SliceType::ReferenceRecords; }
		bool isOttavaRecord(void)       { return m_type == SliceType::Ottavas;          }
		bool isInterpretationSlice(void);
		bool isDataSlice(void);
		bool hasSpines(void);
		std::string getNullTokenForSlice(void);
		SliceType getType(void)    { return m_type; }

		void transferTokens        (HumdrumFile& outfile, bool recip);
		void initializePartStaves  (std::vector<MxmlPart>& partdata);
		void initializeBySlice     (GridSlice* slice);
		void initializeByStaffCount(int staffcount);
		void reportVerseCount      (int partindex, int staffindex, int count);

		HumNum       getDuration        (void);
		void         setDuration        (HumNum duration);
		HumNum       getTimestamp       (void);
		void         setTimestamp       (HumNum timestamp);
		void         setOwner           (HumGrid* owner);
		HumGrid*     getOwner           (void);
		HumNum       getMeasureDuration (void);
		HumNum       getMeasureTimestamp(void);
		GridMeasure* getMeasure         (void);
		void         invalidate         (void);

		void transferSides        (HumdrumLine& line, GridStaff& sides,
		                           const std::string& empty, int maxvcount,
		                           int maxhcount, int maxfcount);
		void transferSides        (HumdrumLine& line, GridPart& sides,
		                           int partindex, const std::string& empty,
		                           int maxvcount, int maxhcount,
		                           int maxdcount, int maxfcount);
		int getVerseCount         (int partindex, int staffindex);
		int getHarmonyCount       (int partindex, int staffindex = -1);
		int getDynamicsCount      (int partindex, int staffindex = -1);
		int getFiguredBassCount   (int partindex, int staffindex = -1);
		void addToken             (const std::string& tok, int parti, int staffi, int voicei);

	protected:
		HTp  createRecipTokenFromDuration  (HumNum duration);

	private:
		HumGrid*     m_owner;
		GridMeasure* m_measure;
		HumNum       m_timestamp;
		HumNum       m_duration;
		SliceType    m_type;

};


std::ostream& operator<<(std::ostream& output, GridSlice* slice);
std::ostream& operator<<(std::ostream& output, GridSlice& slice);


// END_MERGE

} // end namespace hum

#endif /* _GRIDSLICE_H */



