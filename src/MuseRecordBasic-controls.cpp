//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Fri Apr 12 11:56:08 PDT 2024
// Last Modified: Fri Apr 12 11:56:32 PDT 2024
// Filename:      humlib/src/MuseRecordBasic-controls.cpp
// URL:           http://github.com/craigsapp/humlib/blob/master/src/MuseRecordBasic-controls.cpp
// Syntax:        C++11
// vim:           ts=3
//
// Description:   Functions related to control codes, which is the first character on a line
//                (except for the header which are fixed lines, and inside of multi-line comments.
//
// Control codes from specs (https://wiki.ccarh.org/images/9/9f/Stage2-specs.html) at the end
// of the docuentation:
//
// Fixed order parameters in header:
// #1   = copyight                        isCopyright() isHeaderRecord()
// #2   = ID                              isId()
// #3
// #4   = encoder                         isEncoder() isHeaderRecord()
// #2   = work info                       isWorkdInfo() isHeaderRecord()
// #5   = source info                     isSource() isHeaderRecord()
// #6   = work title                      isWorkTitle() isHeaderRecord()
// #7   = movement title                  isMovementTitle() isHeaderRecord()
// #8   = part name                       isPartName() isHeaderRecord()
// #9
// #10  = group membership                isGroupMembership() isHeaderRecord()
// #11  = group                           isGroup() isHeaderRecord()
// #12
// #13
//
// MuseRecordBasic::isBodyRecord -- True if not a header record.
//
// ' ' = extra note in chord               isChordNote()
// $  = controlling musical attributes     isAttributes()
// &  = comment mode toggle switch         isBlockComment() isAnyComment()
// *  = musical directions                 isDirection()
// A  = regular note                       isRegularNote() isAnyNote() isAnyNoteOrRest()
// B  =    "     "                         isRegularNote()
// C  =    "     "                         isRegularNote()
// D  =    "     "                         isRegularNote()
// E  =    "     "                         isRegularNote()
// F  =    "     "                         isRegularNote()
// G  =    "     "                         isRegularNote()
// /  = end of music or end of file
// @  = single line comment                isLineComment() isAnyComment()
// a  = append to previous line
// b  = backspace in time                  isBackup()
// c  = cue size note                      isCueNote()
// f  = figured harmony                    isFiguredHarmony()
// g  = grace note                         isGraceNote()
// i  = invisible rest                     isInvisibleRest() isAnyRest()
// m  = bar line                           isBarline()
// r  = regular rest                       isRegularRest() isAnyRest()
// S  = sound directions
// P  = print suggestions                  isPrintSuggestion()
//

#include "MuseRecordBasic.h"

#include <cctype>
#include <cstdio>
#include <cstring>
#include <sstream>

using namespace std;

namespace hum {

// START_MERGE


//////////////////////////////
//
// MuseRecordBasic::isPartName --
//

bool MuseRecordBasic::isPartName(void) {
	return m_type == E_muserec_header_part_name;
}



//////////////////////////////
//
// MuseRecordBasic::isAttributes --
//

bool MuseRecordBasic::isAttributes(void) {
	return m_type == E_muserec_musical_attributes;
}



//////////////////////////////
//
// MuseRecordBasic::isSource --
//

bool MuseRecordBasic::isSource(void) {
	return m_type == E_muserec_source;
}



//////////////////////////////
//
// MuseRecordBasic::isEncoder --
//

bool MuseRecordBasic::isEncoder(void) {
	return m_type == E_muserec_encoder;
}



//////////////////////////////
//
// MuseRecordBasic::isId --
//

bool MuseRecordBasic::isId(void) {
	return m_type == E_muserec_id;
}



//////////////////////////////
//
// MuseRecordBasic::isBarline --
//

bool MuseRecordBasic::isBarline(void) {
	return m_type == E_muserec_measure;
}



//////////////////////////////
//
// MuseRecordBasic::isBackup --
//

bool MuseRecordBasic::isBackup(void) {
	return m_type == E_muserec_back;
}



//////////////////////////////
//
// MuseRecordBasic::isAnyComment --
//

bool MuseRecordBasic::isAnyComment(void) {
	return isLineComment() || isBlockComment();
}



//////////////////////////////
//
// MuseRecordBasic::isLineComment --
//

bool MuseRecordBasic::isLineComment(void) {
	return m_type == E_muserec_comment_line;
}



//////////////////////////////
//
// MuseRecordBasic::isBlockComment --
//

bool MuseRecordBasic::isBlockComment(void) {
	return m_type == E_muserec_comment_toggle;
}



//////////////////////////////
//
// MuseRecordBasic::isChordNote -- Is a regular note that is a seoncdary
//    note in a chord (not the first note in the chord).
//

bool MuseRecordBasic::isChordNote(void) {
	return m_type == E_muserec_note_chord;
}



//////////////////////////////
//
// MuseRecordBasic::isDirection -- Is a musical direction (text)
//     instruction.
//

bool MuseRecordBasic::isDirection(void) {
	return m_type == E_muserec_musical_directions;
}



//////////////////////////////
//
// MuseRecordBasic::isMusicalDirection -- Is a musical direction (text)
//     instruction.
//

bool MuseRecordBasic::isMusicalDirection(void) {
	return isDirection();
}



//////////////////////////////
//
// MuseRecordBasic::isGraceNote -- A grace note, either a single note or
//     the first note in a gracenote chord.
//

bool MuseRecordBasic::isGraceNote(void) {
	return m_type == E_muserec_note_grace;
}



//////////////////////////////
//
// MuseRecordBasic::isCueNote --
//

bool MuseRecordBasic::isCueNote(void) {
	return m_type == E_muserec_note_cue;
}



//////////////////////////////
//
// MuseRecordBasic::isChordNote --
//

bool MuseRecordBasic::isChordGraceNote(void) {
	return m_type == E_muserec_note_grace_chord;
}



//////////////////////////////
//
// MuseRecordBasic::isFiguredHarmony --
//

bool MuseRecordBasic::isFiguredHarmony(void) {
	return m_type == E_muserec_figured_harmony;
}



//////////////////////////////
//
// MuseRecordBasic::isPrintSuggestion --
//

bool MuseRecordBasic::isPrintSuggestion(void) {
	switch (m_type) {
		case E_muserec_print_suggestion:
			return true;
	}
	return false;
}



//////////////////////////////
//
// MuseRecordBasic::isRegularNote --
//

bool MuseRecordBasic::isRegularNote(void) {
	switch (m_type) {
		case E_muserec_note_regular:
			return true;
	}
	return false;
}


//////////////////////////////
//
// MuseRecordBasic::isAnyNote --
//

bool MuseRecordBasic::isAnyNote(void) {
	switch (m_type) {
		case E_muserec_note_regular:
		case E_muserec_note_chord:
		case E_muserec_note_cue:
		case E_muserec_note_grace:
		case E_muserec_note_grace_chord:
			return true;
	}
	return false;
}



//////////////////////////////
//
// MuseRecordBasic::isAnyNoteOrRest --
//

bool MuseRecordBasic::isAnyNoteOrRest(void) {
	switch (m_type) {
		case E_muserec_note_regular:
		case E_muserec_note_chord:
		case E_muserec_note_cue:
		case E_muserec_note_grace:
		case E_muserec_note_grace_chord:
		case E_muserec_rest_invisible:
		case E_muserec_rest:
			return true;
	}
	return false;
}



//////////////////////////////
//
// MuseRecordBasic::isInvisibleRest --
//

bool MuseRecordBasic::isInvisibleRest(void) {
	switch (m_type) {
		case E_muserec_rest_invisible:
			return true;
	}
	return false;
}



//////////////////////////////
//
// MuseRecordBasic::isRegularRest --
//

bool MuseRecordBasic::isRegularRest(void) {
	switch (m_type) {
		case E_muserec_rest:
			return true;
	}
	return false;
}



//////////////////////////////
//
// MuseRecordBasic::isAnyRest -- Also cue-sized rests?
//

bool MuseRecordBasic::isAnyRest(void) {
	switch (m_type) {
		case E_muserec_rest_invisible:
		case E_muserec_rest:
			return true;
	}
	return false;
}



//////////////////////////////
//
// MuseRecordBasic::isCopyright --
//

bool MuseRecordBasic::isCopyright(void) {
	switch (m_type) {
		case E_muserec_copyright:
			return true;
	}
	return false;
}



//////////////////////////////
//
// MuseRecordBasic::isWorkInfo --
//

bool MuseRecordBasic::isWorkInfo(void) {
	switch (m_type) {
		case E_muserec_work_info:
			return true;
	}
	return false;
}



//////////////////////////////
//
// MuseRecordBasic::isWorkTitle --
//

bool MuseRecordBasic::isWorkTitle(void) {
	switch (m_type) {
		case E_muserec_work_title:
			return true;
	}
	return false;
}



//////////////////////////////
//
// MuseRecordBasic::isMovementTitle --
//

bool MuseRecordBasic::isMovementTitle(void) {
	switch (m_type) {
		case E_muserec_movement_title:
			return true;
	}
	return false;
}



//////////////////////////////
//
// MuseRecordBasic::isGroup --
//

bool MuseRecordBasic::isGroup(void) {
	switch (m_type) {
		case E_muserec_group:
			return true;
	}
	return false;
}



//////////////////////////////
//
// MuseRecordBasic::isGroupMembership --
//

bool MuseRecordBasic::isGroupMembership(void) {
	switch (m_type) {
		case E_muserec_group_memberships:
			return true;
	}
	return false;
}



//////////////////////////////
//
// MuseRecordBasic::isHeaderRecord -- True if a header, or a comment
//   occurring before the first non-header record.
//

bool MuseRecordBasic::isHeaderRecord(void) {
	return m_header > 0;
}



//////////////////////////////
//
// MuseRecordBasic::isBodyRecord -- True if not a header record.
//

bool MuseRecordBasic::isBodyRecord(void) {
	return m_header == 0;
}


// END_MERGE

} // end namespace hum



