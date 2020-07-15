//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Aug 17 02:39:28 PDT 2015
// Last Modified: Mon Oct  5 23:15:44 PDT 2015
// Filename:      HumdrumFileContent-slur.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/HumdrumFileContent-slur.cpp
// Syntax:        C++11; humlib
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Links slur starting/ending points to each other.
//

#include "HumdrumFileContent.h"

#include <algorithm>

using namespace std;

namespace hum {

// START_MERGE



//////////////////////////////
//
// HumdrumFileContent::analyzeSlurs -- Link start and ends of
//    slurs to each other.
//

bool HumdrumFileContent::analyzeSlurs(void) {
	if (m_analyses.m_slurs_analyzed) {
		return false;
	}
	m_analyses.m_slurs_analyzed = true;
	bool output = true;
	output &= analyzeKernSlurs();
	output &= analyzeMensSlurs();
	return output;
}



//////////////////////////////
//
// HumdrumFileContent::analyzeMensSlurs -- Link start and ends of
//    slurs to each other.  They are the same as **kern, so borrowing
//    analyzeKernSlurs to do the analysis.
//

bool HumdrumFileContent::analyzeMensSlurs(void) {
	vector<HTp> slurstarts;
	vector<HTp> slurends;

	vector<HTp> l;
	vector<pair<HTp, HTp>> labels; // first is previous label, second is next label
	HumdrumFileBase& infile = *this;
	labels.resize(infile.getLineCount());
	l.resize(infile.getLineCount());
	for (int i=0; i<infile.getLineCount(); i++) {
		labels[i].first = NULL;
		labels[i].second = NULL;
		l[i] = NULL;
	}
	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isInterpretation()) {
			continue;
		}
		HTp token = infile.token(i, 0);
		if ((token->compare(0, 2, "*>") == 0) && (token->find("[") == std::string::npos)) {
			l[i] = token;
		}
	}
	HTp current = NULL;
	for (int i=0; i<infile.getLineCount(); i++) {
		if (l[i] != NULL) {
			current = l[i];
		}
		labels[i].first = current;
	}
	current = NULL;
	for (int i=infile.getLineCount() - 1; i>=0; i--) {
		if (l[i] != NULL) {
			current = l[i];
		}
		labels[i].second = current;
	}

	vector<int> endings(infile.getLineCount(), 0);
	int ending = 0;
	for (int i=0; i<(int)endings.size(); i++) {
		if (l[i]) {
			char lastchar = l[i]->back();
			if (isdigit(lastchar)) {
				ending = lastchar - '0';
			} else {
				ending = 0;
			}
		}
		endings[i] = ending;
	}

	vector<HTp> mensspines;
	getSpineStartList(mensspines, "**mens");
	bool output = true;
	string linkSignifier = m_signifiers.getKernLinkSignifier();
	for (int i=0; i<(int)mensspines.size(); i++) {
		output = output && analyzeKernSlurs(mensspines[i], slurstarts, slurends, labels, endings, linkSignifier);
	}
	createLinkedSlurs(slurstarts, slurends);
	return output;
}



//////////////////////////////
//
// HumdrumFileContent::analyzeKernSlurs -- Link start and ends of
//    slurs to each other.
//

bool HumdrumFileContent::analyzeKernSlurs(void) {
	vector<HTp> slurstarts;
	vector<HTp> slurends;

	vector<HTp> l;
	vector<pair<HTp, HTp>> labels; // first is previous label, second is next label
	HumdrumFileBase& infile = *this;
	labels.resize(infile.getLineCount());
	l.resize(infile.getLineCount());
	for (int i=0; i<infile.getLineCount(); i++) {
		labels[i].first = NULL;
		labels[i].second = NULL;
		l[i] = NULL;
	}
	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isInterpretation()) {
			continue;
		}
		HTp token = infile.token(i, 0);
		if ((token->compare(0, 2, "*>") == 0) && (token->find("[") == std::string::npos)) {
			l[i] = token;
		}
	}
	HTp current = NULL;
	for (int i=0; i<infile.getLineCount(); i++) {
		if (l[i] != NULL) {
			current = l[i];
		}
		labels[i].first = current;
	}
	current = NULL;
	for (int i=infile.getLineCount() - 1; i>=0; i--) {
		if (l[i] != NULL) {
			current = l[i];
		}
		labels[i].second = current;
	}

	vector<int> endings(infile.getLineCount(), 0);
	int ending = 0;
	for (int i=0; i<(int)endings.size(); i++) {
		if (l[i]) {
			char lastchar = l[i]->back();
			if (isdigit(lastchar)) {
				ending = lastchar - '0';
			} else {
				ending = 0;
			}
		}
		endings[i] = ending;
	}

	vector<HTp> kernspines;
	getSpineStartList(kernspines, "**kern");
	bool output = true;
	string linkSignifier = m_signifiers.getKernLinkSignifier();
	for (int i=0; i<(int)kernspines.size(); i++) {
		output = output && analyzeKernSlurs(kernspines[i], slurstarts, slurends, labels, endings, linkSignifier);
	}

	createLinkedSlurs(slurstarts, slurends);
	return output;
}


bool HumdrumFileContent::analyzeKernSlurs(HTp spinestart,
		vector<HTp>& linkstarts, vector<HTp>& linkends, vector<pair<HTp, HTp>>& labels,
		vector<int>& endings, const string& linksig) {

	// linked slurs handled separately, so generate an ignore sequence:
	string ignorebegin = linksig + "(";
	string ignoreend = linksig + ")";

	// tracktokens == the 2-D data list for the track,
	// arranged in layers with the second dimension.
	vector<vector<HTp> > tracktokens;
	this->getTrackSeq(tracktokens, spinestart, OPT_DATA | OPT_NOEMPTY);
	// printSequence(tracktokens);

	// sluropens == list of slur openings for each track and elision level
	// first dimension: elision level
	// second dimension: track number
	vector<vector<vector<HTp>>> sluropens;

	sluropens.resize(4); // maximum of 4 elision levels
	for (int i=0; i<(int)sluropens.size(); i++) {
		sluropens[i].resize(8);  // maximum of 8 layers
	}

	int opencount = 0;
	int closecount = 0;
	int elision = 0;
	HTp token;
	for (int row=0; row<(int)tracktokens.size(); row++) {
		for (int track=0; track<(int)tracktokens[row].size(); track++) {
			token = tracktokens[row][track];
			if (!token->isData()) {
				continue;
			}
			if (token->isNull()) {
				continue;
			}
			opencount = (int)count(token->begin(), token->end(), '(');
			closecount = (int)count(token->begin(), token->end(), ')');

			for (int i=0; i<closecount; i++) {
				bool isLinked = isLinkedSlurEnd(token, i, ignoreend);
				if (isLinked) {
					linkends.push_back(token);
					continue;
				}
				elision = token->getSlurEndElisionLevel(i);
				if (elision < 0) {
					continue;
				}
				if (sluropens[elision][track].size() > 0) {
					linkSlurEndpoints(sluropens[elision][track].back(), token);
					// remove slur opening from buffer
					sluropens[elision][track].pop_back();
				} else {
					// No starting slur marker to match to this slur end in the
					// given track.
					// search for an open slur in another track:
					bool found = false;
					for (int itrack=0; itrack<(int)sluropens[elision].size(); itrack++) {
						if (sluropens[elision][itrack].size() > 0) {
							linkSlurEndpoints(sluropens[elision][itrack].back(), token);
							// remove slur opening from buffer
							sluropens[elision][itrack].pop_back();
							found = true;
							break;
						}
					}
					if (!found) {
						int lineindex = token->getLineIndex();
						int endnum = endings[lineindex];
						int pindex = -1;
						if (labels[lineindex].first) {
							pindex = labels[lineindex].first->getLineIndex();
							pindex--;
						}
						int endnumpre = -1;
						if (pindex >= 0) {
							endnumpre = endings[pindex];
						}

						if ((endnumpre > 0) && (endnum > 0) && (endnumpre != endnum)) {
							// This is a slur in an ending that start at the start of an ending.
							HumNum duration = token->getDurationFromStart();
							if (labels[token->getLineIndex()].first) {
								duration -= labels[token->getLineIndex()].first->getDurationFromStart();
							}
							token->setValue("auto", "endingSlurBack", "true");
							token->setValue("auto", "slurSide", "stop");
							token->setValue("auto", "slurDration",
								token->getDurationToEnd());
						} else {
							// This is a slur closing that does not have a matching opening.
							token->setValue("auto", "hangingSlur", "true");
							token->setValue("auto", "slurSide", "stop");
							token->setValue("auto", "slurOpenIndex", to_string(i));
							token->setValue("auto", "slurDration",
								token->getDurationToEnd());
						}
					}
				}
			}

			for (int i=0; i<opencount; i++) {
				bool isLinked = isLinkedSlurBegin(token, i, ignorebegin);
				if (isLinked) {
					linkstarts.push_back(token);
					continue;
				}
				elision = token->getSlurStartElisionLevel(i);
				if (elision < 0) {
					continue;
				}
				sluropens[elision][track].push_back(token);
			}
		}
	}

	// Mark un-closed slur starts:
	for (int i=0; i<(int)sluropens.size(); i++) {
		for (int j=0; j<(int)sluropens[i].size(); j++) {
			for (int k=0; k<(int)sluropens[i][j].size(); k++) {
				sluropens[i][j][k]->setValue("", "auto", "hangingSlur", "true");
				sluropens[i][j][k]->setValue("", "auto", "slurSide", "start");
				sluropens[i][j][k]->setValue("", "auto", "slurDuration",
						sluropens[i][j][k]->getDurationFromStart());
			}
		}
	}

	return true;
}



//////////////////////////////
//
// HumdrumFileContent::createLinkedSlurs --  Currently assume that
//    start and ends are matched.
//

void HumdrumFileContent::createLinkedSlurs(vector<HTp>& linkstarts, vector<HTp>& linkends) {
	int max = (int)linkstarts.size();
	if ((int)linkends.size() < max) {
		max = (int)linkends.size();
	}
	if (max == 0) {
		// nothing to do
		return;
	}

	for (int i=0; i<max; i++) {
		linkSlurEndpoints(linkstarts[i], linkends[i]);
	}
}



//////////////////////////////
//
// HumdrumFileContent::isLinkedSlurEnd --
//

bool HumdrumFileContent::isLinkedSlurEnd(HTp token, int index, const string& pattern) {
	if (pattern.size() <= 1) {
		return false;
	}
	int counter = -1;
	for (int i=0; i<(int)token->size(); i++) {
		if (token->at(i) == ')') {
			counter++;
		}
		if (i == 0) {
			// Can't have linked slur at starting index in string.
			continue;
		}
		if (counter != index) {
			continue;
		}

		int startindex = i - (int)pattern.size() + 1;
		auto loc = token->find(pattern, startindex);
		if ((loc != std::string::npos) && ((int)loc == startindex)) {
			return true;
		}
		return false;
	}
	return false;
}



//////////////////////////////
//
// HumdrumFileContent::isLinkedSlurBegin --
//

bool HumdrumFileContent::isLinkedSlurBegin(HTp token, int index, const string& pattern) {
	if (pattern.size() <= 1) {
		return false;
	}
	int counter = -1;
	for (int i=0; i<(int)token->size(); i++) {
		if (token->at(i) == '(') {
			counter++;
		}
		if (i == 0) {
			continue;
		}
		if (counter != index) {
			continue;
		}
		if (token->find(pattern, i - (int)pattern.size() + 1) != std::string::npos) {
			return true;
		}
		return false;
	}
	return false;
}



//////////////////////////////
//
// HumdrumFileContent::linkSlurEndpoints --  Allow up to two slur starts/ends
//      on a note.
//

void HumdrumFileContent::linkSlurEndpoints(HTp slurstart, HTp slurend) {
	string durtag = "slurDuration";
	string endtag = "slurEndId";
	string starttag = "slurStartId";
	string slurstartnumbertag = "slurStartNumber";
	string slurendnumbertag = "slurEndNumber";

	int slurStartCount = slurstart->getValueInt("auto", "slurStartCount");
	int opencount = (int)count(slurstart->begin(), slurstart->end(), '(');
	slurStartCount++;
	int openEnumeration = opencount - slurStartCount + 1;

	if (openEnumeration > 1) {
		endtag += to_string(openEnumeration);
		durtag += to_string(openEnumeration);
		slurendnumbertag += to_string(openEnumeration);
	}

	int slurEndNumber = slurend->getValueInt("auto", "slurEndCount");
	slurEndNumber++;
	int closeEnumeration = slurEndNumber;
	if (closeEnumeration > 1) {
		starttag += to_string(closeEnumeration);
		slurstartnumbertag += to_string(closeEnumeration);
	}

	HumNum duration = slurend->getDurationFromStart()
			- slurstart->getDurationFromStart();

	slurstart->setValue("auto", endtag,            slurend);
	slurstart->setValue("auto", "id",              slurstart);
	slurstart->setValue("auto", slurendnumbertag,  closeEnumeration);
	slurstart->setValue("auto", durtag,            duration);
	slurstart->setValue("auto", "slurStartCount",  slurStartCount);

	slurend->setValue("auto", starttag, slurstart);
	slurend->setValue("auto", "id", slurend);
	slurend->setValue("auto", slurstartnumbertag, openEnumeration);
	slurend->setValue("auto", "slurEndCount",  slurEndNumber);
}


// END_MERGE

} // end namespace hum



