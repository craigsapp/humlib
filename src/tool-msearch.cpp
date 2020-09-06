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
	define("debug=b",               "diatonic search");
	define("q|query=s:4c4d4e4f4g",  "combined rhythm/pitch query string");
	define("p|pitch=s:cdefg",       "pitch query string");
	define("i|interval=s:2222",     "interval query string");
	define("r|d|rhythm|duration=s:44444", "rhythm query string");
	define("t|text=s:",             "lyrical text query string");
	define("O|no-overlap=b",        "do not allow matches to overlap");
	define("x|cross=b",             "search across parts");
	define("c|color=s",             "highlight color");
	define("m|mark|marker=s:@",     "marking character");
	define("M|no-mark|no-marker=b", "do not mark matches");
	define("Q|quiet=b",             "quite mode: do not summarize matches");
}



/////////////////////////////////
//
// Tool_msearch::run -- Do the main work of the tool.
//

bool Tool_msearch::run(HumdrumFileSet& infiles) {
	bool status = true;
	for (int i=0; i<infiles.getCount(); i++) {
		status &= run(infiles[i]);
	}
	return status;
}


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
	m_debugQ = getBoolean("debug");
	m_quietQ = getBoolean("quiet");
	m_nooverlapQ = getBoolean("no-overlap");
	NoteGrid grid(infile);
	if (m_debugQ) {
		grid.printGridInfo(cerr);
		// return 1;
	}
	initialize();

	if (getBoolean("text")) {
		m_text = getString("text");
	}

	if (m_text.empty()) {
		vector<MSearchQueryToken> query;
		fillMusicQuery(query);
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
	// only allowing a single character for now:
	m_markQ = !getBoolean("no-marker");
	if (!m_markQ) {
		m_marker.clear();
	} else if (!m_marker.empty()) {
		m_marker = m_marker[0];
	}
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
	HTp tok = starttoken->getNextToken();
	while (tok != NULL) {
		if (tok->empty()) {
			tok = tok->getNextToken();
			continue;
		}
		if (tok->isNull()) {
			tok = tok->getNextToken();
			continue;
		}
		if (!tok->isData()) {
			tok = tok->getNextToken();
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
			tok = tok->getNextToken();
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
			tok = tok->getNextToken();
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
			if (hre.search(words.at(j)->fullword, query.at(i).word, "i")) {
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

	if (tcount && m_markQ) {
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

	if (!m_quietQ) {
		addTextSearchSummary(infile, tcount, m_marker);
	}
}


//////////////////////////////
//
// Tool_msearch::printQuery --
//

void Tool_msearch::printQuery(vector<MSearchQueryToken>& query) {
	for (int i=0; i<(int)query.size(); i++) {
		cout << query[i];
	}
}


//////////////////////////////
//
// Tool_msearch::doMusicSearch -- do a basic melodic search of all parts.
//

void Tool_msearch::doMusicSearch(HumdrumFile& infile, NoteGrid& grid,
		vector<MSearchQueryToken>& query) {

	if (m_debugQ) {
		printQuery(query);
	}

	vector<vector<NoteCell*>> attacks;
	attacks.resize(grid.getVoiceCount());
	for (int i=0; i<grid.getVoiceCount(); i++) {
		grid.getNoteAndRestAttacks(attacks[i], i);
	}

	vector<NoteCell*>  match;
	int mcount = 0;
	for (int i=0; i<(int)attacks.size(); i++) {
		for (int j=0; j<(int)attacks[i].size(); j++) {
			bool status = checkForMatchDiatonicPC(attacks[i], j, query, match);
			if (status && !match.empty()) {
				mcount++;
				markMatch(infile, match);
				// cerr << "FOUND MATCH AT " << i << ", " << j << endl;
				// markNotes(attacks[i], j, (int)query.size());
			}
		}
	}

	if (mcount && m_markQ) {
		string content = "!!!RDF**kern: " + m_marker + " = marked note";
		if (getBoolean("color")) {
			content += ", color=\"" + getString("color") + "\"";
		}
		infile.appendLine(content);
		infile.createLinesFromTokens();
	}
	if (!m_quietQ) {
		addMusicSearchSummary(infile, mcount, m_marker);
	}
}


//////////////////////////////
//
// Tool_msearch::addMusicSearchSummary --
//

void Tool_msearch::addMusicSearchSummary(HumdrumFile& infile, int mcount, const string& marker) {
	infile.appendLine("!!@@BEGIN: MUSIC_SEARCH_RESULT");
	string line;

	line = "!!@QUERY:\t";

	if (getBoolean("query")) {
		line += " -q ";
		string qstring = getString("query");
		if (qstring.find(' ') != string::npos) {
			line += '"';
			line += qstring;
			line += '"';
		} else {
			line += qstring;
		}
	}

	if (getBoolean("pitch")) {
		line += " -p ";
		string pstring = getString("pitch");
		if (pstring.find(' ') != string::npos) {
			line += '"';
			line += pstring;
			line += '"';
		} else {
			line += pstring;
		}
	}

	if (getBoolean("rhythm")) {
		line += " -r ";
		string rstring = getString("rhythm");
		if (rstring.find(' ') != string::npos) {
			line += '"';
			line += rstring;
			line += '"';
		} else {
			line += rstring;
		}
	}

	if (getBoolean("interval")) {
		line += " -i ";
		string istring = getString("interval");
		if (istring.find(' ') != string::npos) {
			line += '"';
			line += istring;
			line += '"';
		} else {
			line += istring;
		}
	}

	infile.appendLine(line);

	line = "!!@MATCHES:\t";
	line += to_string(mcount);
	infile.appendLine(line);

	if (m_markQ) {
		line = "!!@MARKER:\t";
		line += marker;
		infile.appendLine(line);
	}

	// Print match location here.
	infile.appendLine("!!@@END: MUSIC_SEARCH_RESULT");
}



//////////////////////////////
//
// Tool_msearch::addTextSearchSummary --
//

void Tool_msearch::addTextSearchSummary(HumdrumFile& infile, int mcount, const string& marker) {
	infile.appendLine("!!@@BEGIN: TEXT_SEARCH_RESULT");
	string line;

	line = "!!@QUERY:\t";

	if (getBoolean("text")) {
		line += " -t ";
		string tstring = getString("text");
		if (tstring.find(' ') != string::npos) {
			line += '"';
			line += tstring;
			line += '"';
		} else {
			line += tstring;
		}
	}

	infile.appendLine(line);

	line = "!!@MATCHES:\t";
	line += to_string(mcount);
	infile.appendLine(line);

	if (m_markQ) {
		line = "!!@MARKER:\t";
		line += marker;
		infile.appendLine(line);
	}

	// Print match location here.
	infile.appendLine("!!@@END: TEXT_SEARCH_RESULT");
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
		// there is an extra NULL token at the end of the music to allow
		// marking tied notes.
	}
	HTp tok = mstart;
	string text;
	while (tok && (tok != mend)) {
		if (!tok->isData()) {
			tok = tok->getNextToken();
			continue;
		}
		if (tok->isNull()) {
			tok = tok->getNextToken();
			continue;
		}
		if (tok->empty()) {
			// skip marking null tokens
			tok = tok->getNextToken();
			continue;
		}
		text = tok->getText();
		size_t pos = text.find(' ');
		if (pos == string::npos) {
			// append to end of non-chord token
			text += m_marker;
		} else {
			// insert marker after first note:
			text.insert(pos, "@");
		}
		tok->setText(text);
		tok = tok->getNextToken();
		if (tok && !tok->isKern()) {
			cerr << "STRANGE LINKING WITH TEXT SPINE" << endl;
			break;
		}
	}
}



//////////////////////////////
//
// Tool_msearch::markTextMatch -- assumes monophonic voices.
//

void Tool_msearch::markTextMatch(HumdrumFile& infile, TextInfo& word) {
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
			tok = tok->getNextToken();
			continue;
		}
		if (tok->isNull()) {
			tok = tok->getNextToken();
			continue;
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
		tok = tok->getNextToken();
	}
}



//////////////////////////////
//
// Tool_msearch::checkForMatchDiatonicPC -- See if the given position
//    in the music matches the query.
//

bool Tool_msearch::checkForMatchDiatonicPC(vector<NoteCell*>& notes, int index,
		vector<MSearchQueryToken>& query, vector<NoteCell*>& match) {

	match.clear();
	int maxi = (int)notes.size() - index;
	if ((int)query.size() > maxi) {
		// Search would extend off of the end of the music, so cannot be a match.
		match.clear();
		return false;
	}

	int c = 0;

	for (int i=0; i<(int)query.size(); i++) {

		int currindex = index + i - c;
		int lastindex = index + i -c - 1;
		int nextindex = index + i -c + 1;
		if (nextindex >= (int)notes.size()) {
			nextindex = -1;
		}

		if (currindex < 0) {
			cerr << "STRANGE NEGATIVE INDEX " << currindex << endl;
			break;
		}

		// If the query item can be anything, it automatically matches:
		if (query[i].anything) {
			match.push_back(notes[currindex]);
			continue;
		}

		//////////////////////////////
		//
		// RHYTHM
		//

		if (!query[i].anyrhythm) {
			if (notes[currindex]->getDuration() != query[i].duration) {
				match.clear();
				return false;
			}
		}

		//////////////////////////////
		//
		// INTERVALS
		//

		if (query[i].dinterval > -1000) {
			// match to a specific diatonic interval to the next note

			double currpitch;
			double nextpitch;

			currpitch = notes[currindex]->getAbsDiatonicPitch();

			if (nextindex >= 0) {
				nextpitch = notes[nextindex]->getAbsDiatonicPitch();
			} else {
				nextpitch = -123456789.0;
			}

			int interval = nextpitch - currpitch;

			if (interval != query[i].dinterval) {
				match.clear();
				return false;
			}
		} else if (query[i].cinterval > -1000) {
			// match to a specific chromatic interval to the next note

			double currpitch;
			double nextpitch;

			currpitch = notes[currindex]->getAbsBase40Pitch();

			if (nextindex >= 0) {
				nextpitch = notes[nextindex]->getAbsBase40Pitch();
			} else {
				nextpitch = -123456789.0;
			}

			int interval = nextpitch - currpitch;

			if (interval != query[i].cinterval) {
				match.clear();
				return false;
			}

		} else if (!query[i].anyinterval) {

			double currpitch;
			double nextpitch;
			double lastpitch;

			currpitch = notes[currindex]->getAbsDiatonicPitchClass();

			if (nextindex >= 0) {
				nextpitch = notes[nextindex]->getAbsDiatonicPitchClass();
			} else {
				nextpitch = -123456789.0;
			}

			if (lastindex >= 0) {
				lastpitch = notes[nextindex]->getAbsDiatonicPitchClass();
			} else {
				lastpitch = -987654321.0;
			}

			if (query[i].anypitch) {
				// search forward interval
				if (nextindex < 0) {
					// Match can not go off the edge of the music.
					match.clear();
					return false;
				} else {
					// check here if either note is a rest
					if (notes[currindex]->isRest() || notes[nextindex]->isRest()) {
						match.clear();
						return false;
					}

					if (query[i].direction > 0) {
						if (nextpitch - currpitch <= 0.0) {
							match.clear();
							return false;
						}
					} if (query[i].direction < 0) {
						if (nextpitch - currpitch >= 0.0) {
							match.clear();
							return false;
						}
					} else if (query[i].direction == 0.0) {
						if (nextpitch - currpitch != 0) {
							match.clear();
							return false;
						}
					}
				}
			} else {
				// search backward interval
				if (lastindex < 0) {
					// Match can not go off the edge of the music.
					match.clear();
					return false;
				} else {
					// check here if either note is a rest.
					if (notes[currindex]->isRest() || notes[nextindex]->isRest()) {
						match.clear();
						return false;
					}

					if (query[i].direction > 0) {
						if (lastpitch - currpitch <= 0.0) {
							match.clear();
							return false;
						}
					} if (query[i].direction < 0) {
						if (lastpitch - currpitch >= 0.0) {
							match.clear();
							return false;
						}
					} else if (query[i].direction == 0.0) {
						if (lastpitch - currpitch != 0) {
							match.clear();
							return false;
						}
					}
				}
			}
		}

		//////////////////////////////
		//
		// PITCH
		//
			
		if (!query[i].anypitch) {
			double qpitch = query[i].pc;
			double npitch = 0;
			if (notes[currindex]->isRest()) {
				if (Convert::isNaN(qpitch)) {
					// both notes are rests, so they match
					match.push_back(notes[currindex]);
					continue;
				} else {
					// query is not a rest but test note is
					match.clear();
					return false;
				}
			} else if (Convert::isNaN(qpitch)) {
				// query is a rest but test note is not
				match.clear();
				return false;
			}

			if (query[i].base == 40) {
				npitch = notes[currindex]->getAbsBase40PitchClass();
			} else if (query[i].base == 12) {
				npitch = ((int)notes[currindex]->getAbsMidiPitch()) % 12;
			} else if (query[i].base == 7) {
				npitch = ((int)notes[currindex]->getAbsDiatonicPitch()) % 7;
			} else {
				npitch = notes[currindex]->getAbsBase40PitchClass();
			}

			if (qpitch != npitch) {
				match.clear();
				return false;
			}
		}

		// All requirements for the note were matched, so store note
		// and continue to next note if needed.
		match.push_back(notes[currindex]);
	}
	
	// Add extra token for marking tied notes at end of match
	if (index + (int)query.size() < (int)notes.size()) {
		match.push_back(notes[index + (int)query.size() - c]);
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

void Tool_msearch::fillMusicQuery(vector<MSearchQueryToken>& query) {
	query.clear();

	string qinput;
	string pinput;
	string iinput;
	string rinput;

	if (getBoolean("query")) {
		qinput = getString("query");
	}

	if (getBoolean("pitch")) {
		pinput = getString("pitch");
	}

	if (getBoolean("interval")) {
		iinput = getString("interval");
	}

	if (getBoolean("rhythm")) {
		rinput = getString("rhythm");
	}

	if (!rinput.empty()) {
		fillMusicQueryRhythm(query, rinput);
	}

	if (!qinput.empty()) {
		fillMusicQueryInterleaved(query, qinput);
	}

	if (!pinput.empty()) {
		fillMusicQueryPitch(query, pinput);
	}

	if (!iinput.empty()) {
		fillMusicQueryInterval(query, iinput);
	}

}



//////////////////////////////
//
// Tool_msearch::fillMusicQueryPitch --
//

void Tool_msearch::fillMusicQueryPitch(vector<MSearchQueryToken>& query,
		const string& input) {
	fillMusicQueryInterleaved(query, input);
}



//////////////////////////////
//
// Tool_msearch::fillMusicQueryRhythm --
//

void Tool_msearch::fillMusicQueryRhythm(vector<MSearchQueryToken>& query,
		const string& input) {
	string output;
	output.reserve(input.size() * 4);

	for (int i=0; i<(int)input.size(); i++) {
		output += input[i];
		output += ' ';
	}
	
	// remove spaces to allow rhythms:
	// 64 => 64
   // 32 => 32
	// 16 => 16
	for (int i=0; i<(int)output.size(); i++) {
		if ((i > 1) && (output[i] == '6') && (output[i-1] == ' ') && (output[i-2] == '1')) {
			output.erase(i-1, 1);
			i--;
		}
		if ((i > 1) && (output[i] == '2') && (output[i-1] == ' ') && (output[i-2] == '3')) {
			output.erase(i-1, 1);
			i--;
		}
		if ((i > 1) && (output[i] == '4') && (output[i-1] == ' ') && (output[i-2] == '6')) {
			output.erase(i-1, 1);
			i--;
		}
      if ((i > 0) && (output[i] == '.')) {
			output.erase(i-1, 1);
			i--;
		}
	}

	fillMusicQueryInterleaved(query, output, true);

}



//////////////////////////////
//
// Tool_msearch::fillMusicQueryInterval --
//

void Tool_msearch::fillMusicQueryInterval(vector<MSearchQueryToken>& query,
		const string& input) {

	char ch;
	int counter = 0;
	MSearchQueryToken temp;
	MSearchQueryToken *active = &temp;

	if (query.size() > 0) {
		active = &query.at(counter);
	} else {
		// what is this for?
	}

	int sign = 1;
	string alteration;
	for (int i=0; i<(int)input.size(); i++) {
		ch = input[i];
		if (ch == ' ') {
			// skip over spaces
			continue;
		}
		if ((ch == 'P') ||  (ch == 'p')) {
			alteration = "P";
			continue;
		}
		if ((ch == 'd') ||  (ch == 'D')) {
			if ((!alteration.empty()) && (alteration[0] == 'd')) {
				alteration += "d";
			} else {
				alteration = "d";
			}
			continue;
		}
		if ((ch == 'A') ||  (ch == 'a')) {
			if ((!alteration.empty()) && (alteration[0] == 'A')) {
				alteration += "A";
			} else {
				alteration = "A";
			}
			continue;
		}
		if ((ch == 'M') ||  (ch == 'm')) {
			alteration = ch;
			continue;
		}
		if (ch == '-') {
			sign = -1;
			continue;
		}
		if (ch == '+') {
			sign = +1;
			continue;
		}
		ch = tolower(ch);

		if (!isdigit(ch)) {
			// skip over non-digits (sign of interval
			// will be read retroactively).
			continue;
		}

		// check for intervals.  Intervals will trigger a
		// new element in the query list

		active->anything = false;
		active->anyinterval = false;
		// active->direction = 1;

		if (alteration.empty()) {
			// store a diatonic interval
			active->dinterval = (ch - '0') - 1; // zero-indexed interval
			active->dinterval *= sign;
		} else {
			active->cinterval = makeBase40Interval((ch - '0') - 1, alteration);
			active->cinterval *= sign;
		}
		sign = 1;
		alteration.clear();

		if (active == &temp) {
			query.push_back(temp);
			temp.clear();
		}
		counter++;
		if ((int)query.size() > counter) {
			active = &query.at(counter);
		} else {
			active = &temp;
		}
	}

	// The last element in the interval search is set to 
	// any pitch, because the interval was already checked
	// to the next note, and this value is needed to highlight
	// the next note of the interval.
	active->anything = true;
	active->anyinterval = true;
	if (active == &temp) {
		query.push_back(temp);
		temp.clear();
	}


}



//////////////////////////////
//
// Tool_msearch::makeBase40Interval --
//

int Tool_msearch::makeBase40Interval(int diatonic, const string& alteration) {
	int sign = 1;
	if (diatonic < 0) {
		sign = -1;
		diatonic = -diatonic;
	}
	bool perfectQ = false;
	int base40 = 0;
	switch (diatonic) {
		case 0:  // unison
			base40 = 0;
			perfectQ = true;
			break;
		case 1:  // second
			base40 = 6;
			perfectQ = false;
			break;
		case 2:  // third
			base40 = 12;
			perfectQ = false;
			break;
		case 3:  // fourth
			base40 = 17;
			perfectQ = true;
			break;
		case 4:  // fifth
			base40 = 23;
			perfectQ = true;
			break;
		case 5:  // sixth
			base40 = 29;
			perfectQ = false;
			break;
		case 6:  // seventh
			base40 = 35;
			perfectQ = false;
			break;
		case 7:  // octave
			base40 = 40;
			perfectQ = true;
			break;
		case 8:  // ninth
			base40 = 46;
			perfectQ = false;
			break;
		case 9:  // tenth
			base40 = 52;
			perfectQ = false;
			break;
		default:
			cerr << "cannot handle this interval yet.  Setting to unison" << endl;
			base40 = 0;
			perfectQ = 1;
	}

	if (perfectQ) {
		if (alteration == "P") {
			// do nothing since the interval is already perfect
		} else if ((!alteration.empty()) && (alteration[0] == 'd')) {
			if (alteration.size() <= 2) {
				base40 -= (int)alteration.size();
			} else {
				cerr << "TOO MUCH DIMINISHED, IGNORING" << endl;
			}
		} else if ((!alteration.empty()) && (alteration[0] == 'A')) {
			if (alteration.size() <= 2) {
				base40 += (int)alteration.size();
			} else {
				cerr << "TOO MUCH AUGMENTED, IGNORING" << endl;
			}
		}
	} else {
		if (alteration == "M") {
			// do nothing since the interval is already major
		} else if (alteration == "m") {
			base40--;
		} else if ((!alteration.empty()) && (alteration[0] == 'd')) {
			if (alteration.size() <= 2) {
				base40 -= (int)alteration.size() - 1;
			} else {
				cerr << "TOO MUCH DIMINISHED, IGNORING" << endl;
			}
		} else if ((!alteration.empty()) && (alteration[0] == 'A')) {
			if (alteration.size() <= 2) {
				base40 += (int)alteration.size();
			} else {
				cerr << "TOO MUCH AUGMENTED, IGNORING" << endl;
			}
		}
	}
	base40 *= sign;
	return base40;
}



//////////////////////////////
//
// Tool_msearch::fillMusicQueryInterleaved --
//

void Tool_msearch::fillMusicQueryInterleaved(vector<MSearchQueryToken>& query,
		const string& input, bool rhythmQ) {

	char ch;
	int counter = 0;
	MSearchQueryToken temp;
	MSearchQueryToken *active = &temp;

	if (query.size() > 0) {
		active = &query.at(counter);
	} else {
		// what is this for?
	}

	for (int i=0; i<(int)input.size(); i++) {
		ch = tolower(input[i]);

		if (ch == ' ') {
			// skip over multiple spaces
			if (i > 0) {
            if (input[i-1] == ' ') {
					continue;
				}
			}
		}

		if (ch == '^') {
			active->anything = false;
			active->anyinterval = false;
			active->direction = -1;
			continue;
		}
		if (ch == 'v') {
			active->anything = false;
			active->anyinterval = false;
			active->direction = 1;
			continue;
		}

		// process rhythm.  This must go first then intervals then pitches
		if (isdigit(ch) || (ch == '.')) {
			active->anything = false;
			active->anyrhythm = false;
			active->rhythm += ch;
			if (i < (int)input.size() - 1) {
				if (input[i+1] == ' ') {
					if (active == &temp) {
						query.push_back(temp);
						temp.clear();
					}
					counter++;
					if ((int)query.size() > counter) {
						active = &query.at(counter);
					} else {
						active = &temp;
					}
					continue;
				}
			} else {
				// this is the last charcter in the input string
				if (active == &temp) {
						query.push_back(temp);
						temp.clear();
				}
				counter++;
				if ((int)query.size() > counter) {
					active = &query.at(counter);
				} else {
					active = &temp;
				}
			}
		}

		// check for intervals.  Intervals will trigger a
		// new element in the query list
		// A new type ^ or v will not increment the query list
		// (and they will expect a pitch after them).
		if (ch == '/') {
			active->anything = false;
			active->anyinterval = false;
			active->direction = 1;
			if (active == &temp) {
				query.push_back(temp);
				temp.clear();
			}
			counter++;
			if ((int)query.size() > counter) {
				active = &query.at(counter);
			} else {
				active = &temp;
			}
			continue;
		} else if (ch == '\\') {
			active->anything = false;
			active->anyinterval = false;
			active->direction = -1;
			if (active == &temp) {
				query.push_back(temp);
				temp.clear();
			}
			counter++;
			if ((int)query.size() > counter) {
				active = &query.at(counter);
			} else {
				active = &temp;
			}
			continue;
		} else if (ch == '=') {
			active->anything = false;
			active->anyinterval = false;
			active->direction = 0;
			if (active == &temp) {
				query.push_back(temp);
				temp.clear();
			}
			counter++;
			if ((int)query.size() > counter) {
				active = &query.at(counter);
			} else {
				active = &temp;
			}
			continue;
		}


		// check for actual pitches
		if ((ch >= 'a' && ch <= 'g')) {
			active->anything = false;
			active->anypitch = false;
			active->base = 7;
			active->pc = (ch - 'a' + 5) % 7;
			if (active == &temp) {
				query.push_back(temp);
				temp.clear();
			}
			counter++;
			if ((int)query.size() > counter) {
				active = &query.at(counter);
			} else {
				active = &temp;
			}
			continue;
		} else if (ch == 'r') {
			active->anything = false;
			active->anypitch = false;
			active->base = 7;
			active->pc = GRIDREST;
			if (active == &temp) {
				query.push_back(temp);
				temp.clear();
			}
			counter++;
			if ((int)query.size() > counter) {
				active = &query.at(counter);
			} else {
				active = &temp;
			}
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
		if (query[i].anyrhythm) {
			continue;
		}
		if (query[i].rhythm.empty()) {
			continue;
		}
		query[i].duration = Convert::recipToDuration(query[i].rhythm);
	}

	// what is this for (end condition)?
	//if ((!query.empty()) && (query[0].base <= 0)) {
	//	temp.clear();
	//	temp.anything = true;
	//	query.insert(query.begin(), temp);
	//}
}


//////////////////////////////
//
// operator<< -- print MSearchQueryToken item.
//

ostream& operator<<(ostream& out, MSearchQueryToken& item) {
	out << "ITEM: "           << endl;
	out << "\tANYTHING:\t"    << item.anything    << endl;
	out << "\tANYPITCH:\t"    << item.anypitch    << endl;
	out << "\tANYINTERVAL:\t" << item.anyinterval << endl;
	out << "\tANYRHYTHM:\t"   << item.anyrhythm   << endl;
	out << "\tPC:\t\t"        << item.pc          << endl;
	out << "\tBASE:\t\t"      << item.base        << endl;
	out << "\tDIRECTION:\t"   << item.direction   << endl;
	out << "\tDINTERVAL:\t"   << item.dinterval   << endl;
	out << "\tCINTERVAL:\t"   << item.cinterval   << endl;
	out << "\tRHYTHM:\t\t"    << item.rhythm      << endl;
	out << "\tDURATION:\t"    << item.duration    << endl;
	return out;
}


// END_MERGE

} // end namespace hum



