//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon May  3 09:58:36 PDT 2021
// Last Modified: Mon May  3 09:58:39 PDT 2021
// Filename:      tool-timebase.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-timebase.cpp
// Syntax:        C++11; humlib
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Interface for timebase tool, which causes data lines to
//                have the same duration.
//

#include "tool-timebase.h"
#include "Convert.h"

#include <algorithm>
#include <cmath>

using namespace std;

namespace hum {

// START_MERGE


/////////////////////////////////
//
// Tool_timebase::Tool_timebase -- Set the recognized options for the tool.
//

Tool_timebase::Tool_timebase(void) {
	define("g|grace=b",       "Keep grace notes");
	define("m|min=b",         "Use minimum time in score for timebase");
	define("t|timebase=s:16", "Timebase rhythm");
	define("q|quiet=b",       "Quite mode: Do not output warnings");
}



///////////////////////////////
//
// Tool_timebase::run -- Primary interfaces to the tool.
//

bool Tool_timebase::run(HumdrumFileSet& infiles) {
	bool status = true;
	for (int i=0; i<infiles.getCount(); i++) {
		status &= run(infiles[i]);
	}
	return status;
}

bool Tool_timebase::run(const string& indata, ostream& out) {
	HumdrumFile infile(indata);
	return run(infile, out);
}


bool Tool_timebase::run(HumdrumFile& infile, ostream& out) {
	bool status = run(infile);
	return status;
}


bool Tool_timebase::run(HumdrumFile& infile) {
	processFile(infile);
	return true;
}



///////////////////////////////
//
// Tool_timebase::processFile -- Adjust intervals of ornaments.
//

void Tool_timebase::processFile(HumdrumFile& infile) {
	m_grace   = getBoolean("grace");
	m_quiet   = getBoolean("quiet");
	if (!getBoolean("timebase")) {
		m_basedur = getMinimumTime(infile);
	} else {
		m_basedur = Convert::recipToDuration(getString("timebase"));
	}
	if (m_basedur == 0) {
		// some problem so don't do anything (return input data)
		return;
	}
	expandScore(infile, m_basedur);
}


//////////////////////////////
//
// Tool_timebase::expandScore --
//

void Tool_timebase::expandScore(HumdrumFile& infile, HumNum mindur) {
	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isData()) {
			m_humdrum_text << infile[i] << endl;
			continue;
		}
		HumNum duration = infile[i].getDuration();
		if (duration == 0) {
			if (m_grace) {
				m_humdrum_text << infile[i] << endl;
			}
			continue;
		}
		HumNum count = duration / mindur;
		if (count < 1) {
			if (!m_quiet) {
				m_humdrum_text << "!!Warning: following commented line was too short to be included in timebase output:\n";
				m_humdrum_text << "!! " << infile[i] << endl;
			}
			continue;
		} else if (count.getDenominator() != 1) {
			if (!m_quiet) {
				m_humdrum_text << "!!Warning: next line does not have proper duration for representing with timebase: " << count.getFloat() << endl;
			}
		}
		m_humdrum_text << infile[i] << endl;
		int repeats = int(count.getFloat()) - 1;
		for (int j=0; j<repeats; j++) {
			for (int k=0; k<infile[i].getFieldCount(); k++) {
				m_humdrum_text << ".";
				if (k < infile[i].getFieldCount() - 1) {
					m_humdrum_text << "\t";
				}
			}
			m_humdrum_text << endl;
		}
	}
	if (!m_quiet) {
		HumNum rhythm = Convert::durationToRecip(mindur);
		m_humdrum_text << "!!timebased: " << rhythm << endl;
	}

}




///////////////////////////////
//
// Tool_timebase::getMinimumTime -- Get the minimum time unit
//    in the file.  This is the smallest non-zero line duration.
//

HumNum Tool_timebase::getMinimumTime(HumdrumFile& infile) {
	HumNum minimum(0, 1);
	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isData()) {
			continue;
		}
		HumNum duration = infile[i].getDuration();
		if (minimum == 0) {
			minimum = duration;
			continue;
		}
		if (minimum > duration) {
			minimum = duration;
		}
	}
	// minimum is in units of quarter notes.
	return minimum;
}





// END_MERGE

} // end namespace hum



