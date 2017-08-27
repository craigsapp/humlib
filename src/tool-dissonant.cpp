//
// Programmer:    Alexander Morgan
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Aug  8 12:24:49 PDT 2015
// Last Modified: Wed May 24 15:39:19 CEST 2017
// Filename:      tool-dissonant.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/tool-dissonant.cpp
// Syntax:        C++11
// vim:           ts=3 noexpandtab
//
// Description:   Dissonance identification and labeling tool.
//

#include "tool-dissonant.h"
#include "Convert.h"
#include "HumRegex.h"

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
	define("s|suppress=b",        "suppress dissonant notes");
	define("d|diatonic=b",        "print diatonic grid");
	define("D|no-dissonant=b",    "don't do dissonance anaysis");
	define("m|midi-pitch=b",      "print midi-pitch grid");
	define("b|base-40=b",         "print base-40 grid");
	define("l|metric-levels=b",   "use metric levels in analysis");
	define("k|kern=b",            "print kern pitch grid");
	define("V|voice-functions=b", "do cadential-voice-function analysis");
	define("v|voice-number=b",    "print voice number of dissonance");
	define("f|self-number=b",     "print self voice number of dissonance");
	define("debug=b",             "print grid cell information");
	define("u|undirected=b",      "use undirected dissonance labels");
	define("c|count=b",           "count dissonances by category");
	define("e|exinterp=s:**cdata","specify exinterp for **cdata spine");
	define("color|colorize=b",    "color dissonant notes by beat level");
	define("color2|colorize2=b",  "color dissonant notes by dissonant interval");
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

	if (getBoolean("voice-number")) {
		m_voicenumQ = true;
	}
	if (getBoolean("self-number")) {
		m_selfnumQ = true;
	}

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

	suppressQ = getBoolean("suppress");
	voiceFuncsQ = getBoolean("voice-functions");

	vector<vector<string> > results;
	vector<vector<string> > results2;
	vector<vector<string> > voiceFuncs;
	vector<vector<NoteCell*> > attacks;
	vector<vector<NoteCell*> > attacks2;

	attacks.resize(grid.getVoiceCount());
	results.resize(grid.getVoiceCount());
	for (int i=0; i<(int)results.size(); i++) {
		results[i].resize(infile.getLineCount());
	}
	doAnalysis(results, grid, attacks, getBoolean("debug"));

	if (suppressQ) {
		suppressDissonances(infile, grid, attacks, results);

		NoteGrid grid2(infile);
		results2.resize(grid2.getVoiceCount());
		for (int i=0; i<(int)results2.size(); i++) {
			results2[i].clear();
			results2[i].resize(infile.getLineCount());
		}
		vector<vector<NoteCell*> > attacks2;
		doAnalysis(results2, grid2, attacks2, getBoolean("debug"));

	}

	if (suppressQ) {
		if (getBoolean("count")) {
			printCountAnalysis(results2);
			return false;
		} else {
			string exinterp = getString("exinterp");
			vector<HTp> kernspines = infile.getKernSpineStartList();
			infile.appendDataSpine(results2.back(), "", exinterp);
			for (int i = (int)results2.size()-1; i>0; i--) {
				int track = kernspines[i]->getTrack();
				infile.insertDataSpineBefore(track, results2[i-1], "", exinterp);
			}
			printColorLegend(infile);
			infile.createLinesFromTokens();
			return true;
		}
	} else if (voiceFuncsQ) { // run cadnetial-voice-function analysis if requested
		// TODO: make sure this count stuff works with the -V setting too.
		// if (getBoolean("count")) {
		// 	printCountAnalysis(voiceFuncs);
		// 	return false;
		// }
		voiceFuncs.resize(grid.getVoiceCount());
		for (int i=0; i<(int)voiceFuncs.size(); i++) {
			voiceFuncs[i].resize(infile.getLineCount());
		}
		for (int vindex=0; vindex<grid.getVoiceCount(); vindex++) {
			findCadentialVoiceFunctions(results, grid, attacks[vindex], voiceFuncs, vindex);
		}

		string exinterp = getString("exinterp");
		vector<HTp> kernspines = infile.getKernSpineStartList();
		infile.appendDataSpine(voiceFuncs.back(), "", exinterp);
		for (int i = (int)voiceFuncs.size()-1; i>0; i--) {
			int track = kernspines[i]->getTrack();
			infile.insertDataSpineBefore(track, voiceFuncs[i-1], "", exinterp);
		}
		printColorLegend(infile);
		infile.createLinesFromTokens();
		return true;
	} else {
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
}



/////////////////////////////
//
// Tool_dissonant::suppressDissonances -- remove dissonances.
//

void Tool_dissonant::suppressDissonances(HumdrumFile& infile, NoteGrid& grid,
		vector<vector<NoteCell*> >& attacks, vector<vector<string> >& results) {

	for (int i=0; i<(int)attacks.size(); i++) {
		suppressDissonancesInVoice(infile, grid, i, attacks[i], results[i]);
	}
}



//////////////////////////////
//
// Tool_dissonant::suppressDissonancesInVoice --
//

void Tool_dissonant::suppressDissonancesInVoice(HumdrumFile& infile, 
		NoteGrid& grid, int vindex, vector<NoteCell*>& attacks,
		vector<string>& results) {

	for (int i=0; i<(int)attacks.size(); i++) {
		int lineindex = attacks[i]->getLineIndex();
		if (results[lineindex] == "") {
			continue;
		} else if (results[lineindex] == ".") {
			continue;
		} else if (results[lineindex] == m_labels[PASSING_UP]) {
			mergeWithPreviousNote(infile, attacks, i);
		} else if (results[lineindex] == m_labels[PASSING_DOWN]) {
			mergeWithPreviousNote(infile, attacks, i);
		} else if (results[lineindex] == m_labels[NEIGHBOR_UP]) {
			mergeWithPreviousNote(infile, attacks, i);
		} else if (results[lineindex] == m_labels[NEIGHBOR_DOWN]) {
			mergeWithPreviousNote(infile, attacks, i);
		}
	}
}



//////////////////////////////
//
// Tool_dissonant::mergeWithPreviousNote --  will not
//  handle chords correctly.
//

void Tool_dissonant::mergeWithPreviousNote(HumdrumFile& infile,
		vector<NoteCell*>& attacks, int index) {

	if (index < 1) {
		return;
	}

	HTp note1 = attacks[index-1]->getToken();
	HTp note2 = attacks[index]->getToken();

	int line1 = note1->getLineIndex();
	int line2 = note2->getLineIndex();

	// bool barlineQ = false;
	for (int i=line1+1; i<line2; i++) {
		if (infile[i].isBarline()) {
			// barlineQ = true;
			break;
		}
	}

	HumNum dur1 = note1->getDuration();
	HumNum dur2 = note2->getDuration();

	HumNum sumdur = dur1 + dur2;

	/*
	cerr << "Notes" << note1;
	cerr << "\tto\t" << note2;
	cerr << "\tline\t" << note1->getLineIndex();
	cerr << "\tnewdur=" << sumdur;
	cerr << endl;
	*/

	bool tied1 = note1->find("[") != string::npos ? true : false;
	bool tied2 = note2->find("[") != string::npos ? true : false;

	if (tied1 || tied2) {
		// don't deal with tied notes for now
		return;
	}


	// for now, replace the pitch of the second note with
	// that of the first note.  Later tied them together or
	// merge into a single note depending on the notational
	// context.

	changePitch(note2, note1);

}



//////////////////////////////
//
// Tool_dissonant::changePitch -- will not handle chords correctly.
//

void Tool_dissonant::changePitch(HTp note2, HTp note1) {
	int b40 = Convert::kernToBase40(note1);
	string pitch = Convert::base40ToKern(b40);
	HumRegex hre;
	string n2 = *note2;
	hre.replaceDestructive(n2, pitch, "[A-Ga-gr#-]+");
	note2->setText(n2);
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
		NoteGrid& grid, vector<vector<NoteCell*> >& attacks, bool debug) {
	attacks.resize(grid.getVoiceCount());

	for (int i=0; i<grid.getVoiceCount(); i++) {
		attacks[i].clear();
		doAnalysisForVoice(results, grid, attacks[i], i, debug);
	}

	for (int i=0; i<grid.getVoiceCount(); i++) {
		findFakeSuspensions(results, grid, attacks[i], i);
	}

	for (int i=0; i<grid.getVoiceCount(); i++) {
		findLs(results, grid, attacks[i], i);
	}

	for (int i=0; i<grid.getVoiceCount(); i++) {
		findYs(results, grid, attacks[i], i);
	}
	
	for (int i=0; i<grid.getVoiceCount(); i++) {
		findAppoggiaturas(results, grid, attacks[i], i);
	}
}



//////////////////////////////
//
// Tool_dissonant::doAnalysisForVoice -- do analysis for a single voice by
//     subtracting NoteCells to calculate the diatonic intervals.
//

void Tool_dissonant::doAnalysisForVoice(vector<vector<string> >& results,
		NoteGrid& grid, vector<NoteCell*>& attacks, int vindex, bool debug) {
	attacks.clear();
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

	HumNum durpp = -1; // duration of previous previous note
	HumNum durp;       // duration of previous melodic note
	HumNum dur;        // duration of current note
	HumNum durn;       // duration of next melodic note
	HumNum odur = -1; // duration of current note in other voice which may have started earlier
	HumNum odurn = -1; // duration of next note in other voice
	double intp;       // diatonic interval from previous melodic note
	double intpp = -99;// diatonic interval to previous melodic note
	double intn;       // diatonic interval to next melodic note
	double levp;       // metric level of the previous melodic note
	double lev;        // metric level of the current note
	double levn;       // metric level of the next melodic note
	int lineindex;     // line in original Humdrum file content that contains note
	int lineindexpp = -1;// line in original Humdrum file content that contains the previous previous note
	// int lineindexn; // next line in original Humdrum file content that contains note
	int attackindexn;  // slice in NoteGrid content that contains next note attack
	int sliceindex;    // current timepoint in NoteGrid.
	int oattackindexn = -1; // next note attack index of the other voice involved in the diss.
	vector<double> harmint(grid.getVoiceCount());  // harmonic intervals
	bool dissonant;    // true if  note is dissonant with other sounding notes.
	char marking = '\0';
	int ovoiceindex = -1;
	string unexp_label; // default dissonance label if none of the diss types apply
	int refMeterNum;    // the numerator of the reference voice's notated time signature
	HumNum refMeterDen; // the denominator of the reference voice's notated time signature
	int othMeterNum;    // the numerator of the other voice's notated time signature
	HumNum othMeterDen; // the denominator of the other voice's notated time signature
	bool ternAgent = false;  // true if the ref voice would be a valid agent of a ternary susp. But if true, the diss is not necessarily a susp.

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

		int nextj = 0;
		int j = 0;

RECONSIDER:

		int value = 0;
		for (j=nextj; j<(int)harmint.size(); j++) {
			if (j == vindex) {
				// don't compare to self
				continue;
			}
			if (Convert::isNaN(harmint[j])) {
				// rest, so ignore
				continue;
			}

			value = (int)harmint[j] % 7; // remove octaves from interval, can return negative ints

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
			} else if (
					((value == 3) && not ((((vpitch-lowestnote) % 7) == 2) ||
					                     (((vpitch-lowestnote) % 7) == 4))) ||
					((value == -3) && not ((((otherpitch-lowestnote) % 7) == 2) ||
					                      (((otherpitch-lowestnote) % 7) == 4)))
					) {
				// If the harmonic interval between two notes is a fourth and 
				// the lower pitch in the interval is not a a third or a fifth
				// above the lowest note.
				dissonant = true;
				diss4Q = true;
				marking = 'N';
				unexp_label = m_labels[UNLABELED_Z4];
				// ovoiceindex = lowestnotei;
				ovoiceindex = j;
				// oattackindexn = grid.cell(ovoiceindex, sliceindex)->getNextAttackIndex();
				oattackindexn = getNextPitchAttackIndex(grid, ovoiceindex, sliceindex);
				break;
			}
		}
		nextj = j+1;


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
		if (i >= 2) {
			intpp = *attacks[i-1] - *attacks[i-2];
			durpp = attacks[i-2]->getDuration();
			lineindexpp = attacks[i-2]->getLineIndex();
		}

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

		// Condition 2: The other (dissonant) voice stayed in place or repeated the
		//    same pitch at the onset of this dissonant interval.
		bool condition2 = true;
		bool condition2b = false;
		double opitch = grid.cell(ovoiceindex, sliceindex)->getSgnMidiPitch();
		double opitchDia = grid.cell(ovoiceindex, sliceindex)->getAbsDiatonicPitch();
		int lastonoteindex = grid.cell(ovoiceindex, sliceindex)->getPrevAttackIndex();
		double lopitch = NAN;
		if (lastonoteindex >= 0) {
			lopitch = grid.cell(ovoiceindex, lastonoteindex)->getAbsMidiPitch();
			double lopitchDia = grid.cell(ovoiceindex, lastonoteindex)->getAbsDiatonicPitch();
			if (abs(int(opitchDia - lopitchDia)) == 7) {
				condition2b = true;
			}
		} else {
			condition2 = false;
		}
		if (opitch < 0) {
			condition2 = true;
		} else if (opitch != lopitch) {
			condition2 = false;
		}

		int oattackindexp = grid.cell(ovoiceindex, sliceindex)->getPrevAttackIndex();
		int oattackindexc = grid.cell(ovoiceindex, sliceindex)->getCurrAttackIndex();
		odur = grid.cell(ovoiceindex, oattackindexc)->getDuration();
		int olineindexc = grid.cell(ovoiceindex, oattackindexc)->getLineIndex();
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
			odurn = grid.cell(ovoiceindex, oattackindexn)->getDuration();
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
		//    its pitch class.  [The voices can leave their pitch classes for 
		//    another note or for a rest.]
		bool condition3a = oattackindexn <= attackindexn ? true : false;

		// For ornamented suspensions.
		bool condition3b = oattackindexnn <= attackindexn ? true : false;

		// valid_sus_acc: determines if the reference voice conforms to the
		// standards of the accompaniment voice for suspensions.
		bool valid_sus_acc = condition2 && condition3a;
		bool valid_ornam_sus_acc = condition2 && condition3b;

		double ointp = opitch - opitchp;
		double ointn = opitchn - opitch;
		double ointnn = opitchnn - opitchn;

		// To distinguish between binary and ternary suspensions and agents
		int    getMeterTop          (void);
		HumNum getMeterBottom       (void);

		// Assign time signature ints here:
		refMeterNum = attacks[i]->getMeterTop();
		refMeterDen = attacks[i]->getMeterBottom();
		othMeterNum = grid.cell(ovoiceindex, sliceindex)->getMeterTop();
		othMeterDen = grid.cell(ovoiceindex, sliceindex)->getMeterBottom();
		HumNum threehalves(3, 2);
		HumNum sixteenthirds(16, 3);
		if (othMeterDen == 0) {
			othMeterDen = 8;
		} else if (othMeterDen == 1) {
			othMeterDen = 4;
		} else if (othMeterDen == 4) {
			othMeterDen = 1;
		}

		ternAgent = false;
		if (((othMeterNum % 3 == 0) && (odur >= othMeterDen)) && // the durational value of the meter's denominator groups in threes and the sus lasts at least as long as the denominator
				((dur == othMeterDen*2) || // the ref note lasts 2 times as long as the meter's denominator
				 ((dur == othMeterDen*threehalves) && ((intn == 0) || (intn == -1))) || // ref note lasts 1.5 times the meter's denominator and next note is a tenorizans ornament
				 ((dur == othMeterDen*threehalves) && ((unexp_label == m_labels[UNLABELED_Z4]) || (intn == 3))) || // 4-3 susp where agent leaps to diatonic pitch class of resolution
				 ((dur == sixteenthirds) && (refMeterNum == 3) && (refMeterDen == threehalves)) || // special case for 3/3 time signature
				 ((odur == othMeterDen*threehalves) && (ointn == -1) && (odurn == 2) && (ointnn == 0)) || // change of agent suspension with ant of resolution
				 ((dur == othMeterDen) && (odur == othMeterDen*2))) && // unornamented change of agent suspension
				(results[ovoiceindex][lineindex] != m_labels[SUS_BIN])) { // the other voice hasn't already been labeled as a binary suspension
			ternAgent = true;
		}


		if (((lev >= levn) || ((lev == 2) && (dur == .5))) && (lev >= levp) && 
			(dur <= durp) && (condition2 || condition2b) && valid_acc_exit) { // weak dissonances
			if (intp == -1) { // descending dissonances
				if (intn == -1) { // downward passing tone
					results[vindex][lineindex] = m_labels[PASSING_DOWN];
				} else if (intn == 1) { // lower neighbor
					results[vindex][lineindex] = m_labels[NEIGHBOR_DOWN];
				} else if ((intn == 0) && (dur <= 2)) { // descending anticipation
					results[vindex][lineindex] = m_labels[ANT_DOWN];
				} else if (intn > 1) { // lower échappée
					results[vindex][lineindex] = m_labels[ECHAPPEE_DOWN];
				} else if (intn < -1) { // descending short nota cambiata
					results[vindex][lineindex] = m_labels[CAMBIATA_DOWN_S];
				}
			} else if (intp == 1) { // ascending dissonances
				if (intn == 1) { // rising passing tone
					results[vindex][lineindex] = m_labels[PASSING_UP];
				} else if (intn == -1) { // upper neighbor
					results[vindex][lineindex] = m_labels[NEIGHBOR_UP];
				} else if (intn < -1) { // upper échappée
					results[vindex][lineindex] = m_labels[ECHAPPEE_UP];
				} else if ((intn == 0) && (dur <= 2)) { // rising anticipation
					results[vindex][lineindex] = m_labels[ANT_UP];
				} else if (intn > 1) { // ascending short nota cambiata
					results[vindex][lineindex] = m_labels[CAMBIATA_UP_S];
				}
			} else if (intp < -1) {
				if (intn == 1) { // reverse lower échappée
					results[vindex][lineindex] = m_labels[REV_ECHAPPEE_DOWN];
				} else if (intn == -1) { // reverse descending nota cambiata
					results[vindex][lineindex] = m_labels[REV_CAMBIATA_DOWN];
				}
			} else if (intp > 1) {
				if (intn == -1) { // reverse upper échappée
					results[vindex][lineindex] = m_labels[REV_ECHAPPEE_UP];
				} else if (intn == 1) { // reverse ascending nota cambiata
					results[vindex][lineindex] = m_labels[REV_CAMBIATA_UP];
				}
			}
		} else if ((durp >= 2) && (dur == 1) && (lev < levn) && valid_acc_exit &&
					 (condition2 || condition2b) && (lev == 1)) {
			if (intp == -1) {
				if (intn == -1) { // dissonant third quarter descending passing tone
					results[vindex][lineindex] = m_labels[THIRD_Q_PASS_DOWN];
				} else if (intn == 1) { // dissonant third quarter lower neighbor
					results[vindex][lineindex] = m_labels[THIRD_Q_LOWER_NEI];
				}
			} else if (intp == 1) {
				if (intn == 1) { // dissonant third quarter ascending passing tone
					results[vindex][lineindex] = m_labels[THIRD_Q_PASS_UP];
				} else if (intn == -1) { // dissonant third quarter upper neighbor
					results[vindex][lineindex] = m_labels[THIRD_Q_UPPER_NEI];
				}
			}
		} else if (((lev > levp) || (durp+durp+durp+durp == dur)) && 
				   (lev == levn) && condition2 && (intn == -1) && 
				   (dur == (durn+durn)) && ((dur+dur) <= odur)) {
			if (fabs(intp) > 1.0) {
				results[vindex][lineindex] = m_labels[SUS_NO_AGENT_LEAP];
			} else if ((fabs(intp) == 1.0) || ((intp == 0) && (fabs(intpp) == 1.0))) {
				results[vindex][lineindex] = m_labels[SUS_NO_AGENT_STEP];
			}
		}

		/////////////////////////////
		////
		//// Code to apply binary or ternary suspension and agent labels and 
		//// also suspension ornament and chanson idiom labels

		else if (valid_sus_acc && ((ointn == -1) || ((ointn == -2) && (ointnn == 1)))) {
			if ((durpp == 1) && (durp == 1) && (intpp == -1) && (intp == 1) &&
				((results[vindex][lineindexpp] == m_labels[THIRD_Q_PASS_DOWN]) ||
				 (results[vindex][lineindexpp] == m_labels[ACC_PASSING_DOWN]) ||
				 (results[vindex][lineindexpp] == m_labels[UNLABELED_Z7]) ||
				 (results[vindex][lineindexpp] == m_labels[UNLABELED_Z4]))) {
				results[vindex][lineindexpp] = m_labels[CHANSON_IDIOM];
			}
			if (ternAgent) { // ternary agent and suspension
				results[vindex][lineindex] = m_labels[AGENT_TERN];
				results[ovoiceindex][lineindex] = m_labels[SUS_TERN];
			} else { // binary agent and suspension
				results[vindex][lineindex] = m_labels[AGENT_BIN];
				results[ovoiceindex][lineindex] = m_labels[SUS_BIN];
			}
		} else if (valid_ornam_sus_acc && ((ointn == 0) && (ointnn == -1))) {
			if ((durpp == 1) && (durp == 1) && (intpp == -1) && (intp == 1) &&
				((results[vindex][lineindexpp] == m_labels[THIRD_Q_PASS_DOWN]) ||
				 (results[vindex][lineindexpp] == m_labels[ACC_PASSING_DOWN]) ||
				 (results[vindex][lineindexpp] == m_labels[UNLABELED_Z7]) ||
				 (results[vindex][lineindexpp] == m_labels[UNLABELED_Z4]))) {
				results[vindex][lineindexpp] = m_labels[CHANSON_IDIOM];
			}
			if (ternAgent) { // ternary agent and suspension
				results[vindex][lineindex] = m_labels[AGENT_TERN];
				results[ovoiceindex][lineindex] = m_labels[SUS_TERN];
			} else { // binary agent and suspension
				results[vindex][lineindex] = m_labels[AGENT_BIN];
				results[ovoiceindex][lineindex] = m_labels[SUS_BIN];
			} // repeated-note of suspension
			results[ovoiceindex][olineindexn] = m_labels[SUSPENSION_REP];
		} else if (valid_ornam_sus_acc && ((ointn == 1) && (ointnn == -2))) {
			if ((durpp == 1) && (durp == 1) && (intpp == -1) && (intp == 1) &&
				((results[vindex][lineindexpp] == m_labels[THIRD_Q_PASS_DOWN]) ||
				 (results[vindex][lineindexpp] == m_labels[ACC_PASSING_DOWN]) ||
				 (results[vindex][lineindexpp] == m_labels[UNLABELED_Z7]) ||
				 (results[vindex][lineindexpp] == m_labels[UNLABELED_Z4]))) {
				results[vindex][lineindexpp] = m_labels[CHANSON_IDIOM];
			}
			if (ternAgent) { // ternary agent and suspension
				results[vindex][lineindex] = m_labels[AGENT_TERN];
				results[ovoiceindex][lineindex] = m_labels[SUS_TERN];
			} else { // binary agent and suspension
				results[vindex][lineindex] = m_labels[AGENT_BIN];
				results[ovoiceindex][lineindex] = m_labels[SUS_BIN];
			} // This ornament is consonant against the agent so no ornament label.
		}

/////////////////////////////

		if (i < ((int)attacks.size() - 2)) { // expand the analysis window

			double intnn = *attacks[i+2] - *attacks[i+1];
			HumNum durnn = attacks[i+2]->getDuration();	// dur of note after next
			// double levnn = attacks[i+2]->getMetricLevel(); // lev of note after next

			if ((dur <= durp) && (lev >= levp) && (lev >= levn) &&
					(intp == -1) && (intn == -2) && (intnn == 1)) { // long-form descending cambiata
				results[vindex][lineindex] = m_labels[CAMBIATA_DOWN_L];
			} else if ((dur <= durp) && (lev >= levp) && (lev >= levn) &&
					(intp == 1) && (intn == 2) && (intnn == -1)) { // long-form ascending nota cambiata
				results[vindex][lineindex] = m_labels[CAMBIATA_UP_L];
			}
		}

		// Decide whether to give an unexplained dissonance label to the ref.
		// voice if none of the dissonant conditions above apply.
		bool refLeaptTo = fabs(intp) > 1 ? true : false;
		bool othLeaptTo = fabs(ointp) > 1 ? true : false;
		bool refLeaptFrom = fabs(intn) > 1 ? true : false;
		bool othLeaptFrom = fabs(ointn) > 1 ? true : false;

		if ((results[vindex][lineindex] == "") && // this voice doesn't already have a dissonance label
			((olineindexc < lineindex) || // other voice does not attack at this point
				((olineindexc == lineindex) && (dur < odur)) || // both voices attack together, but ref voice leaves dissonance first
				(((olineindexc == lineindex) && (dur == odur)) && // both voices enter and leave dissonance simultaneously
				 ((!refLeaptFrom && othLeaptFrom) || // ref voice leaves diss by step or rep and other voice leaves by leap
				  (refLeaptTo && refLeaptFrom && othLeaptTo && othLeaptFrom) || // both voices enter and leave diss by leap
				  ((fabs(intp) == 1) && (intn == 0) && ((fabs(ointp)) > 0 || (fabs(ointn) > 0))) || // ref voice enters by step, leaves by rep, other v repeats no more than once
				  ((fabs(intp) == 1) && (fabs(intn) == 1) && !othLeaptTo && !othLeaptFrom) || // ref voice enters and leaves by step, other voice by step or rep
				  ((fabs(intp) == 1) && (intn == 0) && !othLeaptTo && (ointn == 0)) || // ref enters by step and leaves by rep, other v enters by step or rep and leaves by rep
				  (!refLeaptTo && refLeaptFrom && othLeaptFrom))))) { // ref voice enters diss by step or rep and both voices leave by leap
			results[vindex][lineindex] = unexp_label;
		}


		// If the note was labeled as an unknown dissonance, then go back and check
		// against another note with which it might have a known dissonant function.
		if ((results[vindex][lineindex] == m_labels[UNLABELED_Z4]) || 
				(results[vindex][lineindex] == m_labels[UNLABELED_Z7]) ||
				(results[vindex][lineindex] == m_labels[UNLABELED_Z7])) {
			if (nextj < (int)harmint.size()) {
				goto RECONSIDER;
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
	double intp;        // abs value of diatonic interval from previous melodic note
	int lineindexn;     // line index of the next note in the voice
	bool sfound;        // boolean for if a suspension is found after a Z dissonance

	for (int i=1; i<(int)attacks.size()-1; i++) {
		int lineindex = attacks[i]->getLineIndex();
		if ((results[vindex][lineindex].find("Z") == string::npos) &&
			(results[vindex][lineindex].find("z") == string::npos) &&
			(results[vindex][lineindex].find("M") == string::npos) &&
			(results[vindex][lineindex].find("m") == string::npos)) {
			continue;
		}
		intp = fabs(*attacks[i] - *attacks[i-1]);
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

		if (intp == 1) { // Apply labels for normal fake suspensions.
			results[vindex][lineindex] = m_labels[FAKE_SUSPENSION_STEP];
		} else if (intp > 1) {
			results[vindex][lineindex] = m_labels[FAKE_SUSPENSION_LEAP];
		} else if (i > 1) { // as long as i > 1 intpp will be in range.
			double intpp = fabs(*attacks[i-1] - *attacks[i-2]);
			if (intp == 0) { // fake suspensions preceded by an anticipation.
				if (intpp == 1) {
					results[vindex][lineindex] = m_labels[FAKE_SUSPENSION_STEP];
				} else if (intpp > 1) {
					results[vindex][lineindex] = m_labels[FAKE_SUSPENSION_LEAP];
				}
			}
		}
	}
}

//////////////////////////////
//
// Tool_dissonant::findLs --
//
void Tool_dissonant::findLs(vector<vector<string> >& results, NoteGrid& grid,
		vector<NoteCell*>& attacks, int vindex) {
	HumNum dur;        // duration of current note;
	HumNum odur;       // duration of current note in other voice which may have started earlier;
	double intp;       // diatonic interval from previous melodic note
	double intn;       // diatonic interval to next melodic note
	double ointp;      // diatonic interval from previous melodic note in other voice
	double ointn;      // diatonic interval to next melodic note in other voice
	int lineindex;     // line in original Humdrum file content that contains note
	int olineindex;   // line in original Humdrum file content that contains other voice note
	int sliceindex;    // current timepoint in NoteGrid.
	int oattackindexp; // line index of other voice's previous note
	int oattackindexc; // line index of other voice's current note
	int oattackindexn; // line index of other voice's next note
	double opitchp;    // previous pitch in other voice
	double opitch;     // current pitch in other voice
	double opitchn;    // next pitch in other voice

	for (int i=1; i<(int)attacks.size()-1; i++) {
		lineindex = attacks[i]->getLineIndex();
		if ((results[vindex][lineindex].find("Z") == string::npos) &&
			(results[vindex][lineindex].find("z") == string::npos)) {
			continue;
		}
		dur  = attacks[i]->getDuration();
		intp = *attacks[i] - *attacks[i-1];
		intn = *attacks[i+1] - *attacks[i];
		sliceindex = attacks[i]->getSliceIndex();

		for (int j=0; j<(int)grid.getVoiceCount(); j++) { // j is the voice index of the other voice
			if (vindex == j) { // only compare different voices
				continue;
			}
			if ((results[j][lineindex] == m_labels[AGENT_BIN]) ||
				(results[j][lineindex] == m_labels[AGENT_TERN]) ||
				(results[j][lineindex] == m_labels[UNLABELED_Z7]) ||
				(results[j][lineindex] == m_labels[UNLABELED_Z4]) ||
				(results[j][lineindex] == "")) {
				continue; // skip if other voice is an agent, unexplainable, or empty.
			}
			oattackindexc = grid.cell(j, sliceindex)->getCurrAttackIndex();
			olineindex = grid.cell(j, oattackindexc)->getLineIndex();
			if (olineindex != lineindex) { // if olineindex == lineindex then oattackindexp is in range
				continue; // skip if other voice doesn't attack at the same time
			}
			oattackindexp = grid.cell(j, sliceindex)->getPrevAttackIndex();
			odur = grid.cell(j, oattackindexc)->getDuration();
			if (dur != odur) { // if dur == odur then the oattackindexn will be in range
				continue;
			}
			opitchp = grid.cell(j, oattackindexp)->getAbsDiatonicPitch();
			opitch = grid.cell(j, sliceindex)->getAbsDiatonicPitch();
			oattackindexn = grid.cell(j, sliceindex)->getNextAttackIndex();
			opitchn = grid.cell(j, oattackindexn)->getAbsDiatonicPitch();
			ointp = opitch - opitchp;
			ointn = opitchn - opitch;
			if ((intp == ointp) && (intn == ointn)) { // this note moves in parallel with an identifiable dissonance
				if (intp > 0) {
					results[vindex][lineindex] = m_labels[PARALLEL_UP];
					break;
				} else if (intp < 0) {
					results[vindex][lineindex] = m_labels[PARALLEL_DOWN];
					break;			
				}
			}
		}
	}
}

//////////////////////////////
//
// Tool_dissonant::findYs --
//
void Tool_dissonant::findYs(vector<vector<string> >& results, NoteGrid& grid,
		vector<NoteCell*>& attacks, int vindex) { 
	double intp;       // diatonic interval from previous melodic note
	double intn;       // diatonic interval to next melodic note
	int lineindex;     // line in original Humdrum file content that contains note
	int olineindex;    // line in original Humdrum file content that contains other voice note
	int sliceindex;    // current timepoint in NoteGrid
	int attackindexn;  // line index of next note
	int oattackindexc; // line index of other voice current note
	int oattackindexn; // line index of other voice's next note
	double pitch;      // current pitch in this voice
	double opitch;     // current pitch in other voice
	bool onlyWithValids; // note is only dissonant with identifiable dissonances
	bool valid_acc_exit; // if accompaniment voice conforms to necessary standards

	for (int i=1; i<(int)attacks.size()-1; i++) {
		lineindex = attacks[i]->getLineIndex();
		if ((results[vindex][lineindex].find("Z") == string::npos) &&
			(results[vindex][lineindex].find("z") == string::npos)) {
			continue;
		}
		intp = *attacks[i] - *attacks[i-1];
		intn = *attacks[i+1] - *attacks[i];
		sliceindex = attacks[i]->getSliceIndex();

		int lowestnote = 1000; // lowest sounding diatonic note in any voice at this sliceindex
		double tpitch;
		for (int v=0; v<(int)grid.getVoiceCount(); v++) {
			tpitch = grid.cell(v, sliceindex)->getAbsDiatonicPitch();
			if (!Convert::isNaN(tpitch)) {
				if (tpitch <= lowestnote) {
					lowestnote = tpitch;
				}
			}
		}

		onlyWithValids = true; 
		for (int j=0; j<(int)grid.getVoiceCount(); j++) { // j = index of other voice
			if ((vindex == j) || (onlyWithValids == false)) {
				continue;
			}
			oattackindexc = grid.cell(j, sliceindex)->getCurrAttackIndex();
			oattackindexn = grid.cell(j, sliceindex)->getNextAttackIndex();
			attackindexn = attacks[i]->getNextAttackIndex();
			pitch = attacks[i]->getAbsDiatonicPitch();
			opitch = grid.cell(j, sliceindex)->getAbsDiatonicPitch();
			olineindex = grid.cell(j, oattackindexc)->getLineIndex();
			int thisInt = opitch - pitch; // diatonic interval in this pair
			int thisMod7 = thisInt % 7; // simplify octaves out of thisInt
			valid_acc_exit = oattackindexn < attackindexn ? false : true;
			if (oattackindexn < 0) {
				valid_acc_exit = true;
			}

			if (((thisMod7 == 1) || (thisMod7 == -6)) && // creates 2nd or 7th diss
				((results[j][lineindex] == m_labels[SUS_BIN]) || // other voice is susp
				 (results[j][lineindex] == m_labels[SUS_TERN])) &&
				(fabs(intp) == 1) && (intn == -1) && valid_acc_exit) {
				results[vindex][lineindex] = m_labels[RES_PITCH];
				onlyWithValids = false;
			} else if (((abs(thisMod7) == 1) || (abs(thisMod7) == 6)  ||
				       ((thisInt > 0) && (thisMod7 == 3) && 
				        not (((int(pitch-lowestnote) % 7) == 2) ||
                 	         ((int(pitch-lowestnote) % 7) == 4))) ||
				       ((thisInt < 0) && (thisMod7 == -3) && // a fourth by inversion is -3 and -3%7 = -3.
				        not (((int(opitch-lowestnote) % 7) == 2) ||
                 	         ((int(opitch-lowestnote) % 7) == 4)))) &&
				      ((results[j][olineindex] == m_labels[AGENT_BIN]) ||
				       (results[j][olineindex] == m_labels[AGENT_TERN]) ||
				       (results[j][olineindex] == m_labels[UNLABELED_Z7]) ||
				       (results[j][olineindex] == m_labels[UNLABELED_Z4]) ||
				       ((results[j][olineindex] == "") &&
				        ((results[j][lineindex] != m_labels[SUS_BIN]) &&
				         (results[j][lineindex] != m_labels[SUS_TERN]))))) {
				onlyWithValids = false;
			}
		}

		if (onlyWithValids && ((results[vindex][lineindex] == m_labels[UNLABELED_Z7]) ||
							   (results[vindex][lineindex] == m_labels[UNLABELED_Z4]))) {
			if (intp > 0) {
				results[vindex][lineindex] = m_labels[ONLY_WITH_VALID_UP];
			} else if (intp <= 0) {
				results[vindex][lineindex] = m_labels[ONLY_WITH_VALID_DOWN];
			}
		}
	}
}

//////////////////////////////
//
// Tool_dissonant::findAppoggiaturas --
//
void Tool_dissonant::findAppoggiaturas(vector<vector<string> >& results, NoteGrid& grid,
		vector<NoteCell*>& attacks, int vindex) {
	HumNum durpp;      // duration of previous previous note
	HumNum durp;       // duration of previous note
	HumNum dur;        // duration of current note
	HumNum durn;	   // duration of next note
	double intp;       // diatonic interval from previous melodic note
	double intn;       // diatonic interval to next melodic note
	double lev;        // metric level of the current note
	double levn;       // metric level of the next melodic note
	int lineindexp;    // line in original Humdrum file content that contains previous note
	int lineindex;     // line in original Humdrum file content that contains note
	int sliceindex;    // current timepoint in NoteGrid.
	int attackindexn;  // line index of ref voice's next note
	int oattackindexn; // line index of other voice's next note
	double pitch;      // current pitch in ref voice
	double opitch;     // current pitch in other voice
	bool ant_down;	   // if the current note was preceded by a descending anticipation
	bool ant_up;	   // if the current note was preceded by an ascending anticipation
	bool ant_leapt_to; // if the current note was preceded by an anticipation leapt to

	for (int i=1; i<(int)attacks.size()-1; i++) {
		lineindexp = attacks[i-1]->getLineIndex();
		lineindex = attacks[i]->getLineIndex();
		if ((results[vindex][lineindex].find("Z") == string::npos) &&
			(results[vindex][lineindex].find("z") == string::npos) &&
			(results[vindex][lineindex].find("J") == string::npos) &&
			(results[vindex][lineindex].find("j") == string::npos)) {
			continue;
		}
		durp = attacks[i-1]->getDuration();
		dur  = attacks[i]->getDuration();
		durn = attacks[i+1]->getDuration();
		intp = *attacks[i] - *attacks[i-1];
		intn = *attacks[i+1] - *attacks[i];
		lev  = attacks[i]->getMetricLevel();
		levn = attacks[i+1]->getMetricLevel();
		sliceindex = attacks[i]->getSliceIndex();
		
		if (!((lev <= levn) && (dur <= durn))) {
			continue; // go on when the voice with Z label doesn't fulfill its metric or durational requirements
		}

		// determine if current note was preceded by an anticipation (which may be a consonant anticipation)
		ant_down     = false;
		ant_up       = false;
		ant_leapt_to = false;
		if (i > 1) {
			durpp = attacks[i-2]->getDuration();
			if ((intp == 0) && (durp <= dur) && (durp <= durpp)) {
				if ((*attacks[i-1] - *attacks[i-2]) == -1) {
					ant_down = true;
				} else if ((*attacks[i-1] - *attacks[i-2]) == 1) {
					ant_up = true;
				} else if (fabs(*attacks[i-1] - *attacks[i-2]) > 1) {
					ant_leapt_to = true;
				}
			}
		}

		int lowestnote = 1000; // lowest sounding diatonic note in any voice at this sliceindex
		double tpitch;
		for (int v=0; v<(int)grid.getVoiceCount(); v++) {
			tpitch = grid.cell(v, sliceindex)->getAbsDiatonicPitch();
			if ((!Convert::isNaN(tpitch)) && (tpitch <= lowestnote)) {
				lowestnote = tpitch;
			}
		}

		for (int j=0; j<(int)grid.getVoiceCount(); j++) { // j is the voice index of the other voice
			if (vindex == j) { // only compare different voices
				continue;
			}

			attackindexn = attacks[i]->getNextAttackIndex();
			oattackindexn = grid.cell(j, sliceindex)->getNextAttackIndex();
			if (oattackindexn < attackindexn) {
				continue; // skip this pair if other voice leaves diss first
			}

			pitch = attacks[i]->getAbsDiatonicPitch();
			opitch = grid.cell(j, sliceindex)->getAbsDiatonicPitch();
			int thisInt = opitch - pitch; // diatonic interval in this pair
			int thisMod7 = thisInt % 7; // simplify octaves out of thisInt

			// see if the pair creates a dissonant interval
			if (!((abs(thisMod7) == 1) || (abs(thisMod7) == 6)  ||
				 ((thisInt > 0) && (thisMod7 == 3) && 
				  not (((int(pitch-lowestnote) % 7) == 2) ||
                 	   ((int(pitch-lowestnote) % 7) == 4))) ||
				 ((thisInt < 0) && (thisMod7 == -3) && // a fourth by inversion is -3 and -3%7 == -3.
				  not (((int(opitch-lowestnote) % 7) == 2) ||
                 	   ((int(opitch-lowestnote) % 7) == 4))))) {
				continue;
			} else if (((intp == -1) || ant_down) && ((lev <= levn) && (dur <= durn)) &&
				       ((results[vindex][lineindex] == m_labels[UNLABELED_Z7]) ||
				        (results[vindex][lineindex] == m_labels[UNLABELED_Z4]))) {
				if (intn == -1) {
					results[vindex][lineindex] = m_labels[ACC_PASSING_DOWN]; // descending accented passing tone
				} else if (intn == 1) {
					results[vindex][lineindex] = m_labels[ACC_LO_NEI]; // accented lower neighbor
				}
			} else if (((intp == 1) || ant_up) && ((lev <= levn) && (dur <= durn)) &&
				       ((results[vindex][lineindex] == m_labels[UNLABELED_Z7]) ||
				        (results[vindex][lineindex] == m_labels[UNLABELED_Z4]))) {
				if (intn == 1) {
					results[vindex][lineindex] = m_labels[ACC_PASSING_UP]; // rising accented passing tone
				} else if (intn == -1) {
					results[vindex][lineindex] = m_labels[ACC_UP_NEI]; // accented upper neighbor
				}
			} else if (intn == -1) {
				if ((intp == 2) && (results[vindex][lineindexp] == m_labels[ECHAPPEE_DOWN]) &&
					(((results[vindex][lineindex] == m_labels[UNLABELED_Z7]) ||
				      (results[vindex][lineindex] == m_labels[UNLABELED_Z4]) ||
				      (results[vindex][lineindex] == m_labels[REV_ECHAPPEE_UP])) ||
					 ((lev <= levn) && (dur <= durn)))) {
					results[vindex][lineindexp] = m_labels[DBL_NEIGHBOR_DOWN];
					results[vindex][lineindex]  = m_labels[DBL_NEIGHBOR_DOWN];									
				} else if (((fabs(intp) > 1) || ant_leapt_to) && 
						   ((lev <= levn) && (dur <= durn)) &&
						   ((results[vindex][lineindex] == m_labels[UNLABELED_Z7]) ||
						    (results[vindex][lineindex] == m_labels[UNLABELED_Z4]))) { // upper appoggiatura
					results[vindex][lineindex] = m_labels[APP_UPPER];
				}
			} else if (intn == 1) {
				if ((intp == -2) && (results[vindex][lineindexp] == m_labels[ECHAPPEE_UP]) &&
					(((results[vindex][lineindex] == m_labels[UNLABELED_Z7]) ||
				      (results[vindex][lineindex] == m_labels[UNLABELED_Z4]) ||
				      (results[vindex][lineindex] == m_labels[REV_ECHAPPEE_DOWN])) ||
					 ((lev <= levn) && (dur <= durn)))) {
					results[vindex][lineindexp] = m_labels[DBL_NEIGHBOR_UP];
					results[vindex][lineindex]  = m_labels[DBL_NEIGHBOR_UP];					
				} else if (((fabs(intp) > 1) || ant_leapt_to) && 
						   ((lev <= levn) && (dur <= durn)) &&
						   ((results[vindex][lineindex] == m_labels[UNLABELED_Z7]) ||
						    (results[vindex][lineindex] == m_labels[UNLABELED_Z4]))) { // lower appoggiatura
					results[vindex][lineindex] = m_labels[APP_LOWER];
				}
			}
		}
	}
}




//////////////////////////////
//
// Tool_dissonant::findCadentialVoiceFunctions -- identify the cadential-voice 
//		functions present in each voice. These are the single-line constituents
//		of Renaissance cadences. Five basic types are identified: Cantizans,
//		Altizans, Tenorizans, Leaping Contratenor, and Bassizans. Since the
//		cadential-voice functions are identified contrapuntally, a Cantizans or
//		Altizans must be found set against any of the other three types for 
//		anything to be detected. 
//
void Tool_dissonant::findCadentialVoiceFunctions(vector<vector<string> >& results, NoteGrid& grid,
		vector<NoteCell*>& attacks, vector<vector<string> >& voiceFuncs, int vindex) {
	HumNum dur;        // duration of current note
	HumNum durn;	   // duration of next note
	double intn;       // diatonic interval to next melodic note
	double intnn;	   // diatonic interval from next melodic note to following note
	double ointn;	   // diatonic interval to next melodic note in other voice
	int lineindex;     // line in original Humdrum file content that contains note
	// NB lineindex2 is not needed
	int lineindex3;    // line in original Humdrum file content that contains note two events later
	// int lineindex4;    // line in original Humdrum file content that contains note three events later
	int sliceindex;    // current timepoint in NoteGrid.
	int attInd2;  	   // line index of ref voice's next attack
	int attInd3;       // line index of ref voice's attack two events later
	int oattInd2;      // line index of other voice's next attack
	double pitch;      // current pitch in ref voice
	double opitch;     // current pitch in other voice
	double opitchn;	   // pitch of next note in other voice

	for (int i=1; i<(int)attacks.size()-2; i++) {
		// lineindexp = attacks[i-1]->getLineIndex();
		lineindex  = attacks[i]->getLineIndex();
		lineindex3 = attacks[i+2]->getLineIndex();
		// pass over if ref voice is not a patient
		if ((results[vindex][lineindex].find("S") == string::npos) &&
			(results[vindex][lineindex].find("s") == string::npos)) {
			continue;
		}
		dur  = attacks[i]->getDuration();
		durn = attacks[i+1]->getDuration();
		intn = *attacks[i+1] - *attacks[i];
		intnn = *attacks[i+2] - *attacks[i+1];
		sliceindex = attacks[i]->getSliceIndex();

		for (int j=0; j<(int)grid.getVoiceCount(); j++) { // j is the voice index of the other voice
			if (vindex == j) { // only compare different voices
				continue;
			}

			// skip if other voice isn't an agent
			if ((results[j][lineindex].find("G") == string::npos) &&
				(results[j][lineindex].find("g") == string::npos) ) {
				continue;
			}

			oattInd2 = -22;
			pitch    = attacks[i]->getAbsDiatonicPitch();
			opitch   = grid.cell(j, sliceindex)->getAbsDiatonicPitch();
			attInd2  = attacks[i]->getNextAttackIndex();
			attInd3  = attacks[i+1]->getNextAttackIndex();
			oattInd2 = grid.cell(j, sliceindex)->getNextAttackIndex();
			if (oattInd2 > 0) {
				opitchn = grid.cell(j, oattInd2)->getAbsDiatonicPitch();
				ointn = opitchn - opitch;
			}
			// NB since the ref voice is the one with the suspension, if the 
			// suspension is in the higher voice (the most common case) then the
			// harmonic intervals will actually be negative
			int thisInt = opitch - pitch; // diatonic interval in this pair
			int thisMod7 = thisInt % 7; // simplify octaves out of thisInt

			if ((thisMod7 == -6) && (intn == -2) && (intnn == 2) && 
				(oattInd2 > 0) && (ointn == -2) && (attInd3 == oattInd2)) {
				voiceFuncs[vindex][attInd3] = "C"; // cantizans
				voiceFuncs[j][attInd3] = "T"; // tenorizans
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

	m_humdrum_text << "**rdis";
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
		if (i == UNLABELED_Z2) {
			continue;
		}
		if (i == UNLABELED_Z7) {
			continue;
		}

		item = m_labels[i];

		if (brief && (reduced.find(item) != reduced.end())) {
			continue;
		}
		reduced[item] = 1;

		sum = 0;
		for (j=0; j<(int)analysis.size(); j++) {
			if (analysis[j].find(item) != analysis[j].end()) {
				sum += analysis[j][item];
				// Don't include agents in dissonant note summation.
				if ((item != m_labels[AGENT_TERN]) && (item != m_labels[AGENT_BIN])) {
					sumsum += analysis[j][item];
				}
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
					if ((item == m_labels[AGENT_BIN]) || (item == m_labels[AGENT_TERN])) {
						m_humdrum_text << ".";
					} else {
						m_humdrum_text << int(analysis[j][item] * 1.0 / sum * 1000.0 + 0.5) / 10.0;
					}
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
	m_labels[ECHAPPEE_UP         ] = "E"; // upper échappée
	m_labels[ECHAPPEE_DOWN       ] = "e"; // lower échappée
	m_labels[CAMBIATA_UP_S       ] = "C"; // ascending short nota cambiata
	m_labels[CAMBIATA_DOWN_S     ] = "c"; // descending short nota cambiata
	m_labels[CAMBIATA_UP_L       ] = "K"; // ascending long nota cambiata
	m_labels[CAMBIATA_DOWN_L     ] = "k"; // descending long nota cambiata
	m_labels[REV_CAMBIATA_UP     ] = "I"; // incomplete anterior upper neighbor
	m_labels[REV_CAMBIATA_DOWN   ] = "i"; // incomplete anterior lower neighbor
	m_labels[REV_ECHAPPEE_UP     ] = "J"; // incomplete posterior upper neighbor
	m_labels[REV_ECHAPPEE_DOWN   ] = "j"; // incomplete posterior lower neighbor
	m_labels[ANT_UP              ] = "A"; // rising anticipation
	m_labels[ANT_DOWN            ] = "a"; // descending anticipation
	m_labels[DBL_NEIGHBOR_UP     ] = "D"; // double neighbor beginning with upper neighbor
	m_labels[DBL_NEIGHBOR_DOWN   ] = "d"; // double neighbor beginning with lower neighbor
	m_labels[THIRD_Q_PASS_UP     ] = "Q"; // dissonant third quarter ascending passing tone
	m_labels[THIRD_Q_PASS_DOWN   ] = "q"; // dissonant third quarter descending passing tone
	m_labels[THIRD_Q_UPPER_NEI   ] = "B"; // dissonant third quarter upper neighbor
	m_labels[THIRD_Q_LOWER_NEI   ] = "b"; // dissonant third quarter lower neighbor
	m_labels[ACC_PASSING_UP		 ] = "V"; // ascending accented passing tone
	m_labels[ACC_PASSING_DOWN	 ] = "v"; // descending accented passing tone
	m_labels[ACC_UP_NEI	 		 ] = "W"; // accented upper neighbor
	m_labels[ACC_LO_NEI			 ] = "w"; // accented lower neighbor
	m_labels[APP_UPPER			 ] = "T"; // appoggiatura resolving down by step
	m_labels[APP_LOWER			 ] = "t"; // appoggiatura resolving up by step
	m_labels[SUS_BIN             ] = "s"; // binary suspension
	m_labels[SUS_TERN            ] = "S"; // ternary suspension
	m_labels[AGENT_BIN           ] = "g"; // binary agent
	m_labels[AGENT_TERN          ] = "G"; // ternary agent
	m_labels[SUSPENSION_REP      ] = "r"; // suspension repeated note
	m_labels[FAKE_SUSPENSION_LEAP] = "F"; // fake suspension approached by leap
	m_labels[FAKE_SUSPENSION_STEP] = "f"; // fake suspension approached by step or by anticipation
	m_labels[SUS_NO_AGENT_LEAP   ] = "M"; // suspension missing a normal agent approached by leap
	m_labels[SUS_NO_AGENT_STEP   ] = "m"; // suspension missing a normal agent approached by step or by anticipation
	m_labels[CHANSON_IDIOM       ] = "h"; // chanson idiom
	m_labels[PARALLEL_UP         ] = "L"; // moves up in parallel with identifiable dissonance
	m_labels[PARALLEL_DOWN       ] = "l"; // moves down in parallel with identifiable dissonance
	m_labels[RES_PITCH			 ] = "x"; // note of resolution of a suspension against suspension dissonance
	m_labels[ONLY_WITH_VALID_UP  ] = "Y"; // only dissonant against identifiable dissonances, approached from below
	m_labels[ONLY_WITH_VALID_DOWN] = "y"; // only dissonant against identifiable dissonances, approached from above
	m_labels[UNKNOWN_DISSONANCE  ] = "Z"; // unknown dissonance
	m_labels[UNLABELED_Z2        ] = "Z"; // unknown dissonance, 2nd interval
	m_labels[UNLABELED_Z7        ] = "Z"; // unknown dissonance, 7th interval
	m_labels[UNLABELED_Z4        ] = "z"; // unknown dissonance, 4th interval
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
	m_labels[ECHAPPEE_UP         ] = "E"; // upper échappée
	m_labels[ECHAPPEE_DOWN       ] = "E"; // lower échappée
	m_labels[CAMBIATA_UP_S       ] = "C"; // ascending short nota cambiata
	m_labels[CAMBIATA_DOWN_S     ] = "C"; // descending short nota cambiata
	m_labels[CAMBIATA_UP_L       ] = "K"; // ascending long nota cambiata
	m_labels[CAMBIATA_DOWN_L     ] = "K"; // descending long nota cambiata
	m_labels[REV_CAMBIATA_UP     ] = "I"; // incomplete anterior upper neighbor
	m_labels[REV_CAMBIATA_DOWN   ] = "I"; // incomplete anterior lower neighbor
	m_labels[REV_ECHAPPEE_UP     ] = "J"; // incomplete posterior upper neighbor
	m_labels[REV_ECHAPPEE_DOWN   ] = "J"; // incomplete posterior lower neighbor
	m_labels[ANT_UP              ] = "A"; // rising anticipation
	m_labels[ANT_DOWN            ] = "A"; // descending anticipation
	m_labels[DBL_NEIGHBOR_UP     ] = "D"; // double neighbor beginning with upper neighbor
	m_labels[DBL_NEIGHBOR_DOWN   ] = "D"; // double neighbor beginning with lower neighbor
	m_labels[THIRD_Q_PASS_UP     ] = "Q"; // dissonant third quarter ascending passing tone
	m_labels[THIRD_Q_PASS_DOWN   ] = "Q"; // dissonant third quarter descending passing tone
	m_labels[THIRD_Q_UPPER_NEI   ] = "B"; // dissonant third quarter upper neighbor
	m_labels[THIRD_Q_LOWER_NEI   ] = "B"; // dissonant third quarter lower neighbor
	m_labels[ACC_PASSING_UP		 ] = "V"; // ascending accented passing tone
	m_labels[ACC_PASSING_DOWN	 ] = "V"; // descending accented passing tone
	m_labels[ACC_UP_NEI	 		 ] = "W"; // accented upper neighbor
	m_labels[ACC_LO_NEI			 ] = "W"; // accented lower neighbor
	m_labels[APP_UPPER			 ] = "T"; // appoggiatura resolving down by step
	m_labels[APP_LOWER			 ] = "T"; // appoggiatura resolving up by step
	m_labels[SUS_BIN             ] = "S"; // binary suspension
	m_labels[SUS_TERN            ] = "S"; // ternary suspension
	m_labels[AGENT_BIN           ] = "G"; // binary agent
	m_labels[AGENT_TERN          ] = "G"; // ternary agent
	m_labels[SUSPENSION_REP      ] = "R"; // suspension repeated note
	m_labels[FAKE_SUSPENSION_LEAP] = "F"; // fake suspension approached by leap
	m_labels[FAKE_SUSPENSION_STEP] = "F"; // fake suspension approached by step or anticipation
	m_labels[SUS_NO_AGENT_LEAP   ] = "M"; // suspension missing a normal agent approached by leap
	m_labels[SUS_NO_AGENT_STEP   ] = "M"; // suspension missing a normal agent approached by step or anticipation
	m_labels[CHANSON_IDIOM       ] = "H"; // chanson idiom
	m_labels[PARALLEL_UP         ] = "L"; // moves up in parallel with identifiable dissonance
	m_labels[PARALLEL_DOWN       ] = "L"; // moves down in parallel with identifiable dissonance
	m_labels[RES_PITCH			 ] = "X"; // note of resolution of a suspension against suspension dissonance
	m_labels[ONLY_WITH_VALID_UP  ] = "Y"; // only dissonant against identifiable dissonances, approached from below
	m_labels[ONLY_WITH_VALID_DOWN] = "Y"; // only dissonant against identifiable dissonances, approached from above
	m_labels[UNKNOWN_DISSONANCE  ] = "Z"; // unknown dissonance
	m_labels[UNLABELED_Z2        ] = "Z"; // unknown dissonance, 2nd interval
	m_labels[UNLABELED_Z7        ] = "Z"; // unknown dissonance, 7th interval
	m_labels[UNLABELED_Z4        ] = "Z"; // unknown dissonance, 4th interval
}


// END_MERGE

} // end namespace hum



