//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Tue Jun 30 22:41:24 PDT 1998
// Last Modified: Sun Apr 14 02:01:37 PDT 2024
// Filename:      humlib/src/MuseRecord-humdrum.cpp
// Web Address:   http://github.com/craigsapp/humlib/blob/master/src/MuseRecord-humdrum.cpp
// Syntax:        C++11
// vim:           ts=3
//
// Description:   Humdrum related conversion functions for MuseRecord.
//
//

#include "Convert.h"
#include "MuseData.h"
#include "MuseRecord.h"

#include <sstream>

using namespace std;

namespace hum {

// START_MERGE


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
// MuseRecord::getKernNoteStyle --
//     default values: beams = 0, stems = 0
//

string MuseRecord::getKernNoteStyle(int beams, int stems) {
	string output;

	if (!isAnyNote()) {
		// not a note, so return nothing
		return "";
	}

	// place the rhythm
	stringstream tempdur;
	HumNum notetype = getGraphicNoteType();
	HumNum mod = getTimeModification();
	if (mod != 1) {
		notetype *= mod;
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

string MuseRecord::getKernRestStyle(void) {

	string output;
	string rhythmstring;

	// place the rhythm
	stringstream tempdur;

	if (!isAnyRest()) {
		// not a rest, so return nothing
		return "";
	}

	// logical duration of the note
	HumNum logicalduration = getTicks();
	logicalduration /= getTpq();
	string durrecip = Convert::durationToRecip(logicalduration);

	/*
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
	*/

	output = durrecip;

	// add the pitch to the output string
	output += "r";

	if (isInvisibleRest()) {
		output += "yy";
	}

	return output;
}



//////////////////////////////
//
// MuseRecord::getKernMeasure -- Return the **kern measure token
//     for barline.
//

string MuseRecord::getKernMeasure(void) {
	if (!isBarline()) {
		return "";
	}
	string measureStyle = getMeasureType();
	string measureFlag  = getMeasureFlags();

	string output = "=";
	if ((measureStyle.find("mheavy") != string::npos) && measureFlag.empty()) {
		output += "=";
	}

	if ((output != "==") && measureNumberQ()) {
		output += getMeasureNumber();
	}

	if (measureStyle == "mheavy1") {
		output += "!";
	} else if (measureStyle == "mheavy2") {
		if (measureFlagEqual(":||:")) {
			output += ":|!|:";
		} else if (measureFlagEqual("|: :|")) {
			// Vivaldi op. 1, no. 1, mvmt. 1, m. 10: mheavy4          |: :|
			output += ":|!|:";
		}
	} else if (measureStyle == "mheavy3") {
		output += "!|";
	} else if (measureStyle == "mheavy4") {
		if (measureFlagEqual(":||:")) {
			output += ":!!:";
		} else if (measureFlagEqual(":||:")) {
			output += ":|!|:";
		} else if (measureFlagEqual("|: :|")) {
			output += ":|!|:";
		} else {
			output += "!!";
		}
	}
	return output;
}



//////////////////////////////
//
// MuseRecord::getKernNoteOtherNotations -- Extract note-level ornaments
//    and articulations.  See MuseRecord::getOtherNotation() for list
//    of "other notations".
//

string MuseRecord::getKernNoteOtherNotations(void) {
	string output;
	string notations = getOtherNotations();
	for (int i=0; i<(int)notations.size(); i++) {
		switch(notations[i]) {
			case 'F': // fermata above
				output += ";";
				break;
			case 'E': // fermata below
				output += ";<";
				break;
			case '.': // staccato
				output += "'";
				break;
			case ',': // breath mark
				output += ",";
				break;
			case '=': // tenuto-staccato
				output += "~'";
				break;
			case '>': // accent
				output += "^";
				break;
			case 'A': // heavy accent
				output += "^^";
				break;
			case 'M': // mordent
				output += "M";
				break;
			case 'r': // turn
				output += "S";
				break;
			case 't': // trill
				output += "T";
				break;
			case 'n': // down bow
				output += "u";
				break;
			case 'v': // up bow
				output += "v";
				break;
			case 'Z': // sfz
				output += "zz";
				break;
		}
	}
	return output;
}


// END_MERGE

} // end namespace hum



