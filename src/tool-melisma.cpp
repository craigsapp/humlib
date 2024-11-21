//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Aug 24 17:50:06 EDT 2019
// Last Modified: Sat Aug 24 17:50:08 EDT 2019
// Filename:      tool-melisma.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/tool-melisma.cpp
// Syntax:        C++11; humlib
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Analyze melismatic activity in vocal music.
//
//

#include "tool-melisma.h"
#include "HumRegex.h"

using namespace std;

namespace hum {

// START_MERGE


/////////////////////////////////
//
// Tool_gridtest::Tool_melisma -- Set the recognized options for the tool.
//

Tool_melisma::Tool_melisma(void) {
	define("m|min=i:2",        "minimum length to identify as a melisma");
	define("r|replace=b",      "replace lyrics with note counts");
	define("a|average|avg=b",  "calculate note-to-syllable ratio");
	define("w|words=b",        "list words that contain a melisma");
	define("p|part=b",         "also calculate note-to-syllable ratios by part");
}



///////////////////////////////
//
// Tool_melisma::run -- Primary interfaces to the tool.
//

bool Tool_melisma::run(HumdrumFileSet& infiles) {
	bool status = true;
	for (int i=0; i<infiles.getCount(); i++) {
		status &= run(infiles[i]);
	}
	return status;
}


bool Tool_melisma::run(const string& indata, ostream& out) {
	HumdrumFile infile(indata);
	return run(infile, out);
}


bool Tool_melisma::run(HumdrumFile& infile, ostream& out) {
	bool status = run(infile);
	return status;
}


bool Tool_melisma::run(HumdrumFile& infile) {
   initialize(infile);
	processFile(infile);
	return true;
}



//////////////////////////////
//
// Tool_melisma::initialize --
//

void Tool_melisma::initialize(HumdrumFile& infile) {
	// do nothing for now
}



//////////////////////////////
//
// Tool_melisma::processFile --
//

void Tool_melisma::processFile(HumdrumFile& infile) {
	vector<vector<int>> notecount;
	getNoteCounts(infile, notecount);
	vector<WordInfo> wordinfo;
	wordinfo.reserve(1000);
	map<string, int> wordlist;
	initializePartInfo(infile);

	if (getBoolean("replace")) {
		replaceLyrics(infile, notecount);
	} else if (getBoolean("words")) {
		markMelismas(infile, notecount);
		extractWordlist(wordinfo, wordlist, infile, notecount);
		printWordlist(infile, wordinfo, wordlist);
	} else {
		markMelismas(infile, notecount);
	}

}



//////////////////////////////
//
// Tool_melisma::initializePartInfo --
//

void Tool_melisma::initializePartInfo(HumdrumFile& infile) {
	m_names.clear();
	m_abbreviations.clear();
	m_partnums.clear();

	m_names.resize(infile.getTrackCount() + 1);
	m_abbreviations.resize(infile.getTrackCount() + 1);
	m_partnums.resize(infile.getTrackCount() + 1);
	fill(m_partnums.begin(), m_partnums.end(), -1);

	vector<HTp> starts;
	infile.getSpineStartList(starts);
	int ktrack = 0;
	int track = 0;
	int part = 0;
	for (int i=0; i<(int)starts.size(); i++) {
		track = starts[i]->getTrack();
		if (starts[i]->isKern()) {
			ktrack = track;
			part++;
			m_partnums[ktrack] = part;
			HTp current = starts[i];
			while (current) {
				if (current->isData()) {
					break;
				}
				if (current->compare(0, 3, "*I\"") == 0) {
					m_names[ktrack] = current->substr(3);
				} else if (current->compare(0, 3, "*I\'") == 0) {
					m_abbreviations[ktrack] = current->substr(3);
				}
				current = current->getNextToken();
			}
		} else if (ktrack) {
			m_names[track] = m_names[ktrack];
			m_abbreviations[track] = m_abbreviations[ktrack];
			m_partnums[track] = m_partnums[ktrack];
		}
	}

}



//////////////////////////////
//
// printWordlist --
//

void Tool_melisma::printWordlist(HumdrumFile& infile, vector<WordInfo>& wordinfo,
		map<string, int> words) {

	// for (auto& item : words) {
	// 	m_free_text << item.first;
	// 	if (item.second > 1) {
	// 		m_free_text << " (" << item.second << ")";
	// 	}
	// 	m_free_text << endl;
	// }

	vector<int> ncounts;
	vector<int> mcounts;
	getMelismaNoteCounts(ncounts, mcounts, infile);

	// m_free_text << "===========================" << endl;

	std::vector<HTp> kspines = infile.getKernSpineStartList();

	m_free_text << "@@BEGIN:\tMELISMAS\n";

	string filename = infile.getFilename();
	auto pos = filename.rfind("/");
	if (pos != string::npos) {
		filename = filename.substr(pos+1);
	}
	m_free_text << "@FILENAME:\t" << filename << endl;
	m_free_text << "@PARTCOUNT:\t" << kspines.size() << endl;
	m_free_text << "@WORDCOUNT:\t" << wordinfo.size() << endl;
	m_free_text << "@SCOREDURATION:\t" << getScoreDuration(infile) << endl;
	m_free_text << "@NOTES:\t\t" << ncounts[0] << endl;
	m_free_text << "@MELISMANOTES:\t" << mcounts[0] << endl;

	m_free_text << "@MELISMASCORE:\t" << int((double)mcounts[0] / (double)ncounts[0] * 1000.0 + 0.5)/10.0 << "%" << endl;
	for (int i=1; i<(int)m_partnums.size(); i++) {
		if (m_partnums[i] == 0) {
			continue;
		}
		if (m_partnums[i] == m_partnums[i-1]) {
			continue;
		}
		m_free_text << "@PARTSCORE-" << m_partnums[i] << ":\t" << int((double)mcounts[i] / (double)ncounts[i] * 1000.0 + 0.5)/10.0 << "%" << endl;
	}

	for (int i=1; i<(int)m_partnums.size(); i++) {
		if (m_partnums[i] == 0) {
			continue;
		}
		if (m_partnums[i] == m_partnums[i-1]) {
			continue;
		}
		m_free_text << "@PARTNAME-" << m_partnums[i] << ":\t" << m_names[i] << endl;
	}

	for (int i=1; i<(int)m_partnums.size(); i++) {
		if (m_partnums[i] == 0) {
			continue;
		}
		if (m_partnums[i] == m_partnums[i-1]) {
			continue;
		}
		m_free_text << "@PARTABBR-" << m_partnums[i] << ":\t" << m_abbreviations[i] << endl;
	}

	m_free_text << endl;

	for (int i=0; i<(int)wordinfo.size(); i++) {
		m_free_text << "@@BEGIN:\tWORD\n";
		m_free_text << "@PARTNUM:\t" << wordinfo[i].partnum << endl;
		// m_free_text << "@NAME:\t\t" << wordinfo[i].name << endl;
		// m_free_text << "@ABBR:\t\t" << wordinfo[i].abbreviation << endl;
		m_free_text << "@WORD:\t\t" << wordinfo[i].word << endl;
		m_free_text << "@STARTTIME:\t" << wordinfo[i].starttime.getFloat() << endl;
		m_free_text << "@ENDTIME:\t" << wordinfo[i].endtime.getFloat() << endl;
		m_free_text << "@STARTBAR:\t" << wordinfo[i].bar << endl;

		m_free_text << "@SYLLABLES:\t";
		for (int j=0; j<(int)wordinfo[i].syllables.size(); j++) {
			m_free_text << wordinfo[i].syllables[j];
			if (j < (int)wordinfo[i].syllables.size() - 1) {
				m_free_text << " ";
			}
		}
		m_free_text << endl;

		m_free_text << "@NOTECOUNTS:\t";
		for (int j=0; j<(int)wordinfo[i].notecounts.size(); j++) {
			m_free_text << wordinfo[i].notecounts[j];
			if (j < (int)wordinfo[i].notecounts.size() - 1) {
				m_free_text << " ";
			}
		}
		m_free_text << endl;

		m_free_text << "@BARLINES:\t";
		for (int j=0; j<(int)wordinfo[i].bars.size(); j++) {
			m_free_text << wordinfo[i].bars[j];
			if (j < (int)wordinfo[i].bars.size() - 1) {
				m_free_text << " ";
			}
		}
		m_free_text << endl;

		m_free_text << "@STARTTIMES:\t";
		for (int j=0; j<(int)wordinfo[i].starttimes.size(); j++) {
			m_free_text << wordinfo[i].starttimes[j].getFloat();
			if (j < (int)wordinfo[i].starttimes.size() - 1) {
				m_free_text << " ";
			}
		}
		m_free_text << endl;

		m_free_text << "@ENDTIMES:\t";
		for (int j=0; j<(int)wordinfo[i].endtimes.size(); j++) {
			m_free_text << wordinfo[i].endtimes[j].getFloat();
			if (j < (int)wordinfo[i].endtimes.size() - 1) {
				m_free_text << " ";
			}
		}
		m_free_text << endl;

		m_free_text << "@@END:\tWORD\n";
		m_free_text << endl;
	}

	m_free_text << "@@END:\tMELISMAS\n";
	m_free_text << endl;
}



//////////////////////////////
//
// Tool_melisma::getScoreDuration --
//

double Tool_melisma::getScoreDuration(HumdrumFile& infile) {
	double output = 0.0;
	for (int i=infile.getLineCount() - 1; i>=0; i--) {
		if (!infile[i].isData()) {
			continue;
		}
		output = (infile[i].getDurationFromStart() + infile[i].getDuration()).getFloat();
		break;
	}
	return output;
}



//////////////////////////////
//
// Tool_melisma::getMelismaNoteCounts --
//

void Tool_melisma::getMelismaNoteCounts(vector<int>& ncounts, vector<int>& mcounts, HumdrumFile& infile) {
	ncounts.resize(infile.getTrackCount() + 1);
	mcounts.resize(infile.getTrackCount() + 1);
	fill(ncounts.begin(), ncounts.end(), 0);
	fill(mcounts.begin(), mcounts.end(), 0);
	vector<HTp> starts = infile.getKernSpineStartList();
	for (int i=0; i<(int)starts.size(); i++) {
		HTp current = starts[i];
		int track = current->getTrack();
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
				current = current->getNextToken();
				continue;
			}
			if (!current->isNoteAttack()) {
				current = current->getNextToken();
				continue;
			}
			ncounts[track]++;
			if (current->find("@") != string::npos) {
				mcounts[track]++;
			}
			current = current->getNextToken();
		}
	}

	for (int i=1; i<(int)mcounts.size(); i++) {
		mcounts[0] += mcounts[i];
		ncounts[0] += ncounts[i];
	}
}



//////////////////////////////
//
// Tool_melisma::extractWordlist --
//

void Tool_melisma::extractWordlist(vector<WordInfo>& wordinfo, map<string, int>& wordlist,
		HumdrumFile& infile, vector<vector<int>>& notecount) {
	int mincount = getInteger("min");
	if (mincount < 2) {
		mincount = 2;
	}
	string word;
	vector<HumNum> lastTimes;
	lastTimes.resize(infile.getMaxTrack() + 1);
	std::fill(lastTimes.begin(), lastTimes.end(), -1);
	WordInfo winfo;
	for (int i=0; i<(int)notecount.size(); i++) {
		for (int j=0; j<(int)notecount[i].size(); j++) {
			if (notecount[i][j] < mincount) {
				continue;
			}
			HTp token = infile.token(i, j);
			word = extractWord(winfo, token, notecount);
			int track = token->getTrack();
			if (winfo.starttime == lastTimes.at(track)) {
				// Exclude duplicate entries of words (multiple melismas in same word).
				continue;
			}
			lastTimes.at(track) = winfo.starttime;
			wordlist[word]++;
			winfo.name = m_names[track];
			winfo.abbreviation = m_abbreviations[track];
			winfo.partnum = m_partnums[track];
			wordinfo.push_back(winfo);
		}
	}
}



//////////////////////////////
//
// Tool_melisma::extractWord --
//

string Tool_melisma::extractWord(WordInfo& winfo, HTp token, vector<vector<int>>& counts) {
	winfo.clear();
	string output = *token;
	string syllable;
	HTp current = token;
	while (current) {
		if (!current->isData()) {
			current = current->getPreviousToken();
			continue;
		}
		if (current->isNull()) {
			current = current->getPreviousToken();
			continue;
		}
		syllable = *current;
		auto pos = syllable.rfind(" ");
		if (pos != string::npos) {
			syllable = syllable.substr(pos + 1);
		}
		if (syllable.size() > 0) {
			if (syllable.at(0) == '-') {
				current = current->getPreviousToken();
				continue;
			} else {
				// found start of word
				break;
			}
		} else {
			// some strange problem
			break;
		}
	}
	if (!current) {
		// strange problem (no start of word)
		return "";
	}
	if (syllable.size() == 0) {
		return "";
	}

	winfo.starttime = current->getDurationFromStart();
	int line = current->getLineIndex();
	int field = current->getFieldIndex();
	winfo.endtime = m_endtimes[line][field];
	winfo.bar = m_measures[line];

	transform(syllable.begin(), syllable.end(), syllable.begin(), ::tolower);
	if (syllable.back() == '-') {
		syllable.resize(syllable.size() - 1);
		winfo.syllables.push_back(syllable);
		winfo.starttimes.push_back(current->getDurationFromStart());
		winfo.endtimes.push_back(m_endtimes[line][field]);
		winfo.notecounts.push_back(counts[line][field]);
		winfo.bars.push_back(m_measures[line]);
	} else {
		// single-syllable word
		winfo.endtime = getEndtime(current);
		transform(syllable.begin(), syllable.end(), syllable.begin(), ::tolower);
		winfo.word = syllable;
		winfo.syllables.push_back(syllable);
		winfo.starttimes.push_back(current->getDurationFromStart());
		winfo.endtimes.push_back(m_endtimes[line][field]);
		winfo.notecounts.push_back(counts[line][field]);
		winfo.bars.push_back(m_measures[line]);
		return syllable;
	}
	output = syllable;
	HumRegex hre;

	current = current->getNextToken();
	while (current) {
		if (!current->isData()) {
			current = current->getNextToken();
			continue;
		}
		if (current->isNull()) {
			current = current->getNextToken();
			continue;
		}
		syllable = *current;

		auto pos = syllable.find(" ");
		if (pos != string::npos) {
			syllable = syllable.substr(0, pos);
		}

		// if there is an elision of words and the second word is more
		// than one syllable, then end the word at the apostrophe.
		pos = syllable.find("'");
		if (pos != string::npos) {
			if (syllable.back() == '-') {
				syllable = syllable.substr(0, pos+1);
			}
		}

		if (syllable.size() == 0) {
			// strange problem
			return "";
		}
		if (syllable.at(0) != '-') {
			// word was not terminated properly?
			cerr << "Syllable error at syllable : " << syllable;
			cerr << ", line: " << current->getLineNumber();
			cerr << ", field: " << current->getFieldNumber();
			cerr << endl;
		} else {
			syllable = syllable.substr(1);
		}
		transform(syllable.begin(), syllable.end(), syllable.begin(), ::tolower);
		winfo.endtime = getEndtime(current);
		hre.replaceDestructive(syllable, "", "[<>.:?!;,\"]", "g");
		winfo.syllables.push_back(syllable);
		winfo.starttimes.push_back(current->getDurationFromStart());
		int cline = current->getLineIndex();
		int cfield = current->getFieldIndex();
		winfo.endtimes.push_back(m_endtimes[cline][cfield]);
		winfo.notecounts.push_back(counts[cline][cfield]);
		winfo.bars.push_back(m_measures[cline]);
		output += syllable;
		if (output.back() == '-') {
			output.resize(output.size() - 1);
			current = current->getNextToken();
			winfo.syllables.back().resize((int)winfo.syllables.back().size() - 1);
			continue;
		} else {
			// last syllable in word
			break;
		}
	}

	winfo.word = output;
	return output;
}



//////////////////////////////
//
// Tool_melisma::getEndtime --
//

HumNum Tool_melisma::getEndtime(HTp text) {
	int line = text->getLineIndex();
	int field = text->getFieldIndex();
	return m_endtimes[line][field];
}



/////////////////////////////
//
// Tool_melisma::markMelismas --
//

void Tool_melisma::markMelismas(HumdrumFile& infile, vector<vector<int>>& counts) {
	int mincount = getInteger("min");
	if (mincount < 2) {
		mincount = 2;
	}
	for (int i=0; i<(int)counts.size(); i++) {
		for (int j=0; j<(int)counts[i].size(); j++) {
			if (counts[i][j] >= mincount) {
				HTp token = infile.token(i, j);
				markMelismaNotes(token, counts[i][j]);
			}
		}
	}
	infile.appendLine("!!!RDF**kern: @ = marked note (melisma)");
	infile.createLinesFromTokens();
}



//////////////////////////////
//
// Tool_melisma::markMelismaNotes --
//

void Tool_melisma::markMelismaNotes(HTp text, int count) {
	int counter = 0;

	HTp current = text->getPreviousFieldToken();
	while (current) {
		if (current->isKern()) {
			break;
		}
		current = current->getPreviousFieldToken();
	}
	if (!current) {
		return;
	}
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
			current = current->getNextToken();
			continue;
		}
		if (current->isNoteAttack()) {
			counter++;
		}
		string text = *current;
		text += "@";
		current->setText(text);
		if (counter >= count) {
			break;
		}
		current = current->getNextToken();
	}
}



//////////////////////////////
//
// Tool_melisma::replaceLyrics --
//

void Tool_melisma::replaceLyrics(HumdrumFile& infile, vector<vector<int>>& counts) {
	for (int i=0; i<(int)counts.size(); i++) {
		for (int j=0; j<(int)counts[i].size(); j++) {
			if (counts[i][j] == -1) {
				continue;
			}
			string text = to_string(counts[i][j]);
			HTp token = infile.token(i, j);
			token->setText(text);
		}
	}
	infile.createLinesFromTokens();
}



//////////////////////////////
//
// Tool_melisma::getNoteCounts --
//

void Tool_melisma::getNoteCounts(HumdrumFile& infile, vector<vector<int>>& counts) {
	infile.initializeArray(counts, -1);
	initBarlines(infile);
	HumNum negativeOne = -1;
	infile.initializeArray(m_endtimes, negativeOne);
	vector<HTp> lyrics;
	infile.getSpineStartList(lyrics, "**text");
	for (int i=0; i<(int)lyrics.size(); i++) {
		getNoteCountsForLyric(counts, lyrics[i]);
	}
}



//////////////////////////////
//
// Tool_melisma::initBarlines --
//

void Tool_melisma::initBarlines(HumdrumFile& infile) {
	m_measures.resize(infile.getLineCount());
	fill(m_measures.begin(), m_measures.end(), 0);
	HumRegex hre;
	for (int i=1; i<infile.getLineCount(); i++) {
		if (!infile[i].isBarline()) {
			m_measures[i] = m_measures[i-1];
			continue;
		}
		HTp token = infile.token(i, 0);
		if (hre.search(token, "(\\d+)")) {
			m_measures[i] = hre.getMatchInt(1);
		}
	}
}




//////////////////////////////
//
// Tool_melisma::getNoteCountsForLyric --
//

void Tool_melisma::getNoteCountsForLyric(vector<vector<int>>& counts, HTp lyricStart) {
	HTp current = lyricStart;
	while (current) {
		if (!current->isData()) {
			current = current->getNextToken();
			continue;
		}
		if (current->isNull()) {
			current = current->getNextToken();
			continue;
		}
		int line = current->getLineIndex();
		int field = current->getFieldIndex();
		counts[line][field] = getCountForSyllable(current);
		current = current->getNextToken();
	}
}



//////////////////////////////
//
// Tool_melisma::getCountForSyllable --
//

int Tool_melisma::getCountForSyllable(HTp token) {
	if (token->back() == '&') {
		return 1;
	}
	HTp nexttok = token->getNextToken();
	int eline   = token->getLineIndex();
	int efield  = token->getFieldIndex();
	m_endtimes[eline][efield] = token->getDurationFromStart() + token->getDuration();
	while (nexttok) {
		if (!nexttok->isData()) {
			nexttok = nexttok->getNextToken();
			continue;
		}
		if (nexttok->isNull()) {
			nexttok = nexttok->getNextToken();
			continue;
		}
		// found non-null data token
		break;
	}

	HumdrumFile& infile = *token->getOwner()->getOwner();
	int endline = infile.getLineCount() - 1;
	if (nexttok) {
		endline = nexttok->getLineIndex();
	}
	int output = 0;
	HTp current = token->getPreviousFieldToken();
	while (current) {
		if (current->isKern()) {
			break;
		}
		current = current->getPreviousFieldToken();
	}
	if (!current) {
		return 0;
	}
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
			current = current->getNextToken();
			continue;
		}
		if (!current->isNoteAttack()) {
			// ignore tied notes
			m_endtimes[eline][efield] = current->getDurationFromStart() + current->getDuration();
			current = current->getNextToken();
			continue;
		}
		int line = current->getLineIndex();
		if (line < endline) {
			m_endtimes[eline][efield] = current->getDurationFromStart() + current->getDuration();
			output++;
		} else {
			break;
		}
		current = current->getNextToken();
	}

	return output;
}



// END_MERGE

} // end namespace hum



