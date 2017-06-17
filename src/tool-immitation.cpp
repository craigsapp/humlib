//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Jun 17 15:24:23 CEST 2017
// Last Modified: Sat Jun 17 22:41:25 CEST 2017
// Filename:      tool-immitation.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/tool-immitation.cpp
// Syntax:        C++11
// vim:           ts=3 noexpandtab
//
// Description:   Counterpoint immitation tool.
//
// Todo:          retrograde searches
//                highlight tied notes in matches
//                terminate matches at rests
//                create **vvdata

#include "tool-immitation.h"
#include "Convert.h"
#include "HumRegex.h"

#include <algorithm>
#include <cmath>

using namespace std;

namespace hum {

// START_MERGE


int Tool_immitation::Enumerator = 0;


/////////////////////////////////
//
// Tool_immitation::Tool_immitation -- Set the recognized options for the tool.
//

Tool_immitation::Tool_immitation(void) {
	define("debug=b",             "print grid cell information");
	define("e|exinterp=s:**vdata","specify exinterp for **vdata spine");
	define("n|threshold=i:5",     "minimum number of notes to match");
	define("D|no-duration=b",     "do not consider duration when matching");
	define("r|rest=b",            "require match trigger to follow a rest");
	define("R|rest2=b",           "require match target to also follow a rest");
	define("m|mark=b",            "mark matched sequences");
}



/////////////////////////////////
//
// Tool_immitation::run -- Do the main work of the tool.
//

bool Tool_immitation::run(const string& indata, ostream& out) {

	HumdrumFile infile(indata);
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_immitation::run(HumdrumFile& infile, ostream& out) {
	int status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_immitation::run(HumdrumFile& infile) {
	NoteGrid grid(infile);

	if (getBoolean("debug")) {
		grid.printGridInfo(cerr);
		// return 1;
	} 

	m_threshold = getInteger("threshold") + 1;
	if (m_threshold < 3) {
		m_threshold = 3;
	}

	m_duration = !getBoolean("no-duration");
	m_mark     = getBoolean("mark");
	m_rest     = getBoolean("rest");
	m_rest2    = getBoolean("rest2");

	vector<vector<string>>    results;
	vector<vector<NoteCell*>> attacks;
	vector<vector<double>>    intervals;

	doAnalysis(results, grid, attacks, intervals, infile, getBoolean("debug"));

	string exinterp = getString("exinterp");
	vector<HTp> kernspines = infile.getKernSpineStartList();
	infile.appendDataSpine(results.back(), "", exinterp);
	for (int i = (int)results.size()-1; i>0; i--) {
		int track = kernspines[i]->getTrack();
		infile.insertDataSpineBefore(track, results[i-1], "", exinterp);
	}
	infile.createLinesFromTokens();
	if (m_mark && Enumerator) {
		string rdfline = "!!!RDF**kern: ";
		rdfline += m_marker;
		rdfline += " = marked note (color=\"chocolate\")";
		infile.appendLine(rdfline);
	}
	return true;
}



//////////////////////////////
//
// Tool_immitation::doAnalysis -- do a basic melodic analysis of all parts.
//

void Tool_immitation::doAnalysis(vector<vector<string> >& results,
		NoteGrid& grid, vector<vector<NoteCell*> >& attacks,
		vector<vector<double>>& intervals, HumdrumFile& infile,
		bool debug) {

	results.resize(grid.getVoiceCount());
	for (int i=0; i<(int)results.size(); i++) {
		results[i].resize(infile.getLineCount());
	}

	attacks.resize(grid.getVoiceCount());
	for (int i=0; i<(int)attacks.size(); i++) {
		grid.getNoteAndRestAttacks(attacks[i], i);
	}

	intervals.resize(grid.getVoiceCount());
	for (int i=0; i<(int)intervals.size(); i++) {
		intervals[i].resize(attacks[i].size());
		getIntervals(intervals[i], attacks[i]);
	}


	for (int i=0; i<(int)attacks.size(); i++) {
		for (int j=i+1; j<(int)attacks.size(); j++) {
			analyzeImmitation(results, attacks, intervals, i, j);
		}
	}
}



///////////////////////////////
//
// Tool_immitation::getIntervals --
//

void Tool_immitation::getIntervals(vector<double>& intervals,
		vector<NoteCell*>& attacks) {
	for (int i=0; i<attacks.size() - 1; i++) {
		intervals[i] = *attacks[i+1] - *attacks[i];
	}
	intervals.back() = NAN;
}



//////////////////////////////
//
// Tool_immitation::analyzeImmitation -- do immitation analysis between two voices.
//

void Tool_immitation::analyzeImmitation(vector<vector<string>>& results,
		vector<vector<NoteCell*>>& attacks, vector<vector<double>>& intervals,
		int v1, int v2) {

	vector<NoteCell*>& v1a = attacks[v1];
	vector<NoteCell*>& v2a = attacks[v2];
	vector<double>& v1i = intervals[v1];
	vector<double>& v2i = intervals[v2];

	int min = m_threshold - 1;
	int count;

	vector<int> enum1(v1a.size(), 0);
	vector<int> enum2(v1a.size(), 0);

	for (int i=0; i<v1i.size() - 1; i++) {
		for (int j=0; j<v2i.size() - 1; j++) {
			if (m_rest || m_rest2) {
				if ((i > 0) && (!Convert::isNaN(attacks[v1][i-1]->getSgnDiatonicPitch()))) {
					// match initiator must be preceded by a rest (or start of music)
					continue;
				}
			}
			if (m_rest2) {
				if ((j > 0) && (!Convert::isNaN(attacks[v2][j-1]->getSgnDiatonicPitch()))) {
					// match target must be preceded by a rest (or start of music)
					continue;
				}
			}
			if ((enum1[i] != 0) && (enum1[i] == enum2[j])) {
				// avoid re-matching an existing match as a submatch
				continue;
			}
			count = compareSequences(v1a, v1i, i, v2a, v2i, j);
			if (count >= min) {
				Enumerator++;
				for (int k=0; k<count; k++) {
					enum1[i+k] = Enumerator;
					enum2[j+k] = Enumerator;
				}
				// cout << "Match length count " << count << endl;
				HTp token1 = attacks[v1][i]->getToken();
				HTp token2 = attacks[v2][j]->getToken();
				HumNum time1 = token1->getDurationFromStart();
				HumNum time2 = token2->getDurationFromStart();
				HumNum distance1 = time2 - time1;
				HumNum distance2 = time1 - time2;

				int interval = *attacks[v2][j] - *attacks[v1][i];
				int line1 = attacks[v1][i]->getLineIndex();
				int line2 = attacks[v2][j]->getLineIndex();
				if (!results[v1][line1].empty()) {
					results[v1][line1] += " ";
				}
				results[v1][line1] += "n";
				results[v1][line1] += to_string(Enumerator);
				results[v1][line1] += ":c";
				results[v1][line1] += to_string(count);
				results[v1][line1] += ":d";
				results[v1][line1] += to_string(distance1.getNumerator());
				if (distance1.getDenominator() != 1) {
					results[v1][line1] += '/';
					results[v1][line1] += to_string(distance1.getNumerator());
				}
				results[v1][line1] += ":i";
				results[v1][line1] += to_string(interval + 1);

				if (!results[v2][line2].empty()) {
					results[v2][line2] += " ";
				}
				results[v2][line2] += "n";
				results[v2][line2] += to_string(Enumerator);
				results[v2][line2] += ":c";
				results[v2][line2] += to_string(count);
				results[v2][line2] += ":d";
				results[v2][line2] += to_string(distance2.getNumerator());
				if (distance2.getDenominator() != 1) {
					results[v2][line2] += '/';
					results[v2][line2] += to_string(distance2.getNumerator());
				}
				results[v2][line2] += ":i";
				results[v2][line2] += to_string(interval + 1);

				if (m_mark) {
					for (int z=0; z<count; z++) {
						token1 = attacks[v1][i+z]->getToken();
						token2 = attacks[v2][j+z]->getToken();
						token1->setText(*token1 + m_marker);
						token2->setText(*token2 + m_marker);
					}
				}

			}
			// skip over match (need to do in i as well somehow)
			j += count;
		} // j loop
	} // i loop
}



///////////////////////////////
//
// Tool_immitation::compareSequences --
//

int Tool_immitation::compareSequences(vector<NoteCell*>& attack1, vector<double>& seq1, int i1,
		vector<NoteCell*>& attack2, vector<double>& seq2, int i2) {
	int count = 0;
	// sequences cannot start with rests
	if (Convert::isNaN(seq1[i1]) || Convert::isNaN(seq2[i2])) {
		return count;
	}

	HumNum dur1;
	HumNum dur2;

	while ((i1+count < (int)seq1.size()) && (i2+count < (int)seq2.size())) {

		if (m_duration) {
			dur1 = attack1[i1+count]->getDuration();
			dur2 = attack2[i2+count]->getDuration();
			if (dur1 != dur2) {
				break;
			}
		}
		
		if (Convert::isNaN(seq1[i1+count])) {
			if (Convert::isNaN(seq2[i2+count])) {
				count++;
				continue;
			} else {
				break;
			}
		} else if (Convert::isNaN(seq2[i2+count])) {
			break;
		} else if (seq1[i1+count] == seq2[i2+count]) {
			count++;
			continue;
		} else {
			break;
		}
	}

	return count;
}


// END_MERGE

} // end namespace hum



