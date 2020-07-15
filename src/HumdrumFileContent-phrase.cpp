//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Fri Dec  6 19:09:35 PST 2019
// Last Modified: Fri Dec  6 19:09:39 PST 2019
// Filename:      HumdrumFileContent-phrase.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/HumdrumFileContent-phrase.cpp
// Syntax:        C++11; humlib
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Links phrase starting/ending points to each other.
//

#include "HumdrumFileContent.h"

#include <algorithm>

using namespace std;

namespace hum {

// START_MERGE



//////////////////////////////
//
// HumdrumFileContent::analyzePhrasings -- Link start and ends of
//    phrases to each other.
//

bool HumdrumFileContent::analyzePhrasings(void) {
	if (m_analyses.m_phrases_analyzed) {
		return false;
	}
	m_analyses.m_phrases_analyzed = true;
	bool output = true;
	output &= analyzeKernPhrasings();
	return output;
}



//////////////////////////////
//
// HumdrumFileContent::analyzeKernPhrasings -- Link start and ends of
//    phrases to each other.
//

bool HumdrumFileContent::analyzeKernPhrasings(void) {
	vector<HTp> phrasestarts;
	vector<HTp> phraseends;

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
		output = output && analyzeKernPhrasings(kernspines[i], phrasestarts, phraseends, labels, endings, linkSignifier);
	}

	createLinkedPhrasings(phrasestarts, phraseends);
	return output;
}


bool HumdrumFileContent::analyzeKernPhrasings(HTp spinestart,
		vector<HTp>& linkstarts, vector<HTp>& linkends, vector<pair<HTp, HTp>>& labels,
		vector<int>& endings, const string& linksig) {

	// linked phrases handled separately, so generate an ignore sequence:
	string ignorebegin = linksig + "{";
	string ignoreend = linksig + "}";

	// tracktokens == the 2-D data list for the track,
	// arranged in layers with the second dimension.
	vector<vector<HTp> > tracktokens;
	this->getTrackSeq(tracktokens, spinestart, OPT_DATA | OPT_NOEMPTY);
	// printSequence(tracktokens);

	// phraseopens == list of phrase openings for each track and elision level
	// first dimension: elision level
	// second dimension: track number
	vector<vector<vector<HTp>>> phraseopens;

	phraseopens.resize(4); // maximum of 4 elision levels
	for (int i=0; i<(int)phraseopens.size(); i++) {
		phraseopens[i].resize(8);  // maximum of 8 layers
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
			opencount = (int)count(token->begin(), token->end(), '{');
			closecount = (int)count(token->begin(), token->end(), '}');

			for (int i=0; i<closecount; i++) {
				bool isLinked = isLinkedPhraseEnd(token, i, ignoreend);
				if (isLinked) {
					linkends.push_back(token);
					continue;
				}
				elision = token->getPhraseEndElisionLevel(i);
				if (elision < 0) {
					continue;
				}
				if (phraseopens[elision][track].size() > 0) {
					linkPhraseEndpoints(phraseopens[elision][track].back(), token);
					// remove phrase opening from buffer
					phraseopens[elision][track].pop_back();
				} else {
					// No starting phrase marker to match to this phrase end in the
					// given track.
					// search for an open phrase in another track:
					bool found = false;
					for (int itrack=0; itrack<(int)phraseopens[elision].size(); itrack++) {
						if (phraseopens[elision][itrack].size() > 0) {
							linkPhraseEndpoints(phraseopens[elision][itrack].back(), token);
							// remove phrase opening from buffer
							phraseopens[elision][itrack].pop_back();
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
							// This is a phrase in an ending that start at the start of an ending.
							HumNum duration = token->getDurationFromStart();
							if (labels[token->getLineIndex()].first) {
								duration -= labels[token->getLineIndex()].first->getDurationFromStart();
							}
							token->setValue("auto", "endingPhraseBack", "true");
							token->setValue("auto", "phraseSide", "stop");
							token->setValue("auto", "phraseDration",
								token->getDurationToEnd());
						} else {
							// This is a phrase closing that does not have a matching opening.
							token->setValue("auto", "hangingPhrase", "true");
							token->setValue("auto", "phraseSide", "stop");
							token->setValue("auto", "phraseOpenIndex", to_string(i));
							token->setValue("auto", "phraseDration",
								token->getDurationToEnd());
						}
					}
				}
			}

			for (int i=0; i<opencount; i++) {
				bool isLinked = isLinkedPhraseBegin(token, i, ignorebegin);
				if (isLinked) {
					linkstarts.push_back(token);
					continue;
				}
				elision = token->getPhraseStartElisionLevel(i);
				if (elision < 0) {
					continue;
				}
				phraseopens[elision][track].push_back(token);
			}
		}
	}

	// Mark un-closed phrase starts:
	for (int i=0; i<(int)phraseopens.size(); i++) {
		for (int j=0; j<(int)phraseopens[i].size(); j++) {
			for (int k=0; k<(int)phraseopens[i][j].size(); k++) {
				phraseopens[i][j][k]->setValue("", "auto", "hangingPhrase", "true");
				phraseopens[i][j][k]->setValue("", "auto", "phraseSide", "start");
				phraseopens[i][j][k]->setValue("", "auto", "phraseDuration",
						phraseopens[i][j][k]->getDurationFromStart());
			}
		}
	}

	return true;
}



//////////////////////////////
//
// HumdrumFileContent::createLinkedPhrasings --  Currently assume that
//    start and ends are matched.
//

void HumdrumFileContent::createLinkedPhrasings(vector<HTp>& linkstarts, vector<HTp>& linkends) {
	int max = (int)linkstarts.size();
	if ((int)linkends.size() < max) {
		max = (int)linkends.size();
	}
	if (max == 0) {
		// nothing to do
		return;
	}

	for (int i=0; i<max; i++) {
		linkPhraseEndpoints(linkstarts[i], linkends[i]);
	}
}



//////////////////////////////
//
// HumdrumFileContent::isLinkedPhraseEnd --
//

bool HumdrumFileContent::isLinkedPhraseEnd(HTp token, int index, const string& pattern) {
	if (pattern.size() <= 1) {
		return false;
	}
	int counter = -1;
	for (int i=0; i<(int)token->size(); i++) {
		if (token->at(i) == ')') {
			counter++;
		}
		if (i == 0) {
			// Can't have linked phrase at starting index in string.
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
// HumdrumFileContent::isLinkedPhraseBegin --
//

bool HumdrumFileContent::isLinkedPhraseBegin(HTp token, int index, const string& pattern) {
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
// HumdrumFileContent::linkPhraseEndpoints --  Allow up to two phrase starts/ends
//      on a note.
//

void HumdrumFileContent::linkPhraseEndpoints(HTp phrasestart, HTp phraseend) {
	string durtag = "phraseDuration";
	string endtag = "phraseEnd";
	int phraseEndCount = phrasestart->getValueInt("auto", "phraseEndCount");
	phraseEndCount++;
	if (phraseEndCount > 1) {
		endtag += to_string(phraseEndCount);
		durtag += to_string(phraseEndCount);
	}
	string starttag = "phraseStart";
	int phraseStartCount = phraseend->getValueInt("auto", "phraseStartCount");
	phraseStartCount++;
	if (phraseStartCount > 1) {
		starttag += to_string(phraseStartCount);
	}

	phrasestart->setValue("auto", endtag, phraseend);
	phrasestart->setValue("auto", "id", phrasestart);
	phraseend->setValue("auto", starttag, phrasestart);
	phraseend->setValue("auto", "id", phraseend);
	HumNum duration = phraseend->getDurationFromStart()
			- phrasestart->getDurationFromStart();
	phrasestart->setValue("auto", durtag, duration);
	phrasestart->setValue("auto", "phraseEndCount", to_string(phraseEndCount));
	phraseend->setValue("auto", "phraseStartCount", to_string(phraseStartCount));
}


// END_MERGE

} // end namespace hum



