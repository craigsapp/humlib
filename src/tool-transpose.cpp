//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Apr 12 23:22:19 PDT 2004
// Last Modified: Thu Feb 24 22:43:17 PST 2005 Added -k option
// Last Modified: Wed Jun 24 15:39:58 PDT 2009 Updated for GCC 4.4
// Last Modified: Sun Sep 13 12:34:51 PDT 2009 Added -s option
// Last Modified: Wed Nov 18 14:01:20 PST 2009 Added *Tr markers
// Last Modified: Thu Nov 19 14:08:32 PST 2009 Added -q, -d and -c options
// Last Modified: Thu Nov 19 15:12:01 PST 2009 Added -I options and *ITr marks
// Last Modified: Thu Nov 19 19:28:26 PST 2009 Added -W and -C options
// Last Modified: Mon Dec  5 23:28:50 PST 2016 Ported to humlib from humextras
// Last Modified: Wed May 16 22:47:11 PDT 2018 Added **mxhm transposition
// Last Modified: Thu Jun 14 15:30:53 PDT 2018 Added rest position transposition
// Filename:      tool-transpose.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/tool-transpose.cpp
// Syntax:        C++11; humlib; humlib
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Transpose **kern musical data.
//

#include "tool-transpose.h"
#include "Convert.h"
#include "HumRegex.h"
#include <cstring>
#include <ctype.h>

using namespace std;

namespace hum {

// START_MERGE


#define STYLE_CONCERT 0
#define STYLE_WRITTEN 1

/////////////////////////////////
//
// Tool_transpose::Tool_transpose -- Set the recognized options for the tool.
//

Tool_transpose::Tool_transpose(void) {
	define("b|base40=i:0",    "the base-40 transposition value");
	define("d|diatonic=i:0",  "the diatonic transposition value");
	define("c|chromatic=i:0", "the chromatic transposition value");
	define("o|octave=i:0",    "the octave addition to tranpose value");
	define("t|transpose=s",   "musical interval transposition value");
	define("k|settonic=s",    "transpose to the given key/tonic (mode will not change)");
	define("auto=b",          "auto. trans. inst. parts to concert pitch");
	define("debug=b",         "print debugging statements");
	define("s|spines=s:",     "transpose only specified spines");
	// quiet reversed with -T option (actively need to request transposition code now)
	// define("q|quiet=b",       "suppress *Tr interpretations in output");
	define("T|transcode=b",   "include transposition code to reverse transposition");
	define("I|instrument=b",  "insert instrument code (*ITr) as well");
	define("C|concert=b",     "transpose written score to concert pitch");
	define("W|written=b",     "trans. concert pitch score to written score");
	define("n|negate=b",      "negate transposition indications");
	define("rotation=b",      "display transposition in half-steps");

	define("author=b",   "author of program");
	define("version=b",  "compilation info");
	define("example=b",  "example usages");
	define("help=b",     "short description");
}



/////////////////////////////////
//
// Tool_transpose::run -- Do the main work of the tool.
//

bool Tool_transpose::run(HumdrumFileSet& infiles) {
	bool status = true;
	for (int i=0; i<infiles.getCount(); i++) {
		status &= run(infiles[i]);
	}
	return status;
}


bool Tool_transpose::run(const string& indata, ostream& out) {
	HumdrumFile infile(indata);
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_transpose::run(HumdrumFile& infile, ostream& out) {
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}

//
// In-place processing of file:
//

bool Tool_transpose::run(HumdrumFile& infile) {
	initialize(infile);

	if (ssettonicQ) {
		transval = calculateTranspositionFromKey(ssettonic, infile);
		transval = transval + octave * 40;
		if (debugQ) {
			m_humdrum_text << "!!Key TRANSVAL = " << transval;
		}
	}

	if (getBoolean("rotation")) {
		// returns the base-12 pitch transposition for use in conjunction
		// with the mkeyscape --rotate option
		int value = 60 - Convert::base40ToMidiNoteNumber(162 - transval);
		m_free_text << value << endl;
		return false;
	}

	if (concertQ) {
		convertScore(infile, STYLE_CONCERT);
	} else if (writtenQ) {
		convertScore(infile, STYLE_WRITTEN);
	} else if (autoQ) {
		doAutoTransposeAnalysis(infile);
	} else {
		vector<bool> spineprocess;
		infile.makeBooleanTrackList(spineprocess, spinestring);
		// filter out non-kern spines so they are not analyzed.
		// but now also allowing for *mxhm spines (musicxml harmony)
		for (int t=1; t<=infile.getMaxTrack(); t++) {
			if (!(infile.getTrackStart(t)->isKern() ||
					infile.getTrackStart(t)->isDataType("mxhm"))) {
				spineprocess[t] = false;
			}
		}
		processFile(infile, spineprocess);
	}

	return true;
}



//////////////////////////////
//
// Tool_transpose::convertScore -- create a concert pitch score from
//     a written pitch score.  The function will search for *Tr
//     interpretations in spines, and convert them to *ITr interpretations
//     as well as transposing notes, and transposing key signatures and
//     key interpretations.  Or create a written score from a
//     concert pitch score based on the style parameter.
//

void Tool_transpose::convertScore(HumdrumFile& infile, int style) {
	// transposition values for each spine
	vector<int> tvals(infile.getMaxTrack()+1, 0);

	int ptrack;
	int i, j;
	for (i=0; i<infile.getLineCount(); i++) {
		if (infile[i].isInterpretation()) {
				// scan the line for transposition codes
				// as well as key signatures and key markers
				processInterpretationLine(infile, i, tvals, style);
		} else if (infile[i].isData()) {
			// transpose notes according to tvals data
			for (j=0; j<infile[i].getFieldCount(); j++) {
				if (!infile.token(i, j)->isKern()) {
					m_humdrum_text << infile.token(i, j);
					if (j < infile[i].getFieldCount() - 1) {
					 		m_humdrum_text << "\t";
					}
					continue;
				}
				ptrack = infile.token(i, j)->getTrack();
				if (tvals[ptrack] == 0) {
					  m_humdrum_text << infile.token(i, j);
				} else {
					  printTransposedToken(infile, i, j, tvals[ptrack]);
				}
				if (j < infile[i].getFieldCount() - 1) {
					  m_humdrum_text << "\t";
				}
			}
			m_humdrum_text << "\n";
		} else {
			m_humdrum_text << infile[i] << "\n";
		}
	}
}



//////////////////////////////
//
// Tool_transpose::processInterpretationLine --  Used in converting between
//   concert pitch and written pitch scores.  Has some duplicate code that
//   is not used here.
//

void Tool_transpose::processInterpretationLine(HumdrumFile& infile, int line,
	  vector<int>& tvals, int style) {

	if (hasTrMarkers(infile, line)) {
		switch (style) {
			case STYLE_CONCERT:
				convertToConcertPitches(infile, line, tvals);
				break;
			case STYLE_WRITTEN:
				convertToWrittenPitches(infile, line, tvals);
				break;
			default: m_humdrum_text << infile[line];
		}
		m_humdrum_text << "\n";
		return;
	}

	for (int j=0; j<infile[line].getFieldCount(); j++) {
		int ptrack = infile.token(line, j)->getTrack();

		// check for *ITr or *Tr markers
		// ignore *ITr markers when creating a Concert-pitch score
		// ignore *Tr  markers when creating a Written-pitch score
		HumRegex hre;
		if (hre.search(infile.token(line, j), "^\\*k\\[([a-gA-G\\#-]*)\\]", "")) {
			// transpose *k[] markers if necessary
			if (tvals[ptrack] != 0) {
				string value = hre.getMatch(1);
				printNewKeySignature(value, tvals[ptrack]);
			} else {
				m_humdrum_text << infile.token(line, j);
			}
		} else if (isKeyMarker(*infile.token(line, j))) {
			// transpose *C: markers and like if necessary
			if (tvals[ptrack] != 0) {
				printNewKeyInterpretation(infile[line], j, tvals[ptrack]);
			} else if (transval != 0) {
				// maybe not quite right for all possible cases
				printNewKeyInterpretation(infile[line], j, transval);
			} else {
				m_humdrum_text << infile.token(line, j);
			}
		} else {
			// other interpretations just echoed to output:
			m_humdrum_text << infile.token(line, j);
		}
		if (j<infile[line].getFieldCount()-1) {
			m_humdrum_text << "\t";
		}
	}
	m_humdrum_text << "\n";

}



//////////////////////////////
//
// Tool_transpose::convertToWrittenPitches --
//

void Tool_transpose::convertToWrittenPitches(HumdrumFile& infile, int line,
		vector<int>& tvals) {
	HumRegex hre;
	int j;
	int base;
	int ptrack;
	for (j=0; j<infile[line].getFieldCount(); j++) {
		if (!infile.token(line, j)->isKern()) {
			m_humdrum_text << infile.token(line, j);
			if (j < infile[line].getFieldCount() - 1) {
				m_humdrum_text << "\t";
			}
			continue;
		}
		if (hre.search(infile.token(line, j),
		"^\\*ITrd[+-]?\\d+c[+-]?\\d+$", "")) {
			base = Convert::transToBase40(*infile.token(line, j));

			string output = "*Tr";
			output += Convert::base40ToTrans(base);
			m_humdrum_text << output;
			ptrack = infile.token(line, j)->getTrack();
			tvals[ptrack] = base;
		} else {
			m_humdrum_text << infile.token(line, j);
		}
		if (j < infile[line].getFieldCount() - 1) {
			m_humdrum_text << "\t";
		}
	}
}



//////////////////////////////
//
// Tool_transpose::convertToConcertPitches --
//

void Tool_transpose::convertToConcertPitches(HumdrumFile& infile, int line, vector<int>& tvals) {
	HumRegex hre;
	int j;
	int base;
	int ptrack;
	for (j=0; j<infile[line].getFieldCount(); j++) {
		if (!infile.token(line, j)->isKern()) {
			m_humdrum_text << infile.token(line, j);
			if (j < infile[line].getFieldCount() - 1) {
				m_humdrum_text << "\t";
			}
			continue;
		}
		if (hre.search(infile.token(line, j),
				"^\\*Trd[+-]?\\d+c[+-]?\\d+$", "")) {
			base = Convert::transToBase40(*infile.token(line, j));
			string output = "*ITr";
			output += Convert::base40ToTrans(base);
			m_humdrum_text << output;
			ptrack = infile.token(line, j)->getTrack();
			tvals[ptrack] = -base;
		} else {
			m_humdrum_text << infile.token(line, j);
		}
		if (j < infile[line].getFieldCount() - 1) {
			m_humdrum_text << "\t";
		}
	}
}



//////////////////////////////
//
// Tool_transpose::hasTrMarkers -- returns true if there are any tokens
//    which start with *ITr or *Tr and contains c and d
//    with numbers after each of them.
//

int Tool_transpose::hasTrMarkers(HumdrumFile& infile, int line) {
	HumRegex hre;
	int j;
	for (j=0; j<infile[line].getFieldCount(); j++) {
		if (!infile.token(line, j)->isKern()) {
			continue;
		}
		if (hre.search(infile.token(line, j),
				"^\\*I?Trd[+-]?\\d+c[+-]?\\d+$", "")) {
			return 1;
		}
	}

	return 0;
}



//////////////////////////////
//
// Tool_transpose::isKeyMarker -- returns true if the interpretation is
//    a key description, such as *C: for C major, or *a:.
//

int Tool_transpose::isKeyMarker(const string& str) {
	HumRegex hre;
	return hre.search(str, "^\\*[a-g]?[\\#-]?:", "i");
}



//////////////////////////////
//
// Tool_transpose::printTransposedToken -- print a Humdrum token with the given
//    base-40 transposition value applied.  Only **kern data is
//    know now to transpose, other data types are currently not
//    allowed to be transposed (but could be added here later).
//

void Tool_transpose::printTransposedToken(HumdrumFile& infile, int row, int col, int transval) {
	if (!infile.token(row, col)->isKern()) {
		// don't know how to transpose this type of data, so leave it as is
		m_humdrum_text << infile.token(row, col);
	} else {
		printHumdrumKernToken(infile[row], col, transval);
	}
}



//////////////////////////////
//
// Tool_transpose::calculateTranspositionFromKey --
//

int Tool_transpose::calculateTranspositionFromKey(int targetkey,
		HumdrumFile& infile) {
	HumRegex hre;
	int base40 = 0;
	int currentkey = 0;
	int mode = 0;
	int found = 0;

	for (int i=0; i<infile.getLineCount(); i++) {
		if (infile[i].isData()) {
			// no initial key label was found, so don't transpose.
			// in the future, maybe allow an automatic key analysis
			// to be performed on the data if there is not explicit
			// key designation.
			return 0;
		}
		if (!infile[i].isInterpretation()) {
			continue;
		}
		for (int j=0; j<infile[i].getFieldCount(); j++) {
			if (!infile.token(i, j)->isKern()) {
				continue;
			}
			if (!hre.search(infile.token(i, j), "^\\*([A-G][#-]?):", "i")) {
				continue;
			}

			mode = 0;  // major key
			if (islower(infile.token(i, j)->at(1))) {
				mode = 1;  // minor key
			}
			base40 = Convert::kernToBase40(infile.token(i, j));
			// base40 = base40 + transval;
			base40 = base40 + 4000;
			base40 = base40 % 40;
			base40 = base40 + (3 + mode) * 40;
			currentkey = base40;
	 		found = 1;
			break;
		}
		if (found) {
			break;
		}
	}

	int trans = (targetkey%40 - currentkey%40);
	// base40 = targetkey + (3 + mode) * 40;
	if (trans > 40) {
		trans -= 40;
	}
	if (trans > 20) {
		trans = 40 - trans;
		trans = -trans;
	}
	if (trans < -40) {
		trans += 40;
	}
	if (trans < -20) {
		trans = -40 - trans;
		trans = -trans;
	}

	return trans;
}



//////////////////////////////
//
// Tool_transpose::printTransposeInformation -- collect and print *Tr interpretations
//      at the start of the spine.  Looks for *Tr markers at the start
//      of the file before any data.
//

void Tool_transpose::printTransposeInformation(HumdrumFile& infile,
		vector<bool>& spineprocess, int line, int transval) {
	int j;
	int ptrack;

	vector<int> startvalues(infile.getMaxTrack()+1);
	vector<int> finalvalues(infile.getMaxTrack()+1);

	for (j=0; j<infile[line].getFieldCount(); j++) {
		if (!infile.token(line, j)->isKern()) {
			continue;
		}
		ptrack = infile.token(line, j)->getTrack();
		startvalues[ptrack] = getTransposeInfo(infile, line, j);
		// m_humdrum_text << "Found transpose value " << startvalues[ptrack] << endl;
	}

	int entry = 0;
	// check if any spine will be transposed after final processing
	for (j=0; j<infile[line].getFieldCount(); j++) {
		if (!infile.token(line, j)->isKern()) {
			continue;
		}
		ptrack = infile.token(line, j)->getTrack();
		if (spineprocess[ptrack]) {
		finalvalues[ptrack] = transval;
		if (!instrumentQ) {
			finalvalues[ptrack] += startvalues[ptrack];
		}
		if (finalvalues[ptrack] != 0) {
			entry = 1;
		}
	} else {
			finalvalues[ptrack] = startvalues[ptrack];
			if (finalvalues[ptrack] != 0) {
				entry = 1;
			}
		}
	}

	if (!entry) {
		return;
	}

	for (j=0; j<infile[line].getFieldCount(); j++) {
		if (!infile.token(line, j)->isKern()) {
			m_humdrum_text << "*";
			if (j < infile[line].getFieldCount()-1) {
				m_humdrum_text << "\t";
			}
			continue;
		}
		ptrack = infile.token(line, j)->getTrack();
		if (finalvalues[ptrack] == 0) {
			m_humdrum_text << "*";
		} else {
			if (instrumentQ) {
				m_humdrum_text << "*ITr";
				m_humdrum_text << Convert::base40ToTrans(-finalvalues[ptrack]);
			} else {
				m_humdrum_text << "*Tr";
				m_humdrum_text << Convert::base40ToTrans(finalvalues[ptrack]);
			}
		}
		if (j < infile[line].getFieldCount()-1) {
			m_humdrum_text << "\t";
		}

	}
	m_humdrum_text << "\n";
}



//////////////////////////////
//
// getTransposeInfo -- returns the Transpose information found in
//    the specified spine starting at the current line, and searching
//    until data is found (or a *- record is found). Return value is a
//    base-40 number.
//

int Tool_transpose::getTransposeInfo(HumdrumFile& infile, int row, int col) {
	int track = infile.token(row, col)->getTrack();
	int ptrack;
	HumRegex hre;
	int base;
	int output = 0;

	for (int i=row; i<infile.getLineCount(); i++) {
		if (infile[i].isData()) {
			break;
		}
		if (!infile[i].isInterpretation()) {
			continue;
		}
		for (int j=0; j<infile[i].getFieldCount(); j++) {
			if (!infile.token(i, j)->isKern()) {
				continue;
			}
			ptrack = infile.token(i, j)->getTrack();
			if (ptrack != track) {
				continue;
			}
			if (hre.search(infile.token(i, j),
					"^\\*Trd[+-]?\\d+c[+-]?\\d+$", "")) {
				base = Convert::transToBase40(*infile.token(i, j));
				output += base;
				// erase the *Tr value because it will be printed elsewhere
				infile.token(i, j)->setText("*XTr");
				// ggg
			}
		}
	}

	return output;
}



//////////////////////////////
//
// Tool_transpose::checkForDeletedLine -- check to see if a "*deletedTr
//

int Tool_transpose::checkForDeletedLine(HumdrumFile& infile, int line) {
	int j;
	if (!infile[line].isInterpretation()) {
		return 0;
	}

	int present = 0;
	int composite = 0;
	for (j=0; j<infile[line].getFieldCount(); j++) {
		if (!infile.token(line, j)->isKern()) {
			continue;
		}
		if (infile.token(line, j)->find("deletedTr") != string::npos) {
			present = 1;
		} else if (infile.token(line, j)->isNull()) {
			// do nothing: not composite
		} else {
			// not a *deletedTr token or a * token, so have to print line later
			composite = 1;
		}
	}

	if (present == 0) {
		// no *deletedTr records found on the currnet line, so process normally
		return 0;
	}

	if (composite == 0) {
		// *deletedTr found, but no other important data found on line.
		return 1;
	}

	// print non-deleted elements in line.
	for (j=0; j<infile[line].getFieldCount(); j++) {;
		if ((string)(*infile.token(line, j)) == "deletedTr") {
			m_humdrum_text << "*";
		} else {
			m_humdrum_text << infile.token(line, j);
		}
		if (j < infile[line].getFieldCount() - 1) {
			m_humdrum_text << "\t";
		}
	}
	m_humdrum_text << "\n";

	return 1;
}



//////////////////////////////
//
// Tool_transpose::processFile --
//

void Tool_transpose::processFile(HumdrumFile& infile,
		vector<bool>& spineprocess) {
	int i;
	int diatonic;
	int j;
	HumRegex hre;
	int interpstart = 0;

	for (i=0; i<infile.getLineCount(); i++) {
		if (!quietQ && (interpstart == 1)) {
			interpstart = 2;
			printTransposeInformation(infile, spineprocess, i, transval);
		}
		if (checkForDeletedLine(infile, i)) {
			continue;
		}

		if (infile[i].isData()) {
			printHumdrumDataRecord(infile[i], spineprocess);
			m_humdrum_text << "\n";
		} else if (infile[i].isInterpretation()) {
			for (j=0; j<infile[i].getFieldCount(); j++) {

				if (infile.token(i, j)->compare(0, 2, "**") == 0) {
						interpstart = 1;
				}

				// check for key signature in a spine which is being
				// transposed, and adjust it. 
				// Should also check tandem spines for updating
				// key signatures in non-kern spines.
				if (spineprocess[infile.token(i, j)->getTrack()] &&
						hre.search(infile.token(i, j),
							"^\\*k\\[([a-gA-G#-]*)\\]", "i")) {
						string value = hre.getMatch(1);
						printNewKeySignature(value, transval);
						if (j<infile[i].getFieldCount()-1) {
						m_humdrum_text << "\t";
						}
						continue;
				}

				// Check for key designations and tranpose
				// if the spine data is being transposed.
				// Should also check tandem spines for updating
				// key designations in non-kern spines.
				if (spineprocess[infile.token(i, j)->getTrack()] &&
						hre.search(infile.token(i, j), "^\\*([A-G])[#-]?:", "i")) {
					diatonic = tolower(hre.getMatch(1)[0]) - 'a';
					if (diatonic >= 0 && diatonic <= 6) {
					  	printNewKeyInterpretation(infile[i], j, transval);
					  	if (j<infile[i].getFieldCount()-1) {
							m_humdrum_text << "\t";
					  	}
					  	continue;
					}
				}
				m_humdrum_text << infile.token(i, j);
				if (j<infile[i].getFieldCount()-1) {
					m_humdrum_text << "\t";
				}
			}
			m_humdrum_text << "\n";

		} else {
			m_humdrum_text << infile[i] << "\n";
		}
	}
}



//////////////////////////////
//
// Tool_transpose::printNewKeySignature --
//

void Tool_transpose::printNewKeySignature(const string& keysig, int trans) {
	int counter = 0;
	int len = (int)keysig.size();
	for (int i=0; i<len; i++) {
		switch(keysig[i]) {
			case '-': counter--; break;
			case '#': counter++; break;
		}
	}

	int xxx = Convert::base40IntervalToLineOfFifths(trans);
	int newkey = xxx + counter;
	m_humdrum_text << Convert::keyNumberToKern(newkey);
}



//////////////////////////////
//
// Tool_transpose::printNewKeyInterpretation --
//

void Tool_transpose::printNewKeyInterpretation(HumdrumLine& aRecord,
		int index, int transval) {
	int mode = 0;
	if (islower(aRecord.token(index)->at(1))) {
		mode = 1;
	}
	int base40 = Convert::kernToBase40(*aRecord.token(index));
	currentkey = base40;
	base40 = base40 + transval;
	base40 = base40 + 4000;
	base40 = base40 % 40;
	base40 = base40 + (3 + mode) * 40;

	m_humdrum_text << "*" << Convert::base40ToKern(base40) << ":";

	HumRegex hre;
	string tvalue = *aRecord.token(index);
	if (hre.search(tvalue, ":(.+)$", "")) {
		string value = hre.getMatch(1);
		m_humdrum_text << value;
	}
}



//////////////////////////////
//
// Tool_transpose::printHumdrumDataRecord --
//

void Tool_transpose::printHumdrumDataRecord(HumdrumLine& record,
		vector<bool>& spineprocess) {
	int i;
	for (i=0; i<record.getFieldCount(); i++) {
		if (!(record.token(i)->isKern() ||
				record.token(i)->isDataType("mxhm"))) {
			// don't try to transpose non-kern and non-mxhm spines
			m_humdrum_text << record.token(i);
			if (i<record.getFieldCount()-1) {
				m_humdrum_text << "\t";
			}
			continue;
		}
		if (!spineprocess[record.token(i)->getTrack()]) {
			// don't try to transpose spines which were not indicated.
			m_humdrum_text << record.token(i);
			if (i<record.getFieldCount()-1) {
				m_humdrum_text << "\t";
			}
			continue;
		}

		if (record.token(i)->isKern()) {
			printHumdrumKernToken(record, i, transval);
		} else if (record.token(i)->isDataType("mxhm")) {
			printHumdrumMxhmToken(record, i, transval);
		} else {
			m_humdrum_text << record.token(i);
		}

		if (i<record.getFieldCount()-1) {
			m_humdrum_text << "\t";
		}
		continue;
	}

}



//////////////////////////////
//
// Tool_transpose::printHumdrumKernToken --
//

void Tool_transpose::printHumdrumKernToken(HumdrumLine& record, int index,
		int transval) {
	if (record.token(index)->isNull()) {
		// null record element (no pitch).
		m_humdrum_text << record.token(index);
		return;
	}
	if (!record.token(index)->isKern()) {
		m_humdrum_text << record.token(index);
		return;
	}
	string buffer;
	int tokencount = record.token(index)->getSubtokenCount();
	for (int k=0; k<tokencount; k++) {
		buffer = record.token(index)->getSubtoken(k);
		printNewKernString(buffer, transval);
		if (k<tokencount-1) {
			m_humdrum_text << " ";
		}
	}
}



//////////////////////////////
//
// Tool_transpose::printHumdrumMxhmToken --
//

void Tool_transpose::printHumdrumMxhmToken(HumdrumLine& record, int index,
		int transval) {
	if (record.token(index)->isNull()) {
		// null record element (nothing to do)
		m_humdrum_text << record.token(index);
		return;
	}
	if (!record.token(index)->isDataType("mxhm")) {
		m_humdrum_text << record.token(index);
		return;
	}
	HumRegex hre;
	if (hre.search(record.token(index), "N\\.C\\.")) {
		// no pitch information so just echo the text:
		m_humdrum_text << record.token(index);
	} else if (hre.search(record.token(index), "([A-Ga-g]+[n#-]{0,2})")) {
		string pitch = hre.getMatch(1);
		int b40 = Convert::kernToBase40(pitch) + transval;
		b40 = b40 % 40 + 120;
		pitch = Convert::base40ToKern(b40);
		string newtext = *record.token(index);
		hre.replaceDestructive(newtext, pitch, "([A-Ga-g]+[n#-]{0,2})");
		m_humdrum_text << newtext;
	} else {
		m_humdrum_text << record.token(index);
		return;
	}
}




//////////////////////////////
//
// Tool_transpose::printNewKernString --
//

void Tool_transpose::printNewKernString(const string& input, int transval) {
	HumRegex hre;
	if (input.rfind('r') != string::npos) {
		string output = input;
		if (hre.search(input, "([A-Ga-g]+[#n-]*)")) {
			// transpose pitch portion of rest (indicating vertical position)
			string pitch = hre.getMatch(1);
			int base40 = Convert::kernToBase40(pitch);
			string newpitch = Convert::base40ToKern(base40 + transval);
			hre.replaceDestructive(newpitch, "", "[-#n]+");
			hre.replaceDestructive(output, newpitch, "([A-Ga-g]+[#n-]*)");
		}
		// don't transpose rests...
		m_humdrum_text << output;
		return;
	}
	if (input == ".") {
		// don't transpose null tokens...
		m_humdrum_text << input;
		return;
	}

	int base40 = Convert::kernToBase40(input);
	string newpitch = Convert::base40ToKern(base40 + transval);

	// consider interaction of #X -X n interaction vs. nX.
	string output;
	if (hre.search(input, "([A-Ga-g#n-]+)")) {
		string oldpitch = hre.getMatch(1);
		output = hre.replaceCopy(input, newpitch, oldpitch);
	}
	m_humdrum_text << output;
}



//////////////////////////////
//
// Tool_transpose::getBase40ValueFromInterval -- note: only ninth interval range allowed
//

int Tool_transpose::getBase40ValueFromInterval(const string& interval) {
	int sign = 1;
	if (interval.find('-') != string::npos) {
		sign = -1;
	}

	string icopy = interval;
	for (int i=0; i<(int)icopy.size(); i++) {
		if (icopy[i] == 'p') { icopy[i] = 'P'; }
		if (icopy[i] == 'a') { icopy[i] = 'A'; }
		if (icopy[i] == 'D') { icopy[i] = 'd'; }
	}

	int output = 0;

	if (icopy.find("dd1") != string::npos)      { output = -2; }
	else if (icopy.find("d1") != string::npos)  { output = -1; }
	else if (icopy.find("P1") != string::npos)  { output =  0; }
	else if (icopy.find("AA1") != string::npos) { output =  2; }
	else if (icopy.find("A1") != string::npos)  { output =  1; }

	else if (icopy.find("dd2") != string::npos) { output =  3; }
	else if (icopy.find("d2") != string::npos)  { output =  4; }
	else if (icopy.find("m2") != string::npos)  { output =  5; }
	else if (icopy.find("M2") != string::npos)  { output =  6; }
	else if (icopy.find("AA2") != string::npos) { output =  8; }
	else if (icopy.find("A2") != string::npos)  { output =  7; }

	else if (icopy.find("dd3") != string::npos) { output =  9; }
	else if (icopy.find("d3") != string::npos)  { output = 10; }
	else if (icopy.find("m3") != string::npos)  { output = 11; }
	else if (icopy.find("M3") != string::npos)  { output = 12; }
	else if (icopy.find("AA3") != string::npos) { output = 14; }
	else if (icopy.find("A3") != string::npos)  { output = 13; }

	else if (icopy.find("dd4") != string::npos) { output = 15; }
	else if (icopy.find("d4") != string::npos)  { output = 16; }
	else if (icopy.find("P4") != string::npos)  { output = 17; }
	else if (icopy.find("AA4") != string::npos) { output = 19; }
	else if (icopy.find("A4") != string::npos)  { output = 18; }

	else if (icopy.find("dd5") != string::npos) { output = 21; }
	else if (icopy.find("d5") != string::npos)  { output = 22; }
	else if (icopy.find("P5") != string::npos)  { output = 23; }
	else if (icopy.find("AA5") != string::npos) { output = 25; }
	else if (icopy.find("A5") != string::npos)  { output = 24; }

	else if (icopy.find("dd6") != string::npos) { output = 26; }
	else if (icopy.find("d6") != string::npos)  { output = 27; }
	else if (icopy.find("m6") != string::npos)  { output = 28; }
	else if (icopy.find("M6") != string::npos)  { output = 29; }
	else if (icopy.find("AA6") != string::npos) { output = 31; }
	else if (icopy.find("A6") != string::npos)  { output = 30; }

	else if (icopy.find("dd7") != string::npos) { output = 32; }
	else if (icopy.find("d7") != string::npos)  { output = 33; }
	else if (icopy.find("m7") != string::npos)  { output = 34; }
	else if (icopy.find("M7") != string::npos)  { output = 35; }
	else if (icopy.find("AA7") != string::npos) { output = 37; }
	else if (icopy.find("A7") != string::npos)  { output = 36; }

	else if (icopy.find("dd8") != string::npos) { output = 38; }
	else if (icopy.find("d8") != string::npos)  { output = 39; }
	else if (icopy.find("P8") != string::npos)  { output = 40; }
	else if (icopy.find("AA8") != string::npos) { output = 42; }
	else if (icopy.find("A8") != string::npos)  { output = 41; }

	else if (icopy.find("dd9") != string::npos) { output = 43; }
	else if (icopy.find("d9") != string::npos)  { output = 44; }
	else if (icopy.find("m9") != string::npos)  { output = 45; }
	else if (icopy.find("M9") != string::npos)  { output = 46; }
	else if (icopy.find("AA9") != string::npos) { output = 48; }
	else if (icopy.find("A9") != string::npos)  { output = 47; }

	return output * sign;
}



//////////////////////////////
//
// Tool_transpose::example --
//

void Tool_transpose::example(void) {


}



//////////////////////////////
//
// Tool_transpose::usage --
//

void Tool_transpose::usage(const string& command) {

}



///////////////////////////////////////////////////////////////////////////
//
// Automatic transposition functions
//


//////////////////////////////
//
// Tool_transpose::doAutoTransposeAnalysis --
//

void Tool_transpose::doAutoTransposeAnalysis(HumdrumFile& infile) {
	vector<int> ktracks(infile.getMaxTrack()+1, 0);

	vector<HTp> tracks;
	infile.getTrackStartList(tracks);
	int i;
	for (i=0; i<(int)tracks.size(); i++) {
		if (tracks[i]->isKern()) {
			ktracks[i] = tracks[i]->getTrack();
		} else {
			ktracks[i] = 0;
		}
	}

	int segments = int(infile.getScoreDuration().getFloat()+0.5);
	if (segments < 1) {
		segments = 1;
	}

	vector<vector<vector<double> > > trackhist;
	trackhist.resize(ktracks.size());

	for (i=1; i<(int)trackhist.size(); i++) {
		if (ktracks[i]) {
			storeHistogramForTrack(trackhist[i], infile, i, segments);
		}
	}

	if (debugQ) {
		m_free_text << "Segment pitch histograms: " << endl;
		printHistograms(segments, ktracks, trackhist);
	}

	int level = 16;
	int hop   = 8;
	int count = segments / hop;

	if (segments < count * level / (double)hop) {
		level = level / 2;
		hop   = hop / 2;
	}
	if (segments < count * level / (double)hop) {
		count = count / 2;
	}

	if (segments < count * level / (double)hop) {
		level = level / 2;
		hop   = hop / 2;
	}
	if (segments < count * level / (double)hop) {
		count = count / 2;
	}

	vector<vector<vector<double> > > analysis;

	doAutoKeyAnalysis(analysis, level, hop, count, segments, ktracks, trackhist);

	// print analyses raw results

	m_free_text << "Raw key analysis by track:" << endl;
	printRawTrackAnalysis(analysis, ktracks);

	doTranspositionAnalysis(analysis);
}



//////////////////////////////
//
// Tool_transpose::doTranspositionAnalysis --
//

void Tool_transpose::doTranspositionAnalysis(vector<vector<vector<double> > >& analysis) {
	int i, j, k;
	int value1;
	int value2;
	int value;

	for (i=0; i<1; i++) {
		for (j=2; j<3; j++) {
			for (k=0; k<(int)analysis[i].size(); k++) {
				if (analysis[i][k][24] >= 0 && analysis[j][k][24] >= 0) {
					value1 = (int)analysis[i][k][25];
	 				if (value1 >= 12) {
						  value1 = value1 - 12;
					}
					value2 = (int)analysis[j][k][25];
	 				if (value2 >= 12) {
						  value2 = value2 - 12;
					}
					value = value1 - value2;
					if (value < 0) {
						  value = value + 12;
					}
					if (value > 6) {
						  value = 12 - value;
					}
					m_free_text << value << endl;
				}
			}
		}
	}
}



//////////////////////////////
//
// Tool_transpose::printRawTrackAnalysis --
//

void Tool_transpose::printRawTrackAnalysis(vector<vector<vector<double> > >& analysis,
		vector<int>& ktracks) {

	int i, j;
	int value;
	int value2;

	for (i=0; i<(int)analysis[0].size(); i++) {
		m_free_text << "Frame\t" << i << ":";
		for (j=0; j<(int)analysis.size(); j++) {
			m_free_text << "\t";
	 		value = (int)analysis[j][i][24];
	 		if (value >= 12) {
				value = value - 12;
			}
	 		value2 = (int)analysis[j][i][25];
	 		if (value2 >= 12) {
				value2 = value2 - 12;
			}
			m_free_text << value;
	 		// if (value != value2) {
	 		//    m_free_text << "," << value2;
			// }
		}
		m_free_text << "\n";
	}
}



//////////////////////////////
//
// doAutoKeyAnalysis --
//

void Tool_transpose::doAutoKeyAnalysis(vector<vector<vector<double> > >& analysis, int level,
		int hop, int count, int segments, vector<int>& ktracks,
		vector<vector<vector<double> > >& trackhist) {

	vector<double> majorweights;
	vector<double> minorweights;
	fillWeightsWithKostkaPayne(majorweights, minorweights);

	int size = 0;
	int i;
	for (i=1; i<(int)ktracks.size(); i++) {
		if (ktracks[i]) {
			size++;
		}
	}

	analysis.resize(size);
	for (i=0; i<(int)analysis.size(); i++) {
		analysis[i].reserve(count);
	}

	int aindex = 0;
	for (i=1; i<(int)ktracks.size(); i++) {
		if (!ktracks[i]) {
			continue;
		}
		doTrackKeyAnalysis(analysis[aindex++], level, hop, count,
				trackhist[i], majorweights, minorweights);
	}
}



//////////////////////////////
//
// Tool_transpose::doTrackKeyAnalysis -- Do individual key analyses of sections of the
//   given track.
//

void Tool_transpose::doTrackKeyAnalysis(vector<vector<double> >& analysis, int level, int hop,
		int count, vector<vector<double> >& trackhist,
		vector<double>& majorweights, vector<double>& minorweights) {

	int i;
	for (i=0; i<count; i++) {
		if (i * hop + level > (int)trackhist.size()) {
			break;
		}
		analysis.resize(i+1);
		doSingleAnalysis(analysis[analysis.size()-1], i*hop+level, level,
				trackhist, majorweights, minorweights);
	}
}



//////////////////////////////
//
// Tool_transpose::doSingleAnalysis --
//

void Tool_transpose::doSingleAnalysis(vector<double>& analysis, int startindex, int length,
		vector<vector<double> >& trackhist, vector<double>& majorweights,
		vector<double>& minorweights) {
	vector<double> histsum(12, 0);

	for (int i=0; (i<length) && (startindex+i+length<(int)trackhist.size()); i++) {
		for (int k=0; k<12; k++) {
			histsum[k] += trackhist[i+startindex][k];
		}
	}

	identifyKey(analysis, histsum, majorweights, minorweights);
}



///////////////////////////////
//
// Tool_transpose::fillWeightsWithKostkaPayne --
//

void Tool_transpose::fillWeightsWithKostkaPayne(vector<double>& maj, vector<double>& min) {
	maj.resize(12);
	min.resize(12);

	// found in David Temperley: Music and Probability 2006
	maj[0]  = 0.748;	// C major weights
	maj[1]  = 0.060;	// C#
	maj[2]  = 0.488;	// D
	maj[3]  = 0.082;	// D#
	maj[4]  = 0.670;	// E
	maj[5]  = 0.460;	// F
	maj[6]  = 0.096;	// F#
	maj[7]  = 0.715;	// G
	maj[8]  = 0.104;	// G#
	maj[9]  = 0.366;	// A
	maj[10] = 0.057;	// A#
	maj[11] = 0.400;	// B
	min[0]  = 0.712;	// c minor weights
	min[1]  = 0.084;	// c#
	min[2]  = 0.474;	// d
	min[3]  = 0.618;	// d#
	min[4]  = 0.049;	// e
	min[5]  = 0.460;	// f
	min[6]  = 0.105;	// f#
	min[7]  = 0.747;	// g
	min[8]  = 0.404;	// g#
	min[9]  = 0.067;	// a
	min[10] = 0.133;	// a#
	min[11] = 0.330;	// b
}



////////////////////////////////////////
//
// identifyKey -- correls contains the 12 major key correlation
//      values, then the 12 minor key correlation values, then two
//      more values: index=24 is the best key, and index=25 is the
//      second best key.  If [24] or [25] is -1, then that means that
//      all entries in the original histogram were zero (all rests).
//

void Tool_transpose::identifyKey(vector<double>& correls,
		vector<double>& histogram, vector<double>& majorweights,
		vector<double>& minorweights) {

	correls.clear();
	correls.reserve(26);

	double testsum = 0.0;
	for (int i=0; i<12; i++) {
		testsum += histogram[i];
	}
	if (testsum == 0.0) {
		correls.resize(26);
		fill(correls.begin(), correls.end(), -1);
		correls[24] = -1;
		correls[25] = -1;
		return;
	}

	vector<double> majorcorrels;
	vector<double> minorcorrels;
	for (int i=0; i<12; i++) {
		majorcorrels[i] = Convert::pearsonCorrelation(majorweights, histogram);
		minorcorrels[i] = Convert::pearsonCorrelation(minorweights, histogram);
	}

	// find max value
	int besti;
	int majorbesti = 0;
	int minorbesti = 0;
	for (int i=1; i<12; i++) {
		if (majorcorrels[i] > majorcorrels[majorbesti]) {
			majorbesti = i;
		}
		if (minorcorrels[i] > minorcorrels[minorbesti]) {
			minorbesti = i;
		}
	}
	besti = majorbesti;
	if (majorcorrels[majorbesti] < minorcorrels[minorbesti]) {
		besti = minorbesti + 12;
	}

	// find second best major key
	int majorsecondbesti = 0;
	if (majorbesti == 0) {
		majorsecondbesti = 1;
	}
	for (int i=1; i<12; i++) {
		if (i == majorbesti) {
			continue;
		}
		if (majorcorrels[i] > majorcorrels[majorsecondbesti]) {
			majorsecondbesti = i;
		}
	}

	// find second best minor key
	int minorsecondbesti = 0;
	if (minorbesti == 0) {
		minorsecondbesti = 1;
	}
	for (int i=1; i<12; i++) {
		if (i == minorbesti) {
			continue;
		}
		if (minorcorrels[i] > minorcorrels[minorsecondbesti]) {
			minorsecondbesti = i;
		}
	}

	int secondbesti = majorsecondbesti;
	if (majorcorrels[majorsecondbesti] < minorcorrels[minorsecondbesti]) {
		secondbesti = minorsecondbesti;
	}
	secondbesti += 12;

	correls = majorcorrels;
	correls.insert(correls.end(), minorcorrels.begin(), minorcorrels.end());
	correls.push_back(besti);
	correls.push_back(secondbesti);
}



//////////////////////////////
//
// Tool_transpose::printHistograms --
//

void Tool_transpose::printHistograms(int segments, vector<int> ktracks,
vector<vector<vector<double> > >& trackhist) {
	int i, j, k;
	int start;

	for (i=0; i<segments; i++) {
//m_free_text << "i=" << i << endl;
		m_free_text << "segment " << i
				<< " ==========================================\n";
		for (j=0; j<12; j++) {
			start = 0;
//m_free_text << "j=" << i << endl;
			for (k=1; k<(int)ktracks.size(); k++) {
//m_free_text << "k=" << i << endl;
				if (!ktracks[k]) {
					continue;
				}
				if (!start) {
					m_free_text << j;
					start = 1;
				}
				m_free_text << "\t";
				m_free_text << trackhist[k][i][j];
			}
	 if (start) {
				m_free_text << "\n";
			}
		}
	}
	m_free_text << "==========================================\n";
}



//////////////////////////////
//
// Tool_transpose::storeHistogramForTrack --
//

double Tool_transpose::storeHistogramForTrack(vector<vector<double> >& histogram,
		HumdrumFile& infile, int track, int segments) {

	histogram.clear();
	histogram.reserve(segments);

	int i;
	int j;
	int k;

	for (i=0; i<(int)histogram.size(); i++) {
		histogram[i].resize(12);
		fill(histogram[i].begin(), histogram[i].end(), 0);
	}

	double totalduration = infile.getScoreDuration().getFloat();

	double duration;
	string buffer;
	int pitch;
	double start;
	int tokencount;
	for (i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isData()) {
			continue;
		}
		start = infile[i].getDurationFromStart().getFloat();
		for (j=0; j<infile[i].getFieldCount(); j++) {
			if (!infile.token(i, j)->isKern()) {
				continue;
			}
	 		if (infile.token(i, j)->getTrack() != track) {
				continue;
			}
			if (!infile.token(i, j)->isKern()) {
				continue;
			}
			if (!infile.token(i, j)->isNull()) {
				continue;
			}
			tokencount = infile.token(i, j)->getSubtokenCount();
			for (k=0; k<tokencount; k++) {
				buffer = *infile.token(j, k);
				if (buffer == ".") {
					continue;  // ignore illegal inline null tokens
				}
				pitch = Convert::kernToMidiNoteNumber(buffer);
				if (pitch < 0) {
					continue;  // ignore rests or strange objects
				}
				pitch = pitch % 12;  // convert to chromatic pitch-class
				duration = Convert::recipToDuration(buffer).getFloat();
				if (duration <= 0.0) {
					continue;   // ignore grace notes and strange objects
				}
				addToHistogramDouble(histogram, pitch,
						  start, duration, totalduration, segments);
			}
		}
	}

	return totalduration;
}



//////////////////////////////
//
// Tool_transpose::addToHistogramDouble -- fill the pitch histogram in the right spots.
//

void Tool_transpose::addToHistogramDouble(vector<vector<double> >& histogram, int pc,
		double start, double dur, double tdur, int segments) {

	pc = (pc + 12) % 12;

	double startseg = start / tdur * segments;
	double startfrac = startseg - (int)startseg;

	double segdur = dur / tdur * segments;

	if (segdur <= 1.0 - startfrac) {
		histogram[(int)startseg][pc] += segdur;
		return;
	} else if (1.0 - startfrac > 0.0) {
		histogram[(int)startseg][pc] += (1.0 - startfrac);
		segdur -= (1.0 - startfrac);
	}

	int i = (int)(startseg + 1);
	while (segdur > 0.0 && i < (int)histogram.size()) {
		if (segdur < 1.0) {
			histogram[i][pc] += segdur;
			segdur = 0.0;
		} else {
			histogram[i][pc] += 1.0;
			segdur -= 1.0;
		}
		i++;
	}
}



//////////////////////////////
//
// Tool_transpose::initialize --
//

void Tool_transpose::initialize(HumdrumFile& infile) {

	// handle basic options:
	if (getBoolean("author")) {
		m_free_text << "Written by Craig Stuart Sapp, "
			  << "craig@ccrma.stanford.edu, 12 Apr 2004" << endl;
		exit(0);
	} else if (getBoolean("version")) {
		m_free_text << getArg(0) << ", version: 10 Dec 2016" << endl;
		m_free_text << "compiled: " << __DATE__ << endl;
		exit(0);
	} else if (getBoolean("help")) {
		usage(getArg(0));
		exit(0);
	} else if (getBoolean("example")) {
		example();
		exit(0);
	}

	transval     =  getInteger("base40");
	ssettonicQ   =  getBoolean("settonic");
	ssettonic    =  Convert::kernToBase40(getString("settonic").c_str());
	autoQ        =  getBoolean("auto");
	debugQ       =  getBoolean("debug");
	spinestring  =  getString("spines");
	octave       =  getInteger("octave");
	concertQ     =  getBoolean("concert");
	writtenQ     =  getBoolean("written");
	quietQ       = !getBoolean("transcode");
	instrumentQ  =  getBoolean("instrument");

	switch (getBoolean("diatonic") + getBoolean("chromatic")) {
		case 1:
			cerr << "Error: both -d and -c options must be specified" << endl;
			exit(1);
			break;
		case 2:
			{
				char buffer[128] = {0};
				sprintf(buffer, "d%dc%d", getInt("d"), getInt("c"));
				transval = Convert::transToBase40(buffer);
			}
			break;
	}

	ssettonic = ssettonic % 40;

	if (getBoolean("transpose")) {
		transval = getBase40ValueFromInterval(getString("transpose").c_str());
	}

	transval += 40 * octave;
}


// END_MERGE

} // end namespace hum



/* BRIEF DOCUMENTATION


transpose options:

   -t interval = transpose music by the specified interval, where
                 interval is of the form:
                    P1 = perfect unison (no transposition)
                    m2 = up a minor second
                   -m2 = down a minor second
                    M3 = up a major third
                   -A4 = down an augmented fourth
                    d5 = up a diminished fifth

   -b interval = transpose by the base-40 equivalent to the -t option interval
                     0 = perfect unison (no transposition)
                     6 = up a minor second
                    -6 = down a minor second
                    12 = up a major third
                   -18 = down an augmented fourth
                    22 = up a diminished fifth


   -o octave  = transpose (additionally by given octave)
		transpose -t m3 -o 1 = transpose up by an octave and a minor
			               third.


   -s fieldstring = transpose only the given list of data spines.
		    example:
 			transpose -f1-2,4 -t P4 fourpart.krn
 			transpose -f3     -t P4 fourpart.krn
		   Note: does not work yet with the -k option



##########################################################################
##
## EXAMPLES
##



input: file.krn  -- an example with the key being a minor:

**kern
*a:
4A
4B
4c
4d
4e
4f
4g
4a
*-



#####################################################################
#
# Transpose the file up a minor third (so that it is in C Minor):
#

tranpose -t m3 file.krn

**kern
*c:
4c
4d
4e-
4f
4g
4a-
4b-
4cc
*-



#####################################################################
#
# Transpose the file down a minor third (so that it is in F# Minor):
#

tranpose -t -m3 file.krn

**kern
*f#:
4F#
4G#
4A
4B
4c#
4d
4e
4f#
*-


#####################################################################
#
# Transpose the file up a perfect octave:
#

tranpose -t P8 file.krn

**kern
*a:
4A
4B
4cc
4dd
4ee
4ff
4gg
4aa
*-



#####################################################################
#
# Force the file to a tonic on C rather than a:
#

transpose -k c file.krn

**kern
*c:
4c
4d
4e-
4f
4g
4a-
4b-
4cc
*-


# case -k option value is irrelevant:

transpose -k C file.krn

**kern
*c:
4c
4d
4e-
4f
4g
4a-
4b-
4cc
*-


# Transpose from A Minor to G# Minor:

transpose -k G# file.krn

**kern
*g#:
4G#
4A#
4B
4c#
4d#
4e
4f#
4g#
*-


# Changing input files to:

**kern
*C:
4c
4d
4e
4f
*G:
4g
4a
4b
4cc
*-


# Using -k option will convert all keys to same in output:

transpose -k e file.krn

**kern
*E:
4e
4f#
4g#
4a
*E:
4e
4f#
4g#
4a
*-



##############################
##
## octave transpositions
##


# Back to original data file, transposing up a minor tenth:

transpose -o 1 -t m3 file.krn

**kern
*E-:
8ee-
8ff
8gg
8aa-
8bb-
8ccc
8ddd
8eee-
*-

# transpose down two octaves:

transpose -o -2 file.krn

**kern
*a:
4AAA
4BBB
4CC
4DD
4EE
4FF
4GG
4AA
*-


####################
##
## base 40 -b option instead of -t option
##


# Transpose down two octaves:
transpose -b -80 file.krn

**kern
*a:
4AAA
4BBB
4CC
4DD
4EE
4FF
4GG
4AA
*-

*/



