//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Wed Apr 10 09:47:42 PDT 2024
// Last Modified: Wed Apr 10 09:47:44 PDT 2024
// Filename:      humlib/src/MuseRecordBasic-direction.cpp
// URL:           http://github.com/craigsapp/humlib/blob/master/src/MuseRecordBasic-direction.cpp
// Syntax:        C++11
// vim:           ts=3
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
// MuseRecordBasic::getMusicalDirectionBuffer -- buffered musical directions
//    that should be applied at the next note.
//

vector<MuseRecordBasic*>& MuseRecordBasic::getMusicalDirectionBuffer(void) {
	return m_musicalDirectionBuffer;
}



//////////////////////////////
//
// MuseRecordBasic::clearMusicalDirectionBuffer -- clear buffered musical
//    directions.
//

void MuseRecordBasic::clearMusicalDirectionBuffer(void) {
	m_musicalDirectionBuffer.clear();
}



//////////////////////////////
//
// MuseRecordBasic::addMusicalDirectionBuffer -- add buffered musical
//    directions.
//

void MuseRecordBasic::addMusicalDirectionBuffer(MuseRecordBasic* mr) {
	m_musicalDirectionBuffer.push_back(mr);
}



// END_MERGE

} // end namespace hum



