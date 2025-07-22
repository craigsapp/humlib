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
			cout << infile[i] << "\n";
			continue;
		}
		printWithoutBarNumbers(infile[i]);
	}
}



//////////////////////////////
//
// Tool_barnum::printWithoutBarNumbers --
//

void Tool_barnum::printWithoutBarNumbers(HumdrumLine& humline) {
	for (int i=0; i<humline.getFieldCount(); i++) {
		string& token = *humline.token(i);
		if (token[0] != '=') {
			cout << token;
		} else {
			for (int j=0; j<(int)token.size(); j++) {
				if (!std::isdigit(token[j])) {
					cout << token[j];
				}
			}
		}
		if (i < humline.getFieldCount()-1) {
			cout << "\t";
		}
	}
	cout << "\n";
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
	vector<HumNum> measurebeats;  // duration of measure
	vector<HumNum> timesigbeats;  // duration according to timesignature
	vector<int>    control;       // control = numbered measure
	vector<int>    measurenums;   // output measure numbers

	measureline.reserve(infile.getLineCount());
	measurebeats.reserve(infile.getLineCount());
	timesigbeats.reserve(infile.getLineCount());

	HumNum timesigdur = 0.0;
	int timetop = 4;
	HumNum timebot = 1;
	HumNum value   = 1;
	HumNum lastvalue = 1;
	for (int i=0; i<infile.getLineCount(); i++) {
		if (m_debugQ) {
			cout << "LINE " << i+1 << "\t" << infile[i] << endl;
		}
		if (infile[i].isInterpretation()) {
			for (int j=0; j<infile[i].getFieldCount(); j++) {
				if ((infile.token(i, j)->compare(0, 2, "*M") == 0)
						&& (infile.token(i, j)->find('/') != string::npos)) {
					timetop = Convert::kernTimeSignatureTop(*infile.token(i, j));
					timebot = Convert::kernTimeSignatureBottomToDuration(*infile.token(i, j));
					timesigdur = timebot * timetop;
					// fix last timesigbeats value
					if (timesigbeats.size() > 0) {
						timesigbeats[(int)timesigbeats.size()-1] = timesigdur;
						measurebeats[(int)measurebeats.size()-1] = lastvalue * timebot;
					}
				}
			}
		} else if (infile[i].isBarline()) {
			measureline.push_back(i);
			lastvalue = infile[i].getBeat();
			// shouldn't use timebot (now analyzing rhythm by "4")
			// value = lastvalue * timebot;
			value = lastvalue;
			measurebeats.push_back(value);
			timesigbeats.push_back(timesigdur);
		}
	}

	if (m_debugQ) {
		cout << "measure beats / timesig beats" << endl;
		for (int i=0; i<(int)measurebeats.size(); i++) {
			cout << measurebeats[i] << "\t" << timesigbeats[i] << endl;
		}
	}

	if (measurebeats.size() == 0) {
		// no barlines, nothing to do...
		cout << infile;
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
	std::fill(control.begin(), control.end(), -1);
	std::fill(measurenums.begin(), measurenums.end(), -1);

	// If the time signature and the number of beats in a measure
	// agree, then the bar is worth numbering:
	for (int i=0; i<(int)control.size(); i++) {
		if (measurebeats[i] == timesigbeats[i]) {
			control[i] = 1;
		}
	}

	// Determine first bar (which is marked with a negative value
	// if there is a pickup bar)
	if (measurebeats[0] < 0) {
		if (-measurebeats[0] == timesigbeats[0]) {
			control[0] = 1;
		}
	}

	// Check for intermediate barlines which split one measure
	for (int i=0; i<(int)control.size()-2; i++) {
		if ((control[i] == 1) || (control[i+1] == 1)) {
			continue;
		}
		if (timesigbeats[i] != timesigbeats[i+1]) {
			continue;
		}
		if ((measurebeats[i]+measurebeats[i+1]) == timesigbeats[i]) {
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
		if (measurebeats[i] == 0) {
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
			if ((measurebeats[0] > 0) && dataq) {
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
		cout << "cont\tnum\tbeats" << endl;
		for (int i=0; i<(int)control.size(); i++) {
			cout << control[i] << "\t" << measurenums[i] << "\t"
				  << measurebeats[i] << "\t" << timesigbeats[i] << endl;
		}
	}

	int endingbarline = getEndingBarline(infile);

	// ready to print the new barline numbers
	int mindex = 0;
	for (int i=0; i<infile.getLineCount(); i++) {
		if (infile[i].isBarline()) {
			cout << infile[i] << "\n";
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
	for (int i=0; i<humline.getFieldCount(); i++) {
		string& token = *humline.token(i);
		printSingleBarNumber(token,  measurenum);
		if (i < humline.getFieldCount() -1) {
			 cout << "\t";
		 }
	}
	cout << "\n";
}



//////////////////////////////
//
// Tool_barnum::printSingleBarNumber --
//

void Tool_barnum::printSingleBarNumber(const string& astring, int measurenum) {
	for (int i=0; i<(int)astring.size(); i++) {
		if ((astring[i] == '=') && (astring[i+1] != '=')) {
			cout << astring[i] << measurenum;
		} else if (!isdigit(astring[i])) {
			cout << astring[i];
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



