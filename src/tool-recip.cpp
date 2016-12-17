//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Wed Dec  7 08:01:07 PST 2016
// Last Modified: Wed Dec  7 12:40:28 PST 2016
// Filename:      tool-recip.cpp
// URL:           https://github.com/craigsapp/minHumdrum/blob/master/include/tool-recip.cpp
// Syntax:        C++11
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Extract **recip data from **kern data.
//
// Todo: deal with *v spines.
//

#include "tool-recip.h"
#include "Convert.h"
#include "HumRegex.h"

#include <algorithm>
#include <cmath>

using namespace std;

namespace hum {

// START_MERGE


/////////////////////////////////
//
// Tool_gridtest::Tool_recip -- Set the recognized options for the tool.
//

Tool_recip::Tool_recip(void) {
	define("c|composite=b",          "do composite rhythm analysis");
	define("a|append=b",             "append composite analysis to input");
	define("p|prepend=b",            "prepend composite analysis to input");
	define("r|replace=b",            "replace **kern data with **recip data");
	define("x|attacks-only=b",       "only mark lines with note attacks");
	define("G|ignore-grace-notes=b", "ignore grace notes");
	define("k|kern-spine=i:1",       "analyze only given kern spine");
	define("K|all-spines=b",         "analyze each kern spine separately");
}



///////////////////////////////
//
// Tool_recip::run -- Primary interfaces to the tool.
//

bool Tool_recip::run(const string& indata, ostream& out) {
	HumdrumFile infile(indata);
	return run(infile, out);
}


bool Tool_recip::run(HumdrumFile& infile, ostream& out) {
	int status = run(infile);
	out << infile;
	return status;
}


bool Tool_recip::run(HumdrumFile& infile) {
   initialize(infile);

	int lineCount = infile.getLineCount();
	if (lineCount == 0) {
		m_error << "No input data";
		return false;
	}

	if (getBoolean("composite") || getBoolean("append") || getBoolean("prepend")) {
		doCompositeAnalysis(infile);
		infile.createLinesFromTokens();
		return true;
	} else if (getBoolean("replace")) {
		replaceKernWithRecip(infile);
		infile.createLinesFromTokens();
		return true;
	}
	HumdrumFile cfile = infile;
	cfile.analyzeStructure();
	replaceKernWithRecip(cfile);
	cfile.createLinesFromTokens();
	insertAnalysisSpines(infile, cfile);
	// infile.adjustMergeSpineLines();
	infile.createLinesFromTokens();
	return true;
}



//////////////////////////////
//
// Tool_recip::insertAnalysisSpines -- Could be more efficient than the 
//     k-index loop...
//

void Tool_recip::insertAnalysisSpines(HumdrumFile& infile, HumdrumFile& cfile) {
	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].hasSpines()) {
			continue;
		}
		for (int k=(int)m_kernspines.size()-1; k>=0; k--) {
			int fcount = infile[i].getFieldCount();
			int ktrack = m_kernspines[k]->getTrack();
			int insertj = -1;
			for (int j=fcount-1; j>=0; j--) {
				if (!infile.token(i, j)->isKern()) {
					continue;
				}
				int track = infile.token(i, j)->getTrack();
				if (track != ktrack) {
					continue;
				}
				if (insertj < 0) {
					insertj = j;
				}
				infile[i].appendToken(insertj, cfile.token(i, j)->getText());
				// infile.token(i, insertj+1)->setTrack(remapping[k]);
			}
		}
	}
}



//////////////////////////////
//
// Tool_recip::doCompositeAnalysis --
//

void Tool_recip::doCompositeAnalysis(HumdrumFile& infile) {

	// Calculate composite rhythm **recip spine:

	vector<HumNum> composite(infile.getLineCount());
	for (int i=0; i<(int)composite.size(); i++) {
		composite[i] = infile[i].getDuration();
	}
	
	// convert durations to **recip strings
	vector<string> recips(composite.size());
	for (int i=0; i<(int)recips.size(); i++) {
		if ((!m_graceQ) && (composite[i] == 0)) {
			continue;
		}
		recips[i] = Convert::durationToRecip(composite[i]);
	}

	if (getBoolean("append")) {
		infile.appendDataSpine(recips, "", m_exinterp);
		return;
	} else if (getBoolean("prepend")) {
		infile.prependDataSpine(recips, "", m_exinterp);
		return;
	} else {
		infile.prependDataSpine(recips, "", m_exinterp);
		infile.printFieldIndex(0, m_text);
		infile.clear();
		infile.readString(m_text.str());
	}
}



//////////////////////////////
//
// Tool_recip::replaceKernWithRecip --
//

void Tool_recip::replaceKernWithRecip(HumdrumFile& infile) {
	vector<HTp> kspines = infile.getKernSpineStartList();
	HumRegex hre;
	string expression = "[^q\\d.%\\]\\[]+";
	for (int i=0; i<infile.getStrandCount(); i++) {
		HTp stok = infile.getStrandStart(i);
		if (!stok->isKern()) {
			continue;
		}
		HTp etok = infile.getStrandEnd(i);
		HTp tok = stok;
		while (tok && (tok != etok)) {
			if (!tok->isData()) {
				tok = tok->getNextToken();
				continue;
			}
			if (tok->isNull()) {
				tok = tok->getNextToken();
				continue;
			}
			if (tok->find('q') != string::npos) {
				if (m_graceQ) {
					tok->setText("q");
				} else {
					tok->setText(".");
				}
			} else {
				hre.replaceDestructive(*tok, "", expression, "g");
			}
			tok = tok->getNextToken();
		}
	}

	for (int i=0; i<(int)kspines.size(); i++) {
		kspines[i]->setText(m_exinterp);
	}

}




//////////////////////////////
//
// Tool_recip::initialize --
//

void Tool_recip::initialize(HumdrumFile& infile) {
	m_kernspines = infile.getKernSpineStartList();
	m_graceQ = !getBoolean("ignore-grace-notes");

	m_exinterp = getString("exinterp");
	if (m_exinterp.empty()) {
		m_exinterp = "**recip";
	} else if (m_exinterp[0] != '*') {
		m_exinterp.insert(0, "*");
	}
	if (m_exinterp[1] != '*') {
		m_exinterp.insert(0, "*");
	}
}



// END_MERGE

} // end namespace hum



