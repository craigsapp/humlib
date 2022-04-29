//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Jul 18 11:23:42 PDT 2005
// Last Modified: Thu Apr 28 18:28:47 PDT 2022 Ported to humlib.
// Filename:      tool-thru.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/tool-thru.cpp
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   C++ implementation of the Humdrum Toolkit thru command.
//

#include "tool-thru.h"
#include "HumRegex.h"

#include <stdlib.h>
#include <string.h>
#include <iostream>

using namespace std;

namespace hum {

// START_MERGE


/////////////////////////////////
//
// Tool_thru::Tool_thru -- Set the recognized options for the tool.
//

Tool_thru::Tool_thru(void) {
	define("v|variation=s:",   "Choose the expansion variation");
	define("l|list=b:",        "Print list of labels in file");
	define("k|keep=b:",        "Keep variation interpretations");
	define("i|info=b:",        "Print info list of labels in file");
	define("r|realization=s:", "Alternate relaization label sequence");
}


/////////////////////////////////
//
// Tool_thru::run -- Do the main work of the tool.
//

bool Tool_thru::run(HumdrumFileSet& infiles) {
	bool status = true;
	for (int i=0; i<infiles.getCount(); i++) {
		status &= run(infiles[i]);
	}
	return status;
}


bool Tool_thru::run(const string& indata, ostream& out) {
	HumdrumFile infile(indata);
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_thru::run(HumdrumFile& infile, ostream& out) {
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_thru::run(HumdrumFile& infile) {
	initialize();
	processFile(infile);
	return true;
}



//////////////////////////////
//
// Tool_thru::initialize --  Initializations that only have to be done once
//    for all HumdrumFile segments.
//

void Tool_thru::initialize(void) {
	m_variation   = getString("variation");
	m_realization = getString("realization");
	m_listQ       = getBoolean("list");
	m_infoQ       = getBoolean("info");
	m_keepQ       = getBoolean("keep");
}



//////////////////////////////
//
// Tool_thru::processFile --
//

void Tool_thru::processFile(HumdrumFile& infile) {
	if (m_listQ) {
		printLabelList(infile);
		return;
	} else if (m_infoQ) {
		printLabelInfo(infile);
		return;
	}
	processData(infile);

	// analyze the input file according to command-line options
	// infiles[i].printNonemptySegmentLabel(m_humdrum_text);
}



//////////////////////////////
//
// Tool_thru::printLabelList -- print a list of the thru labels.
//

void Tool_thru::printLabelList(HumdrumFile& infile) {
	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isInterpretation()) {
			continue;
		}
		HTp token = infile.token(i, 0);
		if (token->compare(0, 2, "*>") != 0) {
			continue;   // ignore non-labels
		}
		//if (token->find('[') != NULL) {
		//   continue;   // ignore realizations
		//}
		m_humdrum_text << token->substr(2);
		m_humdrum_text << '\n';
	}
}



//////////////////////////////
//
// Tool_thru::printLabelInfo -- print a list of the thru labels.
//

void Tool_thru::printLabelInfo(HumdrumFile& infile) {
	// infile.analyzeRhythm();
	vector<int> labellines;
	labellines.reserve(1000);

	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isInterpretation()) {
			continue;
		}
		HTp token = infile.token(i, 0);

		if (token->compare(0, 2, "*>") != 0) {
			continue;   // ignore non-labels
		}
		if (token->find('[') != string::npos) {
			m_humdrum_text << "!!>";
			m_humdrum_text << token->substr(2) << endl;
			m_humdrum_text << '\n';
			continue;   // ignore realizations
		}
		labellines.push_back(i);
	}

	vector<int> barlines(1000, -1);
	for (int i=0; i<(int)labellines.size(); i++) {
		barlines[i] = getBarline(infile, labellines[i]);
	}

	if (barlines.size() > 0) {
		barlines[0] = adjustFirstBarline(infile);
	}

	int startline;
	int endline;
	HumNum startbeat;
	HumNum endbeat;
	HumNum duration;

	m_humdrum_text << "**label\t**sline\t**eline\t**sbeat\t**ebeat\t**dur\t**bar\n";
	for (int i=0; i<(int)labellines.size(); i++) {
		startline = labellines[i];
		if (i<(int)labellines.size()-1) {
			endline = labellines[i+1]-1;
		} else {
			endline = infile.getLineCount() - 1;
		}
		startbeat = infile[startline].getDurationFromStart();
		endbeat = infile[endline].getDurationFromStart();
		duration = endbeat - startbeat;
		duration = int(duration.getFloat() * 10000.0 + 0.5) / 10000.0;
		HTp token = infile.token(startline, 0);
		m_humdrum_text << token->substr(2);
		m_humdrum_text << '\t';
		m_humdrum_text << startline + 1;
		m_humdrum_text << '\t';
		m_humdrum_text << endline + 1;
		m_humdrum_text << '\t';
		m_humdrum_text << startbeat;
		m_humdrum_text << '\t';
		m_humdrum_text << endbeat;
		m_humdrum_text << '\t';
		m_humdrum_text << duration;
		m_humdrum_text << '\t';
		m_humdrum_text << barlines[i];
		m_humdrum_text << '\n';

	}
	m_humdrum_text << "*-\t*-\t*-\t*-\t*-\t*-\t*-\n";

}



//////////////////////////////
//
// Tool_thru::adjustFirstBarline --
//

int Tool_thru::adjustFirstBarline(HumdrumFile& infile) {
	int number = 0;
	HumRegex hre;
	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isBarline()) {
			continue;
		}
		if (infile[i].getDurationFromStart() > 0) {
			break;
		}
		HTp token = infile.token(i, 0);
		if (hre.search(token, "=.*(\\d+)")) {
			number = hre.getMatchInt(1);
		}
		break;
	}
	return number;
}



//////////////////////////////
//
// Tool_thru::getBarline --
//

int Tool_thru::getBarline(HumdrumFile& infile, int line) {

	if (infile[line].getDurationFromStart() == 0) {
		return 0;
	}

	int missingcount = 0;
	int number = -1;
	HumRegex hre;
	for (int i=line; i>0; i--) {
		if (!infile[i].isBarline()) {
			continue;
		}
		HTp token = infile.token(i, 0);

		if (hre.search(token, "=.*(\\d+)")) {
			number = hre.getMatchInt(1);
			break;
		} else {
			missingcount++;
		}
		if (missingcount > 1) {
			break;
		}
	}

	return number;
}



//////////////////////////////
//
// Tool_thru::processData --
//

void Tool_thru::processData(HumdrumFile& infile) {
	vector<string> labelsequence;
	labelsequence.reserve(1000);

	vector<string> labels;
	labels.reserve(1000);

	vector<int> startline;
	startline.reserve(1000);

	vector<int> stopline;
	stopline.reserve(1000);

	int header = -1;
	int footer = -1;
	string labelsearch;
	labelsearch = "*>";
	labelsearch += m_variation;
	labelsearch += "[";

	// check for label to expand
	int foundlabel = 0;
	string tempseq;
	if (m_realization.size()  == 0) {
		for (int i=0; i<infile.getLineCount(); i++) {
			if (!infile[i].isInterpretation()) {
				continue;
			}
			HTp token = infile.token(i, 0);
			if (token->compare(0, labelsearch.size(), labelsearch) != 0) {
				continue;
			}

			tempseq = token->substr(labelsearch.size());
			getLabelSequence(labelsequence, tempseq);
			foundlabel = 1;
			break;
		}
	} else {
		foundlabel = 1;
		getLabelSequence(labelsequence, m_realization);
	}

	if (foundlabel == 0) {
		// did not find the label to expand, so echo the data back
		for (int i=0; i<infile.getLineCount(); i++) {
			HTp token = infile.token(i, 0);
			if (*token == "*thru") {
				continue;
			}
			m_humdrum_text << infile[i] << "\n";
			if (token->compare(0, 2, "**") == 0) {
				for (int j=0; j<infile[i].getFieldCount(); j++) {
					m_humdrum_text << "*thru";
					if (j < infile[i].getFieldCount() - 1) {
						m_humdrum_text << "\t";
					}
				}
				m_humdrum_text << "\n";
			}
		}
		return;
	}

	// for (i=0; i<(int)labelsequence.size(); i++) {
	//    m_humdrum_text << i+1 << "\t=\t" << labelsequence[i] << endl;
	// }

	// search for the labeled sections in the music
	string label;
	int location;
	int index;
	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isInterpretation()) {
			continue;
		}
		HTp token = infile.token(i, 0);
		if (*token == "*-") {
			location = i-1;
			footer = i;
			stopline.push_back(location);
		}
		if (token->compare(0, 2, "*>") != 0) {
			continue;
		}
		if (token->find('[') != string::npos) {
			continue;
		}
		if (token->find(']') != string::npos) {
			continue;
		}

		if (labels.size() == 0) {
			header = i-1;
		}

		label = token->substr(2);
		index = (int)labels.size();
		location = i-1;
		if (startline.size() > 0) {
			stopline.push_back(location);
		}
		labels.resize(index+1);
		labels[index] = label;
		startline.push_back(i);
	}

	// m_humdrum_text << "FOOTER = " << footer << endl;
	// m_humdrum_text << "HEADER = " << header << endl;
	// for (i=0; i<(int)labels.size(); i++) {
	//    m_humdrum_text << "\t" << i << "\t=\t" << labels[i]
	//         << "\t" << startline[i] << "\t" << stopline[i]
	//         << endl;
	// }

	// now ready to copy the labeled segements into a final file.


	// print header:
	for (int i=0; i<=header; i++) {
		HTp token = infile.token(i, 0);
		if (*token == "*thru") {
			continue;
		}

		if (!m_keepQ) {
			if (infile[i].isInterpretation()) {
				if (token->compare(0, 2, "*>") == 0) {
					if (token->find('[') != string::npos) {
						continue;
					}
				}
			}
		}

		m_humdrum_text << infile[i] << "\n";
		if (token->compare(0, 2, "**") == 0) {
			for (int j=0; j<infile[i].getFieldCount(); j++) {
				m_humdrum_text << "*thru";
				if (j < infile[i].getFieldCount() - 1) {
					m_humdrum_text << "\t";
				}
			}
			m_humdrum_text << "\n";
		}
	}

	int start;
	int stop;
	for (int i=0; i<(int)labelsequence.size(); i++) {
		index = getLabelIndex(labels, labelsequence[i]);
		if (index < 0) {
			m_humdrum_text << "!! THRU ERROR: label " << labelsequence[i]
				  << " does not exist, skipping.\n";
		}
		start = startline[index];
		stop  = stopline[index];
		for (int j=start; j<=stop; j++) {
			if (!m_keepQ) {
				if (infile[j].isInterpretation()) {
					HTp token = infile.token(j, 0);
					if (token->compare(0, 2, "*>") == 0) {
						if (token->find('[') != string::npos) {
							continue;
						}
					}
				}
			}
			m_humdrum_text << infile[j] << "\n";
		}
	}

	// print footer:
	for (int i=footer; i<infile.getLineCount(); i++) {
		if (!m_keepQ) {
			if (infile[i].isInterpretation()) {
				HTp token = infile.token(i, 0);
				if (token->compare(0, 2, "*>") == 0) {
					if (token->find('[') != string::npos) {
						continue;
					}
				}
			}
		}
		m_humdrum_text << infile[i] << "\n";
	}

}



//////////////////////////////
//
// Tool_thru::getLabelIndex --
//

int Tool_thru::getLabelIndex(vector<string>& labels, string& key) {
	for (int i=0; i<(int)labels.size(); i++) {
		if (key == labels[i]) {
			return i;
		}
	}
	return -1;
}



//////////////////////////////
//
// Tool_thru::getLabelSequence --
//

void Tool_thru::getLabelSequence(vector<string>& labelsequence,
		const string& astring) {
	int slength = (int)astring.size();
	char* sdata = new char[slength+1];
	strcpy(sdata, astring.c_str());
	const char* ignorecharacters = ", [] ";
	int index;

	char* strptr = strtok(sdata, ignorecharacters);
	while (strptr != NULL) {
		labelsequence.resize((int)labelsequence.size() + 1);
		index = (int)labelsequence.size() - 1;
		labelsequence[index] = strptr;
		strptr = strtok(NULL, ignorecharacters);
	}

	delete [] sdata;
}


// END_MERGE

} // end namespace hum



