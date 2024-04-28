//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Thu Apr  4 23:50:20 PDT 2024
// Last Modified: Thu Apr  4 23:50:24 PDT 2024
// Filename:      tool-vcross.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/tool-vcross.cpp
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Highlight voice crossing.
//

#include "tool-vcross.h"
#include "Convert.h"

#include <vector>

using namespace std;

namespace hum {

// START_MERGE


/////////////////////////////////
//
// Tool_vcross::Tool_vcross -- Set the recognized options for the tool.
//

Tool_vcross::Tool_vcross(void) {
}



/////////////////////////////////
//
// Tool_vcross::run -- Do the main work of the tool.
//

bool Tool_vcross::run(HumdrumFileSet& infiles) {
	bool status = true;
	for (int i=0; i<infiles.getCount(); i++) {
		status &= run(infiles[i]);
	}
	return status;
}


bool Tool_vcross::run(const string& indata, ostream& out) {
	HumdrumFile infile(indata);
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_vcross::run(HumdrumFile& infile, ostream& out) {
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_vcross::run(HumdrumFile& infile) {
	processFile(infile);
	return true;
}



//////////////////////////////
//
// Tool_vcross::initialize --  Initializations that only have to be done once
//    for all HumdrumFile segments.
//

void Tool_vcross::initialize(void) {
	m_redQ   = false;
	m_greenQ = false;
	m_blueQ  = false;
}



//////////////////////////////
//
// Tool_vcross::processFile --
//

void Tool_vcross::processFile(HumdrumFile& infile) {
	initialize();
	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isData()) {
			continue;
		}
		processLine(infile, i);
	}
	if ((!m_redQ) && (!m_greenQ) && (!m_blueQ)) {
		return;
	}
	infile.generateLinesFromTokens();
	m_humdrum_text << infile;
	if (m_redQ) {
		m_humdrum_text << "!!!RDF**kern: 🟥 = marked note, color=\"crimson\", lower part's note is higher than higher part's note" << endl;
	}
	if (m_greenQ) {
		m_humdrum_text << "!!!RDF**kern: 🟩 = marked note, color=\"limegreen\", lower part's note is equal to higher part's note" << endl;
	}
	if (m_blueQ) {
		m_humdrum_text << "!!!RDF**kern: 🟦 = marked note, color=\"dodgerblue\", higher part's note is lower than lower part's note" << endl;
	}
}



//////////////////////////////
//
// Tool_vcross::processLine --
//

void Tool_vcross::processLine(HumdrumFile& infile, int index) {
	vector<vector<HTp>> tokens;
	int ctrack = -1;
	for (int j=0; j<infile[index].getFieldCount(); j++) {
		HTp token = infile.token(index, j);
		if (!token->isKern()) {
			continue;
		}
		int track = token->getTrack();
		if (ctrack == -1) {
			tokens.resize(tokens.size() + 1);
			ctrack = track;
		} else if (ctrack != track) {
			tokens.resize(tokens.size() + 1);
			ctrack = track;
		}
		tokens.back().push_back(token);
	}
	for (int i=1; i<(int)tokens.size(); i++) {
		compareVoices(tokens.at(i-1), tokens.at(i));
	}
}



//////////////////////////////
//
// Tool_vcross::compareVoices --
//

void Tool_vcross::compareVoices(vector<HTp>& lower, vector<HTp>& higher) {
	vector<vector<int>> midihi(higher.size());
	vector<vector<int>> midilo(lower.size());

	for (int i=0; i<(int)higher.size(); i++) {
		getMidiInfo(midihi.at(i), higher.at(i));
	}

	for (int i=0; i<(int)lower.size(); i++) {
		getMidiInfo(midilo.at(i), lower.at(i));
	}

	int highestLo = -1;
	int lowestHi  = -1;

	for (int i=0; i<(int)midihi.size(); i++) {
		for (int j=0; j<(int)midihi.at(i).size(); j++) {
			if (midihi[i][j] < 0) {
				continue;
			}
			if (midihi[i][j] < 0) {
				continue;
			} else if (lowestHi < 0) {
				lowestHi = midihi[i][j];
			} else if (midihi[i][j] < lowestHi) {
				lowestHi = midihi[i][j];
			}
		}
	}

	for (int i=0; i<(int)midilo.size(); i++) {
		for (int j=0; j<(int)midilo.at(i).size(); j++) {
			if (midilo[i][j] < 0) {
				continue;
			}
			if (midilo[i][j] > highestLo) {
				highestLo = midilo[i][j];
			}
		}
	}

	if (highestLo < 0) {
		return;
	}
	if (lowestHi < 0) {
		return;
	}
	if (highestLo < lowestHi) {
		return;
	}

	for (int i=0; i<(int)midihi.size(); i++) {
		for (int j=0; j<(int)midihi.at(i).size(); j++) {
			if (midihi[i][j] < 0) {
				continue;
			}
			if (midihi[i][j] < highestLo) {
				if (!higher.at(i)->isNull()) {
					string subtok = higher.at(i)->getSubtoken(j);
					subtok += "🟦";
					m_blueQ = true;
					higher.at(i)->replaceSubtoken(j, subtok);
				}
			} else if (midihi[i][j] == highestLo) {
				if (!higher.at(i)->isNull()) {
					string subtok = higher.at(i)->getSubtoken(j);
					subtok += "🟩";
					m_greenQ = true;
					higher.at(i)->replaceSubtoken(j, subtok);
				}
			}
		}
	}

	for (int i=0; i<(int)midilo.size(); i++) {
		for (int j=0; j<(int)midilo.at(i).size(); j++) {
			if (midilo[i][j] < 0) {
				continue;
			}
			if (midilo[i][j] > lowestHi) {
				if (!lower.at(i)->isNull()) {
					string subtok = lower.at(i)->getSubtoken(j);
					subtok += "🟥";
					m_redQ = true;
					lower.at(i)->replaceSubtoken(j, subtok);
				}
			} else if (midilo[i][j] == lowestHi) {
				if (!lower.at(i)->isNull()) {
					string subtok = lower.at(i)->getSubtoken(j);
					subtok += "🟩";
					m_greenQ = true;
					lower.at(i)->replaceSubtoken(j, subtok);
				}
			}
		}
	}
}



//////////////////////////////
//
// Tool_vcross::getMidiInfo --
//

void Tool_vcross::getMidiInfo(vector<int>& midis, HTp token) {
	if (token->isNull()) {
		token = token->resolveNull();
		if (!token) {
			midis.clear();
			return;
		}
	}
	vector<string> subtokens = token->getSubtokens();
	midis.resize(subtokens.size());
	for (int i=0; i<(int)subtokens.size(); i++) {
		if (subtokens[i].find("r") != string::npos) {
			midis.at(i) = -1;
			continue;
		}
		midis.at(i) = Convert::kernToMidiNoteNumber(subtokens.at(i));
	}
}


// END_MERGE

} // end namespace hum



