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
	define("x|cross=b",     "search across parts");
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

	vector<MSearchQueryToken> query;
	fillQuery(query, getString("query"));

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
		vector<MSearchQueryToken>& query) {

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
		vector<MSearchQueryToken>& dpcQuery, vector<NoteCell*>& match) {
	match.clear();

	int maxi = (int)notes.size() - index;
	if ((int)dpcQuery.size() > maxi) {
		return false;
	}
	int interval;
	for (int i=0; i<(int)dpcQuery.size(); i++) {
		if ((Convert::isNaN(notes[index+i]->getAbsDiatonicPitchClass()) &&
				Convert::isNaN(dpcQuery[i].pc)) ||
				(notes[index + i]->getAbsDiatonicPitchClass() == dpcQuery[i].pc)) {
			if ((index + i>0) && dpcQuery[i].direction) {
				interval = notes[index + i]->getAbsBase40Pitch() -
						notes[index + i - 1]->getAbsBase40Pitch();
				if ((dpcQuery[i].direction > 0) && (interval <= 0)) {
					match.clear();
					return false;
				}
				if ((dpcQuery[i].direction < 0) && (interval >= 0)) {
					match.clear();
					return false;
				}
			}
			match.push_back(notes[index+i]);
		} else {
			// not a match
			match.clear();
			return false;
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
// Tool_msearch::fillQuery -- 
//

void Tool_msearch::fillQuery(vector<MSearchQueryToken>& query, const string& input) {
	query.clear();
	char ch;

	MSearchQueryToken temp;

	for (int i=0; i<(int)input.size(); i++) {
		ch = tolower(input[i]);

		if (ch == '^') {
			temp.direction = 1;
			continue;
		}
		if (ch == 'v') {
			temp.direction = -1;
			continue;
		}

		if ((ch >= 'a' && ch <= 'g')) {
			temp.base = 7;
			temp.pc = (ch - 'a' + 5) % 7;
			query.push_back(temp);
			temp.clear();
			continue;
		} else if (ch == 'r') {
			temp.base = 7;
			temp.pc = GRIDREST;
			query.push_back(temp);
			temp.clear();
			continue;
		}

		// deal with accidentals here
		// deal with duration here
	}
}



// END_MERGE

} // end namespace hum



