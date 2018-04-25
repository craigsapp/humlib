//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sun Mar  4 21:09:10 PST 2018
// Last Modified: Sun Mar  4 21:09:13 PST 2018
// Filename:      tool-binroll.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/tool-binroll.cpp
// Syntax:        C++11
// vim:           ts=3 noexpandtab
//
// Description:   Extract a binary pinao roll of note in a score.
//

#include "tool-binroll.h"
#include "Convert.h"
#include "HumRegex.h"


using namespace std;

namespace hum {

// START_MERGE


/////////////////////////////////
//
// Tool_binroll::Tool_binroll -- Set the recognized options for the tool.
//

Tool_binroll::Tool_binroll(void) {
	// add options here
	define("t|timebase=s:16", "timebase to do analysis at");
}



/////////////////////////////////
//
// Tool_binroll::run -- Do the main work of the tool.
//

bool Tool_binroll::run(const string& indata, ostream& out) {
	HumdrumFile infile(indata);
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_binroll::run(HumdrumFile& infile, ostream& out) {
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_binroll::run(HumdrumFile& infile) {
	m_duration.setValue(1, 4); // 16th note
	processFile(infile);
	return true;
}



//////////////////////////////
//
// Tool_binroll::processFile --
//

void Tool_binroll::processFile(HumdrumFile& infile) {
	vector<vector<char>> output;
	output.resize(128);
	int count = (infile.getScoreDuration() / m_duration).getInteger() + 1;
	for (int i=0; i<(int)output.size(); i++) {
		output[i].resize(count);
		std::fill(output[i].begin(), output[i].end(), 0);
	}

	int strandcount = infile.getStrandCount();
	for (int i=0; i<strandcount; i++) {
		HTp starting = infile.getStrandStart(i);
		if (!starting->isKern()) {
			continue;
		}
		HTp ending = infile.getStrandEnd(i);
		processStrand(output, starting, ending);
	}

	printAnalysis(infile, output);

}



//////////////////////////////
//
// Tool_binroll::printAnalysis --
//

void Tool_binroll::printAnalysis(HumdrumFile& infile,
		vector<vector<char>>& roll) {
	HumRegex hre;

	for (int i=0; i<infile.getLineCount(); i++) {
		if (infile[i].isExclusive()) {
			break;
		}
		if (infile[i].isEmpty()) {
			continue;
		}
		string line = infile[i].getText();
		int found = 0;
		for (int j=0; j<(int)line.size(); j++) {
			if ((line[j] == '!') && !found) {
				m_free_text << "#";
			} else {
				found = 1;
				m_free_text << line[j];
			}
		}
		m_free_text << "\n";
	}

	for (int i=0; i<(int)roll[0].size(); i++) {
		for (int j=0; j<(int)roll.size(); j++) {
			m_free_text << (int)roll[j][i];
			if (j < (int)roll.size() - 1) {
				m_free_text << ' ';
			}
		}
		m_free_text << "\n";
	}

	int startindex = infile.getLineCount() - 1;
	for (int i=infile.getLineCount()-1; i>=0; i--) {
		if (infile[i].isManipulator()) {
			startindex = i+1;
			break;
		}
		startindex = i;
	}

	for (int i=startindex; i<infile.getLineCount(); i++) {
		if (infile[i].isEmpty()) {
			continue;
		}
		string line = infile[i].getText();
		int found = 0;
		for (int j=0; j<(int)line.size(); j++) {
			if ((line[j] == '!') && !found) {
				m_free_text << "#";
			} else {
				found = 1;
				m_free_text << line[j];
			}
		}
		m_free_text << "\n";
	}
}



//////////////////////////////
//
// Tool_binroll::processStrand --
//

void Tool_binroll::processStrand(vector<vector<char>>& roll, HTp starting,
		HTp ending) {
	HTp current = starting;
	int base12;
	HumNum starttime;
	HumNum duration;
	int startindex;
	int endindex;
	while (current && (current != ending)) {
		if (!current->isNonNullData()) {
			current = current->getNextToken();
			continue;
		}
		if (current->isRest()) {
			current = current->getNextToken();
			continue;
		}

		if (current->isChord()) {
			int stcount = current->getSubtokenCount();
			starttime = current->getDurationFromStart();
			startindex = (starttime / m_duration).getInteger();
			for (int s=0; s<stcount; s++) {
				string tok = current->getSubtoken(s);
				base12 = Convert::kernToMidiNoteNumber(tok);
				if ((base12 < 0) || (base12 > 127)) {
					continue;
				}
				duration = Convert::recipToDuration(tok);
				endindex = ((starttime+duration) / m_duration).getInteger();
				roll[base12][startindex] = 2;
				for (int i=startindex+1; i<endindex; i++) {
					roll[base12][i] = 1;
				}
			}
		} else {
			base12 = Convert::kernToMidiNoteNumber(current);
			if ((base12 < 0) || (base12 > 127)) {
				current = current->getNextToken();
				continue;
			}
			starttime = current->getDurationFromStart();
			duration = current->getDuration();
			startindex = (starttime / m_duration).getInteger();
			endindex   = ((starttime+duration) / m_duration).getInteger();
			roll[base12][startindex] = 2;
			for (int i=startindex+1; i<endindex; i++) {
				roll[base12][i] = 1;
			}
		}
		current = current->getNextToken();
	}
}




// END_MERGE

} // end namespace hum



