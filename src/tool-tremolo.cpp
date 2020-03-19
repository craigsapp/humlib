//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sun Oct 13 11:41:16 PDT 2019
// Last Modified: Mon Oct 28 21:56:33 PDT 2019
// Filename:      tool-tremolo.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/tool-tremolo.cpp
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Tremolo expansion tool.
//

#include "tool-tremolo.h"
#include "Convert.h"
#include "HumRegex.h"

using namespace std;

namespace hum {

// START_MERGE


/////////////////////////////////
//
// Tool_tremolo::Tool_tremolo -- Set the recognized options for the tool.
//

Tool_tremolo::Tool_tremolo(void) {
	// add input options here
}



/////////////////////////////////
//
// Tool_tremolo::run -- Do the main work of the tool.
//

bool Tool_tremolo::run(HumdrumFileSet& infiles) {
	bool status = true;
	for (int i=0; i<infiles.getCount(); i++) {
		status &= run(infiles[i]);
	}
	return status;
}


bool Tool_tremolo::run(const string& indata, ostream& out) {
	HumdrumFile infile(indata);
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_tremolo::run(HumdrumFile& infile, ostream& out) {
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_tremolo::run(HumdrumFile& infile) {
	processFile(infile);
	return true;
}


//////////////////////////////
//
// Tool_tremolo::processFile --
//

void Tool_tremolo::processFile(HumdrumFile& infile) {
	HumRegex hre;
	for (int i=infile.getLineCount()-1; i>=0; i--) {
		if (!infile[i].isData()) {
			continue;
		}
		if (infile[i].getDuration() == 0) {
			// don't deal with grace notes
			continue;
		}
		for (int j=0; j<infile[i].getFieldCount(); j++) {
			HTp token = infile.token(i, j);
			if (!token->isKern()) {
				continue;
			}
			if (token->isNull()) {
				continue;
			}
			if (hre.search(token, "@(\\d+)@")) {
				int value = hre.getMatchInt(1);
				HumNum duration = Convert::recipToDuration(token);
				HumNum count = duration;
				count *= value;
				count /= 4;
				HumNum increment = 4;
				increment = 4;
				increment /= value;
				if (!count.isInteger()) {
					cerr << "Error: time value cannot be used: " << value << endl;
					continue;
				}
				int kcount = count.getNumerator();
				HumNum starttime = token->getDurationFromStart();
				HumNum timestamp;
				for (int k=1; k<kcount; k++) {
					timestamp = starttime + (increment * k);
					infile.insertNullDataLine(timestamp);
				}
			}
		}
	}

	m_humdrum_text << infile;
}


// END_MERGE

} // end namespace hum



