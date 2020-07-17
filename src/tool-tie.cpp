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
	printQ = getBoolean("printable");
	mergeQ = getBoolean("merge");
	splitQ = getBoolean("split");
}



//////////////////////////////
//
// Tool_tie::processFile --
//

void Tool_tie::processFile(HumdrumFile& infile) {
	if (mergeQ) {
		mergeTies(infile);
	}
}

//////////////////////////////
//
// Tool_tie::mergeTies --
//

void Tool_tie::mergeTies(HumdrumFile& infile) {
	infile.analyzeKernTies();

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

	for (int i=0; i<(int)tiednotes.size(); i++) {
		tiednotes[i]->setText(".");
	}
	// set initial tied notes with updated recip.
	string text = *token;
	HumRegex hre;
	hre.replaceDestructive(text, recip, "\\d+(?:%\\d+)?\\.*", "g");
	hre.replaceDestructive(text, "", "\\[", "g");
	token->setText(text);
}



// END_MERGE

} // end namespace hum



