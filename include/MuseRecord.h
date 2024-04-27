//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Tue Jun 30 22:36:03 PDT 1998
// Last Modified: Mon Sep 23 21:05:40 PDT 2019
// Filename:      humextra/include/MuseRecord.h
// Web Address:   https://github.com/craigsapp/humextra/blob/master/include/MuseRecord.h
// Syntax:        C++
// vim:           ts=3:nowrap
//
// Description:   MuseRecord is class that stores one line of data for a Musedata file.
//

#ifndef _MUSERECORD_H_INCLUDED
#define _MUSERECORD_H_INCLUDED

#include "MuseRecordBasic.h"
#include "HumNum.h"

#include <iostream>
#include <string>
#include <map>


namespace hum {

// START_MERGE


class MuseRecord : public MuseRecordBasic {
	public:
		            MuseRecord                  (void);
		            MuseRecord                  (const std::string& aLine);
		            MuseRecord                  (MuseRecord& aRecord);
		           ~MuseRecord                  ();

		MuseRecord& operator=                   (MuseRecord& aRecord);


	//////////////////////////////
	//
	// functions which process regular notes (A-G), cue notes (c), grace notes (g),
	//     and chords (" ").  Definitions stored in MuseRecord-note.cpp.
	//
		// columns 1-5: pitch field information
		std::string      getNoteField                 (void);
		int              getOctave                    (void);
		std::string      getOctaveString              (void);
		int              getPitch                     (void);
		std::string      getPitchString               (void);
		int              getPitchClass                (void);
		std::string      getPitchClassString          (void);
		int              getAccidental                (void);
		std::string      getAccidentalString          (void);
		int              getBase40                    (void);
		void             setPitch                     (int base40, int chordnote = 0,
		                                               int gracenote = 0);
		void             setPitch                     (const std::string& pitchname);
		void             setPitchAtIndex              (int index,
		                                               const std::string& pitchname);
		void             setChordPitch                (const std::string& pitchname);
		void             setGracePitch                (const std::string& pitchname);
		void             setGraceChordPitch           (const std::string& pitchname);
		void             setCuePitch                  (const std::string& pitchname);
		void             setStemDown                  (void);
		void             setStemUp                    (void);

		// columns 6-9: duration field information
		std::string      getTickDurationField         (void);
		std::string      getTickDurationString        (void);
		int              getTickDuration              (void);
		int              getLineTickDuration          (void);
		int              getNoteTickDuration          (void);
		std::string      getTieString                 (void);
		int              getTie                       (void);
		int              setTie                       (int hidden = 0);
		int              tieQ                         (void);
		int              getTicks                     (void);
		void             setTicks                     (int value);
		void             setBack                      (int value);
		void             setDots                      (int value);
		int              getDotCount                  (void);
		void             setNoteheadShape             (HumNum duration);
		void             setNoteheadShapeMensural     (HumNum duration);
		void             setNoteheadMaxima            (void);
		void             setNoteheadLong              (void);
		void             setNoteheadBreve             (void);
		void             setNoteheadBreveSquare       (void);
		void             setNoteheadBreveRound        (void);

		void             setNoteheadWhole             (void);
		void             setNoteheadHalf              (void);
		void             setNoteheadQuarter           (void);
		void             setNotehead8th               (void);
		void             setNotehead16th              (void);
		void             setNotehead32nd              (void);
		void             setNotehead64th              (void);
		void             setNotehead128th             (void);
		void             setNotehead256th             (void);

		void             setNoteheadBreveMensural     (void);
		void             setNoteheadWholeMensural     (void);
		void             setNoteheadHalfMensural      (void);
		void             setNoteheadQuarterMensural   (void);
		void             setNotehead8thMensural       (void);
		void             setNotehead16thMensural      (void);
		void             setNotehead32ndMensural      (void);
		void             setNotehead64thMensural      (void);
		void             setNotehead128thMensural     (void);
		void             setNotehead256thMensural     (void);

		// columns 10-12 ---> blank

		// columns 13-80: graphical and interpretive information

		// column 13: footnote flag
		std::string      getFootnoteFlagField         (void);
		std::string      getFootnoteFlagString        (void);
		int              getFootnoteFlag              (void);
		int              footnoteFlagQ                (void);

		// column 14: level number
		std::string      getLevelField                (void);
		std::string      getLevelString               (void);
		int              getLevel                     (void);
		int              levelQ                       (void);

		// column 15: track number
		std::string      getTrackField                (void);
		std::string      getTrackString               (void);
		int              getTrack                     (void);
		int              trackQ                       (void);

		// column 16 ---> blank

		// column 17: graphic note type
		std::string      getGraphicNoteTypeField      (void);
		std::string      getGraphicNoteTypeString     (void);
		int              getGraphicNoteType           (void);
		int              getGraphicNoteTypeSize       (void);
		int              graphicNoteTypeQ             (void);
		std::string      getGraphicRecip              (void);

		// column 18: dots of prolongation
		std::string      getProlongationField         (void);
		std::string      getProlongationString        (void);
		int              getProlongation              (void);
		std::string      getStringProlongation        (void);
		int              prolongationQ                (void);

		// column 19: notated accidentals
		std::string      getNotatedAccidentalField    (void);
		std::string      getNotatedAccidentalString   (void);
		int              getNotatedAccidental         (void);
		int              notatedAccidentalQ           (void);

		// columns 20-22: time modification
		std::string      getTimeModificationField     (void);
		std::string      getTimeModificationString    (void);
		HumNum           getTimeModification          (void);
		std::string      getTimeModificationLeftField (void);
		std::string      getTimeModificationLeftString(void);
		int              getTimeModificationLeft      (void);
		std::string      getTimeModificationRightField(void);
		std::string      getTimeModificationRightString(void);
		int              getTimeModificationRight     (void);
		bool             timeModificationQ            (void);
		bool             timeModificationLeftQ        (void);
		bool             timeModificationRightQ       (void);

		// column 23: stem direction
		std::string      getStemDirectionField        (void);
		std::string      getStemDirectionString       (void);
		int              getStemDirection             (void);
		int              stemDirectionQ               (void);

		// column 24: staff number for multi-staff parts
		std::string      getStaffField                (void);
		std::string      getStaffString               (void);
		int              getStaff                     (void);
		int              staffQ                       (void);

		// column 25: blank

		// columns 26-31: beaming
		std::string      getBeamField                 (void);
		int              beamQ                        (void);
		char             getBeam8                     (void);
		char             getBeam16                    (void);
		char             getBeam32                    (void);
		char             getBeam64                    (void);
		char             getBeam128                   (void);
		char             getBeam256                   (void);
		int              beam8Q                       (void);
		int              beam16Q                      (void);
		int              beam32Q                      (void);
		int              beam64Q                      (void);
		int              beam128Q                     (void);
		int              beam256Q                     (void);
		void             setBeamInfo                  (std::string& strang);

		// columns 32-43: additional notation
		std::string      getAdditionalNotationsField  (void); // merge with below
		std::string      getOtherNotations            (void); // merge with above
		int              hasFermata                   (void);
		int              additionalNotationsQ         (void);
		int              getAddCount                  (void);
		std::string      getAddItem                   (int elementIndex);
		int              addAdditionalNotation        (char symbol);
		int              addAdditionalNotation        (const std::string& symbol);
		int              getAddItemLevel              (int elementIndex);
		std::string      getEditorialLevels           (void);
		int              addEditorialLevelQ           (void);
		//  protected:   getAddElementIndex
		int              findField                    (const std::string& key);
		int              findField                    (char key, int mincol,
		                                               int maxcol);
		// int           getNotationLevel
		int              getSlurStartColumn           (void);
		std::string      getSlurParameterRegion       (void);
		void             getSlurInfo                  (std::string& slurstarts, std::string& slurends);


		// columns 44-80: text underlay
		std::string      getTextUnderlayField         (void);
		int              textUnderlayQ                (void);
		int              getVerseCount                (void);
		std::string      getVerse                     (int index);
		std::string      getVerseUtf8                 (int index);



	//////////////////////////////
	//
	// functions which work with basso continuo figuration records ('f'), definitions
	//     stored in MuseRecord-figure.cpp
	//
		// column 2: number of figure fields
		std::string      getFigureCountField          (void);
		std::string      getFigureCountString         (void);
		int              getFigureCount               (void);

		// columns 3-5: blank

		// columns 6-8: figure division pointer advancement (duration)
		// this is the offset to the next figure and is not part
		// of the note pointer advancement
		std::string      getFigurePointerField        (void);
		std::string      getFigurePointer             (void);
		int              figurePointerQ               (void);
		int              getFigureDuration            (void);


		// columns 9-12: blank

		// columns 13-15: footnote and level information
		// column 13: footnote: uses same functions as note records in col 13.
		// column 14: level: uses same functions as note records on column 14.
		// column 15: blank

		// columns 17-80: figure fields
		std::string      getFigureFields              (void);
		std::string      getFigureString              (void);
		int              figureFieldsQ                (void);
		std::string      getFigure                    (int index = 0);



	//////////////////////////////
	//
	// functions that work with combined records ('b', 'i'):
	//



	//////////////////////////////
	//
	// functions which work with measure records ('m'), definitions stored
	//    in MuseRecord-measure.cpp.
	//
		// columns 1-7: measure style information
		std::string      getMeasureType               (void);

		// columns 9-12: measure number (left justified)
		std::string      getMeasureNumberField        (void);
		std::string      getMeasureNumber             (void);
		bool             measureNumberQ               (void);

		// columns 17-80: measure flags
		std::string      getMeasureFlags              (void);
		int              measureFermataQ              (void);
		bool             measureFlagEqual             (const std::string& key);
		void             addMeasureFlag               (const std::string& strang);



	//////////////////////////////
	//
	// Notation Attributes: functions which process musical attributes records ('$'),
	//    definitions stored in MuseRecord-attributes.cpp.
	//

		std::string      getAttributes                (void);
		void             getAttributeMap              (std::map<std::string, std::string>& amap);
		int              attributeQ                   (const std::string& attribute);
		int              getAttributeInt              (char attribute);
		int              getAttributeField            (std::string& output, const std::string& attribute);



	//////////////////////////////
	//
	// functions which work with musical direction records ('*'),
	//     definitions stored in MuseRecord-directions.cpp.
	//
		// columns 17-18: type of direction
		std::string      getDirectionTypeField        (void);
		std::string      getDirectionTypeString       (void);
		bool             isDashStart                  (void);
		bool             isDashStop                   (void);
		bool             isDynamic                    (void);
		bool             isHairpin                    (void);
		bool             isHairpinStart               (void);
		bool             isHairpinStop                (void);
		bool             isOctaveUpStart              (void);
		bool             isOctaveDownStart            (void);
		bool             isOctaveStop                 (void);
		bool             isPedalStart                 (void);
		bool             isPedalEnd                   (void);
		bool             isRehearsal                  (void);
		bool             isTextDirection              (void);

		std::string      getDirectionText             (void);
		std::string      getTextDirection             (void) { return getDirectionText(); }

		// Musical Directions: Lines starting with *
		// Functions stored in src/MuseRecordBasic-directions.cpp
		void             addMusicDirection           (int deltaIndex);
		std::string      getDirectionAsciiCharacters (void);
		bool             hasMusicalDirection         (void);
		MuseRecord*      getMusicalDirection         (int index = 0);
		std::string      getDynamicText              (void);
		MuseRecord*      getDirectionRecord          (int deltaIndex);
		std::string      getDirectionType            (void);



	//////////////////////////////
	//
	// Print Suggestings: Lines starting with "P".  Definititions stored
	//     in MuseRecord-suggestions.cpp.
	//

		void            addPrintSuggestion            (int deltaIndex);
		bool             hasPrintSuggestions          (void);
		void             getAllPrintSuggestions       (std::vector<std::string>& suggestions);
		void             getPrintSuggestions          (std::vector<std::string>& suggestions, int column);



	//////////////////////////////
	//
	// Humdrum conversion related functions, store in MuseRecord-humdrum.cpp:
	//

		std::string      getKernMeasure               (void);
		std::string      getKernBeamStyle             (void);
		std::string      getKernNoteStyle             (int beams = 0, int stems = 0);
		std::string      getKernNoteAccents           (void);
		std::string      getKernNoteOtherNotations    (void);
		std::string      getKernRestStyle             (void);


	//
	//////////////////////////////

	protected:
		void             allowNotesOnly               (const std::string& functionName);
		void             allowNotesAndRestsOnly       (const std::string& functionName);
		void             allowMeasuresOnly            (const std::string& functioName);
		void             allowFigurationOnly          (const std::string& functioName);
		void             allowFigurationAndNotesOnly  (const std::string& functioName);
		void             allowDirectionsOnly          (const std::string& functioName);
		int              getAddElementIndex           (int& index, std::string& output,
		                                               const std::string& input);
		void             zerase                       (std::string& inout, int num);
};


// END_MERGE

} // end namespace hum

#endif  /* _MUSERECORD_H_INCLUDED */



