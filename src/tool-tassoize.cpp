//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Wed Oct 18 13:40:23 PDT 2017
// Last Modified: Wed Oct 18 13:40:26 PDT 2017
// Filename:      tool-tassoize.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/tool-tassoize.cpp
// Syntax:        C++11
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Refinements and corrections for TiM scores imported
//                from Finale/Sibelius.
//

#include "tool-tassoize.h"
#include "Convert.h"

#include <algorithm>

using namespace std;

namespace hum {

// START_MERGE


/////////////////////////////////
//
// Tool_tassoize::Tool_tassoize -- Set the recognized options for the tool.
//

Tool_tassoize::Tool_tassoize(void) {
	// no options yet
}



/////////////////////////////////
//
// Tool_tassoize::run -- Primary interfaces to the tool.
//

bool Tool_tassoize::run(const string& indata, ostream& out) {
	HumdrumFile infile(indata);
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_tassoize::run(HumdrumFile& infile, ostream& out) {
	int status = run(infile);
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

bool Tool_tassoize::run(HumdrumFile& infile) {
	processFile(infile);

	// Re-load the text for each line from their tokens.
	infile.createLinesFromTokens();
	return true;
}



//////////////////////////////
//
// Tool_tassoize::processFile --
//

void Tool_tassoize::processFile(HumdrumFile& infile) {

	m_pstates.resize(infile.getMaxTrack() + 1);
	m_estates.resize(infile.getMaxTrack() + 1);
	for (int i=0; i<(int)m_pstates.size(); i++) {
		m_pstates[i].resize(70);
		fill(m_pstates[i].begin(), m_pstates[i].end(), 0);
		m_estates[i].resize(70);
		fill(m_estates[i].begin(), m_estates[i].end(), false);
	}

	for (int i=0; i<infile.getLineCount(); i++) {
		if (infile[i].isInterpretation()) {
			updateKeySignatures(infile, i);
			continue;
		} else if (infile[i].isBarline()) {
			clearStates();
			continue;
		} else if (infile[i].isData()) {
			checkDataLine(infile, i);
		}
	}
}



////////////////////////////////
//
// Tool_tassoize::checkDataLine --
//

void Tool_tassoize::checkDataLine(HumdrumFile& infile, int lineindex) {
	HumdrumLine& line = infile[lineindex];

	HTp token;
	bool editQ;
	int diatonic;
	// int octave;
	int accid;
	int track;
	for (int i=0; i<line.getFieldCount(); i++) {
		token = line.token(i);
		track = token->getTrack();
		if (token->isNull()) {
			continue;
		}
		if (!token->isKern()) {
			continue;
		}
		if (token->isRest()) {
			continue;
		}
		diatonic = Convert::kernToBase7(token);
		// octave = diatonic / 7;
		diatonic = diatonic % 7;
		accid = Convert::kernToAccidentalCount(token);
		editQ = false;

		// hard-wired to "i" as editorial accidental marker
		if (token->find("ni") != string::npos) {
			editQ = true;
		} else if (token->find("-i") != string::npos) {
			editQ = true;
		} else if (token->find("#i") != string::npos) {
			editQ = true;
		}

		if (editQ) {
			// store new editorial pitch state
			m_estates.at(track).at(diatonic) = true;
			m_pstates.at(track).at(diatonic) = accid;
			continue;
		}

		if (!m_estates[track][diatonic]) {
			// last note with same pitch did not have editorial accidental
			m_pstates[track][diatonic] = accid;
			continue;
		}

		if (accid != m_pstates[track][diatonic]) {
			// change in accidental (which would be explicit)
			m_pstates[track][diatonic] = accid;
			m_estates[track][diatonic] = false;
			continue;
		}

		// At this point the previous note with this pitch class
		// had an editorial accidental, and this note also has the
		// same accidentla, so the note should have an editorial
		// mark applied (since Sibelius will drop secondary editorial
		// accidentals in a measure when exporting MusicXML).

		m_estates[track][diatonic] = true;
		m_pstates[track][diatonic] = accid;

		string text = token->getText();
		string output = "";
		bool foundQ = false;
		for (int j=0; j<(int)text.size(); j++) {
			if (text[j] == 'n') {
				output += "ni";
				foundQ = true;
			} else if (text[j] == '#') {
				output += "#i";
				foundQ = true;
			} else if (text[j] == '-') {
				output += "-i";
				foundQ = true;
			} else {
				output += text[j];
			}
		}

		if (foundQ) {
			token->setText(output);
			continue;
		}

		// The note is natural, but has no natural sign.
		// add the natural sign and editorial mark.
		for (int j=(int)output.size()-1; j>=0; j--) {
			if ((tolower(output[j]) >= 'a') && (tolower(output[j]) <= 'g')) {
				output.insert(j+1, "ni");
				break;
			}
		}
		token->setText(output);
	}
}



////////////////////////////////
//
// Tool_tassoize::updateKeySignatures --
//

void Tool_tassoize::updateKeySignatures(HumdrumFile& infile, int lineindex) {
	HumdrumLine& line = infile[lineindex];
	int track;
	for (int i=0; i<line.getFieldCount(); i++) {
		if (!line.token(i)->isKeySignature()) {
			continue;
		}
		HTp token = line.token(i);
		track = token->getTrack();
		string text = token->getText();
		fill(m_pstates[track].begin(), m_pstates[track].end(), 0);
		for (int j=3; j<(int)text.size()-1; j++) {
			switch (text[j]) {
				case 'a': case 'A':
					switch (text[j+1]) {
						case '#': m_pstates[track][5] = +1;
						case '-': m_pstates[track][5] = -1;
					}
					break;

				case 'b': case 'B':
					switch (text[j+1]) {
						case '#': m_pstates[track][6] = +1;
						case '-': m_pstates[track][6] = -1;
					}
					break;

				case 'c': case 'C':
					switch (text[j+1]) {
						case '#': m_pstates[track][0] = +1;
						case '-': m_pstates[track][0] = -1;
					}
					break;

				case 'd': case 'D':
					switch (text[j+1]) {
						case '#': m_pstates[track][1] = +1;
						case '-': m_pstates[track][1] = -1;
					}
					break;

				case 'e': case 'E':
					switch (text[j+1]) {
						case '#': m_pstates[track][2] = +1;
						case '-': m_pstates[track][2] = -1;
					}
					break;

				case 'f': case 'F':
					switch (text[j+1]) {
						case '#': m_pstates[track][3] = +1;
						case '-': m_pstates[track][3] = -1;
					}
					break;

				case 'g': case 'G':
					switch (text[j+1]) {
						case '#': m_pstates[track][4] = +1;
						case '-': m_pstates[track][4] = -1;
					}
					break;
			}
			for (int j=0; j<7; j++) {
				if (m_pstates[track][j] == 0) {
					continue;
				}
				for (int k=1; k<10; k++) {
					m_pstates[track][j+k*7] = m_pstates[track][j];
				}
			}
		}
	}
}



////////////////////////////////
//
// Tool_tassoize::clearStates --
//

void Tool_tassoize::clearStates(void) {
	for (int i=0; i<(int)m_pstates.size(); i++) {
		fill(m_pstates[i].begin(), m_pstates[i].end(), 0);
	}
	for (int i=0; i<(int)m_estates.size(); i++) {
		fill(m_estates[i].begin(), m_estates[i].end(), false);
	}
}



// END_MERGE

} // end namespace hum



