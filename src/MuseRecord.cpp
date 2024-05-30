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
#include "HumRegex.h"
#include "MuseData.h"
#include "MuseRecord.h"

#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sstream>

using namespace std;

namespace hum {

// START_MERGE


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



//////////////////////////////
//
// MuseRecord::operator= --
//

MuseRecord& MuseRecord::operator=(MuseRecord& aRecord) {
	// don't copy onto self
	if (&aRecord == this) {
		return *this;
	}

	setLine(aRecord.getLine());
	setType(aRecord.getType());
	m_lineindex = aRecord.m_lineindex;

	m_qstamp       = aRecord.m_qstamp;
	m_lineduration = aRecord.m_lineduration;
	m_noteduration = aRecord.m_noteduration;

	m_b40pitch     = aRecord.m_b40pitch;
	m_nexttiednote = aRecord.m_nexttiednote;
	m_lasttiednote = aRecord.m_lasttiednote;

	return *this;
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
// MuseRecord::allDirectionsOnly --
//

void MuseRecord::allowDirectionsOnly(const std::string& functionName) {
	switch (getType()) {
		case E_muserec_musical_directions:
			break;
		default:
			cerr << "Error: can only access " << functionName
				  << " on a musical direction record.  Line is: " << getLine() << endl;
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



/////////////////////////////////////////
//
// MuseRecord::getDirectionTypeString -- columns 17 and 18.
//    A = segno sign
//    B = right-justified text
//    C = center-justified text
//    D = left-justified text
//    E = dynamics hairpin start
//    F = dynamics hairpin end
//    G = letter dynamics (text given starting in column 25)
//    H = begin dashes (after words)
//    J = end dashes
//    P = pedal start
//    Q = pedal stop
//    R = rehearsal number or letter
//    U = octave up start
//    V = octave down start
//    W = octave stop
//

std::string MuseRecord::getDirectionTypeField(void) {
	allowDirectionsOnly("getDirectionType");
	return extract(17, 18);
}



//////////////////////////////
//
// MuseRecord::getDirectionTypeString -- Same as the field version, but
//    trailing spaces are removed (not leading ones, at least for now).
//

std::string MuseRecord::getDirectionTypeString(void) {
	string output = getDirectionTypeField();
	if (output.back() == ' ') {
		output.resize(output.size() - 1);
	}
	if (output.back() == ' ') {
		output.resize(output.size() - 1);
	}
	return output;
}



//////////////////////////////
//
// MuseRecord::isTextDirection -- Text is stored starting at column 25.
//    B = right justified
//    C = center justified
//    D = left justified
//

bool MuseRecord::isTextDirection(void) {
	string typefield = getDirectionTypeField();
	if (typefield.find('B') != string::npos) {
		return true;
	}
	if (typefield.find('C') != string::npos) {
		return true;
	}
	if (typefield.find('D') != string::npos) {
		return true;
	}
	return false;
}



//////////////////////////////
//
// MuseRecord::isHairpin --
//

bool MuseRecord::isHairpin(void) {
	string typefield = getDirectionTypeField();
	if (isHairpinStart()) {
		return true;
	}
	if (isHairpinStop()) {
		return true;
	}
	return false;
}



//////////////////////////////
//
// MuseRecord::isHairpinStart --
//

bool MuseRecord::isHairpinStart(void) {
	string typefield = getDirectionTypeField();
	if (typefield.find('E') != string::npos) {
		return true;
	}
	return false;
}



//////////////////////////////
//
// MuseRecord::isHairpinStop --
//

bool MuseRecord::isHairpinStop(void) {
	string typefield = getDirectionTypeField();
	if (typefield.find('F') != string::npos) {
		return true;
	}
	return false;
}



//////////////////////////////
//
// MuseRecord::isDashStart --
//

bool MuseRecord::isDashStart(void) {
	string typefield = getDirectionTypeField();
	if (typefield.find('H') != string::npos) {
		return true;
	}
	return false;
}



//////////////////////////////
//
// MuseRecord::isDashStop --
//

bool MuseRecord::isDashStop(void) {
	string typefield = getDirectionTypeField();
	if (typefield.find('J') != string::npos) {
		return true;
	}
	return false;
}



//////////////////////////////
//
// MuseRecord::isPedalStart --
//

bool MuseRecord::isPedalStart(void) {
	string typefield = getDirectionTypeField();
	if (typefield.find('P') != string::npos) {
		return true;
	}
	return false;
}



//////////////////////////////
//
// MuseRecord::isPedalEnd --
//

bool MuseRecord::isPedalEnd(void) {
	string typefield = getDirectionTypeField();
	if (typefield.find('Q') != string::npos) {
		return true;
	}
	return false;
}



//////////////////////////////
//
// MuseRecord::isRehearsal --
//

bool MuseRecord::isRehearsal(void) {
	string typefield = getDirectionTypeField();
	if (typefield.find('R') != string::npos) {
		return true;
	}
	return false;
}



//////////////////////////////
//
// MuseRecord::isOctiveUpStart --
//

bool MuseRecord::isOctaveUpStart(void) {
	string typefield = getDirectionTypeField();
	if (typefield.find('U') != string::npos) {
		return true;
	}
	return false;
}



//////////////////////////////
//
// MuseRecord::isOctaveDownStart --
//

bool MuseRecord::isOctaveDownStart(void) {
	string typefield = getDirectionTypeField();
	if (typefield.find('V') != string::npos) {
		return true;
	}
	return false;
}



//////////////////////////////
//
// MuseRecord::isOctaveStop --
//

bool MuseRecord::isOctaveStop(void) {
	string typefield = getDirectionTypeField();
	if (typefield.find('W') != string::npos) {
		return true;
	}
	return false;
}



//////////////////////////////
//
// MuseRecord::getDirectionText -- Return the text starting in column 25.
//

std::string MuseRecord::getDirectionText(void) {
	int length = (int)m_recordString.size();
	if (length < 25) {
		// no text
		return "";
	}
	return trimSpaces(m_recordString.substr(24));
}



//////////////////////////////
//
// MuseRecord::hasPrintSuggestions --
//

bool MuseRecord::hasPrintSuggestions(void) {
	MuseData* md = getOwner();
	if (md == NULL) {
		return false;
	}
	if (m_lineindex < 0) {
		return false;
	}
	if (m_lineindex >= md->getLineCount() - 1) {
		return false;
	}
	if (md->getRecord(m_lineindex).isPrintSuggestion()) {
		return true;
	} else {
		return false;
	}
}



//////////////////////////////
//
// MuseRecord::getPrintSuggestions -- Return any print suggestions
//     for the given column number
//

void MuseRecord::getPrintSuggestions(vector<string>& suggestions, int column) {
	suggestions.clear();

	MuseData* md = getOwner();
	if (md == NULL) {
		return;
	}
	if (m_lineindex < 0) {
		return;
	}
	if (m_lineindex >= md->getLineCount() - 1) {
		return;
	}
	if (!md->getRecord(m_lineindex+1).isPrintSuggestion()) {
		return;
	}

	string pline = md->getLine(m_lineindex+1);
	HumRegex hre;
	vector<string> entries;
	hre.split(entries, pline, "\\s+");
	for (int i=0; i<(int)entries.size(); i++) {
		if (entries[i][0] != 'C') {
			continue;
		}
		if (hre.search(entries[i], "C(\\d+):([^\\s]+)")) {
			int value = hre.getMatchInt(1);
			if (value == column) {
				suggestions.push_back(hre.getMatch(2));
			}
		}
	}
}



//////////////////////////////
//
// MuseRecord::getAllPrintSuggestions -- Return all print suggestions.
//

void MuseRecord::getAllPrintSuggestions(vector<string>& suggestions) {
	suggestions.clear();

	MuseData* md = getOwner();
	if (md == NULL) {
		return;
	}
	if (m_lineindex < 0) {
		return;
	}
	if (m_lineindex >= md->getLineCount() - 1) {
		return;
	}
	if (!md->getRecord(m_lineindex+1).isPrintSuggestion()) {
		return;
	}

	string pline = md->getLine(m_lineindex+1);
	HumRegex hre;
	vector<string> entries;
	hre.split(entries, pline, " ");
	for (int i=0; i<(int)entries.size(); i++) {
		if (entries[i][0] != 'C') {
			continue;
		}
		if (hre.search(entries[i], "C(\\d+):([^\\s]+)")) {
			suggestions.push_back(entries[i]);
		}
	}
}






// END_MERGE

} // end namespace hum



