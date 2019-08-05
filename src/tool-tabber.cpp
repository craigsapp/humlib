//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Wed Jun 12 12:08:10 CEST 2019
// Last Modified: Wed Jun 12 12:08:13 CEST 2019
// Filename:      tool-tabber.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/tool-tabber.cpp
// Syntax:        C++11; humlib
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Add/remove extra spacing tabs from Humdrum data.
//
//

#include "tool-tabber.h"

using namespace std;

namespace hum {

// START_MERGE


/////////////////////////////////
//
// Tool_gridtest::Tool_tabber -- Set the recognized options for the tool.
//

Tool_tabber::Tool_tabber(void) {
	// do nothing for now.
	define("r|remove=b",    "remove any extra tabs");
}



///////////////////////////////
//
// Tool_tabber::run -- Primary interfaces to the tool.
//

bool Tool_tabber::run(HumdrumFileSet& infiles) {
	bool status = true;
	for (int i=0; i<infiles.getCount(); i++) {
		status &= run(infiles[i]);
	}
	return status;
}


bool Tool_tabber::run(const string& indata, ostream& out) {
	HumdrumFile infile(indata);
	return run(infile, out);
}


bool Tool_tabber::run(HumdrumFile& infile, ostream& out) {
	bool status = run(infile);
	out << m_free_text.str();
	return status;
}


bool Tool_tabber::run(HumdrumFile& infile) {
   initialize(infile);
	processFile(infile);
	return true;
}



//////////////////////////////
//
// Tool_tabber::initialize --
//

void Tool_tabber::initialize(HumdrumFile& infile) {
	// do nothing for now
}



//////////////////////////////
//
// Tool_tabber::processFile --
//

void Tool_tabber::processFile(HumdrumFile& infile) {
	if (getBoolean("remove")) {
		infile.removeExtraTabs();
	} else {
		infile.addExtraTabs();
	}
	infile.createLinesFromTokens();
}


// END_MERGE

} // end namespace hum



