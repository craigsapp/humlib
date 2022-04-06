//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Wed Mar  9 21:55:00 PST 2022
// Last Modified: Wed Mar  9 21:55:04 PST 2022
// Filename:      tool-fb.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/tool-fb.cpp
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Extract figured bass from melodic content.
// Reference:     https://github.com/WolfgangDrescher/humdrum-figured-bass-filter-demo
//

#include "tool-fb.h"
#include "HumRegex.h"
#include "Convert.h"

using namespace std;

namespace hum {

// START_MERGE


/////////////////////////////////
//
// Tool_fb::Tool_fb -- Set the recognized options for the tool.
//

Tool_fb::Tool_fb(void) {
	define("d|debug=b", "Print debug information");
	define("r|reference=i:0", "Reference kern spine (1 indexed)");
}



/////////////////////////////////
//
// Tool_fb::run -- Do the main work of the tool.
//

bool Tool_fb::run(HumdrumFileSet& infiles) {
	bool status = true;
	for (int i=0; i<infiles.getCount(); i++) {
		status &= run(infiles[i]);
	}
	return status;
}


bool Tool_fb::run(const string& indata, ostream& out) {
	HumdrumFile infile(indata);
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_fb::run(HumdrumFile& infile, ostream& out) {
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_fb::run(HumdrumFile& infile) {
	initialize();
	processFile(infile);
	return true;
}



//////////////////////////////
//
// Tool_fb::initialize --  Initializations that only have to be done once
//    for all HumdrumFile segments.
//

void Tool_fb::initialize(void) {
	m_debugQ = getBoolean("debug");
	m_reference = getInteger("reference") - 1;
}



//////////////////////////////
//
// Tool_fb::processFile --
//

void Tool_fb::processFile(HumdrumFile& infile) {
	setupScoreData(infile);
	getHarmonicIntervals(infile);
	printOutput(infile);
}



//////////////////////////////
//
// Tool_fb::getHarmonicIntervals -- Fill in
//

void Tool_fb::getHarmonicIntervals(HumdrumFile& infile) {
	m_intervals.resize(infile.getLineCount());

	vector<HTp> tokens(m_kernspines.size(), NULL);
	for (int i=0; i<infile.getLineCount(); i++) {
		m_intervals[i].resize(0);
		if (!infile[i].isData()) {
			continue;
		}
		fill(tokens.begin(), tokens.end(), (HTp)NULL);
		for (int j=0; j<infile[i].getFieldCount(); j++) {
			HTp token = infile.token(i, j);
			if (!token->isKern()) {
				continue;
			}
			int track = token->getTrack();
			int index = m_track2index.at(track);
			tokens[index] = token;
			// cerr << token << "\t";
		}
		m_intervals[i].resize(m_kernspines.size());
		calculateIntervals(m_intervals[i], tokens, m_reference);
		// cerr << endl;

		if (m_debugQ) {
			for (int j=0; j<(int)m_intervals[i].size(); j++) {
				m_free_text << tokens[j] << "\t(";
				if (m_intervals[i][j] == m_rest) {
					m_free_text << "R";
				} else {
					m_free_text << m_intervals[i][j];
				}
				m_free_text << ")";
				if (j < (int)m_intervals[i].size() - 1) {
					m_free_text << "\t";
				}
			}
			m_free_text << endl;
		}
	}
}



//////////////////////////////
//
// Tool_fb::calculateIntervals --
//

void Tool_fb::calculateIntervals(vector<int>& intervals,
		vector<HTp>& tokens, int bassIndex) {
	if (intervals.size() != tokens.size()) {
		cerr << "ERROR: Size if vectors do not match" << endl;
		return;
	}

	HTp reftok = tokens[m_reference];
	if (reftok->isNull()) {
		reftok = reftok->resolveNull();
	}

	if (!reftok || reftok->isRest()) {
		for (int i=0; i<(int)tokens.size(); i++) {
			intervals[i] = m_rest;
		}
		return;
	}

	int base40ref = Convert::kernToBase40(reftok);

	for (int i=0; i<(int)tokens.size(); i++) {
		if (i == m_reference) {
			intervals[i] = m_rest;
			continue;
		}
		if (tokens[i]->isRest()) {
			intervals[i] = m_rest;
			continue;
		}
		if (tokens[m_reference]->isRest()) {
			intervals[i] = m_rest;
			continue;
		}
		if (tokens[i]->isNull()) {
			continue;
		}
		int base40 = Convert::kernToBase40(tokens[i]);
		int interval = base40 - base40ref;
		intervals[i] = interval;
	}
}



//////////////////////////////
//
// Tool_fb::setupScoreData --
//

void Tool_fb::setupScoreData(HumdrumFile& infile) {
	infile.getKernSpineStartList(m_kernspines);
	m_kerntracks.resize(m_kernspines.size());
	for (int i=0; i<(int)m_kernspines.size(); i++) {
		m_kerntracks[i] = m_kernspines[i]->getTrack();
	}

	int maxtrack = infile.getMaxTrack();
	m_track2index.resize(maxtrack + 1);
	fill(m_track2index.begin(), m_track2index.end(), -1);
	for (int i=0; i<(int)m_kerntracks.size(); i++) {
		m_track2index.at(m_kerntracks[i]) = i;
	}

	if (m_reference >= (int)m_kernspines.size()) {
		m_reference = (int)m_kernspines.size() - 1;
	}
	if (m_reference < 0) {
		m_reference = 0;
	}

	vector<int> pcs(7, 0);

	m_keyaccid.resize(infile.getLineCount());
	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isInterpretation()) {
			continue;
		}
		for (int j=0; j<infile[i].getFieldCount(); j++) {
			HTp token = infile.token(i, j);
			if (!token->isKern()) {
				continue;
			}
			if (token->isKeySignature()) {
				fill(pcs.begin(), pcs.end(), 0);
				HumRegex hre;
				if (hre.search(token, "c#")) { pcs[0] = +1;}
				if (hre.search(token, "d#")) { pcs[1] = +1;}
				if (hre.search(token, "e#")) { pcs[2] = +1;}
				if (hre.search(token, "f#")) { pcs[3] = +1;}
				if (hre.search(token, "g#")) { pcs[4] = +1;}
				if (hre.search(token, "a#")) { pcs[5] = +1;}
				if (hre.search(token, "b#")) { pcs[6] = +1;}
				if (hre.search(token, "c-")) { pcs[0] = -1;}
				if (hre.search(token, "d-")) { pcs[1] = -1;}
				if (hre.search(token, "e-")) { pcs[2] = -1;}
				if (hre.search(token, "f-")) { pcs[3] = -1;}
				if (hre.search(token, "g-")) { pcs[4] = -1;}
				if (hre.search(token, "a-")) { pcs[5] = -1;}
				if (hre.search(token, "b-")) { pcs[6] = -1;}
				m_keyaccid[i] = pcs;
			}
		}
	}

	for (int i=1; i<infile.getLineCount(); i++) {
		if (m_keyaccid[i].empty()) {
			m_keyaccid[i] = m_keyaccid[i-1];
		}
	}
	for (int i=infile.getLineCount() - 2; i>=0; i--) {
		if (m_keyaccid[i].empty()) {
			m_keyaccid[i] = m_keyaccid[i+1];
		}
	}
}



//////////////////////////////
//
// Tool_fb:printOutput --
//

void Tool_fb::printOutput(HumdrumFile& infile) {
	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].hasSpines()) {
			m_humdrum_text << infile[i] << endl;
			continue;
		}
		printLineStyle3(infile, i);
	}
}



//////////////////////////////
//
// Tool_fb::printLineStyle3 --
//

void Tool_fb::printLineStyle3(HumdrumFile& infile, int line) {
	bool printed = false;
	int reftrack = m_kerntracks[m_reference];
	bool tab = false;

	for (int i=0; i<infile[line].getFieldCount(); i++) {
		HTp token = infile.token(line, i);
		int track = token->getTrack();
		if (printed || (track != reftrack + 1)) {
			if (tab) {
				m_humdrum_text << "\t" << token;
			} else {
				tab = true;
				m_humdrum_text << token;
			}
			continue;
		}
		// print analysis spine and then next spine
		if (tab) {
			m_humdrum_text << "\t";
		} else {
			tab = true;
		}
		m_humdrum_text << getAnalysisTokenStyle3(infile, line, i);
		printed = true;
		m_humdrum_text << "\t" << token;
	}
	m_humdrum_text << "\n";
}



//////////////////////////////
//
// Tool_fb::getAnalysisTokenStyle3 --
//

string Tool_fb::getAnalysisTokenStyle3(HumdrumFile& infile, int line, int field) {
	if (infile[line].isCommentLocal()) {
		return "!";
	}
	if (infile[line].isInterpretation()) {
		HTp token = infile.token(line, 0);
		if (token->compare(0, 2, "**") == 0) {
			return "**fb";
		} else if (*token == "*-") {
			return "*-";
		} else if (token->isLabel()) {
			return *token;
		} else if (token->isExpansionList()) {
			return *token;
		} else if (token->isKeySignature()) {
			return *token;
		} else if (token->isKeyDesignation()) {
			return *token;
		} else {
			return "*";
		}
	}
	if (infile[line].isBarline()) {
		HTp token = infile.token(line, 0);
		return *token;
	}

	// create data token
	string output;

	for (int i=(int)m_intervals[line].size()-1; i>=0; i--) {
		if (i == m_reference) {
			continue;
		}
		int base40int = m_intervals[line][i];
		string iname = Convert::base40ToIntervalAbbr(base40int);
		output += iname;
		output += " ";
	}
	if (!output.empty()) {
		output.resize((int)output.size() - 1);
	}

	return output;
}


// END_MERGE

} // end namespace hum



