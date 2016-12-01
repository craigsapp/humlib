//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Aug  8 12:24:49 PDT 2015
// Last Modified: Tue Nov 29 01:03:06 PST 2016
// Filename:      tool-metlev.h
// URL:           https://github.com/craigsapp/minHumdrum/blob/master/include/tool-metlev.h
// Syntax:        C++11
// vim:           ts=3 noexpandtab
//
// Description:   Extract metric levels
//
// Todo: meters such as 2+3/4
//       pickups (particularly if they do not start on a beat
//       secondary partial measures (if they divide a beat)
//

#include "tool-metlev.h"
#include "Convert.h"

#include <algorithm>
#include <cmath>

using namespace std;

namespace hum {

// START_MERGE


/////////////////////////////////
//
// Tool_gridtest::Tool_metlev -- Set the recognized options for the tool.
//

Tool_metlev::Tool_metlev(void) {
	define("a|append=b",          "append data analysis to input file");
	define("p|prepend=b",         "prepend data analysis to input file");
	define("i|integer=b",         "quantize metric levels to int values");
	define("x|attacks-only=b",    "only mark lines with note attacks");
	define("G|no-grace-notes=b",  "do not mark grace note lines");
	define("k|kern-spine=i:1",    "analyze only given kern spine");
	define("K|all-spines=b",      "analyze each kern spine separately");
}



/////////////////////////////////
//
// Tool_metlev::run --
//

bool Tool_metlev::run(HumdrumFile& infile, ostream& out) {

	int lineCount = infile.getLineCount();
	if (lineCount == 0) {
		m_error << "No input data";
		return false;
	}

	m_kernspines = infile.getKernSpineStartList();

	vector<double> beatlev(lineCount, NAN);
	int track = 0;
	if (m_kernspines.size() > 0) {
		track = m_kernspines[0]->getTrack();
	} else {
		m_error << "No **kern spines in input file" << endl;
		return false;
	}
	infile.getMetricLevels(beatlev, track, NAN);

	for (int i=0; i<lineCount; i++) {
		if (!infile[i].isData()) {
				continue;
		}
		if (getBoolean("no-grace-notes") && (infile[i].getDuration() == 0)) {
			beatlev[i] = NAN;
			continue;
		}
		if (getBoolean("attacks-only")) {
			if (!infile[i].getKernNoteAttacks()) {
				beatlev[i] = NAN;
				continue;
			}
		}
		if (beatlev[i] - (int)beatlev[i] != 0.0) {
			if (getBoolean("integer")) {
					beatlev[i] = floor(beatlev[i]);
			} else {
				beatlev[i] = Convert::significantDigits(beatlev[i], 2);
			}
		}
	}

	// print the analysis results:
	if (getBoolean("kern-spine")) {
		int kspine = getInteger("kern-spine") - 1;
		if ((kspine >= 0) && (kspine < m_kernspines.size())) {
			vector<vector<double> > results;
			fillVoiceResults(results, infile, beatlev);
			if (kspine == (int)m_kernspines.size() - 1) {
				infile.appendDataSpine(results.back(), "nan", "**blev");
			} else {
				int track = m_kernspines[kspine+1]->getTrack();
				infile.insertDataSpineBefore(track, results[kspine],
						"nan", "**blev");
			}
			out << infile;
		}
	} else if (getBoolean("all-spines")) {
		vector<vector<double> > results;
		fillVoiceResults(results, infile, beatlev);
		infile.appendDataSpine(results.back(), "nan", "**blev");
		for (int i = (int)results.size()-1; i>0; i--) {
			int track = m_kernspines[i]->getTrack();
			infile.insertDataSpineBefore(track, results[i-1], "nan", "**blev");
		}
		out << infile;
	} else if (getBoolean("append")) {
		infile.appendDataSpine(beatlev, "nan", "**blev");
		out << infile;
	} else if (getBoolean("prepend")) {
		infile.prependDataSpine(beatlev, "nan", "**blev");
		out << infile;
	} else {
		infile.prependDataSpine(beatlev, "nan", "**blev");
		infile.printFieldIndex(0, out);
	}

	return 0;
}



//////////////////////////////
//
// Tool_metlev::fillVoiceResults -- Split the metric level analysis into values
//     for each voice.
//

void Tool_metlev::fillVoiceResults(vector<vector<double> >& results,
		HumdrumFile& infile, vector<double>& beatlev) {

	results.resize(m_kernspines.size());
	for (int i=0; i<(int)results.size(); i++) {
		results[i].resize(beatlev.size());
		fill(results[i].begin(), results[i].end(), NAN);
	}
	int track;
	vector<int> rtracks(infile.getTrackCount() + 1, -1);
	for (int i=0; i<(int)m_kernspines.size(); i++) {
		int track = m_kernspines[i]->getTrack();
		rtracks[track] = i;
	}

	bool attacksQ = getBoolean("attacks-only");
	vector<int> nonnullcount(m_kernspines.size(), 0);
	vector<int> attackcount(m_kernspines.size(), 0);
	HTp token;
	int voice;
	int i, j;
	for (i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isData()) {
			continue;
		}
		for (j=0; j<infile[i].getFieldCount(); j++) {
			token = infile.token(i, j);
			if (!token->isKern()) {
				continue;
			}
			if (token->isNull()) {
				continue;
			}
			track = token->getTrack();
			voice = rtracks[track];
			nonnullcount[voice]++;
			if (token->isNoteAttack()) {
				attackcount[voice]++;
			}
		}
		for (int v=0; v<(int)m_kernspines.size(); v++) {
			if (attacksQ) {
				if (attackcount[v]) {
					results[v][i] = beatlev[i];
					attackcount[v] = 0;
				}
			} else {
				if (nonnullcount[v]) {
					results[v][i] = beatlev[i];
				}
				nonnullcount[v] = 0;
			}
		}
	}
}

// END_MERGE

} // end namespace hum



