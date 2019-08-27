//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Fri Aug  9 17:58:05 EDT 2019
// Last Modified: Fri Aug  9 17:58:08 EDT 2019
// Filename:      tool-homophonic.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/tool-homophonic.cpp
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Identify homophonic regions of music.
//

#include "tool-homophonic.h"
#include "Convert.h"
#include "HumRegex.h"

using namespace std;

namespace hum {

// START_MERGE


/////////////////////////////////
//
// Tool_homophonic::Tool_homophonic -- Set the recognized options for the tool.
//

Tool_homophonic::Tool_homophonic(void) {
	define("a|append=b", "Append analysis to end of input data");
	define("attacks=b", "Append attack counts for each sonority");
	define("p|prepend=b", "Prepend analysis to end of input data");
	define("r|raw-sonority=b", "Display individual sonority scores only");
	define("raw-score=b", "Display accumulated scores");
	define("M|no-marks=b", "Do not mark homophonic section notes");
	define("f|fraction=b", "calculate fraction of music that is homophonic");
	define("v|voice=b", "display voice information or fraction results");
	define("F|filename=b", "show filename for f option");
	define("n|t|threshold=d:4.0", "Threshold score sum required for homophonic texture detection");
	define("s|score=d:1.0", "Score assigned to a sonority with three or more attacks");
	define("m|intermediate-score=d:0.5", "Score to give sonority between two adjacent attack sonoroties");
}



/////////////////////////////////
//
// Tool_homophonic::run -- Do the main work of the tool.
//

bool Tool_homophonic::run(HumdrumFileSet& infiles) {
	bool status = true;
	for (int i=0; i<infiles.getCount(); i++) {
		status &= run(infiles[i]);
	}
	return status;
}


bool Tool_homophonic::run(const string& indata, ostream& out) {
	HumdrumFile infile;
	infile.readStringNoRhythm(indata);
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_homophonic::run(HumdrumFile& infile, ostream& out) {
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_homophonic::run(HumdrumFile& infile) {
	initialize();
	infile.analyzeStructure();
	m_voice_count = getExtantVoiceCount(infile);
	processFile(infile);
	infile.createLinesFromTokens();
	return true;
}



//////////////////////////////
//
// Tool_homophonic::markHomophonicNotes --
//

void Tool_homophonic::markHomophonicNotes(void) {
	// currently done with a **color spine
}



//////////////////////////////
//
// Tool_homophonic::initialize --
//

void Tool_homophonic::initialize(void) {
	m_threshold = getInteger("threshold");
	if (m_threshold < 1.0) {
		m_threshold = 1.0;
	}

	m_score = getDouble("score");
	if (m_score < 1.0) {
		m_score = 1.0;
	}

	m_intermediate_score = getDouble("intermediate-score");
	if (m_intermediate_score < 0.0) {
		m_intermediate_score = 0.0;
	}

	if (m_intermediate_score > m_score) {
		m_intermediate_score = m_score;
	}

}



//////////////////////////////
//
// Tool_homophonic::processFile --
//

void Tool_homophonic::processFile(HumdrumFile& infile) {
	vector<int> data;
	data.reserve(infile.getLineCount());

	m_homophonic.clear();
	m_homophonic.resize(infile.getLineCount());

	m_notecount.clear();
	m_notecount.resize(infile.getLineCount());
	fill(m_notecount.begin(), m_notecount.end(), 0);

	m_attacks.clear();
	m_attacks.resize(infile.getLineCount());
	fill(m_attacks.begin(), m_attacks.end(), 0);

	m_notes.clear();
	m_notes.resize(infile.getLineCount());

	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isData()) {
			continue;
		}
		data.push_back(i);
		analyzeLine(infile, i);
	}

	// change Y N Y patterns to Y Y Y
	for (int i=1; i<(int)data.size() - 1; i++) {
		if (m_homophonic[data[i]] == "Y") {
			continue;
		}
		if (m_homophonic[data[i+1]] == "N") {
			continue;
		}
		if (m_homophonic[data[i-1]] == "N") {
			continue;
		}
	  	m_homophonic[data[i]] = "NY";  // not homphonic by will get intermediate score.
	}

	vector<double> score(infile.getLineCount(), 0);
	vector<double> raw(infile.getLineCount(), 0);

	double sum;
	for (int i=0; i<(int)data.size(); i++) {
		if (m_homophonic[data[i]].find("Y") != string::npos) {
			if (m_homophonic[data[i]].find("N") != string::npos) {
				// sonority between two homophonic-like sonorities.
				// maybe also differentiate based on metric position.
				sum += m_intermediate_score;
				raw[data[i]] = m_intermediate_score;
			} else {
				sum += m_score;
				raw[data[i]] = m_score;
			}
		} else {
			sum = 0.0;
		}
		score[data[i]] = sum;
	}

	for (int i=(int)data.size()-2; i>=0; i--) {
		if (score[data[i]] == 0) {
			continue;
		}
		if (score[data[i+1]] > score[data[i]]) {
			score[data[i]] = score[data[i+1]];
		}
	}

	if (getBoolean("raw-sonority")) {
		printRawAnalysis(infile, raw);
	} else if (getBoolean("raw-score")) {
		printAccumulatedScores(infile, score);
	} else if (getBoolean("fraction")) {
		printFractionAnalysis(infile, score);
	} else if (getBoolean("attacks")) {
		printAttacks(infile, m_attacks);

	} else {
		// Color the notes within homophonic textures.
		// mark homophonic regions in red,
		// non-homophonic sonorities within these regions in green
		// and non-homophonic regions in black.
		for (int i=0; i<(int)data.size(); i++) {
			if (score[data[i]] >= m_threshold) {
				if (m_attacks[data[i]] < (int)m_notes[data[i]].size() - 1) {
					m_homophonic[data[i]] = "chartreuse";
				} else {
					m_homophonic[data[i]] = "red";
				}
			} else {
				m_homophonic[data[i]] = "black";
			}
		}
		infile.appendDataSpine(m_homophonic, "", "**color");
	}

	// problem with **color spine in javascript, so output via humdrum text
	m_humdrum_text << infile;
}



//////////////////////////////
//
// Tool_homophonic::printAccumulatedScores --
//

void Tool_homophonic::printAccumulatedScores(HumdrumFile& infile, vector<double>& score) {
	infile.appendDataSpine(score, "", "**score");
}



//////////////////////////////
//
// Tool_homophonic::printRawAnalysis --
//

void Tool_homophonic::printRawAnalysis(HumdrumFile& infile, vector<double>& raw) {
	infile.appendDataSpine(raw, "", "**raw");
}



//////////////////////////////
//
// Tool_homophonic::printAttacks --
//

void Tool_homophonic::printAttacks(HumdrumFile& infile, vector<int>& attacks) {
	infile.appendDataSpine(attacks, "", "**atks");
}



//////////////////////////////
//
// Tool_homophonic::analyzeLine --
//

void Tool_homophonic::printFractionAnalysis(HumdrumFile& infile, vector<double>& score) {
	double sum = 0.0;
	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isData()) {
			continue;
		}
		if (score[i] > m_threshold) {
			sum += infile[i].getDuration().getFloat();
		}
	}
	double total = infile.getScoreDuration().getFloat();
	int ocount = getOriginalVoiceCount(infile);
	double fraction = sum / total;
	double percent = int(fraction * 1000.0 + 0.5)/10.0;
	if (getBoolean("filename")) {
		m_free_text << infile.getFilename() << "\t";
	}
	if (getBoolean("voice")) {
		m_free_text << ocount;
		m_free_text << "\t";
		m_free_text << m_voice_count;
		m_free_text << "\t";
		if (ocount == m_voice_count) {
			m_free_text << "complete" << "\t";
		} else {
			m_free_text << "incomplete" << "\t";
		}
	}
	if (m_voice_count < 2) {
		m_free_text << -1;
	} else {
		m_free_text << percent;
	}
	m_free_text << endl;
}



//////////////////////////////
//
// Tool_homophonic::getOriginalVoiceCount --
//

int Tool_homophonic::getOriginalVoiceCount(HumdrumFile& infile) {
	HumRegex hre;
	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isReference()) {
			continue;
		}
		HTp token = infile.token(i, 0);
		if (hre.search(token, "^\\!\\!\\!voices\\s*:\\s*(\\d+)")) {
			int count = hre.getMatchInt(1);
			if (hre.search(token, "bc", "i")) {
				// add one for basso-continuo
				count++;
			}
			return count;
		}
	}
	return 0;
}



//////////////////////////////
//
// Tool_homophonic::getExtantVoiceCount --
//

int Tool_homophonic::getExtantVoiceCount(HumdrumFile& infile) {
	vector<HTp> spines = infile.getKernSpineStartList();
	return (int)spines.size();
}



//////////////////////////////
//
// Tool_homophonic::analyzeLine --
//

void Tool_homophonic::analyzeLine(HumdrumFile& infile, int line) {
	m_notes[line].reserve(10);
	HPNote note;
	if (!infile[line].isData()) {
		return;
	}
	int nullQ = 0;
	for (int i=0; i<infile[line].getFieldCount(); i++) {
		HTp token = infile.token(line, i);
		if (!token->isKern()) {
			continue;
		}
		if (token->isRest()) {
			continue;
		}
		if (token->isNull()) {
			nullQ = 1;
			token = token->resolveNull();
			if (!token) {
				continue;
			}
			if (token->isRest()) {
				continue;
			}
		} else {
			nullQ = 0;
		}
		int track = token->getTrack();
		vector<string> subtokens = token->getSubtokens();
		for (int j=0; j<(int)subtokens.size(); j++) {
			note.track = track;
			note.line = token->getLineIndex();
			note.field = token->getFieldIndex();
			note.subfield = j;
			note.token = token;
			note.text = subtokens[j];
			note.duration = Convert::recipToDuration(note.text);
			if (nullQ) {
				note.attack = false;
				note.nullQ = true;
			} else {
				note.nullQ = false;
				if ((note.text.find("_") != string::npos) ||
				    (note.text.find("]") != string::npos)) {
					note.attack = false;
				} else {
					note.attack = true;
				}
			}
			m_notes[line].push_back(note);
		}
	}

	// There must be at least three attacks to be considered homophonic
	// maybe adjust to N-1 or three voices, or a similar rule.
	vector<HumNum> adurs;
	for (int i=0; i<(int)m_notes[line].size(); i++) {
		if (m_notes[line][i].attack) {
			adurs.push_back(m_notes[line][i].duration);
			m_attacks[line]++;
		}
	}
	// if ((int)m_attacks[line] >= (int)m_notes[line].size() - 1) {
	if ((int)m_attacks[line] >= 3) {
		string value = "Y";
		// value += to_string(m_attacks[line]);
		m_homophonic[line] = value;
	} else if ((m_voice_count == 3) && (m_attacks[line] == 2)) {
		if ((adurs.size() >= 2) && (adurs[0] == adurs[1])) {
			m_homophonic[line] = "Y";
		} else {
			m_homophonic[line] = "N";
		}
	} else {
		string value = "N";
		// value += to_string(m_attacks[line]);
		m_homophonic[line] = value;
	}
	// redundant or three-or-more case:
	if (m_notes[line].size() <= 2) {
		m_homophonic[line] = "N";
	}
}


// END_MERGE

} // end namespace hum



