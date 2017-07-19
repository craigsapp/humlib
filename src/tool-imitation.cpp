//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Jun 17 15:24:23 CEST 2017
// Last Modified: Sat Jul  8 17:17:21 CEST 2017
// Filename:      tool-imitation.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/tool-imitation.cpp
// Syntax:        C++11
// vim:           ts=3 noexpandtab
//
// Description:   Counterpoint imitation tool.
//
// Todo:          retrograde searches
//                imitation at specific rhythmic scaling
//                highlight tied notes in matches
//                color imitations by interval
//                terminate matches at rests
//                match must come within a specified duration
//                target must come before end of initiator match
//                all inexact rhythm for last note in match
//                allow inexact rhythm after x notes with exact rhythm
//                count points of imitation that were found
//                create an index of points of imitation

#include "tool-imitation.h"
#include "Convert.h"
#include "HumRegex.h"

#include <algorithm>
#include <cmath>

using namespace std;

namespace hum {

// START_MERGE


int Tool_imitation::Enumerator = 0;


/////////////////////////////////
//
// Tool_imitation::Tool_imitation -- Set the recognized options for the tool.
//

Tool_imitation::Tool_imitation(void) {
	define("debug=b",             "print grid cell information");
	define("e|exinterp=s:**vvdata","specify exinterp for **vvdata spine");
	define("n|threshold=i:7",     "minimum number of notes to match");
	define("D|no-duration=b",     "do not consider duration when matching");
	define("d|max-distance=d",    "maximum distance in quarter notes between imitations");
	define("r|rest=b",            "require match trigger to follow a rest");
	define("R|rest2=b",           "require match target to also follow a rest");
	define("i|intervals=s",       "require given interval sequence in imitation");
	define("M|no-mark=b",         "do not mark matched sequences");
}



/////////////////////////////////
//
// Tool_imitation::run -- Do the main work of the tool.
//

bool Tool_imitation::run(const string& indata, ostream& out) {

	HumdrumFile infile(indata);
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_imitation::run(HumdrumFile& infile, ostream& out) {
	int status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_imitation::run(HumdrumFile& infile) {
	Enumerator = 0;

	NoteGrid grid(infile);

	if (getBoolean("debug")) {
		grid.printGridInfo(cerr);
		// return 1;
	} 

	m_threshold = getInteger("threshold") + 1;
	if (m_threshold < 3) {
		m_threshold = 3;
	}

	m_maxdistanceQ = getBoolean("max-distance");
	m_maxdistance = getDouble("max-distance");
	m_duration = !getBoolean("no-duration");
	m_mark     = !getBoolean("no-mark");
	m_rest     = getBoolean("rest");
	m_rest2    = getBoolean("rest2");
	if (getBoolean("intervals")) {
		vector<string> values;
		HumRegex hre;
		string intstring = getString("intervals");
		hre.split(values, intstring.c_str(), "[^0-9+-]+");
		m_intervals.resize(values.size());
		for (int i=0; i<(int)values.size(); i++) {
			m_intervals[i] = stoi(values[i]);
			// subtract one since intervals in caluculations are zero-indexed:
			if (m_intervals[i] > 0) {
				m_intervals[i]--;
			} else if (m_intervals[i] < 0) {
				m_intervals[i]++;
			}
		}
	}

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
// Tool_imitation::doAnalysis -- do a basic melodic analysis of all parts.
//

void Tool_imitation::doAnalysis(vector<vector<string> >& results,
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
			analyzeImitation(results, attacks, intervals, i, j);
		}
	}
}



///////////////////////////////
//
// Tool_imitation::getIntervals --
//

void Tool_imitation::getIntervals(vector<double>& intervals,
		vector<NoteCell*>& attacks) {
	for (int i=0; i<(int)attacks.size() - 1; i++) {
		intervals[i] = *attacks[i+1] - *attacks[i];
	}
	intervals.back() = NAN;

	if (getBoolean("debug")) {
		cout << endl;
		for (int i=0; i<(int)intervals.size(); i++) {
			cout << "INTERVAL " << i << "\t=\t" << intervals[i] << "\tATK " 
			     << attacks[i]->getSgnDiatonicPitch() << "\t" << attacks[i]->getToken() << endl;
		}
	}

}



//////////////////////////////
//
// Tool_imitation::analyzeImitation -- do imitation analysis between two voices.
//

void Tool_imitation::analyzeImitation(vector<vector<string>>& results,
		vector<vector<NoteCell*>>& attacks, vector<vector<double>>& intervals,
		int v1, int v2) {

	vector<NoteCell*>& v1a = attacks[v1];
	vector<NoteCell*>& v2a = attacks[v2];
	vector<double>& v1i = intervals[v1];
	vector<double>& v2i = intervals[v2];

	int min = m_threshold - 1;
	int count;

	vector<int> enum1(v1a.size(), 0);
	vector<int> enum2(v2a.size(), 0);

	for (int i=0; i<(int)v1i.size() - 1; i++) {
		for (int j=0; j<(int)v2i.size() - 1; j++) {
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
			if ((count >= min) && (m_intervals.size() > 0)) {
				count = checkForIntervalSequence(m_intervals, v1i, i, count);
			}
			if (count < min) {
				j += count;
				continue;
			}

			// cout << "Match length count " << count << endl;
			HTp token1 = attacks[v1][i]->getToken();
			HTp token2 = attacks[v2][j]->getToken();
			HumNum time1 = token1->getDurationFromStart();
			HumNum time2 = token2->getDurationFromStart();
			HumNum distance1 = time2 - time1;
			HumNum distance2 = time1 - time2;

			if (m_maxdistanceQ && (distance1.getAbs().getFloat() > m_maxdistance)) {
				j += count;
				continue;
			}

			Enumerator++;
			for (int k=0; k<count; k++) {
				enum1[i+k] = Enumerator;
				enum2[j+k] = Enumerator;
			}

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

               if (attacks[v1][i+z]->isRest() && (z < count - 1) ) {
						markedTiedNotes(attacks[v1][i+z]->m_tiedtokens);
					} else if (!attacks[v1][i+z]->isRest()) {
						markedTiedNotes(attacks[v1][i+z]->m_tiedtokens);
					}

               if (attacks[v2][j+z]->isRest() && (z < count - 1) ) {
						markedTiedNotes(attacks[v2][j+z]->m_tiedtokens);
					} else if (!attacks[v2][j+z]->isRest()) {
						markedTiedNotes(attacks[v2][j+z]->m_tiedtokens);
					}

				}
			}

			// skip over match (need to do in i as well somehow)
			j += count;
		} // j loop
	} // i loop
}



//////////////////////////////
//
// Tool_imitation::markedTiedNotes --
//

void Tool_imitation::markedTiedNotes(vector<HTp>& tokens) {
	for (int i=0; i<(int)tokens.size(); i++) {
		tokens[i]->setText(*tokens[i] + m_marker);
	}
}



//////////////////////////////
//
// Tool_imitation::checkForIntervalSequence --
//

int Tool_imitation::checkForIntervalSequence(vector<int>& m_intervals,
		vector<double>& v1i, int starti, int count) {

	int endi = starti + count - m_intervals.size();
	for (int i=starti; i<endi; i++) {
		for (int j=0; j<(int)m_intervals.size(); j++) {
			if (m_intervals[j] != v1i[i+j]) {
				break;
			}
			if (j == (int)m_intervals.size() - 1) {
				// successfully found the interval pattern in imitation
				return count;
			}
		}
	}

	// pattern was not found so say that there was no match
	return 0;
}



///////////////////////////////
//
// Tool_imitation::compareSequences --
//

int Tool_imitation::compareSequences(vector<NoteCell*>& attack1,
		vector<double>& seq1, int i1, vector<NoteCell*>& attack2,
		vector<double>& seq2, int i2) {
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
			// the first voice's interval is to/from a rest
			if (Convert::isNaN(seq2[i2+count])) {
				// The seoncd voice's interval is also to/from a rest,
				// so increment count and continue.
				count++;
				continue;
			} else {
				// The second voice's interval is not to/from a rest,
				// so return the current count.
				return count;
			}
		} else if (Convert::isNaN(seq2[i2+count])) {
			// The second voice's interval is to/from a rest
			// but already know that the first one is not, so return
			// current count;
			return count;
			break;
		} else if (seq1[i1+count] == seq2[i2+count]) {
         // The two sequences match at this point, so continue.
			count++;
			continue;
		} else {
			// The sequences do not match so return the current count.
			return count;
		}
	}

	return count;
}


// END_MERGE

} // end namespace hum



