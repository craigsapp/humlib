//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Fri Nov 13 02:39:59 PST 2020
// Last Modified: Sat Nov 14 21:25:35 PST 2020
// Filename:      tool-semitones.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/tool-semitones.cpp
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Switch between semitones encoding and corrected encoding.
//

#include "tool-semitones.h"
#include "Convert.h"
#include "HumRegex.h"

using namespace std;

namespace hum {

// START_MERGE


/////////////////////////////////
//
// Tool_semitones::Tool_semitones -- Set the recognized options for the tool.
//

Tool_semitones::Tool_semitones(void) {
	define("1|first=b", "mark only the first note of intervals");
	define("2|second=b", "mark only the second note of intervals");
	define("A|O|no-analysis|no-output=b", "do not print analysis spines");
	define("I|no-input=b", "do not print input data spines");
	define("M|no-mark|no-marks=b", "do not mark notes");
	define("R|no-rests=b", "ignore rests");
	define("T|no-ties=b", "do not mark ties");
	define("X|include|only=s", "include only **kern tokens with given pattern");
	define("color=s:red", "mark color");
	define("c|cdata=b", "store resulting data as **cdata (allowing display in VHV");
	define("d|down=b", "highlight notes that that have a negative semitone interval");
	define("j|jump=i:3", "starting interval defining leaps");
	define("l|leap=b", "highlight notes that have leap motion");
	define("mark=s:@", "mark character");
	define("m|midi=b", "show MIDI note number for pitches");
	define("n|count=b", "output count of intervals being marked");
	define("r|same|repeat|repeated=b", "highlight notes that are repeated ");
	define("s|step=b", "highlight notes that have step-wise motion");
	define("u|up=b", "highlight notes that that have a positive semitone interval");
	define("x|exclude=s", "exclude **kern tokens with given pattern");
}



/////////////////////////////////
//
// Tool_semitones::run -- Do the main work of the tool.
//

bool Tool_semitones::run(HumdrumFileSet& infiles) {
	bool status = true;
	for (int i=0; i<infiles.getCount(); i++) {
		status &= run(infiles[i]);
	}
	return status;
}


bool Tool_semitones::run(const string& indata, ostream& out) {
	HumdrumFile infile(indata);
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_semitones::run(HumdrumFile& infile, ostream& out) {
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_semitones::run(HumdrumFile& infile) {
	initialize();
	processFile(infile);
	return true;
}



//////////////////////////////
//
// Tool_semitones::initialize --  Initializations that only have to be done once
//    for all HumdrumFile segments.
//

void Tool_semitones::initialize(void) {
	// processing of options goes here

	m_cdataQ      = getBoolean("cdata");
	m_count       = getBoolean("count");
	m_downQ       = getBoolean("down");
	m_firstQ      = getBoolean("first");
	m_leapQ       = getBoolean("leap");
	m_midiQ       = getBoolean("midi");
	m_noanalysisQ = getBoolean("no-analysis");
	m_noinputQ    = getBoolean("no-input");
	m_nomarkQ     = getBoolean("no-marks");
	m_notiesQ     = getBoolean("no-ties");
	m_repeatQ     = getBoolean("repeat");
	m_norestsQ    = getBoolean("no-rests");
	m_secondQ     = getBoolean("second");
	m_stepQ       = getBoolean("step");
	m_upQ         = getBoolean("up");

	m_leap        = getInteger("jump");

	m_color       = getString("color");
	m_exclude     = getString("exclude");
	m_include     = getString("include");
	m_marker      = getString("mark");

	if (!m_firstQ && !m_secondQ) {
		m_firstQ  = true;
		m_secondQ = true;
	}
}



//////////////////////////////
//
// Tool_semitones::processFile --
//

void Tool_semitones::processFile(HumdrumFile& infile) {
	m_markCount = 0;
	for (int i=0; i<infile.getLineCount(); i++) {
		analyzeLine(infile, i);
	}
	if (m_markCount > 0) {
		m_humdrum_text << "!!!RDF**kern: ";
		m_humdrum_text << m_marker;
		m_humdrum_text << " = marked note";
		if (getBoolean("color")) {
			m_humdrum_text << ", color=" << m_color;
		}
		m_humdrum_text << '\n';
	}
	if (m_count) {
		showCount();
	}
}



//////////////////////////////
//
// Tool_semitones::showCount -- Give a count for the number of
//     intervals that were marked.
//

void Tool_semitones::showCount(void) {
	m_humdrum_text << "!!semitone_count: " << m_markCount;
	if (m_repeatQ) {
		m_humdrum_text << " REPEAT";
	}
	if (m_upQ) {
		m_humdrum_text << " UP";
	}
	if (m_downQ) {
		m_humdrum_text << " DOWN";
	}
	if (m_stepQ) {
		m_humdrum_text << " STEP";
	}
	if (m_leapQ) {
		m_humdrum_text << " LEAP";
	}
	if ((m_stepQ || m_leapQ) && (m_leap != 3)) {
		m_humdrum_text << " JUMP:" << m_leap;
	}
	if (m_marker != "@") {
		m_humdrum_text << " MARK:" << m_marker;
	}
	m_humdrum_text << '\n';
}



//////////////////////////////
//
// Tool_semitones::analyzeLine --  Append analysis spines after every **kern
//   spine.
//

void Tool_semitones::analyzeLine(HumdrumFile& infile, int line) {
	int group = 0;
	if (!infile[line].hasSpines()) {
		m_humdrum_text << infile[line] << "\n";
		return;
	}
	for (int i=0; i<infile[line].getFieldCount(); i++) {
		HTp token = infile.token(line, i);
		if (!m_noinputQ) {
			if (!token->isKern()) {
				m_humdrum_text << token;
				if (i < infile[line].getFieldCount() - 1) {
					m_humdrum_text << '\t';
				}
				continue;
			}
		}
		i = processKernSpines(infile, line, i, group++);
		if (!m_noinputQ) {
			if (i < infile[line].getFieldCount() - 1) {
				m_humdrum_text << '\t';
			}
		}
	}
	m_humdrum_text << '\n';
}



//////////////////////////////
//
// Tool_semitones::processKernSpine --
//

int Tool_semitones::processKernSpines(HumdrumFile& infile, int line, int start, int kspine) {
	HTp token = infile.token(line, start);
	if (!token->isKern()) {
		return start;
	}
	int track = token->getTrack();
	vector<HTp> toks;
	toks.push_back(token);
	for (int i=start+1; i<infile[line].getFieldCount(); i++) {
		HTp newtok = infile.token(line, i);
		int newtrack = newtok->getTrack();
		if (newtrack == track) {
			toks.push_back(token);
			continue;
		}
		break;
	}

	int toksize = (int)toks.size();

	// calculate intervals/MIDI note numbers if appropriate
	bool allQ = m_stepQ || m_leapQ || m_upQ || m_downQ || m_repeatQ;
	bool dirQ = m_upQ || m_downQ;
	bool typeQ = m_stepQ || m_leapQ;
	vector<string> intervals(toksize);
	if (infile[line].isData()) {
		for (int i=0; i<toksize; i++) {
			intervals[i] = getTwelveToneIntervalString(toks[i]);
		}
		if (allQ && !m_midiQ) {
			for (int i=0; i<(int)intervals.size(); i++) {
				if (intervals[i].empty()) {
					continue;
				}
            if (!isdigit(intervals[i].back())) {
					continue;
				}
				int value = stoi(intervals[i]);
				if (m_upQ && m_stepQ && (value > 0) && (value < m_leap)) {
					markInterval(toks[i]);
				} else if (m_downQ && m_stepQ && (value < 0) && (value > -m_leap)) {
					markInterval(toks[i]);
				} else if (!dirQ && m_stepQ && (value != 0) && (abs(value) < m_leap)) {
					markInterval(toks[i]);

				} else if (m_upQ && m_leapQ && (value > 0) && (value >= m_leap)) {
					markInterval(toks[i]);
				} else if (m_downQ && m_leapQ && (value < 0) && (value <= -m_leap)) {
					markInterval(toks[i]);
				} else if (!dirQ && m_leapQ && (value != 0) && (abs(value) >= m_leap)) {
					markInterval(toks[i]);

				} else if (m_repeatQ && (value == 0)) {
					markInterval(toks[i]);
				} else if (!typeQ && m_upQ && (value > 0)) {
					markInterval(toks[i]);
				} else if (!typeQ && m_downQ && (value < 0)) {
					markInterval(toks[i]);
				}
			}
		}
	}

	// print the **kern fields
	if (!m_noinputQ) {
		for (int i=0; i<toksize; i++) {
			m_humdrum_text << toks[i];
			if (i < toksize - 1) {
				m_humdrum_text << '\t';
			}
		}
	}

	// then print the parallel analysis fields

	if (!m_noanalysisQ) {
		if (!m_noinputQ) {
			m_humdrum_text << '\t';
		} else if (m_noinputQ && (kspine != 0)) {
			m_humdrum_text << '\t';
		}
		if (!infile[line].isData()) {
			if (infile[line].isLocalComment()) {
				printTokens("!", toksize);
	 		} else if (infile[line].isInterpretation()) {
				if (toks[0]->compare(0, 2, "**") == 0) { 
					if (m_cdataQ) {
						printTokens("**cdata", toksize);
					} else if (m_midiQ) {
						printTokens("**mnn", toksize);
					} else {
						printTokens("**tti", toksize);
					}
				} else {
					for (int i=0; i<toksize; i++) {
						m_humdrum_text << toks[i];
						if (i < toksize - 1) {
							m_humdrum_text << '\t';
						}
					}
				}
	 		} else if (infile[line].isBarline()) {
				printTokens(*toks[0], toksize);
			} else {
				cerr << "STRANGE ERROR " << toks[0] << endl;
			}
			return start + toksize - 1;
		}
		// print twelve-tone analyses.
		string value;
		for (int i=0; i<toksize; i++) {
			value = getTwelveToneIntervalString(toks[i]);
			m_humdrum_text << value;
			if (i < toksize - 1) {
				m_humdrum_text << '\t';
			}
		}
	}

	return start + toksize - 1;
}



//////////////////////////////
//
// Tool_semitones::markInterval -- mark the current note, any notes tied
//    after it, and then the next note and any tied notes attached to
//    that note.
//

void Tool_semitones::markInterval(HTp token) {
	if (!token->isData()) {
		return;
	}
	if (!token->isKern()) {
		return;
	}
	if (token->isNull()) {
		return;
	}
	if (token->isRest()) {
		return;
	}
	if (token->isUnpitched()) {
		return;
	}
	m_markCount++;
	token = markNote(token, m_firstQ);
	if (m_firstQ && !m_secondQ) {
		return;
	}
	// find next note
	HTp current = token->getNextToken();
	while (current) {
		if (!current->isData()) {
			current = current->getNextToken();
			continue;
		}
		if (current->isNull()) {
			current = current->getNextToken();
			continue;
		}
		markNote(current, m_secondQ);
		break;
	}
}



//////////////////////////////
//
// Tool_semitones::markNote -- make note and any tied notes after it.
//      Return the last note of a tied note (or the note if no tied notes
//      after it).
//

HTp Tool_semitones::markNote(HTp token, bool markQ) {
	string subtok = token->getSubtoken(0);
	bool hasTieEnd = false;
	if (subtok.find('_') != string::npos) {
		hasTieEnd = true;
	} else if (subtok.find(']') != string::npos) {
		hasTieEnd = true;
	}

	if (!(hasTieEnd && m_notiesQ)) {
		if (markQ) {
			addMarker(token);
		}
	}

	bool hasTie = false;
	if (subtok.find('[') != string::npos) {
		hasTie = true;
	} else if (subtok.find('_') != string::npos) {
		hasTie = true;
	}

	if (!hasTie) {
		return token;
	}
	HTp current = token->getNextToken();
	while (current) {
		if (!current->isData()) {
			current = current->getNextToken();
			continue;
		}
		if (current->isNull()) {
			current = current->getNextToken();
			continue;
		}
		subtok = current->getSubtoken(0);
		bool hasTie = false;
		if (subtok.find('[') != string::npos) {
			hasTie = true;
		} else if (subtok.find('_') != string::npos) {
			hasTie = true;
		}
		if (!hasTie) {
			if (subtok.find(']') != string::npos) {
				markNote(current, markQ);
			}
			return current;
		} else {
			return markNote(current, markQ);
		}
		break;
	}
	return NULL;
}



//////////////////////////////
//
// Tool_semitones::addMarker --
//

void Tool_semitones::addMarker(HTp token) {
	if (!m_nomarkQ) {
		string contents = m_marker;
		contents += token->getText();
		token->setText(contents);
	}
}



//////////////////////////////
//
// Tool_semitones::printTokens --
//

void Tool_semitones::printTokens(const string& value, int count) {
	for (int i=0; i<count; i++) {
		m_humdrum_text << value;
		if (i < count - 1) {
			m_humdrum_text << '\t';
		}
	}
}



///////////////////////////////
//
// Tool_semitones::getTwelveToneIntervalString --
//

string Tool_semitones::getTwelveToneIntervalString(HTp token) {
	if (token->isNull()) {
		return ".";
	}
	if (token->isRest()) {
		if (m_midiQ) {
			return "r";
		} else {
			return ".";
		}
	}
	if (token->isUnpitched()) {
		if (m_midiQ) {
			return "R";
		} else {
			return ".";
		}
	}
	if ((m_include.size() > 0) || (m_exclude.size() > 0)) {
		int status = filterData(token);
		if (!status) {
			return ".";
		}
	}
	string tok = token->getSubtoken(0);
	if (tok.find(']') != string::npos) {
		return ".";
	}
	if (tok.find('_') != string::npos) {
		return ".";
	}
	int value = Convert::kernToMidiNoteNumber(tok);

	if (m_midiQ) {
		string output;
		output = to_string(value);
		return output;
	}

	string nexttok = getNextNoteAttack(token);
	if (nexttok.empty()) {
		return ".";
	}
	if (nexttok.find('r') != string::npos) {
		// no interval since next note is a rest
		return "r";
	}
	int value2 = Convert::kernToMidiNoteNumber(nexttok);
	int interval =  value2 - value;
	string output = to_string(interval);
	return output;
}



///////////////////////////////
//
// Tool_semitones::getNextNoteAttack -- Or rest.
//

string Tool_semitones::getNextNoteAttack(HTp token) {
	HTp current = token;
	current = current->getNextToken();
	string tok;
	while (current) {
		if (!current->isData()) {
			current = current->getNextToken();
			continue;
		}
		if (current->isNull()) {
			current = current->getNextToken();
			continue;
		}
		if (current->isRest()) {
			if (!m_norestsQ) {
				return "r";
			} else {
				current = current->getNextToken();
				continue;
			}
		}
		if (current->isUnpitched()) {
			return "R";
		}
		string tok = current->getSubtoken(0);
		if (tok.find(']') != string::npos) {
			current = current->getNextToken();
			continue;
		}
		if (tok.find('_') != string::npos) {
			current = current->getNextToken();
			continue;
		}
		return tok;
	}

	if (!current) {
		return "";
	}
	if (!current->isData()) {
		return "";
	}
	// Some other strange problem.
	return ".";
}



//////////////////////////////
//
// Tool_semitones::filterData -- select or deselect an interval based
//    on regular expression pattern.  Return true if the note should
//    be kept; otherwise, return false.
//

bool Tool_semitones::filterData(HTp token) {
	vector<HTp> toks = getTieGroup(token);
	HumRegex hre;
	if (!m_exclude.empty()) {
		for (int i=0; i<(int)toks.size(); i++) {
			if (hre.search(toks[i], m_exclude)) {
				return false;
			}
		}
		return true;
	} else if (!m_include.empty()) {
		for (int i=0; i<(int)toks.size(); i++) {
			if (hre.search(toks[i], m_include)) {
				return true;
			}
		}
		return false;
	}
	return false;
}



//////////////////////////////
//
// Tool_semitones::getTieGroup --
//

vector<HTp> Tool_semitones::getTieGroup(HTp token) {
	vector<HTp> output;
	if (!token) {
		return output;
	}
	if (token->isNull()) {
		return output;
	}
	if (!token->isData()) {
		return output;
	}
	output.push_back(token);
	if (token->isRest()) {
		return output;
	}
	string subtok = token->getSubtoken(0);
	bool continues = hasTieContinue(subtok);
	HTp current = token;
	while (continues) {
		current = getNextNote(current);
		if (!current) {
			break;
		}
		string subtok = current->getSubtoken(0);
		if (subtok.find(']') != string::npos) {
			output.push_back(current);
			break;
		}
		continues = hasTieContinue(subtok);
	}
	return output;
}



//////////////////////////////
//
// Tool_semitones::hasTieContinue --
//

bool Tool_semitones::hasTieContinue(const string& value) {
	if (value.find('_') != string::npos) {
		return true;
	}
	if (value.find('[') != string::npos) {
		return true;
	}
	return false;
}



//////////////////////////////
//
// getNextNote --
//

HTp Tool_semitones::getNextNote(HTp token) {
	HTp current = token->getNextToken();
	while (current) {
		if (!current->isData()) {
			current = current->getNextToken();
			continue;
		}
		if (current->isNull()) {
			current = current->getNextToken();
			continue;
		}
		break;
	}
	return current;
}



// END_MERGE

} // end namespace hum



