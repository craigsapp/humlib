//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Thu Sep  9 21:30:46 PDT 2004
// Last Modified: Tue Jul 22 19:34:37 CEST 2025
// Filename:      src/tool-barnum.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/tool-barnum.cpp
// Syntax:        C++11; humlib
//
// Description:   Number, renumber or remove measure numbers from Humdrum files.
//

#include "tool-barnum.h"
#include "Convert.h"

using namespace std;

namespace hum {

// START_MERGE


//////////////////////////////
//
// Tool_barnum::Tool_barnum -- Constructor
//

Tool_barnum::Tool_barnum(void) {
	define("r|remove=b", "Remove bar numbers from the file");
	define("s|start=i:1", "Starting bar number");
	define("a|all=b",     "Print numbers on all barlines");
	define("debug=b",     "Print debug info");
}



/////////////////////////////////
//
// Tool_barnum::run -- Do the main work of the tool.
//

bool Tool_barnum::run(HumdrumFileSet& infiles) {
	bool status = true;
	for (int i=0; i<infiles.getCount(); i++) {
		status &= run(infiles[i]);
	}
	return status;
}


bool Tool_barnum::run(const string& indata, ostream& out) {
	HumdrumFile infile(indata);
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_barnum::run(HumdrumFile& infile, ostream& out) {
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_barnum::run(HumdrumFile& infile) {
	initialize();
	processFile(infile);
	return true;
}



///////////////////////////////
//
// Tool_barnum::removeBarNumbers -- You guessed it.
//

void Tool_barnum::removeBarNumbers(HumdrumFile& infile) {
	int i;
	for (i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isBarline()) {
			m_humdrum_text << infile[i] << "\n";
		} else {
			printWithoutBarNumbers(infile[i]);
		}
	}
}



//////////////////////////////
//
// Tool_barnum::printWithoutBarNumbers --
//

void Tool_barnum::printWithoutBarNumbers(HumdrumLine& humline) {
	if (!humline.isBarline()) {
		m_humdrum_text << humline << endl;
		return;
	}
	for (int i=0; i<humline.getFieldCount(); i++) {
		string& token = *humline.token(i);
		if (token[0] != '=') {
			m_humdrum_text << token;
		} else {
			for (int j=0; j<(int)token.size(); j++) {
				if (!std::isdigit(token[j])) {
					m_humdrum_text << token[j];
				}
			}
		}
		if (i < humline.getFieldCount()-1) {
			m_humdrum_text << "\t";
		}
	}
	m_humdrum_text << "\n";
}



//////////////////////////////
//
// Tool_barnum::getEndingBarline -- Return the index of the last barline,
//      returning -1 if none.  Ending barline is defined as the
//      last barline after all data records.
//

int Tool_barnum::getEndingBarline(HumdrumFile& infile) {
	int i;
	for (i=infile.getLineCount()-1; i>=0; i--) {
		if (infile[i].isData()) {
			return -1;
		} else if (infile[i].isBarline()) {
			return i;
		}
	}

	return -1;
}



//////////////////////////////
//
// Tool_barnum::processFile --
//

void Tool_barnum::processFile(HumdrumFile& infile) {
	if (m_removeQ) {
		removeBarNumbers(infile);
		return;
	}

	vector<int>    measureline;   // line number in the file where measure occur
	vector<HumNum> measuredurs;  // duration of measure
	vector<HumNum> timesigdurs;  // duration according to timesignature
	vector<int>    control;       // control = numbered measure
	vector<int>    measurenums;   // output measure numbers

	measureline.reserve(infile.getLineCount());
	measuredurs.reserve(infile.getLineCount());
	timesigdurs.reserve(infile.getLineCount());

	HumNum timesigdur = 0;
	for (int i=0; i<infile.getLineCount(); i++) {
		if (m_debugQ) {
			m_humdrum_text << "!! LINE " << i+1 << "\t" << infile[i] << endl;
		}
		if (infile[i].isInterpretation()) {
			for (int j=0; j<infile[i].getFieldCount(); j++) {
				HTp token = infile.token(i, j);
				if ((token->compare(0, 2, "*M") == 0)
						&& (infile.token(i, j)->find('/') != string::npos)) {
					timesigdur = Convert::timeSigToDurationInQuarters(token);
					if (m_debugQ) {
						cerr << "!!! TSIG=" << token << "\tDUR=" << timesigdur << endl;
					}
					// Update time signature for measure (only one time signature per measue)
					// but looping through all time signatures for all parts right now.
					if (timesigdurs.size() > 0) {
						timesigdurs[(int)timesigdurs.size()-1] = timesigdur;
					}
				}
			}
		} else if (infile[i].isBarline()) {
			measureline.push_back(i);
			HumNum bardur = infile[i].getDurationToBarline();
			measuredurs.push_back(bardur);
			timesigdurs.push_back(timesigdur);
		}
	}

	if (m_debugQ) {
		m_humdrum_text << "!! measure beats / timesig beats" << endl;
		for (int i=0; i<(int)measuredurs.size(); i++) {
			m_humdrum_text << measuredurs[i] << "\t" << timesigdurs[i] << endl;
		}
	}

	if (measuredurs.size() == 0) {
		// no barlines, nothing to do...
		m_humdrum_text << infile;
		return;
	}

	// Identify controlling/non-controlling barlines
	// at each measure line determine one of three cases:
	//
	// (1) all ok -- the summation of durations in the measure
	//     matches the current time sign
	// (2) a partial measure -- the measure durations do not
	//     add up to the time signature, but the measure is
	//     at the start/end of a musical section such as the
	//     beginning of a piece, end of a piece, or between
	//     repeat bar dividing a full measure.
	// (3) the sum of the durations does not match the
	//     time signature because the durations are incorrectly
	//     given.
	//

	control.resize(measureline.size());
	measurenums.resize(measureline.size());
	if (m_debugQ) {
		cerr << "!! SETTING MEASURE NUMBERS TO SIZE " << measurenums.size() << endl;
	}
	std::fill(control.begin(), control.end(), -1);
	std::fill(measurenums.begin(), measurenums.end(), -1);

	// If the time signature and the number of beats in a measure
	// agree, then the bar is worth numbering:
	for (int i=0; i<(int)control.size(); i++) {
		if (m_debugQ) {
			cerr << "!! MEASURE BEATS " << i << " = " << measuredurs[i]
			     << "\tTIMESIGBEATS = " << timesigdurs[i] << endl;
		}
		if (measuredurs[i] == timesigdurs[i]) {
			control[i] = 1;
		}
	}

	// Determine first bar (which is marked with a negative value
	// if there is a pickup bar)
	if (measuredurs[0] < 0) {
		if (-measuredurs[0] == timesigdurs[0]) {
			control[0] = 1;
		}
	}

	// Check for intermediate barlines which split one measure
	for (int i=0; i<(int)control.size()-2; i++) {
		if ((control[i] == 1) || (control[i+1] == 1)) {
			continue;
		}
		if (timesigdurs[i] != timesigdurs[i+1]) {
			continue;
		}
		if ((measuredurs[i]+measuredurs[i+1]) == timesigdurs[i]) {
			// found a barline which splits a complete measure
			// into two pieces.
			control[i] = 1;
			control[i+1] = 0;
			i++;
		}
	}

	// if two (or more) non-controlling bars occur in a row, then
	// make them controlling:
	//for (int i=0; i<control.size()-1; i++) {
	//   if ((control[i] < 1) && (control[i+1] < 1)) {
	//      while ((i < control.size()) && (control[i] < 1)) {
	//         control[i++] = 1;
	//      }
	//   }
	//}

	for (int i=0; i<(int)control.size()-1; i++) {
		if ((control[i] == 0) && (control[i+1] < 0)) {
			control[i+1] = 1;
		}
	}

	// if a measure contains no beats, then it is not a controlling barline
	for (int i=0; i<(int)control.size(); i++) {
		if (measuredurs[i] == 0) {
			control[i] = 0;
		}
	}

	// if the first bar is not a pickup measure, but there is no
	// starting measure, then subtract one from the starting barline
	// count;
	int offset = 0;
	int dataq = 0;
	for (int i=0; i<infile.getLineCount(); i++) {
		if (infile[i].isData()) {
			dataq = 1;
			continue;
		}
		if (infile[i].isBarline()) {
			if ((measuredurs[0] > 0) && dataq) {
				offset = 1;
			}
			break;
		}
	}

	// if the last bar is incomplete, but the bar before it
	// is not incomplete, then allow barline on last measure,
	// excluding any ending barlines with no data after them.
	for (int i=(int)control.size()-1; i>=0; i--) {
		if (control[i] == 0) {
			continue;
		}
		if ((control[i] < 0) && (i > 0) && (control[i-1] > 0)) {
			control[i] = 1;
		}
		break;
	}

	if (m_allQ) {
		std::fill(control.begin(), control.end(), 1);
		offset = 0;
	}

	// if there is no time data, just label each barline
	// as a new measure.
	if (infile[infile.getLineCount()-1].getBeat() == 0.0) {
		for (int i=0; i<(int)control.size(); i++) {
			control[i] = 1;
		}
		// don't mark the last barline if there is no data
		// line after it.
		for (int i=infile.getLineCount()-1; i>=0; i--) {
			if (infile[i].isData()) {
				break;
			}
			if (infile[i].isBarline()) {
				control.back() = -1;
				break;
			}
		}
	}

	// assign the measure numbers;
	int mnum = m_startnum + offset;
	for (int i=0; i<(int)measurenums.size(); i++) {
		if (control[i] == 1) {
			measurenums[i] = mnum++;
		}
	}

	if (m_debugQ) {
		m_humdrum_text << "!! cont\tnum\tbeats\ttbeats" << endl;
		for (int i=0; i<(int)control.size(); i++) {
			m_humdrum_text << "!!" << control[i] << "\t" << measurenums[i] << "\t"
				  << measuredurs[i] << "\t" << timesigdurs[i] << endl;
		}
	}

	int endingbarline = getEndingBarline(infile);

	// ready to print the new barline numbers
	int mindex = 0;
	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isBarline()) {
			m_humdrum_text << infile[i] << "\n";
			continue;
		}

		if (endingbarline == i) {
			printWithoutBarNumbers(infile[i]);
		} else if (measurenums[mindex] < 0) {
			printWithoutBarNumbers(infile[i]);
		} else {
			printWithBarNumbers(infile[i], measurenums[mindex]);
		}

		mindex++;
	}

}



//////////////////////////////
//
// printWithBarNumbers --
//

void Tool_barnum::printWithBarNumbers(HumdrumLine& humline, int measurenum) {
	if (!humline.isBarline()) {
		m_humdrum_text << humline << endl;
		return;
	}
	for (int i=0; i<humline.getFieldCount(); i++) {
		string& token = *humline.token(i);
		printSingleBarNumber(token,  measurenum);
		if (i < humline.getFieldCount() -1) {
			 m_humdrum_text << "\t";
		 }
	}
	m_humdrum_text << "\n";
}



//////////////////////////////
//
// Tool_barnum::printSingleBarNumber --
//

void Tool_barnum::printSingleBarNumber(const string& astring, int measurenum) {
	for (int i=0; i<(int)astring.size(); i++) {
		if ((astring[i] == '=') && (astring[i+1] != '=')) {
			m_humdrum_text << astring[i] << measurenum;
		} else if (!isdigit(astring[i])) {
			m_humdrum_text << astring[i];
		}
	}
}



//////////////////////////////
//
// Tool_barnum::initialize -- validate and process command-line options.
//


void Tool_barnum::initialize(void) {
	m_removeQ  = getBoolean("remove");
	m_startnum = getInteger("start");
	m_debugQ   = getBoolean("debug");
	m_allQ     = getBoolean("all");
}


// END_MERGE

} // end namespace hum



