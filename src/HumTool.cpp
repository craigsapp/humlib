//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Nov 28 08:55:15 PST 2016
// Last Modified: Mon Nov 28 08:55:38 PST 2016
// Filename:      HumTool.h
// URL:           https://github.com/craigsapp/minHumdrum/blob/master/src/HumTool.cpp
// Syntax:        C++11
// vim:           ts=3 noexpandtab
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
	return m_error.rdbuf()->in_avail();
}



//////////////////////////////
//
// HumTool::getError -- Return any error messages generated when
//     running the tool.
//

string HumTool::getError(void) {
	return m_error.str();
}


// END_MERGE

} // end namespace hum



