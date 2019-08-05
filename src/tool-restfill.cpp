//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sun Jul 14 01:03:07 CEST 2019
// Last Modified: Sun Jul 14 01:03:12 CEST 2019
// Filename:      tool-restfill.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/tool-restfill.cpp
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Fill in kern measures that have no duration with rests.
//                For now, not considering spine splits.
//

#include "tool-restfill.h"
#include "Convert.h"
#include "HumRegex.h"

using namespace std;

namespace hum {

// START_MERGE


/////////////////////////////////
//
// Tool_restfill::Tool_restfill -- Set the recognized options for the tool.
//

Tool_restfill::Tool_restfill(void) {
	define("y|hidden-rests=b",        "hide inserted rests");
	define("i|exinterp=s:kern",       "type of spine to fill with rests");
}



/////////////////////////////////
//
// Tool_restfill::run -- Do the main work of the tool.
//

bool Tool_restfill::run(HumdrumFileSet& infiles) {
	bool status = true;
	for (int i=0; i<infiles.getCount(); i++) {
		status &= run(infiles[i]);
	}
	return status;
}


bool Tool_restfill::run(const string& indata, ostream& out) {
	HumdrumFile infile;
	infile.readStringNoRhythm(indata);
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_restfill::run(HumdrumFile& infile, ostream& out) {
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_restfill::run(HumdrumFile& infile) {
	initialize();
	processFile(infile);
	infile.createLinesFromTokens();
	return true;
}



//////////////////////////////
//
// Tool_restfill::initialize --
//

void Tool_restfill::initialize(void) {
	m_hiddenQ = getBoolean("hidden-rests");
	m_exinterp = getString("exinterp");
	if (m_exinterp.empty()) {
		m_exinterp = "**kern";
	}
	if (m_exinterp.compare(0, 2, "**") != 0) {
		if (m_exinterp.compare(0, 1, "*") != 0) {
			m_exinterp = "**" + m_exinterp;
		} else {
			m_exinterp = "*" + m_exinterp;
		}
	}
	
}



//////////////////////////////
//
// Tool_restfill::processFile --
//

void Tool_restfill::processFile(HumdrumFile& infile) {

	vector<HTp> starts;
	infile.getSpineStartList(starts, m_exinterp);
	vector<bool> process(starts.size(), false);
	for (int i=0; i<(int)starts.size(); i++) {
		process[i] = hasBlankMeasure(starts[i]);
		if (process[i]) {
			starts[i]->setText("**temp-kern");
		}
	}
	infile.analyzeStructure();
	for (int i=0; i<(int)starts.size(); i++) {
		if (!process[i]) {
			continue;
		}
		starts[i]->setText("**kern");
		fillInRests(starts[i]);
	}
}



//////////////////////////////
//
// Tool_restfill::hasBlankMeasure --
//

bool Tool_restfill::hasBlankMeasure(HTp start) {
	bool foundcontent = false;
	HTp current = start;
	int founddata = false;
	while (current) {

		if (current->isBarline()) {
			if (founddata && !foundcontent) {
				return true;
			}
			foundcontent = false;
			founddata = false;
			current = current->getNextToken();
			continue;
		}
		if (!current->isData()) {
			current = current->getNextToken();
			continue;
		}
		founddata = true;
		if (!current->isNull()) {
			foundcontent = true;
		}
		current = current->getNextToken();

	}
	return false;
}



//////////////////////////////
//
// Tool_restfill::fillInRests --
//   Also deal with cases where the last measure does not end in a barline.
//

void Tool_restfill::fillInRests(HTp start) {
	HTp current = start;
	HTp firstcell = NULL;
	int founddata = false;
	bool foundcontent = false;
	HumNum lasttime = 0;
	HumNum currtime = 0;
	HumNum duration = 0;
	while (current) {
		if (current->isBarline()) {
			if (firstcell) {
				lasttime = firstcell->getDurationFromStart();
			}
			currtime = getNextTime(current);
			if (firstcell && founddata && !foundcontent) {
				duration = currtime - lasttime;
				addRest(firstcell, duration);
			}
			firstcell = NULL;
			founddata = false;
			foundcontent = false;
			current = current->getNextToken();
			lasttime = currtime;
			continue;
		}
		if (!current->isData()) {
			current = current->getNextToken();
			continue;
		}
		if (current->getDuration() == 0) {
			// grace-note line, so ignore
			current = current->getNextToken();
			continue;
		}
		founddata = true;
		if (!current->isNull()) {
			foundcontent = true;
		}
		if (!firstcell) {
			firstcell = current;
		}
		current = current->getNextToken();
	}
}



//////////////////////////////
//
// Tool_restfill::addRest --
//

void Tool_restfill::addRest(HTp cell, HumNum duration) {
	if (!cell) {
		return;
	}
	string text = Convert::durationToRecip(duration);
	text += "r";
	if (m_hiddenQ) {
		text += "yy";
	}
	cell->setText(text);
}



//////////////////////////////
//
// Tool_restfill::getNextTime --
//

HumNum Tool_restfill::getNextTime(HTp token) {
	HTp current = token;
	while (current) {
		if (current->isData()) {
			return current->getDurationFromStart();
		}
		current = current->getNextToken();
	}
	return token->getOwner()->getOwner()->getScoreDuration();
}




// END_MERGE

} // end namespace hum



