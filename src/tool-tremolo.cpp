//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sun Oct 13 11:41:16 PDT 2019
// Last Modified: Mon Oct 28 21:56:33 PDT 2019
// Filename:      tool-tremolo.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/tool-tremolo.cpp
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Tremolo expansion tool.
//

#include "tool-tremolo.h"
#include "Convert.h"
#include "HumRegex.h"

using namespace std;

namespace hum {

// START_MERGE


/////////////////////////////////
//
// Tool_tremolo::Tool_tremolo -- Set the recognized options for the tool.
//

Tool_tremolo::Tool_tremolo(void) {
	define("k|keep=b", "Keep tremolo rhythm markup");
	define("F|no-fill=b", "Do not fill in tremolo spaces");
}



/////////////////////////////////
//
// Tool_tremolo::run -- Do the main work of the tool.
//

bool Tool_tremolo::run(HumdrumFileSet& infiles) {
	bool status = true;
	for (int i=0; i<infiles.getCount(); i++) {
		status &= run(infiles[i]);
	}
	return status;
}


bool Tool_tremolo::run(const string& indata, ostream& out) {
	HumdrumFile infile(indata);
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_tremolo::run(HumdrumFile& infile, ostream& out) {
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_tremolo::run(HumdrumFile& infile) {
	processFile(infile);
	return true;
}


//////////////////////////////
//
// Tool_tremolo::processFile --
//

void Tool_tremolo::processFile(HumdrumFile& infile) {
	HumRegex hre;
	m_markup_tokens.reserve(1000);
	for (int i=infile.getLineCount()-1; i>=0; i--) {
		if (!infile[i].isData()) {
			continue;
		}
		if (infile[i].getDuration() == 0) {
			// don't deal with grace notes
			continue;
		}
		for (int j=0; j<infile[i].getFieldCount(); j++) {
			HTp token = infile.token(i, j);
			if (!token->isKern()) {
				continue;
			}
			if (token->isNull()) {
				continue;
			}
			if (hre.search(token, "@(\\d+)@")) {
				m_markup_tokens.push_back(token);
				int value = hre.getMatchInt(1);
				HumNum duration = Convert::recipToDuration(token);
				HumNum count = duration;
				count *= value;
				count /= 4;
				HumNum increment = 4;
				increment /= value;
				if (!count.isInteger()) {
					cerr << "Error: time value cannot be used: " << value << endl;
					continue;
				}
				int kcount = count.getNumerator();
				HumNum starttime = token->getDurationFromStart();
				HumNum timestamp;
				for (int k=1; k<kcount; k++) {
					timestamp = starttime + (increment * k);
					infile.insertNullDataLine(timestamp);
				}
			}
		}
	}

	if (!getBoolean("no-fill")) {
		expandTremolos();
	} else if (!m_keepQ) {
		removeMarkup();
	}

	if (m_modifiedQ) {
		infile.createLinesFromTokens();
	}

	m_humdrum_text << infile;
}


//////////////////////////////
//
// Tool_tremolo::expandTremolos --
//

void Tool_tremolo::expandTremolos(void) {
	for (int i=0; i<(int)m_markup_tokens.size(); i++) {
		expandTremolo(m_markup_tokens[i]);
	}
}



//////////////////////////////
//
// Tool_tremolo::expandTremolos --
//

void Tool_tremolo::expandTremolo(HTp token) {
	HumRegex hre;
	int value = 0;
	HumNum duration;
	HumNum repeat;
	HumNum increment;
	int tnotes = -1;
	if (hre.search(token, "@(\\d+)@")) {
		value = hre.getMatchInt(1);
		if (!Convert::isPowerOfTwo(value)) {
			cerr << "Error: not a power of two: " << token << endl;
			return;
		}
		if (value < 8) {
			cerr << "Error: tremolo can only be eighth-notes or shorter" << endl;
			return;
		}
		duration = Convert::recipToDuration(token);
		repeat = duration;
		repeat *= value;
		repeat /= 4;
		increment = 4;
		increment /= value;
		if (!repeat.isInteger()) {
			cerr << "Error: tremolo repetition count must be an integer: " << token << endl;
			return;
		}
		tnotes = repeat.getNumerator();
	} else {
		return;
	}

	int beams = log((double)(value))/log(2.0) - 2;
	string markup = "@" + to_string(value) + "@";
	string base = token->getText();
	hre.replaceDestructive(base, "", markup, "g");
	// Currently not allowed to add tremolo to beamed notes, so remove all beaming:
	hre.replaceDestructive(base, "", "[LJKk]+", "g");
	string startbeam;
	string endbeam;
	for (int i=0; i<beams; i++) {
		startbeam += 'L';
		endbeam   += 'J';
	}
	// Set the rhythm of the tremolo notes.
	// Augmentation dot is expected adjacent to regular rhythm value.
	// Maybe allow anywhere?
	hre.replaceDestructive(base, to_string(value), "\\d+%?\\d*\\.*", "g");
	string initial = base + startbeam;
	// remove slur end from start of tremolo:
	hre.replaceDestructive(initial, "", "[)]+[<>]?", "g");
	if (m_keepQ) {
		initial += markup;
	}
	string terminal = base + endbeam;
	// remove slur start information from end of tremolo:
	hre.replaceDestructive(terminal, "", "[(]+[<>]?", "g");

	// remove slur information from middle of tremolo:
	hre.replaceDestructive(base, "", "[()]+[<>]?", "g");

	token->setText(initial);
	token->getOwner()->createLineFromTokens();

	// Now fill in the rest of the tremolos.
	HumNum starttime = token->getDurationFromStart();
	HumNum timestamp = starttime + increment;
	HTp current = token->getNextToken();
	int counter = 1;
	while (current) {
		if (!current->isData()) {
			// Also check if line is non-zero duration (not a grace-note line).
			current = current->getNextToken();
			continue;
		}
		HumNum cstamp = current->getDurationFromStart();
		if (cstamp < timestamp) {
			current = current->getNextToken();
			continue;
		}
		if (cstamp > timestamp) {
			cerr << "Warning: terminating tremolo insertion early" << endl;
			break;
		}
		counter++;
		if (tnotes == counter) {
			current->setText(terminal);
		} else {
			current->setText(base);
		}
		current->getOwner()->createLineFromTokens();
		if (counter >= tnotes) {
			// done with inserting of tremolo notes.
			break;
		}
		timestamp += increment;
		current = current->getNextToken();
	}
}



//////////////////////////////
//
// Tool_tremolo::removeMarkup --  Remove markup such as "@16@" from tokens.
//

void Tool_tremolo::removeMarkup(void) {
	if (m_markup_tokens.empty()) {
		return;
	}
	HumRegex hre;
	for (int i=0; i<(int)m_markup_tokens.size(); i++) {
		HTp token = m_markup_tokens[i];
		string text = *token;
		hre.replaceDestructive(text, "", "@\\d+@");
		token->setText(text);
		token->getOwner()->createLineFromTokens();
	}
}



// END_MERGE

} // end namespace hum



