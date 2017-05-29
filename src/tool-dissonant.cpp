//
// Programmer:    Alexander Morgan
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Aug  8 12:24:49 PDT 2015
// Last Modified: Wed May 24 15:39:19 CEST 2017
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
	define("p|percent=b",         "print counts as percentages");
	define("d|diatonic=b",        "print diatonic grid");
	define("D|no-dissonant=b",    "don't do dissonance anaysis");
	define("m|midi-pitch=b",      "print midi-pitch grid");
	define("b|base-40=b",         "print base-40 grid");
	define("l|metric-levels=b",   "use metric levels in analysis");
	define("k|kern=b",            "print kern pitch grid");
	define("debug=b",             "print grid cell information");
	define("u|undirected=b",      "use undirected dissonance labels");
	define("count=b",             "count dissonances by category");
	define("e|exinterp=s:**cdata","specify exinterp for **cdata spine");
	define("c|colorize=b",        "color dissonant notes by beat level");
	define("C|colorize2=b",       "color dissonant notes by dissonant interval");
}



/////////////////////////////////
//
// Tool_dissonant::run -- Do the main work of the tool.
//

bool Tool_dissonant::run(const string& indata, ostream& out) {

	if (getBoolean("undirected")) {
		fillLabels2();
	} else {
		fillLabels();
	}

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

	if (getBoolean("undirected")) {
		fillLabels2();
	} else {
		fillLabels();
	}

	int status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_dissonant::run(HumdrumFile& infile) {

	if (getBoolean("undirected")) {
		fillLabels2();
	} else {
		fillLabels();
	}

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

	dissL0Q = false;
	dissL1Q = false;
	dissL2Q = false;

	vector<vector<string> > results;

	results.resize(grid.getVoiceCount());
	for (int i=0; i<(int)results.size(); i++) {
		results[i].resize(infile.getLineCount());
	}
	doAnalysis(results, grid, getBoolean("debug"));

	if (getBoolean("count")) {
		printCountAnalysis(results);
		return false;
	} else {
		string exinterp = getString("exinterp");
		vector<HTp> kernspines = infile.getKernSpineStartList();
		infile.appendDataSpine(results.back(), "", exinterp);
		for (int i = (int)results.size()-1; i>0; i--) {
			int track = kernspines[i]->getTrack();
			infile.insertDataSpineBefore(track, results[i-1], "", exinterp);
		}

		printColorLegend(infile);
		infile.createLinesFromTokens();
		return true;
	}
}



//////////////////////////////
//
// Tool_dissonant::printColorLegend --
//

void Tool_dissonant::printColorLegend(HumdrumFile& infile) {
	if (getBoolean("colorize")) {
		if (dissL0Q) {
			infile.appendLine("!!!RDF**kern: N = strong dissonant marked note, color=\"#bb3300\"");
		}
		if (dissL1Q) {
			infile.appendLine("!!!RDF**kern: @ = weak 1 dissonant marked note, color=\"#33bb00\"");
		}
		if (dissL2Q) {
			infile.appendLine("!!!RDF**kern: + = weak 2 dissonant marked note, color=\"#0099ff\"");
		}
	} else if (getBoolean("colorize2")) {
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
}



//////////////////////////////
//
// Tool_dissonant::doAnalysis -- do a basic melodic analysis of all parts.
//

void Tool_dissonant::doAnalysis(vector<vector<string> >& results,
		NoteGrid& grid, bool debug) {
	vector<vector<NoteCell*> > attacks;
	attacks.resize(grid.getVoiceCount());

	for (int i=0; i<grid.getVoiceCount(); i++) {
		doAnalysisForVoice(results, grid, attacks[i], i, debug);
	}

	for (int i=0; i<grid.getVoiceCount(); i++) {
		findFakeSuspensions(results, grid, attacks[i], i);
	}

}



//////////////////////////////
//
// Tool_dissonant::doAnalysisForVoice -- do analysis for a single voice by
//     subtracting NoteCells to calculate the diatonic intervals.
//

void Tool_dissonant::doAnalysisForVoice(vector<vector<string> >& results, NoteGrid& grid,
		vector<NoteCell*>& attacks, int vindex, bool debug) {
	// vector<NoteCell*> attacks;
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
	bool colorize2Q = getBoolean("colorize2");

	HumNum durp;     // duration of previous melodic note;
	HumNum dur;      // duration of current note;
	HumNum durn;     // duration of next melodic note;
	double intp;     // diatonic interval from previous melodic note
	double intn;     // diatonic interval to next melodic note
	double levp;     // metric level of the previous melodic note
	double lev;      // metric level of the current note
	double levn;     // metric level of the next melodic note
	int lineindex;   // line in original Humdrum file content that contains note
	// int lineindexn;  // next line in original Humdrum file content that contains note
	int attackindexn;// slice in NoteGrid content that contains next note attack
	int sliceindex;  // current timepoint in NoteGrid.
	int oattackindexn = -1; // next note attack index of the other voice involved in the diss.
	vector<double> harmint(grid.getVoiceCount());  // harmonic intervals;
	bool dissonant;  // true if  note is dissonant with other sounding notes.
	char marking = '\0';
	int ovoiceindex = -1;
	string unexp_label; // default dissonance label if none of the diss types apply

	for (int i=1; i<(int)attacks.size() - 1; i++) {
		sliceindex = attacks[i]->getSliceIndex();
		lineindex = attacks[i]->getLineIndex();
		// lineindexn = attacks[i+1]->getLineIndex();
		attackindexn = attacks[i]->getNextAttackIndex();

		marking = '\0';

		// calculate harmonic intervals:
		int lowestnote = 1000;
		double tpitch;
		// int lowestnotei = -1;
		for (int j=0; j<(int)harmint.size(); j++) {
			tpitch = grid.cell(j, sliceindex)->getAbsDiatonicPitch();
			if (!Convert::isNaN(tpitch)) {
				if (tpitch <= lowestnote) {
					lowestnote = tpitch;
					// lowestnotei = j;
				}
			}
			if (j == vindex) {
				harmint[j] = 0;
			}

			harmint[j] = *grid.cell(j, sliceindex) -
					*grid.cell(vindex, sliceindex);

/*
			if (j < vindex) {
				harmint[j] = *grid.cell(vindex, sliceindex) -
						*grid.cell(j, sliceindex);
			} else {
				harmint[j] = *grid.cell(j, sliceindex) -
						*grid.cell(vindex, sliceindex);
			}
*/
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
			int vpitch = (int)grid.cell(vindex, sliceindex)->getAbsDiatonicPitch();
			int otherpitch = (int)grid.cell(j, sliceindex)->getAbsDiatonicPitch();

			if ((value == 1) || (value == -1)) {
				// forms a second with another sounding note
				dissonant = true;
				diss2Q = true;
				marking = '@';
				unexp_label = m_labels[UNLABELED_Z2];
				ovoiceindex = j;
				oattackindexn = getNextPitchAttackIndex(grid, ovoiceindex, sliceindex);
				break;
			} else if ((value == 6) || (value == -6)) {
				// forms a seventh with another sounding note
				dissonant = true;
				diss7Q = true;
				marking = '+';
				unexp_label = m_labels[UNLABELED_Z7];
				ovoiceindex = j;
				oattackindexn = getNextPitchAttackIndex(grid, ovoiceindex, sliceindex);
				break;
			} else if (((value == 3) && ((vpitch % 7) == (lowestnote % 7))) ||
			          ((value == -3) && ((otherpitch % 7) == (lowestnote % 7)))) {
				// If the harmonic interval between two notes is a fourth and the reference note
				// of the interval is the same pitch class as the lowest note (or is the lowest note);
				// or if the interval is a fourth with the voices crossed and the other pitch is
				// the same pitch class as the lowest note...
				dissonant = true;
				diss4Q = true;
				marking = 'N';
				// unexp_label = m_labels[UNLABELED_Z4];
				unexp_label = m_labels[UNLABELED_Z4];
				// ovoiceindex = lowestnotei;
				ovoiceindex = j;
				// oattackindexn = grid.cell(ovoiceindex, sliceindex)->getNextAttackIndex();
				oattackindexn = getNextPitchAttackIndex(grid, ovoiceindex, sliceindex);
			}
		}


/*
		double vpitch = grid.cell(vindex, sliceindex)->getAbsDiatonicPitch();
		if (vpitch - lowestnote > 0) {
			if (int(vpitch - lowestnote) % 7 == 3) {
				diss4Q = true;
				dissonant = true;
				marking = 'N';
				ovoiceindex = lowestnotei;
				oattackindexn = grid.cell(ovoiceindex, sliceindex)->getNextAttackIndex();
				unexp_label = m_labels[UNLABELED_Z4];
			}
		}
*/


		// Don't label current note if not dissonant with other sounding notes.
		if (!dissonant) {
			if (!nodissonanceQ) {
				continue;
			}
		}

		if (colorizeQ) {
			int metriclevel = attacks[i]->getMetricLevel();
			if (metriclevel <= 0) {
				dissL0Q = true;
				marking = 'N';
			} else if (metriclevel < 2) {
				dissL1Q = true;
				marking = '@';
			} else {
				dissL2Q = true;
				marking = '+';
			}
		}

		if ((colorizeQ || colorize2Q) && marking) {
			// mark note
			string text = *attacks[i]->getToken();
			if (text.find(marking) == string::npos) {
				text += marking;
				attacks[i]->getToken()->setText(text);
			}
		}

		// variables for dissonant voice
		durp = attacks[i-1]->getDuration();
		dur  = attacks[i]->getDuration();
		durn = attacks[i+1]->getDuration();
		intp = *attacks[i] - *attacks[i-1];
		intn = *attacks[i+1] - *attacks[i];
		levp = attacks[i-1]->getMetricLevel();
		lev  = attacks[i]->getMetricLevel();
		levn = attacks[i+1]->getMetricLevel();

		// Non-suspension test cases ////////////////////////////////////////////

		// valid_acc_exit determines if the other (accompaniment) voice conforms to the
		// standards of all dissonant types except suspensions.

		// The reference (dissonant) voice moves out of the dissonance to a different
		// pitch at the same time or before the other (accompaniment) voice moves to a
		// different pitch class or a rest:
		bool valid_acc_exit = oattackindexn < attackindexn ? false : true;
		if (oattackindexn < 0) {
			valid_acc_exit = true;
		}

		// Suspension test cases ////////////////////////////////////////////////

		// valid_sus_acc: determines if the reference voice conforms to the
		// standards of the accompaniment voice for suspensions.

		// Condition 1: The reference (accompaniment) voice moved to a different
		//    pitch class at the onset of this dissonant interval) &&
		bool condition1 = true;
		if (intp == 0) {
			// no repeated note attacks
			condition1 = false;
		}

		// Condition 2: The other (dissonant) voice stayed in place or repeated the
		//    same pitch at the onset of this dissonant interval.
		bool condition2 = true;
		double opitch = grid.cell(ovoiceindex, sliceindex)->getSgnMidiPitch();
		int lastonoteindex = grid.cell(ovoiceindex, sliceindex)->getPrevAttackIndex();
		double lopitch = NAN;
		if (lastonoteindex >= 0) {
			lopitch  = grid.cell(ovoiceindex, lastonoteindex)->getAbsMidiPitch();
		} else {
			condition2 = false;
		}
		if (opitch < 0) {
			condition2 = true;
		} else if (opitch != lopitch) {
			condition2 = false;
		}

		int oattackindexp = grid.cell(ovoiceindex, sliceindex)->getPrevAttackIndex();
		double opitchp = NAN;
		if (oattackindexp >= 0) {
			opitchp = grid.cell(ovoiceindex, oattackindexp)->getAbsDiatonicPitch();
		}

		opitch = grid.cell(ovoiceindex, sliceindex)->getAbsDiatonicPitch();
		int oattackindexn = grid.cell(ovoiceindex, sliceindex)->getNextAttackIndex();

		int olineindexn = -1;
		if (oattackindexn >= 0) {
			olineindexn = grid.cell(ovoiceindex, oattackindexn)->getLineIndex();
		}
		double opitchn = NAN;
		if (oattackindexn >= 0) {
			opitchn = grid.cell(ovoiceindex, oattackindexn)->getAbsDiatonicPitch();
		}
		int oattackindexnn = -1;
		if (oattackindexn >= 0) {
			oattackindexnn = grid.cell(ovoiceindex, oattackindexn)->getNextAttackIndex();
		}
		double opitchnn = NAN;
		if (oattackindexnn >= 0) {
			opitchnn = grid.cell(ovoiceindex, oattackindexnn)->getAbsDiatonicPitch();
		}

		// Condition 3: The other (dissonant) voice leaves its note before
		//    or at the same time as the accompaniment (reference) voice leaves
		//    its pitch class.  [The other (accompaniment) voice can leave its pitch
		//    class for another note or for a rest.]
		bool condition3a = oattackindexn <= attackindexn ? true : false;

		// deal with ornamented suspensions.
		bool condition3b = oattackindexnn <= attackindexn ? true : false;

		bool valid_sus_acc = condition1 && condition2 && condition3a;
		bool valid_ornam_sus_acc = condition1 && condition2 && condition3b;

		double ointp = opitch - opitchp;
		double ointn = opitchn - opitch;
		double ointnn = opitchnn - opitchn;

		if ((dur <= durp) && (lev >= levp) && (lev >= levn) && valid_acc_exit
			){ // weak dissonances
			if (intp == -1) { // descending dissonances
				if (intn == -1) {
					results[vindex][lineindex] = m_labels[PASSING_DOWN]; // downward passing tone
				} else if (intn == 1) {
					results[vindex][lineindex] = m_labels[NEIGHBOR_DOWN]; // lower neighbor
				} else if (intn == 0) {
					results[vindex][lineindex] = m_labels[ANT_DOWN]; // descending anticipation
				} else if (intn > 1) {
					results[vindex][lineindex] = m_labels[ECHAPPE_DOWN]; // lower échappée
				} else if (intn == -2) {
					results[vindex][lineindex] = m_labels[CAMBIATA_DOWN_S]; // descending short nota cambiata
				} else if (intn < -2) {
					results[vindex][lineindex] = m_labels[IPOSTLOW_NEIGHBOR]; // incomplete posterior lower neighbor
				}
			} else if (intp == 1) { // ascending dissonances
				if (intn == 1) {
					results[vindex][lineindex] = m_labels[PASSING_UP]; // rising passing tone
				} else if (intn == -1) {
					results[vindex][lineindex] = m_labels[NEIGHBOR_UP]; // upper neighbor
				} else if (intn < -1) {
					results[vindex][lineindex] = m_labels[ECHAPPE_UP]; // upper échappée
				} else if (intn == 0) {
					results[vindex][lineindex] = m_labels[ANT_UP]; // rising anticipation
				} else if (intn == 2) {
					results[vindex][lineindex] = m_labels[CAMBIATA_UP_S]; // ascending short nota cambiata
				} else if (intn > 2) {
					results[vindex][lineindex] = m_labels[IPOSTHI_NEIGHBOR]; // incomplete posterior upper neighbor
				}
			} else if ((intp < -2) && (intn == 1)) {
				results[vindex][lineindex] = m_labels[IANTLOW_NEIGHBOR]; // incomplete anterior lower neighbor
			} else if ((intp > 2) && (intn == -1)) {
				results[vindex][lineindex] = m_labels[IANTHI_NEIGHBOR]; // incomplete anterior upper neighbor
			}
		} else if ((durp >= 2) && (dur == 1) && (lev < levn) &&
			(intp == -1) && (intn == -1) && valid_acc_exit
			) {
			results[vindex][lineindex] = m_labels[THIRD_QUARTER]; // dissonant third quarter
		}


		else if (valid_sus_acc && (ointn == -1)) {
			results[vindex][lineindex] = m_labels[SUSPENSION_AGENT]; // the accompaniment voice of the suspension
			results[ovoiceindex][lineindex] = m_labels[SUSPENSION]; // suspension
		}

		else if (valid_sus_acc && ((ointn == 0) && (ointnn == -1))) {
			results[vindex][lineindex] = m_labels[SUSPENSION_AGENT]; // the accompaniment voice of the suspension
			results[ovoiceindex][lineindex] = m_labels[SUSPENSION]; // suspension
			results[ovoiceindex][olineindexn] = m_labels[SUSPENSION_REP]; // repeated-note of suspension
		}

		else if (valid_ornam_sus_acc && ((ointn == -2) && (ointnn == 1))) {
			results[vindex][lineindex] = m_labels[SUSPENSION_AGENT]; // the accompaniment voice of the suspension
			results[ovoiceindex][lineindex] = m_labels[SUSPENSION]; // suspension
			results[ovoiceindex][olineindexn] = m_labels[SUSPENSION_ORNAM]; // suspension ornament
		}

		if (i < ((int)attacks.size() - 2)) { // expand the analysis window

			double intnn = *attacks[i+2] - *attacks[i+1];
			HumNum durnn = attacks[i+2]->getDuration();	// dur of note after next
			double levnn = attacks[i+2]->getMetricLevel(); // lev of note after next

			if ((dur == durn) && (lev == 1) && (levn == 2) && (levnn == 0) &&
				(intp == -1) && (intn == -1) && (intnn == 1) && valid_acc_exit
				) {
				results[vindex][lineindex] = m_labels[CHANSON_IDIOM]; // chanson idiom

			} else if ((dur <= durp) && (lev >= levp) && (lev >= levn) &&
				(intp == -1) && (intn == -2) && (intnn == 1)) {
				results[vindex][lineindex] = m_labels[CAMBIATA_DOWN_L]; // long-form descending cambiata
			} else if ((dur <= durp) && (lev >= levp) && (lev >= levn) &&
				(intp == 1) && (intn == 2) && (intnn == -1)) {
				results[vindex][lineindex] = m_labels[CAMBIATA_UP_L]; // long-form ascending nota cambiata
			}
		}

		// Decide whether to give an unexplained dissonance label to the ref.
		// voice if none of the dissonant conditions above apply.
		if (results[vindex][lineindex] == "") { // ref. voice doesn't  have a diss label
			if ((Convert::isNaN(intp) && (not Convert::isNaN(ointp)) &&
				 (not Convert::isNaN(ointn))) ||
				 // ref. voice is approached or left by leap but the other voice resolves by step
				 (((abs(intp) > 1) || (abs(intn) > 1)) && (abs(ointn) == 1))) {
				continue;
			} else if (((not Convert::isNaN(intp)) && condition2) || // ref. voice repeated or moved into diss obliquely
				 (((condition1) && (ointp != 0)) && // both voices moved to new pitches at start of diss
				 (attackindexn <= oattackindexn)) // and the other voice doesn't leave diss. first
				){
				results[vindex][lineindex] = unexp_label;
			}
		}
	}
}



//////////////////////////////
//
// Tool_dissonant::findFakeSuspensions --
//

void Tool_dissonant::findFakeSuspensions(vector<vector<string> >& results, NoteGrid& grid,
		vector<NoteCell*>& attacks, int vindex) {
	double intp;        // diatonic interval from previous melodic note
	int lineindexn;     // line index of the next note in the voice
	bool sfound;        // boolean for if a suspension if found after a Z dissonance

	for (int i=1; i<(int)attacks.size()-1; i++) {
		int lineindex = attacks[i]->getLineIndex();
		if (results[vindex][lineindex].find("Z") == string::npos) {
			continue;
		}
		intp = *attacks[i] - *attacks[i-1];
		lineindexn = attacks[i+1]->getLineIndex();
		sfound = false;
		for (int j=lineindex + 1; j<=lineindexn; j++) {
			if ((results[vindex][j].compare(0, 1, "s") == 0) ||
			    (results[vindex][j].compare(0, 1, "S") == 0)) {
				sfound = true;
				break;
			}
		}
		if (!sfound) {
			continue;
		}
		// Also may need to check for the existance of another voice attacked before Z 
		// and sustained through to the beginning of the resolution.

		// Apply labels for normal fake suspensions.
		if (intp == 1) {
			results[vindex][lineindex] = m_labels[FAKE_SUSPENSION_UP];
		} else if (intp == -1) {
			results[vindex][lineindex] = m_labels[FAKE_SUSPENSION_DOWN];
		} else if (i > 1) { // as long as i > 1 intpp will be in range.
			// The next two fake suspension types are preceded by an anticipation.
			double intpp = *attacks[i-1] - *attacks[i-2];
			if ((intp == 0) && (intpp == 1)) {
				results[vindex][lineindex] = m_labels[FAKE_SUSPENSION_UP];
			} else if ((intp == 0) && (intpp == -1)) {
				results[vindex][lineindex] = m_labels[FAKE_SUSPENSION_DOWN];
			}
		}
	}
}



///////////////////////////////
//
// printCountAnalysis --
//

void Tool_dissonant::printCountAnalysis(vector<vector<string> >& data) {

	map<string, bool> reduced;
	bool brief = getBoolean("u");
	bool percentQ = getBoolean("percent");

	vector<map<string, int> > analysis;
	analysis.resize(data.size());
	int i;
	int j;
	for (i=0; i<(int)data.size(); i++) {
		for (j=0; j<(int)data[i].size(); j++) {
			if (analysis[i].find(data[i][j]) != analysis[i].end()) {
				analysis[i][data[i][j]]++;
			} else {
				analysis[i][data[i][j]] = 1;
			}
		}
	}

	m_humdrum_text << "**dis";
	if (brief) {
		m_humdrum_text << "u";
	}
	m_humdrum_text << "\t**sum";
	for (j=0; j<(int)analysis.size(); j++) {
		m_humdrum_text << "\t" << "**v" << j + 1;
	}
	m_humdrum_text << endl;

	int sumsum = 0;
	int sum;
	string item;
	for (i=0; i<(int)LABELS_SIZE; i++) {

		item = m_labels[i];

		if (brief && (reduced.find(item) != reduced.end())) {
			continue;
		}
		reduced[item] = 1;

		sum = 0;
		for (j=0; j<(int)analysis.size(); j++) {
			if (analysis[j].find(item) != analysis[j].end()) {
				sum += analysis[j][item];
				sumsum += analysis[j][item];
			}
		}

		if (sum == 0) {
			continue;
		}

		m_humdrum_text << item;
		m_humdrum_text << "\t" << sum;

		for (int j=0; j<(int)analysis.size(); j++) {
			m_humdrum_text << "\t";
			if (analysis[j].find(item) != analysis[j].end()) {
				if (percentQ) {
					m_humdrum_text << int(analysis[j][item] * 1.0 / sum * 1000.0 + 0.5) / 10.0;
				} else {
					m_humdrum_text << analysis[j][item];
				}
			} else {
				m_humdrum_text << 0;
			}
		}
		m_humdrum_text << endl;
	}

	m_humdrum_text << "*-\t*-";
	for (j=0; j<(int)analysis.size(); j++) {
		m_humdrum_text << "\t" << "*-";
	}
	m_humdrum_text << endl;

	m_humdrum_text << "!!total_dissonances:\t" << sumsum << endl;

}



//////////////////////////////
//
// Tool_dissonant::getNextPitchAttackIndex -- Get the [line] index of the next
//     note attack, excluding any repeated pitch note attacks.
//

int Tool_dissonant::getNextPitchAttackIndex(NoteGrid& grid, int voicei, int sliceindex) {
	double pitch = NAN;
	int endslice = -1;
	if (sliceindex >= 0) {
		pitch = grid.cell(voicei, sliceindex)->getAbsMidiPitch();
		endslice = grid.cell(voicei, sliceindex)->getNextAttackIndex();
	}

	double pitch2 = NAN;
	if (endslice >= 0) {
		pitch2 = grid.cell(voicei, endslice)->getAbsMidiPitch();
	}

	if (Convert::isNaN(pitch)) {
		return endslice;
	}

	while (pitch == pitch2) {
		endslice = grid.cell(voicei, endslice)->getNextAttackIndex();
		pitch2 = NAN;
		if (endslice >= 0) {
			pitch2 = grid.cell(voicei, endslice)->getAbsMidiPitch();
		} else {
			break;
		}
	}

	return endslice;
}



//////////////////////////////
//
// Tool_dissonant::fillLabels -- Assign the labels for non-harmonic tone analysis.
//

void Tool_dissonant::fillLabels(void) {
	m_labels.resize(LABELS_SIZE);
	m_labels[PASSING_UP          ] = "P"; // rising passing tone
	m_labels[PASSING_DOWN        ] = "p"; // downward passing tone
	m_labels[NEIGHBOR_UP         ] = "N"; // upper neighbor
	m_labels[NEIGHBOR_DOWN       ] = "n"; // lower neighbor
	m_labels[ECHAPPE_UP          ] = "E"; // upper échappée
	m_labels[ECHAPPE_DOWN        ] = "e"; // lower échappée
	m_labels[CAMBIATA_UP_S       ] = "C"; // ascending short nota cambiata
	m_labels[CAMBIATA_DOWN_S     ] = "c"; // descending short nota cambiata
	m_labels[CAMBIATA_UP_L       ] = "K"; // ascending long nota cambiata
	m_labels[CAMBIATA_DOWN_L     ] = "k"; // descending long nota cambiata
	m_labels[IPOSTHI_NEIGHBOR    ] = "J"; // incomplete posterior upper neighbor
	m_labels[IPOSTLOW_NEIGHBOR   ] = "j"; // incomplete posterior lower neighbor
	m_labels[IANTHI_NEIGHBOR     ] = "I"; // incomplete anterior upper neighbor
	m_labels[IANTLOW_NEIGHBOR    ] = "i"; // incomplete anterior lower neighbor
	m_labels[ANT_UP              ] = "A"; // rising anticipation
	m_labels[ANT_DOWN            ] = "a"; // descending anticipation
	m_labels[THIRD_QUARTER       ] = "Q"; // dissonant third quarter
	m_labels[SUSPENSION          ] = "s"; // suspension
	m_labels[SUSPENSION_AGENT    ] = "G"; // suspension agent
	m_labels[SUSPENSION_ORNAM    ] = "o"; // suspension ornament
	m_labels[SUSPENSION_REP      ] = "r"; // suspension repeated note
	m_labels[FAKE_SUSPENSION_UP  ] = "F"; // fake suspension approached by step up
	m_labels[FAKE_SUSPENSION_DOWN] = "f"; // fake suspension approached by step down
	m_labels[CHANSON_IDIOM       ] = "h"; // chanson idiom
	m_labels[UNKNOWN_DISSONANCE  ] = "Z"; // unknown dissonance
	m_labels[UNLABELED_Z2        ] = "Z2"; // unknown dissonance, 2nd interval
	m_labels[UNLABELED_Z7        ] = "Z7"; // unknown dissonance, 7th interval
	m_labels[UNLABELED_Z4        ] = "Z4"; // unknown dissonance, 4th interval
}



//////////////////////////////
//
// Tool_dissonant::fillLabels2 -- Assign the labels for non-harmonic tone analysis.
//     This version without direction separation.
//

void Tool_dissonant::fillLabels2(void) {
	m_labels.resize(LABELS_SIZE);
	m_labels[PASSING_UP          ] = "P"; // rising passing tone
	m_labels[PASSING_DOWN        ] = "P"; // downward passing tone
	m_labels[NEIGHBOR_UP         ] = "N"; // upper neighbor
	m_labels[NEIGHBOR_DOWN       ] = "N"; // lower neighbor
	m_labels[ECHAPPE_UP          ] = "E"; // upper échappée
	m_labels[ECHAPPE_DOWN        ] = "E"; // lower échappée
	m_labels[CAMBIATA_UP_S       ] = "C"; // ascending short nota cambiata
	m_labels[CAMBIATA_DOWN_S     ] = "C"; // descending short nota cambiata
	m_labels[CAMBIATA_UP_L       ] = "K"; // ascending long nota cambiata
	m_labels[CAMBIATA_DOWN_L     ] = "K"; // descending long nota cambiata
	m_labels[IPOSTHI_NEIGHBOR    ] = "J"; // incomplete posterior upper neighbor
	m_labels[IPOSTLOW_NEIGHBOR   ] = "J"; // incomplete posterior lower neighbor
	m_labels[IANTHI_NEIGHBOR     ] = "I"; // incomplete anterior upper neighbor
	m_labels[IANTLOW_NEIGHBOR    ] = "I"; // incomplete anterior lower neighbor
	m_labels[ANT_UP              ] = "A"; // rising anticipation
	m_labels[ANT_DOWN            ] = "A"; // descending anticipation
	m_labels[THIRD_QUARTER       ] = "Q"; // dissonant third quarter
	m_labels[SUSPENSION          ] = "S"; // suspension
	m_labels[SUSPENSION_AGENT    ] = "G"; // suspension agent
	m_labels[SUSPENSION_ORNAM    ] = "O"; // suspension ornament
	m_labels[SUSPENSION_REP      ] = "R"; // suspension repeated note
	m_labels[FAKE_SUSPENSION_UP  ] = "F"; // fake suspension approached by step up
	m_labels[FAKE_SUSPENSION_DOWN] = "F"; // fake suspension approached by step down
	m_labels[CHANSON_IDIOM       ] = "H"; // chanson idiom
	m_labels[UNKNOWN_DISSONANCE  ] = "Z"; // unknown dissonance
	m_labels[UNLABELED_Z2        ] = "Z"; // unknown dissonance, 2nd interval
	m_labels[UNLABELED_Z7        ] = "Z"; // unknown dissonance, 7th interval
	m_labels[UNLABELED_Z4        ] = "Z"; // unknown dissonance, 4th interval
}



// END_MERGE

} // end namespace hum



