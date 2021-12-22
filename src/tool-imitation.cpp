//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Jun 17 15:24:23 CEST 2017
// Last Modified: Fri Aug 11 18:25:50 EDT 2017
// Filename:      tool-imitation.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/tool-imitation.cpp
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Counterpoint imitation tool.
//
// Todo:          retrograde/inversion searches
//                imitation at specific rhythmic scaling (double, half, etc)
//                add inexact rhythm for first/last note in match
//                allow inexact rhythm after x notes with exact rhythm
//                color imitations by interval
//                terminate matches at rests
//                target must come before end of initiator match
//                count points of imitation that were found
//                create an index of points of imitation
//

#include "tool-imitation.h"
#include "Convert.h"
#include "HumRegex.h"

#include <algorithm>
#include <cmath>
#include <sstream>

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

	define("n|t|threshold=i:7",   "minimum number of notes to match");
	define("f|first=b",           "only give info for first sequence of matched pair");

	define("q|quiet|no-info=b",   "do not add spines giving information about matches");

	define("N|no-enumeration=b",  "do not display enumeration number");
	define("C|no-count=b",        "do not display note-count number");
	define("D|no-distance=b",     "do not display distance between first notes of sequences");
	define("I|no-interval=b",     "do not display interval transposite between sequences");

	define("NN|no-enumeration2=b", "do not display enumeration number on second sequence");
	define("CC|no-count2=b",       "do not display note-count number on second sequence");
	define("DD|no-distance2=b",    "do not display distance between first notes of sequences on second sequence");
	define("II|no-interval2=b",    "do not display interval transposition between sequences on second sequence");
	define("2|enumerate-second-only=b", "Display enumeration number on second sequence only (no count, distance, or interval");

	define("p|no-duration=b",     "pitch only when matching: do not consider duration");
	define("d|max-distance=d",    "maximum distance in quarter notes between imitations");
	define("s|single-mark=b",     "place a single mark on matched notes (not one for each match pair");
	define("r|rest=b",            "require match trigger to follow a rest");
	define("R|rest2=b",           "require match target to also follow a rest");
	define("i|intervals=s",       "require given interval sequence in imitation");
	define("M|no-mark=b",         "do not mark matched sequences");
	define("Z|no-zero=b",         "do not mark imitation starting at the same time");
	define("z|only-zero=b",       "Mark only imitation starting at the same time (parallel motion)");
	define("m|measure=b",         "Include measure number in imitation information");
	define("b|beat=b",            "Include beat number (really quarter-note number) in imitation information");
	define("l|length=b",          "Include length of imitation (in quarter-note units)");

	define("a|add=b",             "add inversions, retrograde, etc. if specified to normal search");
	define("v|inversion=b",       "match inversions");
	define("g|retrograde=b",      "match retrograde");
}



/////////////////////////////////
//
// Tool_imitation::run -- Do the main work of the tool.
//

bool Tool_imitation::run(HumdrumFileSet& infiles) {
	bool status = true;
	for (int i=0; i<infiles.getCount(); i++) {
		status &= run(infiles[i]);
	}
	return status;
}


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
	bool status = run(infile);
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

	Enumerator = 0;
	m_threshold = getInteger("threshold") + 1;
	if (m_threshold < 3) {
		m_threshold = 3;
	}

	m_nozero       = getBoolean("no-zero");
	m_onlyzero     = getBoolean("only-zero");
	if (m_nozero && m_onlyzero) {
		// only one of these two options can be used at the same time
		m_nozero = false;
	}
	m_maxdistanceQ = getBoolean("max-distance");
	m_maxdistance  = getDouble("max-distance");
	m_duration     = !getBoolean("no-duration");
	m_measure      = getBoolean("measure");
	m_length       = getBoolean("length");
	m_beat         = getBoolean("beat");
	if (m_measure) {
		m_barlines = infile.getMeasureNumbers();
	}

	m_noInfo = getBoolean("quiet");

	m_noN    = getBoolean("N");
	m_noC    = getBoolean("C");
	m_noD    = getBoolean("D");
	m_noI    = getBoolean("I");

	m_noNN   = getBoolean("NN");
	m_noCC   = getBoolean("CC");
	m_noDD   = getBoolean("DD");
	m_noII   = getBoolean("II");

	m_inversion  = getBoolean("inversion");
	m_retrograde = getBoolean("retrograde");

	m_addsearches = false;
	if (getBoolean("add")) {
		m_inversion = false;
		m_retrograde = false;
		m_addsearches = true;
	}

	if (getBoolean("enumerate-second-only")) {
		m_noNN = false;
		m_noCC = true;
		m_noDD = true;
		m_noII = true;
	}

	m_first   = getBoolean("first");
	m_mark    = !getBoolean("no-mark");
	m_rest    = getBoolean("rest");
	m_rest2   = getBoolean("rest2");
	m_single  = getBoolean("single-mark");

	if (getBoolean("intervals")) {
		vector<string> values;
		HumRegex hre;
		string intstring = getString("intervals");
		hre.split(values, intstring.c_str(), "[^0-9+-]+");
		m_intervals.resize(values.size());
		for (int i=0; i<(int)values.size(); i++) {
			m_intervals.at(i) = stoi(values.at(i));
			// subtract one since intervals in caluculations are zero-indexed:
			if (m_intervals.at(i) > 0) {
				m_intervals.at(i)--;
			} else if (m_intervals.at(i) < 0) {
				m_intervals.at(i)++;
			}
		}
	}

	vector<vector<string>>    results;
	vector<vector<NoteCell*>> attacks;
	vector<vector<double>>    intervals;

	doAnalysis(results, grid, attacks, intervals, infile, getBoolean("debug"));
	char originalMarker = m_marker;
	if (m_addsearches && getBoolean("inversion")) {
		m_inversion = true;
		m_marker = 'N';
		doAnalysis(results, grid, attacks, intervals, infile, getBoolean("debug"));
	}
	m_marker = originalMarker;

	if (!getBoolean("no-info")) {
		string exinterp = getString("exinterp");
		vector<HTp> kernspines = infile.getKernSpineStartList();
		infile.appendDataSpine(results.back(), "", exinterp);
		for (int i = (int)results.size()-1; i>0; i--) {
			int track = kernspines.at(i)->getTrack();
			infile.insertDataSpineBefore(track, results.at(i-1), "", exinterp);
		}
	}
	if (m_mark && Enumerator) {
		string rdfline = "!!!RDF**kern: ";
		rdfline += m_marker;
		rdfline += " = marked note (color=\"chocolate\")";
		infile.appendLine(rdfline);
		if (getBoolean("add") && getBoolean("inversion")) {
			rdfline = "!!!RDF**kern: ";
			rdfline += "N";
			rdfline += " = marked note (color=\"limegreen\"), inversion match";
			infile.appendLine(rdfline);
		}
	}
	infile.createLinesFromTokens();
	// new data spines not showing up after createLinesFromTokens(), so force to text for now:
	m_humdrum_text << infile;
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
		results.at(i).resize(infile.getLineCount());
	}

	attacks.resize(grid.getVoiceCount());
	for (int i=0; i<(int)attacks.size(); i++) {
		grid.getNoteAndRestAttacks(attacks.at(i), i);
	}

	intervals.resize(grid.getVoiceCount());
	for (int i=0; i<(int)intervals.size(); i++) {
		intervals.at(i).resize(attacks.at(i).size());
		getIntervals(intervals.at(i), attacks.at(i));
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
		intervals.at(i) = *attacks.at(i+1) - *attacks.at(i);
	}
	intervals.back() = NAN;

	if (getBoolean("debug")) {
		cout << endl;
		for (int i=0; i<(int)intervals.size(); i++) {
			cout << "INTERVAL " << i << "\t=\t" << intervals.at(i) << "\tATK "
			     << attacks.at(i)->getSgnDiatonicPitch() << "\t" << attacks.at(i)->getToken() << endl;
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

	vector<NoteCell*>& v1a = attacks.at(v1);
	vector<NoteCell*>& v2a = attacks.at(v2);
	vector<double>& v1i = intervals.at(v1);
	vector<double>& v2i = intervals.at(v2);

	int min = m_threshold - 1;
	int count;

	vector<int> enum1(v1a.size(), 0);
	vector<int> enum2(v2a.size(), 0);

	for (int i=0; i<(int)v1i.size() - 1; i++) {
		count = 0;
		for (int j=0; j<(int)v2i.size() - 1; j++) {
			if (m_rest || m_rest2) {
				if ((i > 0) && (!Convert::isNaN(attacks.at(v1).at(i-1)->getSgnDiatonicPitch()))) {
					// match initiator must be preceded by a rest (or start of music)
					continue;
				}
			}
			if (m_rest2) {
				if ((j > 0) && (!Convert::isNaN(attacks.at(v2).at(j-1)->getSgnDiatonicPitch()))) {
					// match target must be preceded by a rest (or start of music)
					continue;
				}
			}
			if ((enum1.at(i) != 0) && (enum1.at(i) == enum2.at(j))) {
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
			HTp token1 = attacks.at(v1).at(i)->getToken();
			HTp token2 = attacks.at(v2).at(j)->getToken();
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
				enum1.at(i+k) = Enumerator;
				enum2.at(j+k) = Enumerator;
			}

			int interval = int(*attacks.at(v2).at(j) - *attacks.at(v1).at(i));

			if (!m_noInfo) {
				if (!(m_first && (distance1 < 0))) {
					int line1 = attacks.at(v1).at(i)->getLineIndex();
					if (!results.at(v1).at(line1).empty()) {
						results.at(v1).at(line1) += " ";
					}

					bool data = false;

					if (!m_noN) {
						data = true;
						if (m_inversion) {
							results.at(v1).at(line1) += "v";
						} else if (m_retrograde) {
							results.at(v1).at(line1) += "r";
						} else {
							results.at(v1).at(line1) += "n";
						}
						results.at(v1).at(line1) += to_string(Enumerator);
					}

					if (m_measure) {
						if (data) {
							results.at(v1).at(line1) += ":";
						}
						data = true;
						results.at(v1).at(line1) += "m";
						int line = attacks.at(v1).at(i)->getToken()->getLineIndex();
						results.at(v1).at(line1) += to_string(m_barlines[line]);
					}

					if (m_beat) {
						if (data) {
							results.at(v1).at(line1) += ":";
						}
						data = true;
						results.at(v1).at(line1) += "b";
						HLp humline = attacks.at(v1).at(i)->getToken()->getOwner();
						stringstream ss;
						ss.str("");
						ss << humline->getBeat().getFloat();
						results.at(v1).at(line1) += ss.str();
					}

					if (m_length) {
						if (data) {
							results.at(v1).at(line1) += ":";
						}
						data = true;
						results.at(v1).at(line1) += "L";
						// time1 is the starttime
						HumNum endtime;
						HTp endtoken = NULL;
						if (i+count < (int)attacks.at(v1).size()) {
							endtoken = attacks.at(v1).at(i+count)->getToken();
							endtime = endtoken->getDurationFromStart();
						} else {
							endtime = token1->getOwner()->getOwner()->getScoreDuration();
						}
						HumNum duration = endtime - time1;
						stringstream ss;
						ss.str("");
						ss << duration.getFloat();
						results.at(v1).at(line1) += ss.str();
					}

					if (!m_noC) {
						if (data) {
							results.at(v1).at(line1) += ":";
						}
						data = true;
						results.at(v1).at(line1) += "c";
						results.at(v1).at(line1) += to_string(count);
					}

					if (!m_noD) {
						if (data) {
							results.at(v1).at(line1) += ":";
						}
						data = true;
						results.at(v1).at(line1) += "d";
						// maybe allow fractions?
						results.at(v1).at(line1) += to_string(distance1.getNumerator());
					}

					if (!m_noI) {
						if (data) {
							results.at(v1).at(line1) += ":";
						}
						data = true;
						if (distance1.getDenominator() != 1) {
							results.at(v1).at(line1) += '/';
							results.at(v1).at(line1) += to_string(distance1.getNumerator());
						}
						results.at(v1).at(line1) += "i";
						if (interval > 0) {
							results.at(v1).at(line1) += to_string(interval + 1);
						} else {
							int newinterval = -(interval + 1);
							if (newinterval == -1) {
								newinterval = 1; // unison (no sign)
							}
							results.at(v1).at(line1) += to_string(newinterval);
						}
					}
				}

				if (!(m_first && (distance2 <= 0))) {
					int line2 = attacks.at(v2).at(j)->getLineIndex();

					if (!results.at(v2).at(line2).empty()) {
						results.at(v2).at(line2) += " ";
					}

					bool data2 = false;

					if ((!m_noN) && (!m_noNN)) {
						data2 = true;
						if (m_inversion) {
							results.at(v2).at(line2) += "v";
						} else if (m_retrograde) {
							results.at(v2).at(line2) += "r";
						} else {
							results.at(v2).at(line2) += "n";
						}
						results.at(v2).at(line2) += to_string(Enumerator);
					}

					if (m_measure) {
						if (data2) {
							results.at(v2).at(line2) += ":";
						}
						data2 = true;
						results.at(v2).at(line2) += "m";
						int line = attacks.at(v2).at(j)->getToken()->getLineIndex();
						results.at(v2).at(line2) += to_string(m_barlines[line]);
					}

					if (m_beat) {
						if (data2) {
							results.at(v2).at(line2) += ":";
						}
						data2 = true;
						results.at(v2).at(line2) += "b";
						HLp humline = attacks.at(v2).at(j)->getToken()->getOwner();
						stringstream ss;
						ss.str("");
						ss << humline->getBeat().getFloat();
						results.at(v2).at(line2) += ss.str();
					}

					if (m_length) {
						if (data2) {
							results.at(v2).at(line2) += ":";
						}
						data2 = true;
						results.at(v2).at(line2) += "L";
						// time1 is the starttime
						HumNum endtime;
						HTp endtoken = NULL;
						if (j+count < (int)attacks.at(v2).size()) {
							endtoken = attacks.at(v2).at(j+count)->getToken();
							endtime = endtoken->getDurationFromStart();
						} else {
							endtime = token2->getOwner()->getOwner()->getScoreDuration();
						}
						HumNum duration = endtime - time2;
						stringstream ss;
						ss.str("");
						ss << duration.getFloat();
						results.at(v2).at(line2) += ss.str();
					}

					if ((!m_noC) && (!m_noCC)) {
						if (data2) {
							results.at(v2).at(line2) += ":";
						}
						data2 = true;
						results.at(v2).at(line2) += "c";
						results.at(v2).at(line2) += to_string(count);
					}

					if ((!m_noD) && (!m_noDD)) {
						if (data2) {
							results.at(v2).at(line2) += ":";
						}
						data2 = true;
						results.at(v2).at(line2) += "d";
						results.at(v2).at(line2) += to_string(distance2.getNumerator());
					}

					if ((!m_noI) && (!m_noII)) {
						if (data2) {
							results.at(v2).at(line2) += ":";
						}
						data2 = true;
						if (distance2.getDenominator() != 1) {
							results.at(v2).at(line2) += '/';
							results.at(v2).at(line2) += to_string(distance2.getNumerator());
						}
						results.at(v2).at(line2) += "i";
						if (interval > 0) {
							int newinterval = -(interval + 1);
							if (newinterval == -1) {
								newinterval = 1; // unison (no sign)
							}
							results.at(v2).at(line2) += to_string(newinterval);
						} else {
							results.at(v2).at(line2) += to_string(interval + 1);
						}
					}
				}
			}

			if (m_mark) {
				for (int z=0; z<count; z++) {
					if (i+z >= (int)attacks.at(v1).size()) {
						break;
					}
					token1 = attacks.at(v1).at(i+z)->getToken();
					if (j+z >= (int)attacks.at(v2).size()) {
						break;
					}
					token2 = attacks.at(v2).at(j+z)->getToken();
					if (m_single) {
						if (token1->find(m_marker) == string::npos) {
							token1->setText(*token1 + m_marker);
						}
						if (token2->find(m_marker) == string::npos) {
							token2->setText(*token2 + m_marker);
						}
					} else {
						token1->setText(*token1 + m_marker);
						token2->setText(*token2 + m_marker);
					}

               if (attacks.at(v1).at(i+z)->isRest() && (z < count - 1) ) {
						markedTiedNotes(attacks.at(v1).at(i+z)->m_tiedtokens);
					} else if (!attacks.at(v1).at(i+z)->isRest()) {
						markedTiedNotes(attacks.at(v1).at(i+z)->m_tiedtokens);
					}

               if (attacks.at(v2).at(j+z)->isRest() && (z < count - 1) ) {
						markedTiedNotes(attacks.at(v2).at(j+z)->m_tiedtokens);
					} else if (!attacks.at(v2).at(j+z)->isRest()) {
						markedTiedNotes(attacks.at(v2).at(j+z)->m_tiedtokens);
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
		if (m_single) {
			if (tokens.at(i)->find(m_marker) == string::npos) {
				tokens.at(i)->setText(*tokens.at(i) + m_marker);
			}
		} else {
			tokens.at(i)->setText(*tokens.at(i) + m_marker);
		}
	}
}



//////////////////////////////
//
// Tool_imitation::checkForIntervalSequence --
//

int Tool_imitation::checkForIntervalSequence(vector<int>& m_intervals,
		vector<double>& v1i, int starti, int count) {

	int endi = starti + count - (int)m_intervals.size();
	for (int i=starti; i<endi; i++) {
		for (int j=0; j<(int)m_intervals.size(); j++) {
			if (m_intervals.at(j) != v1i.at(i+j)) {
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
// Tool_imitation::compareSequences -- Returns the number of notes that
//     match between the two sequences (which is one more than the
//     interval count).
//

int Tool_imitation::compareSequences(vector<NoteCell*>& attack1,
		vector<double>& seq1, int i1, vector<NoteCell*>& attack2,
		vector<double>& seq2, int i2) {
	int count = 0;
	// sequences cannot start with rests
	if (Convert::isNaN(seq1.at(i1)) || Convert::isNaN(seq2.at(i2))) {
		return count;
	}
	if (m_nozero) {
		// exclude matches that start at the same time.
		if (attack1.at(i1)->getToken()->getDurationFromStart() == attack2.at(i2)->getToken()->getDurationFromStart()) {
			return count;
		}
	} else if (m_onlyzero) {
		// exclude matches that do not start at the same time (parallel motion).
		if (attack1.at(i1)->getToken()->getDurationFromStart() != attack2.at(i2)->getToken()->getDurationFromStart()) {
			return count;
		}
	}

	HumNum dur1;
	HumNum dur2;

	while ((i1+count < (int)seq1.size()) && (i2+count < (int)seq2.size())) {

		if (m_duration) {
			dur1 = attack1.at(i1+count)->getDuration();
			dur2 = attack2.at(i2+count)->getDuration();
			if (dur1 != dur2) {
				break;
			}
		}

		if (Convert::isNaN(seq1.at(i1+count))) {
			// the first voice's interval is to/from a rest
			if (Convert::isNaN(seq2.at(i2+count))) {
				// The seoncd voice's interval is also to/from a rest,
				// so increment count and continue.
				count++;
				continue;
			} else {
				// The second voice's interval is not to/from a rest,
				// so return the current count.
				if (count) {
					return count + 1;
				} else {
					return count;
				}
			}
		} else if (Convert::isNaN(seq2.at(i2+count))) {
			// The second voice's interval is to/from a rest
			// but already know that the first one is not, so return
			// current count;
			if (count) {
				return count + 1;
			} else {
				return count;
			}
		} else if (m_inversion && (seq1.at(i1+count) == -seq2.at(i2+count))) {
         // The two sequences match as inversions at this point, so continue.
			count++;
		} else if ((!m_inversion) && (seq1.at(i1+count) == seq2.at(i2+count))) {
         // The two sequences match at this point, so continue.
			count++;
			continue;
		} else {
			// The sequences do not match so return the current count.
			if (count) {
				return count + 1;
			} else {
				return count;
			}
		}
	}

	if (count) {
		// don't add one for some reaason (this will cause out-of-bounds)
		return count;
	} else {
		return count;
	}
}


// END_MERGE

} // end namespace hum



