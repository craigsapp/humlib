//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Wed Jun 12 12:08:10 CEST 2019
// Last Modified: Wed Jun 12 12:08:13 CEST 2019
// Filename:      tool-tabber.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-tabber.cpp
// Syntax:        C++11
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
}



///////////////////////////////
//
// Tool_tabber::run -- Primary interfaces to the tool.
//

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
	vector<int> trackWidths = getTrackWidths(infile);
	vector<int> local(trackWidths.size());
	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].hasSpines()) {
			m_free_text << infile[i] << "\n";
			continue;
		}
		fill(local.begin(), local.end(), 0);
		int lasttrack = 0;
		int track = 0;
		for (int j=0; j<infile[i].getFieldCount(); j++) {
			lasttrack = track;
			HTp token = infile.token(i, j);
			track = token->getTrack();
			if ((track != lasttrack) && (lasttrack > 0)) {
				int diff = trackWidths[lasttrack] - local[lasttrack];
				if (diff > 0) {
					for (int k=0; k<diff; k++) {
						m_free_text << '\t';
					}
				}
			}
			if (j > 0) {
				m_free_text << '\t';
			}
			local[track]++;
			m_free_text << token;
		}
		m_free_text << '\n';
	}
}



//////////////////////////////
//
// Tool_tabber::getTrackWidths
//

vector<int> Tool_tabber::getTrackWidths(HumdrumFile& infile) {
	vector<int> output(infile.getTrackCount() + 1, 1);
	output[0] = 0;
	vector<int> local(infile.getTrackCount() + 1);
	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].hasSpines()) {
			continue;
		}
		fill(local.begin(), local.end(), 0);
		for (int j=0; j<infile[i].getFieldCount(); j++) {
			HTp token = infile.token(i, j);
			int track = token->getTrack();
			local[track]++;
		}
		for (int j=1; j<(int)local.size(); j++) {
			if (local[j] > output[j]) {
				output[j] = local[j];
			}
		}
	}
	return output;
}


// END_MERGE

} // end namespace hum



