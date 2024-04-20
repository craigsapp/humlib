//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Thu Jul 16 17:58:16 PDT 2020
// Last Modified: Tue Mar 29 22:13:01 PDT 2022
// Filename:      tool-tie.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/tool-tie.cpp
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Interface for splitting or merging tied notes.
//
// Todo:          Currently does not handle chords in a fully generalized manner (all
//                notes in chord have to have the same duration.  But splitting
//                does not even allow ties for now as well.
//
//                Fix case where merged tie group exceeds one measure and -i is also used.
//
// Limitation:    At least one part in a score must not have an overfill note
//                at the end of any given measure in order for the timing analysis
//                to be done correctly.
//

#include "tool-tie.h"
#include "Convert.h"
#include "HumRegex.h"

using namespace std;

namespace hum {

// START_MERGE


/////////////////////////////////
//
// Tool_tie::Tool_tie -- Set the recognized options for the tool.
//

Tool_tie::Tool_tie(void) {
	define("s|split=b",          "split overfill notes into tied notes across barlines.");
	define("m|merge=b",          "merge tied notes into a single note.");
	define("p|printable=b",      "merge tied notes only if single note is a printable note.");
	define("M|mark=b",           "mark overfill notes.");
	define("i|invisible=b",      "mark overfill barlines invisible.");
	define("I|skip-invisible=b", "skip invisible measures when splitting overfill durations.");
}



/////////////////////////////////
//
// Tool_tie::run -- Do the main work of the tool.
//

bool Tool_tie::run(HumdrumFileSet& infiles) {
	bool status = true;
	for (int i=0; i<infiles.getCount(); i++) {
		status &= run(infiles[i]);
	}
	return status;
}


bool Tool_tie::run(const string& indata, ostream& out) {
	HumdrumFile infile(indata);
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_tie::run(HumdrumFile& infile, ostream& out) {
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_tie::run(HumdrumFile& infile) {
	initialize();
	processFile(infile);
	infile.createLinesFromTokens();
	m_humdrum_text << infile;
	return true;
}



//////////////////////////////
//
// Tool_tie::initialize --
//

void Tool_tie::initialize(void) {
	m_printQ         = getBoolean("printable");
	m_mergeQ         = getBoolean("merge");
	m_splitQ         = getBoolean("split");
	m_markQ          = getBoolean("mark");
	m_invisibleQ     = getBoolean("invisible");
	m_skipInvisibleQ = getBoolean("skip-invisible");
}



//////////////////////////////
//
// Tool_tie::processFile --
//

void Tool_tie::processFile(HumdrumFile& infile) {
	if (m_mergeQ) {
		mergeTies(infile);
	} else if (m_splitQ) {
		splitOverfills(infile);
	} else if (m_markQ) {
		int count = markOverfills(infile);
		if (count > 0) {
			string rdfline = "!!!RDF**kern: ";
			rdfline += m_mark;
			rdfline += " = marked note, overfill (total: ";
			rdfline += to_string(count);
			rdfline += ")";
			infile.appendLine(rdfline);
		}
	}
}



//////////////////////////////
//
// Tool_tie::splitOverfills -- Both notes and rests that extend
//    past the end of the measure are split into two or more notes/rests,
//    with the notes connected with ties.
//

void Tool_tie::splitOverfills(HumdrumFile& infile) {

	for (int i=0; i<infile.getStrandCount(); i++) {
		HTp stok = infile.getStrandStart(i);
		if (!stok->isKern()) {
			continue;
		}
		HTp etok = infile.getStrandEnd(i);
		HTp tok = stok;
		while (tok && (tok != etok)) {
			if (!tok->isData()) {
				tok = tok->getNextToken();
				continue;
			}
			if (tok->isNull()) {
				tok = tok->getNextToken();
				continue;
			}
			bool overQ = checkForOverfill(tok);
			if (overQ) {
				splitToken(tok);
			}
			tok = tok->getNextToken();
		}
	}
	infile.createLinesFromTokens();


}



//////////////////////////////
//
// Tool_tie::splitToken --
//

void Tool_tie::splitToken(HTp tok) {
	HumNum duration = tok->getDuration();
	HumNum toBarline = getDurationToNextBarline(tok);
	HumNum newdur = toBarline;
	duration = duration - toBarline;
	string text = "[";
	text += tok->getText();
	HumRegex hre;
	string recip = Convert::durationToRecip(newdur);
	hre.replaceDestructive(text, recip, "\\d+(?:%\\d+)?\\.*", "g");
	tok->setText(text);
	carryForwardLeftoverDuration(duration, tok);
}



//////////////////////////////
//
// Tool_tie::carryForwardLeftoverDuration --
//

void Tool_tie::carryForwardLeftoverDuration(HumNum duration, HTp tok) {
	if (duration <= 0) {
		return;
	}
	HTp current = tok->getNextToken();
	// find next barline:
	while (current) {
		if (current->isBarline()) {
			if (m_skipInvisibleQ) {
				if (current->find("-") == string::npos) {
					break;
				}
			} else {
				break;
			}
		}
		current = current->getNextToken();
	}
	if (!current) {
		// strange problem: no next barline
		return;
	}
	if (!current->isBarline()) {
		// strange problem that cannot happen
		return;
	}
	HTp barline = current;
	if (m_invisibleQ && (barline->find('-') != string::npos)) {
		HumRegex hre;
		string text = *barline;
		hre.replaceDestructive(text, "", "-", "g");
		barline->setText(text);
	}
	HumNum bardur = getDurationToNextBarline(current);

	// find first null token after barline (that is not on a grace-note line)
	// if the original note is an overfill note, there must be
	// a null data token.
	current = current->getNextToken();
	bool foundQ = false;
	while (current) {
		if (current->isNull()) {
			HLp line = current->getOwner();
			if (!line) {
				// strange error
				return;
			}
			if (line->getDuration() > 0) {
				// non-grace note null token to exit loop
				foundQ = true;
				break;
			}
		}
		current = current->getNextToken();
	}
	if (!foundQ) {
		// strange error
		return;
	}
	if (!current->isNull()) {
		// strange error
		return;
	}
	HTp storage = current;
	// get next note or barline after null token
	current = current->getNextToken();
	foundQ = 0;
	while (current) {
		if (current->isBarline()) {
			if (m_skipInvisibleQ) {
				if (current->find("-") == string::npos) {
					foundQ = true;
					break;
				}
			} else {
				foundQ = true;
				break;
			}
		}
		if (current->isData()) {
			if (!current->isNull()) {
				foundQ = true;
				break;
			}
		}
		current = current->getNextToken();
	}

	if (!foundQ) {
		// strange error
		return;
	}
	HumNum barstart = barline->getDurationFromStart();
	HumNum nextstart = current->getDurationFromStart();
	HumNum available = nextstart - barstart;
	if (duration < available) {
		cerr << "DURATION " << duration << " IS LESS THAN AVAILABLE " << available << endl;
		// strange error
		return;
	}

	string text = *tok;
	HumRegex hre;
	hre.replaceDestructive(text, "", "[_[]", "g");
	string recip = Convert::durationToRecip(available);
	hre.replaceDestructive(text, recip, "\\d+(?:%\\d+)?\\.*", "g");

	if (available == duration) {
		// this is the last note in the tie group;
		text += ']';
		storage->setText(text);
		return;
	}

	// There is some more space for the remaining duration, but not
	// big enough for all of it.  Place the piece that can fit here
	// and then kick the can down the road for the remainder.
	text += '_';
	storage->setText(text);
	duration = duration - available;
	carryForwardLeftoverDuration(duration, storage);
}



//////////////////////////////
//
// Tool_tie::mergeTies --
//

void Tool_tie::mergeTies(HumdrumFile& infile) {
	// infile.analyzeKernTies();

	for (int i=0; i<infile.getStrandCount(); i++) {
		HTp stok = infile.getStrandStart(i);
		if (!stok->isKern()) {
			continue;
		}
		HTp etok = infile.getStrandEnd(i);
		HTp tok = stok;
		while (tok && (tok != etok)) {
			if (!tok->isData()) {
				tok = tok->getNextToken();
				continue;
			}
			if (tok->isNull()) {
				tok = tok->getNextToken();
				continue;
			}
			if (tok->find('[') == string::npos) {
				tok = tok->getNextToken();
				continue;
			}
			mergeTie(tok);
			tok = tok->getNextToken();
		}
	}
	infile.createLinesFromTokens();

}



//////////////////////////////
//
// Tool_tie::mergeTie --
//

void Tool_tie::mergeTie(HTp token) {
	if (token->find('[') == string::npos) {
		return;
	}

	vector<HTp> tiednotes;

	HumNum totaldur = token->getDuration();
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
		bool isMiddle = current->find('_') != string::npos;
		bool isEnd    = current->find(']') != string::npos;
		if (!(isMiddle ^ isEnd)) {
			// strange problem so don't merge any more notes
			break;
		}
		tiednotes.push_back(current);
		totaldur += current->getDuration();
		if (isEnd) {
			break;
		}
		current = current->getNextToken();
	}

	string recip = Convert::durationToRecip(totaldur);

	// cerr << "TOTAL DURATION OF " << token << " IS " << totaldur <<  " RECIP " << recip << endl;

	bool makeinvis = false;
	if (m_invisibleQ) {
		makeinvis = checkForInvisible(token);
	}

	for (int i=0; i<(int)tiednotes.size(); i++) {
		if (m_invisibleQ) {
			if (checkForInvisible(tiednotes[i])) {
				markNextBarlineInvisible(tiednotes[i]);
			}
		}
		tiednotes[i]->setText(".");
	}
	// set initial tied notes with updated recip.
	string text = *token;
	HumRegex hre;
	hre.replaceDestructive(text, recip, "\\d+(?:%\\d+)?\\.*", "g");
	hre.replaceDestructive(text, "", "\\[", "g");
	token->setText(text);
	if (makeinvis) {
		markNextBarlineInvisible(token);
	}

}



//////////////////////////////
//
// Tool_tie::markNextBarlineInvisible --  Multiple layers are not dealt with yet.
//

void Tool_tie::markNextBarlineInvisible(HTp tok) {
	HTp current = tok;
	while (current) {
		if (!current->isBarline()) {
			current = current->getNextToken();
			continue;
		}
		if (current->find('-') != string::npos) {
			// Already invisible
			break;
		}
		string text = *current;
		text += '-';
		current->setText(text);
		break;
	}
}



//////////////////////////////
//
// Tool_tie::markOverfills --
//

int Tool_tie::markOverfills(HumdrumFile& infile) {
	int counter = 0;

	for (int i=0; i<infile.getStrandCount(); i++) {
		HTp stok = infile.getStrandStart(i);
		if (!stok->isKern()) {
			continue;
		}
		HTp etok = infile.getStrandEnd(i);
		HTp tok = stok;
		while (tok && (tok != etok)) {
			if (!tok->isData()) {
				tok = tok->getNextToken();
				continue;
			}
			if (tok->isNull()) {
				tok = tok->getNextToken();
				continue;
			}
			bool overQ = checkForOverfill(tok);
			if (overQ) {
				string text = *tok;
				text += m_mark;
				tok->setText(text);
				counter++;
			}

			tok = tok->getNextToken();
		}
	}
	return counter;
}



//////////////////////////////
//
// Tool_tie::checkForInvisible --
//

bool Tool_tie::checkForInvisible(HTp tok) {
	HumNum duration = tok->getDuration();
	HumNum tobarline = tok->getDurationToBarline();
	if ((tok->find('[') != string::npos) ||
	   (tok->find('_') != string::npos)) {
		if (duration >= tobarline) {
			return true;
		} else {
			return false;
		}
	}
	if (duration > tobarline) {
		return true;
	} else {
		return false;
	}
}



//////////////////////////////
//
// Tool_tie::checkForOverfill --
//

bool Tool_tie::checkForOverfill(HTp tok) {
	HumNum duration = tok->getDuration();
	HumNum tobarline = getDurationToNextBarline(tok);
	if (duration > tobarline) {
		return true;
	} else {
		return false;
	}
}



//////////////////////////////
//
// Tool_tie::getDurationToNextVisibleBarline --
//

HumNum Tool_tie::getDurationToNextVisibleBarline(HTp tok) {
	HTp current = tok;
	HTp barline = NULL;
	while (current) {
		if (!current->isBarline()) {
			current = current->getNextToken();
			continue;
		}
		if (current->find("-") != string::npos) {
			// invisible so skip this barline
			current = current->getNextToken();
			continue;
		}
		barline = current;
		break;
	}

	if (!barline) {
		return tok->getDurationToEnd();
	}
	HumNum startpos   = tok->getDurationFromStart();
	HumNum endpos     = barline->getDurationFromStart();
	HumNum difference = endpos - startpos;
	return difference;
}



//////////////////////////////
//
// Tool_tie::getDurationToNextBarline --
//

HumNum Tool_tie::getDurationToNextBarline(HTp tok) {
	if (m_skipInvisibleQ) {
		return getDurationToNextVisibleBarline(tok);
	} else {
		return tok->getDurationToBarline();
	}
}


// END_MERGE

} // end namespace hum



