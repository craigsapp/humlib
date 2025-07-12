//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Tue Jul  8 11:27:33 CEST 2025
// Last Modified: Tue Jul  8 19:19:28 CEST 2025
// Filename:      tool-got2hum.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/tool-got2hum.cpp
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Convert from TSV German Organ Tablature data to Humdrum (**kern) data.
//
// Example input: https://docs.google.com/spreadsheets/d/1kE-mLmoC04lpjObuHDKQKSqQi57qnjCfl9YLyaM8Bv0/edit?gid=0#gid=0
//

#include "tool-got2hum.h"
#include "HumRegex.h"

#include <cstdlib>
#include <cstring>
#include <iostream>

using namespace std;

namespace hum {

// START_MERGE


/////////////////////////////////
//
// Tool_got2hum::Tool_got2hum -- Set the recognized options for the tool.
//

Tool_got2hum::Tool_got2hum(void) {
	define("c|cautionary=b",   "display cautionary accidentals (based on previous measure accidental states");
	define("E|no-editorial=b", "do not display parentheses around natural accidentals");
	define("g|got=b",          "output **gotr/**gotp data rather than **kern data");
	define("X|no-force-acc=b", "do not force all accidentals to be shown.");
}



/////////////////////////////////
//
// Tool_got2hum::run -- Do the main work of the tool.
//

bool Tool_got2hum::run(HumdrumFileSet& infiles) {
	return false;
}


bool Tool_got2hum::run(const string& indata, ostream& out) {
	initialize();
	processFile(indata);
	if (hasAnyText()) {
		getAllText(out);
		return true;
	} else {
		return false;
	}
}


bool Tool_got2hum::run(HumdrumFile& infile, ostream& out) {
	return false;
}


bool Tool_got2hum::run(HumdrumFile& infile) {
	return false;
}



//////////////////////////////
//
// Tool_got2hum::initialize --  Initializations that only have to be done once
//    for all HumdrumFile segments.
//

void Tool_got2hum::initialize(void) {
	m_editorialQ  = !getBoolean("no-editorial");
	m_cautionaryQ = getBoolean("cautionary");
	m_gotQ        = getBoolean("got");
	m_modern_accQ = getBoolean("no-force-acc");
}



//////////////////////////////
//
// Tool_got2hum::processFile --
//

void Tool_got2hum::processFile(const string& instring) {
	if (!m_editorialQ) {
		m_gotscore.setNoEditorial();
	}
	if (m_cautionaryQ) {
		m_gotscore.setCautionary();
	}
	if (m_modern_accQ) {
		m_gotscore.setNoForcedAccidentals();
	}

	m_gotscore.loadLines(instring);

	if (m_gotQ) {
		m_humdrum_text << m_gotscore.getGotHumdrum();
	} else {
		m_humdrum_text << m_gotscore.getKernHumdrum();
	}
}



// END_MERGE

} // end namespace hum



