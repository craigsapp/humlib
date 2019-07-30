//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sun Jul 14 01:03:07 CEST 2019
// Last Modified: Sun Jul 14 01:03:12 CEST 2019
// Filename:      tool-composite.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/tool-composite.cpp
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Generate composite rhythm spine for music.
//

#include "tool-composite.h"
#include "tool-extract.h"
#include "Convert.h"

using namespace std;

namespace hum {

// START_MERGE


/////////////////////////////////
//
// Tool_composite::Tool_composite -- Set the recognized options for the tool.
//

Tool_composite::Tool_composite(void) {
	define("pitch=s:e",  "pitch to display for composite rhythm");
	define("a|append=b", "append data to end of line");
	define("p|prepend=b", "prepend data to end of line");
}



/////////////////////////////////
//
// Tool_composite::run -- Do the main work of the tool.
//

bool Tool_composite::run(const string& indata, ostream& out) {
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


bool Tool_composite::run(HumdrumFile& infile, ostream& out) {
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_composite::run(HumdrumFile& infile) {
	initialize();
	processFile(infile);
	infile.createLinesFromTokens();
	return true;
}



//////////////////////////////
//
// Tool_composite::initialize --
//

void Tool_composite::initialize(void) {
	m_pitch = getString("pitch");
}



//////////////////////////////
//
// Tool_composite::processFile --
//

void Tool_composite::processFile(HumdrumFile& infile) {
	Tool_extract extract;
	bool appendQ = getBoolean("append");
	bool prependQ = getBoolean("prepend");

	if (appendQ) {
		extract.setModified("s", "1-$,0");
	} else {
		extract.setModified("s", "0,1-$");
	}

	extract.run(infile);
	infile.readString(extract.getAllText());
	HTp token;
	for (int i=0; i<infile.getLineCount(); i++) {
		if (infile[i].isInterpretation()) {
			if (appendQ) {
				token = infile.token(i, infile[i].getFieldCount() - 1);
			} else {
				token = infile.token(i, 0);
			}
			if (token->compare("**blank") == 0) {
				token->setText("**kern");
			}
		}
		if (!infile[i].isData()) {
			continue;
		}
		if (infile[i].getDuration() == 0) {
			continue;
		}
		string recip = Convert::durationToRecip(infile[i].getDuration());
		
		if (appendQ) {
			token = infile.token(i, infile[i].getFieldCount() - 1);
		} else {
			token = infile.token(i, 0);
		}
		recip += getString("pitch");
		token->setText(recip);
	}

	if (!(appendQ || prependQ)) {
		Tool_extract extract2;
		extract2.setModified("s", "1");
		extract2.run(infile);
		infile.readString(extract2.getAllText());
	}

}



// END_MERGE

} // end namespace hum



