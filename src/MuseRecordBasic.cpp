//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Tue Jun 30 21:44:58 PDT 1998
// Last Modified: Mon Sep 23 22:49:43 PDT 2019 Convert to STL
// Filename:      humlib/src/MuseRecordBasic.cpp
// URL:           http://github.com/craigsapp/humlib/blob/master/src/MuseRecordBasic.cpp
// Syntax:        C++11
// vim:           ts=3
//

#include "MuseRecordBasic.h"

#include <cctype>
#include <cstdio>
#include <cstring>
#include <sstream>

using namespace std;

namespace hum {

// START_MERGE



//////////////////////////////
//
// MuseRecordBasic::MuseRecordBasic --
//

MuseRecordBasic::MuseRecordBasic(void) {
	m_recordString.reserve(81);
	setType(E_muserec_unknown);
	m_owner        = NULL;
	m_lineindex    =   -1;
	m_qstamp      =    0;
	m_lineduration =    0;
	m_noteduration =    0;
	m_b40pitch     = -100;
	m_nexttiednote =   -1;
	m_lasttiednote =   -1;
	m_roundBreve   =    0;
	m_header       =   -1;
	m_layer        =    0;
}


// default value: index = -1;
MuseRecordBasic::MuseRecordBasic(const string& aLine, int index) {
	m_recordString.reserve(81);
	setLine(aLine);
	setType(E_muserec_unknown);
	m_lineindex = index;
	m_owner        = NULL;
	m_qstamp      =    0;
	m_lineduration =    0;
	m_noteduration =    0;
	m_b40pitch     = -100;
	m_nexttiednote =   -1;
	m_lasttiednote =   -1;
	m_roundBreve   =    0;
	m_header       =   -1;
	m_layer        =    0;
}


MuseRecordBasic::MuseRecordBasic(MuseRecordBasic& aRecord) {
	*this = aRecord;
}



//////////////////////////////
//
// MuseRecordBasic::~MuseRecordBasic --
//

MuseRecordBasic::~MuseRecordBasic() {
	m_recordString.resize(0);
	m_owner        = NULL;
	m_lineindex    =   -1;
	m_qstamp      =    0;
	m_lineduration =    0;
	m_noteduration =    0;
	m_b40pitch     = -100;
	m_nexttiednote =   -1;
	m_lasttiednote =   -1;
	m_roundBreve   =    0;
	m_layer        =    0;
}



//////////////////////////////
//
// MuseRecordBasic::clear -- remove content of record.
//

void MuseRecordBasic::clear(void) {
	m_recordString.clear();
	m_owner        = NULL;
	m_qstamp      =    0;
	m_lineindex    =   -1;
	m_lineduration =    0;
	m_noteduration =    0;
	m_b40pitch     = -100;
	m_nexttiednote =   -1;
	m_lasttiednote =   -1;
	m_roundBreve   =    0;
	m_header       =   -1;
	m_layer        =    0;
}



/////////////////////////////
//
// MuseRecordBasic::isEmpty -- returns true if only spaces on line, ignoring
//     non-printable characters (which may no longer be necessary).
//

int MuseRecordBasic::isEmpty(void) {
	for (int i=0; i<(int)m_recordString.size(); i++) {
		if (!std::isprint(m_recordString[i])) {
			continue;
		}
		if (!std::isspace(m_recordString[i])) {
			return 0;
		}
	}
	return 1;
}



//////////////////////////////
//
// MuseRecordBasic::extract -- extracts the character columns from the
//	storage string.  Appends a null character to the end of the
//	copied string.
//

string MuseRecordBasic::extract(int start, int end) {
	string output;
	int count = end - start + 1;
	for (int i=0; i<count; i++) {
		if (i+start <= getLength()) {
			output += getColumn(i+start);
		} else {
			output += ' ';
		}
	}
	return output;
}



//////////////////////////////
//
// MuseRecordBasic::getColumn -- same as operator[] but with an
//	offset of 1 rather than 0.
//

char& MuseRecordBasic::getColumn(int columnNumber) {
	int realindex = columnNumber - 1;
	int length = (int)m_recordString.size();
	// originally the limit for data columns was 80:
	// if (realindex < 0 || realindex >= 80) {
	// the new limit is somewhere above 900, but limit to 180
	if (realindex < 0 || realindex >= 180) {
		cerr << "Error trying to access column: " << columnNumber  << endl;
		cerr << "CURRENT DATA: ===============================" << endl;
		cerr << (*this);
		static char x = ' ';
		return x;
	} else if (realindex >= (int)m_recordString.size()) {
		m_recordString.resize(realindex+1);
		for (int i=length; i<=realindex; i++) {
			m_recordString[i] = ' ';
		}
	}
	return m_recordString[realindex];
}



//////////////////////////////
//
// MuseRecordBasic::getColumns --
//

string MuseRecordBasic::getColumns(int startcol, int endcol) {
	string output;
	int charcount = endcol - startcol + 1;
	if (charcount <= 0) {
		return output;
	}
	for (int i=startcol; i<=endcol; i++) {
		output += getColumn(i);
	}
	return output;
}



//////////////////////////////
//
// MuseRecordBasic::setColumns --
//

void MuseRecordBasic::setColumns(string& data, int startcol, int endcol) {
	if (startcol > endcol) {
		int temp = startcol;
		startcol = endcol;
		endcol = temp;
	}

	int dsize = (int)data.size();
	getColumn(endcol) = ' '; // allocate space if not already done
	int i;
	int ii;
	for (i=startcol; i<=endcol; i++) {
		ii = i - startcol;
		if (ii < dsize) {
			getColumn(i) = data[ii];
		} else {
			break;
		}
	}

}



//////////////////////////////
//
// MuseRecordBasic::getLength -- returns the size of the
//	character string being stored.  A number between
//	0 and 80.
//

int MuseRecordBasic::getLength(void) const {
	return (int)m_recordString.size();
}



//////////////////////////////
//
// MuseRecordBasic::getLine -- returns a pointer to data record
//

string MuseRecordBasic::getLine(void) {
	return m_recordString;
}



//////////////////////////////
//
// MuseRecordBasic::getType -- returns the type of the record.
//

int MuseRecordBasic::getType(void) const {
	return m_type;
}



//////////////////////////////
//
// MuseRecordBasic::operator=
//

MuseRecordBasic& MuseRecordBasic::operator=(MuseRecordBasic& aRecord) {
	// don't copy onto self
	if (&aRecord == this) {
		return *this;
	}

	setLine(aRecord.getLine());
	setType(aRecord.getType());
	m_lineindex = aRecord.m_lineindex;

	m_qstamp = aRecord.m_qstamp;
	m_lineduration = aRecord.m_lineduration;
	m_noteduration = aRecord.m_noteduration;

	m_b40pitch     = aRecord.m_b40pitch;
	m_nexttiednote = aRecord.m_nexttiednote;
	m_lasttiednote = aRecord.m_lasttiednote;

	return *this;
}


MuseRecordBasic& MuseRecordBasic::operator=(MuseRecordBasic* aRecord) {
	*this = *aRecord;
	return *this;
}


MuseRecordBasic& MuseRecordBasic::operator=(const string& aLine) {
	setLine(aLine);
	setType(aLine[0]);

	m_lineindex    =   -1;
	m_qstamp      =    0;
	m_lineduration =    0;
	m_noteduration =    0;
	m_b40pitch     = -100;
	m_nexttiednote =   -1;
	m_lasttiednote =   -1;

	return *this;
}



//////////////////////////////
//
// MuseRecordBasic::operator[] -- character array offset from 0.
//

char& MuseRecordBasic::operator[](int index) {
	return getColumn(index+1);
}



//////////////////////////////
//
// MuseRecordBasic::setLine -- sets the record to a (new) string
//

void MuseRecordBasic::setLine(const string& aLine) {
	m_recordString = aLine;
	// Line lengths should not exceed 80 characters according
	// to MuseData standard, so maybe have a warning or error if exceeded.
}



//////////////////////////////
//
// MuseRecordBasic::setType -- sets the type of the record
//

void MuseRecordBasic::setType(int aType) {
	m_type = aType;
}



//////////////////////////////
//
// MuseRecordBasic::setTypeGraceNote -- put a "g" in the first column.
//    shift pitch information over if it exists?  Maybe later.
//    Currently will destroy any pitch information.
//

void MuseRecordBasic::setTypeGraceNote(void) {
	setType(E_muserec_note_grace);
	(*this)[0] = 'g';
}



//////////////////////////////
//
// MuseRecordBasic::setTypeGraceChordNote -- put a "g" in the first column,
//    and a space in the second column.  Shift pitch information over if
//    it exists?  Maybe later.  Currently will destroy any pitch information.
//

void MuseRecordBasic::setTypeGraceChordNote(void) {
	setType(E_muserec_note_grace_chord);
	(*this)[0] = 'g';
	(*this)[1] = ' ';
}



//////////////////////////////
//
// MuseRecordBasic::shrink -- removes trailing spaces in a MuseData record
//

void MuseRecordBasic::shrink(void) {
	int i = (int)m_recordString.size() - 1;
	while (i >= 0 && m_recordString[i] == ' ') {
		m_recordString.resize((int)m_recordString.size()-1);
		i--;
	}
}



//////////////////////////////
//
// MuseRecordBasic::insertString --
//

void MuseRecordBasic::insertString(int column, const string& strang) {
	int len = (int)strang.size();
	if (len == 0) {
		return;
	}
	int index = column - 1;
	// make sure that record has text data up to the end of sring in
	// final location by preallocating the end location of string:
	(*this)[index+len-1] = ' ';
	int i;
	for (i=0; i<len; i++) {
		(*this)[i+index] = strang[i];
	}
}



//////////////////////////////
//
// MuseRecordBasic::insertStringRight -- Insert string right-justified
//    starting at given index.
//

void MuseRecordBasic::insertStringRight(int column, const string& strang) {
	int len = (int)strang.size();
	int index = column - 1;
	// make sure that record has text data up to the end of sring in
	// final location by preallocating the end location of string:
	(*this)[index] = ' ';
	int i;
	int ii;
	for (i=0; i<len; i++) {
		ii = index - i;
		if (ii < 0) {
			break;
		}
		(*this)[ii] = strang[len-i-1];
	}
}



//////////////////////////////
//
// MuseRecordBasic::appendString -- add a string to the end of the current
//     data in the record.
//

void MuseRecordBasic::appendString(const string& astring) {
	insertString(getLength()+1, astring);
}



//////////////////////////////
//
// MuseRecordBasic::appendInteger -- Insert an integer after the last character
//     in the current line.
//

void MuseRecordBasic::appendInteger(int value) {
	string buffer = to_string(value);
	insertString(getLength()+1, buffer);
}



//////////////////////////////
//
// MuseRecordBasic::appendRational -- Insert a rational after the last character
//     in the current line.
//

void MuseRecordBasic::appendRational(HumNum& value) {
	stringstream tout;
	value.printTwoPart(tout);
	tout << ends;
	insertString(getLength()+1, tout.str());
}



//////////////////////////////
//
// MuseRecordBasic::append -- append multiple objects in sequence
// from left to right onto the record.  The format contains
// characters with two possibilities at the moment:
//    "i": integer value
//    "s": string value
//

void MuseRecordBasic::append(const char* format, ...) {
	va_list valist;
	int     i;

	va_start(valist, format);

	union Format_t {
		int   i;
		char *s;
		int  *r;  // array of two integers for rational number
	} FormatData;

	HumNum rn;

	int len = (int)strlen(format);
	for (i=0; i<len; i++) {
		switch (format[i]) {   // Type to expect.
			case 'i':
				FormatData.i = va_arg(valist, int);
				appendInteger(FormatData.i);
				break;

			case 's':
				FormatData.s = va_arg(valist, char *);
				if (strlen(FormatData.s) > 0) {
					appendString(FormatData.s);
				}
				break;

			case 'r':
				 FormatData.r = va_arg(valist, int *);
				 rn.setValue(FormatData.r[0], FormatData.r[1]);
				 appendRational(rn);
				break;

			default:
				// don't put any character other than "i", "r" or "s"
				// in the format string
				break;
		}
	}

	va_end(valist);
}



//////////////////////////////
//
// MuseRecordBasic::setString --
//

void MuseRecordBasic::setString(string& astring) {
	m_recordString = astring;
}



//////////////////////////////
//
// MuseRecordBasic::setAbsBeat --
//


void MuseRecordBasic::setAbsBeat(HumNum value) {
	m_qstamp = value;
}


void MuseRecordBasic::setQStamp(HumNum value) {
	m_qstamp = value;
}


// default value botval = 1
void MuseRecordBasic::setAbsBeat(int topval, int botval) {
	m_qstamp.setValue(topval, botval);
}


void MuseRecordBasic::setQStamp(int topval, int botval) {
	m_qstamp.setValue(topval, botval);
}



//////////////////////////////
//
// MuseRecordBasic::getAbsBeat -- Quarter notes from the
//     start of the music.
//

HumNum MuseRecordBasic::getAbsBeat(void) {
	return m_qstamp;
}

HumNum MuseRecordBasic::getQStamp(void) {
	return m_qstamp;
}



//////////////////////////////
//
// MuseRecordBasic::setLineDuration -- set the duration of the line
//     in terms of quarter notes.
//

void MuseRecordBasic::setLineDuration(HumNum value) {
	m_lineduration = value;
}


// default value botval = 1
void MuseRecordBasic::setLineDuration(int topval, int botval) {
	m_lineduration.setValue(topval, botval);
}



//////////////////////////////
//
// MuseRecordBasic::getLineDuration -- set the duration of the line
//     in terms of quarter notes.
//

HumNum MuseRecordBasic::getLineDuration(void) {
	return m_lineduration;
}



//////////////////////////////
//
// MuseRecordBasic::setNoteDuration -- set the duration of the note
//     in terms of quarter notes.  If the line does not represent a note,
//     then the note duration should probably be 0...
//

void MuseRecordBasic::setNoteDuration(HumNum value) {
	m_noteduration = value;
}


// default value botval = 1
void MuseRecordBasic::setNoteDuration(int topval, int botval) {
	m_noteduration.setValue(topval, botval);
}



//////////////////////////////
//
// MuseRecordBasic::getNoteDuration -- get the duration of the note
//     in terms of quarter notes.  If the line does not represent a note,
//     then the note duration should probably be 0...
//

HumNum MuseRecordBasic::getNoteDuration(void) {
	return m_noteduration;
}



//////////////////////////////
//
// MuseRecordBasic::setLineIndex --
//

void MuseRecordBasic::setLineIndex(int index) {
	m_lineindex = index;
}



//////////////////////////////
//
// MuseRecordBasic::isTied -- True if the note is tied to a note
//    before or after it.
//  0 = no ties
//  1 = tied to previous note
//  2 = tied to future note
//  3 = tied to both previous and future note
//

int MuseRecordBasic::isTied(void) {
	int output = 0;
	if (getLastTiedNoteLineIndex() >= 0) {
		output += 1;
	}

	if (getNextTiedNoteLineIndex() >= 0) {
		output += 2;
	}

	return output;
}



//////////////////////////////
//
// MuseRecordBasic::getLastTiedNoteLineIndex --
//


int MuseRecordBasic::getLastTiedNoteLineIndex(void) {
	return m_lasttiednote;
}



//////////////////////////////
//
// MuseRecordBasic::getNextTiedNoteLineIndex --
//


int MuseRecordBasic::getNextTiedNoteLineIndex(void) {
	return m_nexttiednote;
}



//////////////////////////////
//
// MuseRecordBasic::setLastTiedNoteLineIndex --
//


void MuseRecordBasic::setLastTiedNoteLineIndex(int index) {
	m_lasttiednote = index;
}



//////////////////////////////
//
// MuseRecordBasic::setNextTiedNoteLineIndex --
//


void MuseRecordBasic::setNextTiedNoteLineIndex(int index) {
	m_nexttiednote = index;
}



//////////////////////////////
//
// MuseRecordBasic::setRoundedBreve -- set double whole notes rounded flag.
//

void MuseRecordBasic::setRoundedBreve(void) {
	m_roundBreve = 1;
}



//////////////////////////////
//
// MuseRecordBasic::setMarkupPitch -- set the base-40 pitch information
//   in the markup area.  Does not change the original pitch in
//   the text line of the data.
//

void MuseRecordBasic::setMarkupPitch(int aPitch) {
	m_b40pitch = aPitch;
}



//////////////////////////////
//
// MuseRecordBasic::getMarkupPitch -- get the base-40 pitch information
//   in the markup area.  Does not look at the original pitch in
//   the text line of the data. A negative value is a rest (or invalid).
//

int MuseRecordBasic::getMarkupPitch(void) {
	return m_b40pitch;
}



//////////////////////////////
//
// MuseRecordBasic::cleanLineEnding -- remove spaces at the end of the
//    line;
//

void MuseRecordBasic::cleanLineEnding(void) {
	int i = (int)m_recordString.size() - 1;
	// Don't remove first space on line.
	while ((i > 0) && (m_recordString[i] == ' ')) {
		m_recordString.resize((int)m_recordString.size() - 1);
		i = (int)m_recordString.size() - 1;
	}
}



//////////////////////////////
//
// MuseRecordBasic::trimSpaces --
//

string MuseRecordBasic::trimSpaces(std::string input) {
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
// MuseRecordBasic::setHeaderState -- 1 = in header, 0 = in body, -1 = undefined.
//    Access with isHeaderRecord() and isBodyRecord().
//

void MuseRecordBasic::setHeaderState(int state) {
	if (state > 0) {
		m_header = 1;
	} else if (state < 0) {
		m_header = -1;
	} else {
		m_header = 0;
	}
}



//////////////////////////////
//
// MuseRecordBasic::setLayer -- Set the layer for the record.
//    This information is taken from the track parameter
//    of records, but may be inferred from its position in
//    relation to backup commands.  Zero means implicit layer 1.
//

void MuseRecordBasic::setLayer(int layer) {
	if (layer < 0) {
		m_layer = 0;
	} else {
		m_layer = layer;
	}
}



//////////////////////////////
//
// MuseRecordBasic::getLayer -- Get the layer for the record.
//    This information is taken from the track parameter
//    of records, but may be inferred from its position in
//    relation to backup commands.  Zero means implicit layer 1.
//

int MuseRecordBasic::getLayer(void) {
	return m_layer;
}



//////////////////////////////
//
// MuseRecordBasic:hasTpq --
//

bool MuseRecordBasic::hasTpq(void) {
	if (m_tpq) {
		return true;
	} else {
		return false;
	}
}



//////////////////////////////
//
// MuseRecordBasic:getTpq --
//

int MuseRecordBasic::getTpq(void) {
	return m_tpq;
}



//////////////////////////////
//
// MuseRecordBasic:setTpq --
//

void MuseRecordBasic::setTpq(int value) {
	if (value <= 0) {
		m_tpq = 0;
	} else {
		m_tpq = value;
	}
}



//////////////////////////////
//
// MuseRecordBasic:setVoice --
//

void MuseRecordBasic::setVoice(GridVoice* voice) {
	m_voice = voice;
}



//////////////////////////////
//
// MuseRecordBasic:getVoice --
//

GridVoice* MuseRecordBasic::getVoice(void) {
	return m_voice;
}



//////////////////////////////
//
// MuseRecordBasic:LayoutVis -- Return the graphical display of the
//    rhythm for the note/rest if it was different than the logical version.
//    This is temporary storage for inserting a layout command into the
//    MuseData-to-Humdrum converter (tool-musedata2hum.cpp).
//

std::string MuseRecordBasic::getLayoutVis(void) {
	return m_graphicrecip;
}



//////////////////////////////
//
// MuseRecordBasic::musedataToUtf8 --
//

string MuseRecordBasic::musedataToUtf8(string& input) {
	string output;
	int isize = (int)input.size();
	for (int i=0; i<isize; i++) {
		if (input[i] != '\\') {
			output += input[i];
			continue;
		}
		if (i+2 >= isize) {
			output += input[i];
			continue;
		}
		string st = input.substr(i+1, 2);

		// graves
		if (st == "A8") { output += (char)0xc3; output += (char)0x80; i+=2; continue; }
		if (st == "E8") { output += (char)0xc3; output += (char)0x88; i+=2; continue; }
		if (st == "I8") { output += (char)0xc3; output += (char)0x8c; i+=2; continue; }
		if (st == "O8") { output += (char)0xc3; output += (char)0x92; i+=2; continue; }
		if (st == "U8") { output += (char)0xc3; output += (char)0x99; i+=2; continue; }
		if (st == "a8") { output += (char)0xc3; output += (char)0xa0; i+=2; continue; }
		if (st == "e8") { output += (char)0xc3; output += (char)0xa8; i+=2; continue; }
		if (st == "i8") { output += (char)0xc3; output += (char)0xac; i+=2; continue; }
		if (st == "o8") { output += (char)0xc3; output += (char)0xb2; i+=2; continue; }
		if (st == "u8") { output += (char)0xc3; output += (char)0xb9; i+=2; continue; }

		// acutes
		if (st == "A7") { output += (char)0xc3; output += (char)0x81; i+=2; continue; }
		if (st == "E7") { output += (char)0xc3; output += (char)0x89; i+=2; continue; }
		if (st == "I7") { output += (char)0xc3; output += (char)0x8d; i+=2; continue; }
		if (st == "O7") { output += (char)0xc3; output += (char)0x93; i+=2; continue; }
		if (st == "U7") { output += (char)0xc3; output += (char)0x9a; i+=2; continue; }
		if (st == "a7") { output += (char)0xc3; output += (char)0xa1; i+=2; continue; }
		if (st == "e7") { output += (char)0xc3; output += (char)0xa9; i+=2; continue; }
		if (st == "i7") { output += (char)0xc3; output += (char)0xad; i+=2; continue; }
		if (st == "o7") { output += (char)0xc3; output += (char)0xb3; i+=2; continue; }
		if (st == "u7") { output += (char)0xc3; output += (char)0xba; i+=2; continue; }

		// umlauts
		if (st == "A3") { output += (char)0xc3; output += (char)0x84; i+=2; continue; }
		if (st == "E3") { output += (char)0xc3; output += (char)0x8b; i+=2; continue; }
		if (st == "I3") { output += (char)0xc3; output += (char)0x8f; i+=2; continue; }
		if (st == "O3") { output += (char)0xc3; output += (char)0x96; i+=2; continue; }
		if (st == "U3") { output += (char)0xc3; output += (char)0x9c; i+=2; continue; }
		if (st == "a3") { output += (char)0xc3; output += (char)0xa4; i+=2; continue; }
		if (st == "e3") { output += (char)0xc3; output += (char)0xab; i+=2; continue; }
		if (st == "i3") { output += (char)0xc3; output += (char)0xaf; i+=2; continue; }
		if (st == "o3") { output += (char)0xc3; output += (char)0xb6; i+=2; continue; }
		if (st == "u3") { output += (char)0xc3; output += (char)0xbc; i+=2; continue; }

		// other
		if (st == "s2") { output += (char)0xc3; output += (char)0x9f; i+=2; continue; }  // eszett

		// Older Musedata files reverse the number and letters:

		// graves
		if (st == "8A") { output += (char)0xc3; output += (char)0x80; i+=2; continue; }
		if (st == "8E") { output += (char)0xc3; output += (char)0x88; i+=2; continue; }
		if (st == "8I") { output += (char)0xc3; output += (char)0x8c; i+=2; continue; }
		if (st == "8O") { output += (char)0xc3; output += (char)0x92; i+=2; continue; }
		if (st == "8U") { output += (char)0xc3; output += (char)0x99; i+=2; continue; }
		if (st == "8a") { output += (char)0xc3; output += (char)0xa0; i+=2; continue; }
		if (st == "8e") { output += (char)0xc3; output += (char)0xa8; i+=2; continue; }
		if (st == "8i") { output += (char)0xc3; output += (char)0xac; i+=2; continue; }
		if (st == "8o") { output += (char)0xc3; output += (char)0xb2; i+=2; continue; }
		if (st == "8u") { output += (char)0xc3; output += (char)0xb9; i+=2; continue; }

		// acutes
		if (st == "7A") { output += (char)0xc3; output += (char)0x81; i+=2; continue; }
		if (st == "7E") { output += (char)0xc3; output += (char)0x89; i+=2; continue; }
		if (st == "7I") { output += (char)0xc3; output += (char)0x8d; i+=2; continue; }
		if (st == "7O") { output += (char)0xc3; output += (char)0x93; i+=2; continue; }
		if (st == "7U") { output += (char)0xc3; output += (char)0x9a; i+=2; continue; }
		if (st == "7a") { output += (char)0xc3; output += (char)0xa1; i+=2; continue; }
		if (st == "7e") { output += (char)0xc3; output += (char)0xa9; i+=2; continue; }
		if (st == "7i") { output += (char)0xc3; output += (char)0xad; i+=2; continue; }
		if (st == "7o") { output += (char)0xc3; output += (char)0xb3; i+=2; continue; }
		if (st == "7u") { output += (char)0xc3; output += (char)0xba; i+=2; continue; }

		// umlauts
		if (st == "3A") { output += (char)0xc3; output += (char)0x84; i+=2; continue; }
		if (st == "3E") { output += (char)0xc3; output += (char)0x8b; i+=2; continue; }
		if (st == "3I") { output += (char)0xc3; output += (char)0x8f; i+=2; continue; }
		if (st == "3O") { output += (char)0xc3; output += (char)0x96; i+=2; continue; }
		if (st == "3U") { output += (char)0xc3; output += (char)0x9c; i+=2; continue; }
		if (st == "3a") { output += (char)0xc3; output += (char)0xa4; i+=2; continue; }
		if (st == "3e") { output += (char)0xc3; output += (char)0xab; i+=2; continue; }
		if (st == "3i") { output += (char)0xc3; output += (char)0xaf; i+=2; continue; }
		if (st == "3o") { output += (char)0xc3; output += (char)0xb6; i+=2; continue; }
		if (st == "3u") { output += (char)0xc3; output += (char)0xbc; i+=2; continue; }

		// other
		if (st == "2s") { output += (char)0xc3; output += (char)0x9f; i+=2; continue; }  // eszett

	}

	return output;
}



//////////////////////////////
//
// MuseRecordBasic::setOwner --
//

void MuseRecordBasic::setOwner(MuseData* owner) {
	m_owner = owner;
}



//////////////////////////////
//
// MuseRecordBasic::getOwner --
//

MuseData* MuseRecordBasic::getOwner(void) {
	return m_owner;
}



///////////////////////////////////////////////////////////////////////////


//////////////////////////////
//
// operator<<
//

ostream& operator<<(ostream& out, MuseRecordBasic& aRecord) {
	aRecord.shrink();  // have to shrink automatically because
						    // muse2ps program chokes on line 9 of header
						    // if it has more than one space on a blank line.
	out << aRecord.getLine();
	return out;
}

ostream& operator<<(ostream& out, MuseRecordBasic* aRecord) {
	out << *aRecord;
	return out;
}


// END_MERGE

} // end namespace hum



