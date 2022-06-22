//
// Programmer:    Kiana Hu
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Fri Feb 18 22:00:48 PST 2022
// Last Modified: Sat May 14 12:49:23 PDT 2022
// Filename:      tool-cmr.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/tool-cmr.cpp
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Analyze Conspicuous Melodic Repetition (CMR).
//

#include "tool-cmr.h"
#include "Convert.h"

using namespace std;

namespace hum {

// START_MERGE

double cmr_note_info::m_syncopationWeight = 1.0;
double cmr_note_info::m_leapWeight = 0.5;

/////////////////////////////////
//
// Tool_cmr::Tool_cmr -- Set the recognized options for the tool.
//

Tool_cmr::Tool_cmr(void) {
	define("data|raw|raw-data=b",  "print analysis data");
	define("m|mark|marker=s:@",    "symbol to mark cmr notes");
	define("c|color=s:red",        "color of marked notes");
	define("r|ignore-rest=d:1.0",  "ignore rests smaller than given value (in whole notes)");
	define("n|number=i:3",         "number of high notes in a row");
	define("d|dur|duration=d:6.0", "maximum duration between cmr note attacks in whole notes");
	define("i|info=b",             "print cmr info");
	define("p|cmrs=b",             "detect only positive cmrs");
	define("t|troughs=b",          "detect only negative cmrs");
	define("A|not-accented=b",     "counts only cmrs that do not have melodic accentation");
	define("s|syncopation-weight=d:1.0","weight for syncopated notes");
	define("leap|leap-weight=d:0.5",    "weight for leapng notes");
	define("l|local-peaks=b",      "mark local peaks");
	define("L|only-local-peaks=b", "mark local peaks only");
}



/////////////////////////////////
//
// Tool_cmr::run -- Do the main work of the tool.
//

bool Tool_cmr::run(HumdrumFileSet& infiles) {
	bool status = true;
	for (int i=0; i<infiles.getCount(); i++) {
		status &= run(infiles[i]);
	}
	return status;
}


bool Tool_cmr::run(const string& indata, ostream& out) {
	HumdrumFile infile(indata);
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_cmr::run(HumdrumFile& infile, ostream& out) {
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_cmr::run(HumdrumFile& infile) {
	initialize();
	processFile(infile);
	return true;
}



//////////////////////////////
//
// Tool_cmr::initialize --  Initializations that only have to be done once
//    for all HumdrumFile segments.
//

void Tool_cmr::initialize(void) {
	m_rawQ         = getBoolean("raw-data");
	m_cmrQ         = getBoolean("cmrs");
	m_ncmrQ        = getBoolean("troughs");
	m_naccentedQ   = getBoolean("not-accented");
	m_localQ       = getBoolean("local-peaks");
	m_localOnlyQ   = getBoolean("only-local-peaks");
	if (m_localOnlyQ) {
		m_localQ = true;
	}

	m_marker       = getString("marker");
	m_color        = getString("color");

	cmr_note_info::m_syncopationWeight = getDouble("syncopation-weight");
	cmr_note_info::m_leapWeight = getDouble("leap-weight");

	m_smallRest    = getDouble("ignore-rest") * 4.0;  // convert from whole notes to quarter notes
	m_cmrNum       = getInteger("number");
	m_cmrDur       = getInteger("duration") * 4.0;    // convert from whole notes to quarter notes
	m_infoQ        = getBoolean("info");
	m_count        = 0;
}



//////////////////////////////
//
// Tool_cmr::processFile --
//

void Tool_cmr::processFile(HumdrumFile& infile) {
	// get list of music spines (columns):
	vector<HTp> starts = infile.getKernSpineStartList();

	m_local_count = 0;

	m_barNum = infile.getMeasureNumbers();

	// The first "spine" is the lowest part on the system.
	// The last "spine" is the highest part on the system.
	for (int i=0; i<(int)starts.size(); i++) {
		if (m_cmrQ) {
			processSpine(starts[i]);
		} else if (m_ncmrQ) {
			processSpineFlipped(starts[i]);
		} else {
			processSpine(starts[i]);
			processSpineFlipped(starts[i]);
		}
	}

	infile.createLinesFromTokens();

	if (!m_rawQ) {
		m_humdrum_text << infile;

		if (!m_localOnlyQ) {
			m_humdrum_text << "!!!RDF**kern: ";
			m_humdrum_text << m_marker;
			m_humdrum_text << " = marked note, color=";
			m_humdrum_text << m_color;
			m_humdrum_text << endl;
		}

		if (m_local_count > 0) {
			m_humdrum_text << "!!!RDF**kern: ";
			m_humdrum_text << m_local_marker;
			m_humdrum_text << " = marked note, color=";
			m_humdrum_text << m_local_color;
			m_humdrum_text << endl;
		}

		if (m_local_count_n > 0) {
			m_humdrum_text << "!!!RDF**kern: ";
			m_humdrum_text << m_local_marker_n;
			m_humdrum_text << " = marked note, color=";
			m_humdrum_text << m_local_color_n;
			m_humdrum_text << endl;
		}

	}

	if (!m_localOnlyQ) {
		postProcessAnalysis(infile);
	}
}



//////////////////////////////
//
// Tool_cmr::postProcessAnalysis --
//

void Tool_cmr::postProcessAnalysis(HumdrumFile& infile) {
	mergeOverlappingPeaks();

	int all_note_count = countNotesInScore(infile);

	int cmr_note_count = 0;

	/*

	for (int i=0; i<(int)m_noteGroups.size(); i++) {
		cmr_note_count += m_noteGroups[i].getNoteCount();
	}

	*/
	for (int i=0; i<(int)m_cmrIndex.size(); i++) {
		if (m_cmrIndex[i] < 0) {
			continue;
		}
		cmr_note_count += m_cmrPeakCount[i];
	}
	//print all statistics for cmr groups

	// if (m_infoQ) {
	m_humdrum_text << "!!!cmr_groups: " << m_count << endl;
	m_humdrum_text << "!!!cmr_notes: "  << cmr_note_count << endl;
	m_humdrum_text << "!!!score_notes: " << all_note_count << endl;
	// print density information for cmrs per mille
	m_humdrum_text << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"  << endl;
	m_humdrum_text << "!!!cmr_note_density: " << ((double)cmr_note_count / all_note_count) * 1000 << " permil" << endl;
	m_humdrum_text << "!!!cmr_group_density: " << ((double)m_count / all_note_count) * 1000 << " permil" << endl;

	int pcounter = 1;
	/* 
		for (int i=0; i<(int)m_noteGroups.size(); i++) {
		
		}
	*/
	for (int i=0; i<(int)m_cmrIndex.size(); i++) {
		if (m_cmrIndex[i] < 0) {
			// This group has been merged into a larger one.
			continue;
		}
		m_humdrum_text << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"  << endl;
		m_humdrum_text << "!!!cmr_group: "     << pcounter++            << endl;
		m_humdrum_text << "!!!start_measure: "  << m_cmrMeasureBegin[i] << endl;
		m_humdrum_text << "!!!end_measure: "    << m_cmrMeasureEnd[i]   << endl;
		m_humdrum_text << "!!!group_duration: " << m_cmrDuration[i].getFloat()/4.0 << endl;
		m_humdrum_text << "!!!group_pitches:";
		for (int j=0; j<(int)m_cmrPitch[i].size(); j++) {
			m_humdrum_text << " " << m_cmrPitch[i][j];
			m_humdrum_text << "(" << m_cmrPitch[i][j]->getLineIndex() << ")";
		}
		m_humdrum_text << endl;
		m_humdrum_text << "!!!group_cmrcount: " << m_cmrPeakCount[i]    << endl;
	}
}



//////////////////////////////
//
// mergeOverlappingPeaks -- Merge overlapping cmr groups.
//    Groups that need to be merged:
//    * Have the same track number (same staff)
//    * Have the same MIDI pitch
//    * Have and starttime for one group that starts before or on the
//      endtime of another group.
//    Merged groups are indicated as inactive if their index is set to
//    a negative value.
//

void Tool_cmr::mergeOverlappingPeaks(void) {
	// This algorithm does not handle multiple groups that
	// need merging, so redo the overlap identification
	// several more times to enture multiple groups are
	// merged.
	for (int k=0; k<100; k++) {
		bool mergers = false;
		for (int i=0; i<(int)m_cmrIndex.size(); i++) {
			for (int j=i+1; j<(int)m_cmrIndex.size(); j++) {
				mergers |= checkGroupPairForMerger(i, j);
			}
		}
		if (!mergers) {
			break;
		}
	}

	// re-calculate m_count (number of cmr groups):
	m_count = 0;
	for (int i=0; i<(int)m_cmrIndex.size(); i++) {
		if (m_cmrIndex[i] >= 0) {
			m_count++;
		}
	}
}


//////////////////////////////
//
// Tool_cmr::checkGroupPairForMerger --
//    * Have and starttime for one group that starts before or on the
//      endtime of another group.
//    Merged groups are indicated as inactive if their index is set to
//    a negative value.
// Return value is true if there was a merger; otherwise, returns false.
//

bool Tool_cmr::checkGroupPairForMerger(int index1, int index2) {

	// Groups must not have been merged already:
	if (m_cmrIndex[index1] < 0) {
		return false;
	}
	if (m_cmrIndex[index2] < 0) {
		return false;
	}

	// Groups must have the same track number (i.e., the same staff/part):
	if (m_cmrTrack[index1] != m_cmrTrack[index2]) {
		return false;
	}

	// Groups must have the same MIDI pitch:
	if (m_cmrPitch[index1].empty()) {
		return false;
	}
	if (m_cmrPitch[index2].empty()) {
		return false;
	}
	int midi1 = m_cmrPitch[index1][0]->getMidiPitch();
	int midi2 = m_cmrPitch[index2][0]->getMidiPitch();
	if (midi1 != midi2) {
		return false;
	}

	// Check if they overlap:
	HumNum start1 = m_startTime[index1];
	HumNum start2 = m_startTime[index2];
	HumNum end1   = m_endTime[index1];
	HumNum end2   = m_endTime[index2];

	bool mergeQ = false;
	bool flipQ  = false;
	if (start1 < start2) {
		if (start2 <= end1) {
			mergeQ = true;
		}
	} else {
		if (start1 <= end2) {
			flipQ = true;
			mergeQ = true;
		}
	}

	if (mergeQ == false) {
		return false;
	}

	// merge the two groups:
	if (flipQ) {
		int tempi = index1;
		index1 = index2;
		index2 = tempi;
	}

	// Deactivate the second group by setting a negative index:
	m_cmrIndex[index2] *= -1;

	// Set the endtime of the first group to the end of the second group:
	m_endTime[index1] = m_endTime[index2];

	// Likewise, merge the ending measure numbers:
	m_cmrMeasureEnd[index1] = m_cmrMeasureEnd[index2];

	// Update the duration of the merged cmr group:
	m_cmrDuration[index1] = m_endTime[index2] - m_startTime[index1];

	// merge the notes/counts:
	for (int i=0; i<(int)m_cmrPitch[index2].size(); i++) {
		vector<HTp> newtoks;
		newtoks.clear();
		for (int j=0; j<(int)m_cmrPitch[index1].size(); j++) {
			HTp token1 = m_cmrPitch[index1][j];
			HTp token2 = m_cmrPitch[index2][i];
			if (token2 == NULL) {
				continue;
			}
			if (token1 == token2) {
				m_cmrPitch[index2][i] = NULL;
			}
		}
	}

	for (int k=0; k<(int)m_cmrPitch[index2].size(); k++) {
		HTp token = m_cmrPitch[index2][k];
		if (!token) {
			continue;
		}
		m_cmrPitch[index1].push_back(token);
	}

	m_cmrPeakCount[index1] = m_cmrPitch[index1].size();

	return true;
}



//////////////////////////////
//
// Tool_cmr::processSpine -- Process one part.  Only the first voice/layer
//    on the staff will be processed (so only the top part if there is a divisi).
//

void Tool_cmr::processSpine(HTp startok) {
	// notelist is a two dimensional array of notes.   The
	// first dimension is a list of the note attacks in time
	// (plus rests), and the second dimension is for a list of the
	// tied notes after the first one (this is so that we can
	// highlight both the starting note and any tied notes to that
	// starting note later).
	vector<vector<HTp>> notelist = getNoteList(startok);

	// midinums: MIDI note numbers for each note (with rests being 0).
	vector<int> midinums = getMidiNumbers(notelist);

	// cmrnotesQ: True = the note is a local high pitch.
	vector<bool> cmrnotesQ(midinums.size(), false);
	identifyLocalPeaks(cmrnotesQ, midinums);

	if (m_localQ) {
		markNotes(notelist, cmrnotesQ, m_local_marker);
	}
	if (m_localOnlyQ) {
		return;
	}

	// cmrnotelist: Only the local cmr notes which will be extracted
	// from all note list in the getLocalPeakNotes() function.
	vector<vector<HTp>> cmrnotelist;
	getLocalPeakNotes(cmrnotelist, notelist, cmrnotesQ);

	// cmrmidinums: MIDI note numbers for cmrnotelist notes.
	vector<int> cmrmidinums = getMidiNumbers(cmrnotelist);

	// globalcmrnotes: boolean list that indicates if a local
	// cmr note is part of a longer sequence of cmr notes.
	// This variable will be filled in by identifyPeakSequence().
	vector<bool> globalcmrnotes(cmrnotelist.size(), false);
	identifyPeakSequence(globalcmrnotes, cmrmidinums, cmrnotelist);

	if (m_rawQ) {
		printData(notelist, midinums, cmrnotesQ);
	} else {
		markNotesInScore(cmrnotelist, globalcmrnotes);
	}
}



//////////////////////////////
//
// Tool_cmr::markNotes -- mark notes in list that are true
//     with given marker.
//

void Tool_cmr::markNotes(vector<vector<HTp>>& notelist,
		vector<bool>& cmrnotesQ, const string& marker) {
	bool negative = false;
	if (marker == m_local_marker_n) {
		negative = true;
	}
	for (int i=0; i<(int)cmrnotesQ.size(); i++) {
		if (!cmrnotesQ[i]) {
			continue;
		}
		for (int j=0; j<(int)notelist.at(i).size(); j++) {
			string text = *notelist[i][j];
			if (text.find(marker) == string::npos) {
				text += marker;
				notelist[i][j]->setText(text);
				if (negative) {
            	m_local_count_n++;
				} else {
            	m_local_count++;
				}
			}
		}
	}
}



//////////////////////////////
//
// Tool_cmr::processSpineFlipped -- Similar to processSpine(), but
//    searches for minima CMRs rather than maxima CMRs.
//

void Tool_cmr::processSpineFlipped(HTp startok) {
	// notelist is a two dimensional array of notes.   The
	// first dimension is a list of the note attacks in time
	// (plus rests), and the second dimension is for a list of the
	// tied notes after the first one (this is so that we can
	// highlight both the starting note and any tied notes to that
	// starting note later).
	vector<vector<HTp>> notelist = getNoteList(startok);

	// midinums: MIDI note numbers for each note (with rests being 0).
	vector<int> midinums = getMidiNumbers(notelist);
	midinums = flipMidiNumbers(midinums);

	// cmrnotesQ: True = the note is a local high pitch.
	vector<bool> cmrnotesQ(midinums.size(), false);
	identifyLocalPeaks(cmrnotesQ, midinums);

	if (m_localQ) {
		markNotes(notelist, cmrnotesQ, m_local_marker_n);
	}
	if (m_localOnlyQ) {
		return;
	}

	// cmrnotelist: Only the local cmr notes which will be extracted
	// from all note list in the getLocalPeakNotes() function.
	vector<vector<HTp>> cmrnotelist;
	getLocalPeakNotes(cmrnotelist, notelist, cmrnotesQ);

	// cmrmidinums: MIDI note numbers for cmrnotelist notes.
	vector<int> cmrmidinums = getMidiNumbers(cmrnotelist);

	// globalcmrnotes: boolean list that indicates if a local
	// cmr note is part of a longer sequence of cmr notes.
	// This variable will be filled in by identifyPeakSequence().
	vector<bool> globalcmrnotes(cmrnotelist.size(), false);
	identifyPeakSequence(globalcmrnotes, cmrmidinums, cmrnotelist);

	if (m_rawQ) {
		printData(notelist, midinums, cmrnotesQ);
	} else {
		markNotesInScore(cmrnotelist, globalcmrnotes);
	}
}


vector<int> Tool_cmr::flipMidiNumbers(vector<int>& midinums) {
	for (int i=0; i<(int)midinums.size(); i++) {
		if (midinums[i] == 0) {
			continue;
		}
		int flippedMidiNum = (midinums[i] * -1) + 128;
		midinums[i] = flippedMidiNum;
	}
	return midinums;
}

//////////////////////////////
//
// Tool_cmr::markNotesInScore --
//

void Tool_cmr::markNotesInScore(vector<vector<HTp>>& cmrnotelist, vector<bool>& iscmr) {
	for (int i=0; i<(int)cmrnotelist.size(); i++) {
		if (!iscmr[i]) {
			continue;
		}
		for (int j=0; j<(int)cmrnotelist[i].size(); j++) {
			string text = *(cmrnotelist[i][j]);
			text += m_marker;
			cmrnotelist[i][j]->setText(text);
		}
	}
}



//////////////////////////////
//
// Tool_cmr::getLocalPeakNotes -- Throw away notes/rests that are not cmrs.
//

void Tool_cmr::getLocalPeakNotes(vector<vector<HTp>>& newnotelist,
		vector<vector<HTp>>& oldnotelist, vector<bool>& cmrnotesQ) {

	// durations == duration of notes in quarter-note units
	vector<double> durations;
	getDurations(durations, oldnotelist);

	// strongbeat == true if on a beat (whole-note metric postion).
	vector<bool> strongbeat;
	getBeat(strongbeat, oldnotelist);


	////////////////////////////
	//
	// Refinement to add to following loop: If the note has
	// a duration less than or equal two 2 (half note), and
	// the note is not on a beat then do not add it to the
	// newnotelist vector.
	//
	////////////////////////////

	newnotelist.clear();
	for (int i=0; i<(int)cmrnotesQ.size(); i++) {
		if ((durations[i] <= 2) && (strongbeat[i] == false)) {
			continue;
		}
		if (cmrnotesQ[i]) {
			newnotelist.push_back(oldnotelist[i]);
		}
	}

}



//////////////////////////////
//
// Tool_cmr::identifyLocalPeaks -- Identify notes that are higher than their
//    adjacent neighbors.  The midinumbs are MIDI note numbers (integers)
//    for the pitch, with higher number meaning higher pitches.  Rests are
//    the value 0.  Do not assign a note as a cmr note if one of the
//    adjacent notes is a rest. (This could be refined later, such as ignoring
//    short rests).
//

void Tool_cmr::identifyLocalPeaks(vector<bool>& cmrnotesQ, vector<int>& midinums) { //changed to midinums from 'notelist'
	for (int i=1; i<(int)midinums.size() - 1; i++) {
		if ((midinums[i - 1] <= 0) && (midinums[i + 1] <= 0)) { //not sandwiched by rests
			continue;
		} else if (midinums[i] <= 0) {
			continue;
		}
		if ((midinums[i] > midinums[i - 1]) && (midinums[i + 1] == 0)) { //allow rest after note
			cmrnotesQ[i] = 1;
		}
		if ((midinums[i - 1] == 0) && (midinums[i] > midinums[i + 1])) { //allow rest before note
			cmrnotesQ[i] = 1;
		}
		if ((midinums[i] > midinums[i - 1]) && (midinums[i] > midinums[i + 1])) { //check neighboring notes
			cmrnotesQ[i] = 1;
		}
	}
}



//////////////////////////////
//
// Tool_cmr::identifyPeakSequence -- Identify a sequence of local cmrs that can form
//       a cmr sequence
//
// Input variables:
//      globalcmrnotes -- output data that is set to true if the note is part of a
//                         global cmr note sequence.  This vector has the same size
//                         as the cmrmidinums vector and the values are initially set
//                         to false.  This function will set notes that are part of a sequence
//                         of local cmr notes (with length m_cmrNum) to true.
//      cmrmidinums    -- MIDI number vector for all local cmr notes.
//      notes           -- double vector of Humdrum notes (needed to get timestamps).
//
// Member variables that are needed in the Algorithm:
//    int    m_cmrNum: Number of consecutive local cmrnotes needed to mark global cmr.
//    double m_cmrDur: Maximum duration of the first/last cmr note in global cmr sequence.
//
//

void Tool_cmr::identifyPeakSequence(vector<bool>& globalcmrnotes, vector<int>& cmrmidinums,
		vector<vector<HTp>>& notes) {

	// Set initial positions of globalcmrnotes to false:
	globalcmrnotes.resize(cmrmidinums.size());
	fill(globalcmrnotes.begin(), globalcmrnotes.end(), false);
	// The code below (under Algorithms) will set notes identified as a "global cmr" to true
	// in this vector.

	// Get the timestamps of all local cmr notes:
	vector<double> timestamps(notes.size(), 0.0);
	for (int i=0; i<(int)notes.size(); i++) {
		timestamps[i] = notes[i][0]->getDurationFromStart().getFloat();
	}

	///////////////////////////////////////////
	//
	// Algorithm:
	//
	//    * Loop through each element in the cmrmidinums vector checking if it is a part of
	//      a longer cmr sequence of the same MIDI note number.  The loop will start at
	//      index 0 and go until but not including cmrmidinums.size() - m_cmrNum.
	//
	//    * A "global cmr" means that there are m_cmrNum MIDI note numbers in a row in cmrmidinums.
	//      (all of the cmr notes have to be the same MIDI note number).
	//
	//    *  In addition, the time difference between the starting and ending note of the cmr
	//       sequence has to be equal to or less than m_cmrDur (6 whole notes by default).
	//       Subtract the timestamp of the last and first notes and see if the duration between
	//       these notes is equal or less than m_cmrDur.
	//
	//////////////////////////////////////////

	for (int i=0; i<(int)cmrmidinums.size() - m_cmrNum; i++) {
		bool match = true;
		bool accented = isMelodicallyAccented(notes[i][0]);
		for (int j=1; j<m_cmrNum; j++) {
			accented |= isMelodicallyAccented(notes[i+j][0]);
			if (cmrmidinums[i+j] != cmrmidinums[i+j-1]) {
				match = false;
				break;
			}
		}
		if (!match) {
			continue;
		}
		if ((!m_naccentedQ) && (!accented)){
			continue;
		}
		if ((m_naccentedQ) && (accented)) {
			continue;
		}

		HumNum duration = timestamps[i + m_cmrNum - 1] - timestamps[i];
		if (duration.getFloat() > m_cmrDur) {
			continue;
		}
		//data for every sub-sequeunce
		m_count += 1;
		int line = notes[i][0]->getLineIndex();
		int line2 = notes[i + m_cmrNum - 1].back()->getLineIndex();

		m_cmrDuration.push_back(duration.getFloat()/4.0);
		m_cmrMeasureBegin.push_back(m_barNum[line]);
		m_cmrMeasureEnd.push_back(m_barNum[line2]);
		vector<HTp> pnotes;
		for (int j=0; j<m_cmrNum; j++) {
			pnotes.push_back(notes.at(i+j).at(0));
		}
		m_cmrPitch.push_back(pnotes);
		m_cmrPeakCount.push_back((int)pnotes.size());

		// variables to do cmr group mergers later:
		int track = notes[i][0]->getTrack();
		m_cmrTrack.push_back(track);
		m_cmrIndex.push_back(m_cmrIndex.size());
		HumNum starttime = notes[i][0]->getDurationFromStart();
		HumNum endtime   = notes[i+m_cmrNum-1].back()->getDurationFromStart();
		m_startTime.push_back(starttime);
		m_endTime.push_back(endtime);

		for (int j=0; j<m_cmrNum; j++) {
			globalcmrnotes[i+j] = true;
		}
	}
}



//////////////////////////////
//
// Tool_cmr::printData -- Print input and output data.  First column is the MIDI note
//      number, second one is the cmr analysis (true=local maximum note)
//

void Tool_cmr::printData(vector<vector<HTp>>& notelist, vector<int>& midinums, vector<bool>& cmrnotes) {
	m_free_text << "MIDI\tPEAK\tKERN" << endl;
	for (int i=0; i<(int)notelist.size(); i++) {
		m_free_text << midinums.at(i) << "\t";
		m_free_text << cmrnotes.at(i);
		for (int j=0; j<(int)notelist[i].size(); j++) {
			m_free_text << "\t" << notelist[i][j];
		}
		m_free_text << endl;
	}
	m_free_text << "******************************************" << endl;
	m_free_text << endl;
}



//////////////////////////////
//
// Tool_cmr::getMidiNumbers -- convert note tokens into MIDI note numbers.
//    60 = middle C (C4), 62 = D4, 72 = C5, 48 = C3.
//

vector<int> Tool_cmr::getMidiNumbers(vector<vector<HTp>>& notelist) {
	vector<int> output(notelist.size(), 0);  // fill with rests by default
	for (int i=0; i<(int)notelist.size(); i++) {
		output[i] = Convert::kernToMidiNoteNumber(notelist.at(i).at(0));
		if (output[i] < 0) {
			// Set rests to be 0
			output[i] = 0;
		}
	}
	return output;
}



//////////////////////////////
//
// Tool_cmr::getNoteList -- Return a list of the notes and rests for
//     a part, with the input being the starting token of the part from
//     which the note list should be extracted.  The output is a two-
//     dimensional vector.  The first dimension is for the list of notes,
//     and the second dimension is used to store any subsequent tied notes
//     so that they can be marked and highlighted in the score.
//

vector<vector<HTp>> Tool_cmr::getNoteList(HTp starting) {
	vector<vector<HTp>> tempout;
	tempout.reserve(2000);

	HTp previous = starting;
	HTp current = starting;
	while (current) {
		if (!current->isData()) {
			previous = current;
			current = current->getNextToken();
			continue;
		}
		if (current->isNull()) {
			previous = current;
			current = current->getNextToken();
			continue;
		}
		if (current->isNoteSustain()) {
			if (tempout.size() > 0) {
				tempout.back().push_back(current);
			}
			previous = current;
			current = current->getNextToken();
			continue;
		}
		if (current->isRest()) {
			if (previous->isRest()) {
				// do not store previous rest
				previous = current;
				current = current->getNextToken();
				continue;
			}
		}
		tempout.resize(tempout.size() + 1);
		tempout.back().push_back(current);
		if (!current->isRest()) {
			m_noteCount++;
		}
		previous = current;
		current = current->getNextToken();
	}

	// Remove any rests that are shorter or equal to m_shortRest:
	vector<vector<HTp>> output;
	output.reserve(tempout.size());
	for (int i=0; i<(int)tempout.size() - 1; i++) {
		if (!tempout[i][0]->isRest()) {
			output.push_back(tempout[i]);
			continue;
		}
		// get the duration of the (multi-rest):
		HumNum restStart = tempout[i][0]->getDurationFromStart();
		HumNum noteStart = tempout[i+1][0]->getDurationFromStart();
		HumNum duration = noteStart - restStart;
		if (duration.getFloat() > m_smallRest) {
			output.push_back(tempout[i]);
		}
	}

	return output;
}



//////////////////////////////
//
// getNoteDurations --
//

void  Tool_cmr::getDurations(vector<double>& durations, vector<vector<HTp>>& notelist) {
	durations.resize(notelist.size());
	for (int i=0; i<(int)notelist.size(); i++) {
		HumNum duration = notelist[i][0]->getTiedDuration();
		durations[i] = duration.getFloat();
	}
}



//////////////////////////////
//
// Tool_cmr::getBeat --
//

void  Tool_cmr::getBeat(vector<bool>& metpos, vector<vector<HTp>>& notelist) {
	metpos.resize(notelist.size());
	for (int i=0; i<(int)notelist.size(); i++) {
		HumNum position = notelist[i][0]->getDurationFromBarline();
		if (position.getDenominator() != 1) {
			metpos[i] = false;
		} if (position.getNumerator() % 4 == 0) {
			metpos[i] = true;
		} else {
			metpos[i] = false;
		}
	}
}



//////////////////////////////
//
// Tool_cmr::getMetricLevel --
//

int  Tool_cmr::getMetricLevel(HTp token) {
	HumNum beat = token->getDurationFromBarline();
	if (!beat.isInteger()) { // anything less than quarter note level
		return -1;
	}
	if (beat.getNumerator() % 4 == 0) { // whole note level
		return 2;
	}
	if (beat.getNumerator() % 2 == 0) { // half note level
		return 1;
	} else { // quarter note level
		return 0;
	}
}



//////////////////////////////
//
// Tool_cmr::isMelodicallyAccented --
//

bool  Tool_cmr::isMelodicallyAccented(HTp token) {
	return hasLeapBefore(token) || isSyncopated(token);
}


//////////////////////////////
//
// Tool_cmr::hasLeapBefore --
//

bool  Tool_cmr::hasLeapBefore(HTp token) {
	HTp current = token->getPreviousToken();
	int startNote = token->getMidiPitch();
	while (current) {
		if (!current->isData()) {
			current = current->getPreviousToken();
			continue;
		}
		if (current->isNull()) {
			current = current->getPreviousToken();
			continue;
		}
		if (current->isRest()) {
			current = current->getPreviousToken();
			continue;
		}
		int testNote = current->getMidiPitch();
		int interval = startNote - testNote;
		return interval > 2;
	}
	return false;
}



//////////////////////////////
//
// Tool_cmr::isSyncopated --
//

bool  Tool_cmr::isSyncopated(HTp token) {
	HumNum dur = token->getTiedDuration();
	double logDur = log2(dur.getFloat());
	int metLev = getMetricLevel(token);
	if (metLev >= 2) { // no syncopations occuring on whole-note level or higher
		return false;
	}
	if (logDur > metLev) {
		return true;
	} else {
		return false;
	}
}



//////////////////////////////
//
// Tool_cmr::countNotesInScore --
//

int Tool_cmr::countNotesInScore(HumdrumFile& infile) {
	int counter = 0;
	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isData()) {
			continue;
		}
		for (int j=0; j<infile[i].getFieldCount(); j++) {
			HTp token = infile.token(i, j);
			if (!token->isKern()) {
				continue;
			}
			if (token->isNull()) {
				continue;
			}
			if (token->isRest()) {
				continue;
			}
		  if (token->isSecondaryTiedNote()) {
				continue;
			}
			counter++;

		}
	}
	return counter;
}



// END_MERGE

} // end namespace hum
