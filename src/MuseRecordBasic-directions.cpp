//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Wed Apr 10 09:47:42 PDT 2024
// Last Modified: Wed Apr 10 09:47:44 PDT 2024
// Filename:      humlib/src/MuseRecordBasic-direction.cpp
// URL:           http://github.com/craigsapp/humlib/blob/master/src/MuseRecordBasic-direction.cpp
// Syntax:        C++11
// vim:           ts=3
//

#include "MuseData.h"
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
// MuseRecordBasic::addMusicDirection -- add a delta index for associated
//     print suggestion.
//

void MuseRecordBasic::addMusicDirection(int deltaIndex) {
	m_musicalDirections.push_back(deltaIndex);
}



//////////////////////////////
//
// MuseRecordBasic::getDirectionAsciiCharacters -- returns columns 25
//    and later, but with the return string removing any trailing spaces.
//

std::string MuseRecordBasic::getDirectionAsciiCharacters(void) {
	if (!isDirection()) {
		return "";
	}
	string& mrs = m_recordString;
	if (mrs.size() < 25) {
		return "";
	}
	string output = mrs.substr(24);
	size_t endpos = output.find_last_not_of(" \t\r\n");
   return (endpos != std::string::npos) ? output.substr(0, endpos + 1) : "";
}



//////////////////////////////
//
// MuseRecordBasic::hasMusicalDirection --
//

bool MuseRecordBasic::hasMusicalDirection(void) {
	if (isDirection()) {
		return true;
	}
	if (!m_musicalDirections.empty()) {
		return true;
	}
	return false;
}



//////////////////////////////
//
// MuseRecordBasic::getMusicalDuration -- return any associated
//     Musical Direction record for the current record.  If there
//     is no linked direction, then return NULL.  If the record is
//     itself a muscial direction, return the pointer to the record.
//     Default value for index is 0.
//

MuseRecordBasic* MuseRecordBasic::getMusicalDirection(int index) {
	if (m_musicalDirections.empty()) {
		return NULL;
	}
	if (index >= (int)m_musicalDirections.size()) {
		return NULL;
	}
	return getDirectionRecord(m_musicalDirections.at(index));
}



//////////////////////////////
//
// MuseRecordBasic::getDirectionRecord -- return the given direction from the store
//    delta index for the musical direction line.  Default value index = 0.
//

MuseRecordBasic* MuseRecordBasic::getDirectionRecord(int deltaIndex) {
	int index = m_lineindex + deltaIndex;
	if (index < 0) {
		return NULL;
	}
	if (!m_owner) {
		return NULL;
	}
	int lineCount = m_owner->getLineCount();
	if (index >= lineCount) {
		return NULL;
	}
	return m_owner->getRecordPointer(index);
}



//////////////////////////////
//
// MuseRecordBasic::getDirectionType -- columns 17 and 18 of
//    musical directions.  This function will remove space
//    chaters from the columns.
//
// Direction Types:
// =================
// A = segno sign
// E = dynamics hairpin start (qualifiers [BCDG])
// F = dynamics hairpin start
// G = dynamics letters (in columns 25+)
// H = dash line start (qualifiers [BCDG])
// J = dash line end (qualifiers [BCDG])
// P = piano pedal start
// Q = piano pedal end
// R = rehearsal number or letter
// U = shift notes up (usually by 8va)
// V = shift notes down (usually by 8va)
// W = stop octave shift
// X = tie terminator
//

string MuseRecordBasic::getDirectionType(void) {
	if (!isDirection()) {
		return "";
	}
	string value = getColumns(17, 18);
	if (value[1] == ' ') {
		value.resize(1);
	}
	if (value[0] == ' ') {
		value.resize(0);
	}
	return value;
}



//////////////////////////////
//
// MuseRecordBasic::isDynamic -- helper function for getDirectionType() == "G".
//

bool MuseRecordBasic::isDynamic(void) {
	string dirtype = getDirectionType();
	if (dirtype.empty()) {
		return false;
	}
	if (dirtype.at(0) == 'G') {
		return true;
	} else {
		return false;
	}
}



//////////////////////////////
//
// MuseRecordBasic::getDynamicText -- 
//

string MuseRecordBasic::getDynamicText(void) {
	return getDirectionAsciiCharacters();
}


// END_MERGE

} // end namespace hum



