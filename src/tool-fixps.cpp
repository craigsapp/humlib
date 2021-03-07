//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Wed Feb  3 18:54:38 PST 2021
// Last Modified: Wed Feb  3 18:54:42 PST 2021
// Filename:      tool-fixps.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/tool-fixps.cpp
// Syntax:        C++11; humlib
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Refinements and corrections for TiM scores imported
//                from Finale/Sibelius/MuseScore.
//

#include "tool-fixps.h"

#include "Convert.h"

using namespace std;

namespace hum {

// START_MERGE


/////////////////////////////////
//
// Tool_fixps::Tool_fixps -- Set the recognized options for the tool.
//

Tool_fixps::Tool_fixps(void) {
	// define ("n|only-remove-empty-transpositions=b", "Only remove empty transpositions");
}



/////////////////////////////////
//
// Tool_fixps::run -- Primary interfaces to the tool.
//

bool Tool_fixps::run(HumdrumFileSet& infiles) {
	bool status = true;
	for (int i=0; i<infiles.getCount(); i++) {
		status &= run(infiles[i]);
	}
	return status;
}


bool Tool_fixps::run(const string& indata, ostream& out) {
	HumdrumFile infile(indata);
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_fixps::run(HumdrumFile& infile, ostream& out) {
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

bool Tool_fixps::run(HumdrumFile& infile) {
	processFile(infile);
	return true;
}



//////////////////////////////
//
// Tool_fixps::processFile --
//

void Tool_fixps::processFile(HumdrumFile& infile) {
	removeDuplicateDynamics(infile);
	markEmptyVoices(infile);
	vector<vector<HTp>> newlist;
	removeEmpties(newlist, infile);
	outputNewSpining(newlist, infile);
}



//////////////////////////////
//
// Tool_fixps::outputNewSpining --
//

void Tool_fixps::outputNewSpining(vector<vector<HTp>>& newlist, HumdrumFile& infile) {
	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].hasSpines()) {
			m_humdrum_text << infile[i] << endl;
			continue;
		}
		if ((i > 0) && (!newlist[i].empty()) && newlist[i][0]->isCommentLocal()) {
			if (!newlist[i-1].empty() && newlist[i-1][0]->isCommentLocal()) {
				if (newlist[i].size() == newlist[i-1].size()) {
					bool same = true;
					for (int j=0; j<(int)newlist[i].size(); j++) {
						if (*(newlist[i][j]) != *(newlist[i-1][j])) {
cerr << "GOT HERE " << i << " " << j << endl;
cerr << infile[i-1] << endl;
cerr << infile[i] << endl;
cerr << endl;
							same = false;
							break;
						}
					}
					if (same) {
						continue;
					}
				}
			}
		}
		if (!infile[i].isManipulator()) {
			m_humdrum_text << newlist[i].at(0);
			for (int j=1; j<(int)newlist[i].size(); j++) {
				m_humdrum_text << "\t";
				m_humdrum_text << newlist[i].at(j);
			}
			m_humdrum_text << endl;
			continue;
		}
		if ((i > 0) && !infile[i-1].isManipulator()) {
			printNewManipulator(infile, newlist, i);
		}
	}
}


//////////////////////////////
//
// Tool_fixps::printNewManipulator --
//

void Tool_fixps::printNewManipulator(HumdrumFile& infile, vector<vector<HTp>>& newlist, int line) {
	HTp token = infile.token(line, 0);
	if (*token == "*-") {
		m_humdrum_text << infile[line] << endl;
		return;
	}
	if (token->compare(0, 2, "**") == 0) {
		m_humdrum_text << infile[line] << endl;
		return;
	}
	m_humdrum_text << "++++++++++++++++++++" << endl;
}

//////////////////////////////
//
// Tool_fixps::removeDuplicateDynamics --
//

void Tool_fixps::removeDuplicateDynamics(HumdrumFile& infile) {
	int scount = infile.getStrandCount();
	for (int i=0; i<scount; i++) {
		HTp sstart = infile.getStrandBegin(i);
		if (!sstart->isDataType("**dynam")) {
			continue;
		}
		HTp send   = infile.getStrandEnd(i);
		HTp current = sstart;
		while (current && (current != send)) {
			vector<string> subtoks = current->getSubtokens();
			if (subtoks.size() % 2 == 1) {
				current = current->getNextToken();
				continue;
			}
			bool equal = true;
			int half = (int)subtoks.size() / 2;
			for (int j=0; j<half; j++) {
				if (subtoks[i] != subtoks[i+half]) {
					equal = false;
				}
			}
			if (equal) {
				string newtext = subtoks[0];
				for (int j=1; j<half; j++) {
					newtext += " ";
					newtext += subtoks[j];
				}
				current->setText(newtext);
			}
		}
	}
}



//////////////////////////////
//
// Tool_fixps::removeEmpties --
//

void Tool_fixps::removeEmpties(vector<vector<HTp>>& newlist, HumdrumFile& infile) {
	newlist.resize(infile.getLineCount());
	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].hasSpines()) {
			continue;
		}
		if (infile[i].isManipulator()) {
			continue;
		}
		for (int j=0; j<infile[i].getFieldCount(); j++) {
			HTp token = infile.token(i, j);
			string value = token->getValue("delete");
			if (value == "true") {
				continue;
			}
			newlist[i].push_back(token);
		}
	}
}



//////////////////////////////
//
// Tool_fixps::markEmptyVoices --
//

void Tool_fixps::markEmptyVoices(HumdrumFile& infile) {
	HLp barline = NULL;
	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].hasSpines()) {
			continue;
		}
		if (infile[i].isManipulator()) {
			continue;
		}
		if (infile[i].isInterpretation()) {
			if (infile.token(i, 0)->compare(0, 2, "**")) {
				barline = &infile[i];
			}
			continue;
		}
		if (infile[i].isBarline()) {
			barline = &infile[i];
		}
		if (!infile[i].isData()) {
			continue;
		}
		if (!barline) {
			continue;
		}
		// check on the data line if:
		// * it is in the first subspine
		// * it is an invisible rest
		// * it takes the full duration of the measure
		// If so, then mark the tokens for deletion in that layer.
		for (int j=0; j<infile[i].getFieldCount(); j++) {
			HTp token = infile.token(i, j);
			// int track = token->getTrack();
			int subtrack = token->getSubtrack();
			if (subtrack != 1) {
				continue;
			}
			if (token->find("yy") == string::npos) {
				continue;
			}
			if (!token->isRest()) {
				continue;
			}
			HumNum duration = token->getDuration();
			HumNum bardur = token->getDurationToBarline();
			HTp current = token;
			while (current) {
			   subtrack = current->getSubtrack();
				if (subtrack != 1) {
					break;
				}
				current->setValue("delete", "true");
				if (current->isBarline()) {
					break;
				}
				current = current->getNextToken();
			}
			current = token;
			current = current->getPreviousToken();
			while (current) {
				if (current->isManipulator()) {
					break;
				}
				if (current->isBarline()) {
					break;
				}
				subtrack = current->getSubtrack();
				if (subtrack != 1) {
					break;
				}
				current->setValue("delete", "true");
				current = current->getPreviousToken();
			}
		}
	}

}



// END_MERGE

} // end namespace hum



