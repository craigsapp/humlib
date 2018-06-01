//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Thu May 31 16:41:35 PDT 2018
// Last Modified: Thu May 31 16:41:39 PDT 2018
// Filename:      tool-phrase.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-phrase.cpp
// Syntax:        C++11
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Analyses phrases
//

#include "tool-phrase.h"
#include "Convert.h"

#include <algorithm>
#include <cmath>

using namespace std;

namespace hum {

// START_MERGE


/////////////////////////////////
//
// Tool_gridtest::Tool_phrase -- Set the recognized options for the tool.
//

Tool_phrase::Tool_phrase(void) {
	define("k|kern-spine=i:1",    "analyze only given kern spine");
}



///////////////////////////////
//
// Tool_phrase::run -- Primary interfaces to the tool.
//

bool Tool_phrase::run(const string& indata, ostream& out) {
	HumdrumFile infile(indata);
	return run(infile, out);
}


bool Tool_phrase::run(HumdrumFile& infile, ostream& out) {
	bool status = run(infile);
	return status;
}


bool Tool_phrase::run(HumdrumFile& infile) {
	vector<HTp> starts = infile.getKernSpineStartList();
	for (int i=0; i<(int)starts.size(); i++) {
		if (getBoolean("kern-spine")) {
			if (i+1 != getInteger("kern-spine")) {
				continue;
			}
		}
cerr << "PROCESSING SPINE " << i << endl;
		analyzeSpine(starts, i);
	}
	cout << m_free_text.str();
	return true;
}



///////////////////////////////
//
// Tool_phrase::analyzeSpine --
//

void Tool_phrase::analyzeSpine(vector<HTp>& starts, int index) {
	HTp start    = starts[index];
	HTp current  = start;
	HTp lastnote = NULL;   // last note to be processed
	HTp pstart   = NULL;   // phrase start;
	HumNum dur;
	while (current) {
cerr << "\tPROCESSING " << current << endl;
		if (current->isBarline()) {
			if (current->find("||") != std::string::npos) {
				if (pstart) {
					dur = current->getDurationFromStart()
							- pstart->getDurationFromStart();
					if (dur > 0) {
						m_free_text << dur.getFloat() << "\n";
					}
					pstart = NULL;
				}
			}
		}
		if (!current->isData()) {
			current = current->getNextToken();
			continue;
		}
		if (current->isNull()) {
			current = current->getNextToken();
			continue;
		}
		if (pstart && current->isRest()) {
			if (lastnote) {
				dur = current->getDurationFromStart()
						- pstart->getDurationFromStart();
				if (dur > 0) {
					m_free_text << dur.getFloat() << "\n";
					cerr << "PHRASE\t" << dur.getFloat() << endl;
				}
			}
			pstart = NULL;
			lastnote = NULL;
			current = current->getNextToken();
			continue;
		}
		if (current->isRest()) {
			current = current->getNextToken();
			continue;
		}
		if (current->isNote()) {
			lastnote = current;
		}
		if (current->isNote() && pstart == NULL) {
			pstart = current;
cerr << "INITIALIZEING PHRASE" << endl;
		}
		current = current->getNextToken();
	}
	if (pstart) {
		dur = start->getOwner()->getOwner()->getScoreDuration()
				- pstart->getDurationFromStart();
		if (dur > 0) {
			m_free_text << dur.getFloat() << "\n";
			cerr << "PHRASE\t" << dur.getFloat() << endl;
		}
	}
}



// END_MERGE

} // end namespace hum



