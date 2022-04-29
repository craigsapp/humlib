//
// Programmer:    Kiana Hu
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Fri Feb 18 22:00:48 PST 2022
// Last Modified: Mon Feb 28 11:05:24 PST 2022
// Filename:      tool-peak.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/tool-peak.cpp
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Analyze high-points in melodies.
//

#include "tool-peak.h"
#include "Convert.h"

using namespace std;

namespace hum {

// START_MERGE


/////////////////////////////////
//
// Tool_peak::Tool_peak -- Set the recognized options for the tool.
//

Tool_peak::Tool_peak(void) {
	define("data|raw|raw-data=b",  "print analysis data");
	define("m|mark|marker=s:@",    "symbol to mark peak notes");
	define("c|color=s:red",        "color of marked notes");
	define("r|ignore-rest=d:1.0",  "ignore rests smaller than given value (in whole notes)");
	define("n|number=i:3",         "number of high notes in a row");
	define("d|dur|duration=d:6.0", "maximum duration between peak note attacks in whole notes");
	define("i|info=b",             "print peak info");
	define("p|peaks=b",            "detect only peaks");
	define("t|troughs=b",          "detect only negative peaks");
	define("S|not_syncopated=b",   "counts only peaks that do not have syncopation");
}



/////////////////////////////////
//
// Tool_peak::run -- Do the main work of the tool.
//

bool Tool_peak::run(HumdrumFileSet& infiles) {
	bool status = true;
	for (int i=0; i<infiles.getCount(); i++) {
		status &= run(infiles[i]);
	}
	return status;
}


bool Tool_peak::run(const string& indata, ostream& out) {
	HumdrumFile infile(indata);
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_peak::run(HumdrumFile& infile, ostream& out) {
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_peak::run(HumdrumFile& infile) {
	initialize();
	processFile(infile);
	return true;
}



//////////////////////////////
//
// Tool_peak::initialize --  Initializations that only have to be done once
//    for all HumdrumFile segments.
//

void Tool_peak::initialize(void) {
	m_rawQ      = getBoolean("raw-data");
	m_peakQ     = getBoolean("peaks");
	m_npeakQ    = getBoolean("troughs");
	m_nsyncoQ   = getBoolean("not_syncopated");
	m_marker    = getString("marker");
	m_color     = getString("color");
	m_smallRest = getDouble("ignore-rest") * 4.0;  // convert to quarter notes
	m_peakNum   = getInteger("number");
	m_peakDur   = getInteger("duration") * 4.0;    // convert to quarter notes
	m_infoQ     = getBoolean("info");
	m_count     = 0;
}



//////////////////////////////
//
// Tool_peak::processFile --
//

void Tool_peak::processFile(HumdrumFile& infile) {
	// get list of music spines (columns):
	vector<HTp> starts = infile.getKernSpineStartList();

	m_barNum = infile.getMeasureNumbers();

	// The first "spine" is the lowest part on the system.
	// The last "spine" is the highest part on the system.
	for (int i=0; i<(int)starts.size(); i++) {
		if (m_peakQ) {
			processSpine(starts[i]);
		} else if (m_npeakQ) {
			processSpineFlipped(starts[i]);
		} else {
			processSpine(starts[i]);
			processSpineFlipped(starts[i]);
		}
	}

	infile.createLinesFromTokens();
	if (!m_rawQ) {
		m_humdrum_text << infile;
		m_humdrum_text << "!!!RDF**kern: ";
		m_humdrum_text << m_marker;
		m_humdrum_text << " = marked note, color=";
		m_humdrum_text << m_color;
		m_humdrum_text << endl;
	}

	mergeOverlappingPeaks();

	int all_note_count = countNotesInScore(infile);

	int peak_note_count = 0;
	for (int i=0; i<(int)m_peakIndex.size(); i++) {
		if (m_peakIndex[i] < 0) {
			continue;
		}
		peak_note_count += m_peakPeakCount[i];
	}
	//print all statistics for peak groups

	//if (m_infoQ) {
  m_humdrum_text << "!!!peak_groups: " << m_count << endl;
  m_humdrum_text << "!!!peak_notes: "  << peak_note_count << endl;
  m_humdrum_text << "!!!score_notes: " << all_note_count << endl;
	//print density information for peaks in myriads
	m_humdrum_text << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"  << endl;
	m_humdrum_text << "!!!peak_note_density (myriad): " << ((double)peak_note_count / all_note_count) * 1000 << endl;
	m_humdrum_text << "!!!peak_group_density (myriad): " << ((double)m_count / all_note_count) * 1000 << endl;

	//print density information for peaks in percents
	m_humdrum_text << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"  << endl;
	m_humdrum_text << "!!!peak_note_density (percentage): " << ((double)peak_note_count / all_note_count) * 100 << endl;
	m_humdrum_text << "!!!peak_group_density (percentage): " << ((double)m_count / all_note_count) * 100 << endl;

  int pcounter = 1;
  for (int i=0; i<(int)m_peakIndex.size(); i++) {
  	if (m_peakIndex[i] < 0) {
  		// This group has been merged into a larger one.
  		continue;
  	}
  	m_humdrum_text << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"  << endl;
  	m_humdrum_text << "!!!peak_group: "     << pcounter++            << endl;
  	m_humdrum_text << "!!!start_measure: "  << m_peakMeasureBegin[i] << endl;
  	m_humdrum_text << "!!!end_measure: "    << m_peakMeasureEnd[i]   << endl;
  	m_humdrum_text << "!!!group_duration: " << m_peakDuration[i].getFloat()/4.0 << endl;
  	m_humdrum_text << "!!!group_pitches:";
  	for (int j=0; j<(int)m_peakPitch[i].size(); j++) {
  		m_humdrum_text << " " << m_peakPitch[i][j];
  		m_humdrum_text << "(" << m_peakPitch[i][j]->getLineIndex() << ")";
  	}
  	m_humdrum_text << endl;
  	m_humdrum_text << "!!!group_peakcount: " << m_peakPeakCount[i]    << endl;
  }
	//}
}



//////////////////////////////
//
// mergeOverlappingPeaks -- Merge overlapping peak groups.
//    Groups that need to be merged:
//    * Have the same track number (same staff)
//    * Have the same MIDI pitch
//    * Have and starttime for one group that starts before or on the
//      endtime of another group.
//    Merged groups are indicated as inactive if their index is set to
//    a negative value.
//

void Tool_peak::mergeOverlappingPeaks(void) {
	// This algorithm does not handle multiple groups that
	// need merging, so redo the overlap identification
	// several more times to enture multiple groups are
	// merged.
	for (int k=0; k<100; k++) {
		bool mergers = false;
		for (int i=0; i<(int)m_peakIndex.size(); i++) {
			for (int j=i+1; j<(int)m_peakIndex.size(); j++) {
				mergers |= checkGroupPairForMerger(i, j);
			}
		}
		if (!mergers) {
			break;
		}
	}

	// re-calculate m_count (number of peak groups):
	m_count = 0;
	for (int i=0; i<(int)m_peakIndex.size(); i++) {
		if (m_peakIndex[i] >= 0) {
			m_count++;
		}
	}
}


//////////////////////////////
//
// Tool_peak::checkGroupPairForMerger --
//    * Have and starttime for one group that starts before or on the
//      endtime of another group.
//    Merged groups are indicated as inactive if their index is set to
//    a negative value.
// Return value is true if there was a merger; otherwise, returns false.
//

bool Tool_peak::checkGroupPairForMerger(int index1, int index2) {

	// Groups must not have been merged already:
	if (m_peakIndex[index1] < 0) {
		return false;
	}
	if (m_peakIndex[index2] < 0) {
		return false;
	}

	// Groups must have the same track number (i.e., the same staff/part):
	if (m_peakTrack[index1] != m_peakTrack[index2]) {
		return false;
	}

	// Groups must have the same MIDI pitch:
	if (m_peakPitch[index1].empty()) {
		return false;
	}
	if (m_peakPitch[index2].empty()) {
		return false;
	}
	int midi1 = m_peakPitch[index1][0]->getMidiPitch();
	int midi2 = m_peakPitch[index2][0]->getMidiPitch();
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
	m_peakIndex[index2] *= -1;

	// Set the endtime of the first group to the end of the second group:
	m_endTime[index1] = m_endTime[index2];

	// Likewise, merge the ending measure numbers:
	m_peakMeasureEnd[index1] = m_peakMeasureEnd[index2];

	// Update the duration of the merged peak group:
	m_peakDuration[index1] = m_endTime[index2] - m_startTime[index1];

	// merge the notes/counts:
	for (int i=0; i<(int)m_peakPitch[index2].size(); i++) {
		vector<HTp> newtoks;
		newtoks.clear();
		for (int j=0; j<(int)m_peakPitch[index1].size(); j++) {
			HTp token1 = m_peakPitch[index1][j];
			HTp token2 = m_peakPitch[index2][i];
			if (token2 == NULL) {
				continue;
			}
			if (token1 == token2) {
				m_peakPitch[index2][i] = NULL;
			}
		}
	}

	for (int k=0; k<(int)m_peakPitch[index2].size(); k++) {
		HTp token = m_peakPitch[index2][k];
		if (!token) {
			continue;
		}
		m_peakPitch[index1].push_back(token);
	}

	m_peakPeakCount[index1] = m_peakPitch[index1].size();

	return true;
}



//////////////////////////////
//
// Tool_peak::processSpine -- Process one line of music.
//

void Tool_peak::processSpine(HTp startok) {
	// notelist is a two dimensional array of notes.   The
	// first dimension is a list of the note attacks in time
	// (plus rests), and the second dimension is for a list of the
	// tied notes after the first one (this is so that we can
	// highlight both the starting note and any tied notes to that
	// starting note later).
	vector<vector<HTp>> notelist = getNoteList(startok);

	// midinums: MIDI note numbers for each note (with rests being 0).
	vector<int> midinums = getMidiNumbers(notelist);

	// peaknotes: True = the note is a local high pitch.
	vector<bool> peaknotes(midinums.size(), false);
	identifyLocalPeaks(peaknotes, midinums);

	// peaknotelist: Only the local peak notes which will be extracted
	// from all note list in the getLocalPeakNotes() function.
	vector<vector<HTp>> peaknotelist;
	getLocalPeakNotes(peaknotelist, notelist, peaknotes);

	// peakmidinums: MIDI note numbers for peaknotelist notes.
	vector<int> peakmidinums = getMidiNumbers(peaknotelist);

	// globalpeaknotes: boolean list that indicates if a local
	// peak note is part of a longer sequence of peak notes.
	// This variable will be filled in by identifyPeakSequence().
	vector<bool> globalpeaknotes(peaknotelist.size(), false);
	identifyPeakSequence(globalpeaknotes, peakmidinums, peaknotelist);

	if (m_rawQ) {
		printData(notelist, midinums, peaknotes);
	} else {
		markNotesInScore(peaknotelist, globalpeaknotes);
	}
}

void Tool_peak::processSpineFlipped(HTp startok) {
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

	// peaknotes: True = the note is a local high pitch.
	vector<bool> peaknotes(midinums.size(), false);
	identifyLocalPeaks(peaknotes, midinums);

	// peaknotelist: Only the local peak notes which will be extracted
	// from all note list in the getLocalPeakNotes() function.
	vector<vector<HTp>> peaknotelist;
	getLocalPeakNotes(peaknotelist, notelist, peaknotes);

	// peakmidinums: MIDI note numbers for peaknotelist notes.
	vector<int> peakmidinums = getMidiNumbers(peaknotelist);

	// globalpeaknotes: boolean list that indicates if a local
	// peak note is part of a longer sequence of peak notes.
	// This variable will be filled in by identifyPeakSequence().
	vector<bool> globalpeaknotes(peaknotelist.size(), false);
	identifyPeakSequence(globalpeaknotes, peakmidinums, peaknotelist);

	if (m_rawQ) {
		printData(notelist, midinums, peaknotes);
	} else {
		markNotesInScore(peaknotelist, globalpeaknotes);
	}
}


vector<int> Tool_peak::flipMidiNumbers(vector<int>& midinums) {
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
// Tool_peak::markNotesInScore --
//

void Tool_peak::markNotesInScore(vector<vector<HTp>>& peaknotelist, vector<bool>& ispeak) {
	for (int i=0; i<(int)peaknotelist.size(); i++) {
		if (!ispeak[i]) {
			continue;
		}
		for (int j=0; j<(int)peaknotelist[i].size(); j++) {
			string text = *(peaknotelist[i][j]);
			text += m_marker;
			peaknotelist[i][j]->setText(text);
		}
	}
}



//////////////////////////////
//
// Tool_peak::getLocalPeakNotes -- Throw away notes/rests that are not peaks.
//

void Tool_peak::getLocalPeakNotes(vector<vector<HTp>>& newnotelist,
		vector<vector<HTp>>& oldnotelist, vector<bool>& peaknotes) {

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
	for (int i=0; i<(int)peaknotes.size(); i++) {
		if ((durations[i] <= 2) && (strongbeat[i] == false)) {
			continue;
		}
		if (peaknotes[i]) {
			newnotelist.push_back(oldnotelist[i]);
		}
	}

}



//////////////////////////////
//
// Tool_peak::identifyLocalPeaks -- Identify notes that are higher than their
//    adjacent neighbors.  The midinumbs are MIDI note numbers (integers)
//    for the pitch, with higher number meaning higher pitches.  Rests are
//    the value 0.  Do not assign a note as a peak note if one of the
//    adjacent notes is a rest. (This could be refined later, such as ignoring
//    short rests).
//

void Tool_peak::identifyLocalPeaks(vector<bool>& peaknotes, vector<int>& midinums) { //changed to midinums from 'notelist'
	for (int i=1; i<(int)midinums.size() - 1; i++) {
		if ((midinums[i - 1] <= 0) || (midinums[i + 1] <= 0)) { //not next to a rest
			continue;
		} else if (midinums[i] <= 0) {
			continue;
		}
		if ((midinums[i] > midinums[i - 1]) && (midinums[i] > midinums[i + 1])) { //check neighboring notes
			peaknotes[i] = 1;
		}
	}
}



//////////////////////////////
//
// Tool_peak::identifyPeakSequence -- Identify a sequence of local peaks that can form
//       a peak sequence
//
// Input variables:
//      globalpeaknotes -- output data that is set to true if the note is part of a
//                         global peak note sequence.  This vector has the same size
//                         as the peakmidinums vector and the values are initially set
//                         to false.  This function will set notes that are part of a sequence
//                         of local peak notes (with length m_peakNum) to true.
//      peakmidinums    -- MIDI number vector for all local peak notes.
//      notes           -- double vector of Humdrum notes (needed to get timestamps).
//
// Member variables that are needed in the Algorithm:
//    int    m_peakNum: Number of consecutive local peaknotes needed to mark global peak.
//    double m_peakDur: Maximum duration of the first/last peak note in global peak sequence.
//
//

void Tool_peak::identifyPeakSequence(vector<bool>& globalpeaknotes, vector<int>& peakmidinums,
		vector<vector<HTp>>& notes) {

	// Set initial positions of globalpeaknotes to false:
	globalpeaknotes.resize(peakmidinums.size());
	fill(globalpeaknotes.begin(), globalpeaknotes.end(), false);
	// The code below (under Algorithms) will set notes identified as a "global peak" to true
	// in this vector.

	// Get the timestamps of all local peak notes:
	vector<double> timestamps(notes.size(), 0.0);
	for (int i=0; i<(int)notes.size(); i++) {
		timestamps[i] = notes[i][0]->getDurationFromStart().getFloat();
	}

	///////////////////////////////////////////
	//
	// Algorithm:
	//
	//    * Loop through each element in the peakmidinums vector checking if it is a part of
	//      a longer peak sequence of the same MIDI note number.  The loop will start at
	//      index 0 and go until but not including peakmidinums.size() - m_peakNum.
	//
	//    * A "global peak" means that there are m_peakNum MIDI note numbers in a row in peakmidinums.
	//      (all of the peak notes have to be the same MIDI note number).
	//
	//    *  In addition, the time difference between the starting and ending note of the peak
	//       sequence has to be equal to or less than m_peakDur (6 whole notes by default).
	//       Subtract the timestamp of the last and first notes and see if the duration between
	//       these notes is equal or less than m_peakDur.
	//
	//////////////////////////////////////////

	for (int i=0; i<(int)peakmidinums.size() - m_peakNum; i++) {
		bool match = true;
		bool synco = isSyncopated(notes[i][0]);
		for (int j=1; j<m_peakNum; j++) {
			synco |= isSyncopated(notes[i+j][0]);
			if (peakmidinums[i+j] != peakmidinums[i+j-1]) {
				match = false;
				//synco |= isSyncopated(notes[i+j][0]);
				break;
			}
		}
		if (!match) {
			continue;
		}
		if ((!m_nsyncoQ) && (!synco)){
			continue;
		}
		if ((m_nsyncoQ) && (synco)) {
			continue;
		}

		HumNum duration = timestamps[i + m_peakNum - 1] - timestamps[i];
		if (duration.getFloat() > m_peakDur) {
			continue;
		}
		//data for every sub-sequeunce
		m_count += 1;
		int line = notes[i][0]->getLineIndex();
		int line2 = notes[i + m_peakNum - 1].back()->getLineIndex();

		m_peakDuration.push_back(duration.getFloat()/4.0);
		m_peakMeasureBegin.push_back(m_barNum[line]);
		m_peakMeasureEnd.push_back(m_barNum[line2]);
		vector<HTp> pnotes;
		for (int j=0; j<m_peakNum; j++) {
			pnotes.push_back(notes.at(i+j).at(0));
		}
		m_peakPitch.push_back(pnotes);
		m_peakPeakCount.push_back((int)pnotes.size());

		// variables to do peak group mergers later:
		int track = notes[i][0]->getTrack();
		m_peakTrack.push_back(track);
		m_peakIndex.push_back(m_peakIndex.size());
		HumNum starttime = notes[i][0]->getDurationFromStart();
		HumNum endtime   = notes[i+m_peakNum-1].back()->getDurationFromStart();
		m_startTime.push_back(starttime);
		m_endTime.push_back(endtime);

		for (int j=0; j<m_peakNum; j++) {
			globalpeaknotes[i+j] = true;
		}
	}
}



//////////////////////////////
//
// Tool_peak::printData -- Print input and output data.  First column is the MIDI note
//      number, second one is the peak analysis (true=local maximum note)
//

void Tool_peak::printData(vector<vector<HTp>>& notelist, vector<int>& midinums, vector<bool>& peaknotes) {
	m_free_text << "MIDI\tPEAK\tKERN" << endl;
	for (int i=0; i<(int)notelist.size(); i++) {
		m_free_text << midinums.at(i) << "\t";
		m_free_text << peaknotes.at(i);
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
// Tool_peak::getMidiNumbers -- convert note tokens into MIDI note numbers.
//    60 = middle C (C4), 62 = D4, 72 = C5, 48 = C3.
//

vector<int> Tool_peak::getMidiNumbers(vector<vector<HTp>>& notelist) {
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
// Tool_peak::getNoteList -- Return a list of the notes and rests for
//     a part, with the input being the starting token of the part from
//     which the note list should be extracted.  The output is a two-
//     dimensional vector.  The first dimension is for the list of notes,
//     and the second dimension is used to store any subsequent tied notes
//     so that they can be marked and highlighted in the score.
//

vector<vector<HTp>> Tool_peak::getNoteList(HTp starting) {
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

void  Tool_peak::getDurations(vector<double>& durations, vector<vector<HTp>>& notelist) {
	durations.resize(notelist.size());
	for (int i=0; i<(int)notelist.size(); i++) {
		HumNum duration = notelist[i][0]->getTiedDuration();
		durations[i] = duration.getFloat();
	}
}



//////////////////////////////
//
// Tool_peak::getBeat --
//

void  Tool_peak::getBeat(vector<bool>& metpos, vector<vector<HTp>>& notelist) {
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
// Tool_peak::getMetricLevel --
//

int  Tool_peak::getMetricLevel(HTp token) {
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
// Tool_peak::isSyncopated --
//

bool  Tool_peak::isSyncopated(HTp token) {
	HumNum dur = token->getTiedDuration();
	double logDur = log2(dur.getFloat());
	int metLev = getMetricLevel(token);
	if (logDur > metLev) {
		return true;
	} else {
		return false;
	}
}



//////////////////////////////
//
// Tool_peak::countNotesInScore --
//

int Tool_peak::countNotesInScore(HumdrumFile& infile) {
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
