//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Thu Jul 16 17:58:16 PDT 2020
// Last Modified: Thu Jul 16 19:05:13 PDT 2020
// Filename:      tool-tie.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/tool-tie.cpp
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Interface for splitting or merging tied notes.
//
// Todo:          Currently does not handle chords in a fully generalized manner (all
//                notes in chord have to have the same duration.
//
//                Only does merging (need to add splitting and printable test).
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
	define("s|split=b", "split overfill notes into tied notes across barlines.");
	define("m|merge=b", "merge tied notes into a single note.");
	define("p|printable=b", "merge tied notes only if single note is a printable note.");
	define("M|mark=b", "Mark overfill notes.");
	define("i|invisible=b", "Mark overfill barlines invisible.");
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
	return true;
}



//////////////////////////////
//
// Tool_tie::initialize --
//

void Tool_tie::initialize(void) {
	m_printQ      = getBoolean("printable");
	m_mergeQ      = getBoolean("merge");
	m_splitQ      = getBoolean("split");
	m_markQ       = getBoolean("mark");
	m_invisibleQ  = getBoolean("invisible");
}



//////////////////////////////
//
// Tool_tie::processFile --
//

void Tool_tie::processFile(HumdrumFile& infile) {
	if (m_mergeQ) {
		mergeTies(infile);
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
	if (tok->find('[') != string::npos) {
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
	HumNum tobarline = tok->getDurationToBarline();
	if (duration > tobarline) {
		return true;
	} else {
		return false;
	}
}


// END_MERGE

} // end namespace hum



