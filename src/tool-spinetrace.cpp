//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Oct 16 21:44:03 PDT 2000
// Last Modified: Fri Jun 14 11:45:17 CEST 2019 Updated for humlib.
// Filename:      src/tool-spinetrace.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/tool-spinetrace.cpp
// Syntax:        C++11; humlib
//
// Description:   Identifies data field spine memberships
//

#include "tool-spinetrace.h"

using namespace std;

namespace hum {

// START_MERGE


/////////////////////////////////
//
// Tool_gridtest::Tool_spinetrace -- Set the recognized options for the tool.
//

Tool_spinetrace::Tool_spinetrace(void) {
	define("a|append=b", "append analysis to input data lines");
	define("p|prepend=b", "prepend analysis to input data lines");
}



///////////////////////////////
//
// Tool_spinetrace::run -- Primary interfaces to the tool.
//

bool Tool_spinetrace::run(const string& indata, ostream& out) {
	HumdrumFile infile(indata);
	return run(infile, out);
}


bool Tool_spinetrace::run(HumdrumFile& infile, ostream& out) {
	bool status = run(infile);
	return status;
}


bool Tool_spinetrace::run(HumdrumFile& infile) {
   initialize(infile);
	processFile(infile);
	return true;
}



//////////////////////////////
//
// Tool_spinetrace::initialize --
//

void Tool_spinetrace::initialize(HumdrumFile& infile) {
	// do nothing for now
}



//////////////////////////////
//
// Tool_spinetrace::processFile --
//

void Tool_spinetrace::processFile(HumdrumFile& infile) {
	bool appendQ = getBoolean("append");
	bool prependQ = getBoolean("prepend");

	int linecount = infile.getLineCount();
	for (int i=0; i<linecount; i++) {
		if (!infile[i].hasSpines()) {
			m_humdrum_text << infile[i] << endl;
			continue;
		}
		if (appendQ) {
			m_humdrum_text << infile[i] << "\t";
		}

		if (!infile[i].isData()) {
			if (infile[i].isInterpretation()) {
				int fieldcount = infile[i].getFieldCount();
				for (int j=0; j<fieldcount; j++) {
					HTp token = infile.token(i, j);
					if (token->compare(0, 2, "**") == 0) {
						m_humdrum_text << "**spine";
					} else {
						m_humdrum_text << token;
					}
					if (j < fieldcount - 1) {
						m_humdrum_text << "\t";
					}
				}
			} else {
				m_humdrum_text << infile[i];
			}
		} else {
			int fieldcount = infile[i].getFieldCount();
			for (int j=0; j<fieldcount; j++) {
				m_humdrum_text << infile[i].token(j)->getSpineInfo();
				if (j < fieldcount - 1) {
					m_humdrum_text << '\t';
				}
			}
		}

		if (prependQ) {
			m_humdrum_text << "\t" << infile[i];
		}
		m_humdrum_text << "\n";
	}
}


// END_MERGE

} // end namespace hum



