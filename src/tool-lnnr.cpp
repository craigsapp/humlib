//
// Programmer:    Wolfgang Drescher <drescher.wolfgang@gmail.com>
// Creation Date: Sat Jul 13 2024 09:44:00 CET
// Filename:      tool-lnnr.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/tool-lnnr.cpp
// Syntax:        C++11; humlib
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Add line numbers or line indices as spine
//

#include "tool-lnnr.h"
#include "Convert.h"
#include "HumRegex.h"

using namespace std;

namespace hum {

// START_MERGE

//////////////////////////////
//
// Tool_lnnr::Tool_lnnr -- Set the recognized options for the tool.
//

Tool_lnnr::Tool_lnnr(void) {
	define("i|index=b", "output line indices instead of line numbers");
	define("p|prepend=b", "add **lnnr spine as first spine");
}



//////////////////////////////
//
// Tool_lnnr::run -- Do the main work of the tool.
//

bool Tool_lnnr::run(HumdrumFileSet &infiles) {
	bool status = true;
	for (int i = 0; i < infiles.getCount(); i++) {
		status &= run(infiles[i]);
	}
	return status;
}

bool Tool_lnnr::run(const string &indata, ostream &out) {
	HumdrumFile infile(indata);
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}

bool Tool_lnnr::run(HumdrumFile &infile, ostream &out) {
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}

bool Tool_lnnr::run(HumdrumFile &infile) {
	initialize();
	processFile(infile);
	return true;
}



//////////////////////////////
//
// Tool_lnnr::initialize --
//

void Tool_lnnr::initialize(void) {
	m_indexQ = getBoolean("index");
	m_prependQ = getBoolean("prepend");
}



//////////////////////////////
//
// Tool_lnnr::processFile --
//

void Tool_lnnr::processFile(HumdrumFile& infile) {
	vector<string> trackData = getTrackData(infile);
	string exinterp = m_indexQ ? "**lnidx" : "**lnnr";
	if (m_prependQ) {
		infile.insertDataSpineBefore(1, trackData, ".", exinterp);
	} else {
		infile.appendDataSpine(trackData, ".", exinterp);
	}
	m_humdrum_text << infile;
}



//////////////////////////////
//
// Tool_lnnr::getTrackData --
//

vector<string> Tool_lnnr::getTrackData(HumdrumFile& infile) {
	vector<string> trackData;
	trackData.resize(infile.getLineCount());

	for (int i = 0; i < infile.getLineCount(); i++) {
		HLp line = infile.getLine(i);
		trackData[i] = m_indexQ ? to_string(line->getLineIndex()) : to_string(line->getLineNumber());
	}

	return trackData;
}



// END_MERGE

} // end namespace hum
