//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sun Aug 27 06:15:38 PDT 2017
// Last Modified: Mon Oct 29 10:38:26 EDT 2018
// Filename:      tool-msearch.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/tool-msearch.cpp
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Music searching tool
//

#include "tool-msearch.h"
#include "HumRegex.h"
#include "Convert.h"

using namespace std;

namespace hum {

// START_MERGE


/////////////////////////////////
//
// Tool_msearch::Tool_msearch -- Set the recognized options for the tool.
//

Tool_msearch::Tool_msearch(void) {
	define("debug=b",           "diatonic search");
	define("q|query=s:cdefg",   "query string");
	define("t|text=s:",         "lyrical text query string");
	define("x|cross=b",         "search across parts");
	define("c|color=s",         "highlight color");
	define("m|mark|marker=s:@", "marking character");
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
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_msearch::run(HumdrumFile& infile) {
	NoteGrid grid(infile);
	if (getBoolean("debug")) {
		grid.printGridInfo(cerr);
		// return 1;
	}
	initialize();

	if (getBoolean("text")) {
		m_text = getString("text");
	}

	if (m_text.empty()) {
		vector<MSearchQueryToken> query;
		fillMusicQuery(query, getString("query"));
		doMusicSearch(infile, grid, query);
	} else {
		vector<MSearchTextQuery> query;
		fillTextQuery(query, getString("text"));
		doTextSearch(infile, grid, query);
	}

	return 1;
}


//////////////////////////////
//
// Tool_msearch::initialize --
//

void Tool_msearch::initialize(void) {
	m_marker = getString("marker");
	m_marker = m_marker[0];

}



//////////////////////////////
//
// Tool_msearch::fillWords --
//

void Tool_msearch::fillWords(HumdrumFile& infile, vector<TextInfo*>& words) {
	vector<HTp> textspines;
	infile.getSpineStartList(textspines, "**silbe");
	if (textspines.empty()) {
		infile.getSpineStartList(textspines, "**text");
	}
	for (int i=0; i<(int)textspines.size(); i++) {
		fillWordsForTrack(words, textspines[i]);
	}
}



//////////////////////////////
//
// Tool_msearch::fillWordsForTrack --
//

void Tool_msearch::fillWordsForTrack(vector<TextInfo*>& words,
		HTp starttoken) {
	HTp tok = starttoken->getNextNNDT();
	while (tok != NULL) {
		if (tok->empty()) {
			tok = tok->getNextNNDT();
			continue;
		}
		if (tok->at(0) == '-') {
			// append a syllable to the end of previous word
			if (!words.empty()) {
				words.back()->fullword += tok->substr(1, string::npos);
				if (words.back()->fullword.back() == '-') {
					words.back()->fullword.pop_back();
				}
			}
			tok = tok->getNextNNDT();
			continue;
		} else {
			// start a new word
			TextInfo* temp = new TextInfo();
			temp->nexttoken = NULL;
			if (!words.empty()) {
				words.back()->nexttoken = tok;
			}
			temp->fullword = *tok;
			if (!temp->fullword.empty()) {
				if (temp->fullword.back() == '-') {
					temp->fullword.pop_back();
				}
			}
			temp->starttoken = tok;
			words.push_back(temp);
			tok = tok->getNextNNDT();
			continue;
		}
	}
}



//////////////////////////////
//
// Tool_msearch::doTextSearch -- do a basic text search of all parts.
//

void Tool_msearch::doTextSearch(HumdrumFile& infile, NoteGrid& grid,
		vector<MSearchTextQuery>& query) {

	vector<TextInfo*> words;
	words.reserve(10000);
	fillWords(infile, words);
	int tcount = 0;

	HumRegex hre;
	for (int i=0; i<(int)query.size(); i++) {
		for (int j=0; j<(int)words.size(); j++) {
			if (hre.search(words[j]->fullword, query[i].word, "i")) {
				tcount++;
				markTextMatch(infile, *words[j]);
			}
		}
	}

	string textinterp = "**text";
	vector<HTp> interps;
	infile.getSpineStartList(interps);
	//int textcount = 0;
	int silbecount = 0;
	for (int i=0; i<(int)interps.size(); i++) {
		//if (interps[i]->getText() == "**text") {
		//	textcount++;
		//}
		if (interps[i]->getText() == "**silbe") {
			silbecount++;
		}
	}
	if (silbecount > 0) {
		// giving priority to **silbe content
		textinterp = "**silbe";
	}

	if (tcount) {
		string content = "!!!RDF";
		content += textinterp;
		content += ": ";
		content += m_marker;
		content += " = marked text";
		if (getBoolean("color")) {
			content += ", color=\"" + getString("color") + "\"";
		}
		infile.appendLine(content);
		infile.createLinesFromTokens();
	}

	for (int i=0; i<(int)words.size(); i++) {
		delete words[i];
		words[i] = NULL;
	}
}



//////////////////////////////
//
// Tool_msearch::doMusicSearch -- do a basic melodic search of all parts.
//

void Tool_msearch::doMusicSearch(HumdrumFile& infile, NoteGrid& grid,
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
		string content = "!!!RDF**kern: " + m_marker + " = marked note";
		if (getBoolean("color")) {
			content += ", color=\"" + getString("color") + "\"";
		}
		infile.appendLine(content);
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
	} else {
		cerr << "GOT TO THIS STRANGE PLACE start=" << mstart << endl;
	}
	HTp tok = mstart;
	string text;
	while (tok && (tok != mend)) {
		if (!tok->isData()) {
			return;
		}
		text = tok->getText() + m_marker;
		tok->setText(text);
		tok = tok->getNextNNDT();
		if (tok && !tok->isKern()) {
			cerr << "STRANGE LINKING WITH TEXT SPINE IN getNextNNDT()" << endl;
			break;
		}
	}
}



//////////////////////////////
//
// Tool_msearch::markTextMatch -- assumes monophonic music.
//

void Tool_msearch::markTextMatch(HumdrumFile& infile, TextInfo& word) {
// ggg
	HTp mstart = word.starttoken;
	HTp mnext = word.nexttoken;
	// while (mstart && !mstart->isKern()) {
	// 	mstart = mstart->getPreviousFieldToken();
	// }
	// HTp mend = word.nexttoken;
	// while (mend && !mend->isKern()) {
	// 	mend = mend->getPreviousFieldToken();
	// }

	if (mstart) {
		if (!mstart->isData()) {
			return;
		} else if (mstart->isNull()) {
			return;
		}
	}

	//if (mend) {
	//	if (!mend->isData()) {
	//		mend = NULL;
	//	} else if (mend->isNull()) {
	//		mend = NULL;
	//	}
	//}

	HTp tok = mstart;
	string text;
	while (tok && (tok != mnext)) {
		if (!tok->isData()) {
			return;
		}
		text = tok->getText();
		if ((!text.empty()) && (text.back() == '-')) {
			text.pop_back();
			text += m_marker;
			text += '-';
		} else {
			text += m_marker;
		}
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
	bool lastIsInterval = false;
	int interval;
	bool rhymatch;
	int c = 0;
	for (int i=0; i<(int)dpcQuery.size(); i++) {
		if (dpcQuery[i].anything) {
			match.push_back(notes[index+i-c]);
			continue;
		}
		rhymatch = true;
		if ((!dpcQuery[i].rhythm.empty())
				&& (notes[index+i-c]->getDuration() != dpcQuery[i].duration)) {
			// duration needs to match query, but does not
			rhymatch = false;
		}

		// check for gross-contour queries:
		if (dpcQuery[i].base <= 0) {
			lastIsInterval = true;
			// Search by gross contour
			if ((dpcQuery[i].direction == 1) && (notes[index+i-c]->getAbsMidiPitch() >
					notes[index+i-1-c]->getAbsMidiPitch())) {
				match.push_back(notes[index+i-c]);
				continue;
			} else if ((dpcQuery[i].direction == -1) && (notes[index+i-c]->getAbsMidiPitch() <
					notes[index+i-1-c]->getAbsMidiPitch())) {
				match.push_back(notes[index+i-c]);
				continue;
			} else if ((dpcQuery[i].direction == 0) && (notes[index+i-c]->getAbsMidiPitch() ==
					notes[index+i-1-c]->getAbsMidiPitch())) {
				match.push_back(notes[index+i-c]);
				continue;
			} else {
				match.clear();
				return false;
			}
		}

		// Interface between interval moving to pitch:
		if (lastIsInterval) {
			c++;
			match.pop_back();
			lastIsInterval = false;
		}

		// Search by pitch/rest
		if (dpcQuery[i].base == 40) {
			if ((Convert::isNaN(notes[index+i-c]->getAbsBase40PitchClass()) &&
					Convert::isNaN(dpcQuery[i].pc)) ||
					(notes[index+i-c]->getAbsBase40PitchClass() == dpcQuery[i].pc)) {
				if ((index+i-c>0) && dpcQuery[i].direction) {
					interval = (int)(notes[index+i-c]->getAbsBase40Pitch() -
							notes[index+i-1-c]->getAbsBase40Pitch());
					if ((dpcQuery[i].direction > 0) && (interval <= 0)) {
						match.clear();
						return false;
					}
					if ((dpcQuery[i].direction < 0) && (interval >= 0)) {
						match.clear();
						return false;
					}
				}
				if (rhymatch) {
					match.push_back(notes[index+i-c]);
				} else {
					match.clear();
					return false;
				}
			} else {
				// not a match
				match.clear();
				return false;
			}
		} else if ((Convert::isNaN(notes[index+i-c]->getAbsDiatonicPitchClass()) &&
				Convert::isNaN(dpcQuery[i].pc)) ||
				(notes[index+i-c]->getAbsDiatonicPitchClass() == dpcQuery[i].pc)) {
			if ((index+i-c>0) && dpcQuery[i].direction) {
				interval = (int)(notes[index+i-c]->getAbsBase40Pitch() -
						notes[index+i-1-c]->getAbsBase40Pitch());
				if ((dpcQuery[i].direction > 0) && (interval <= 0)) {
					match.clear();
					return false;
				}
				if ((dpcQuery[i].direction < 0) && (interval >= 0)) {
					match.clear();
					return false;
				}
			}
			if (rhymatch) {
				match.push_back(notes[index+i-c]);
			} else {
				match.clear();
				return false;
			}

		} else {
			// not a match
			match.clear();
			return false;
		}
	}

	// Add extra token for marking tied notes at end of match
	if (index + (int)dpcQuery.size() < (int)notes.size()) {
		match.push_back(notes[index + (int)dpcQuery.size() - c]);
	} else {
		match.push_back(NULL);
	}

	return true;
}



//////////////////////////////
//
// Tool_msearch::fillTextQuery --
//

void Tool_msearch::fillTextQuery(vector<MSearchTextQuery>& query,
		const string& input) {
	query.clear();
	bool inquote = false;

	query.resize(1);

	for (int i=0; i<(int)input.size(); i++) {
		if (input[i] == '"') {
			inquote = !inquote;
			query.resize(query.size() + 1);
			continue;
		}
		if (isspace(input[i])) {
			query.resize(query.size() + 1);
		}
		query.back().word.push_back(input[i]);
		if (inquote) {
			query.back().link = true;
		}
	}
}



//////////////////////////////
//
// Tool_msearch::fillMusicQuery --
//

void Tool_msearch::fillMusicQuery(vector<MSearchQueryToken>& query,
		const string& input) {
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

		if (isdigit(ch)) {
			temp.rhythm += ch;
		}

		if (ch == '.') {
			temp.rhythm += ch;
		}

		if (ch == '/') {
			temp.direction = 1;
			temp.base = -1;
			temp.pc = -1;
			query.push_back(temp);
			temp.clear();
			continue;
		} else if (ch == '\\') {
			temp.direction = -1;
			temp.base = -1;
			temp.pc = -1;
			query.push_back(temp);
			temp.clear();
			continue;
		} else if (ch == '=') {
			temp.direction = 0;
			temp.base = -1;
			temp.pc = -1;
			query.push_back(temp);
			temp.clear();
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

		// accidentals:
		if ((!query.empty()) && (ch == 'n') && (!Convert::isNaN(query.back().pc))) {
			query.back().base = 40;
			query.back().pc = Convert::base7ToBase40((int)query.back().pc + 70) % 40;
		} else if ((!query.empty()) && (ch == '#') && (!Convert::isNaN(query.back().pc))) {
			query.back().base = 40;
			query.back().pc = (Convert::base7ToBase40((int)query.back().pc + 70) + 1) % 40;
		} else if ((!query.empty()) && (ch == '-') && (!Convert::isNaN(query.back().pc))) {
			query.back().base = 40;
			query.back().pc = (Convert::base7ToBase40((int)query.back().pc + 70) - 1) % 40;
		}
		// deal with double sharps and double flats here
	}


	// Convert rhythms to durations
	for (int i=0; i<(int)query.size(); i++) {
		if (query[i].rhythm.empty()) {
			continue;
		}
		query[i].duration = Convert::recipToDuration(query[i].rhythm);
	}

	if ((!query.empty()) && (query[0].base <= 0)) {
		temp.clear();
		temp.anything = true;
		query.insert(query.begin(), temp);
	}

}


// END_MERGE

} // end namespace hum



