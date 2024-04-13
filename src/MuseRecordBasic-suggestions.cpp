//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Wed Apr 10 09:47:42 PDT 2024
// Last Modified: Wed Apr 10 09:47:44 PDT 2024
// Filename:      humlib/src/MuseRecordBasic-suggestions.cpp
// URL:           http://github.com/craigsapp/humlib/blob/master/src/MuseRecordBasic-suggestions.cpp
// Syntax:        C++11
// vim:           ts=3
//

#include "MuseRecordBasic.h"

using namespace std;

namespace hum {

// START_MERGE


//////////////////////////////
//
// MuseRecordBasic::addPrintSuggestion -- add a delta index for associated
//     print suggestion.
//

void MuseRecordBasic::addPrintSuggestion(int deltaIndex) {
	m_printSuggestions.push_back(deltaIndex);
}



// END_MERGE

} // end namespace hum



