//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Nov 28 08:55:15 PST 2016
// Last Modified: Tue Dec 20 22:33:15 PST 2016
// Filename:      HumTool.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/HumTool.cpp
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
// HumTool::~HumTool --
//

HumTool::~HumTool() {
	// do nothing
}



//////////////////////////////
//
// HumTool::hasAnyText -- Returns true if the output contains
//    text content in Humdrum syntax.
//

bool HumTool::hasAnyText(void) {
	return (m_humdrum_text.rdbuf()->in_avail()
			|| m_free_text.rdbuf()->in_avail()
			|| m_json_text.rdbuf()->in_avail());
}



//////////////////////////////
//
// HumTool::getAllText -- Get the text content from any output
//     streams except warnings and errors.
//

string HumTool::getAllText(void) {
	return  m_humdrum_text.str()
	      + m_json_text.str()
	      + m_free_text.str();
}

//
// ostream version:
//

ostream& HumTool::getAllText(ostream& out) {
	out << m_humdrum_text.str();
	out << m_json_text.str();
	out << m_free_text.str();
	return out;
}



//////////////////////////////
//
// HumTool::hasHumdrumText -- Returns true if the output contains
//    text content in Humdrum syntax.
//

bool HumTool::hasHumdrumText(void) {
	return m_humdrum_text.rdbuf()->in_avail() ? true : false;
}



//////////////////////////////
//
// HumTool::getHumdrumText -- Get the text content which represents
//     Humdrum syntax.
//

string HumTool::getHumdrumText(void) {
	return m_humdrum_text.str();
}

//
// ostream version:
//

ostream& HumTool::getHumdrumText(ostream& out) {
	out << m_humdrum_text.str();
	return out;
}



//////////////////////////////
//
// HumTool::hasFreeText --
//

bool HumTool::hasFreeText(void) {
	return m_free_text.rdbuf()->in_avail() ? true : false;
}



//////////////////////////////
//
// HumTool::getFreeText -- Return any free-form text output from the 
//     tool.
//

string HumTool::getFreeText(void) {
	return m_free_text.str();
}

//
// ostream version:
//

ostream& HumTool::getFreeText(ostream& out) {
	out << m_free_text.str();
	return out;
}



//////////////////////////////
//
// HumTool::hasJsonText --
//

bool HumTool::hasJsonText(void) {
	return m_json_text.rdbuf()->in_avail() ? true : false;
}



//////////////////////////////
//
// HumTool::getFreeText -- Return any JSON text output from the 
//     tool.
//

string HumTool::getJsonText(void) {
	return m_json_text.str();
}

//
// ostream version:
//

ostream& HumTool::getJsonText(ostream& out) {
	out << m_json_text.str();
	return out;
}



//////////////////////////////
//
// HumTool::hasWarning --
//

bool HumTool::hasWarning(void) {
	return m_warning_text.rdbuf()->in_avail() ? true : false;
}



//////////////////////////////
//
// HumTool::getWarning -- Return any warning messages generated when
//     running the tool.
//

string HumTool::getWarning(void) {
	return m_warning_text.str();
}

//
// ostream version:
//

ostream& HumTool::getWarning(ostream& out) {
	out << m_warning_text.str();
	return out;
}



//////////////////////////////
//
// HumTool::hasError -- Return true if there is an error in processing
//    the options or input file(s).
//

bool HumTool::hasError(void) {
	if (hasParseError()) {
		return true;
	}
	return m_error_text.rdbuf()->in_avail() ? true : false;
}



//////////////////////////////
//
// HumTool::getError -- Return any error messages generated when
//     running the tool.   This includes option parsing errors as
//     well.
//

string HumTool::getError(void) {
	string output = getParseError();
	output += m_error_text.str();
	return output;
}

//
// ostream version:
//

ostream& HumTool::getError(ostream& out) {
	out << getParseError();
	out << m_error_text.str();
	return out;
}


//////////////////////////////
//
// HumTool::clearOutput -- clear write buffers to get ready to
//     process another file.
//

void HumTool::clearOutput(void) {
	m_humdrum_text.str("");
	m_json_text.str("");
	m_free_text.str("");
  	m_warning_text.str("");
  	m_error_text.str("");
}


// END_MERGE

} // end namespace hum



