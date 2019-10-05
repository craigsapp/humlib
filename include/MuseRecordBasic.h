//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Tue Jun 30 11:51:01 PDT 1998
// Last Modified: Mon Sep 23 22:43:09 PDT 2019 Convert to STL
// Filename:      humilb/include/MuseRecordBasic.h
// URL:           http://github.com/craigsapp/humlib/blob/master/include/MuseRecordBasic.h
// Syntax:        C++11
// vim:           ts=3
//
// Description:   basic data manipulations for lines in a Musedata file.
//

#ifndef _MUSERECORDBASIC_H_INCLUDED
#define _MUSERECORDBASIC_H_INCLUDED

#include "HumNum.h"

#include <stdarg.h>

#include <iostream>


namespace hum {

// START_MERGE

// Reference:     Beyond Midi, page 410.
#define E_muserec_note_regular       'N'
	//                                'A' --> use type E_muserec_note_regular
	//                                'B' --> use type E_muserec_note_regular
	//                                'C' --> use type E_muserec_note_regular
	//                                'D' --> use type E_muserec_note_regular
	//                                'E' --> use type E_muserec_note_regular
	//                                'F' --> use type E_muserec_note_regular
	//                                'G' --> use type E_muserec_note_regular
#define E_muserec_note_chord         'C'
#define E_muserec_note_cue           'c'
#define E_muserec_note_grace         'g'
#define E_muserec_note_grace_chord   'G'
#define E_muserec_print_suggestion   'P'
#define E_muserec_sound_directives   'S'
#define E_muserec_end                '/'
#define E_muserec_endtext            'T'
#define E_muserec_append             'a'
#define E_muserec_backspace          'b'
#define E_muserec_back               'b'
#define E_muserec_backward           'b'
#define E_muserec_figured_harmony    'f'
#define E_muserec_rest_invisible     'i'
#define E_muserec_forward            'i'
#define E_muserec_measure            'm'
#define E_muserec_rest               'r'
#define E_muserec_musical_attributes '$'
#define E_muserec_comment_toggle     '&'
#define E_muserec_comment_line       '@'
#define E_muserec_musical_directions '*'
#define E_muserec_copyright          '1'  // reserved for copyright notice
#define E_muserec_header_1           '1'  // reserved for copyright notice
#define E_muserec_header_2           '2'  // reserved for identification
#define E_muserec_id                 '2'  // reserved for identification
#define E_muserec_header_3           '3'  // reserved
#define E_muserec_header_4           '4'  // <date> <name of encoder>
#define E_muserec_encoder            '4'  // <date> <name of encoder>
#define E_muserec_header_5           '5'  // WK#:<work number> MV#:<mvmt num>
#define E_muserec_work_info          '5'  // WK#:<work number> MV#:<mvmt num>
#define E_muserec_header_6           '6'  // <source>
#define E_muserec_source             '6'  // <source>
#define E_muserec_header_7           '7'  // <work title>
#define E_muserec_work_title         '7'  // <work title>
#define E_muserec_header_8           '8'  // <movement title>
#define E_muserec_movement_title     '8'  // <movement title>
#define E_muserec_header_9           '9'  // <name of part>
#define E_muserec_header_part_name   '9'  // <name of part>
#define E_muserec_header_10          '0'  // misc designations
#define E_muserec_header_11          'A'  // group memberships
#define E_muserec_group_memberships  'A'  // group memberships
// multiple musered_head_12 lines can occur:
#define E_muserec_header_12          'B'  // <name1>: part <x> of <num in group>
#define E_muserec_group              'B'  // <name1>: part <x> of <num in group>
#define E_muserec_unknown            'U'  // unknown record type
#define E_muserec_empty              'E'  // nothing on line and not header
	                                       // or multi-line comment
#define E_muserec_deleted            'D'  // deleted line
// non-standard record types for MuseDataSet
#define E_muserec_filemarker         '+'
#define E_muserec_filename           'F'
#define E_musrec_header               1000
#define E_musrec_footer               2000


class MuseRecordBasic {
	public:
		                  MuseRecordBasic    (void);
		                  MuseRecordBasic    (const std::string& aLine, int index = -1);
		                  MuseRecordBasic    (MuseRecordBasic& aRecord);
		                 ~MuseRecordBasic    ();

		void              clear              (void);
		int               isEmpty            (void);
		void              cleanLineEnding    (void);
		std::string       extract            (int start, int stop);
		char&             getColumn          (int index);
		std::string       getColumns         (int startcol, int endcol);
		void              setColumns         (std::string& data, int startcol,
		                                      int endcol);
		int               getLength          (void) const;
		std::string       getLine            (void);
		int               getLineIndex       (void) { return m_lineindex; }
		void              setLineIndex       (int index);
		int               getLineNumber      (void) { return m_lineindex+1; }
		int               getType            (void) const;
		void              setTypeGraceNote   (void);
		void              setTypeGraceChordNote(void);
		void              setHeaderState     (int state);

		// Humdrum conversion variables
		void              setToken           (HTp token);
		HTp               getToken           (void);
		void              setVoice           (GridVoice* voice);
		GridVoice*        getVoice           (void);

		MuseRecordBasic&  operator=          (MuseRecordBasic& aRecord);
		MuseRecordBasic&  operator=          (MuseRecordBasic* aRecord);
		MuseRecordBasic&  operator=          (const std::string& aRecord);
		char&             operator[]         (int index);
		void              setLine            (const std::string& aString);
		void              setType            (int aType);
		void              shrink             (void);
		void              insertString       (int column, const std::string& strang);
		void              insertStringRight  (int column, const std::string& strang);
		void              setString          (std::string& strang);
		void              appendString       (const std::string& strang);
		void              appendInteger      (int value);
		void              appendRational     (HumNum& value);
		void              append             (const char* format, ...);

		// mark-up accessor functions:

		void              setAbsBeat         (HumNum value);
		void              setAbsBeat         (int topval, int botval = 1);
		HumNum            getAbsBeat         (void);

		void              setLineDuration    (HumNum value);
		void              setLineDuration    (int topval, int botval = 1);
		HumNum            getLineDuration    (void);

		void              setNoteDuration    (HumNum value);
		void              setNoteDuration    (int topval, int botval = 1);
		HumNum            getNoteDuration    (void);
		void              setRoundedBreve    (void);

		void              setMarkupPitch     (int aPitch);
		int               getMarkupPitch     (void);

		void              setLayer           (int layer);
		int               getLayer           (void);

		// tied note functions:
		int               isTied                  (void);
		int               getLastTiedNoteLineIndex(void);
		int               getNextTiedNoteLineIndex(void);
		void              setLastTiedNoteLineIndex(int index);
		void              setNextTiedNoteLineIndex(int index);

		std::string       getLayoutVis       (void);

		// boolean type fuctions:
		bool              isAnyNote          (void);
		bool              isAnyNoteOrRest    (void);
		bool              isAttributes       (void);
		bool              isBackup           (void);
		bool              isBarline          (void);
		bool              isBodyRecord       (void);
		bool              isChordGraceNote   (void);
		bool              isChordNote        (void);
		bool              isAnyComment       (void);
		bool              isLineComment      (void);
		bool              isBlockComment     (void);
		bool              isCopyright        (void);
		bool              isCueNote          (void);
		bool              isEncoder          (void);
		bool              isFiguredHarmony   (void);
		bool              isGraceNote        (void);
		bool              isGroup            (void);
		bool              isGroupMembership  (void);
		bool              isHeaderRecord     (void);
		bool              isId               (void);
		bool              isMovementTitle    (void);
		bool              isPartName         (void);
		bool              isRegularNote      (void);
		bool              isAnyRest          (void);
		bool              isRegularRest      (void);
		bool              isInvisibleRest    (void);
		bool              isSource           (void);
		bool              isWorkInfo         (void);
		bool              isWorkTitle        (void);
		bool              hasTpq             (void);
		int               getTpq             (void);
		void              setTpq             (int value);

	protected:
		std::string       m_recordString;    // actual characters on line

		// mark-up data for the line:
		int               m_lineindex;       // index into original file
		int               m_type;            // category of MuseRecordBasic record
		HumNum            m_absbeat;         // dur in quarter notes from start
		HumNum            m_lineduration;    // duration of line
		HumNum            m_noteduration;    // duration of note

		int               m_b40pitch;        // base 40 pitch
		int               m_nexttiednote;    // line number of next note tied to
		                                     // this one (-1 if no tied note)
		int               m_lasttiednote;    // line number of previous note tied
		                                     // to this one (-1 if no tied note)
		int               m_roundBreve;
		int               m_header = -1;     // -1 = undefined, 0 = no, 1 = yes
		int               m_layer = 0;       // voice/layer (track info but may be analyzed)
		int               m_tpq = 0;         // ticks-per-quarter for durations
		std::string       m_graphicrecip;    // graphical duration of note/rest
		GridVoice*			m_voice = NULL;    // conversion structure that token is stored in.

	public:
		static std::string       trimSpaces         (std::string input);
};


std::ostream& operator<<(std::ostream& out, MuseRecordBasic& aRecord);
std::ostream& operator<<(std::ostream& out, MuseRecordBasic* aRecord);



// END_MERGE

} // end namespace hum

#endif  /* _MUSERECORDBASIC_H_INCLUDED */



