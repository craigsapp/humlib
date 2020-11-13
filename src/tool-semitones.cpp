//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Wed Aug 26 15:24:30 PDT 2020
// Last Modified: Sun Sep 20 12:36:46 PDT 2020
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
	// initializations go here
	define("c|cdata=b", "store resulting data as **cdata (allowing display in VHV");
	define("m|midi=b", "show MIDI note number for pitches");
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
	m_cdata = getBoolean("cdata");
	m_midi  = getBoolean("midi");
}



//////////////////////////////
//
// Tool_semitones::processFile --
//

void Tool_semitones::processFile(HumdrumFile& infile) {
	for (int i=0; i<infile.getLineCount(); i++) {
		analyzeLine(infile, i);
	}
}


//////////////////////////////
//
// Tool_semitones::analyzeLine --  Append analysis spines after every **kern
//   spine.
//

void Tool_semitones::analyzeLine(HumdrumFile& infile, int line) {
	if (!infile[line].hasSpines()) {
		m_humdrum_text << infile[line] << "\n";
		return;
	}
	for (int i=0; i<infile[line].getFieldCount(); i++) {
		HTp token = infile.token(line, i);
		if (!token->isKern()) {
			m_humdrum_text << token;
			if (i < infile[line].getFieldCount() - 1) {
				m_humdrum_text << '\t';
			}
         continue;
		}
		i = processKernSpines(infile, line, i);
		if (i < infile[line].getFieldCount() - 1) {
			m_humdrum_text << '\t';
		}
	}
	m_humdrum_text << '\n';
}


//////////////////////////////
//
// Tool_semitones::processKernSpine --
//

int Tool_semitones::processKernSpines(HumdrumFile& infile, int line, int start) {
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
	for (int i=0; i<(int)toks.size(); i++) {
		m_humdrum_text << toks[i] << '\t';
	}
	int csize = (int)toks.size();
	if (!infile[line].isData()) {
		if (infile[line].isLocalComment()) {
			printTokens("!", csize);
	 	} else if (infile[line].isInterpretation()) {
			if (toks[0]->compare(0, 2, "**") == 0) { 
				if (m_cdata) {
					printTokens("**cdata", csize);
				} else if (m_midi) {
					printTokens("**mnn", csize);
				} else {
					printTokens("**tti", csize);
				}
			} else {
				for (int i=0; i<csize; i++) {
					m_humdrum_text << toks[i];
					if (i < csize - 1) {
						m_humdrum_text << '\t';
					}
				}
			}
	 	} else if (infile[line].isBarline()) {
			printTokens(*toks[0], csize);
		} else {
			cerr << "STRANGE ERROR " << toks[0] << endl;
		}
		return start + csize - 1;
	}
	// print twelve-tone analyses.
	string value;
	for (int i=0; i<csize; i++) {
		value = getTwelveToneIntervalString(toks[i]);
		m_humdrum_text << value;
		if (i < csize - 1) {
			m_humdrum_text << '\t';
		}
	}

	return start + csize - 1;
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
		if (m_midi) {
			return "r";
		} else {
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

	if (m_midi) {
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
			return "r";
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



// END_MERGE

} // end namespace hum



