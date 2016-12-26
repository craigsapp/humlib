//
// Programmer:    Alexander Morgan
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Aug  8 12:24:49 PDT 2015
// Last Modified: Sat Dec 24 15:48:01 PST 2016
// Filename:      tool-dissonant.cpp
// URL:           https://github.com/craigsapp/minHumdrum/blob/master/src/tool-dissonant.cpp
// Syntax:        C++11
// vim:           ts=3 noexpandtab
//
// Description:   Dissonance identification and labeling tool.
//

#include "tool-dissonant.h"
#include "Convert.h"

#include <algorithm>
#include <cmath>

using namespace std;

namespace hum {

// START_MERGE


/////////////////////////////////
//
// Tool_dissonant::Tool_dissonant -- Set the recognized options for the tool.
//

Tool_dissonant::Tool_dissonant(void) {
	define("r|raw=b",             "print raw grid");
	define("d|diatonic=b",        "print diatonic grid");
	define("D|no-dissonant=b",    "don't do dissonance anaysis");
	define("m|midi-pitch=b",      "print midi-pitch grid");
	define("b|base-40=b",         "print base-40 grid");
	define("l|metric-levels=b",   "use metric levels in analysis");
	define("k|kern=b",            "print kern pitch grid");
	define("debug=b",             "print grid cell information");
	define("e|exinterp=s:**data", "specify exinterp for **data spine");
	define("c|colorize=b",        "color dissonant notes");
}



/////////////////////////////////
//
// Tool_dissonant::run -- Do the main work of the tool.
//

bool Tool_dissonant::run(const string& indata, ostream& out) {
	HumdrumFile infile(indata);
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_dissonant::run(HumdrumFile& infile, ostream& out) {
	int status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_dissonant::run(HumdrumFile& infile) {
	NoteGrid grid(infile);

	if (getBoolean("debug")) {
		grid.printGridInfo(cerr);
		// return 1;
	} else if (getBoolean("raw")) {
		grid.printRawGrid(m_free_text);
		return 1;
	} else if (getBoolean("diatonic")) {
		grid.printDiatonicGrid(m_free_text);
		return 1;
	} else if (getBoolean("midi-pitch")) {
		grid.printMidiGrid(m_free_text);
		return 1;
	} else if (getBoolean("base-40")) {
		grid.printBase40Grid(m_free_text);
		return 1;
	} else if (getBoolean("kern")) {
		grid.printKernGrid(m_free_text);
		return 1;
	}

	diss2Q = false;
	diss7Q = false;
	diss4Q = false;

	vector<vector<string> > results;

	results.resize(grid.getVoiceCount());
	for (int i=0; i<(int)results.size(); i++) {
		results[i].resize(infile.getLineCount());
	}
	doAnalysis(results, grid, getBoolean("debug"));

	string exinterp = getString("exinterp");
	vector<HTp> kernspines = infile.getKernSpineStartList();
	infile.appendDataSpine(results.back(), "", exinterp);
	for (int i = (int)results.size()-1; i>0; i--) {
		int track = kernspines[i]->getTrack();
		infile.insertDataSpineBefore(track, results[i-1], "", exinterp);
	}

	if (getBoolean("colorize")) {
		if (diss2Q) {
			infile.appendLine("!!!RDF**kern: @ = dissonant 2nd, marked note, color=\"#33bb00\"");
		}
		if (diss7Q) {
			infile.appendLine("!!!RDF**kern: + = dissonant 7th, marked note, color=\"#0099ff\"");
		}
		if (diss4Q) {
			infile.appendLine("!!!RDF**kern: N = dissonant 4th marked note, color=\"#bb3300\"");
		}
	}
	infile.createLinesFromTokens();

	return true;
}



//////////////////////////////
//
// Tool_dissonant::doAnalysis -- do a basic melodic analysis of all parts.
//

void Tool_dissonant::doAnalysis(vector<vector<string> >& results,
		NoteGrid& grid, bool debug) {
	for (int i=0; i<grid.getVoiceCount(); i++) {
		doAnalysisForVoice(results[i], grid, i, debug);
	}
}



//////////////////////////////
//
// Tool_dissonant::doAnalysisForVoice -- do analysis for a single voice by
//     subtracting NoteCells to calculate the diatonic intervals.
//

void Tool_dissonant::doAnalysisForVoice(vector<string>& results, NoteGrid& grid,
		int vindex, bool debug) {
	vector<NoteCell*> attacks;
	grid.getNoteAndRestAttacks(attacks, vindex);

	if (debug) {
		cerr << "=======================================================";
		cerr << endl;
		cerr << "Note attacks for voice number "
		     << grid.getVoiceCount()-vindex << ":" << endl;
		for (int i=0; i<(int)attacks.size(); i++) {
			attacks[i]->printNoteInfo(cerr);
		}
	}
	bool nodissonanceQ = getBoolean("no-dissonant");
	bool colorizeQ = getBoolean("colorize");

	HumNum durp;     // duration of previous melodic note;
	HumNum dur;      // duration of current note;
	HumNum durn;     // duration of next melodic note;
	HumNum durnn;    // duration of next next melodic note;
	double intp;     // diatonic interval from previous melodic note
	double intn;     // diatonic interval to next melodic note
	double levp;     // metric level of the previous melodic note
	double lev;      // metric level of the current note
	double levn;     // metric level of the next melodic note
	int lineindex;   // line in original Humdrum file content that contains note
	int sliceindex;  // current timepoint in NoteGrid.
	vector<double> harmint(grid.getVoiceCount());  // harmonic intervals;
	bool dissonant;  // true if  note is dissonant with other sounding notes.
	char marking = '\0';
	
	for (int i=1; i<(int)attacks.size() - 1; i++) {
		sliceindex = attacks[i]->getSliceIndex();
		lineindex = attacks[i]->getLineIndex();
		marking = '\0';

		// calculate harmonic intervals:
		double lowestnote = 1000;
		double tpitch;
		for (int j=0; j<(int)harmint.size(); j++) {
			tpitch = grid.cell(j, sliceindex)->getAbsDiatonicPitch();
			if (!Convert::isNaN(tpitch)) {
				if (tpitch < lowestnote) {
					lowestnote = tpitch;
				}
			}
			if (j == vindex) {
				harmint[j] = 0;
			}
			if (j < vindex) {
				harmint[j] = *grid.cell(vindex, sliceindex) -
						*grid.cell(j, sliceindex);
			} else {
				harmint[j] = *grid.cell(j, sliceindex) -
						*grid.cell(vindex, sliceindex);
			}
		}

		// check if current note is dissonant to another sounding note:
		dissonant = false;
		for (int j=0; j<(int)harmint.size(); j++) {
			if (j == vindex) {
				// don't compare to self
				continue;
			}
			if (Convert::isNaN(harmint[j])) {
				// rest, so ignore
				continue;
			}
			int value = (int)harmint[j];
			if (value > 7) {
				value = value % 7; // remove octaves from interval
			} else if (value < -7) {
				value = -(-value % 7); // remove octaves from interval
			}

			if ((value == 1) || (value == -1)) {
				// forms a second with another sounding note
				dissonant = true;
				diss2Q = true;
				marking = '@';
				results[lineindex] = "d2";
				break;
			} else if ((value == 6) || (value == -6)) {
				// forms a seventh with another sounding note
				dissonant = true;
				diss7Q = true;
				marking = '+';
				results[lineindex] = "d7";
				break;
			}
		}
		double vpitch = grid.cell(vindex, sliceindex)->getAbsDiatonicPitch();
		if (vpitch - lowestnote > 0) {
			if (int(vpitch - lowestnote) % 7 == 3) {
				diss4Q = true;
				marking = 'N';
				results[lineindex] = "d4";
				dissonant = true;
			}
		}
	
		// Don't label current note if not dissonant with other sounding notes.
		if (!dissonant) {
			if (!nodissonanceQ) {
				continue;
			}
		}

		if (colorizeQ && marking) {
			// mark note
			string text = *attacks[i]->getToken();
			if (text.find(marking) == string::npos) {
				text += marking;
				attacks[i]->getToken()->setText(text);
			}
		}

		durp = attacks[i-1]->getDuration();
		dur  = attacks[i]->getDuration();
		durn = attacks[i+1]->getDuration();
		intp = *attacks[i] - *attacks[i-1];
		intn = *attacks[i+1] - *attacks[i];
		levp = attacks[i-1]->getMetricLevel();
		lev  = attacks[i]->getMetricLevel();
		levn = attacks[i+1]->getMetricLevel();

		if ((dur <= durp) && (lev >= levp) && (lev >= levn)) { // weak dissonances
			if (intp == -1) { // descending dissonances
				if (intn == -1) {
					results[lineindex] = "pd"; // downward passing tone
				} else if (intn == 1) {
					results[lineindex] = "nd"; // lower neighbor
				} else if (intn == 0) {
					results[lineindex] = "ad"; // descending anticipation
				} else if (intn > 1) {
					results[lineindex] = "ed"; // lower échappée
				} else if (intn == -2) {
					results[lineindex] = "cd"; // descending nota cambiata
				} else if (intn < -2) {
					results[lineindex] = "ipd"; // incomplete posterior lower neighbor
				}
			} else if (intp == 1) { // ascending dissonances
				if (intn == 1) {
					results[lineindex] = "pu"; // rising passing tone
				} else if (intn == -1) {
					results[lineindex] = "nu"; // upper neighbor
				} else if (intn < -1) {
					results[lineindex] = "eu"; // upper échappée
				} else if (intn == 0) {
					results[lineindex] = "au"; // rising anticipation
				} else if (intn == 2) {
					results[lineindex] = "cu"; // ascending nota cambiata
				} else if (intn > 2) {
					results[lineindex] = "ipu"; // incomplete posterior upper neighbor
				}
			} else if ((intp < -2) && (intn == 1)) {
				results[lineindex] = "iad"; // incomplete anterior lower neighbor
			} else if ((intp > 2) && (intn == -1)) {
				results[lineindex] = "iau"; // incomplete anterior upper neighbor
			}
		} else if ((durp >= 2) && (dur == 1) && (lev < levn) &&
			(intp == -1) && (intn == -1)) {
			results[lineindex] = "dq"; // dissonant third quarter
		} else if (i < ((int)attacks.size() - 2)) { // expand the analysis window
			double interval3 = *attacks[i+2] - *attacks[i+1];
			HumNum durnn = attacks[i+2]->getDuration();	// dur of note after next
			double levnn = attacks[i+2]->getMetricLevel(); // lev of note after next

			if ((dur == durn) && (lev == 1) && (levn == 2) && (levnn == 0) &&
				(intp == -1) && (intn == -1) && (interval3 == 1)) {
				results[lineindex] = "ci"; // chanson idiom
			}
		}
	}
}



// END_MERGE

} // end namespace hum



