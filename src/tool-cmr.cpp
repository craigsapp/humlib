//
// Programmer:    Kiana Hu
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Fri Feb 18 22:00:48 PST 2022
// Last Modified: Tue Jul  5 23:31:54 PDT 2022
// Filename:      tool-cmr.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/tool-cmr.cpp
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Analyze Conspicuous Melodic Repetition (CMR).
//
// Algorithm overview:
//
// This tool searches for strong repetitions of notes in polyphonic vocal music
// for the Josquin Research Project.  A conspicuous melodic repetition (CMR) is
// defined as three or more notes within 6 wholes notes between first and last
// note attacks, where at least one of the notes has a melodic leap or is
// syncopated.   Each of the repeated notes must be a local highpoint in the
// melody (the adjacent notes are lower in pitch), although if there is a adjacent
// notes a step away from a potential high note, then the local peak requirement
// is not required if there are no other notes higher (or lower for inverse peaks)
// betwen adjacent repetition notes.
//

#include "tool-cmr.h"
#include "Convert.h"
#include "HumRegex.h"

using namespace std;

namespace hum {

// START_MERGE

double cmr_note_info::m_syncopationWeight = 1.0;
double cmr_note_info::m_leapWeight = 0.5;


///////////////////////////////////////////////////////////////////////////
//
// cmr_note_info -- Helper class describing a conspicuous repetition note.
//    The cmr_group_info contains a vector of cmr_note_info objects.
//

//////////////////////////////
//
// cmr_note_info::cmr_note_info -- Constructor.
//

cmr_note_info::cmr_note_info(void) {
	clear();
}



/////////////////////////////
//
// cmr_note_info::clear -- Clear or initialized the contents.
//

void cmr_note_info::clear(void) {
	m_tokens.clear();
	m_measureBegin   = -1;
	m_measureEnd     = -1;
	m_hasSyncopation = -1;
	m_hasLeapBefore  = -1;
}



/////////////////////////////
//
// cmr_note_info::getMeasureBegin -- Get the measure number that
//    the note starts in.
//

int cmr_note_info::getMeasureBegin(void) {
	return m_measureBegin;
}



/////////////////////////////
//
// cmr_note_info::getMeasureEnd -- Get the measure number that the
//    note ends in (including consideration of the duration of the
//    note or tied groups that follow the notes).
//

int cmr_note_info::getMeasureEnd(void) {
	return m_measureEnd;
}



/////////////////////////////
//
// cmr_note_info::setMeasureBegin -- Get the measure number that
//    the note starts in.
//

void cmr_note_info::setMeasureBegin(int measure) {
	m_measureBegin = measure;
}



/////////////////////////////
//
// cmr_note_info::setMeasureEnd -- Set the measure number that the
//    note ends in (including consideration of the duration of the note
//    or tied group of notes that follow it).
//

void cmr_note_info::setMeasureEnd(int measure) {
	m_measureEnd = measure;
}



/////////////////////////////
//
// cmr_note_info::getStartTime -- Return the starting time of the note
//    in units of quarter notes since the start of the score.
//

HumNum cmr_note_info::getStartTime(void) {
	if (m_tokens.empty()) {
		return -1;
	} else {
		return m_tokens[0]->getDurationFromStart();
	}
}



/////////////////////////////
//
// cmr_note_info::getEndTime -- Return the ending time of the note in
//    units of quarter notes since the start of the score.  Note that this
//    is the ending duration of the last note in the tied group.
//

HumNum cmr_note_info::getEndTime(void) {
	if (m_tokens.empty()) {
		return -1;
	} else {
		HumNum noteDur = m_tokens.back()->getTiedDuration();
		return m_tokens.back()->getDurationFromStart() + noteDur;
	}
}



/////////////////////////////
//
// cmr_note_info::getMidiPitch -- Return the MIDI pitch of the note or -1 if invalid.
//
int cmr_note_info::getMidiPitch(void) {
	if (m_tokens.empty()) {
		return -1;
	} else {
		return m_tokens[0]->getMidiPitch();
	}
}



/////////////////////////////
//
// cmr_note_info::hasSyncopation -- True if the note is syncopated.
//

bool cmr_note_info::hasSyncopation(void) {
	if (m_hasSyncopation >= 0) {
		return (bool)m_hasSyncopation;
	} else if (m_tokens.empty()) {
		m_hasSyncopation = 0;
		return false;
	} else {
		m_hasSyncopation = (int)cmr_note_info::isSyncopated(m_tokens[0]);
		return (bool)m_hasSyncopation;
	}
}



//////////////////////////////
//
// cmr_note_info::isSyncopated -- Return true if the note is syncopated,which
//   means that the note is longer than the metric position that it is one; however,
//   this analysis is chopped off for notes longer than a beat (whole note).
//   Static function.
//

bool cmr_note_info::isSyncopated(HTp token) {
	if (token == NULL) {
		return false;
	}
	HumNum dur = token->getTiedDuration();
	double logDur = log2(dur.getFloat());
	int metlev = cmr_note_info::getMetricLevel(token);
	if (metlev >= 2) { // no syncopations occuring on whole-note level or higher
		return false;
	} else if (logDur > metlev) {
		return true;
	} else {
		return false;
	}
}



/////////////////////////////
//
// cmr_note_info::hasLeapBefore -- True if there is a melodic leap before the note.
//

bool cmr_note_info::hasLeapBefore(void) {
	if (m_hasLeapBefore >= 0) {
		return (bool)m_hasLeapBefore;
	} else if (m_tokens.empty()) {
		m_hasLeapBefore = 0;
		return false;
	} else {
		m_hasLeapBefore = cmr_note_info::isLeapBefore(m_tokens[0]);
		return (bool)m_hasLeapBefore;
	}
}



//////////////////////////////
//
// cmr_note_info::isLeapBefore -- Return true if the note is syncopated, which
//   means that the note is longer than the metric position that it is one; however,
//   this analysis is chopped off for notes longer than a beat (whole note).
//   Static function.
//

bool cmr_note_info::isLeapBefore(HTp token) {
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
			return false;
			// current = current->getPreviousToken();
			// continue;
		}
		int testNote = current->getMidiPitch();
		int interval = startNote - testNote;
		return interval > 2;
	}
	return false;
}



//////////////////////////////
//
// cmr_note_info::getMetricLevel -- Return the metric level of the note:
//    whole note:   2
//    half note:    1
//    quarter note: 0
//    less than quarter note: -1
//

double cmr_note_info::getMetricLevel(HTp token) {
	HumNum beat = token->getDurationFromBarline();
	if (!beat.isInteger()) { // anything less than quarter note level
		return -1;
	} else if (beat.getNumerator() % 4 == 0) { // whole note level
		return 2;
	} else if (beat.getNumerator() % 2 == 0) { // half note level
		return 1;
	} else { // quarter note level
		return 0;
	}
}



/////////////////////////////
//
// cmr_note_info::getNoteStrength -- Calculate a strength value for the
//     note.  This is based on if the note is syncopated and/or if it
//     is preceded by a melodic leap.
//

double cmr_note_info::getNoteStrength(void) {
	double output = 1.0;
	if (hasSyncopation()) {
		output += m_syncopationWeight;
	}
	if (hasLeapBefore()) {
		output += m_leapWeight;
	}
	return output;
}



//////////////////////////////
//
// cmr_note_info::markNote -- Only print the first note in a series
//     of notes with the same pitch.
//

void cmr_note_info::markNote(const string& marker) {
	for (int i=0; i<(int)m_tokens.size(); i++) {
		HTp token = m_tokens[i];
		if (i > 0) {
			// don't color repeated secondary notes.
			if (token->isNoteAttack()) {
				break;
			}
		}
		string text = *token;
		if (text.find(marker) != string::npos) {
			continue;
		}
		text += marker;
		token->setText(text);
	}
}



//////////////////////////////
//
// cmr_note_info::printNote --
//    Default values:
//       output = std::cerr
//       marker = ""
// If marker is not empty, then the marker string will be removed from the
// output.
//

ostream& cmr_note_info::printNote(ostream& output, const string& marker) {
	string newmarker = marker;
	if (marker == "+") {
		newmarker = "\\+";
	} else if (marker == "|") {
		newmarker = "\\|";
	} else if (marker == "?") {
		newmarker = "\\?";
	}

	string contents;
	for (int i=0; i<(int)m_tokens.size(); i++) {
		contents +=  *m_tokens[i];
		if (i < (int)m_tokens.size() - 1) {
			contents += ",";
		}
	}
	contents += "(";
	if (hasSyncopation()) {
		contents += "S";
	}
	if (hasLeapBefore()) {
		contents += "L";
	}
	contents += to_string(m_tokens[0]->getLineNumber());
	contents += ")";
	if (!newmarker.empty()) {
		HumRegex hre;
		hre.replaceDestructive(contents, "", newmarker, "g");
	}
	output << contents;
	return output;
}



//////////////////////////////
//
// cmr_note_info::getPitch -- Return scientific pitch name.
//

string cmr_note_info::getPitch(void) {
	if (m_tokens.empty()) {
		return "R";
	} else {
		int octave = Convert::kernToOctaveNumber(m_tokens.at(0));
		int accidentals = Convert::kernToAccidentalCount(m_tokens.at(0));
		int dpc = Convert::kernToDiatonicPC(m_tokens.at(0));
		string output;
		switch (dpc) {
			case 0: output += "C"; break;
			case 1: output += "D"; break;
			case 2: output += "E"; break;
			case 3: output += "F"; break;
			case 4: output += "G"; break;
			case 5: output += "A"; break;
			case 6: output += "B"; break;
		}
		for (int i=0; i<abs(accidentals); i++) {
			if (accidentals < 0) {
				output += "b";
			} else {
				output += "#";
			}
		}
		output += to_string(octave);
		return output;
	}
}



///////////////////////////////
//
// cmr_note_info::getToken --
//

HTp cmr_note_info::getToken(void) {
	if (m_tokens.empty()) {
		return NULL;
	} else {
		return m_tokens.at(0);
	}
}



///////////////////////////////////////////////////////////////////////////
//
// cmr_group_info -- helper class describing a conspicuous repetition groups.
//    This class is used to store a conspicuous melodic repetition (which will
//    be merged with adjacent CMRs later if they overlap.
//

//////////////////////////////
//
// cmr_group_info::cmr_group_info -- Constructor.
//

cmr_group_info::cmr_group_info(void) {
	clear();
}



//////////////////////////////
//
// cmr_group_info::clear -- Clear or initialize object.
//

void cmr_group_info::clear(void) {
	m_serial    = -1;
	m_direction =  0;
	m_notes.clear();
}



//////////////////////////////
//
// cmr_group_info::getNoteCount -- Return the number of notes in the group.
//

int cmr_group_info::getNoteCount(void) {
	if (m_serial < 0) {
		return 0;
	} else {
		return (int)m_notes.size();
	}
}



//////////////////////////////
//
// cmr_group_info::getGroupDuration -- Return the duration of the group, which
//     is the duration beetween the first and last note attack.
//

HumNum cmr_group_info::getGroupDuration(void) {
	if (m_notes.empty()) {
		return -1;
	} else {
		HumNum startPos = m_notes[0].m_tokens[0]->getDurationFromStart();
		HumNum endPos   = m_notes.back().m_tokens[0]->getDurationFromStart();
		return endPos - startPos;
	}
}



//////////////////////////////
//
// cmr_group_info::addNote -- Add a note to the list. The input is a list of
//     notes tied together.
//

void cmr_group_info::addNote(vector<HTp>& tiednotes, vector<int>& barnums) {
	if (tiednotes.empty()) {
		cerr << "Strange problem in cmr_group_info::addNote" << endl;
	}
	m_notes.resize(m_notes.size() + 1);
	m_notes.back().m_tokens = tiednotes;

	int line = m_notes.back().m_tokens.at(0)->getLineIndex();
	int mstart = barnums.at(line);
	m_notes.back().setMeasureBegin(mstart);

	line = m_notes.back().m_tokens.back()->getLineIndex();
	int mend = barnums.at(line);
	m_notes.back().setMeasureEnd(mend);
}



//////////////////////////////
//
// cmr_group_info::getMeasureBegin --
//

int cmr_group_info::getMeasureBegin(void) {
	if (m_notes.empty()) {
		return -1;
	} else {
		return m_notes[0].getMeasureBegin();
	}
}



//////////////////////////////
//
// cmr_group_info::getMeasureEnd -- Return the starting measure of the last note in the group.
//

int cmr_group_info::getMeasureEnd(void) {
	if (m_notes.empty()) {
		return -1;
	}
	return m_notes.back().getMeasureEnd();
}



//////////////////////////////
//
// cmr_group_info::getStartTime -- Return the starting time of the first
//     note in the CMR in units of quarter notes since the start of the music.
//

HumNum cmr_group_info::getStartTime(void) {
	if (m_notes.empty()) {
		return -1;
	} else {
		return m_notes[0].getStartTime();
	}
}



//////////////////////////////
//
// cmr_group_info::getEndTime -- Return the starting time of the last note
//     in the CMR group in units of quarter notes since the start of the music.
//

HumNum cmr_group_info::getEndTime(void) {
	if (m_notes.empty()) {
		return -1;
	} else {
		return m_notes.back().getStartTime();
	}
}



//////////////////////////////
//
// cmr_group_info::getMidiPitch -- Return the MIDI pitch of the group's note.
//

int cmr_group_info::getMidiPitch(void) {
	if (m_notes.empty()) {
		return -1;
	} else {
		return m_notes[0].getMidiPitch();
	}
}



//////////////////////////////
//
// cmr_group_info::getSerial --
//

int cmr_group_info::getSerial(void) {
	return m_serial;
}



//////////////////////////////
//
// cmr_group_info::setSerial --
//

void cmr_group_info::setSerial(int serial) {
	m_serial = serial;
}



//////////////////////////////
//
// cmr_group_info::getDirection --
//

int cmr_group_info::getDirection(void) {
	return m_direction;
}


//////////////////////////////
//
// cmr_group_info::setDirectionUp --
//

void cmr_group_info::setDirectionUp(void) {
	m_direction = +1;
}



//////////////////////////////
//
// cmr_group_info::setDirectionDown --
//

void cmr_group_info::setDirectionDown(void) {
	m_direction = -1;
}



//////////////////////////////
//
// cmr_group_info::makeInvalid --
//

void cmr_group_info::makeInvalid(void) {
	if (m_serial > 0) {
		m_serial *= -1;
	}
}



//////////////////////////////
//
// cmr_group_info::getTrack --
//

int cmr_group_info::getTrack(void) {
	if (getNoteCount() <= 0) {
		return -1;
	} else {
		HTp token = getNote(0);
		if (token) {
			return token->getTrack();
		} else {
			return -1;
		}
	}
}



//////////////////////////////
//
// cmr_group_info::getStartLineNumber --
//

int cmr_group_info::getStartLineNumber(void) {
	if (m_notes.empty()) {
		return -1;
	} else {
		return m_notes[0].m_tokens[0]->getLineNumber();
	}
}



//////////////////////////////
//
// cmr_group_info::getStartLineNumber --
//

int cmr_group_info::getStartFieldNumber(void) {
	if (m_notes.empty()) {
		return -1;
	} else {
		return m_notes[0].m_tokens[0]->getFieldNumber();
	}
}



//////////////////////////////
//
// cmr_group_info::getGroupStrength -- Return the strength value for the CMR.
//

double cmr_group_info::getGroupStrength(void) {
	double output = 0.0;
	for (int i=0; i<(int)m_notes.size(); i++) {
		output += m_notes[i].getNoteStrength();
	}
	return output;
}



//////////////////////////////
//
// cmr_group_info::getNote -- return the note attack at the given index.
//    Return NULL if out of range.
//

HTp cmr_group_info::getNote(int index) {
	if (index < 0) {
		return NULL;
	} else if (index >= getNoteCount()) {
		return NULL;
	} else {
		return m_notes[index].m_tokens[0];
	}
}



//////////////////////////////
//
// cmr_group_info::getStartNote -- return attack note of first note in group, or
//    return NULL if out of range.
//

HTp cmr_group_info::getFirstToken(void) {
	if (m_notes.empty()) {
		return NULL;
	} else {
		return m_notes[0].getToken();
	}
}



//////////////////////////////
//
// cmr_group_info::isValid -- Group that is not merged
//

bool cmr_group_info::isValid(void) {
	if (m_serial >= 0) {
		return true;
	} else {
		return false;
	}
}



//////////////////////////////
//
// cmr_group_info::markNotes --
//

void cmr_group_info::markNotes(const string& marker) {
	for (int i=0; i<(int)m_notes.size(); i++) {
		m_notes[i].markNote(marker);
	}
}



//////////////////////////////
//
// cmr_group_info::mergeGroup -- Merges overlapping groups into a single
//     larger group and invalidates the second group after merging.
//

bool cmr_group_info::mergeGroup(cmr_group_info& group) {
	if (this == &group) {
		// Same group.
		return false;
	}
	if (!this->isValid()) {
		return false;
	}
	if (!group.isValid()) {
		return false;
	}

	HumNum start1 = this->getStartTime();
	HumNum start2 = group.getStartTime();
	HumNum end1   = this->getEndTime();
	HumNum end2   = group.getEndTime();

	if (start2 > end1) {
		// nothing to merge
		return false;
	}

	int duplicates = 0;

	// Identify duplicate notes shared between groups:
	vector<bool> duplicateNote(group.getNoteCount(), false);
	for (int i=0; i<group.getNoteCount(); i++) {
		for (int j=0; j<(int)getNoteCount(); j++) {
			HTp token1 = getNote(j);
			HTp token2 = group.getNote(i);
			if (duplicateNote[i]) {
				continue;
			}
			if (token1 == token2) {
				duplicateNote[i] = true;
				duplicates++;
			}
		}
	}

	if (duplicates == group.getNoteCount()) {
		// group is a subset of this.
		group.makeInvalid();
		return "true";
	}

	// Copy unshared notes into first group:
	for (int i=0; i<group.getNoteCount(); i++) {
		if (duplicateNote[i]) {
			continue;
		}
		m_notes.push_back(group.m_notes[i]);
	}

	// Deactivate group being merged:
	if (group.getSerial() > 0) {
		group.makeInvalid();
	} else {
		cerr << "Strange problem merging group" << endl;
		return false;
	}

	return true;
}



//////////////////////////////
//
// cmr_group_info::printNotes --
//    Default values:
//       output = std::cerr
//       marker = ""
// If marker is not empty, then the marker string will be removed from the
// output.
//

ostream& cmr_group_info::printNotes(ostream& output, const string& marker) {
	for (int i=0; i<(int)m_notes.size(); i++) {
		m_notes[i].printNote(output, marker);
		if (i < (int)m_notes.size() - 1) {
			output << " ";
		}
	}
	return output;
}


//////////////////////////////
//
// cmr_group_info::getPitch -- Return scientific pitch name.
//

string cmr_group_info::getPitch(void) {
	if (m_notes.empty()) {
		return "R";
	} else {
		return m_notes.at(0).getPitch();
	}
}


///////////////////////////////////////////////////////////////////////////

/////////////////////////////////
//
// Tool_cmr::Tool_cmr -- Constructor: set the recognized options for the tool.
//

Tool_cmr::Tool_cmr(void) {
	define("data|raw|raw-data=b",       "print analysis data");
	define("m|mark-up|marker-up=s:+",   "symbol to mark peak cmr notes");
	define("M|mark-down|marker-down=s:@", "symbol to mark anti-peak cmr notes");
	define("c|color|color-up=s:red",    "color of CMR peak notes");
	define("C|color-down=s:orange",     "color of CMR anti-peak notes");
	define("r|ignore-rest=d:1.0",       "ignore rests smaller than given value (in whole notes)");
	define("n|number=i:3",              "number of high notes in a row");
	define("N|cmr-number=b",            "show enumeration number of CMR above/below starting note");
	define("d|dur|duration=d:6.0",      "maximum duration between cmr note attacks in whole notes");
	define("i|info=b",                  "print cmr info");
	define("p|peaks=b",                 "detect only positive cmrs");
	define("t|troughs=b",               "detect only negative cmrs");
	define("A|not-accented=b",          "counts only cmrs that do not have melodic accentation");
	define("s|syncopation-weight=d:1.0","weight for syncopated notes");
	define("leap|leap-weight=d:0.5",    "weight for leapng notes");
	define("l|local-peaks=b",           "mark local peaks");
	define("L|only-local-peaks=b",      "mark local peaks only");
	define("merge|merged|show-merged=b","print merged groups");
	define("S|summary=b",               "summarize CMRs for multiple inputs");
	define("v|vega=b",                  "output default Vega-lite plot");
	define("V|html=b",                  "output Vega-lite plot with HTML");
	define("w|cmr-count=b",             "output Vega-lite plot for CMR count");
	define("W|strength=b",              "output Vega-lite plot with strength scores");
	define("h|half=b",                  "durations given in half notes (mimims)");
	define("D|debug=b",                 "print debug information");
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
	m_rawQ          = getBoolean("raw-data");
	m_peaksQ        = getBoolean("peaks");
	m_npeaksQ       = getBoolean("troughs");
	m_naccentedQ    = getBoolean("not-accented");
	m_localQ        = getBoolean("local-peaks");
	m_localOnlyQ    = getBoolean("only-local-peaks");
	m_showMergedQ   = getBoolean("show-merged");
	m_summaryQ      = getBoolean("summary");
	m_vegaQ         = getBoolean("vega");
	m_htmlQ         = getBoolean("html");
	m_vegaCountQ    = getBoolean("cmr-count");
	m_vegaStrengthQ = getBoolean("strength");
	m_numberQ       = getBoolean("cmr-number");
	m_debugQ        = getBoolean("debug");
	if (m_localOnlyQ) {
		m_localQ = true;
	}

	m_markerUp     = getString("marker-up");
	m_markerDown   = getString("marker-down");
	m_colorUp      = getString("color-up");
	m_colorDown    = getString("color-down");

	m_smallRest    = getDouble("ignore-rest") * 4.0;  // convert from whole notes to quarter notes
	m_cmrNum       = getInteger("number");
	m_cmrDur       = getInteger("duration") * 4.0;    // convert from whole notes to quarter notes
	m_infoQ        = getBoolean("info");
	m_halfQ        = getBoolean("half");

	cmr_note_info::m_syncopationWeight = getDouble("syncopation-weight");
	cmr_note_info::m_leapWeight        = getDouble("leap-weight");

	m_noteGroups.clear();
}


//////////////////////////////
//
// Tool_cmr::processFile -- Do CMR analysis on score.
//

void Tool_cmr::processFile(HumdrumFile& infile) {
	vector<HTp> starts = infile.getKernSpineStartList();
	m_minPitch.clear();
	m_maxPitch.clear();
	m_minPitch.resize(infile.getMaxTrack() + 1);
	m_maxPitch.resize(infile.getMaxTrack() + 1);

	m_local_count = 0;

	m_barNum = infile.getMeasureNumbers();

	// print information about each note group:
	getPartNames(m_partNames, infile);

	// Analyze CMR for each part, starting with the highest part:
	for (int i=(int)starts.size()-1; i>=0; i--) {
		getNoteList(m_notelist, starts.at(i));
		getVocalRange(m_minPitch, m_maxPitch, m_notelist);
		if (m_peaksQ) {
			processSpine(starts[i]);
		} else if (m_npeaksQ) {
			processSpineFlipped(starts[i]);
		} else {
			processSpine(starts[i]);
			processSpineFlipped(starts[i]);
		}
	}

	mergeOverlappingPeaks();

	if (m_vegaQ || m_htmlQ || m_vegaStrengthQ || m_vegaCountQ) {
		m_free_text << " " << endl;
	}

	if (!(m_rawQ || m_summaryQ || m_vegaQ || m_htmlQ || m_vegaStrengthQ || m_vegaCountQ)) {
		markNotesInScore();
	}

	if (!(m_rawQ || m_summaryQ || m_vegaQ || m_htmlQ || m_vegaStrengthQ || m_vegaCountQ)) {
		if (m_numberQ) {
			addGroupNumbersToScore(infile);
		}
		infile.createLinesFromTokens();
		m_humdrum_text << infile;

		if (!m_localOnlyQ) {

			if (hasGroupUp()) {
				m_humdrum_text << "!!!RDF**kern: ";
				m_humdrum_text << m_markerUp;
				m_humdrum_text << " = marked note (up), color=";
				m_humdrum_text << m_colorUp;
				m_humdrum_text << endl;
			}

			if (hasGroupDown()) {
				m_humdrum_text << "!!!RDF**kern: ";
				m_humdrum_text << m_markerDown;
				m_humdrum_text << " = marked note (down), color=";
				m_humdrum_text << m_colorDown;
				m_humdrum_text << endl;
			}

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
		if (m_summaryQ) {
			printSummaryStatistics(infile);
		} else if (m_vegaQ || m_htmlQ || m_vegaCountQ || m_vegaStrengthQ) {
			storeVegaData(infile);
		} else {
			printStatistics(infile);
		}
	}

	if (m_infoQ && !m_summaryQ) {
		prepareHtmlReport();
	}

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
	if (m_notelist.empty()) {
		m_midinums.clear();
		m_metlevs.clear();
		m_syncopation.clear();
		m_leapbefore.clear();
		return;
	}

	m_track = m_notelist.at(0).at(0)->getTrack();
	getMidiNumbers(m_midinums, m_notelist);
	identifyLocalPeaks(m_localpeaks, m_midinums);
	getMetlev(m_metlevs, m_notelist);
	getSyncopation(m_syncopation, m_notelist);
	getLeapBefore(m_leapbefore, m_midinums);

	if (m_localQ) {
		markNotes(m_notelist, m_localpeaks, m_local_marker);
	}
	if (m_localOnlyQ) {
		return;
	}

	for (int i=0; i<(int)m_notelist.size(); i++) {
		checkForCmr(i, 1);
	}

	if (m_rawQ) {
		printAnalysisData();
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
	if (m_notelist.empty()) {
		m_midinums.clear();
		m_metlevs.clear();
		m_syncopation.clear();
		m_leapbefore.clear();
		return;
	}

	m_track = m_notelist.at(0).at(0)->getTrack();
	getMidiNumbers(m_midinums, m_notelist);
	flipMidiNumbers(m_midinums);
	identifyLocalPeaks(m_localpeaks, m_midinums);
	getMetlev(m_metlevs, m_notelist);
	getSyncopation(m_syncopation, m_notelist);
	getLeapBefore(m_leapbefore, m_midinums);


	if (m_rawQ) {
		printAnalysisData();
	}

	if (m_localQ) {
		markNotes(m_notelist, m_localpeaks, m_local_marker_n);
	}
	if (m_localOnlyQ) {
		return;
	}

	for (int i=0; i<(int)m_notelist.size(); i++) {
		checkForCmr(i, -1);
	}
}



//////////////////////////////
//
// checkForCmr -- store CMR if identified.
//

void Tool_cmr::checkForCmr(int index, int direction) {
	// Local peak must be present to trigger CMR
	if (!m_localpeaks.at(index)) {
		return;
	}

	// The local peak note must have a leap before it (or a rest)
	// and/or a syncopation to trigger a CMR search:
	if (!(m_syncopation.at(index) || m_leapbefore.at(index))) {
		return;
	}

	int pitch = m_midinums.at(index);

	// Create list of notes with same pitch within target duration after target note.
	vector<int> candidates;
	candidates.push_back(index);

	int i = index+1;
	HumNum duration = 0;
	if (i < (int)m_notelist.size()) {
		duration = m_notelist.at(i).at(0)->getDurationFromStart() - m_notelist.at(index).at(0)->getDurationFromStart();
	}

	// Check for matching peaks after target note:
	while ((i < (int)m_notelist.size()) && (duration.getFloat() <= m_cmrDur)) {
		if (m_midinums.at(i) > pitch + 2) {
			// Cannot exceed a major second above peak note
			// maybe check if adjacent to a peak note
			break;
		}
		if (m_midinums.at(i) == pitch) {
			double metlev = m_metlevs.at(i);
			if (metlev > 1) { // has to be on whole-note level (metlev == 2)
				candidates.push_back(i);
			} else if (isMelodicallyAccented(i)) {
				candidates.push_back(i);
			}
		}
		i++;
		if (i < (int)m_notelist.size()) {
			duration = m_notelist.at(i).at(0)->getDurationFromStart() - m_notelist.at(index).at(0)->getDurationFromStart();
		}
	}

	i = index-1;
	duration = 0;
	if (i < (int)m_notelist.size()) {
		duration = m_notelist.at(index).at(0)->getDurationFromStart() - m_notelist.at(i).at(0)->getDurationFromStart();
	}

	// Check for matching peaks before target note:
	while ((i >= 0) && (duration.getFloat() <= m_cmrDur)) {
		if (m_midinums.at(i) > pitch + 2) {
			// Cannot exceed a major second above peak note
			// maybe check if adjacent to a peak note
			break;
		}
		if (m_midinums.at(i) == pitch) {
			double metlev = m_metlevs.at(i);
			if (metlev > 1) { // has to be on whole-note level (metlev == 2)
				candidates.insert(candidates.begin(), i);
			} else if (isMelodicallyAccented(i)) {
				candidates.insert(candidates.begin(), i);
			}
		}
		i--;
		if (i >= 0) {
			duration = m_notelist.at(index).at(0)->getDurationFromStart() - m_notelist.at(i).at(0)->getDurationFromStart();
		}
	}

	if ((int)candidates.size() < m_cmrNum) {
		// Not enough note to consider a CMR.
		return;
	}

	for (int i=0; i<=(int)candidates.size() - m_cmrNum; i++) {
		int index1 = candidates.at(i);
		int index2 = candidates.at(i+m_cmrNum-1);
		HumNum dur1 = m_notelist.at(index1).at(0)->getDurationFromStart();
		HumNum dur2 = m_notelist.at(index2).at(0)->getDurationFromStart();
		HumNum duration = dur2 - dur1;
		if (duration > m_cmrDur) {
			continue;
		}
		if (hasHigher(pitch, 2, m_midinums, index1, index2)) {
			continue;
		}


		// found a CMR (or piece of longer one that will be merged later)
		// so store it at the end of m_noteGroups:
		m_noteGroups.resize(m_noteGroups.size() + 1);
		for (int j=0; j<m_cmrNum; j++) {
			int tindex = candidates.at(i+j);
			m_noteGroups.back().addNote(m_notelist.at(tindex), m_barNum);
		}
		m_noteGroups.back().setSerial((int)m_noteGroups.size() + 1);
		if (direction < 0) {
			m_noteGroups.back().setDirectionDown();
		} else {
			m_noteGroups.back().setDirectionUp();
		}
	}
}



//////////////////////////////
//
// Tool_cmr::hasHigher --
//

bool Tool_cmr::hasHigher(int pitch, int tolerance, vector<int> midinums, int index1, int index2) {
	for (int i=index1; i<=index2; i++) {
		if (midinums.at(i) > pitch + tolerance) {
			return true;
		}
	}
	return false;
}


//////////////////////////////
//
// Tool_cmr::hasGroupUp -- True if there is at least one CMR that contains a peak note.
//

bool Tool_cmr::hasGroupUp(void) {
	for (int i=0; i<(int)m_noteGroups.size(); i++) {
		if (m_noteGroups.at(i).isValid()) {
			if (m_noteGroups.at(i).getDirection() == +1) {
				return true;
			}
		}
	}
	return false;
}



//////////////////////////////
//
// Tool_cmr::hasGroupDown -- True if there is at least one CMR that contains an antipeak note.
//

bool Tool_cmr::hasGroupDown(void) {
	for (int i=0; i<(int)m_noteGroups.size(); i++) {
		if (m_noteGroups.at(i).isValid()) {
			if (m_noteGroups.at(i).getDirection() == -1) {
				return true;
			}
		}
	}
	return false;
}

//////////////////////////////
//
// [fill in about new function]
//

string Tool_cmr::getComposer(HumdrumFile& infile) {
   // Options options;
   // options.process(argc, argv);
	 //
   // HumdrumFileStream instream(options);
   // HumdrumFile infile;
   HumRegex hre;

	 string filename = infile.getFilename();
	 // Remove any directory prefix:
	 string code = "unknown";
	 // Search for a capital letter followed by two small letters
	 // followed by four digits:
	 if (hre.search(filename, "([A-Z][a-z][a-z])")) {
			// Store the first match contents (stuff in parenthese on above line):
			code = hre.getMatch(1);
	 }
	return code;
}

//////////////////////////////
//
// Tool_cmr::printSummaryStatistics --
//

void Tool_cmr::printSummaryStatistics(HumdrumFile& infile) {

	m_free_text << getGroupCount() << "	" << ((double)getGroupNoteCount() / countNotesInScore(infile)) * 1000.0 << "	" << infile.getFilename() << endl;

	m_cmrCount.push_back(getGroupCount());
	m_cmrNoteCount.push_back(getGroupNoteCount());
	m_scoreNoteCount.push_back(countNotesInScore(infile));

	// store the results in these two variables for later averaging and SD (defined in tool-cmr.h)
	//	std::vector<int>      m_cmrCount;       // number of CMRs in each input file
	//	std::vector<int>      m_cmrNoteCount;   // number of CMRs in each input file
	//	std::vector<int>      m_scoreNoteCount;   // number of notes in each input file
}

//////////////////////////////
//
// Tool_cmr::storeVegaData --
//

void Tool_cmr::storeVegaData(HumdrumFile& infile) {

	string composer = getComposer(infile);
	m_vegaData << "{" << endl;
	m_vegaData << "	\"Composers\": " << '"' << composer << '"' << "," << endl;
	m_vegaData << "	\"Score\": \"" << infile.getFilename() << "\"," << endl;
	m_vegaData << "	\"CMR note density permil\": " << ((double)getGroupNoteCount() / countNotesInScore(infile)) * 1000 << "," << endl;
	if (getGroupCount() == 0) {
		m_vegaData << "	\"CMR strength density\": " << "0" << "," << endl;
	} else {
		m_vegaData << "	\"CMR strength density\": " << ((double)getStrengthScore() / getGroupCount()) << "," << endl;
	}
	m_vegaData << "	\"CMR count\": " << getGroupCount() << endl;
	m_vegaData << "}," << endl;

}

////////////////////////////
//
// Tool_cmr::printVegaPlot --
//

void Tool_cmr::printVegaPlot(void) {
	string vegaDataHeader = R"(
		{
		  "$schema": "https://vega.github.io/schema/vega-lite/v5.json",
		  "description": "A vertical box plot showing median, min, and max CMR count in Josquin.",
		  "data": {
		    "values": [)";
		cout << vegaDataHeader << endl;

		cout << m_vegaData.str() << endl;

		if (m_vegaCountQ) {
			string vegaDataFooter = R"(
			]},
			 "mark": {
				 "type": "boxplot",
				 "extent": "min-max"
			 },
			 "encoding": {
				 "x": {"field": "Composers", "type": "nominal"},
				 "color": {"field": "Composers", "type": "nominal", "legend": null},
				 "y": {
					 "field": "CMR count",
					 "type": "quantitative",
					 "scale": {"zero": false}
				 }
			 }
		 })";

		 cout << vegaDataFooter << endl;
	 } else	if (m_vegaStrengthQ) {
	 			string vegaDataFooter = R"(
	 			]},
	 			 "mark": {
	 				 "type": "boxplot",
	 				 "extent": "min-max"
	 			 },
	 			 "encoding": {
	 				 "x": {"field": "Composers", "type": "nominal"},
	 				 "color": {"field": "Composers", "type": "nominal", "legend": null},
	 				 "y": {
	 					 "field": "CMR strength density",
	 					 "type": "quantitative",
	 					 "scale": {"zero": false}
	 				 }
	 			 }
	 		 })";

		cout << vegaDataFooter << endl;
	 	} else {
			string vegaDataFooter = R"(
		 ]},
			"mark": {
				"type": "boxplot",
				"extent": "min-max"
			},
			"encoding": {
				"x": {"field": "Composers", "type": "nominal"},
				"color": {"field": "Composers", "type": "nominal", "legend": null},
				"y": {
					"field": "CMR note density permil",
					"type": "quantitative",
					"scale": {"zero": false}
				}
			}
		})";
			cout << vegaDataFooter << endl;
	 }
}

////////////////////////////
//
// Tool_cmr::printHtmlPlot --
//

void Tool_cmr::printHtmlPlot(void) {

	string header = R"(<!DOCTYPE html>
<html>
<head>
<title>Sortcount output</title>
<script src="https://cdn.jsdelivr.net/npm/vega@4.4.0"></script>
<script src="https://cdn.jsdelivr.net/npm/vega-lite@3.0.0-rc12"></script>
<script src="https://cdn.jsdelivr.net/npm/vega-embed@3.29.1"></script>
</head>
<body>
<div id="plotarea"></div>
<script type="text/javascript">
var mydata =)";
	cout << header << endl;

	printVegaPlot();

	string footer = R"(vegaEmbed('#plotarea', mydata);
</script>
</body>
</html>)";
	cout << footer << endl;
}


//////////////////////////////
//
// Tool_cmr::finally --
//

void Tool_cmr::finally(void) {
	//cerr << "\nFUNCTION RUN AFTER ALL INPUT FILES HAVE BEEN PROCESSED" << endl;

	if (m_vegaQ || m_vegaCountQ || m_vegaStrengthQ) {
		printVegaPlot();
	} else if (m_htmlQ) {
		printHtmlPlot();
	} else {
		double meanCmrCount = Convert::mean(m_cmrCount);
		double stdDevCmrCount = Convert::standardDeviation(m_cmrCount);

		vector<double> cmrNoteDensities(m_cmrNoteCount.size());
		for (int i=0; i < (int)cmrNoteDensities.size(); i++) {
			cmrNoteDensities[i] = (double)m_cmrNoteCount[i] / m_scoreNoteCount[i];
		}

		double meanCmrNoteDen = Convert::mean(cmrNoteDensities);
		double stdDevCmrNoteDen = Convert::standardDeviation(cmrNoteDensities);

		cout << "CMR count mean: " << meanCmrCount << endl;
		cout << "CMR count standard deviation: " << stdDevCmrCount << endl;
		cout << "CMR note density mean: " << meanCmrNoteDen * 1000 << " permil " << endl;
		cout << "CMR note density standard deviation: " << stdDevCmrNoteDen * 1000 << " permil " << endl;
	}

}


//////////////////////////////
//
// Tool_cmr::printStatistics --
//

void Tool_cmr::printStatistics(HumdrumFile& infile) {
	int all_note_count = countNotesInScore(infile);

	m_humdrum_text << "!!!!!!!!!! CMR INFO !!!!!!!!!!!!!!!!!!!!!!!!!!!!!"  << endl;

	m_humdrum_text << "!!!cmr_groups: "  << getGroupCount()     << endl;
	m_humdrum_text << "!!!cmr_notes: "   << getGroupNoteCount() << endl;
	m_humdrum_text << "!!!score_notes: " << all_note_count      << endl;

	double groupDensity = ((double)getGroupCount() / all_note_count);
	double groupNoteDensity = ((double)getGroupNoteCount() / all_note_count);

	m_humdrum_text << "!!!cmr_group_density: " << groupDensity * 1000.0 << " permil" << endl;
	m_humdrum_text << "!!!cmr_note_density: "  << groupNoteDensity * 1000.0 << " permil" << endl;

	printGroupStatistics(infile);

	m_humdrum_text << "!!!!!!!!!! END CMR INFO !!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
}



/////////////////////////////
//
// printGroupStatistics -- Print information about individual CMR groups.
//

void Tool_cmr::printGroupStatistics(HumdrumFile& infile) {
	int pcounter = 1;
	int ncounter = -1;
	for (int i=0; i<(int)m_noteGroups.size(); i++) {
		if (!m_showMergedQ) {
			if (!m_noteGroups[i].isValid()) {
				continue;
			}
		}
		double groupDuration = m_noteGroups[i].getGroupDuration().getFloat() / 4.0;
		if (m_halfQ) {
			groupDuration *= 2;
		}
		int track = m_noteGroups[i].getTrack();

		m_humdrum_text << "!!!!!!!!!! CMR GROUP INFO !!!!!!!!!!!!!!!!!!!!!!!"               << endl;
		if (m_noteGroups.at(i).isValid()) {
			m_humdrum_text << "!!!cmr_group_num: "  << pcounter++                            << endl;
		} else {
			m_humdrum_text << "!!!cmr_merge_num: "  << ncounter--                            << endl;
		}
		if (track > 0) {
			m_humdrum_text << "!!!cmr_track: "      << track                                 << endl;
			m_humdrum_text << "!!!cmr_part: "       << m_partNames.at(track)                 << endl;
			m_humdrum_text << "!!!voice_pitch_min: "<< m_minPitch.at(track)                  << endl;
			m_humdrum_text << "!!!voice_pitch_max: "<< m_maxPitch.at(track)                  << endl;
		}
		m_humdrum_text << "!!!cmr_start_line: "    << m_noteGroups[i].getStartLineNumber()  << endl;
		m_humdrum_text << "!!!cmr_start_field: "   << m_noteGroups[i].getStartFieldNumber() << endl;
		m_humdrum_text << "!!!cmr_start_measure: " << m_noteGroups[i].getMeasureBegin()     << endl;
		m_humdrum_text << "!!!cmr_end_measure: "   << m_noteGroups[i].getMeasureEnd()       << endl;
		// Durations are in units of whole notes (semibreves):
		m_humdrum_text << "!!!cmr_duration: "      << groupDuration << m_durUnit            << endl;
		m_humdrum_text << "!!!cmr_strength: "      << m_noteGroups[i].getGroupStrength()    << endl;
		m_humdrum_text << "!!!cmr_direction: "     << m_noteGroups[i].getDirection()        << endl;
		m_humdrum_text << "!!!cmr_note_count: "    << m_noteGroups[i].getNoteCount()        << endl;
		m_humdrum_text << "!!!cmr_pitch: "         << m_noteGroups[i].getPitch()            << endl;
		m_humdrum_text << "!!!cmr_pitches: ";
		if (m_noteGroups.at(i).getDirection() == -1) {
			m_noteGroups.at(i).printNotes(m_humdrum_text, m_markerDown);
		} else {
			m_noteGroups.at(i).printNotes(m_humdrum_text, m_markerUp);
		}
		m_humdrum_text << endl;
	}
}



//////////////////////////////
//
// Tool_cmr::getPartNames -- partNames is indexed by track.
//

void Tool_cmr::getPartNames(vector<string>& partNames, HumdrumFile& infile) {
	partNames.clear();
	partNames.resize(infile.getMaxTrack() + 1);

	for (int i=0; i<infile.getLineCount(); i++) {
		if (infile[i].isData()) {
			break;
		}
		if (!infile[i].isInterpretation()) {
			continue;
		}
		for (int j=0; j<infile[i].getFieldCount(); j++) {
			HTp token = infile.token(i, j);
			int track = token->getTrack();
			if (!partNames[track].empty()) {
				continue;
			}
			if (token->isInstrumentAbbreviation()) {
				if (token->size() > 3) {
					partNames[track] = token->substr(3);
				}
			}
		}
	}

	for (int i=0; i<infile.getLineCount(); i++) {
		if (infile[i].isData()) {
			break;
		}
		if (!infile[i].isInterpretation()) {
			continue;
		}
		for (int j=0; j<infile[i].getFieldCount(); j++) {
			HTp token = infile.token(i, j);
			int track = token->getTrack();
			if (!partNames[track].empty()) {
				continue;
			}
			if (token->isInstrumentName()) {
				if (token->size() > 3) {
					partNames[track] = token->substr(3);
				}
			}
		}
	}

}



//////////////////////////////
//
// Tool_cmr::prepareHtmlReport -- Create HTML template for displaying CMR analysis data.
//

void Tool_cmr::prepareHtmlReport(void) {
	string htmlReport = R"(!!@@BEGIN: PREHTML
!!@CONTENT:
!!<h1 class='cmr'>Conspicuous Melodic Repetition</h1>
!!<table class='gcmr'>
!!   <tr><td>Group density</td><td>@{cmr_group_density}</td></tr>
!!   <tr><td>Group note density</td><td>@{cmr_note_density}</td></tr>
!!</table>
!! <br/>
!! @{printTable:}
!!<style>
!!   h1.cmr { font-size: 24px; }
!!   table.cmr tr:not(:first-child):hover { background: #ff000011; }
!!   table.cmr { max-width: 500px; }
!!   table.gcmr td:nth-child(2) { width:100%; }
!!   table.gcmr td:first-child {white-space: pre; text-align: right; padding-right: 10px; font-weight: bold; }
!!   table.gcmr td:first-child::after { content: ':'; }
!!</style>
!!@JAVASCRIPT:
!!function printTable(refs1, refs2, language) {
!!   let numbers = refs2.cmr_group_num;
!!   let durations = refs2.cmr_duration;
!!   let pitches = refs2.cmr_pitch;
!!   let minpitch = refs2.voice_pitch_min;
!!   let maxpitch = refs2.voice_pitch_max;
!!   let strengths = refs2.cmr_strength;
!!   let count = refs2.cmr_note_count;
!!   let directions = refs2.cmr_direction;
!!   let parts = refs2.cmr_part;
!!   let smeasure = refs2.cmr_start_measure;
!!   let emeasure = refs2.cmr_end_measure;
!!   let output = '';
!!   output += `<table class='cmr'>`;
!!   output += '<tr>';
!!   output += '<th>CMR</th>';
!!   output += '<th>Notes</th>';
!!   output += '<th>Pitch</th>';
!!   output += '<th>Range</th>';
!!   output += '<th>Duration</th>';
!!   output += '<th>Strength</th>';
!!   output += '<th>Measure(s)</th>';
!!   output += '</tr>';
!!   for (let i=0; i<numbers.length; i++) {
!!      let direction = parseInt(directions[i].value) == 1 ? "+" : "-";;
!!      output += '<tr>';
!!      output += `<td>${numbers[i].value}</td>`;
!!      output += `<td>${count[i].value}</td>`;
!!      output += `<td>${direction}${pitches[i].value}</td>`;
!!      output += `<td>${minpitch[i].value}&ndash;${maxpitch[i].value}</td>`;
!!      output += `<td>${durations[i].value}</td>`;
!!      output += `<td>${strengths[i].value}</td>`;
!!      let location = '';
!!      let part = parts[i].value;
!!      let startm = parseInt(smeasure[i].value);
!!      let endm   = parseInt(emeasure[i].value);
!!      if (startm !== endm) {
!!         location = `${startm}&ndash;${endm}`;
!!      } else {
!!         location = `${startm}`;
!!      }
!!      if (part) {
!!         location = `${part}: ${location}`;
!!      }
!!      output += `<td>${location}</td>`;
!!      output += '</tr>';
!!   }
!!   output += '</table>';
!!   return output;
!!}
!!@@END: PREHTML)";

	m_humdrum_text << htmlReport << endl;
}



//////////////////////////////
//
// Tool_cmr::mergeOverlappingPeaks -- Merge overlapping cmr groups.
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
	// merged (that is what the for-loop is for, which
	// keeps merging until there are no mergers left).

	for (int k=0; k<100; k++) {
		bool mergers = false;
		for (int i=0; i<(int)m_noteGroups.size(); i++) {
			for (int j=i+1; j<(int)m_noteGroups.size(); j++) {
				mergers |= checkGroupPairForMerger(m_noteGroups.at(i), m_noteGroups.at(j));
			}
		}
		if (!mergers) {
			break;
		}
	}

	adjustGroupSerials();
}



//////////////////////////////
//
// adjustGroupSerials -- Renumber groups after mergers.
//

void Tool_cmr::adjustGroupSerials(void) {
	int pcounter = 1;
	int ncounter = -1;
	for (int i=0; i<(int)m_noteGroups.size(); i++) {
		if (m_noteGroups.at(i).getSerial() > 0) {
			m_noteGroups.at(i).setSerial(pcounter++);
		} else {
			m_noteGroups.at(i).setSerial(ncounter--);
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

bool Tool_cmr::checkGroupPairForMerger(cmr_group_info& group1, cmr_group_info& group2) {
	if (!group1.isValid()) {
		return false;
	}
	if (!group2.isValid()) {
		return false;
	}
	if (&group1 == &group2) {
		// Same group.
		return false;
	}

	// Groups must have the same track number (i.e., the same staff/part):
	if (group1.getTrack() != group2.getTrack()) {
		return false;
	}

	// Groups must have the same MIDI pitch:
	int midi1 = group1.getMidiPitch();
	int midi2 = group1.getMidiPitch();
	if (midi1 != midi2) {
		return false;
	}

	// Check if they overlap:
	HumNum start1 = group1.getStartTime();
	HumNum start2 = group2.getStartTime();
	// HumNum end1   = group1.getEndTime();
	// HumNum end2   = group2.getEndTime();

	if (start1 == start2) {
		if (group1.getNoteCount() > group2.getNoteCount()) {
			return group1.mergeGroup(group2);
		} else {
			return group2.mergeGroup(group1);
		}
	} else if (start1 < start2) {
		return group1.mergeGroup(group2);
	} else {
		return group2.mergeGroup(group1);
	}
}



//////////////////////////////
//
// Tool_cmr::printAnalysisData -- For debugging.
//

void Tool_cmr::printAnalysisData(void) {
	string partname = m_partNames.at(m_track);
	cerr << "NOTELIST FOR " << partname << " ===================================" << endl;
	cerr << "BAR\tMIDI\tLPEAK\tMETLEV\tSYNC\tLEAP\tNOTES\n";
	for (int i=0; i<(int)m_notelist.size(); i++) {
		cerr << m_barNum.at(m_notelist.at(i).at(0)->getLineIndex());
		cerr << "\t";
		cerr << m_midinums.at(i);
		cerr << "\t";
		cerr << m_localpeaks.at(i);
		cerr << "\t";
		cerr << m_metlevs.at(i);
		cerr << "\t";
		cerr << m_syncopation.at(i);
		cerr << "\t";
		cerr << m_leapbefore.at(i);
		cerr << "\t";
		for (int j=0; j<(int)m_notelist.at(i).size(); j++) {
			cerr << m_notelist.at(i).at(j) << " ";
		}
		cerr << endl;
	}
	cerr << "==================================================" << endl;
}



//////////////////////////////
//
// Tool_cmr::markNotes -- mark notes in list that are true
//     with given marker.
//

void Tool_cmr::markNotes(vector<vector<HTp>>& notelist, vector<bool> localpeaks, const string& marker) {
	bool negative = false;
	if (marker == m_local_marker_n) {
		negative = true;
	}
	for (int i=0; i<(int)localpeaks.size(); i++) {
		if (!localpeaks[i]) {
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
// Tool_cmr::flipMidiNumbers -- Invert midi numbers to search for minima rather than maxima.
//

void Tool_cmr::flipMidiNumbers(vector<int>& midinums) {
	for (int i=0; i<(int)midinums.size(); i++) {
		if (midinums[i] == 0) {
			continue;
		}
		int flippedMidiNum = (midinums[i] * -1) + 128;
		midinums[i] = flippedMidiNum;
	}
}



//////////////////////////////
//
// Tool_cmr::markNotesInScore -- Mark all valid CMR notes.
//

void Tool_cmr::markNotesInScore(void) {
	for (int i=0; i<(int)m_noteGroups.size(); i++) {
		if (m_noteGroups.at(i).isValid()) {
			if (m_noteGroups.at(i).getDirection() == -1) {
				m_noteGroups.at(i).markNotes(m_markerDown);
			} else {
				m_noteGroups.at(i).markNotes(m_markerUp);
			}
		}
	}
}



//////////////////////////////
//
// Tool_cmr::getLocalPeakNotes -- Throw away notes/rests that are not cmrs.
//

void Tool_cmr::getLocalPeakNotes(vector<vector<HTp>>& newnotelist,
		vector<vector<HTp>>& oldnotelist, vector<bool>& localpeaks) {

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
	for (int i=0; i<(int)localpeaks.size(); i++) {
		if ((durations[i] <= 2) && (strongbeat[i] == false)) {
			continue;
		}
		if (localpeaks[i]) {
			newnotelist.push_back(oldnotelist[i]);
		}
	}

}



//////////////////////////////
//
// Tool_cmr::identifyLocalPeaks -- Identify notes that are higher than their
//    adjacent neighbors.  The midinums are MIDI note numbers (integers)
//    for the pitch, with higher number meaning higher pitches.  Rests are
//    the value 0.  Do not assign a note as a cmr note if one of the
//    adjacent notes is a rest. (This could be refined later, such as ignoring
//    short rests).
//

void Tool_cmr::identifyLocalPeaks(vector<bool>& localpeaks, vector<int>& midinums) {
	localpeaks.resize(midinums.size());
	fill(localpeaks.begin(), localpeaks.end(), false);

	if (midinums.size() < 3) {
		// Avoid silly cases.
		return;
	}

	for (int i=0; i<(int)midinums.size() - 1; i++) {
		if ((i > 0) && (midinums.at(i-1) <= 0) && (midinums.at(i+1) <= 0)) {
			// Ignore notes that have rests on both sides.
			continue;
		} else if (midinums.at(i) <= 0) {
			// Ignore rests.
			continue;
		}
		if ((i == 0) && (midinums.at(i) > 0) && (midinums.at(i) > midinums.at(i+1))) {
			// Allow for peak at start of music
			localpeaks.at(i) = true;
		} else if ((i == (int)midinums.size() - 1) && (midinums.back() > 0) && (midinums.back() > midinums.at((int)midinums.size() - 2))) {
			// Allow for peak at end of music
			localpeaks.at(i) = true;
		} else if ((i > 0) && (midinums.at(i) > midinums.at(i-1)) && (midinums.at(i+1) == 0)) {
			// Allow rest after peak note.
			localpeaks.at(i) = true;
		} else if ((i > 0) && (midinums.at(i-1) == 0) && (midinums.at(i) > midinums.at(i+1))) {
			// Allow rest before peak note.
			localpeaks.at(i) = true;
		} else if ((i > 0) && (midinums.at(i) > midinums.at(i-1)) && (midinums.at(i) > midinums.at(i+1))) {
			// Check neighboring notes.
			localpeaks.at(i) = true;
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

	// Get the timestamps of all local peak notes:
	vector<double> timestamps(notes.size(), 0.0);
	for (int i=0; i<(int)notes.size(); i++) {
		timestamps[i] = notes[i][0]->getDurationFromStart().getFloat();
	}

	// The code below (under Algorithms) will set notes identified as a "global cmr" to true
	// in this vector.

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
		bool accented = isMelodicallyAccented(i);
		for (int j=1; j<m_cmrNum; j++) {
			accented |= isMelodicallyAccented(i+j);
			if (cmrmidinums.at(i+j) != cmrmidinums.at(i+j-1)) {
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

		HumNum duration = timestamps.at(i + m_cmrNum - 1) - timestamps.at(i);
		if (duration.getFloat() > m_cmrDur) {
			continue;
		}

		// Store the (potential) CMR:
		m_noteGroups.resize(m_noteGroups.size() + 1);
		for (int j=0; j<m_cmrNum; j++) {
			m_noteGroups.back().addNote(notes.at(i+j), m_barNum);
		}

		// variables to do cmr group mergers later:
		m_noteGroups.back().setSerial(i+1);

		for (int j=0; j<m_cmrNum; j++) {
			globalcmrnotes[i+j] = true;
		}
	}
}



//////////////////////////////
//
// Tool_cmr::getMidiNumbers -- convert note tokens into MIDI note numbers.
//    60 = middle C (C4), 62 = D4, 72 = C5, 48 = C3.
//

void Tool_cmr::getMidiNumbers(vector<int>& midinums, vector<vector<HTp>>& notelist) {

	midinums.resize(notelist.size());

	fill(midinums.begin(), midinums.end(), 0); // fill with rests by default
	for (int i=0; i<(int)notelist.size(); i++) {
		midinums.at(i) = Convert::kernToMidiNoteNumber(notelist.at(i).at(0));
		if (midinums.at(i) < 0) {
			// Set rests to be 0
			midinums.at(i) = 0;
		}
	}
}



//////////////////////////////
//
// Tool_cmr::getSyncopation -- Identify if notes are syncopated or not.
//

void Tool_cmr::getSyncopation(std::vector<bool>& synco, std::vector<std::vector<HTp>>& notelist) {
	synco.resize(notelist.size());
	for (int i=0; i<(int)synco.size(); i++) {
		synco.at(i) = isSyncopated(notelist.at(i).at(0));
	}
}



//////////////////////////////
//
// Tool_cmr::getLeapBefore -- Identify if notes have melodic leaps up before them.
//

void Tool_cmr::getLeapBefore(std::vector<bool>& leap, std::vector<int>& midinums) {
	leap.resize(midinums.size());
	fill(leap.begin(), leap.end(), false);
	for (int i=1; i<(int)leap.size(); i++) {
		int note1 = midinums.at(i);
		if (note1 <= 0) {
			continue;
		}
		int note2 = midinums.at(i-1);
		// Consider duration of rests here.
		if ((note2 <= 0) && (i >= 2)) {
			note2 = midinums.at(i-2);
		}
		if (note2 <= 0) {
			continue;
		}
		int interval = note1 - note2;
		if (interval > 2) {
			leap.at(i) = true;
		}
	}
}



//////////////////////////////
//
// Tool_cmr::getMetlev -- convert note tokens into Metric levels.
//

void Tool_cmr::getMetlev(std::vector<double>& metlevs, std::vector<std::vector<HTp>>& notelist) {
	metlevs.resize(notelist.size());
	for (int i=0; i<(int)metlevs.size(); i++) {
		metlevs.at(i) = cmr_note_info::getMetricLevel(notelist.at(i).at(0));
	}
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

void Tool_cmr::getNoteList(vector<vector<HTp>>& notelist, HTp starting) {
	notelist.clear();
	notelist.reserve(2000);

	int lastpitch = -1;

	// HTp previous = NULL;
	HTp current = starting;
	while (current) {
		if (!current->isData()) {
			current = current->getNextToken();
			continue;
		}
		if (current->isNull()) {
			current = current->getNextToken();
			continue;
		}
		if (current->isNoteSustain()) {
			if (notelist.size() > 0) {
				notelist.back().push_back(current);
			}
			// previous = current;
			current = current->getNextToken();
			continue;
		}
		if (current->isRest()) {
			if ((!notelist.empty()) && notelist.back().at(0)->isRest()) {
				notelist.back().push_back(current);
				// previous = current;
				current = current->getNextToken();
				lastpitch = -1;
				continue;
			}
		}
		int pitch = current->getMidiPitch();
		if (pitch == lastpitch) {
			if (notelist.size() > 0) {
				notelist.back().push_back(current);
			}
			// previous = current;
			current = current->getNextToken();
			continue;
		}
		lastpitch = pitch;
		notelist.resize(notelist.size() + 1);
		notelist.back().push_back(current);
		if (!current->isRest()) {
			m_noteCount++;
		}
		// previous = current;
		current = current->getNextToken();
	}

	// Remove any rests that are shorter or equal to m_shortRest:
	vector<vector<HTp>> output;
	output.reserve(notelist.size());
	for (int i=0; i<(int)notelist.size() - 1; i++) {
		if (!notelist.at(i).at(0)->isRest()) {
			output.push_back(notelist.at(i));
			continue;
		}
		// get the duration of the (multi-rest):
		HumNum restStart = notelist.at(i).at(0)->getDurationFromStart();
		HumNum noteStart = notelist.at(i+1).at(0)->getDurationFromStart();
		HumNum duration = noteStart - restStart;
		if (duration.getFloat() > m_smallRest) {
			output.push_back(notelist.at(i));
		}
	}
}



//////////////////////////////
//
// getNoteDurations --
//

void  Tool_cmr::getDurations(vector<double>& durations, vector<vector<HTp>>& notelist) {
	durations.resize(notelist.size());
	for (int i=0; i<(int)notelist.size(); i++) {
		HumNum duration = notelist.at(i).at(0)->getTiedDuration();
		durations.at(i) = duration.getFloat();
	}
}



//////////////////////////////
//
// Tool_cmr::getBeat --
//

void  Tool_cmr::getBeat(vector<bool>& metpos, vector<vector<HTp>>& notelist) {
	metpos.resize(notelist.size());
	for (int i=0; i<(int)notelist.size(); i++) {
		HumNum position = notelist.at(i).at(0)->getDurationFromBarline();
		if (position.getDenominator() != 1) {
			metpos.at(i) = false;
		} if (position.getNumerator() % 4 == 0) {
			metpos.at(i) = true;
		} else {
			metpos.at(i) = false;
		}
	}
}



//////////////////////////////
//
// Tool_cmr::isMelodicallyAccented --
//

bool Tool_cmr::isMelodicallyAccented(int index) {
	return m_leapbefore.at(index) || m_syncopation.at(index);
}



//////////////////////////////
//
// Tool_cmr::isSyncopated --
//

bool Tool_cmr::isSyncopated(HTp token) {
	return cmr_note_info::isSyncopated(token);
}



//////////////////////////////
//
// Tool_cmr::countNotesInScore -- Count the number of notes in a score,
//     ignoring tied notes that are not attacks, and treating chords as a single note.
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



//////////////////////////////
//
// Tool_cmr::getGroupCount -- Return the number of groups.
//

int Tool_cmr::getGroupCount(void) {
	int output = 0;
	for (int i=0; i<(int)m_noteGroups.size(); i++) {
		if (m_noteGroups[i].isValid()) {
			output++;
		}
	}
	return output;
}



//////////////////////////////
//
// Tool_cmr::getGroupNoteCount -- Return the number notes in all groups.
//

int Tool_cmr::getGroupNoteCount(void) {
	int output = 0;
	for (int i=0; i<(int)m_noteGroups.size(); i++) {
		if (m_noteGroups[i].isValid()) {
			output += m_noteGroups[i].getNoteCount();
		}
	}
	return output;
}



//////////////////////////////
//
// Tool_cmr::getStrengthScore -- Return the total strength score for all CMRs.
//

int Tool_cmr::getStrengthScore(void) {
	int output = 0;
	for (int i=0; i<(int)m_noteGroups.size(); i++) {
		if (m_noteGroups[i].isValid()) {
			output += m_noteGroups[i].getGroupStrength();
		}
	}
	return output;
}




/////////////////////////////
//
// Tool_cmr::getVocalRange --
//

void Tool_cmr::getVocalRange(vector<string>& minpitch, vector<string>& maxpitch, vector<vector<HTp>>& notelist) {
	if (notelist.empty()) {
		return;
	}
	int track = notelist.at(0).at(0)->getTrack();
	HTp mintok = NULL;
	HTp maxtok = NULL;
	int minb40 = -1;
	int maxb40 = -1;

	for (int i=0; i<(int)notelist.size(); i++) {
		if (notelist.at(i).at(0)->isRest()) {
			continue;
		}

		if (mintok == NULL) {
			mintok = notelist.at(i).at(0);
			minb40 = Convert::kernToBase40(mintok);
		} else {
			int b40 = Convert::kernToBase40(notelist.at(i).at(0));
			if (b40 < minb40) {
				minb40 = b40;
				mintok = notelist.at(i).at(0);
			}
		}

		if (maxtok == NULL) {
			maxtok = notelist.at(i).at(0);
			maxb40 = Convert::kernToBase40(maxtok);
		} else {
			int b40 = Convert::kernToBase40(notelist.at(i).at(0));
			if (b40 > maxb40) {
				maxb40 = b40;
				maxtok = notelist.at(i).at(0);
			}
		}
	}

	minpitch.at(track) = getPitch(mintok);
	maxpitch.at(track) = getPitch(maxtok);
}



//////////////////////////////
//
// Tool_cmr::getPitch -- Return scientific pitch name.
//

string Tool_cmr::getPitch(HTp token) {
	if (!token) {
		return "";
	}
	if (token->isRest()) {
		return "R";
	} else {
		int octave = Convert::kernToOctaveNumber(token);
		int accidentals = Convert::kernToAccidentalCount(token);
		int dpc = Convert::kernToDiatonicPC(token);
		string output;
		switch (dpc) {
			case 0: output += "C"; break;
			case 1: output += "D"; break;
			case 2: output += "E"; break;
			case 3: output += "F"; break;
			case 4: output += "G"; break;
			case 5: output += "A"; break;
			case 6: output += "B"; break;
		}
		for (int i=0; i<abs(accidentals); i++) {
			if (accidentals < 0) {
				output += "b";
			} else {
				output += "#";
			}
		}
		output += to_string(octave);
		return output;
	}
}



//////////////////////////////
//
// Tool_cmr::addGroupNumbersToScore --
//

void Tool_cmr::addGroupNumbersToScore(HumdrumFile& infile) {
	for (int i=0; i<(int)m_noteGroups.size(); i++) {
		if (m_noteGroups.at(i).getSerial() <= 0) {
			continue;
		}
		HTp token = m_noteGroups.at(i).getFirstToken();
		if (!token) {
			continue;
		}
		int serial = m_noteGroups.at(i).getSerial();
		int direction = m_noteGroups.at(i).getDirection();
		addGroupNumberToScore(infile, token, serial, direction);
	}
}



//////////////////////////////
//
// Tool_cmr::addGroupNumberToScore
//

void Tool_cmr::addGroupNumberToScore(HumdrumFile& infile, HTp note, int number, int dir) {
	if (!note) {
		return;
	}

	int index = note->getLineIndex();
	int field = note->getFieldIndex();

	if (index > 0) {
		if (infile[index-1].isCommentLocal()) {
			HTp token = infile.token(index-1, field);
			if (*token == "!") {
				string output = getLocalLabelToken(number, dir);
				token->setText(output);
				return;
			}
		} else {
			string newline;
			for (int i=0; i<infile[index].getFieldCount(); i++) {
				if (i == field) {
					string newtoken = getLocalLabelToken(number, dir);
					newline += newtoken;
				} else {
					newline += "!";
				}
				if (i < infile[index].getFieldCount() - 1) {
					newline += "\t";
				}
			}
			infile.insertLine(index, newline);
		}
	}
}



//////////////////////////////
//
// Tool_cmr::getLocalLabelToken --
//

string Tool_cmr::getLocalLabelToken(int number, int dir) {
	string output = "!LO:TX:";
	if (dir > 0) {
		output += "a:color=";
		output += m_colorUp;
	} else {
		output += "b:color=";
		output += m_colorDown;
	}
	output += ":t=";
	output += to_string(number);
	return output;
}



// END_MERGE

} // end namespace hum
