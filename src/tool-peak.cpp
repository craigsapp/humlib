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

	// The first "spine" is the lowest part on the system.
	// The last "spine" is the highest part on the system.
	for (int i=0; i<(int)starts.size(); i++) {
		processSpine(starts[i]);
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

	if (!m_infoQ) {
		m_humdrum_text << "!!!peaks: " << m_count << endl;
	}

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
		//markNotesInScore(notelist, peaknotes);
		// Uncomment out the following line, and comment the above line,
		// when identifyPeakSequence() is implemented:
		markNotesInScore(peaknotelist, globalpeaknotes);
	}
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

// 	int previousNote = -1;
// 	int sequenceNum = 0;
// 	int sequenceStart = 0;
//
// 	for (int i=0; i<(int)peakmidinums.size() - m_peakNum; i++) {
// 		if (peakmidinums[i] == previousNote) { //check if same as previous peak
// 			sequenceNum += 1;
// 		} else {
// 			sequenceNum = 0;
// 		}
// 		if (sequenceNum == 1) { //identify beginning of peak sequence
// 			sequenceStart = i - 1;
// 		}
// 		if (sequenceNum >= m_peakNum - 1) {
// 			int duration = timestamps[i] - timestamps[sequenceStart];
// 			if (duration <= m_peakDur) {
// 				for (int j=0; j<m_peakNum; j++) {
// 					globalpeaknotes[sequenceStart + j] = true;
// 				}
// 		  } else {
// 				sequenceStart += 1;
// 		  }
// 	  }
// 		previousNote = peakmidinums[i];
//   }
// }
	   for (int i=0; i<(int)peakmidinums.size() - m_peakNum; i++) {
			 bool match = true;
			 for (int j=1; j<m_peakNum; j++) {
				 if (peakmidinums[i+j] != peakmidinums[i+j-1]) {
					 match = false;
					 break;
				 }
			 }
			 if (match != true) {
			 	continue;
			 }
			 HumNum duration = timestamps[i + m_peakNum - 1] - timestamps[i];
			 cerr << duration.getFloat() << "   " << m_peakDur << endl;
			 if (duration.getFloat() > m_peakDur) {
				 continue;
			 }
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
		// cerr << "DURATION FOR " << notelist[i][0] << " IS " << durations[i] << endl;
	}
}



//////////////////////////////
//
// getBeat --
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
		cerr << "Position FOR " << notelist[i][0] << " IS " << metpos[i] << endl;
	}
}






// END_MERGE

} // end namespace hum
