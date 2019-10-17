//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Tue Jun 30 22:41:24 PDT 1998
// Last Modified: Tue Sep 24 06:47:55 PDT 2019
// Filename:      humlib/src/MuseRecord.cpp
// Web Address:   http://github.com/craigsapp/humlib/blob/master/src/MuseRecord.cpp
// Syntax:        C++11
// vim:           ts=3
//
// Description:   A class that stores one line of data for a Musedata file.
//
// To do: check on gracenotes/cuenotes with chord notes.
//

#include "Convert.h"
#include "MuseRecord.h"
#include "HumRegex.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <cctype>
#include <sstream>

using namespace std;

namespace hum {

// START_MERGE

#define E_unknown   (0x7fff)

//////////////////////////////
//
// MuseRecord::MuseRecord --
//

MuseRecord::MuseRecord(void) : MuseRecordBasic() { }
MuseRecord::MuseRecord(const string& aLine) : MuseRecordBasic(aLine) { }
MuseRecord::MuseRecord(MuseRecord& aRecord) : MuseRecordBasic(aRecord) { }



//////////////////////////////
//
// MuseRecord::~MuseRecord --
//

MuseRecord::~MuseRecord() {
	// do nothing
}



//////////////////////////////////////////////////////////////////////////
//
// functions that work with note records
//


//////////////////////////////
//
// MuseRecord::getNoteField -- returns the string containing the pitch,
//	accidental and octave characters.
//

string MuseRecord::getNoteField(void) {
	switch (getType()) {
		case E_muserec_note_regular:
			return extract(1, 4);
			break;
		case E_muserec_note_chord:
		case E_muserec_note_cue:
		case E_muserec_note_grace:
			return extract(2, 5);
			break;
		default:
			cerr << "Error: cannot use getNoteField function on line: "
			   << getLine() << endl;
	}
	return "";
}




//////////////////////////////
//
// MuseRecord::getOctave -- returns the first numeric character
//	in the note field of a MuseData note record
//

int MuseRecord::getOctave(void) {
	string recordInfo = getNoteField();
	int index = 0;
	while ((index < (int)recordInfo.size()) && !std::isdigit(recordInfo[index])) {
		index++;
	}
	if (index >= (int)recordInfo.size()) {
		cerr << "Error: no octave specification in note field: " << recordInfo
			  << endl;
		return 0;
	}
	return recordInfo[index] - '0';
}


string MuseRecord::getOctaveString(void) {
	string recordInfo = getNoteField();
	int index = 0;
	while ((index < (int)recordInfo.size()) && !std::isdigit(recordInfo[index])) {
		index++;
	}
	if (index >= (int)recordInfo.size()) {
		cerr << "Error: no octave specification in note field: " << recordInfo
			  << endl;
		return "";
	}
	string output;
	output += recordInfo[index];
	return output;
}



//////////////////////////////
//
// MuseRecord::getPitch -- int version returns the base40 representation
//

int MuseRecord::getPitch(void) {
	string recordInfo = getNoteField();
	return Convert::museToBase40(recordInfo);
}


string MuseRecord::getPitchString(void) {
	string output = getNoteField();
	int len = (int)output.size();
	int index = len-1;
	while (index >= 0 && output[index] == ' ') {
		output.resize(index);
		index--;
	}
	return output;
}



//////////////////////////////
//
// MuseRecord::getPitchClass -- returns the pitch without the octave information
//

int MuseRecord::getPitchClass(void) {
	return getPitch() % 40;
}


string MuseRecord::getPitchClassString(void) {
	string output = getNoteField();
	int index = 0;
	while ((index < (int)output.size()) &&  !std::isdigit(output[index])) {
		index++;
	}
	output.resize(index);
	return output;
}



//////////////////////////////
//
// MuseRecord::getAccidental -- int version return -2 for double flat,
//	-1 for flat, 0 for natural, +1 for sharp, +2 for double sharp
//

int MuseRecord::getAccidental(void) {
	string recordInfo = getNoteField();
	int output = 0;
	int index = 0;
	while ((index < (int)recordInfo.size()) && (index < 16)) {
		if (recordInfo[index] == 'f') {
			output--;
		} else if (recordInfo[index] == '#') {
			output++;
		}
		index++;
	}
	return output;
}


string MuseRecord::getAccidentalString(void) {
	string output;
	int type = getAccidental();
	switch (type) {
		case -2: output = "ff"; break;
		case -1: output =  "f"; break;
		case  0: output =   ""; break;
		case  1: output =  "#"; break;
		case  2: output = "##"; break;
		default:
			output = getNoteField();
			cerr << "Error: unknown type of accidental: " << output << endl;
			return "";
	}
	return output;
}



//////////////////////////////
//
// MuseRecord::getBase40 -- return the base40 pitch value of the data
// line.  Middle C set to 40 * 4 + 2;  Returns -100 for non-pitched items.
// (might have to update for note_cur_chord and note_grace_chord which
// do not exist yet.
//

int MuseRecord::getBase40(void) {
	switch (getType()) {
		case E_muserec_note_regular:
		case E_muserec_note_chord:
		case E_muserec_note_cue:
		case E_muserec_note_grace:
			break;
		default:
			return -100;
	}
	return getPitch();
}



//////////////////////////////
//
// MuseRecord::setStemDown --
//

void MuseRecord::setStemDown(void) {
	getColumn(23) = 'd';
}



//////////////////////////////
//
// MuseRecord::setStemUp --
//

void MuseRecord::setStemUp(void) {
	getColumn(23) = 'u';
}



//////////////////////////////
//
// MuseRecord::setPitch -- input is a base40 value which gets converted
// to a diatonic pitch name.
//    Default value: chordnote = 0
//    Default value: gracenote = 0
//

void MuseRecord::setPitch(int base40, int chordnote, int gracenote) {
	string diatonic;
	switch (Convert::base40ToDiatonic(base40) % 7) {
		case 0:  diatonic = 'C'; break;
		case 1:  diatonic = 'D'; break;
		case 2:  diatonic = 'E'; break;
		case 3:  diatonic = 'F'; break;
		case 4:  diatonic = 'G'; break;
		case 5:  diatonic = 'A'; break;
		case 6:  diatonic = 'B'; break;
		default: diatonic = 'X';
	}

	string octave;
	octave  += char('0' + base40 / 40);

	string accidental;
	int acc = Convert::base40ToAccidental(base40);
	switch (acc) {
		case -2:   accidental = "ff"; break;
		case -1:   accidental = "f";  break;
		case +1:   accidental = "#";  break;
		case +2:   accidental = "##"; break;
	}
	string pitchname = diatonic + accidental + octave;

	if (chordnote) {
		if (gracenote) {
			setGraceChordPitch(pitchname);
		} else {
			setChordPitch(pitchname);
		}
	} else {
		setPitch(pitchname);
	}
}


void MuseRecord::setChordPitch(const string& pitchname) {
	getColumn(1) = ' ';
	setPitchAtIndex(1, pitchname);
}

void MuseRecord::setGracePitch(const string& pitchname) {
	getColumn(1) = 'g';
	setPitchAtIndex(1, pitchname);
}

void MuseRecord::setGraceChordPitch(const string& pitchname) {
	getColumn(1) = 'g';
	getColumn(2) = ' ';
	setPitchAtIndex(2, pitchname);
}

void MuseRecord::setCuePitch(const string& pitchname) {
	getColumn(1) = 'c';
	setPitchAtIndex(1, pitchname);
}


void MuseRecord::setPitch(const string& pitchname) {
	int start = 0;
	// If the record is already set to a grace note or a cue note,
	// then place pitch information starting at column 2 (index 1).
	if ((getColumn(1) == 'g') || (getColumn(1) == 'c')) {
		start = 1;
	}
	setPitchAtIndex(start, pitchname);
}


void MuseRecord::setPitchAtIndex(int index, const string& pitchname) {
	int len = (int)pitchname.size();
	if ((len > 4) && (pitchname != "irest")) {
		cerr << "Error in MuseRecord::setPitchAtIndex: " << pitchname << endl;
		return;
	}
	insertString(index+1, pitchname);

	// Clear any text fields not used by current pitch data.
	for (int i=4-len-1; i>=0; i--) {
		(*this)[index + len + i] = ' ';
	}
}



//////////////////////////////
//
// MuseRecord::getTickDurationField -- returns the string containing the
//      duration, and tie information.
//

string MuseRecord::getTickDurationField(void) {
	switch (getType()) {
		case E_muserec_figured_harmony:
		case E_muserec_note_regular:
		case E_muserec_note_chord:
		case E_muserec_rest:
		case E_muserec_backward:
		case E_muserec_forward:
			return extract(6, 9);
			break;
		// these record types do not have duration, per se:
		case E_muserec_note_cue:
		case E_muserec_note_grace:
		default:
			return "    ";
			// cerr << "Error: cannot use getTickDurationField function on line: "
			//      << getLine() << endl;
			// return "";
	}
	return "";
}



//////////////////////////////
//
// MuseRecord::getTickDurationString -- returns the string containing the duration,
//

string MuseRecord::getTickDurationString(void) {
	string output = getTickDurationField();
	int length = (int)output.size();
	int i = length - 1;
	while (i>0 && (output[i] == '-' || output[i] == ' ')) {
		output.resize(i);
		i--;
		length--;
	}

	int start = 0;
	while (output[start] == ' ') {
		start++;
	}

	if (start != 0) {
		for (i=0; i<length-start; i++) {
			output[i] = output[start+i];
		}
	}
	output.resize(length-start);

	return output;
}



//////////////////////////////
//
// MuseRecord::getTickDuration -- return the tick value found
//    in columns 6-8 in some data type, returning 0 if the record
//    type does not have a duration field.
//

int MuseRecord::getTickDuration(void) {
	string recordInfo = getTickDurationString();
	if (recordInfo.empty()) {
		return 0;
	}
	return std::stoi(recordInfo);
}



//////////////////////////////
//
// MuseRecord::getLineTickDuration -- returns the logical duration of the
//      data line.  Supresses the duration field of secondary chord notes.
//

int MuseRecord::getLineTickDuration(void) {
	if (getType() == E_muserec_note_chord) {
		return 0;
	}

	string recordInfo = getTickDurationString();
	if (recordInfo.empty()) {
		return 0;
	}
	int value = std::stoi(recordInfo);
	if (getType() == E_muserec_backspace) {
		return -value;
	}

	return value;
}



//////////////////////////////
//
// MuseRecord::getTicks -- similar to getLineTickDuration, but is non-zero
//    for secondary chord notes.
//

int MuseRecord::getTicks(void) {
	string recordInfo = getTickDurationString();
	if (recordInfo.empty()) {
		return 0;
	}
	int value = std::stoi(recordInfo);
	if (getType() == E_muserec_backspace) {
		return -value;
	}

	return value;
}


//////////////////////////////
//
// MuseRecord::getNoteTickDuration -- Similar to getLineTickDuration,
//    but do not suppress the duration of secondary chord-tones.
//

int MuseRecord::getNoteTickDuration(void) {
	string recordInfo = getTickDurationString();
	int value = 0;
	if (recordInfo.empty()) {
		return value;
	}
	value = std::stoi(recordInfo);
	if (getType() == E_muserec_backspace) {
		return -value;
	}
	return value;
}



//////////////////////////////
//
// MuseRecord::setDots --
//

void MuseRecord::setDots(int value) {
	switch (value) {
		case 0: getColumn(18) = ' ';   break;
		case 1: getColumn(18) = '.';   break;
		case 2: getColumn(18) = ':';   break;
		case 3: getColumn(18) = ';';   break;
		case 4: getColumn(18) = '!';   break;
		default: cerr << "Error in MuseRecord::setDots : " << value << endl;
	}
}



//////////////////////////////
//
// MuseRecord::getDotCount --
//

int MuseRecord::getDotCount(void) {
	char value = getColumn(18);
	switch (value) {
		case ' ': return 0;
		case '.': return 1;
		case ':': return 2;
		case ';': return 3;
		case '!': return 4;
	}
	return 0;
}



//////////////////////////////
//
// MuseRecord::setNoteheadShape -- Duration with augmentation dot component
//      removed.  Duration of 1 is quarter note.
//

void MuseRecord::setNoteheadShape(HumNum duration) {
	HumNum  note8th(1,2);
	HumNum  note16th(1,4);
	HumNum  note32nd(1,8);
	HumNum  note64th(1,16);
	HumNum  note128th(1,32);
	HumNum  note256th(1,64);

	if (duration > 16) {                 // maxima
		setNoteheadMaxima();
	} else if (duration > 8) {           // long
		setNoteheadLong();
	} else if (duration > 4) {           // breve
		if (m_roundBreve) {
			setNoteheadBreveRound();
		} else {
			setNoteheadBreve();
		}
	} else if (duration > 2) {           // whole note
		setNoteheadWhole();
	} else if (duration > 1) {           // half note
		setNoteheadHalf();
	} else if (duration > note8th) {     // quarter note
		setNoteheadQuarter();
	} else if (duration > note16th) {    // eighth note
		setNotehead8th();
	} else if (duration > note32nd) {    // 16th note
		setNotehead16th();
	} else if (duration > note64th) {    // 32nd note
		setNotehead32nd();
	} else if (duration > note128th) {   // 64th note
		setNotehead64th();
	} else if (duration > note256th) {   // 128th note
		setNotehead128th();
	} else if (duration == note256th) {  // 256th note
		// not allowing tuplets on the 256th note level.
		setNotehead256th();
	} else {
		cerr << "Error in duration: " << duration << endl;
		return;
	}
}



//////////////////////////////
//
// MuseRecord::setNoteheadShape -- Duration with augmentation dot component
//      removed.  Duration of 1 is quarter note.
//

void MuseRecord::setNoteheadShapeMensural(HumNum duration) {
	HumNum note8th(1, 2);
	HumNum note16th(1, 4);
	HumNum note32th(1, 8);
	HumNum note64th(1, 16);
	HumNum note128th(1, 32);
	HumNum note256th(1, 64);

	if (duration > 16) {                 // maxima
		setNoteheadMaxima();
	} else if (duration > 8) {           // long
		setNoteheadLong();
	} else if (duration > 4) {           // breve
		setNoteheadBreve();
	} else if (duration > 2) {           // whole note
		setNoteheadWholeMensural();
	} else if (duration > 1) {           // half note
		setNoteheadHalfMensural();
	} else if (duration > note8th) {     // quarter note
		setNoteheadQuarterMensural();
	} else if (duration > note16th) {    // eighth note
		setNotehead8thMensural();
	} else if (duration > note32th) {    // 16th note
		setNotehead16thMensural();
	} else if (duration > note64th) {    // 32nd note
		setNotehead32ndMensural();
	} else if (duration > note128th) {   // 64th note
		setNotehead64thMensural();
	} else if (duration > note256th) {   // 128th note
		setNotehead128thMensural();
	} else if (duration >= note256th) {  // 256th note
		// don't allow tuplets on 256th note level.
		setNotehead256thMensural();
	} else {
		cerr << "Error in duration: " << duration << endl;
		return;
	}
}

void MuseRecord::setNoteheadMaxima(void) {
	if ((*this)[0] == 'c' || ((*this)[0] == 'g')) {
		cerr << "Error: cue/grace notes cannot be maximas in setNoteheadLong"
			  << endl;
		return;
	} else {
		getColumn(17) = 'M';
	}
}

void MuseRecord::setNoteheadLong(void) {
	if ((*this)[0] == 'c' || ((*this)[0] == 'g')) {
		cerr << "Error: cue/grace notes cannot be longs in setNoteheadLong"
			  << endl;
		return;
	} else {
		getColumn(17) = 'L';
	}
}

void MuseRecord::setNoteheadBreve(void) {
	setNoteheadBreveSquare();
}

void MuseRecord::setNoteheadBreveSquare(void) {
	if ((*this)[0] == 'g') {        // grace note
		getColumn(8) = 'A';
	} else if ((*this)[0] == 'c') { // cue-sized note (with duration)
		getColumn(17) = 'A';
	} else {                        // normal note
		getColumn(17) = 'B';
	}
}

void MuseRecord::setNoteheadBreveRound(void) {
	if ((*this)[0] == 'g') {        // grace note
		getColumn(8) = 'A';
	} else if ((*this)[0] == 'c') { // cue-sized note (with duration)
		getColumn(17) = 'A';
	} else {                        // normal note
		getColumn(17) = 'b';
	}
}

void MuseRecord::setNoteheadBreveMensural(void) {
	setNoteheadBreveSquare();
}

void MuseRecord::setNoteheadWhole(void) {
	if ((*this)[0] == 'g') {        // grace note
		getColumn(8) = '9';
	} else if ((*this)[0] == 'c') { // cue-sized note (with duration)
		getColumn(17) = '9';
	} else {                        // normal note
		getColumn(17) = 'w';
	}
}

void MuseRecord::setNoteheadWholeMensural(void) {
	if ((*this)[0] == 'g') {        // grace note
		getColumn(8) = '9';
	} else if ((*this)[0] == 'c') { // cue-sized note (with duration)
		getColumn(17) = '9';
	} else {                        // normal note
		getColumn(17) = 'W';
	}
}

void MuseRecord::setNoteheadHalf(void) {
	if ((*this)[0] == 'g') {        // grace note
		getColumn(8) = '8';
	} else if ((*this)[0] == 'c') { // cue-sized note (with duration)
		getColumn(17) = '8';
	} else {                        // normal note
		getColumn(17) = 'h';
	}
}

void MuseRecord::setNoteheadHalfMensural(void) {
	if ((*this)[0] == 'g') {        // grace note
		getColumn(8) = '8';
	} else if ((*this)[0] == 'c') { // cue-sized note (with duration)
		getColumn(17) = '8';
	} else {                        // normal note
		getColumn(17) = 'H';
	}
}

void MuseRecord::setNoteheadQuarter(void) {
	if ((*this)[0] == 'g') {        // grace note
		getColumn(8) = '7';
	} else if ((*this)[0] == 'c') { // cue-sized note (with duration)
		getColumn(17) = '7';
	} else {                        // normal note
		getColumn(17) = 'q';
	}
}

void MuseRecord::setNoteheadQuarterMensural(void) {
	if ((*this)[0] == 'g') {        // grace note
		getColumn(8) = '7';
	} else if ((*this)[0] == 'c') { // cue-sized note (with duration)
		getColumn(17) = '7';
	} else {                        // normal note
		getColumn(17) = 'Q';
	}
}

void MuseRecord::setNotehead8th(void) {
	if ((*this)[0] == 'g') {        // grace note
		getColumn(8) = '6';
	} else if ((*this)[0] == 'c') { // cue-sized note (with duration)
		getColumn(17) = '6';
	} else {                        // normal note
		getColumn(17) = 'e';
	}
}

void MuseRecord::setNotehead8thMensural(void) {
	if ((*this)[0] == 'g') {        // grace note
		getColumn(8) = '6';
	} else if ((*this)[0] == 'c') { // cue-sized note (with duration)
		getColumn(17) = '6';
	} else {                        // normal note
		getColumn(17) = 'E';
	}
}

void MuseRecord::setNotehead16th(void) {
	if ((*this)[0] == 'g') {        // grace note
		getColumn(8) = '5';
	} else if ((*this)[0] == 'c') { // cue-sized note (with duration)
		getColumn(17) = '5';
	} else {                        // normal note
		getColumn(17) = 's';
	}
}

void MuseRecord::setNotehead16thMensural(void) {
	if ((*this)[0] == 'g') {        // grace note
		getColumn(8) = '5';
	} else if ((*this)[0] == 'c') { // cue-sized note (with duration)
		getColumn(17) = '5';
	} else {                        // normal note
		getColumn(17) = 'S';
	}
}

void MuseRecord::setNotehead32nd(void) {
	if ((*this)[0] == 'g') {        // grace note
		getColumn(8) = '4';
	} else if ((*this)[0] == 'c') { // cue-sized note (with duration)
		getColumn(17) = '4';
	} else {                        // normal note
		getColumn(17) = 't';
	}
}

void MuseRecord::setNotehead32ndMensural(void) {
	if ((*this)[0] == 'g') {        // grace note
		getColumn(8) = '4';
	} else if ((*this)[0] == 'c') { // cue-sized note (with duration)
		getColumn(17) = '4';
	} else {                        // normal note
		getColumn(17) = 'T';
	}
}

void MuseRecord::setNotehead64th(void) {
	if ((*this)[0] == 'g') {        // grace note
		getColumn(8) = '3';
	} else if ((*this)[0] == 'c') { // cue-sized note (with duration)
		getColumn(17) = '3';
	} else {                        // normal note
		getColumn(17) = 'x';
	}
}

void MuseRecord::setNotehead64thMensural(void) {
	if ((*this)[0] == 'g') {        // grace note
		getColumn(8) = '3';
	} else if ((*this)[0] == 'c') { // cue-sized note (with duration)
		getColumn(17) = '3';
	} else {                        // normal note
		getColumn(17) = 'X';
	}
}

void MuseRecord::setNotehead128th(void) {
	if ((*this)[0] == 'g') {        // grace note
		getColumn(8) = '2';
	} else if ((*this)[0] == 'c') { // cue-sized note (with duration)
		getColumn(17) = '2';
	} else {                        // normal note
		getColumn(17) = 'y';
	}
}

void MuseRecord::setNotehead128thMensural(void) {
	if ((*this)[0] == 'g') {        // grace note
		getColumn(8) = '2';
	} else if ((*this)[0] == 'c') { // cue-sized note (with duration)
		getColumn(17) = '2';
	} else {                        // normal note
		getColumn(17) = 'Y';
	}
}

void MuseRecord::setNotehead256th(void) {
	if ((*this)[0] == 'g') {        // grace note
		getColumn(8) = '1';
	} else if ((*this)[0] == 'c') { // cue-sized note (with duration)
		getColumn(17) = '1';
	} else {                        // normal note
		getColumn(17) = 'z';
	}
}

void MuseRecord::setNotehead256thMensural(void) {
	if ((*this)[0] == 'g') {        // grace note
		getColumn(8) = '1';
	} else if ((*this)[0] == 'c') { // cue-sized note (with duration)
		getColumn(17) = '1';
	} else {                        // normal note
		getColumn(17) = 'Z';
	}
}


/////////////////////////////
//
// MuseRecord::setBack --
//

void MuseRecord::setBack(int value) {
	insertString(1, "back");
	setTicks(value);
}



/////////////////////////////
//
// MuseRecord::setTicks -- return the numeric value in columns 6-9.
//

void MuseRecord::setTicks(int value) {
	if ((value < 0) || (value >= 1000)) {
		cerr << "@ Error: ticks out of range in MuseRecord::setTicks" << endl;
	}
	stringstream ss;
	ss << value;
	int len = (int)ss.str().size();
	insertString(5+3-len+1, ss.str());
}



//////////////////////////////
//
// MuseRecord::getTie --
//

string MuseRecord::getTieString(void) {
	string output;
	output += getColumn(9);
	if (output == " ") {
		output = "";
	}
	return output;
}


int MuseRecord::getTie(void) {
	return tieQ();
}


//////////////////////////////
//
// MuseRecord::getTie -- Set a tie marker in column 9.  Currently
// the function does not check the type of data, so will overr-write any
// data found in column 9 (such as if the record is not for a note).
//
// If the input parameter hidden is true, then the visual tie is not
// displayed, but the sounding tie is displayed.
//

int MuseRecord::setTie(int hidden) {
	getColumn(9) = '-';
	if (!hidden) {
		return addAdditionalNotation('-');
	} else {
		return -1;
	}
}



//////////////////////////////
//
// MuseRecord::addAdditionalNotation -- ties, slurs and tuplets.
//    Currently not handling editorial levels.
//

int MuseRecord::addAdditionalNotation(char symbol) {
	// search columns 32 to 43 for the specific symbol.
	// if it is found, then don't add.  If it is not found,
	// then do add.
	int i;
	int blank = -1;
	int nonempty = 0;  // true if a non-space character was found.

	for (i=43; i>=32; i--) {
		if (getColumn(i) == symbol) {
			return i;
		} else if (!nonempty && (getColumn(i) == ' ')) {
			blank = i;
		} else {
			nonempty = i;
		}
	}

	if (symbol == '-') {
	  // give preferential treatment to placing only ties in
	  // column 32
	  if (getColumn(32) == ' ') {
		  getColumn(32) = '-';
		  return 32;
	  }
	}

	if (blank < 0) {
		cerr << "Error in MuseRecord::addAdditionalNotation: "
			  << "no empty space for notation" << endl;
		return 0;
	}

	if ((blank <= 32) && (getColumn(33) == ' ')) {
		// avoid putting non-tie items in column 32.
		blank = 33;
	}

	getColumn(blank) = symbol;
	return blank;
}


// add a multi-character additional notation (such as a dynamic like mf):

int MuseRecord::addAdditionalNotation(const string& symbol) {
	int len = (int)symbol.size();
	// search columns 32 to 43 for the specific symbol.
	// if it is found, then don't add.  If it is not found,
	// then do add.
	int i, j;
	int blank = -1;
	int found = 0;
	int nonempty = 0;  // true if a non-space character was found.

	for (i=43-len; i>=32; i--) {
		found = 1;
		for (j=0; j<len; j++) {
			if (getColumn(i+j) != symbol[j]) {
				found = 0;
				break;
			}
		}
		if (found) {
			return i;
		} else if (!nonempty && (getColumn(i) == ' ')) {
// cout << "@COLUMN " << i << " is blank: " << getColumn(i) << endl;
			blank = i;
			// should check that there are enough blank lines to the right
			// as well...
		} else if (getColumn(i) != ' ') {
			nonempty = i;
		}
	}

	if (blank < 0) {
		cerr << "Error in MuseRecord::addAdditionalNotation2: "
			  << "no empty space for notation" << endl;
		return 0;
	}

// cout << "@ GOT HERE symbol = " << symbol << " and blank = " << blank << endl;
	if ((blank <= 32) && (getColumn(33) == ' ')) {
		// avoid putting non-tie items in column 32.
		blank = 33;
		// not worrying about overwriting something to the right
		// of column 33 since the empty spot was checked starting
		// on the right and moving towards the left.
	}
// cout << "@COLUMN 33 = " << getColumn(33) << endl;
// cout << "@ GOT HERE symbol = " << symbol << " and blank = " << blank << endl;

	for (j=0; j<len; j++) {
		getColumn(blank+j) = symbol[j];
	}
	return blank;
}



//////////////////////////////
//
// MuseRecord::tieQ -- returns true if the current line contains
//   a tie to a note in the future.  Does not check if there is a tie
//   to a note in the past.
//

int MuseRecord::tieQ(void) {
	int output = 0;
	switch (getType()) {
		case E_muserec_note_regular:
		case E_muserec_note_chord:
		case E_muserec_note_cue:
		case E_muserec_note_grace:
			if (getColumn(9) == '-') {
				output = 1;
			} else if (getColumn(9) == ' ') {
				output = 0;
			} else {
				output = -1;
			}
			break;
		default:
			return 0;
	}

	return output;
}


//////////////////////////////////////////////////////////////////////////
//
// graphical and intrepretive information for notes
//

//////////////////////////////
//
// MuseRecord::getFootnoteFlagField -- returns column 13 value
//

string MuseRecord::getFootnoteFlagField(void) {
	allowFigurationAndNotesOnly("getFootnoteField");
	return extract(13, 13);
}



//////////////////////////////
//
// MuseRecord::getFootnoteFlagString --
//

string MuseRecord::getFootnoteFlagString(void) {
	string output = getFootnoteFlagField();
	if (output[0] == ' ') {
		output = "";
	}
	return output;
}



//////////////////////////////
//
// MuseRecord::getFootnoteFlag --
//

int MuseRecord::getFootnoteFlag(void) {
	int output = 0;
	string recordInfo = getFootnoteFlagString();
	if (recordInfo[0] == ' ') {
		output = -1;
	} else {
		output = std::strtol(recordInfo.c_str(), NULL, 36);
	}
	return output;
}



//////////////////////////////
//
// MuseRecord::footnoteFlagQ --
//

int MuseRecord::footnoteFlagQ(void) {
	int output = 0;
	string recordInfo = getFootnoteFlagField();
	if (recordInfo[0] == ' ') {
		output = 0;
	} else {
		output = 1;
	}
	return output;
}



//////////////////////////////
//
// MuseRecord::getLevelField -- return column 14
//

string MuseRecord::getLevelField(void) {
	allowFigurationAndNotesOnly("getLevelField");
	return extract(14, 14);
}



//////////////////////////////
//
// MuseRecord::getLevel --
//

string MuseRecord::getLevelString(void) {
	string output = getLevelField();
	if (output[0] == ' ') {
		output = "";
	}
	return output;
}


int MuseRecord::getLevel(void) {
	int output = 1;
	string recordInfo = getLevelField();
	if (recordInfo[0] == ' ') {
		output = 1;
	} else {
		output = std::strtol(recordInfo.c_str(), NULL, 36);
	}
	return output;
}



//////////////////////////////
//
// MuseRecord::levelQ --
//

int MuseRecord::levelQ(void) {
	int output = 0;
	string recordInfo = getLevelField();
	if (recordInfo[0] == ' ') {
		output = 0;
	} else {
		output = 1;
	}
	return output;
}



//////////////////////////////
//
// MuseRecord::getTrackField -- return column 15
//

string MuseRecord::getTrackField(void) {
	if (!isAnyNoteOrRest()) {
		return extract(15, 15);
	} else {
		return " ";
	}
}



//////////////////////////////
//
// MuseRecord::getTrackString --
//

string MuseRecord::getTrackString(void) {
	string output = getTrackField();
	if (output[0] == ' ') {
		output = "";
	}
	return output;
}



//////////////////////////////
//
// MuseRecord::getTrack -- Return 0 if no track information (implicitly track 1,
//     or unlabelled higher track).
//

int MuseRecord::getTrack(void) {
	int output = 1;
	string recordInfo = getTrackField();
	if (recordInfo[0] == ' ') {
		output = 0;
	} else {
		output = std::strtol(recordInfo.c_str(), NULL, 36);
	}
	return output;
}



//////////////////////////////
//
// MuseRecord::trackQ --
//

int MuseRecord::trackQ(void) {
	int output = 0;
	string recordInfo = getTrackField();
	if (recordInfo[0] == ' ') {
		output = 0;
	} else {
		output = 1;
	}

	return output;
}



//////////////////////////////
//
// MuseRecord::getGraphicNoteTypeField -- return column 17
//

string MuseRecord::getGraphicNoteTypeField(void) {
// allowNotesOnly("getGraphicNoteTypefield");
	if (getLength() < 17) {
		return " ";
	} else {
		return extract(17, 17);
	}
}



//////////////////////////////
//
// MuseRecord::getGraphicNoteType --
//

string MuseRecord::getGraphicNoteTypeString(void) {
	string output = getGraphicNoteTypeField();
	if (output[0] == ' ') {
		output = "";
	}
	return output;
}



//////////////////////////////
//
// MuseRecord::getGraphicRecip --
//

string MuseRecord::getGraphicRecip(void) {
	int notetype = getGraphicNoteType();
	string output;
	switch (notetype) {
		case -3: output = "0000"; break;  // double-maxima
		case -2: output = "000"; break;   // maxima
		case -1: output = "00"; break;    // long
		default:
			output = to_string(notetype);  // regular **recip number
	}
	int dotcount = getDotCount();
	for (int i=0; i<dotcount; i++) {
		output += '.';
	}
	return output;
}



//////////////////////////////
//
// MuseRecord::getGraphicNoteType --
//

int MuseRecord::getGraphicNoteType(void) {
	int output = 0;
	string recordInfo = getGraphicNoteTypeField();
	if (recordInfo[0] == ' ') {
		if (isInvisibleRest()) {
			// invisible rests do not have graphic note types
			// so make one up from the logical note type
			HumNum value = getTickDuration();
			value /= getTpq();
			if (value >= 32) {
				return -2;
			} else if (value >= 16) {
				return -1;
			} else if (value >= 8) {
				return 0;
			} else if (value >= 4) {
				return 1;
			} else if (value >= 2) {
				return 2;
			} else if (value >= 1) {
				return 4;
			} else if (value.getFloat() >= 0.5) {
				return 8;
			} else if (value.getFloat() >= 0.25) {
				return 16;
			} else if (value.getFloat() >= 0.125) {
				return 32;
			} else if (value.getFloat() >= 0.0625) {
				return 64;
			} else if (value.getFloat() >= 1.0/128) {
				return 128;
			} else if (value.getFloat() >= 1.0/256) {
				return 256;
			} else if (value.getFloat() >= 1.0/512) {
				return 512;
			} else {
				return 0;
			}
		} else {
			cerr << "Error: no graphic note type specified: " << getLine() << endl;
			return 0;
		}
	}

	switch (recordInfo[0]) {
		case 'M':                          // Maxima
			output = -2;           break;
		case 'L':   case 'B':              // Longa
			output = -1;           break;
		case 'b':   case 'A':              // Breve
			output = 0;            break;
		case 'w':   case '9':              // Whole
			output = 1;            break;
		case 'h':   case '8':              // Half
			output = 2;            break;
		case 'q':   case '7':              // Quarter
			output = 4;            break;
		case 'e':   case '6':              // Eighth
			output = 8;            break;
		case 's':   case '5':              // Sixteenth
			output = 16;           break;
		case 't':   case '4':              // 32nd note
			output = 32;           break;
		case 'x':   case '3':              // 64th note
			output = 64;           break;
		case 'y':   case '2':              // 128th note
			output = 128;          break;
		case 'z':   case '1':              // 256th note
			output = 256;          break;
		default:
			cerr << "Error: unknown graphical note type in column 17: "
				  << getLine() << endl;
	}

	return output;
}


//////////////////////////////
//
// MuseRecord::graphicNoteTypeQ --
//

int MuseRecord::graphicNoteTypeQ(void) {
	int output = 0;
	string recordInfo = getGraphicNoteTypeField();
	if (recordInfo[0] == ' ') {
		output = 0;
	} else {
		output = 1;
	}
	return output;
}



//////////////////////////////
//
// MuseRecord::graphicNoteTypeSize -- return 0 if cue note size,
//	otherwise, it will return 1 if regular size
//

int MuseRecord::getGraphicNoteTypeSize(void) {
	int output = 1;
	string recordInfo = getGraphicNoteTypeField();
	if (recordInfo[0] == ' ') {
		cerr << "Error: not graphic note specified in column 17: "
			  << getLine() << endl;
		return 0;
	}

	switch (recordInfo[0]) {
		case 'L': case 'b': case 'w': case 'h': case 'q': case 'e':
		case 's': case 't': case 'x': case 'y': case 'z':
			output = 1;
			break;
		case 'B': case 'A': case '9': case '8': case '7': case '6':
		case '5': case '4': case '3': case '2': case '1':
			output = 0;
			break;
		default:
			cerr << "Error: unknown graphical note type in column 17: "
				  << getLine() << endl;
			return 0;
	}

	return output;
}



//////////////////////////////
//
// MuseRecord::getProlongationField -- returns column 18
//

string MuseRecord::getProlongationField(void) {
//   allowNotesOnly("getProlongationField");   ---> rests also
	if (getLength() < 18) {
		return " ";
	} else {
		return extract(18, 18);
	}
}



//////////////////////////////
//
// MuseRecord::getProlongationString --
//

string MuseRecord::getProlongationString(void) {
	string output = getProlongationField();
	if (output[0] == ' ') {
		output = "";
	}
	return output;
}



//////////////////////////////
//
// MuseRecord::getProlongation --
//

int MuseRecord::getProlongation(void) {
	int output = 0;
	string recordInfo = getProlongationField();
	switch (recordInfo[0]) {
		case ' ':   output = 0;   break;
		case '.':   output = 1;   break;
		case ':':   output = 2;   break;
		default:
			cerr << "Error: unknon prologation character (column 18): "
				  << getLine() << endl;
			return 0;
	}
	return output;
}



//////////////////////////////
//
// MuseRecord::getStringProlongation --
//

string MuseRecord::getStringProlongation(void) {
	switch (getProlongation()) {
		case 0:   return "";     break;
		case 1:   return ".";    break;
		case 2:   return "..";   break;
		case 3:   return "...";  break;
		case 4:   return "...."; break;
		default:
			cerr << "Error: unknown number of prolongation dots (column 18): "
				  << getLine() << endl;
			return "";
	}
	return "";
}



//////////////////////////////
//
// MuseRecord::prolongationQ --
//

int MuseRecord::prolongationQ(void) {
	return getProlongation();
}


//////////////////////////////
//
// MuseRecord::getNotatedAccidentalField -- actual notated accidental is
//	    stored in column 19.
//

string MuseRecord::getNotatedAccidentalField(void) {
	allowNotesOnly("getNotatedAccidentalField");
	if (getLength() < 19) {
		return " ";
	} else {
		string temp;
		temp += getColumn(19);
		return temp;
	}
}



//////////////////////////////
//
// MuseRecord::getNotatedAccidentalString --
//

string MuseRecord::getNotatedAccidentalString(void) {
	string output = getNotatedAccidentalField();
	if (output[0] == ' ') {
		output = "";
	}
	return output;
}



//////////////////////////////
//
// MuseRecord::getNotatedAccidental --
//

int MuseRecord::getNotatedAccidental(void) {
	int output = 0;
	string recordInfo = getNotatedAccidentalField();
	switch (recordInfo[0]) {
		case ' ':   output =  0;   break;
		case '#':   output =  1;   break;
		case 'n':   output =  0;   break;
		case 'f':   output = -1;   break;
		case 'x':   output =  2;   break;
		case 'X':   output =  2;   break;
		case '&':   output = -2;   break;
		case 'S':   output =  1;   break;
		case 'F':   output = -1;   break;
		default:
			cerr << "Error: unknown accidental: " << recordInfo[0] << endl;
			return 0;
	}
	return output;
}



//////////////////////////////
//
// MuseRecord::notatedAccidentalQ --
//

int MuseRecord::notatedAccidentalQ(void) {
	int output;
	string recordInfo = getNotatedAccidentalField();
	if (recordInfo[0] == ' ') {
		output = 0;
	} else {
		output = 1;
	}
	return output;
}



///////////////////////////////
//
// MuseRecord::getTimeModificationField -- return columns 20 -- 22.
//

string MuseRecord::getTimeModificationField(void) {
//   allowNotesOnly("getTimeModificationField");   ---> rests also
	if (getLength() < 20) {
		return  "   ";
	} else {
		return extract(20, 22);
	}
}



//////////////////////////////
//
// MuseRecord::getTimeModification --
//

string MuseRecord::getTimeModification(void) {
	string output = getTimeModificationField();
	int index = 2;
	while (index >= 0 && output[index] == ' ') {
		output.resize(index);
		index--;
	}
	if (output.size() > 2) {
		if (output[0] == ' ') {
			output[0] = output[1];
			output[1] = output[2];
			output.resize(2);
		}
	}
	if (output.size() > 1) {
		if (output[0] == ' ') {
			output[0] = output[1];
			output.resize(1);
		}
	}
	if (output[0] == ' ') {
		cerr << "Error: funny error occured in time modification "
			  << "(columns 20-22): " << getLine() << endl;
		return "";
	}
	return output;
}



//////////////////////////////
//
// MuseRecord::getTimeModificationLeftField -- return column 20
//

string MuseRecord::getTimeModificationLeftField(void) {
	string output = getTimeModificationField();
	output.resize(1);
	return output;
}



//////////////////////////////
//
// MuseRecord::getTimeModificationLeftString --
//

string MuseRecord::getTimeModificationLeftString(void) {
	string output = getTimeModificationField();
	if (output[0] == ' ') {
		output = "";
	} else {
		output.resize(1);
	}
	return output;
}



//////////////////////////////
//
// MuseRecord::getTimeModificationLeft --
//

int MuseRecord::getTimeModificationLeft(void) {
	int output = 0;
	string recordInfo = getTimeModificationLeftString();
	if (recordInfo[0] == ' ') {
		output = 0;
	} else {
		output = std::strtol(recordInfo.c_str(), NULL, 36);
	}
	return output;
}



//////////////////////////////
//
// MuseRecord::getTimeModificationRightField -- return column 20
//

string MuseRecord::getTimeModificationRightField(void) {
	string output = getTimeModificationField();
	output = output[2];
	return output;
}



//////////////////////////////
//
// MuseRecord::getTimeModificationRight --
//

string MuseRecord::getTimeModificationRightString(void) {
	string output = getTimeModificationField();
	if (output[2] == ' ') {
		output = "";
	} else {
		output = output[2];
	}
	return output;
}



//////////////////////////////
//
// MuseRecord::getTimeModificationRight --
//

int MuseRecord::getTimeModificationRight(void) {
	int output = 0;
	string recordInfo = getTimeModificationRightString();
	if (recordInfo[2] == ' ') {
		output = 0;
	} else {
		string temp = recordInfo.substr(2);
		output = std::strtol(temp.c_str(), NULL, 36);
	}
	return output;
}



//////////////////////////////
//
// MuseRecord::timeModificationQ --
//

int MuseRecord::timeModificationQ(void) {
	int output = 0;
	string recordInfo = getTimeModificationField();
	if (recordInfo[0] != ' ' || recordInfo[1] != ' ' || recordInfo[2] != ' ') {
		output = 1;
	} else {
		output = 0;
	}
	return output;
}



//////////////////////////////
//
// MuseRecord::timeModificationLeftQ --
//

int MuseRecord::timeModificationLeftQ(void) {
	int output = 0;
	string recordInfo = getTimeModificationField();
	if (recordInfo[0] == ' ') {
		output = 0;
	} else {
		output = 1;
	}
	return output;
}



//////////////////////////////
//
// MuseRecord::timeModificationRightQ --
//

int MuseRecord::timeModificationRightQ(void) {
	int output = 0;
	string recordInfo = getTimeModificationField();
	if (recordInfo[2] == ' ') {
		output = 0;
	} else {
		output = 1;
	}
	return output;
}



//////////////////////////////
//
// MuseRecord::getStemDirectionField --
//

string MuseRecord::getStemDirectionField(void) {
	allowNotesOnly("getStemDirectionField");
	if (getLength() < 23) {
		return " ";
	} else {
		string temp;
		temp += getColumn(23);
		return temp;
	}
}



//////////////////////////////
//
// MuseRecord::getStemDirectionString --
//

string MuseRecord::getStemDirectionString(void) {
	string output = getStemDirectionField();
	if (output[0] == ' ') {
		output = "";
	}
	return output;
}



//////////////////////////////
//
// MuseRecord::getStemDirection --
//

int MuseRecord::getStemDirection(void) {
	int output = 0;
	string recordInfo = getStemDirectionField();
	switch (recordInfo[0]) {
		case 'u':   output = 1;   break;
		case 'd':   output = -1;  break;
		case ' ':   output = 0;   break;
		default:
			cerr << "Error: unknown stem direction: " << recordInfo[0] << endl;
			return 0;
	}
	return output;
}



//////////////////////////////
//
// MuseRecord::stemDirectionQ --
//

int MuseRecord::stemDirectionQ(void) {
	int output = 0;
	string recordInfo = getStemDirectionField();
	if (recordInfo[0] == ' ') {
		output = 0;
	} else {
		output = 1;
	}
	return output;
}



//////////////////////////////
//
// MuseRecord::getStaffField  -- returns column 24.
//

string MuseRecord::getStaffField(void) {
	allowNotesOnly("getStaffField");
	if (getLength() < 24) {
		return " ";
	} else {
		string temp;
		temp += getColumn(24);
		return temp;
	}
}



//////////////////////////////
//
// MuseRecord::getStaffString --
//

string MuseRecord::getStaffString(void) {
	string output = getStaffField();
	if (output[0] == ' ') {
		output = "";
	}
	return output;
}



//////////////////////////////
//
// MuseRecord::getStaff --
//

int MuseRecord::getStaff(void) {
	int output = 1;
	string recordInfo = getStaffField();
	if (recordInfo[0] == ' ') {
		output = 1;
	} else {
		output = std::strtol(recordInfo.c_str(), NULL, 36);
	}
	return output;
}



//////////////////////////////
//
// MuseRecord::staffQ --
//

int MuseRecord::staffQ(void) {
	int output = 0;
	string recordInfo = getStaffField();
	if (recordInfo[0] == ' ') {
		output = 0;
	} else {
		output = 1;
	}
	return output;
}



//////////////////////////////
//
// MuseRecord::getBeamField --
//

string MuseRecord::getBeamField(void) {
	allowNotesOnly("getBeamField");
	if (getLength() < 26) {
		return "      ";
	} else {
		return extract(26, 31);
	}
}



//////////////////////////////
//
// MuseRecord::setBeamInfo --
//

void MuseRecord::setBeamInfo(string& strang) {
	setColumns(strang, 26, 31);
}



//////////////////////////////
//
// MuseRecord::beamQ --
//

int MuseRecord::beamQ(void) {
	int output = 0;
	allowNotesOnly("beamQ");
	if (getLength() < 26) {
		output = 0;
	} else {
		for (int i=26; i<=31; i++) {
			if (getColumn(i) != ' ') {
				output = 1;
				break;
			}
		}
	}
	return output;
}



//////////////////////////////
//
// MuseRecord::getBeam8 -- column 26
//

char MuseRecord::getBeam8(void) {
	allowNotesOnly("getBeam8");
	return getColumn(26);
}



//////////////////////////////
//
// MuseRecord::getBeam16 -- column 27
//

char MuseRecord::getBeam16(void) {
	allowNotesOnly("getBeam16");
	return getColumn(27);
}



//////////////////////////////
//
// MuseRecord::getBeam32 -- column 28
//

char MuseRecord::getBeam32(void) {
	allowNotesOnly("getBeam32");
	return getColumn(28);
}



//////////////////////////////
//
// MuseRecord::getBeam64 -- column 29
//

char MuseRecord::getBeam64(void) {
	allowNotesOnly("getBeam64");
	return getColumn(29);
}



//////////////////////////////
//
// MuseRecord::getBeam128 -- column 30
//

char MuseRecord::getBeam128(void) {
	allowNotesOnly("getBeam128");
	return getColumn(30);
}



//////////////////////////////
//
// MuseRecord::getBeam256 -- column 31
//

char MuseRecord::getBeam256(void) {
	allowNotesOnly("getBeam256");
	return getColumn(31);
}



//////////////////////////////
//
// MuseRecord::beam8Q --
//

int MuseRecord::beam8Q(void) {
	int output = 0;
	if (getBeam8() == ' ') {
		output = 0;
	} else {
		output = 1;
	}
	return output;
}



//////////////////////////////
//
// MuseRecord::beam16Q --
//

int MuseRecord::beam16Q(void) {
	int output = 0;
	if (getBeam16() == ' ') {
		output = 0;
	} else {
		output = 1;
	}
	return output;
}



//////////////////////////////
//
// MuseRecord::beam32Q --
//

int MuseRecord::beam32Q(void) {
	int output = 0;
	if (getBeam32() == ' ') {
		output = 0;
	} else {
		output = 1;
	}
	return output;
}



//////////////////////////////
//
// MuseRecord::beam64Q --
//

int MuseRecord::beam64Q(void) {
	int output = 0;
	if (getBeam64() == ' ') {
		output = 0;
	} else {
		output = 1;
	}
	return output;
}



//////////////////////////////
//
// MuseRecord::beam128Q --
//

int MuseRecord::beam128Q(void) {
	int output = 0;
	if (getBeam128() == ' ') {
		output = 0;
	} else {
		output = 1;
	}
	return output;
}



//////////////////////////////
//
// MuseRecord::beam256Q --
//

int MuseRecord::beam256Q(void) {
	int output = 0;
	if (getBeam256() == ' ') {
		output = 0;
	} else {
		output = 1;
	}
	return output;
}



//////////////////////////////
//
// MuseRecord::getKernBeamStyle --
//

string MuseRecord::getKernBeamStyle(void) {
	string output;
	string beams = getBeamField();
	for (int i=0; i<(int)beams.size(); i++) {
		switch (beams[i]) {
			case '[':                 // start beam
				output += "L";
				break;
			case '=':                 // continue beam
				// do nothing
				break;
			case ']':                 // end beam
				output += "J";
				break;
			case '/':                 // forward hook
				output += "K";
				break;
			case '\\':                 // backward hook
				output += "k";
				break;
			default:
				;  // do nothing
		}
	}
	return output;
}



//////////////////////////////
//
// MuseRecord::getAdditionalNotationsField -- returns the contents
// 	of columns 32-43.
//

string MuseRecord::getAdditionalNotationsField(void) {
	allowNotesOnly("getAdditionalNotationsField");
	return extract(32, 43);
}



//////////////////////////////
//
// MuseRecord::additionalNotationsQ --
//

int MuseRecord::additionalNotationsQ(void) {
	int output = 0;
	if (getLength() < 32) {
		output = 0;
	} else {
		for (int i=32; i<=43; i++) {
			if (getColumn(i) != ' ') {
				output = 1;
				break;
			}
		}
	}
	return output;
}



//////////////////////////////
//
// MuseRecord::getAddCount -- returns the number of items
//	in the additional notations field
//

int MuseRecord::getAddCount(void) {
	string addString = getAdditionalNotationsField();
	string addElement;    // element from the notation field

	int count = 0;
	int index = 0;
	while (getAddElementIndex(index, addElement, addString)) {
		count++;
	}

	return count;
}



//////////////////////////////
//
// MuseRecord::getAddItem -- returns the specified item
//	in the additional notations field
//

string MuseRecord::getAddItem(int elementIndex) {
	string output;
	int count = 0;
	int index = 0;
	string addString = getAdditionalNotationsField();

	while (count <= elementIndex) {
		getAddElementIndex(index, output, addString);
		count++;
	}

	return output;
}



//////////////////////////////
//
// MuseRecord::getAddItemLevel -- returns the specified item's
//	editorial level in the additional notations field
//

int MuseRecord::getAddItemLevel(int elementIndex) {
	int count = 0;
	int index = 0;
	string number;
	string addString = getAdditionalNotationsField();
	string elementString; // element field

	while (count < elementIndex) {
		getAddElementIndex(index, elementString, addString);
		count++;
	}

	int output = -1;
repeating:
	while (addString[index] != '&' && index >= 0) {
		index--;
	}
	if (addString[index] == '&' && !isalnum(addString[index+1])) {
		index--;
		goto repeating;
	} else if (addString[index] == '&') {
		number = addString[index+1];
		output = std::strtol(number.c_str(), NULL, 36);
	}

	return output;
}



//////////////////////////////
//
// MuseRecord::getEditorialLevels -- returns a string containing the
//	edit levels given in the additional notation fields
//

string MuseRecord::getEditorialLevels(void) {
	string output;
	string addString = getAdditionalNotationsField();
	for (int index = 0; index < 12-1; index++) {
		if (addString[index] == '&' && isalnum(addString[index+1])) {
			output += addString[index+1];
		}
	}
	return output;
}



//////////////////////////////
//
// MuseRecord::addEditorialLevelQ -- returns true if there are any editorial
//	levels present in the additional notations fields
//

int MuseRecord::addEditorialLevelQ(void) {
	string addString = getAdditionalNotationsField();
	int output = 0;
	for (int i=0; i<12-1; i++) {   // minus one for width 2 (&0)
		if (addString[i] == '&' && isalnum(addString[i+1])) {
			output = 1;
		}
	}
	return output;
}



//////////////////////////////
//
// MuseRecord::findField -- returns true when it finds the first
//	instance of the key in the additional fields record.
//

int MuseRecord::findField(const string& key) {
	int len = (int)key.size();
	string notations = getAdditionalNotationsField();
	int output = 0;
	for (int i=0; i<12-len; i++) {
		if (notations[i] == key[0]) {
			output = 1;
			for (int j=0; j<len; j++) {
				if (notations[i] != key[j]) {
					output = 0;
					goto endofloop;
				}
			}
		}

		if (output == 1) {
			break;
		}
endofloop:
	;
	}

	return output;
}



//////////////////////////////
//
// MuseRecord::findField --
//

int MuseRecord::findField(char key, int mincol, int maxcol) {
	int start = mincol;
	int stop = getLength() - 1;

	if (start > stop) {
		return -1;
	}

	if (maxcol < stop) {
		stop = maxcol;
	}

	int i;
	for (i=start; i<=stop; i++) {
		if (m_recordString[i-1] == key) {
			return i;   // return the column which is offset from 1
		}
	}

	return -1;
}



//////////////////////////////
//
// MuseRecord::getSlurParameterRegion --
//

string MuseRecord::getSlurParameterRegion(void) {
	return getColumns(31, 43);
}



//////////////////////////////
//
// MuseRecord::getSlurStartColumn -- search column 32 to 43 for a slur
//    marker.  Returns the first one found from left to right.
//    returns -1 if a slur character was not found.
//

int MuseRecord::getSlurStartColumn(void) {
	int start = 31;
	int stop = getLength() - 1;
	if (stop >= 43) {
		stop = 42;
	}
	int i;
	for (i=start; i<=stop; i++) {
		switch (m_recordString[i]) {
			case '(':   // slur level 1
			case '[':   // slur level 2
			case '{':   // slur level 3
			case 'z':   // slur level 4
				return i+1;  // column is offset from 1
		}
	}

	return -1;
}



//////////////////////////////
//
// MuseRecord::getTextUnderlayField -- returns the contents
// 	of columns 44-80.
//

string MuseRecord::getTextUnderlayField(void) {
	allowNotesOnly("getTextUnderlayField");
	return extract(44, 80);
}



//////////////////////////////
//
// MuseRecord::textUnderlayQ --
//

int MuseRecord::textUnderlayQ(void) {
	int output = 0;
	if (getLength() < 44) {
		output = 0;
	} else {
		for (int i=44; i<=80; i++) {
			if (getColumn(i) != ' ') {
				output = 1;
				break;
			}
		}
	}
	return output;
}



//////////////////////////////
//
// MuseRecord::getVerseCount --
//

int MuseRecord::getVerseCount(void) {
	if (!textUnderlayQ()) {
		return 0;
	}

	int count = 1;
	for (int i=44; i<=getLength() && i <= 80; i++) {
		if (getColumn(i) == '|') {
			count++;
		}
	}

	return count;
}



//////////////////////////////
//
// MuseRecord::getVerse --
//

string MuseRecord::getVerse(int index) {
	string output;
	if (!textUnderlayQ()) {
		return output;
	}
	int verseCount = getVerseCount();
	if (index >= verseCount) {
		return output;
	}

	int tindex = 44;
	int c = 0;
	while (c < index && tindex < 80) {
		if (getColumn(tindex) == '|') {
			c++;
		}
		tindex++;
	}

	while (tindex <= 80 && getColumn(tindex) != '|') {
		output += getColumn(tindex++);
	}

	// remove trailing spaces
	int zindex = (int)output.size() - 1;
	while (output[zindex] == ' ') {
		zindex--;
	}
	zindex++;
	output.resize(zindex);

	// remove leading spaces
	int spacecount = 0;
	while (output[spacecount] == ' ') {
		spacecount++;
	}

	// problem here?
	for (int rr = 0; rr <= zindex-spacecount; rr++) {
		output[rr] = output[rr+spacecount];
	}

	return output;
}



//////////////////////////////
//
// MuseRecord::getVerseUtf8 --
//

string MuseRecord::getVerseUtf8(int index) {
	string tverse = getVerse(index);
	return MuseRecord::musedataToUtf8(tverse);
}



//////////////////////////////
//
// MuseRecord::getKernNoteStyle --
//	    default values: beams = 0, stems = 0
//

string MuseRecord::getKernNoteStyle(int beams, int stems) {
	string output;

	// place the rhythm
	stringstream tempdur;
	int notetype = getGraphicNoteType();
	if (timeModificationLeftQ()) {
		notetype = notetype / 4 * getTimeModificationLeft();
		if (timeModificationRightQ()) {
			notetype = notetype * getTimeModificationRight();
		} else {
			notetype = notetype * 2;
		}
	}

	// logical duration of the note
	HumNum logicalduration = getTicks();
	logicalduration /= getTpq();
	string durrecip = Convert::durationToRecip(logicalduration);

	// graphical duration of the note
	string graphicrecip = getGraphicRecip();
	HumNum graphicdur = Convert::recipToDuration(graphicrecip);

	string displayrecip;

	if (graphicdur != logicalduration) {
		// switch to the logical duration and store the graphic
		// duration.  The logical duration will be used on the
		// main kern token, and the graphic duration will be stored
		// as a layout parameter, such as !LO:N:vis=4. to display
		// the note as a dotted quarter regardless of the logical
		// duration.

		// Current test file has encoding bug related to triplets, so
		// disable graphic notation dealing with tuplets for now.

		// for now just looking to see if one has a dot and the other does not
		if ((durrecip.find(".") != string::npos) &&
				(graphicrecip.find(".") == string::npos)) {
			m_graphicrecip = graphicrecip;
			displayrecip = durrecip;
		} else if ((durrecip.find(".") == string::npos) &&
				(graphicrecip.find(".") != string::npos)) {
			m_graphicrecip = graphicrecip;
			displayrecip = durrecip;
		}
	}

	if (displayrecip.size() > 0) {
		output = displayrecip;
	} else {
		tempdur << notetype;
		output = tempdur.str();
		// add any dots of prolongation to the output string
		output += getStringProlongation();
	}

	// add the pitch to the output string
	string musepitch = getPitchString();
	string kernpitch = Convert::musePitchToKernPitch(musepitch);
	output += kernpitch;

	string logicalAccidental = getAccidentalString();
	string notatedAccidental = getNotatedAccidentalString();

	if (notatedAccidental.empty() && !logicalAccidental.empty()) {
		// Indicate that the logical accidental should not be
		// displayed (because of key signature or previous
		// note in the measure that alters the accidental
		// state of the current note).
		output += "y";
	} else if ((logicalAccidental == notatedAccidental) && !notatedAccidental.empty()) {
		// Indicate that the accidental should be displayed
		// and is not suppressed by the key signature or a
		// previous note in the measure.
		output += "X";
	}
	// There can be cases where the logical accidental
	// is natural but the notated accidetnal is sharp (but
	// the notated accidental means play a natural accidetnal).
	// Deal with this later.

	// if there is a notated natural sign, then add it now:
	string temp = getNotatedAccidentalField();
	if (temp == "n") {
		output += "n";
	}

	// check if a grace note
	if (getType() == 'g') {
		output += "Q";
	}

	// if stems is true, then show stem directions
	if (stems && stemDirectionQ()) {
		switch (getStemDirection()) {
			case 1:                         // 'u' = up
				output += "/";
				break;
			case -1:                        // 'd' = down
				output += "\\";
			default:
				; // nothing                 // ' ' = no stem (if stage 2)
		}
	}

	// if beams is true, then show any beams
	if (beams && beamQ()) {
		temp = getKernBeamStyle();
		output += temp;
	}

	if (isTied()) {
		string tiestarts;
		string tieends;
		int lasttie = getLastTiedNoteLineIndex();
		int nexttie = getNextTiedNoteLineIndex();
		int state = 0;
		if (lasttie >= 0) {
			state |= 2;
		}
		if (nexttie >= 0) {
			state |= 1;
		}
		switch (state) {
			case 1:
				tiestarts += "[";
				break;
			case 2:
				tieends += "]";
				break;
			case 3:
				tieends += "_";
				break;
		}
		if (state) {
			output = tiestarts + output + tieends;
		}
	}

	string slurstarts;
	string slurends;
	getSlurInfo(slurstarts, slurends);
	if ((!slurstarts.empty()) || (!slurends.empty())) {
		output = slurstarts + output + slurends;
	}

	return output;
}



//////////////////////////////
//
// MuseRecord::getSlurInfo --
//
//   ( ) = regular slur
//   [ ] = second levels slur, convert to &( and &)
//   { } = third level slur, convert to &&( and &&)
//   Z   = fourth level slur (how to close?)
//

void MuseRecord::getSlurInfo(string& slurstarts, string& slurends) {
	slurstarts.clear();
	slurends.clear();

	string data = getSlurParameterRegion();
	for (int i=0; i<(int)data.size(); i++) {
		if (data[i] == '(') {
			slurstarts += '(';
		} else if (data[i] == ')') {
			slurends += ')';
		} else if (data[i] == '[') {
			slurstarts += "&{";
		} else if (data[i] == ']') {
			slurends += "&)";
		} else if (data[i] == '{') {
			slurstarts += "&&(";
		} else if (data[i] == '}') {
			slurends += "&&)";
		}
	}
}



//////////////////////////////
//
// MuseRecord::getKernNoteAccents --
//

string MuseRecord::getKernNoteAccents(void) {
	string output;
	int addnotecount = getAddCount();
	for (int i=0; i<addnotecount; i++) {
		string tempnote = getAddItem(i);
		switch (tempnote[0]) {
			case 'v':   output += "v";   break; // up-bow
			case 'n':   output += "u";   break; // down-bow
			case 'o':   output += "j";   break; // harmonic
			case 'O':   output += "I";   break; // open string (to generic)
			case 'A':   output += "^";   break; // accent up
			case 'V':   output += "^";   break; // accent down
			case '>':   output += "^";   break; // horizontal accent
			case '.':   output += "'";   break; // staccato
			case '_':   output += "~";   break; // tenuto
			case '=':   output += "~'";  break; // detached legato
			case 'i':   output += "s";   break; // spiccato
			case '\'':  output += ",";   break; // breath mark
			case 'F':   output += ";";   break; // fermata up
			case 'E':   output += ";";   break; // fermata down
			case 'S':   output += ":";   break; // staccato
			case 't':   output += "O";   break; // trill (to generic)
			case 'r':   output += "S";   break; // turn
			case 'k':   output += "O";   break; // delayed turn (to generic)
			case 'w':   output += "O";   break; // shake (to generic)
			case 'M':   output += "O";   break; // mordent (to generic)
			case 'j':   output += "H";   break; // glissando (slide)
	  }
	}

	return output;
}



//////////////////////////////
//
// MuseRecord::getKernRestStyle --
//

string MuseRecord::getKernRestStyle(int quarter) {
	string output;
	string rhythmstring;

	// place the rhythm
	stringstream tempdur;

	int notetype;
	if (graphicNoteTypeQ()) {
		notetype = getGraphicNoteType();

		if (timeModificationLeftQ()) {
			notetype = notetype / 4 * getTimeModificationLeft();
		}
		if (timeModificationRightQ()) {
			notetype = notetype * getTimeModificationRight() / 2;
		}
		tempdur << notetype;
		output =  tempdur.str();

		// add any dots of prolongation to the output string
		output += getStringProlongation();
	} else {   // stage 1 data:
		HumNum dnotetype(getTickDuration(), quarter);
		rhythmstring = Convert::durationToRecip(dnotetype);
		output += rhythmstring;
	}

	// add the pitch to the output string
	output += "r";

	if (isInvisibleRest()) {
		output += "yy";
	}

	return output;
}



//////////////////////////////////////////////////////////////////////////
//
// functions that work with measure records
//


//////////////////////////////
//
// MuseRecord::getMeasureNumberField -- columns 9-12
//

string MuseRecord::getMeasureNumberField(void) {
	allowMeasuresOnly("getMeasureNumberField");
	return extract(9, 12);
}



//////////////////////////////
//
// MuseRecord::getMeasureTypeField -- columns 1 -- 7
//

string MuseRecord::getMeasureTypeField(void) {
	allowMeasuresOnly("getMeasureTypeField");
	return extract(1, 7);
}



//////////////////////////////
//
// MuseRecord::getMeasureNumberString --
//

string MuseRecord::getMeasureNumberString(void) {
	string output = getMeasureNumberField();
	for (int i=3; i>=0; i--) {
		if (output[i] == ' ') {
			output.resize(i);
		}
	}
	return output;
}



//////////////////////////////
//
// MuseRecord::getMeasureNumber --
//

int MuseRecord::getMeasureNumber(void) {
	string measureInfo = getMeasureNumberField();
	if (measureInfo.empty()) {
		return 0;
	}
	return std::stoi(measureInfo);
}



//////////////////////////////
//
// MuseRecord::measureNumberQ --
//

int MuseRecord::measureNumberQ(void) {
	string temp = getMeasureNumberString();
	int i = 0;
	int output = 0;
	while (temp[i] != '\0') {
		if (temp[i] != ' ') {
			output = 1;
			break;
		}
	}
	return output;
}



//////////////////////////////
//
// MuseRecord::getMeasureFlagsString --  Columns 17 to 80.
//

string MuseRecord::getMeasureFlagsString(void) {
	if (m_recordString.size() < 17) {
		return "";
	} else {
		return trimSpaces(m_recordString.substr(16));
	}
}



//////////////////////////////
//
// MuseRecord::measureFermataQ -- returns true if there is a
//	fermata above or below the measure
//

int MuseRecord::measureFermataQ(void) {
	int output = 0;
	for (int i=17; i<=80 && i<= getLength(); i++) {
		if (getColumn(i) == 'F' || getColumn(i) == 'E') {
			output = 1;
			break;
		}
	}
	return output;
}



//////////////////////////////
//
// MuseRecord::measureFlagQ -- Returns true if there are non-space
//     characters in columns 17 through 80.   A more smarter way of
//     doing this is checking the allocated length of the record, and
//     do not search non-allocated columns for non-space characters...
//

int MuseRecord::measureFlagQ(const string& key) {
	int output = 0;
	int len = (int)key.size();
	for (int i=17; i<=80-len && i<getLength(); i++) {
		if (getColumn(i) == key[0]) {
			output = 1;
			for (int j=0; j<len; j++) {
				if (getColumn(i+j) != key[j]) {
					output = 0;
					break;
				}
			}
			if (output == 1) {
				break;
			}
		}
	}
	return output;
}



//////////////////////////////
//
// MuseRecord::addMeasureFlag -- add the following characters to the
//    Flag region of the measure flag area (columns 17-80).  But only
//    add the flag if it is not already present in the region.  If it is
//    not present, then append it after the last non-space character
//    in that region.  A space will be added between the last item
//    and the newly added parameter.
//

void MuseRecord::addMeasureFlag(const string& strang) {
	string flags = getColumns(17, 80);
	string flag = strang;

	HumRegex hre;
	hre.replaceDestructive(flag, "\\*", "\\*", "g");
	hre.replaceDestructive(flag, "\\|", "\\|", "g");
	if (hre.search(flags, flag)) {
		// flag was already found in flags, so don't do anything
		return;
	}
	hre.replaceDestructive(flags, "", "\\s+$");
	flags += " ";
	flags += strang;
	setColumns(flags, 17, 80);
}



//////////////////////////////
//
// MuseRecord::getKernMeasureStyle --
//

string MuseRecord::getKernMeasureStyle(void) {
	allowMeasuresOnly("getKernMeasureStyle");
	string temp;
	string tempstyle = getMeasureTypeField();

	string output = "=";
	if (tempstyle == "mheavy2") {
		output += "=";
	} else if (tempstyle == "mheavy3") {
		output += "=";
	} else if (tempstyle == "mheavy4") {
		output += "=";
	}

	if (measureNumberQ()) {
		temp = getMeasureNumberString();
		output += temp;
	}

	// what is this for-loop for?
	for (int i=0; i<(int)temp.size(); i++) {
		temp[i] = std::tolower(temp[i]);
	}

	if (tempstyle == "mheavy1") {
		output += "!";
	} else if (tempstyle == "mheavy2") {
		  if (measureFlagQ(":||:")) {
			  output += ":|!:";
			  zerase(output, 1);             // make "==" become "="
		  } else if (measureFlagQ(":|")) {
			  output += ":|!";
			  zerase(output, 1);             // make "==" become "="
		  }
	} else if (tempstyle == "mheavy3") {
		output += "!|";
	} else if (tempstyle == "mheavy4") {
		if (measureFlagQ(":||:")) {
			output += ":!!:";
		} else {
			output += "!!";
		}
	}
	return output;
}


//////////////////////////////////////////////////////////////////////////
//
// functions that work with musical attributes records
//

//////////////////////////////
//
// MuseRecord::getAttributeMap --
//

void MuseRecord::getAttributeMap(map<string, string>& amap) {
	amap.clear();
	// Should be "3" on the next line, but "1" or "2" might catch poorly formatted data.
	string contents = getLine().substr(2);
	if (contents.empty()) {
		return;
	}
	int i = 0;
	string key;
	string value;
	int state = 0;  // 0 outside, 1 = in key, 2 = in value
	while (i < (int)contents.size()) {
		switch (state) {
			case 0: // outside of key or value
				if (!isspace(contents[i])) {
					if (contents[i] == ':') {
						// Strange: should not happen
						key.clear();
						state = 2;
					} else {
						state = 1;
						key += contents[i];
					}
				}
				break;
			case 1: // in key
				if (!isspace(contents[i])) {
					if (contents[i] == ':') {
						value.clear();
						state = 2;
					} else {
						// Add to key, such as "C2" for second staff clef.
						key += contents[i];
					}
				}
				break;
			case 2: // in value
				if (key == "D") {
					value += contents[i];
				} else if (isspace(contents[i])) {
					// store parameter and clear variables
					amap[key] = value;
					state = 0;
					key.clear();
					value.clear();
				} else {
					value += contents[i];
				}
				break;
		}
		i++;
	}

	if ((!key.empty()) && (!value.empty())) {
		amap[key] = value;
	}
}



//////////////////////////////
//
// MuseRecord::getAttributes --
//

string MuseRecord::getAttributes(void) {
	string output;
	switch (getType()) {
		case E_muserec_musical_attributes:
			break;
		default:
			cerr << "Error: cannot use getAttributes function on line: "
				  << getLine() << endl;
			return "";
	}

	int ending = 0;
	int tempcol;
	for (int column=4; column <= getLength(); column++) {
		if (getColumn(column) == ':') {
			tempcol = column - 1;
			while (tempcol > 0 && getColumn(tempcol) != ' ') {
				tempcol--;
			}
			tempcol++;
			while (tempcol <= column) {
				output += getColumn(tempcol);
				if (output.back() == 'D') {
					ending = 1;
				}
				tempcol++;
			}
		}
		if (ending) {
			break;
		}
	}
	return output;
}



//////////////////////////////
//
// MuseRecord::attributeQ --
//

int MuseRecord::attributeQ(const string& attribute) {
	switch (getType()) {
		case E_muserec_musical_attributes:
			break;
		default:
			cerr << "Error: cannot use getAttributes function on line: "
				  << getLine() << endl;
			return 0;
	}


	string attributelist = getAttributes();

	int output = 0;
	int attstrlength = (int)attributelist.size();
	int attlength = (int)attribute.size();

	for (int i=0; i<attstrlength-attlength+1; i++) {
		if (attributelist[i] == attribute[0]) {
			output = 1;
			for (int j=0; j<attlength; j++) {
				if (attributelist[i] != attribute[j]) {
					output = 0;
					break;
				}
			}
			if (output == 1) {
				break;
			}
		}
	}
	return output;
}



//////////////////////////////
//
// MuseRecord::getAttributeInt --
//

int MuseRecord::getAttributeInt(char attribute) {
	switch (getType()) {
		case E_muserec_musical_attributes:
			break;
		default:
			cerr << "Error: cannot use getAttributeInt function on line: "
				  << getLine() << endl;
			return 0;
	}

	int output = E_unknown;
	int ending = 0;
	int index = 0;
	int tempcol;
	int column;
	for (column=4; column <= getLength(); column++) {
		if (getColumn(column) == ':') {
			tempcol = column - 1;
			while (tempcol > 0 && getColumn(tempcol) != ' ') {
				tempcol--;
			}
			tempcol++;
			while (tempcol <= column) {
				if (getColumn(tempcol) == attribute) {
					ending = 2;
				} else if (getColumn(tempcol) == 'D') {
					ending = 1;
				}
				tempcol++;
				index++;
			}
		}
		if (ending) {
			break;
		}
	}

	if (ending == 0 || ending == 1) {
		return output;
	} else {
		string value = &getColumn(column+1);
		if (value.empty()) {
			output = std::stoi(value);
			return output;
		} else {
			return 0;
		}
	}
}



//////////////////////////////
//
// MuseRecord::getAttributeField -- returns true if found attribute
//

int MuseRecord::getAttributeField(string& value, const string& key) {
	switch (getType()) {
		case E_muserec_musical_attributes:
			break;
		default:
			cerr << "Error: cannot use getAttributeInt function on line: "
				  << getLine() << endl;
			return 0;
	}

	int returnValue = 0;
	int ending = 0;
	int index = 0;
	int tempcol;
	int column;
	for (column=4; column <= getLength(); column++) {
		if (getColumn(column) == ':') {
			tempcol = column - 1;
			while (tempcol > 0 && getColumn(tempcol) != ' ') {
				tempcol--;
			}
			tempcol++;
			while (tempcol <= column) {
				if (getColumn(tempcol) == key[0]) {
					ending = 2;
				} else if (getColumn(tempcol) == 'D') {
					ending = 1;
				}
				tempcol++;
				index++;
			}
		}
		if (ending) {
			break;
		}
	}

	value.clear();
	if (ending == 0 || ending == 1) {
		return returnValue;
	} else {
		returnValue = 1;
		column++;
		while (getColumn(column) != ' ') {
			value += getColumn(column++);
		}
		return returnValue;
	}
}



///////////////////////////////////////////////////////////////////////////
//
// functions that work with basso continuo figuration records (f):
//


//////////////////////////////
//
// MuseRecord::getFigureCountField -- column 2.
//

string MuseRecord::getFigureCountField(void) {
	allowFigurationOnly("getFigureCountField");
	return extract(2, 2);
}



//////////////////////////////
//
// MuseRecord::getFigurationCountString --
//

string MuseRecord::getFigureCountString(void) {
	allowFigurationOnly("getFigureCount");
	string output = extract(2, 2);
	if (output[0] == ' ') {
		output = "";
	}
	return output;
}



//////////////////////////////
//
// MuseRecord::getFigurationCount --
//

int MuseRecord::getFigureCount(void) {
	allowFigurationOnly("getFigureCount");
	string temp = getFigureCountString();
	int output = std::strtol(temp.c_str(), NULL, 36);
	return output;
}



//////////////////////////////
//
// getFigurePointerField -- columns 6 -- 8.
//

string MuseRecord::getFigurePointerField(void) {
	allowFigurationOnly("getFigurePointerField");
	return extract(6, 8);
}


//////////////////////////////
//
// figurePointerQ --
//

int MuseRecord::figurePointerQ(void) {
	allowFigurationOnly("figurePointerQ");
	int output = 0;
	for (int i=6; i<=8; i++) {
		if (getColumn(i) != ' ') {
			output = 1;
			break;
		}
	}
	return output;
}



//////////////////////////////
//
// MuseRecord::getFigureString --
//

string MuseRecord::getFigureString(void) {
	string output = getFigureFields();
	for (int i=(int)output.size()-1; i>= 0; i--) {
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
// MuseRecord::getFigureFields -- columns 17 -- 80
//

string MuseRecord::getFigureFields(void) {
	allowFigurationOnly("getFigureFields");
	return extract(17, 80);
}


//////////////////////////////
//
// MuseRecord::figureFieldsQ --
//

int MuseRecord::figureFieldsQ(void) {
	allowFigurationOnly("figureFieldsQ");
	int output = 0;
	if (getLength() < 17) {
		output = 0;
	} else {
		for (int i=17; i<=80; i++) {
			if (getColumn(i) != ' ') {
				output = 1;
				break;
			}
		}
	}
	return output;
}



//////////////////////////////
//
// getFigure --
//

string MuseRecord::getFigure(int index) {
	string output;
	allowFigurationOnly("getFigure");
	if (index >= getFigureCount()) {
		return output;
	}
	string temp = getFigureString();
	if (index == 0) {
		return temp;
	}
	HumRegex hre;
	vector<string> pieces;
	hre.split(pieces, temp, " +");
	if (index < (int)pieces.size()) {
	output = pieces[index];
	}
	return output;
}



///////////////////////////////////////////////////////////////////////////
//
// protected functions
//


//////////////////////////////
//
// MuseRecord::allowFigurationOnly --
//

void MuseRecord::allowFigurationOnly(const string& functionName) {
	switch (getType()) {
		case E_muserec_figured_harmony:
		  break;
		default:
			cerr << "Error: can only access " << functionName
				  << " on a figuration record.  Line is: " << getLine() << endl;
			return;
	}
}



//////////////////////////////
//
// MuseRecord::allowFigurationAndNotesOnly --
//

void MuseRecord::allowFigurationAndNotesOnly(const string& functionName) {
	switch (getType()) {
		case E_muserec_figured_harmony:
		case E_muserec_note_regular:
		case E_muserec_note_chord:
		case E_muserec_note_grace:
		case E_muserec_note_cue:
		  break;
		default:
			cerr << "Error: can only access " << functionName
				  << " on a figuration record.  Line is: " << getLine() << endl;
			return;
	}
}



//////////////////////////////
//
// MuseRecord::allowMeasuresOnly --
//

void MuseRecord::allowMeasuresOnly(const string& functionName) {
	switch (getType()) {
		case E_muserec_measure:
		  break;
		default:
			cerr << "Error: can only access " << functionName
				  << " on a measure record.  Line is: " << getLine() << endl;
			return;
	}
}



//////////////////////////////
//
// MuseRecord::allowNotesOnly --
//

void MuseRecord::allowNotesOnly(const string& functionName) {
	switch (getType()) {
		case E_muserec_note_regular:
		case E_muserec_note_chord:
		case E_muserec_note_grace:
		case E_muserec_note_cue:
		  break;
		default:
			cerr << "Error: can only access " << functionName
				  << " on a note record.  Line is: " << getLine() << endl;
			return;
	}
}



//////////////////////////////
//
// MuseRecord::allowNotesAndRestsOnly --
//

void MuseRecord::allowNotesAndRestsOnly(const string& functionName) {
	switch (getType()) {
		case E_muserec_note_regular:
		case E_muserec_note_chord:
		case E_muserec_note_grace:
		case E_muserec_note_cue:
		case E_muserec_rest:
		case E_muserec_rest_invisible:
		  break;
		default:
			cerr << "Error: can only access " << functionName
				  << " on a note or rest records.  Line is: " << getLine() << endl;
			return;
	}
}



//////////////////////////////
//
// MuseRecord::getAddElementIndex -- get the first element pointed
//	to by the specified index in the additional notations field.
//	returns 0 if no element was found, or 1 if an element was found.
//

int MuseRecord::getAddElementIndex(int& index, string& output, const string& input) {
	int finished = 0;
	int count = 0;
	output.clear();

	while (!finished) {
		switch (input[index]) {
			case '&':                     // editorial level marker
				// there is exactly one character following an editorial
				// marker.  neither the '&' nor the following character
				// is counted if the following character is in the set
				// [0..9, A..Z, a..z]
				index++;
				if (isalnum(input[index])) {
					index++;
				} else {
					// count '&' as an element
					count++;
					output += '&';
				}
				break;

			case 'p': case 'f':          // piano and forte
				// any sequence of 'p' and 'f' is considered one element
				count++;
				output += input[index++];
				while (input[index] == 'p' || input[index] == 'f') {
					output += input[index++];
				}
				break;

			case 'Z':                    // sfz, or Zp = sfp
				// elements starting with 'Z':
				//    Z      = sfz
				//    Zp     = sfp
				count++;
				output += input[index++];
				if (input[index] == 'p') {
					output += input[index++];
				}
				break;

			case 'm':                      // mezzo
				// a mezzo marking MUST be followed by a 'p' or an 'f'.
				count++;
				output += input[index++];
				if (input[index] == 'p' || input[index] == 'f') {
					output += input[index++];
				} else {
				  cout << "Error at \'m\' in notation field: " << input << endl;
				  return 0;
				}
				break;

			case 'S':                     // arpeggiation
				// elements starting with 'S':
				//   S     = arpeggiate (up)
				//   Sd    = arpeggiate down)
				count++;
				output += input[index++];
				if (input[index] == 'd') {
					output += input[index++];
				}
				break;

			case '1':                     // fingering
			case '2':                     // fingering
			case '3':                     // fingering
			case '4':                     // fingering
			case '5':                     // fingering
			// case ':':                  // finger substitution
				// keep track of finger substitutions
				count++;
				output += input[index++];
				if (input[index] == ':') {
					output += input[index++];
					output += input[index++];
				}
				break;

			//////////////////////////////
			// Ornaments
			//
			case 't':                     // trill (tr.)
			case 'r':                     // turn
			case 'k':                     // delayed turn
			case 'w':                     // shake
			case '~':                     // trill wavy line extension
			case 'c':                     // continued wavy line
			case 'M':                     // mordent
			case 'j':                     // slide (Schleifer)
			  // ornaments can be modified by accidentals:
			  //    s     = sharp
			  //    ss    = double sharp
			  //    f     = flat
			  //    ff    = double flat
			  //    h     = natural
			  //    u     = next accidental is under the ornament
			  // any combination of these characters following a
			  // ornament is considered one element.
			  //
			  count++;
			  index++;
			  while (input[index] == 's' || input[index] == 'f' ||
						input[index] == 'h' || input[index] == 'u') {
				  output += input[index++];
			  }
			  break;

			//////////////////////////////////////////////////////////////
			// The following chars are uniquely SINGLE letter items:    //
			//                                                          //
			//                                                          //
			case '-':                     // tie                        //
			case '(':                     // open  slur #1              //
			case ')':                     // close slur #1              //
			case '[':                     // open  slur #2              //
			case ']':                     // close slur #2              //
			case '{':                     // open  slur #3              //
			case '}':                     // close slur #3              //
			case 'z':                     // open  slur #4              //
			case 'x':                     // close slur #4              //
			case '*':                     // start written tuplet       //
			case '!':                     // end written tuplet         //
			case 'v':                     // up bow                     //
			case 'n':                     // down bow                   //
			case 'o':                     // harmonic                   //
			case 'O':                     // open string                //
			case 'Q':                     // thumb position             //
			case 'A':                     // accent (^)                 //
			case 'V':                     // accent (v)                 //
			case '>':                     // accent (>)                 //
			case '.':                     // staccatto                  //
			case '_':                     // tenuto                     //
			case '=':                     // detached tenuto            //
			case 'i':                     // spiccato                   //
			case '\'':                    // breath mark                //
			case 'F':                     // upright fermata            //
			case 'E':                     // inverted fermata           //
			case 'R':                     // rfz                        //
			case '^':                     // editorial accidental       //
			case '+':                     // cautionary accidental      //
				count++;                                                 //
				output += input[index++];                                //
				break;                                                   //
			//                                                          //
			//                                                          //
			//////////////////////////////////////////////////////////////
			case ' ':
				// ignore blank spaces and continue counting elements
				index++;
				break;
			default:
				cout << "Error: unknown additional notation: "
					  << input[index] << endl;
				return 0;
		}
		if (count != 0 || index >= 12) {
			finished = 1;
		}
	} // end of while (!finished) loop

	return count;
}



////////////////////
//
// MuseRecord::zerase -- removes specified number of characters from
// 	the beginning of the string.
//

void MuseRecord::zerase(string& inout, int num) {
	int len = (int)inout.size();
	if (num >= len) {
		inout = "";
	} else {
		for (int i=num; i<=len; i++) {
			inout[i-num] = inout[i];
		}
	}
	inout.resize(inout.size() - num);
}



// END_MERGE

} // end namespace hum



