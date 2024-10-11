//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Thu Jun  3 14:08:25 PDT 2010
// Last Modified: Tue Jun 15 14:15:42 PDT 2010 (added tied note functionality)
// Filename:      ...sig/src/sigInfo/MuseData.cpp
// Web Address:   http://sig.sapp.org/src/sigInfo/MuseData.cpp
// Syntax:        C++
// vim:           ts=3
//
// Description:   A class that multiple MuseRecord lines.
//

#include "HumRegex.h"
#include "MuseData.h"

#include <cctype>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <sstream>

using namespace std;

namespace hum {

// START_MERGE


///////////////////////////////////////////////////////////////////////////
//
// MuseEventSet class functions --
//


//////////////////////////////
//
// MuseEventSet::MuseEventSet --
//

MuseEventSet::MuseEventSet (void) {
	events.reserve(20);
	clear();
}


MuseEventSet::MuseEventSet(HumNum atime) {
	setTime(atime);
	events.reserve(20);
}

MuseEventSet::MuseEventSet(const MuseEventSet& aSet) {
	absbeat = aSet.absbeat;
	events.resize(aSet.events.size());
	for (int i=0; i<(int)aSet.events.size(); i++) {
		events[i] = aSet.events[i];
	}
}



//////////////////////////////
//
// MuseEventSet::operator= --
//

MuseEventSet MuseEventSet::operator=(MuseEventSet& anevent) {
	if (&anevent == this) {
		return *this;
	}
	absbeat = anevent.absbeat;
	events.resize(anevent.events.size());
	for (int i=0; i<(int)events.size(); i++) {
		events[i] = anevent.events[i];
	}
	return *this;
}



//////////////////////////////
//
// MuseEventSet::clear --
//

void MuseEventSet::clear(void) {
	events.clear();
	absbeat.setValue(0,1);
}



//////////////////////////////
//
// MuseEventSet::setTime --
//

void MuseEventSet::setTime(HumNum abstime) {
	absbeat = abstime;
}



//////////////////////////////
//
// MuseEventSet::getTime --
//

HumNum MuseEventSet::getTime(void) {
	return absbeat;
}



//////////////////////////////
//
// MuseEventSet::appendRecord -- still have to sort after insertion...
//   also add a removeEvent function so deleted elements can be removed
//   gracefully.
//

void MuseEventSet::appendRecord(MuseRecord* arecord) {
	events.push_back(arecord);
}



//////////////////////////////
//
// MuseEventSet::operator[] --
//

MuseRecord& MuseEventSet::operator[](int eindex) {
	return *(events[eindex]);
}



//////////////////////////////
//
// MuseEventSet::getEventCount --
//

int MuseEventSet::getEventCount(void) {
	return (int)events.size();
}



///////////////////////////////////////////////////////////////////////////
//
// MuseData class functions --
//


//////////////////////////////
//
// MuseData::MuseData --
//

MuseData::MuseData(void) {
	m_data.reserve(100000);
}

MuseData::MuseData(MuseData& input) {
	m_data.resize(input.m_data.size());
	MuseRecord* temprec;
	int i;
	for (i=0; i<(int)m_data.size(); i++) {
		temprec  = new MuseRecord;
		*temprec = *(input.m_data[i]);
		m_data[i]  = temprec;
		m_data[i]->setLineIndex(i);
		m_data[i]->setOwner(this);
	}
	m_sequence.resize(input.m_sequence.size());
	for (i=0; i<(int)input.m_sequence.size(); i++) {
	  m_sequence[i] = new MuseEventSet;
	  *(m_sequence[i]) = *(input.m_sequence[i]);
	}

	m_name = input.m_name;
}



//////////////////////////////
//
// MuseData::~MuseData --
//

MuseData::~MuseData() {
	clear();
}



//////////////////////////////
//
// MuseData::operator= --
//

MuseData& MuseData::operator=(MuseData& input) {
	if (this == &input) {
		return *this;
	}
	m_data.resize(input.m_data.size());
	MuseRecord* temprec;
	int i;
	for (i=0; i<(int)m_data.size(); i++) {
		temprec = new MuseRecord;
		*temprec = *(input.m_data[i]);
		m_data[i] = temprec;
		m_data[i]->setLineIndex(i);
		m_data[i]->setOwner(this);
	}
	// do something with m_sequence...
	m_name = input.m_name;
	return *this;
}



//////////////////////////////
//
// MuseData::getLineCount -- return the number of lines in the MuseData file.
//

int MuseData::getLineCount(void) {
	return (int)m_data.size();
}



//////////////////////////////
//
// MuseData::append -- add a MuseRecord to end of file.
//

int MuseData::append(MuseRecord& arecord) {
	MuseRecord* temprec = new MuseRecord;
	*temprec = arecord;
	temprec->setOwner(this);
	m_data.push_back(temprec);
	m_data.back()->setLineIndex((int)m_data.size() - 1);
	return (int)m_data.size()-1;
}


int MuseData::append(MuseData& musedata) {
	int oldsize = (int)m_data.size();
	int newlinecount = musedata.getLineCount();
	if (newlinecount <= 0) {
		return -1;
	}

	m_data.resize((int)m_data.size()+newlinecount);
	for (int i=0; i<newlinecount; i++) {
		m_data[i+oldsize] = new MuseRecord;
		*(m_data[i+oldsize]) = musedata[i];
		m_data[i+oldsize]->setLineIndex(i+oldsize);
		m_data[i+oldsize]->setOwner(this);
	}
	return (int)m_data.size()-1;
}


int MuseData::append(string& charstring) {
	MuseRecord* temprec;
	temprec = new MuseRecord;
	temprec->setString(charstring);
	temprec->setType(E_muserec_unknown);
	temprec->setQStamp(0);
	m_data.push_back(temprec);
	temprec->setLineIndex((int)m_data.size() - 1);
	temprec->setOwner(this);
	return (int)m_data.size()-1;
}



//////////////////////////////
//
// MuseData::insert -- add a MuseRecord to middle of file.  Not the most
//   efficient, but not too bad as long as the file is not too long, the
//   insertion is close to the end of the file, and you don't use this
//   method to add a set of sequential lines (write a different function
//   for that).
//

void MuseData::insert(int lindex, MuseRecord& arecord) {
	MuseRecord* temprec;
	temprec = new MuseRecord;
	*temprec = arecord;
	temprec->setOwner(this);

	m_data.resize(m_data.size()+1);
	for (int i=(int)m_data.size()-1; i>lindex; i--) {
		m_data[i] = m_data[i-1];
		m_data[i]->setLineIndex(i);
	}
	m_data[lindex] = temprec;
	temprec->setLineIndex(lindex);
}



//////////////////////////////
//
// MuseData::clear --
//

void MuseData::clear(void) {
	for (int i=0; i<(int)m_data.size(); i++) {
		if (m_data[i] != NULL) {
			delete m_data[i];
			m_data[i] = NULL;
		}
	}
	for (int i=0; i<(int)m_sequence.size(); i++) {
		m_sequence[i]->clear();
		delete m_sequence[i];
		m_sequence[i] = NULL;
	}
	m_error.clear();
	m_data.clear();
	m_sequence.clear();
	m_name = "";
}



//////////////////////////////
//
// MuseData::operator[] --
//

MuseRecord& MuseData::operator[](int lindex) {
	return *(m_data[lindex]);
}



//////////////////////////////
//
// MuseData::getRecord --
//

MuseRecord& MuseData::getRecord(int lindex) {
	return *(m_data[lindex]);
}



//////////////////////////////
//
// MuseData::getRecordPointer --
//

MuseRecord* MuseData::getRecordPointer(int lindex) {
	return m_data[lindex];
}


//////////////////////////////
//
// MuseData::getRecord -- This version with two index inputs is
//     used to access a data line based on the event time index (time sorted
//     viewpoint) and the particular record index for that event time.
//

MuseRecord& MuseData::getRecord(int eindex, int erecord) {
	return *(m_data[getEvent(eindex)[erecord].getLineIndex()]);
}



//////////////////////////////
//
// MuseData::read -- read a MuseData file from a file or input stream.
//   0x0a      = unix
//   0x0d      = apple
//   0x0d 0x0a = dos
//

int MuseData::read(istream& input) {
	m_error.clear();
	string dataline;
	dataline.reserve(256);
	int character;
	char value;
	int  isnewline;
	char lastvalue = 0;

	while (!input.eof()) {
		character = input.get();
		if (input.eof()) {
			// end of file found without a newline termination on last line.
			if (dataline.size() > 0) {
				MuseData::append(dataline);
				dataline.clear();
				break;
			}
		}
		value = (char)character;
		if ((value == 0x0d) || (value == 0x0a)) {
			isnewline = 1;
		} else {
			isnewline = 0;
		}

		if (isnewline && (value == 0x0a) && (lastvalue == 0x0d)) {
			// ignore the second newline character in a dos-style newline.
			lastvalue = value;
			continue;
		} else {
			lastvalue = value;
		}

		if (isnewline) {
			MuseData::append(dataline);
			dataline.clear();
		} else {
			dataline.push_back(value);
		}
	}

	for (int i=0; i<(int)m_data.size(); i++) {
		m_data[i]->setLineIndex(i);
	}

	doAnalyses();
	if (hasError()) {
		cerr << m_error << endl;
		return 0;
	} else {
		return 1;
	}
}


int MuseData::readFile(const string& filename) {
	ifstream infile(filename);
	return MuseData::read(infile);
}

int MuseData::readString(const string& data) {
	stringstream ss;
	ss << data;
	return MuseData::read(ss);
}



//////////////////////////////
//
// MuseData::doAnalyses --  perform post-processing analysis of the data file
//    (in the correct order).
//

void MuseData::doAnalyses(void) {
	analyzeType();
	analyzeTpq();
	if (hasError()) { return; }
	assignHeaderBodyState();
   analyzeLayers();
	analyzeRhythm();
	if (hasError()) { return; }
	constructTimeSequence();
	if (hasError()) { return; }
	analyzePitch();
	if (hasError()) { return; }
	analyzeTies();
	if (hasError()) { return; }
	linkPrintSuggestions();
	linkMusicDirections();
}



//////////////////////////////
//
// MuseData::analyzeTpq -- Read $ records for Q: field values and store
//    the ticks-per-quarter note value on each line after that $ record.
//    This is used later to extract the logical duration of notes and rests.
//

void MuseData::analyzeTpq(void) {
	HumRegex hre;
	int ticks = 0;
	for (int i=0; i<getLineCount(); i++) {
		MuseRecord* mr = &getRecord(i);
		if (!mr->isAttributes()) {
			mr->setTpq(ticks);

			continue;
		}
		string line = getLine(i);
		if (hre.search(line, " Q:(\\d+)")) {
			ticks = hre.getMatchInt(1);
		}
		mr->setTpq(ticks);
	}
}



//////////////////////////////
//
// MuseData::analyzePitch -- calculate the pitch of all notes in terms
//    of their base40 value.
//

void MuseData::analyzePitch() {
	for (int i=0; i<(int)m_data.size(); i++) {
		m_data[i]->setMarkupPitch(m_data[i]->getBase40());
	}
}



//////////////////////////////
//
// MuseData::analyzeTies -- identify which notes are tied to each other.
//

void MuseData::analyzeTies(void) {
	for (int i=0; i<(int)m_sequence.size(); i++) {
		for (int j=0; j<m_sequence[i]->getEventCount(); j++) {
			if (!getEvent(i)[j].tieQ()) {
				continue;
			}
			processTie(i, j, -1);
		}
	}
}



//////////////////////////////
//
// MuseData::processTie -- follow a tied note to the last note
//   in the tied m_sequence, filling in the tie information along the way.
//   Hanging ties (particularly ties note at the ends of first repeats, etc.)
//   still need to be considered.
//

void MuseData::processTie(int eindex, int rindex, int lastindex) {
	int& i = eindex;
	int& j = rindex;


	// lineindex = index of line in original file for the current line.
	int lineindex = getEvent(i)[j].getLineIndex();


	if ((lastindex < 0) &&
		 (m_data[lineindex]->getLastTiedNoteLineIndex() >= 0)) {
		// If there previously tied note already marked in the data, then
		// this note has already been processed for ties, so exit function
		// without doing any further processing.
		return;
	}

	// store the location of the note tied to previously:
	m_data[lineindex]->setLastTiedNoteLineIndex(lastindex);

	// If the current note contains a tie marker, then there is
	// another tied note in the future, so go look for it.
	if (!m_data[lineindex]->tieQ()) {
		m_data[lineindex]->setNextTiedNoteLineIndex(-1);
		return;
	}

	// There is another note tied to this one in the future, so
	// first get the absolute time location of the future tied note
	HumNum abstime    = m_data[lineindex]->getQStamp();
	HumNum notedur    = m_data[lineindex]->getNoteDuration();
	HumNum searchtime = abstime + notedur;

	// Get the event index which occurs at the search time:
	int nexteindex = getNextEventIndex(eindex, abstime + notedur);

	if (nexteindex < 0) {
		// Couldn't find any data at that absolute time index, so give up:
		m_data[lineindex]->setNextTiedNoteLineIndex(-1);
		return;
	}

	// The pitch of the tied note should match this one; otherwise, it
	// would not be a tied note...
	int base40 = m_data[lineindex]->getPitch();

	// The tied note will preferrably be found in the same track as the
	// current note (but there could be a cross-track tie occurring, so
	// check for that if there is no same-track tie):
	int track = m_data[lineindex]->getTrack();

	int nextrindex = searchForPitch(nexteindex, base40, track);
	if (nextrindex < 0) {
		// Didn't find specified note at the given event index in the given
		// track, so search for the same pitch in any track at the event time:
		 nextrindex = searchForPitch(nexteindex, base40, -1);
	}

	if (nextrindex < 0) {
		// Failed to find a note at the target event time which could be
		// tied to the current note (no pitches at that time which match
		// the pitch of the current note).  This is a haning tie which is
		// either a data error, or is a tie to a note at an earlier time
		// in the music (such as at the beginning of a repeated section).
		// for now just ignore the hanging tie, but probably mark it in
		// some way in the future.
		m_data[lineindex]->setNextTiedNoteLineIndex(-1);
		return;
	}

	// now the specific note to which this one is tied to is known, so
	// go and process that note:

	int nextindex = getEvent(nexteindex)[nextrindex].getLineIndex();

	m_data[lineindex]->setNextTiedNoteLineIndex(nextindex);

	processTie(nexteindex, nextrindex, lineindex);
}



//////////////////////////////
//
// MuseData::searchForPitch -- search for a matching pitch in the given
//   track at the specified event index.  If the track is negative, then
//   find the first matching pitch in any track.
//
//   Will also have to separate by category, so that grace notes are
//   searched for separately from regular notes / chord notes, and
//   cue notes as well.
//

int MuseData::searchForPitch(int eventindex, int b40, int track) {
	int targettrack;
	int targetpitch;
	int targettype;

	for (int j=0; j<m_sequence[eventindex]->getEventCount(); j++) {
		targettype = getEvent(eventindex)[j].getType();
		if ((targettype != E_muserec_note_regular) &&
			 (targettype != E_muserec_note_chord) ) {
			// ignore non-note data (at least ones without durations):
			continue;
		}
		targettrack = getEvent(eventindex)[j].getTrack();
		if ((track >= 0) && (track != targettrack)) {
			continue;
		}
		targetpitch = getEvent(eventindex)[j].getPitch();
		if (targetpitch == b40) {
			return j;
		}
	}

	return -1;
}



//////////////////////////////
//
// MuseData::getNextEventIndex -- return the event index for the given
//   absolute time value.  The starting index is given first, and it
//   is assumed that the target absolute time occurs on or after the
//   starting index value.  Returns -1 if that absolute time is not
//   found in the data (or occurs before the start index.
//

int MuseData::getNextEventIndex(int startindex, HumNum target) {
	int output = -1;
	for (int i=startindex; i<(int)m_sequence.size(); i++) {
		if (m_sequence[i]->getTime() == target) {
			output = i;
			break;
		}
	}
	return output;
}


//////////////////////////////
//
// MuseData::last -- return the last record in the data.  Make sure
// that isEmpty() is not true before calling this function.
//

MuseRecord& MuseData::last(void) {
	return (*this)[getNumLines()-1];
}



//////////////////////////////
//
// MuseData::isEmpty -- return true if there are no MuseRecords in the
//    object; otherwise returns true;
//

int MuseData::isEmpty(void) {
	return m_data.empty();
}



//////////////////////////////
//
// MuseData::analyzeType --
//

void MuseData::analyzeType(void) {
	int commentQ = 0;
	int h = 0;
	MuseData& thing = *this;
	int foundattributes = 0;
	int foundend = 0;

	HumRegex hre;
	int groupmemberships = -1;
	for (int i=0; i<getLineCount(); i++) {
		string line = thing[i].getLine();
		if (hre.search(line, "^Group memberships:")) {
			groupmemberships = i;
			break;
		}
	}
	if (groupmemberships < 0) {
		cerr << "Error cannot parse MuseData content" << endl;
		return;
	}

	thing[groupmemberships].setType(E_muserec_header_11);

	h = 11;
	for (int i=groupmemberships-1; i>=0; i--) {
		if (thing[i].getLength() > 0) {
			if (thing[i][0] == '@') {
				thing[i].setType(E_muserec_comment_line);
				continue;
			}
			if (thing[i][0] == '&') {
				// start or end of multi-line comment;
				commentQ = !commentQ;
				if (!commentQ) {
					thing[i].setType(E_muserec_comment_toggle);
	       continue;
				}
			}
			if (commentQ) {
				thing[i].setType(E_muserec_comment_toggle);
				continue;
			}
		}
		h--;
		if      (h==1)  { thing[i].setType(E_muserec_header_1);  continue; }
		else if (h==2)  { thing[i].setType(E_muserec_header_2);  continue; }
		else if (h==3)  { thing[i].setType(E_muserec_header_3);  continue; }
		else if (h==4)  { thing[i].setType(E_muserec_header_4);  continue; }
		else if (h==5)  { thing[i].setType(E_muserec_header_5);  continue; }
		else if (h==6)  { thing[i].setType(E_muserec_header_6);  continue; }
		else if (h==7)  { thing[i].setType(E_muserec_header_7);  continue; }
		else if (h==8)  { thing[i].setType(E_muserec_header_8);  continue; }
		else if (h==9)  { thing[i].setType(E_muserec_header_9);  continue; }
		else if (h==10) { thing[i].setType(E_muserec_header_10); continue; }
	}

	commentQ = 0;
	h = 11;
	for (int i=groupmemberships+1; i<getNumLines(); i++) {

		if (thing[i].getLength() > 0) {
			if (thing[i][0] == '@') {
				thing[i].setType(E_muserec_comment_line);
				continue;
			}
			if (thing[i][0] == '&') {
				// start or end of multi-line comment;
				commentQ = !commentQ;
				if (!commentQ) {
					thing[i].setType(E_muserec_comment_toggle);
	       continue;
				}
			}
			if (commentQ) {
				thing[i].setType(E_muserec_comment_toggle);
				continue;
			}
		}
		h++;
		if      (h==1)  { thing[i].setType(E_muserec_header_1);  continue; }
		else if (h==2)  { thing[i].setType(E_muserec_header_2);  continue; }
		else if (h==3)  { thing[i].setType(E_muserec_header_3);  continue; }
		else if (h==4)  { thing[i].setType(E_muserec_header_4);  continue; }
		else if (h==5)  { thing[i].setType(E_muserec_header_5);  continue; }
		else if (h==6)  { thing[i].setType(E_muserec_header_6);  continue; }
		else if (h==7)  { thing[i].setType(E_muserec_header_7);  continue; }
		else if (h==8)  { thing[i].setType(E_muserec_header_8);  continue; }
		else if (h==9)  { thing[i].setType(E_muserec_header_9);  continue; }
		else if (h==10) { thing[i].setType(E_muserec_header_10); continue; }

		else if (h==11) { thing[i].setType(E_muserec_header_11); continue; }
		else if (h==12) { thing[i].setType(E_muserec_header_12); continue; }

		if (thing[i].getLength() == 0) {
			thing[i].setType(E_muserec_empty);
			continue;
		}

		if ((h > 12) && (thing[i][0] != '$') && (foundattributes == 0)) {
			thing[i].setType(E_muserec_header_12);
			continue;
		}

		if (foundend && thing[i][0] != '/') {
			thing[i].setType(E_muserec_endtext);
			continue;
		}

		switch (thing[i][0]) {
			case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
			case 'G': thing[i].setType(E_muserec_note_regular);       break;
			case ' ': thing[i].setType(E_muserec_note_chord);         break;
			case 'c': thing[i].setType(E_muserec_note_cue);           break;
			case 'g': thing[i].setType(E_muserec_note_grace);         break;
			case 'P': thing[i].setType(E_muserec_print_suggestion);   break;
			case 'S': thing[i].setType(E_muserec_sound_directives);   break;
			case '/': thing[i].setType(E_muserec_end);
			          foundend = 1;
			          break;
			case 'a': thing[i].setType(E_muserec_append);             break;
			case 'b': thing[i].setType(E_muserec_backspace);          break;
			case 'f': thing[i].setType(E_muserec_figured_harmony);    break;
			case 'i': thing[i].setType(E_muserec_rest_invisible);     break;
			case 'm': thing[i].setType(E_muserec_measure);            break;
			case 'r': thing[i].setType(E_muserec_rest);               break;
			case '*': thing[i].setType(E_muserec_musical_directions); break;
			case '$': thing[i].setType(E_muserec_musical_attributes);
			          foundattributes = 1;
			          break;
		}
	}
}



//////////////////////////////
//
// MuseData::analyzeRhythm -- calculate the start time in quarter notes
//   for each note/rest in the file.
//
//   Secondary chord notes may or may not have a duration listed.
//   If they do not, then the duration of the note is the same
//   as the primary note of the chord.
//
// char*            getTickDurationField         (char* output);
//

void MuseData::analyzeRhythm(void) {
	HumNum cumulative(0,1);
	HumNum linedur(0,1);
	int tpq = 1;
	HumRegex hre;
	HumNum figadj = 0;   // needed for figured harmony
	HumNum primarychordnoteduration(0,1);  // needed for chord notes

	for (int i=0; i<(int)m_data.size(); i++) {
		if (m_data[i]->isAttributes()) {
			string line = m_data[i]->getLine();
			if (hre.search(line, "Q:(\\d+)", "")) {
				tpq = hre.getMatchInt(1);
			}
		}

		if (m_data[i]->isChordNote()) {
			// insert an automatic back command for chord tones
			// also deal with cue-size note chords?
			m_data[i]->setQStamp(cumulative - primarychordnoteduration);

			// Check to see if the secondary chord note has a duration.
			// If so, then set the note duration to that value; otherwise,
			// set the note duration to the duration of the primary chord
			// note (first note before the current note which is not a chord
			// note).
			string buffer = m_data[i]->getTickDurationField();
			if (hre.search(buffer, "\\d", "")) {
				m_data[i]->setNoteDuration(m_data[i]->getNoteTickDuration(), tpq);
			} else {
				m_data[i]->setNoteDuration(primarychordnoteduration);
			}
			m_data[i]->setLineDuration(0);
		} else if (m_data[i]->isFiguredHarmony()) {
			// Tick values on figured harmony lines do not advance the
			// cumulative timestamp; instead they temporarily advance
			// the time placement of the next figure if it occurs
			// during the same note as the previous figure.
			m_data[i]->setQStamp(cumulative + figadj);
			HumNum tick = m_data[i]->getLineTickDuration();
			if (tick == 0) {
				figadj = 0;
			} else {
				HumNum dur = tick;
				dur /= tpq;
				figadj += dur;
			}
		} else {
			m_data[i]->setQStamp(cumulative);
			m_data[i]->setNoteDuration(m_data[i]->getNoteTickDuration(), tpq);
			m_data[i]->setLineDuration(m_data[i]->getNoteDuration());
			linedur.setValue(m_data[i]->getLineTickDuration(), tpq);
			cumulative += linedur;
		}

		switch (m_data[i]->getType()) {
			case E_muserec_note_regular:
			// should also worry about cue and grace note chords?
			primarychordnoteduration = linedur;
		}
	}

	// adjust Sound and Print records so that they occur at the same
	// absolute time as the note they affect.
	for (int i=1; i<(int)m_data.size(); i++) {
		switch (m_data[i]->getType()) {
			case E_muserec_print_suggestion:
			case E_muserec_sound_directives:
				m_data[i]->setQStamp(m_data[i-1]->getQStamp());
		}
	}

}



//////////////////////////////
//
// MuseData::getInitialTpq -- return the Q: field in the first $ record
//    at the top of the file.  If there is an updated Q: field, this
//    function will need to be improved since TPQ cannot change in MIDI files,
//    for example.
//

int MuseData::getInitialTpq(void) {
	int output = 0;
	if (m_data.empty()) {
		return output;
	}
	HumRegex hre;
	int i;
	if (m_data[0]->getType() == E_muserec_unknown) {
		// search for first line which starts with '$':
		for (i=0; i<(int)m_data.size(); i++) {
			if (m_data[i]->getLength() <= 0) {
				continue;
			}
			if ((*m_data[i])[0] == '$') {
				string line = m_data[i]->getLine();
				if (hre.search(line, "Q:(\\d+)", "")) {
					output = hre.getMatchInt(1);
				}
				break;
			}
		}
	} else {
		for (int i=0; i<(int)m_data.size(); i++) {
			if (m_data[i]->getType() == E_muserec_musical_attributes) {
				string line = m_data[i]->getLine();
				if (hre.search(line, "Q:(\\d+)", "")) {
					output = hre.getMatchInt(1);
				}
				break;
			}
		}
	}

	return output;
}



//////////////////////////////
//
// constructTimeSequence -- Make a list of the lines in the file
//    sorted by the absolute time at which they occur.
//

void MuseData::constructTimeSequence(void) {
	// * clear old event set
	// * allocate the size to match number of lines (* 2 probably).

	MuseData& thing = *this;
	for (int i=0; i<(int)m_data.size(); i++) {
		insertEventBackwards(thing[i].getQStamp(), &thing[i]);
		if (hasError()) {
			return;
		}
	}
}



///////////////////////////////
//
// MuseData::getEventCount -- returns the number of unique times
//    at which data line occur in the file.
//

int MuseData::getEventCount(void) {
	return (int)m_sequence.size();
}



///////////////////////////////
//
// MuseData::getEvent --
//

MuseEventSet& MuseData::getEvent(int eindex) {
	return *(m_sequence[eindex]);
}



///////////////////////////////////////////////////////////////////////////
//
// private functions
//


//////////////////////////////
//
// printSequenceTimes --
//

void printSequenceTimes(vector<MuseEventSet*>& m_sequence) {
	for (int i=0; i<(int)m_sequence.size(); i++) {
		cout << m_sequence[i]->getTime().getFloat() << " ";
	}
	cout << endl;
}



//////////////////////////////
//
// MuseData::insertEventBackwards -- insert an event into a time-sorted
//    organization of the file. Searches for the correct time location to
//    insert event starting at the end of the list (since MuseData files
//    are mostly sorted in time.
//
//

void MuseData::insertEventBackwards(HumNum atime, MuseRecord* arecord) {
	if (m_sequence.empty()) {
		MuseEventSet* anevent = new MuseEventSet;
		anevent->setTime(atime);
		anevent->appendRecord(arecord);
		m_sequence.push_back(anevent);
		return;
	}

	for (int i=(int)m_sequence.size()-1; i>=0; i--) {
		if (m_sequence[i]->getTime() == atime) {
			m_sequence[i]->appendRecord(arecord);
			return;
		} else if (m_sequence[i]->getTime() < atime) {
			// insert new event entry after the current one since it occurs later.
			MuseEventSet* anevent = new MuseEventSet;
			anevent->setTime(atime);
			anevent->appendRecord(arecord);
			if (i == (int)m_sequence.size()-1) {
				// just append the event at the end of the list
				m_sequence.push_back(anevent);
	    return;
			} else {
				// event has to be inserted before end of list, so move
				// later ones up in list.
				m_sequence.resize(m_sequence.size()+1);
				for (int j=(int)m_sequence.size()-1; j>i+1; j--) {
					m_sequence[j] = m_sequence[j-1];
				}
				// store the newly created event entry in m_sequence:
				m_sequence[i+1] = anevent;
				return;
			}
		}
	}
	stringstream ss;
	ss << "Funny error occurred at time " << atime;
	setError(ss.str());
}



//////////////////////////////
//
// MuseData::getTiedDuration -- these version acess the record lines
//    via the time-sorted event index.
//

HumNum MuseData::getTiedDuration(int eindex, int erecord) {
	return getTiedDuration(getLineIndex(eindex, erecord));
}



//////////////////////////////
//
// MuseData:getTiedDuration --
//

HumNum MuseData::getTiedDuration(int index) {
	HumNum output(0,1);

	// if the line is not a regular/chord note with duration, then
	// return the duration field.
	if ((getRecord(index).getType() != E_muserec_note_chord) &&
		 (getRecord(index).getType() != E_muserec_note_regular) ) {
		return output;
	}

	// if the note is tied to a previous note, then return a
	// duration of 0 (behavior may change in the future).
	if (getRecord(index).getLastTiedNoteLineIndex() >= 0) {
		return output;
	}

	// if the note is not tied to anything into the future, then
	// gives it actual duration
	if (getRecord(index).getNextTiedNoteLineIndex() < 0) {
		return getRecord(index).getNoteDuration();
	}

	// this is start of a group of tied notes.  Start figuring out
	// how long the duration of the tied group is.
	output = getRecord(index).getNoteDuration();
	int myindex = index;
	while (getRecord(myindex).getNextTiedNoteLineIndex() >= 0) {
		myindex = getRecord(myindex).getNextTiedNoteLineIndex();
		output += getRecord(myindex).getNoteDuration();
	}

	return output;
}



//////////////////////////////
//
// MuseData::getLineIndex -- return the line number of a particular
//    event/record index pair (the line index is the same as the index
//    into the list of lines).
//

int MuseData::getLineIndex(int eindex, int erecord) {
	return getRecord(eindex, erecord).getLineIndex();
}



//////////////////////////////
//
// MuseData::getLineDuration -- return the duration of an isolated line.
//

HumNum MuseData::getLineDuration(int eindex, int erecord) {
	return getRecord(eindex, erecord).getLineDuration();
}



//////////////////////////////
//
// MuseData::getNoteDuration -- return the duration of an isolated note.
//

HumNum MuseData::getNoteDuration(int eindex, int erecord) {
	return getRecord(eindex, erecord).getNoteDuration();
}



//////////////////////////////
//
// MuseData::getLastTiedNoteLineIndex -- return the line index of the
//     previous note to which this one is tied to.  Returns -1 if there
//     is no previous tied note.
//

int MuseData::getLastTiedNoteLineIndex(int eindex, int erecord) {
	return getRecord(eindex, erecord).getLastTiedNoteLineIndex();
}



//////////////////////////////
//
// MuseData::getNextTiedNoteLineIndex -- returns the line index of the
//    next note to which this one is tied to.  Returns -1 if there
//    is no previous tied note.
//

int MuseData::getNextTiedNoteLineIndex(int eindex, int erecord) {
	return getRecord(eindex, erecord).getNextTiedNoteLineIndex();
}



//////////////////////////////
//
// MuseData::getType -- return the record type of a particular event
//     record.
//

int MuseData::getType(int eindex, int erecord) {
	return getRecord(eindex, erecord).getType();
}



//////////////////////////////
//
// MuseData::getQStamp -- return the absolute beat time (quarter
//    note durations from the start of the music until the current
//    object.
//

HumNum MuseData::getQStamp(int lindex) {
	return m_data[lindex]->getQStamp();
}


HumNum MuseData::getAbsBeat(int lindex) {
	return m_data[lindex]->getQStamp();
}



//////////////////////////////
//
// MuseData::getLineTickDuration --
//

int MuseData::getLineTickDuration(int lindex) {
	return m_data[lindex]->getLineTickDuration();
}


//////////////////////////////
//
// MuseData::setFilename --
//

void MuseData::setFilename(const string& filename) {
	m_name = filename;
}



//////////////////////////////
//
// MuseData::getFilename --
//

string MuseData::getFilename(void) {
	return m_name;
}


/*

//////////////////////////////
//
// MuseData::getPartName --
//

string MuseData::getPartName(void) {
	string output;
	for (int i=0; i<getLineCount(); i++) {
		if (isPartName(i)) {
			output = getPartName(i);
			break;
		}
	}
	for (int i=(int)output.size() - 1; i>=0; i--) {
		if (isspace(output[i])) {
			output.resize((int)output.size() - 1);
		} else {
			break;
		}
	}
	return output;
}

*/


//////////////////////////////
//
// MuseData::getPartName -- return name of the part
//

string MuseData::getPartName(void) {
	int line = getPartNameIndex();
	if (line < 0) {
		return "";
	}
	return m_data[line]->getPartName();
}



//////////////////////////////
//
// MuseData::getPartNameIndex -- Search for the part name in the header.
//    search the entire file if it is missing (which it should not be.
//

int MuseData::getPartNameIndex(void) {
	int output = -1;
	for (int i=0; i<(int)m_data.size(); i++) {
		if (m_data[i]->isPartName()) {
			return i;
		}
	}
	return output;
}



//////////////////////////////
//
// MuseData::isMember -- returns true if the file belongs to the
//     given membership string.  Example memberships are "score",
//     "skore", "part", "sound".
//

int MuseData::isMember(const string& mstring) {
	for (int i=0; i<(int)m_data.size(); i++) {
		if (m_data[i]->getType() == E_muserec_group_memberships) {
			if (strstr(m_data[i]->getLine().c_str(), mstring.c_str()) != NULL) {
				return 1;
			} else {
				return 0;
			}
		}
		if (m_data[i]->getType() == E_muserec_musical_attributes) {
			break;
		}
	}
	return 0;
}



//////////////////////////////
//
// MuseData::getMembershipPartNumber -- returns the part number within
//     a given membership for the data.
// Example:
//     sound: part 1 of 4
//  in this case the value returned is 1.
//

int MuseData::getMembershipPartNumber(const string& mstring) {
	string searchstring = "^";
	searchstring += mstring;
	searchstring += ":";

	HumRegex hre;
	for (int i=0; i<(int)m_data.size(); i++) {
		if (m_data[i]->getType() == E_muserec_header_12) {
			string line = m_data[i]->getLine();
			if (hre.search(line, searchstring)) {
				if (hre.search(line, "part\\s*(\\d+)\\s*of\\s*(\\d+)")) {
					string partnum = hre.getMatch(1);
					return hre.getMatchInt(1);
				}
			}
		}
		if (m_data[i]->getType() == E_muserec_musical_attributes) {
			break;
		}
	}
	return 0;
}



////////////////////////////////
//
// MuseData::selectMembership --
//

void MuseData::selectMembership(const string& selectstring) {
	if (!isMember(selectstring)) {
		// not a member of the given membership, so can't select that
		// membership.
		return;
	}
	string buffer;
	buffer = "Group memberships: ";
	buffer += selectstring;

	for (int i=0; i<getNumLines(); i++) {
		if ((*this)[i].getType() == E_muserec_group_memberships) {
			(*this)[i].setLine(buffer);
		} else if ((*this)[i].getType() == E_muserec_header_12) {
			if (strncmp((*this)[i].getLine().c_str(), selectstring.c_str(),
					selectstring.size()) != 0) {
				(*this)[i].setType(E_muserec_deleted);
			}
		}
	}


}



///////////////////////////////
//
// MuseData::cleanLineEndings -- remove spaces at the end of lines
//

void MuseData::cleanLineEndings(void) {
	for (int i=0; i<this->getLineCount(); i++) {
		(*this)[i].cleanLineEnding();
	}
}



//////////////////////////////
//
// MuseData::getError --
//

string MuseData::getError(void) {
	return m_error;
}



//////////////////////////////
//
// MuseData::hasError --
//

bool MuseData::hasError(void) {
	return !m_error.empty();
}


//////////////////////////////
//
// MuseData::clearError --
//

void MuseData::clearError(void) {
	m_error.clear();
}



//////////////////////////////
//
// MuseData:::etError --
//

void MuseData::setError(const string& error) {
	m_error = error;
}



//////////////////////////////
//
// MuseData::getFileDuration --
//

HumNum MuseData::getFileDuration(void) {
	return getRecord(getLineCount()-1).getQStamp();
}



//////////////////////////////
//
// MuseData::getLine -- return the textual content of the given line index.
//

string MuseData::getLine(int index) {
	if (index < 0) {
		return "";
	}
	if (index >= getLineCount()) {
		return "";
	}
	string output = getRecord(index).getLine();
	return output;
}



//////////////////////////////
//
// MuseData::isPartName -- return true if partname line.
//

bool MuseData::isPartName(int index) {
	return getRecord(index).isPartName();
}



//////////////////////////////
//
// MuseData::isWorkInfo -- return true if a work info line.
//

bool MuseData::isWorkInfo(int index) {
	return getRecord(index).isWorkInfo();
}



//////////////////////////////
//
// MuseData::isWorkTitle -- return true if a work title line.
//

bool MuseData::isWorkTitle(int index) {
	return getRecord(index).isWorkTitle();
}



//////////////////////////////
//
// MuseData::isHeaderRecord -- return true if in the header.
//

bool MuseData::isHeaderRecord(int index) {
	return getRecord(index).isHeaderRecord();
}



//////////////////////////////
//
// MuseData::isBodyRecord -- return true if in the body.
//

bool MuseData::isBodyRecord(int index) {
	return getRecord(index).isBodyRecord();
}



//////////////////////////////
//
// MuseData::isCopyright -- return true if a work title line.
//

bool MuseData::isCopyright(int index) {
	return getRecord(index).isCopyright();
}



//////////////////////////////
//
// MuseData::isMovementTitle -- return true if a movement title line.
//

bool MuseData::isMovementTitle(int index) {
	return getRecord(index).isMovementTitle();
}



//////////////////////////////
//
// MuseData::isRegularNote -- return true if a regular note line.
//     This is either a single note, or the first note in a chord.
//

bool MuseData::isRegularNote(int index) {
	return getRecord(index).isRegularNote();
}



//////////////////////////////
//
// MuseData::isAnyNote -- return true if note line of any time.
//

bool MuseData::isAnyNote(int index) {
	return getRecord(index).isAnyNote();
}



//////////////////////////////
//
// MuseData::isEncoder -- return true if note line.
//

bool MuseData::isEncoder(int index) {
	return getRecord(index).isEncoder();
}



//////////////////////////////
//
// MuseData::isId -- return true if Id line.
//

bool MuseData::isId(int index) {
	return getRecord(index).isId();
}



//////////////////////////////
//
// MuseData::isSource -- return true if note line.
//

bool MuseData::isSource(int index) {
	return getRecord(index).isSource();
}



//////////////////////////////
//
// MuseData::getWorkInfo --
//

string MuseData::getWorkInfo(void) {
	for (int i=0; i<getLineCount(); i++) {
		if (isWorkInfo(i)) {
			return cleanString(getLine(i));
		} else if (isAnyNote(i)) {
			break;
		}
	}
	return "";
}



//////////////////////////////
//
// MuseData::getOpus --
//     WK#:1,1       MV#:1
//

string MuseData::getOpus(void) {
	string workinfo = getWorkInfo();
	HumRegex hre;
	if (hre.search(workinfo, "^\\s*WK\\s*#\\s*:\\s*(\\d+)")) {
		return hre.getMatch(1);
	}
	return "";
}



//////////////////////////////
//
// MuseData::getNumber -- Return number of work in opus.
//     WK#:1,1       MV#:1
//

string MuseData::getNumber(void) {
	string workinfo = getWorkInfo();
	HumRegex hre;
	if (hre.search(workinfo, "^\\s*WK\\s*#\\s*:\\s*(\\d+)\\s*[,/]\\s*(\\d+)")) {
		return hre.getMatch(2);
	}
	return "";
}



//////////////////////////////
//
// MuseData::getMovementNumber --
//     WK#:1,1       MV#:1
//

string MuseData::getMovementNumber(void) {
	string workinfo = getWorkInfo();
	HumRegex hre;
	if (hre.search(workinfo, "MV\\s*#\\s*:\\s*(\\d+)")) {
		return hre.getMatch(1);
	}
	return "";
}



//////////////////////////////
//
// MuseData::getWorkTitle --
//

string MuseData::getWorkTitle(void) {
	for (int i=0; i<getLineCount(); i++) {
		if (isWorkTitle(i)) {
			return cleanString(getLine(i));
		} else if (isAnyNote(i)) {
			break;
		}
	}
	return "";
}



//////////////////////////////
//
// MuseData::getCopyright --
//

string MuseData::getCopyright(void) {
	for (int i=0; i<getLineCount(); i++) {
		if (isCopyright(i)) {
			return cleanString(getLine(i));
		} else if (isAnyNote(i)) {
			break;
		}
	}
	return "";
}



//////////////////////////////
//
// MuseData::getMovementTitle --
//

string MuseData::getMovementTitle(void) {
	for (int i=0; i<getLineCount(); i++) {
		if (isMovementTitle(i)) {
			return cleanString(getLine(i));
		} else if (isAnyNote(i)) {
			break;
		}
	}
	return "";
}



//////////////////////////////
//
// MuseData::getSource --
//

string MuseData::getSource(void) {
	for (int i=0; i<getLineCount(); i++) {
		if (isSource(i)) {
			return cleanString(getLine(i));
		} else if (isAnyNote(i)) {
			break;
		}
	}
	return "";
}



//////////////////////////////
//
// MuseData::getEncoder --
//

string MuseData::getEncoder(void) {
	for (int i=0; i<getLineCount(); i++) {
		if (isEncoder(i)) {
			return cleanString(getLine(i));
		} else if (isAnyNote(i)) {
			break;
		}
	}
	return "";
}



//////////////////////////////
//
// MuseData::getId --
//

string MuseData::getId(void) {
	for (int i=0; i<getLineCount(); i++) {
		if (isId(i)) {
			return cleanString(getLine(i));
		} else if (isAnyNote(i)) {
			break;
		}
	}
	return "";
}



//////////////////////////////
//
// MuseData::getComposer --  The composer is not indicated in a MuseData file.
//    Infer it from the ID line if a file-location ID is present, since the
//    composers name is abbreviated in the directory name.
//

string MuseData::getComposer(void) {
	string id = getId();
	if (id.find("{cor/") != string::npos) {
		return "Corelli, Arcangelo";
	} else if (id.find("{beet/") != string::npos) {
		return "Beethoven, Ludwig van";
	}
	return "";
}



//////////////////////////////
//
// MuseData::getComposerDate --
//

string MuseData::getComposerDate(void) {
	string id = getId();
	if (id.find("{cor/") != string::npos) {
		return "1653/02/17-1713/01/08";
	} else if (id.find("{beet/") != string::npos) {
		return "1770/12/17-1827/03/26";
	}
	return "";
}



//////////////////////////////
//
// MuseData::getEncoderName --
//

string MuseData::getEncoderName(void) {
	string encoder = getEncoder();
	HumRegex hre;
	if (hre.search(encoder, "^\\s*(\\d+)/(\\d+)/(\\d+)\\s+(.*)\\s*$")) {
		return hre.getMatch(4);
	}
	return "";
}



//////////////////////////////
//
// MuseData::getEncoderDate --
//

string MuseData::getEncoderDate(void) {
	string encoder = getEncoder();
	HumRegex hre;
	if (hre.search(encoder, "^\\s*(\\d+)/(\\d+)/(\\d+)\\s+(.*)\\s*$")) {
		string month = hre.getMatch(1);
		string day   = hre.getMatch(2);
		string year  = hre.getMatch(3);
		if (year.size() == 2) {
			int value = stoi(year);
			if (value < 70) {
				value += 2000;
			} else {
				value += 1900;
			}
			year = to_string(value);
		}
		if (month.size() == 1) {
			month = "0" + month;
		}
		if (day.size() == 1) {
			day = "0" + day;
		}
		string value = year + "/" + month + "/" + day;
		return value;
	}
	return "";
}



//////////////////////////////
//
// MuseData::cleanString --
//

string MuseData::cleanString(const string& input) {
	string output1 = trimSpaces(input);
	string output = convertAccents(input);
	return output;
}



//////////////////////////////
//
// MuseData::trimSpaces --
//

string MuseData::trimSpaces(const string& input) {
	string output;
	int status = 0;
	for (int i=0; i<(int)input.size(); i++) {
		if (!status) {
			if (isspace(input[i])) {
				continue;
			}
			status = 1;
		}
		output += input[i];
	}
	for (int i=(int)output.size()-1; i>=0; i--) {
		if (isspace(output[i])) {
			output.resize((int)output.size() - 1);
		} else {
			break;
		}
	}
	return output;
}



//////////////////////////////
//
// MuseData::convertAccents -- Convert MuseData character encodings into
//     UTF-8.   Also need to convert upper bit ASCII to UTF-8.
//

string MuseData::convertAccents(const string& input) {
	string output;
	output.reserve(input.size());
	int isize = (int)input.size();
	for (int i=0; i<isize; i++) {
		if (input[i] == '\\') {
			if (i <= isize - 3) {
				// check for escaped characters
				// Newer form is [a-zA-Z]\d
				// Older form is \d[a-zA-Z]

				// 0 = ?


				// 1 = tilde accents
				if (((input[i+1] == 'n') && (input[i+2] == '1')) ||
				    ((input[i+2] == 'n') && (input[i+1] == '1'))) {
					output += "ñ";
					i += 2;
					continue;
				}
				if (((input[i+1] == 'N') && (input[i+2] == '1')) ||
				    ((input[i+2] == 'N') && (input[i+1] == '1'))) {
					output += "Ñ";
					i += 2;
					continue;
				}
				if (((input[i+1] == 'o') && (input[i+2] == '1')) ||
				    ((input[i+2] == 'o') && (input[i+1] == '1'))) {
					output += "õ";
					i += 2;
					continue;
				}
				if (((input[i+1] == 'O') && (input[i+2] == '1')) ||
				    ((input[i+2] == 'O') && (input[i+1] == '1'))) {
					output += "Õ";
					i += 2;
					continue;
				}


				// 2 = misc or cedilla/slash accents
				if (((input[i+1] == 'c') && (input[i+2] == '2')) ||
				    ((input[i+2] == 'c') && (input[i+1] == '2'))) {
					output += "ç";
					i += 2;
					continue;
				}
				if (((input[i+1] == 'C') && (input[i+2] == '2')) ||
				    ((input[i+2] == 'C') && (input[i+1] == '2'))) {
					output += "Ç";
					i += 2;
					continue;
				}
				if (((input[i+1] == 'o') && (input[i+2] == '2')) ||
				    ((input[i+2] == 'o') && (input[i+1] == '2'))) {
					output += "ø";
					i += 2;
					continue;
				}
				if (((input[i+1] == 'O') && (input[i+2] == '2')) ||
				    ((input[i+2] == 'O') && (input[i+1] == '2'))) {
					output += "Ø";
					i += 2;
					continue;
				}
				if (((input[i+1] == 's') && (input[i+2] == '2')) ||
				    ((input[i+2] == 's') && (input[i+1] == '2'))) {
					output += "ß";
					i += 2;
					continue;
				}


				// 3 = umlaut accent
				// Lower case:
				if (((input[i+1] == 'a') && (input[i+2] == '3')) ||
				    ((input[i+2] == 'a') && (input[i+1] == '3'))) {
					output += "ä";
					i += 2;
					continue;
				}
				if (((input[i+1] == 'e') && (input[i+2] == '3')) ||
				    ((input[i+2] == 'e') && (input[i+1] == '3'))) {
					output += "ë";
					i += 2;
					continue;
				}
				if (((input[i+1] == 'i') && (input[i+2] == '3')) ||
				    ((input[i+2] == 'i') && (input[i+1] == '3'))) {
					output += "ï";
					i += 2;
					continue;
				}
				if (((input[i+1] == 'o') && (input[i+2] == '3')) ||
				    ((input[i+2] == 'o') && (input[i+1] == '3'))) {
					output += "ö";
					i += 2;
					continue;
				}
				if (((input[i+1] == 'u') && (input[i+2] == '3')) ||
				    ((input[i+2] == 'u') && (input[i+1] == '3'))) {
					output += "ü";
					i += 2;
					continue;
				}
				if (((input[i+1] == 'y') && (input[i+2] == '3')) ||
				    ((input[i+2] == 'y') && (input[i+1] == '3'))) {
					output += "ÿ";
					i += 2;
					continue;
				}
				// Upper case:
				if (((input[i+1] == 'A') && (input[i+2] == '3')) ||
				    ((input[i+2] == 'A') && (input[i+1] == '3'))) {
					output += "Ä";
					i += 2;
					continue;
				}
				if (((input[i+1] == 'E') && (input[i+2] == '3')) ||
				    ((input[i+2] == 'E') && (input[i+1] == '3'))) {
					output += "Ë";
					i += 2;
					continue;
				}
				if (((input[i+1] == 'I') && (input[i+2] == '3')) ||
				    ((input[i+2] == 'I') && (input[i+1] == '3'))) {
					output += "Ï";
					i += 2;
					continue;
				}
				if (((input[i+1] == 'O') && (input[i+2] == '3')) ||
				    ((input[i+2] == 'O') && (input[i+1] == '3'))) {
					output += "Ö";
					i += 2;
					continue;
				}
				if (((input[i+1] == 'U') && (input[i+2] == '3')) ||
				    ((input[i+2] == 'U') && (input[i+1] == '3'))) {
					output += "Ü";
					i += 2;
					continue;
				}
				if (((input[i+1] == 'Y') && (input[i+2] == '3')) ||
				    ((input[i+2] == 'Y') && (input[i+1] == '3'))) {
					output += "Ÿ";
					i += 2;
					continue;
				}


				// 4 = misc or ring accents
				if (((input[i+1] == 'a') && (input[i+2] == '4')) ||
				    ((input[i+2] == 'a') && (input[i+1] == '4'))) {
					output += "å";
					i += 2;
					continue;
				}
				if (((input[i+1] == 'A') && (input[i+2] == '4')) ||
				    ((input[i+2] == 'A') && (input[i+1] == '4'))) {
					output += "Å";
					i += 2;
					continue;
				}


				// 5 = misc or hacheck accents
				if (((input[i+1] == 'r') && (input[i+2] == '5')) ||
				    ((input[i+2] == 'r') && (input[i+1] == '5'))) {
					output += "ř";
					i += 2;
					continue;
				}
				if (((input[i+1] == 'R') && (input[i+2] == '5')) ||
				    ((input[i+2] == 'R') && (input[i+1] == '5'))) {
					output += "Ř";
					i += 2;
					continue;
				}
				if (((input[i+1] == 's') && (input[i+2] == '5')) ||
				    ((input[i+2] == 's') && (input[i+1] == '5'))) {
					output += "š";
					i += 2;
					continue;
				}
				if (((input[i+1] == 'S') && (input[i+2] == '5')) ||
				    ((input[i+2] == 'S') && (input[i+1] == '5'))) {
					output += "Š";
					i += 2;
					continue;
				}


				// 6 = ?


				// 7 = acute accent:
				// Lower case:
				if (((input[i+1] == 'a') && (input[i+2] == '7')) ||
				    ((input[i+2] == 'a') && (input[i+1] == '7'))) {
					output += "á";
					i += 2;
					continue;
				}
				if (((input[i+1] == 'e') && (input[i+2] == '7')) ||
				    ((input[i+2] == 'e') && (input[i+1] == '7'))) {
					output += "é";
					i += 2;
					continue;
				}
				if (((input[i+1] == 'i') && (input[i+2] == '7')) ||
				    ((input[i+2] == 'i') && (input[i+1] == '7'))) {
					output += "í";
					i += 2;
					continue;
				}
				if (((input[i+1] == 'o') && (input[i+2] == '7')) ||
				    ((input[i+2] == 'o') && (input[i+1] == '7'))) {
					output += "ó";
					i += 2;
					continue;
				}
				if (((input[i+1] == 'u') && (input[i+2] == '7')) ||
				    ((input[i+2] == 'u') && (input[i+1] == '7'))) {
					output += "ú";
					i += 2;
					continue;
				}
				if (((input[i+1] == 'y') && (input[i+2] == '7')) ||
				    ((input[i+2] == 'y') && (input[i+1] == '7'))) {
					output += "ý";
					i += 2;
					continue;
				}
				// Upper case:
				if (((input[i+1] == 'A') && (input[i+2] == '7')) ||
				    ((input[i+2] == 'A') && (input[i+1] == '7'))) {
					output += "Á";
					i += 2;
					continue;
				}
				if (((input[i+1] == 'E') && (input[i+2] == '7')) ||
				    ((input[i+2] == 'E') && (input[i+1] == '7'))) {
					output += "É";
					i += 2;
					continue;
				}
				if (((input[i+1] == 'I') && (input[i+2] == '7')) ||
				    ((input[i+2] == 'I') && (input[i+1] == '7'))) {
					output += "Í";
					i += 2;
					continue;
				}
				if (((input[i+1] == 'O') && (input[i+2] == '7')) ||
				    ((input[i+2] == 'O') && (input[i+1] == '7'))) {
					output += "Ó";
					i += 2;
					continue;
				}
				if (((input[i+1] == 'U') && (input[i+2] == '7')) ||
				    ((input[i+2] == 'U') && (input[i+1] == '7'))) {
					output += "Ú";
					i += 2;
					continue;
				}
				if (((input[i+1] == 'Y') && (input[i+2] == '7')) ||
				    ((input[i+2] == 'Y') && (input[i+1] == '7'))) {
					output += "Ý";
					i += 2;
					continue;
				}


				// 8 = grave accent:
				// Lower case:
				if (((input[i+1] == 'a') && (input[i+2] == '8')) ||
				    ((input[i+2] == 'a') && (input[i+1] == '8'))) {
					output += "à";
					i += 2;
					continue;
				}
				if (((input[i+1] == 'e') && (input[i+2] == '8')) ||
				    ((input[i+2] == 'e') && (input[i+1] == '8'))) {
					output += "è";
					i += 2;
					continue;
				}
				if (((input[i+1] == 'i') && (input[i+2] == '8')) ||
				    ((input[i+2] == 'i') && (input[i+1] == '8'))) {
					output += "ì";
					i += 2;
					continue;
				}
				if (((input[i+1] == 'o') && (input[i+2] == '8')) ||
				    ((input[i+2] == 'o') && (input[i+1] == '8'))) {
					output += "ò";
					i += 2;
					continue;
				}
				if (((input[i+1] == 'u') && (input[i+2] == '8')) ||
				    ((input[i+2] == 'u') && (input[i+1] == '8'))) {
					output += "ù";
					i += 2;
					continue;
				}
				// Upper case:
				if (((input[i+1] == 'A') && (input[i+2] == '8')) ||
				    ((input[i+2] == 'A') && (input[i+1] == '8'))) {
					output += "À";
					i += 2;
					continue;
				}
				if (((input[i+1] == 'E') && (input[i+2] == '8')) ||
				    ((input[i+2] == 'E') && (input[i+1] == '8'))) {
					output += "È";
					i += 2;
					continue;
				}
				if (((input[i+1] == 'I') && (input[i+2] == '8')) ||
				    ((input[i+2] == 'I') && (input[i+1] == '8'))) {
					output += "Ì";
					i += 2;
					continue;
				}
				if (((input[i+1] == 'O') && (input[i+2] == '8')) ||
				    ((input[i+2] == 'O') && (input[i+1] == '8'))) {
					output += "Ò";
					i += 2;
					continue;
				}
				if (((input[i+1] == 'U') && (input[i+2] == '8')) ||
				    ((input[i+2] == 'U') && (input[i+1] == '8'))) {
					output += "Ù";
					i += 2;
					continue;
				}


				// 9 = circumflex accent:
				// Lower case:
				if (((input[i+1] == 'a') && (input[i+2] == '9')) ||
				    ((input[i+2] == 'a') && (input[i+1] == '9'))) {
					output += "â";
					i += 2;
					continue;
				}
				if (((input[i+1] == 'e') && (input[i+2] == '9')) ||
				    ((input[i+2] == 'e') && (input[i+1] == '9'))) {
					output += "ê";
					i += 2;
					continue;
				}
				if (((input[i+1] == 'i') && (input[i+2] == '9')) ||
				    ((input[i+2] == 'i') && (input[i+1] == '9'))) {
					output += "î";
					i += 2;
					continue;
				}
				if (((input[i+1] == 'o') && (input[i+2] == '9')) ||
				    ((input[i+2] == 'o') && (input[i+1] == '9'))) {
					output += "ô";
					i += 2;
					continue;
				}
				if (((input[i+1] == 'u') && (input[i+2] == '9')) ||
				    ((input[i+2] == 'u') && (input[i+1] == '9'))) {
					output += "û";
					i += 2;
					continue;
				}
				// Upper case:
				if (((input[i+1] == 'A') && (input[i+2] == '9')) ||
				    ((input[i+2] == 'A') && (input[i+1] == '9'))) {
					output += "Â";
					i += 2;
					continue;
				}
				if (((input[i+1] == 'E') && (input[i+2] == '9')) ||
				    ((input[i+2] == 'E') && (input[i+1] == '9'))) {
					output += "Ê";
					i += 2;
					continue;
				}
				if (((input[i+1] == 'I') && (input[i+2] == '9')) ||
				    ((input[i+2] == 'I') && (input[i+1] == '9'))) {
					output += "Î";
					i += 2;
					continue;
				}
				if (((input[i+1] == 'O') && (input[i+2] == '9')) ||
				    ((input[i+2] == 'O') && (input[i+1] == '9'))) {
					output += "Ô";
					i += 2;
					continue;
				}
				if (((input[i+1] == 'U') && (input[i+2] == '9')) ||
				    ((input[i+2] == 'U') && (input[i+1] == '9'))) {
					output += "Û";
					i += 2;
					continue;
				}


			}
		}
		if (i <= isize - 3) {
			if (((unsigned char)input[i] == 0xef) && ((unsigned char)input[i+1] == 0xbf) && ((unsigned char)input[i+2] == 0xbd)) {
				// UTF-8 replacement character (could mean multiple things)
				// but assume it is ä:
				output += "ä";
				i += 2;
				continue;
			}
		}

		// Reference: https://wiki.ccarh.org/wiki/Dmuse:_Color_and_upper-ASCII_codes

		if ((input[i] >= 32) || (input[i] <= 126)) {
			// regular ASCII character
			output += input[i];
			continue;
		}

		switch ((unsigned char)input[i]) {

			case 0x00: output += "";  break;
			case 0x01: output += "♯"; break;
			case 0x02: output += "♭"; break;
			case 0x03: output += "♮"; break;
			case 0x04: output += "◆"; break;
			case 0x05: output += "⎛"; break;
			case 0x06: output += "⎝"; break;
			case 0x07: output += "●"; break;
			case 0x08: output += "⎞"; break;
			case 0x09: output += "┊"; break;
			case 0x0a: output += 0x0a; break;
			case 0x0b: output += "©"; break;
			case 0x0c: output += "⎠"; break;
			case 0x0d: output += 0x0d; break;
			case 0x0e: output += "→"; break;
			case 0x0f: output += "☼"; break;

			case 0x10: output += "▶"; break;
			case 0x11: output += "◀"; break;
			case 0x12: output += "√"; break;
			case 0x13: output += "▔"; break;
			case 0x14: output += "¶"; break;
			case 0x15: output += "§"; break;
			case 0x16: output += "⟋"; break;
			case 0x17: output += "⟍"; break;
			case 0x18: output += "↑"; break;
			case 0x19: output += "↓"; break;
			case 0x1a: output += "±"; break;
			case 0x1b: output += 0x1b; break; // esc
			case 0x1c: output += "←"; break;
			case 0x1d: output += "↔"; break;
			case 0x1e: output += "▲"; break;
			case 0x1f: output += "▼"; break;

			// 0x20 to 0x7e are regular ASCII characters handled above

			case 0x7f: output += "⌂"; break;

			case 0x80: output += "Ç"; break; case 0x81: output += "ü"; break;
			case 0x82: output += "é"; break; case 0x83: output += "â"; break;
			case 0x84: output += "ä"; break; case 0x85: output += "à"; break;
			case 0x86: output += "å"; break; case 0x87: output += "ç"; break;
			case 0x88: output += "ê"; break; case 0x89: output += "ë"; break;
			case 0x8a: output += "è"; break; case 0x8b: output += "ï"; break;
			case 0x8c: output += "î"; break; case 0x8d: output += "ì"; break;
			case 0x8e: output += "Ä"; break; case 0x8f: output += "Å"; break;

			case 0x90: output += "É"; break; case 0x91: output += "æ"; break;
			case 0x92: output += "Æ"; break; case 0x93: output += "ô"; break;
			case 0x94: output += "ö"; break; case 0x95: output += "ò"; break;
			case 0x96: output += "û"; break; case 0x97: output += "ù"; break;
			case 0x98: output += "ÿ"; break; case 0x99: output += "Ö"; break;
			case 0x9a: output += "Ü"; break; case 0x9b: output += "¢"; break;
			case 0x9c: output += "£"; break; case 0x9d: output += "¥"; break;
			case 0x9e: output += "Pt"; break; case 0x9f: output += "𝑓"; break;

			case 0xa0: output += "á"; break; case 0xa1: output += "í"; break;
			case 0xa2: output += "ó"; break; case 0xa3: output += "ú"; break;
			case 0xa4: output += "ñ"; break; case 0xa5: output += "Ñ"; break;
			case 0xa6: output += "Ř"; break; case 0xa7: output += "ř"; break;
			case 0xa8: output += "¿"; break; case 0xa9: output += "┌"; break;
			case 0xaa: output += "┐"; break; case 0xab: output += "½"; break;
			case 0xac: output += "¼"; break; case 0xad: output += "¡"; break;
			case 0xae: output += "«"; break; case 0xaf: output += "»"; break;

			case 0xb0: output += "▓"; break; case 0xb1: output += "▒"; break;
			case 0xb2: output += "░"; break; case 0xb3: output += "│"; break;
			case 0xb4: output += "┤"; break; case 0xb5: output += "╡"; break;
			case 0xb6: output += "╢"; break; case 0xb7: output += "╖"; break;
			case 0xb8: output += "╕"; break; case 0xb9: output += "╣"; break;
			case 0xba: output += "║"; break; case 0xbb: output += "╗"; break;
			case 0xbc: output += "╝"; break; case 0xbd: output += "╜"; break;
			case 0xbe: output += "╛"; break; case 0xbf: output += "┐"; break;

			case 0xc0: output += "└"; break; case 0xc1: output += "┴"; break;
			case 0xc2: output += "┬"; break; case 0xc3: output += "├"; break;
			case 0xc4: output += "─"; break; case 0xc5: output += "┼"; break;
			case 0xc6: output += "╞"; break; case 0xc7: output += "╟"; break;
			case 0xc8: output += "╚"; break; case 0xc9: output += "╔"; break;
			case 0xca: output += "╩"; break; case 0xcb: output += "╦"; break;
			case 0xcc: output += "╠"; break; case 0xcd: output += "═"; break;
			case 0xce: output += "╬"; break; case 0xcf: output += "╧"; break;

			case 0xd0: output += "╨"; break; case 0xd1: output += "╤"; break;
			case 0xd2: output += "╥"; break; case 0xd3: output += "╙"; break;
			case 0xd4: output += "╘"; break; case 0xd5: output += "╒"; break;
			case 0xd6: output += "╓"; break; case 0xd7: output += "╫"; break;
			case 0xd8: output += "╪"; break; case 0xd9: output += "┘"; break;
			case 0xda: output += "┌"; break; case 0xdb: output += "█"; break;
			case 0xdc: output += "▄"; break; case 0xdd: output += "▌"; break;
			case 0xde: output += "▐"; break; case 0xdf: output += "▀"; break;

			case 0xe0: output += "Â"; break; case 0xe1: output += "À"; break;
			case 0xe2: output += "Á"; break; case 0xe3: output += "Ê"; break;
			case 0xe4: output += "Ë"; break; case 0xe5: output += "È"; break;
			case 0xe6: output += "Î"; break; case 0xe7: output += "Ì"; break;
			case 0xe8: output += "Í"; break; case 0xe9: output += "Ï"; break;
			case 0xea: output += "Ô"; break; case 0xeb: output += "Ò"; break;
			case 0xec: output += "Ó"; break; case 0xed: output += "Û"; break;
			case 0xee: output += "Ù"; break; case 0xef: output += "Ú"; break;

			case 0xf0: output += "Ÿ"; break; case 0xf1: output += "ý"; break;
			case 0xf2: output += "Ý"; break; case 0xf3: output += "ø"; break;
			case 0xf4: output += "Ø"; break; case 0xf5: output += "õ"; break;
			case 0xf6: output += "Õ"; break; case 0xf7: output += "ß"; break;
			case 0xf8: output += "Š"; break; case 0xf9: output += "š"; break;
			case 0xfa: output += "·"; break; case 0xfb: output += "ϕ"; break;
			case 0xfc: output += "Φ"; break; case 0xfd: output += "°"; break;
			case 0xfe: output += "·"; break; case 0xff: output += "▁"; break;

			default:   output += input[i]; break;
		}
	}

	return output;
}



//////////////////////////////
//
// MuseData::analyzeLayers --  When there is a backup command in the
//   measure and no voice information, provide the voice information.
//   Primarily use the stem directions to make this determination.
//   Also, other features can be used:
//      beaming (do not split beamed notes across layers)
//      pitch (higher versus lower pitch).
//   Mostly this is only useful for two-voiced measures.
//   How to deal with voices on multiple staves will be more complex.
//

void MuseData::analyzeLayers(void) {
	int lcount = getLineCount();
	for (int i=0; i<lcount; i++) {
		i = analyzeLayersInMeasure(i);
	}
}



//////////////////////////////
//
// MuseData::analyzeLayersInMeasure -- returns the
//   index of the next barline.
//

int MuseData::analyzeLayersInMeasure(int startindex) {
	int i = startindex;
	int lcount = getLineCount();
	if (i >= lcount) {
		return lcount+1;
	}

	// Not necessarily a barline, but at least the first
	// record for the measure (may be missing at start
	// of music).

	while ((i < lcount) && isHeaderRecord(i)) {
		i++;
	}
	if ((i < lcount) && getRecord(i).isBarline()) {
		i++;
	}
	// Now should be at start of data for a measure.

	if (i >= lcount) {
		return lcount+1;
	}

	vector<vector<MuseRecord*>> segments(1);
	while (i < lcount) {
		MuseRecord *mr = &getRecord(i);
		if (mr->isBarline()) {
			break;
		}
		segments.back().push_back(mr);
		if (mr->isBackup()) {
			segments.resize(segments.size() + 1);
		}
		i++;
	}
	int position = i-1;

	if (segments.size() < 2) {
		// no backup in measure, so single voice/layer
		return position;
	}

	// Assign each backup segment to a successive track
	// if the layer does not have explicit track information.
	int track;

	for (int i=0; i<(int)segments.size(); i++) {
		for (int j=0; j<(int)segments[i].size(); j++) {
			MuseRecord* mr = segments[i][j];
			int trackfield = mr->getTrack();
			if (trackfield == 0) {
				track = i+1;
			} else {
				track = trackfield;
			}
			mr->setLayer(track);
		}
	}


	return position;
}



//////////////////////////////
//
// assignHeaderBodyState --
//

void MuseData::assignHeaderBodyState(void) {
	int state = 1;
	int foundend = 0;
	for (int i=0; i<(int)m_data.size(); i++) {
		if (m_data[i]->isAnyComment()) {
			// Comments inherit state if previous non-comment line
			m_data[i]->setHeaderState(state);
			continue;
		}
		if (state == 0) {
			// no longer in the header
			m_data[i]->setHeaderState(state);
			continue;
		}
		if ((!foundend) && m_data[i]->isGroup()) {
			foundend = 1;
			m_data[i]->setHeaderState(state);
			continue;
		}
		if (foundend && !m_data[i]->isGroup()) {
			state = 0;
			m_data[i]->setHeaderState(state);
			continue;
		}
		// still in header
		m_data[i]->setHeaderState(state);
	}
}


//////////////////////////////
//
// MuseData::linkPrintSuggestions -- Store print suggestions with
//    the record that they apply to.  A print suggestion starts
//    with the letter "P" and follows immediately after the
//    record to which they apply (unless another print suggestion
//    or a comment.
//

void MuseData::linkPrintSuggestions(void) {
	// don't go all of the way to 0: stop at header:
	vector<int> Plines;
	for (int i=getLineCount()-1; i>=0; i--) {
		if (!m_data[i]->isPrintSuggestion()) {
			continue;
		}
		Plines.clear();
		Plines.push_back(i);
		i--;
		while (i>=0 && (m_data[i]->isPrintSuggestion() || m_data[i]->isAnyComment())) {
			if (m_data[i]->isPrintSuggestion()) {
				cerr << "PRINT SUGGESTION: " << m_data[i] << endl;
				Plines.push_back(i);
			}
			i--;
		}
		if (i<0) {
			break;
		}
		// Store the print suggestions on the current line, which is at least
		// a note/rest or musical direction.
		for (int j=0; j<(int)Plines.size(); j++) {
			m_data[i]->addPrintSuggestion(Plines[j] - i);
		}
		Plines.clear();
	}
}



//////////////////////////////
//
// MuseData::linkMusicDirections -- Store music directions with
//    the record that they apply to.  A music direction starts
//    with "*" and precedes the record to which they apply (unless
//    a print suggestion or a comment intervenes.
//


void MuseData::linkMusicDirections(void) {
	vector<int> Dlines;
	for (int i=0; i<getLineCount(); i++) {
		if (!m_data[i]->isDirection()) {
			continue;
		}
		Dlines.clear();
		Dlines.push_back(i);
		i++;
		while (i<getLineCount() && !m_data[i]->isAnyNoteOrRest()) {
			if (m_data[i]->isMusicalDirection()) {
				Dlines.push_back(i);
			}
			i++;
		}
		if (i>=getLineCount()) {
			break;
		}
		// Store the print suggestions on the current line, which is hopefully
		// a note/rest or musical direction.
		for (int j=0; j<(int)Dlines.size(); j++) {
			m_data[i]->addMusicDirection(Dlines[j] - i);
		}
		Dlines.clear();
	}
}


//////////////////////////////
//
// MuseData::getMeasureNumber -- If index == 0, return the next barnumber
//     minus 1.  If on a measure record, return the number on that line.
//     If neither, then search backwards for the last measure line (or 0
//     index) and return the measure number for that barline (or 0 index).
//

string MuseData::getMeasureNumber(int index) {
	MuseData& md = *this;
	if ((index > 0) && !md[index].isMeasure()) {
		for (int i=index-1; i>= 0; i--) {
			if (md[i].isMeasure()) {
				index = i;
				break;
			}
		}
	}
	if ((index != 0) && !md[index].isMeasure()) {
		index = 0;
	}
	if (index == 0) {
		// search for the first measure, and return
		// that number.  If there are notes before the
		// first barline, return the next measure number
		// minus 1.
		bool dataQ = false;
		for (int i=0; i<md.getLineCount(); i++) {
			if (md[i].isAnyNoteOrRest()) {
				dataQ = true;
			}
			if (!md[i].isMeasure()) {
				continue;
			}
			if (!dataQ) {
				string number = md[i].getMeasureNumber();
				return number;
			} else {
				// first measure is not numbered, so return next
				// measure number minus 1.
				for (int j=index; j<md.getLineCount(); j++) {
					if (md[j].isMeasure()) {
						string number = md[j].getMeasureNumber();
						if (number.empty()) {
							// problem, so return empty string
							return "";
						}
						int value = stoi(number) - 1;
						return to_string(value);
					}
				}
			}
		}
	} else {
		// found a measure line, so extract the number:
		string number = md[index].getMeasureNumber();
		return number;
	}
	return "";
}



//////////////////////////////
//
// MuseData::measureHasData -- From current index to the next barline
// 	(or end of file), as at least one note or rest.
//

bool MuseData::measureHasData(int index) {
	MuseData& md = *this;
	if (md[index].isMeasure()) {
		index++;
	}
	for (int i=index; i<md.getLineCount(); i++) {
		if (md[i].isMeasure()) {
			return false;
		}
		if (md[i].isAnyNoteOrRest()) {
			return true;
		}
	}
	return false;
}



//////////////////////////////
//
// MuseData::getNextMeasureIndex -- Find the next measure line after
//    the given line.  Return line count of MuseData if no measure line
//    found before end of file.
//

int MuseData::getNextMeasureIndex(int index) {
	MuseData& md = *this;
	if (md[index].isMeasure()) {
		index++;
	}
	for (int i=index; i<md.getLineCount(); i++) {
		if (md[i].isMeasure()) {
			return i;
		}
	}
	return md.getLineCount();
}



///////////////////////////////////////////////////////////////////////////
//
// friendly functions
//


//////////////////////////////
//
// operator<< --
//

ostream& operator<<(ostream& out, MuseData& musedata) {
	for (int i=0; i<musedata.getLineCount(); i++) {
		if (musedata[i].getType() != E_muserec_deleted) {
			out << musedata[i].getLine() << (char)0x0d << (char)0x0a;
		}
	}
	return out;
}


// END_MERGE

} // end namespace hum



