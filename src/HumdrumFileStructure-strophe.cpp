//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Tue Oct 20 15:26:17 PDT 2020
// Last Modified: Tue Oct 20 15:26:20 PDT 2020
// Filename:      HumdrumFileStructure-strophe.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/HumdrumFileStructure-strophe.cpp
// Syntax:        C++11; humlib
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Functions for accessing strophe analysis data.
//

#include "HumdrumFileStructure.h"

using namespace std;

namespace hum {

// START_MERGE


//////////////////////////////
//
// HumdrumFileStructure::analyzeStropheMarkers -- Merge this
//    with analyzeStrophes (below) at some point.
//

void HumdrumFileStructure::analyzeStropheMarkers(void) {
	m_analyses.m_strophes_analyzed = true;

	m_strophes1d.clear();
	m_strophes2d.clear();
	int spines = getSpineCount();
	m_strophes2d.resize(spines);

	map<string, HTp> laststrophe;

	HumdrumFileStructure& infile = *this;

	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isInterpretation()) {
			continue;
		}
		for (int j=0; j<infile[i].getFieldCount(); j++) {
			HTp token = infile.token(i, j);
			if (*token == "*strophe") {
				string spineinfo = token->getSpineInfo();
	 			HTp lastone = laststrophe[spineinfo];
				if (lastone) {
					// Improperly terminated strophe,
					// ending here and starting a new one.
					TokenPair tp;
					tp.first = lastone;
					tp.last = token;
					m_strophes1d.push_back(tp);
					int spine = token->getTrack() - 1;
					m_strophes2d.at(spine).push_back(tp);
					laststrophe[spineinfo] = token;
				} else {
					// Store the new strophe.
					laststrophe[spineinfo] = token;
				}
			} else if ((*token == "*Xstrophe") || (*token == "*S-")) {
				string spineinfo = token->getSpineInfo();
	 			HTp lastone = laststrophe[spineinfo];
				if (lastone) {
					TokenPair tp;
					tp.first = lastone;
					tp.last = token;
					m_strophes1d.push_back(tp);
					int spine = token->getTrack() - 1;
					m_strophes2d.at(spine).push_back(tp);
					laststrophe[spineinfo] = NULL;
				} else {
					// Improperly placed *Xstrophe, so ignore
					cerr << "WARNING: unmatched strophe end: " << token << " ON LINE " << token->getLineNumber() << endl;
				}
			}
		}
	}

	// Warn about any improperly terminated *strophe:
	for (auto it = laststrophe.begin(); it != laststrophe.end(); ++it) {
		HTp token = it->second;
		if (token != NULL) {
			cerr << "WARNING: unmatched strophe begin: " << token << " ON LINE " << token->getLineNumber() << endl;
		}
	}
}



//////////////////////////////
//
// HumdrumFileStructure::analyzeStrophes --
//

bool HumdrumFileStructure::analyzeStrophes(void) {
	if (!m_analyses.m_strands_analyzed) {
		analyzeStrands();
	}
	analyzeStropheMarkers();

	int scount = (int)m_strand1d.size();
	// bool dataQ;
	vector<HTp> strophestarts;
	strophestarts.reserve(100);
	for (int i=0; i<scount; i++) {
		// dataQ = false;
		HTp current = m_strand1d.at(i).first;
		HTp send = m_strand1d.at(i).last;
		if (!send) {
			continue;
		}
		while (current && (current != send)) {
			if (!current->isInterpretation()) {
				// not a strophe (data not allowed in subspine before strophe marker).
				break;
			}

			if (current->compare(0, 3, "*S/") == 0) {
				int track = current->getTrack();
				HTp first = current->getPreviousFieldToken();
				if (first) {
					int trackp = first->getTrack();
					if (track == trackp) {
						if (first->compare(0, 3, "*S/") == 0) {
							bool found = false;
							for (int j=0; j<(int)strophestarts.size(); j++) {
								if (strophestarts[j] == first) {
									found = true;
									break;
								}
							}
							if (!found) {
								strophestarts.push_back(first);
							}
						}
					}
				}
				bool found = false;
				for (int j=0; j<(int)strophestarts.size(); j++) {
					if (strophestarts[j] == current) {
						found = true;
						break;
					}
				}
				if (!found) {
					strophestarts.push_back(current);
				}
				break;
			}
			current = current->getNextToken();
		}
	}

	// Now store strophe information in tokens.  Currently
	// spine splits are not allowed in strophes.  Spine merges
	// are OK: the first strophe will dominate in a merge.
	for (int i=0; i<(int)strophestarts.size(); i++) {
		HTp current = strophestarts[i];
		if (current->hasStrophe()) {
			continue;
		}
		current->setStrophe(strophestarts[i]);
		current = current->getNextToken();
		while (current) {
			if (current->hasStrophe()) {
				break;
			}
			if (*current == "*Xstrophe") {
				break;
			}
			if (*current == "*S-") {
				// Alternate for *Xstrophe
				break;
			}
			current->setStrophe(strophestarts[i]);
			current = current->getNextToken();
		}
	}

	return true;
}



//////////////////////////////
//
// HumdrumFileStructure::getStropheCount --  Return the number of
//    strophes in the file (for void input), or return the number
//    of strophes for a particular spine (for int input).
//

int HumdrumFileStructure::getStropheCount(void) {
	return (int)m_strophes1d.size();
}


int HumdrumFileStructure::getStropheCount(int spineindex) {
	if ((spineindex < 0) || (spineindex >= (int)m_strophes2d.size())) {
		return 0;
	}
	return (int)m_strophes2d.at(spineindex).size();
}



//////////////////////////////
//
// HumdrumFileStructure::getStropheStart --
//

HTp HumdrumFileStructure::getStropheStart(int index) {
	if ((index < 0) || (index >= (int)m_strophes1d.size())) {
		return NULL;
	}
	return m_strophes1d.at(index).first;
}

HTp HumdrumFileStructure::getStropheStart(int spine, int index) {
		if ((spine < 0) || (index < 0)) {
			return NULL;
		}
		if (spine >= (int)m_strophes2d.size()) {
			return NULL;
		}
		if (index >= (int)m_strophes2d.at(spine).size()) {
			return NULL;
		}
		return m_strophes2d.at(spine).at(index).first;
}



//////////////////////////////
//
// HumdrumFileStructure::getStropheEnd --
//

HTp HumdrumFileStructure::getStropheEnd(int index) {
	if ((index < 0) || (index >= (int)m_strophes1d.size())) {
		return NULL;
	}
	return m_strophes1d.at(index).last;
}


HTp HumdrumFileStructure::getStropheEnd(int spine, int index) {
		if ((spine < 0) || (index < 0)) {
			return NULL;
		}
		if (spine >= (int)m_strophes2d.size()) {
			return NULL;
		}
		if (index >= (int)m_strophes2d.at(spine).size()) {
			return NULL;
		}
		return m_strophes2d.at(spine).at(index).last;
}



// END_MERGE

} // end namespace hum



