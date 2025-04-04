//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Fri Apr 15 11:18:20 PDT 2022
// Last Modified: Fri Apr 15 11:18:22 PDT 2022
// Filename:      HumdrumFileContent-beam.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/HumdrumFileContent-beam.cpp
// Syntax:        C++11; humlib
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Links beam starting/ending points to each other.
//

#include "HumdrumFileContent.h"

#include <algorithm>

using namespace std;

namespace hum {

// START_MERGE



//////////////////////////////
//
// HumdrumFileContent::analyzeBeams -- Link start and ends of
//    beams to each other.
//

bool HumdrumFileContent::analyzeBeams(void) {
	if (m_analyses.m_beams_analyzed) {
		return false;
	}
	m_analyses.m_beams_analyzed = true;
	bool output = true;
	output &= analyzeKernBeams();
	output &= analyzeMensBeams();
	return output;
}



//////////////////////////////
//
// HumdrumFileContent::analyzeMensBeams -- Link start and ends of
//    beams to each other.  They are the same as **kern, so borrowing
//    analyzeKernBeams to do the analysis.
//

bool HumdrumFileContent::analyzeMensBeams(void) {
	vector<HTp> beamstarts;
	vector<HTp> beamends;

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
		output = output && analyzeKernBeams(mensspines[i], beamstarts, beamends, labels, endings, linkSignifier);
	}
	createLinkedBeams(beamstarts, beamends);
	return output;
}



//////////////////////////////
//
// HumdrumFileContent::analyzeKernBeams -- Link start and ends of
//    beams to each other.
//

bool HumdrumFileContent::analyzeKernBeams(void) {
	vector<HTp> beamstarts;
	vector<HTp> beamends;

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
		output = output && analyzeKernBeams(kernspines[i], beamstarts, beamends, labels, endings, linkSignifier);
	}

	createLinkedBeams(beamstarts, beamends);
	return output;
}


bool HumdrumFileContent::analyzeKernBeams(HTp spinestart,
		vector<HTp>& linkstarts, vector<HTp>& linkends, vector<pair<HTp, HTp>>& labels,
		vector<int>& endings, const string& linksig) {

	// linked beams handled separately, so generate an ignore sequence:
	string ignorebegin = linksig + "L";
	string ignoreend = linksig + "J";

	// tracktokens == the 2-D data list for the track,
	// arranged in layers with the second dimension.
	vector<vector<HTp> > tracktokens;
	this->getTrackSeq(tracktokens, spinestart, OPT_DATA | OPT_NOEMPTY);
	// printSequence(tracktokens);

	// beamopens == list of beam openings for each track and elision level
	// first dimension: elision level
	// second dimension: track number
	vector<vector<vector<HTp>>> beamopens;

	beamopens.resize(4); // maximum of 4 elision levels
	for (int i=0; i<(int)beamopens.size(); i++) {
		beamopens[i].resize(8);  // maximum of 8 layers
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
			opencount = (int)count(token->begin(), token->end(), 'L');
			closecount = (int)count(token->begin(), token->end(), 'J');

			for (int i=0; i<closecount; i++) {
				bool isLinked = isLinkedBeamEnd(token, i, ignoreend);
				if (isLinked) {
					linkends.push_back(token);
					continue;
				}
				elision = token->getBeamEndElisionLevel(i);
				if (elision < 0) {
					continue;
				}
				if (beamopens[elision][track].size() > 0) {
					linkBeamEndpoints(beamopens[elision][track].back(), token);
					// remove beam opening from buffer
					beamopens[elision][track].pop_back();
				} else {
					// No starting beam marker to match to this beam end in the
					// given track.
					// search for an open beam in another track:
					bool found = false;
					for (int itrack=0; itrack<(int)beamopens[elision].size(); itrack++) {
						if (beamopens[elision][itrack].size() > 0) {
							linkBeamEndpoints(beamopens[elision][itrack].back(), token);
							// remove beam opening from buffer
							beamopens[elision][itrack].pop_back();
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
							// This is a beam in an ending that start at the start of an ending.
							HumNum duration = token->getDurationFromStart();
							if (labels[token->getLineIndex()].first) {
								duration -= labels[token->getLineIndex()].first->getDurationFromStart();
							}
							token->setValue("auto", "endingBeamBack", "true");
							token->setValue("auto", "beamSide", "stop");
							token->setValue("auto", "beamDuration",
								token->getDurationToEnd());
						} else {
							// This is a beam closing that does not have a matching opening.
							token->setValue("auto", "hangingBeam", "true");
							token->setValue("auto", "beamSide", "stop");
							token->setValue("auto", "beamOpenIndex", to_string(i));
							token->setValue("auto", "beamDuration",
								token->getDurationToEnd());
						}
					}
				}
			}

			for (int i=0; i<opencount; i++) {
				bool isLinked = isLinkedBeamBegin(token, i, ignorebegin);
				if (isLinked) {
					linkstarts.push_back(token);
					continue;
				}
				elision = token->getBeamStartElisionLevel(i);
				if (elision < 0) {
					continue;
				}
				beamopens[elision][track].push_back(token);
			}
		}
	}

	// Mark un-closed beam starts:
	for (int i=0; i<(int)beamopens.size(); i++) {
		for (int j=0; j<(int)beamopens[i].size(); j++) {
			for (int k=0; k<(int)beamopens[i][j].size(); k++) {
				beamopens[i][j][k]->setValue("", "auto", "hangingBeam", "true");
				beamopens[i][j][k]->setValue("", "auto", "beamSide", "start");
				beamopens[i][j][k]->setValue("", "auto", "beamDuration",
						beamopens[i][j][k]->getDurationFromStart());
			}
		}
	}

	return true;
}



//////////////////////////////
//
// HumdrumFileContent::createLinkedBeams --  Currently assume that
//    start and ends are matched.
//

void HumdrumFileContent::createLinkedBeams(vector<HTp>& linkstarts, vector<HTp>& linkends) {
	int max = (int)linkstarts.size();
	if ((int)linkends.size() < max) {
		max = (int)linkends.size();
	}
	if (max == 0) {
		// nothing to do
		return;
	}

	for (int i=0; i<max; i++) {
		linkBeamEndpoints(linkstarts[i], linkends[i]);
	}
}



//////////////////////////////
//
// HumdrumFileContent::isLinkedBeamEnd --
//

bool HumdrumFileContent::isLinkedBeamEnd(HTp token, int index, const string& pattern) {
	if (pattern.size() <= 1) {
		return false;
	}
	int counter = -1;
	for (int i=0; i<(int)token->size(); i++) {
		if (token->at(i) == 'J') {
			counter++;
		}
		if (i == 0) {
			// Can't have linked beam at starting index in string.
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
// HumdrumFileContent::isLinkedBeamBegin --
//

bool HumdrumFileContent::isLinkedBeamBegin(HTp token, int index, const string& pattern) {
	if (pattern.size() <= 1) {
		return false;
	}
	int counter = -1;
	for (int i=0; i<(int)token->size(); i++) {
		if (token->at(i) == 'L') {
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
// HumdrumFileContent::linkBeamEndpoints --  Allow up to two beam starts/ends
//      on a note.
//

void HumdrumFileContent::linkBeamEndpoints(HTp beamstart, HTp beamend) {
	string durtag = "beamDuration";
	string endtag = "beamEndId";
	string starttag = "beamStartId";
	string beamstartnumbertag = "beamStartNumber";
	string beamendnumbertag = "beamEndNumber";

	int beamStartCount = beamstart->getValueInt("auto", "beamStartCount");
	int opencount = (int)count(beamstart->begin(), beamstart->end(), 'L');
	beamStartCount++;
	int openEnumeration = opencount - beamStartCount + 1;

	if (openEnumeration > 1) {
		endtag += to_string(openEnumeration);
		durtag += to_string(openEnumeration);
		beamendnumbertag += to_string(openEnumeration);
	}

	int beamEndNumber = beamend->getValueInt("auto", "beamEndCount");
	beamEndNumber++;
	int closeEnumeration = beamEndNumber;
	if (closeEnumeration > 1) {
		starttag += to_string(closeEnumeration);
		beamstartnumbertag += to_string(closeEnumeration);
	}

	HumNum duration = beamend->getDurationFromStart()
			- beamstart->getDurationFromStart();

	HumNum durToBar = beamstart->getDurationToBarline();

	if (duration >= durToBar) {
		beamstart->setValue("auto", "beamSpanStart", 1);
		beamend->setValue("auto", "beamSpanEnd", 1);
		markBeamSpanMembers(beamstart, beamend);
	}

	beamstart->setValue("auto", endtag,            beamend);
	beamstart->setValue("auto", "id",              beamstart);
	beamstart->setValue("auto", beamendnumbertag,  closeEnumeration);
	beamstart->setValue("auto", durtag,            duration);
	beamstart->setValue("auto", "beamStartCount",  beamStartCount);

	beamend->setValue("auto", starttag, beamstart);
	beamend->setValue("auto", "id", beamend);
	beamend->setValue("auto", beamstartnumbertag, openEnumeration);
	beamend->setValue("auto", "beamEndCount",  beamEndNumber);
}



//////////////////////////////
//
// HumdrumFileContent::markBeamSpanMembers --
//

void HumdrumFileContent::markBeamSpanMembers(HTp beamstart, HTp beamend) {
	int endindex = beamend->getLineIndex();
	beamstart->setValue("auto", "inBeamSpan", beamstart);
	beamend->setValue("auto", "inBeamSpan", beamstart);
	HTp current = beamstart->getNextToken();
	while (current) {
      int line = current->getLineIndex();
		if (line > endindex) {
			// terminate search for end if getting lost
			break;
		}
		if (current == beamend) {
			break;
		}
		if (!current->isData()) {
			current = current->getNextToken();
			continue;
		}
		if (current->isNull()) {
			current = current->getNextToken();
			continue;
		}
		if (current->getDuration() == 0) {
			// ignore grace notes
			current = current->getNextToken();
			continue;
		}
		current->setValue("auto", "inBeamSpan", beamstart);
		current = current->getNextToken();
	}
}



// END_MERGE

} // end namespace hum



