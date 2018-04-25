//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Jun 17 15:24:23 CEST 2017
// Last Modified: Sat Jul  8 17:17:21 CEST 2017
// Filename:      tool-ruthfix.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/tool-ruthfix.cpp
// Syntax:        C++11
// vim:           ts=3 noexpandtab
//
// Description:   Interface for fixing early-music MusicXML import where a tied note over a barline
//                is represented as an invisible rest.  This occurs in MusicXML output from Sibelius.
//

#include "tool-ruthfix.h"
#include "Convert.h"
#include "HumRegex.h"

#include <algorithm>
#include <cmath>

using namespace std;

namespace hum {

// START_MERGE


/////////////////////////////////
//
// Tool_ruthfix::Tool_ruthfix -- Set the recognized options for the tool.
//

Tool_ruthfix::Tool_ruthfix(void) {
	// add options here
}



/////////////////////////////////
//
// Tool_ruthfix::run -- Do the main work of the tool.
//

bool Tool_ruthfix::run(const string& indata, ostream& out) {
	HumdrumFile infile(indata);
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_ruthfix::run(HumdrumFile& infile, ostream& out) {
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_ruthfix::run(HumdrumFile& infile) {
	insertCrossBarTies(infile);
	return true;
}



//////////////////////////////
//
// Tool_ruthfix::insertCrossBarTies -- 
//

void Tool_ruthfix::insertCrossBarTies(HumdrumFile& infile) {
	int scount = infile.getStrandCount();
	if (scount == 0) {
		// The input file was not read from a file but was created
		// dynamically.  The easiest thing to do is to reload to get the 
		// spine/strand information.
		stringstream ss;
		infile.createLinesFromTokens();
		ss << infile;
		infile.readString(ss.str());
	}
	scount = infile.getStrandCount();


	HTp token;
	for (int i=0; i<scount; i++) {
		token = infile.getStrandStart(i);
		if (!token->isKern()) {
			continue;
		}
		insertCrossBarTies(infile, i);
	}
}


void Tool_ruthfix::insertCrossBarTies(HumdrumFile& infile, int strand) {
	HTp sstart = infile.getStrandStart(strand);
	HTp send   = infile.getStrandEnd(strand);
	HTp s = sstart;
	HTp lastnote = NULL;
	bool barstart = true;
	while (s != send) {
		if (s->isBarline()) {
			barstart = true;
		} else if (s->isNote()) {
			if (lastnote && barstart && (s->find("yy") != string::npos)) {
				createTiedNote(lastnote, s);
			}
			barstart = false;
			lastnote = s;
		} else if (s->isRest()) {
			lastnote = NULL;
			barstart = false;
		}
		s = s->getNextToken();
		if (!s) {
			break;
		}
	}
}



//////////////////////////////
//
// Tool_ruthfix::createTiedNote -- Does not work for chords.
//  change  1E-X TO 2E-Xyy
//      to  [1E-X TO 2E-X]
//

void Tool_ruthfix::createTiedNote(HTp left, HTp right) {
	if (left->isChord() || right->isChord()) {
		return;
	}
	auto loc = right->find("yy");
	if (loc != string::npos) {
		left->insert(0, 1, '[');
		right->replace(loc, 2, "]");
	}
}



// END_MERGE

} // end namespace hum



