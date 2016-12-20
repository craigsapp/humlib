//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Nov 28 08:55:15 PST 2016
// Last Modified: Mon Nov 28 08:55:38 PST 2016
// Filename:      HumTool.cpp
// URL:           https://github.com/craigsapp/minHumdrum/blob/master/src/HumTool.cpp
// Syntax:        C++11
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Common interface for Humdrum tools.
//

#include "HumTool.h"

namespace hum {

// START_MERGE


//////////////////////////////
//
// HumTool::HumTool --
//

HumTool::HumTool(void) {
	// do nothing
}



//////////////////////////////
//
// HumTool::HumTool --
//

HumTool::~HumTool() {
	// do nothing
}



//////////////////////////////
//
// HumTool::hasError --
//

bool HumTool::hasError(void) {
	if (hasParseError()) {
		return true;
	}
	return m_error.rdbuf()->in_avail();
}



//////////////////////////////
//
// HumTool::getError -- Return any error messages generated when
//     running the tool.
//

string HumTool::getError(void) {
	string output = getParseError();
	output += m_error.str();
	return output;
}

//
// ostream version:
//

ostream& HumTool::getError(ostream& out) {
	out << getParseError();
	out << m_text.str();
	return out;
}



//////////////////////////////
//
// HumTool::hasNonHumdrumOutput --
//

bool HumTool::hasNonHumdrumOutput(void) {
	return m_text.rdbuf()->in_avail();
}


//////////////////////////////
//
// HumTool::hasNonHumdrumOutput --
//

string HumTool::getTextOutput(void) {
	return m_text.str();
}

//
// ostream version:
//

ostream& HumTool::getTextOutput(ostream& out) {
	out << m_text.str();
	return out;
}



// END_MERGE

} // end namespace hum



