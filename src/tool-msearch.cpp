//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sun Aug 27 06:15:38 PDT 2017
// Last Modified: Sun Aug 27 06:15:42 PDT 2017
// Filename:      tool-msearch.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/tool-msearch.cpp
// Syntax:        C++11
// vim:           ts=3 noexpandtab
//
// Description:   Music searching tool
//

#include "tool-msearch.h"
#include "HumRegex.h"

using namespace std;

namespace hum {

// START_MERGE


/////////////////////////////////
//
// Tool_msearch::Tool_msearch -- Set the recognized options for the tool.
//

Tool_msearch::Tool_msearch(void) {
	define("debug=b",       "diatonic search");
	define("q|query=s:c d e f g",  "query string");
	define("c|cross=b",     "search across voices");
}



/////////////////////////////////
//
// Tool_msearch::run -- Do the main work of the tool.
//

bool Tool_msearch::run(const string& indata, ostream& out) {
	HumdrumFile infile(indata);
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_msearch::run(HumdrumFile& infile, ostream& out) {
	int status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_msearch::run(HumdrumFile& infile) {
	NoteGrid grid(infile);

	vector<double> query;
	fillQueryDiatonicPC(query, getString("query"));

	if (getBoolean("debug")) {
		grid.printGridInfo(cerr);
		// return 1;
	}

	doAnalysis(infile, grid, query);

	return 1;
}





//////////////////////////////
//
// Tool_msearch::doAnalysis -- do a basic melodic analysis of all parts.
//

void Tool_msearch::doAnalysis(HumdrumFile& infile, NoteGrid& grid,
		vector<double>& query) {

	vector<vector<NoteCell*>> attacks;
	attacks.resize(grid.getVoiceCount());
	for (int i=0; i<grid.getVoiceCount(); i++) {
		grid.getNoteAndRestAttacks(attacks[i], i);
	}

	vector<NoteCell*>  match;
	int mcount = 0;
	for (int i=0; i<(int)attacks.size(); i++) {
		for (int j=0; j<(int)attacks[i].size(); j++) {
			checkForMatchDiatonicPC(attacks[i], j, query, match);
			if (!match.empty()) {
				mcount++;
				markMatch(infile, match);
				// cerr << "FOUND MATCH AT " << i << ", " << j << endl;
				// markNotes(attacks[i], j, (int)query.size());
			}
		}
	}
	
	if (mcount) {
		infile.appendLine("!!!RDF**kern: @ = marked note");
		infile.createLinesFromTokens();
	}
}


//////////////////////////////
//
// Tool_msearch::markMatch -- assumes monophonic music.
//

void Tool_msearch::markMatch(HumdrumFile& infile, vector<NoteCell*>& match) {
	if (match.empty()) {
		return;
	}
	HTp mstart = match[0]->getToken();
	HTp mend = NULL;
	if (match.back() != NULL) {
		mend = match.back()->getToken();
	}
	HTp tok = mstart;
	string text;
	while (tok && (tok != mend)) {
		if (!tok->isData()) {
			return;
		}
		text = tok->getText();
		text += '@';
		tok->setText(text);
		tok = tok->getNextNNDT();
	}
}


//////////////////////////////
//
// Tool_msearch::checkForMatchDiatonicPC --
//

bool Tool_msearch::checkForMatchDiatonicPC(vector<NoteCell*>& notes, int index, 
		vector<double>& dpcQuery, vector<NoteCell*>& match) {
	match.clear();

	int maxi = (int)notes.size() - index;
	if ((int)dpcQuery.size() > maxi) {
		return false;
	}
	for (int i=0; i<(int)dpcQuery.size(); i++) {
		if (notes[index + i]->getAbsDiatonicPitchClass() != dpcQuery[i]) {
			match.clear();
			return false;
		} else {
			match.push_back(notes[index+i]);
		}
	}

	// Add extra token for marking tied notes at end of match
	if (index + (int)dpcQuery.size() < (int)notes.size()) {
		match.push_back(notes[index + (int)dpcQuery.size()]);
	} else {
		match.push_back(NULL);
	}

	return true;
}



//////////////////////////////
//
// Tool_msearch::fillQueryDiatonicPC -- 
//

void Tool_msearch::fillQueryDiatonicPC(vector<double>& query, const string& input) {
	query.clear();
	char ch;
	int value;
	for (int i=0; i<(int)input.size(); i++) {
		ch = tolower(input[i]);
		if (ch >= 'a' && ch <= 'g') {
			value = (ch - 'a' + 5) % 7;
			query.push_back((double)value);
		} else if (ch == 'r') {
			query.push_back(GRIDREST);
		}
	}
}



// END_MERGE

} // end namespace hum



