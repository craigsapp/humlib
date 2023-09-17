//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Sep  2 12:02:01 CEST 2023
// Last Modified: Sat Sep  2 12:02:04 CEST 2023
// Filename:      tool-textdur.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/tool-textdur.cpp
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Calculate duration of text (lyrics).  Either as **text or **sylba
//

#include "tool-textdur.h"
#include "HumRegex.h"

using namespace std;

namespace hum {

// START_MERGE


/////////////////////////////////
//
// Tool_textdur::Tool_textdur -- Set the recognized options for the tool.
//

Tool_textdur::Tool_textdur(void) {
	// add command-line options here
	define("a|analysis=b", "calculate and display analyses");
	define("m|melisma=b", "Count number of notes for each syllable");
	define("d|duration=b", "Duration of each syllable");
	define("i|interleave=b", "Preserve original text, and place analyses below text");
}



/////////////////////////////////
//
// Tool_textdur::run -- Do the main work of the tool.
//

bool Tool_textdur::run(HumdrumFileSet& infiles) {
	bool status = true;
	for (int i=0; i<infiles.getCount(); i++) {
		status &= run(infiles[i]);
	}
	return status;
}


bool Tool_textdur::run(const string& indata, ostream& out) {
	HumdrumFile infile(indata);
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_textdur::run(HumdrumFile& infile, ostream& out) {
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_textdur::run(HumdrumFile& infile) {
	initialize();
	processFile(infile);
	return true;
}


//////////////////////////////
//
// Tool_textdur::initialize --
//

void Tool_textdur::initialize(void) {
	m_analysisQ = getBoolean("analysis");
	m_melismaQ = getBoolean("melisma");
	if (m_melismaQ) {
		m_durationQ = false;
	}
	if (getBoolean("duration")) {
		m_durationQ = true;
	}
	m_interleaveQ = getBoolean("interleave");
}



//////////////////////////////
//
// Tool_textdur::processFile --
//

void Tool_textdur::processFile(HumdrumFile& infile) {
	getTextSpineStarts(infile, m_textStarts);
	fillInstrumentNameInfo();

	if (m_textStarts.empty()) {
		return;
	}

	m_syllables.clear();
	m_syllables.resize(m_textStarts.size());

	m_durations.clear();
	m_durations.resize(m_textStarts.size());

	m_melismas.clear();
	m_melismas.resize(m_textStarts.size());

	for (int i=0; i<(int)m_textStarts.size(); i++) {
		processTextSpine(m_textStarts, i);
	}

	if (!m_interleaveQ) {
		for (int i=0; i<(int)m_textStarts.size(); i++) {
			string text = *m_textStarts[i];
			if (m_melismaQ) {
				text += "-melisma";
			} else {
				text += "-duration";
			}
			m_textStarts[i]->setText(text);
		}
	}

	if (!m_interleaveQ) {
		if (m_melismaQ) {
			printMelismas(infile);
		} else {
			printDurations(infile);
		}
	} else {
		printInterleaved(infile);
	}

	if (m_analysisQ) {
		printAnalysis();
	}

}



//////////////////////////////
//
// Tool_textdur::fillInstrumentNameInfo --
//

void Tool_textdur::fillInstrumentNameInfo(void) {
	m_columnName.clear();
	m_columnName.resize(m_textStarts.size());
	for (int i=0; i<(int)m_columnName.size(); i++) {
		m_columnName[i] = getColumnName(m_textStarts[i]);
	}
}



//////////////////////////////
//
// Tool_textdur::getColumnName --
//

string Tool_textdur::getColumnName(HTp token) {
	HTp kerntok = getTandemKernToken(token);
	if (!kerntok) {
		return "unknown";
	}
	int track = kerntok->getTrack();
	string output = "Track ";
	output += to_string(track);

	HTp current = kerntok->getNextToken();
	HumRegex hre;
	while (current) {
		if (current->isData()) {
			break;
		}
		if (hre.search(current, "^\\*I\\\"(.*)\\s*$")) {
			output = hre.getMatch(1);
			break;
		}
		current = current->getNextToken();
	}
	return output;
}




//////////////////////////////
//
// Tool_textdur::printAnalysis --
//

void Tool_textdur::printAnalysis(void) {
	if (m_melismaQ) {
		printMelismaAverage();
	}
	if (m_durationQ) {
		printDurationAverage();
	}
	printHtmlContent();
}



//////////////////////////////
//
// Tool_textdur::printHtmlContent --
//

void Tool_textdur::printHtmlContent(void) {

	m_humdrum_text << "!!@@BEGIN: PREHTML" << endl;
	m_humdrum_text << "!!@CONTENT: " << endl;
	m_humdrum_text << "!! <h1> Syllable length analysis </h1>" << endl;

	m_humdrum_text << "!! <details open> <summary> " << endl;
	m_humdrum_text << "!! <h3> Number of syllables: @{TOOL-textdur-total-syllables} </h3>" << endl;
	m_humdrum_text << "!! </summary>" << endl;
	m_humdrum_text << "!! <table style='margin-left:50px;width:400px;'> " << endl;

	int sum = 0;
	for (int i=0; i<(int)m_melismas.size(); i++) {
		sum += (int)m_melismas[i].size();
	}

	for (int i=(int)m_melismas.size() - 1; i>=0; i--) {
		double percent = 1.0 * m_melismas.at(i).size() / sum;
		percent = int(percent * 10000.0 + 0.5) / 100.0;
		m_humdrum_text << "!! <tr><td> " << m_columnName.at(i) << "</td>"
		               << "<td style='padding-left:20px; text-align:right;'>" << m_melismas.at(i).size()
		               << "</td><td style='width:100%;'> (" << percent << "%) </td></tr>" << endl;
	}
	m_humdrum_text << "!! </table> " << endl;
	m_humdrum_text << "!! </details>" << endl;

	if (m_melismaQ) {
		m_humdrum_text << "!! <h3> Average syllable note length: @{TOOL-textdur-average-notes-per-syllable} </h3>" << endl;
		m_humdrum_text << "!! <div style='margin-left:50px'></div>" << endl;
		printMelismaHtmlHistogram();
		m_humdrum_text << "!! </div>" << endl;
	}
	if (m_durationQ) {
		m_humdrum_text << "!! <div style='height:50px;'></div>" << endl;
		m_humdrum_text << "!! <h3> Average syllable duration: @{TOOL-textdur-average-syllable-duration} quarter notes </h3>" << endl;
		printDurationHtmlHistogram();
	}
	m_humdrum_text << "!!@@END: PREHTML" << endl;

}


//////////////////////////////
//
// Tool_textdur::printDurationHtmlHistogram --
//

void Tool_textdur::printDurationHtmlHistogram(void) {

	map<HumNum, int> durinfo;
	double total = 0;
	for (int i=0; i<(int)m_durations.size(); i++) {
		// -1 on next line for *- terminator?
		for (int j=0; j<(int)m_durations[i].size() -1 ; j++) {
			HumNum dur = m_durations[i][j];
			int value = durinfo[dur];
			durinfo[dur] = value + 1;
			total += 1.0;
		}
	}

	double maxlen = 0;
	for (auto it = durinfo.begin(); it != durinfo.end(); it++) {
		if (it->second > maxlen) {
			maxlen = it->second;
		}
	}

	m_humdrum_text << "!! <table class='duration-histogram'>" << endl;
	m_humdrum_text << "!! <tr> <th style='white-space:pre; text-align:center;'> Duration (quarter notes)</th> <th style='padding-left:10px; width:100%;'> Syllable count </th> </tr> " << endl;
	stringstream value;
	for (auto it = durinfo.begin(); it != durinfo.end(); it++) {
		double length = 1.0 * it->second / maxlen * 400;
		double percent = 1.0 * it->second / total * 100.0;
		percent = (int)(percent * 100.0 + 0.5) / 100.0;
		value.str("");
		it->first.printMixedFraction(value, "+");
		m_humdrum_text << "!! <tr><td style='padding-left:100px;'> " << value.str() << "</td><td style='padding-left:10px;white-space:pre;'>";
		m_humdrum_text << " <span style='display:inline-block;background-color:black;height:100%;width:" << length << "px;'>&nbsp;</span>";
		m_humdrum_text << "&nbsp;" << it->second << "&nbsp;(" << percent << "%)</td></tr>" << endl;
	}
	m_humdrum_text << "!! </table>" << endl;
}






//////////////////////////////
//
// Tool_textdur::printMelismaHtmlHistogram --
//    default value = -1 (meaning all voices);
//

void Tool_textdur::printMelismaHtmlHistogram(void) {

	map<int, int> melinfo;
	double total = 0;
	for (int i=0; i<(int)m_melismas.size(); i++) {
		// -1 on next line for *- terminator?
		for (int j=0; j<(int)m_melismas[i].size() -1 ; j++) {
			int count = m_melismas[i][j];
			int value = melinfo[count];
			melinfo[count] = value + 1;
			total += 1.0;
		}
	}

	double maxlen = 0;
	for (auto it = melinfo.begin(); it != melinfo.end(); it++) {
		if (it->second > maxlen) {
			maxlen = it->second;
		}
	}

	m_humdrum_text << "!! <table class='melisma-histogram'>" << endl;
	m_humdrum_text << "!! <tr> <th style='text-align:center;'> Syllable&nbsp;notes </th> <th style='padding-left:10px;'> Syllable count </th> </tr> " << endl;
	for (auto it = melinfo.begin(); it != melinfo.end(); it++) {
		double length = 1.0 * it->second / maxlen * 400;
		double percent = 1.0 * it->second / total * 100.0;
		percent = (int)(percent * 100.0 + 0.5) / 100.0;
		m_humdrum_text << "!! <tr><td style='text-align:center;'> " << it->first << "</td><td style='padding-left:10px;white-space:pre;'>";
		m_humdrum_text << " <span style='display:inline-block;background-color:black;height:100%;width:" << length << "px;'>&nbsp;</span>";
		m_humdrum_text << "&nbsp;" << it->second << "&nbsp;(" << percent << "%)</td></tr>" << endl;
	}
	m_humdrum_text << "!! </table>" << endl;
}


// Print individual histograms for each voice:

void printMelismaHtmlHistogram(int index, int maxVal) {

/*
	map<int, int> melinfo;
	double total = maxVal;
	double maxlen = maxVal;

// ggg

	m_humdrum_text << "!! <table class='melisma-histogram-" << index << "'>" << endl;
	m_humdrum_text << "!! <tr> <th style='text-align:center;'> Syllable&nbsp;notes </th> <th style='padding-left:10px;'> Syllable count </th> </tr> " << endl;
	for (auto it = melinfo.begin(); it != melinfo.end(); it++) {
		double length = 1.0 * it->second / maxlen * 400;
		double percent = 1.0 * it->second / total * 100.0;
		percent = (int)(percent * 100.0 + 0.5) / 100.0;
		m_humdrum_text << "!! <tr><td style='text-align:center;'> " << it->first << "</td><td style='padding-left:10px;white-space:pre;'>";
		m_humdrum_text << " <span style='display:inline-block;background-color:black;height:100%;width:" << length << "px;'>&nbsp;</span>";
		m_humdrum_text << "&nbsp;" << it->second << "&nbsp;(" << percent << "%)</td></tr>" << endl;
	}
	m_humdrum_text << "!! </table>" << endl;
}

*/

}



//////////////////////////////
//
// printMelismaAverage --
//

void Tool_textdur::printMelismaAverage() {
	double sum = 0.0;
	int counter = 0;
	for (int i=0; i<(int)m_melismas.size(); i++) {
		// -1 on next line for *- terminator?
		for (int j=0; j<(int)m_melismas[i].size() - 1; j++) {
			sum += m_melismas.at(i).at(j);
			counter++;
		}
	}
	if (!counter) {
		return;
	}
	double average = sum / counter;
	average = int(average * 100.0 + 0.5) / 100.0;
	m_humdrum_text << "!!!TOOL-textdur-average-notes-per-syllable: " << average << endl;
	m_humdrum_text << "!!!TOOL-textdur-total-syllables: " << counter << endl;
}



//////////////////////////////
//
// printDurationAverage --
//

void Tool_textdur::printDurationAverage(void) {
	HumNum sum = 0;
	int counter = 0;
	for (int i=0; i<(int)m_durations.size(); i++) {
		// -1 on next line for *- terminator?
		for (int j=0; j<(int)m_durations[i].size() - 1; j++) {
			sum += m_durations.at(i).at(j);
			counter++;
		}
	}
	if (!counter) {
		return;
	}
	double average = sum.getFloat() / counter;
	average = int(average * 100.0 + 0.5) / 100.0;
	m_humdrum_text << "!!!TOOL-textdur-average-syllable-duration: " << average << endl;
}


//////////////////////////////
//
// Tool_textdur::printInterleaved --
//

void Tool_textdur::printInterleaved(HumdrumFile& infile) {
	vector<bool> textTrack(infile.getMaxTrack() + 1, false);
	for (int i=0; i<(int)m_textStarts.size(); i++) {
		int track = m_textStarts[i]->getTrack();
		textTrack.at(track) = true;
	}


	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].hasSpines()) {
			m_humdrum_text << infile[i] << endl;
			continue;
		}
		printInterleavedLine(infile[i], textTrack);
	}
}



//////////////////////////////
//
// Tool_textdur::printInterleavedLine --
//

void Tool_textdur::printInterleavedLine(HumdrumLine& line, vector<bool>& textTrack) {
	for (int i=0; i<line.getFieldCount(); i++) {
		HTp token = line.token(i);
		int track = token->getTrack();
		m_humdrum_text << token;
		if (textTrack.at(track)) {
			printTokenAnalysis(token);
		}
		if (i < line.getFieldCount() - 1) {
			m_humdrum_text << "\t";
		}
	}
	m_humdrum_text << "\n";
}



//////////////////////////////
//
// Tool_textdur::printTokenAnalysis --
//

void Tool_textdur::printTokenAnalysis(HTp token) {
	if (token->compare(0, 2, "**") == 0) {
		if (m_melismaQ) {
			m_humdrum_text << "\t**text-melisma";
		}
		if (m_durationQ) {
			m_humdrum_text << "\t**text-duration";
		}
		return;
	}

	if (*token == "*-") {
		if (m_melismaQ) {
			m_humdrum_text << "\t*-";
		}
		if (m_durationQ) {
			m_humdrum_text << "\t*-";
		}
		return;
	}

	if (token->compare(0, 1, "*") == 0) {

		HTp tandem = getTandemKernToken(token);
		if (tandem->compare(0, 3, "*I\"") == 0) {
			if (m_melismaQ) {
				m_humdrum_text << "\t*v:mel:";
			}
			if (m_durationQ) {
				m_humdrum_text << "\t*v:dur:";
			}
			return;
		}

		if (m_melismaQ) {
			m_humdrum_text << "\t*";
		}
		if (m_durationQ) {
			m_humdrum_text << "\t*";
		}
		return;
	}

	if (token->compare(0, 1, "!") == 0) {
		if (m_melismaQ) {
			m_humdrum_text << "\t!";
		}
		if (m_durationQ) {
			m_humdrum_text << "\t!";
		}
		return;
	}

	if (token->compare(0, 1, "=") == 0) {
		if (m_melismaQ) {
			m_humdrum_text << "\t" << token;
		}
		if (m_durationQ) {
			m_humdrum_text << "\t" << token;
		}
		return;
	}

	if (*token == ".") {
		if (m_melismaQ) {
			m_humdrum_text << "\t.";
		}
		if (m_durationQ) {
			m_humdrum_text << "\t.";
		}
		return;
	}

	if (!token->isData()) {
		cerr << "WARNING: DATA TOKEN IS NOT DATA: " << token << endl;
		return;
	}

	int index = -1;
	if (token->isDefined("auto", "index")) {
		index = token->getValueInt("auto", "index");
	} else {
		if (m_melismaQ) {
			m_humdrum_text << "\t.";
		}
		if (m_durationQ) {
			m_humdrum_text << "\t.";
		}
		return;
	}

	// Have data analysis to print:

	int track = token->getTrack();
	int column = m_track2column.at(track);

	if (m_melismaQ) {
		int melisma = m_melismas.at(column).at(index);
		m_humdrum_text << "\t" << melisma;
	}
	if (m_durationQ) {
		HumNum duration = m_durations.at(column).at(index);
		m_humdrum_text << "\t" << duration;
	}
}



//////////////////////////////
//
// Tool_textdur::printMelismas --
//

void Tool_textdur::printMelismas(HumdrumFile& infile) {
	// replace text with melisma data:
	for (int i=0; i<(int)m_syllables.size(); i++) {
		// -1 on next line for *- placeholder at end of spine.
		for (int j=0; j<(int)m_syllables.at(i).size() - 1; j++) {
			HTp token = m_syllables.at(i).at(j);
			int jj = token->getValueInt("auto", "index");
			string replacement = to_string(m_melismas.at(i).at(jj));
			token->setText(replacement);
		}
	}

	infile.createLinesFromTokens();
	m_humdrum_text << infile;
}



//////////////////////////////
//
// Tool_textdur::printDurations --
//

void Tool_textdur::printDurations(HumdrumFile& infile) {
	// replace text with duration data:
	stringstream replacement;
	for (int i=0; i<(int)m_syllables.size(); i++) {
		// -1 on next line for *- placeholder at end of spine.
		for (int j=0; j<(int)m_syllables.at(i).size() - 1; j++) {
			HTp token = m_syllables.at(i).at(j);
			int jj = token->getValueInt("auto", "index");
			replacement << m_durations.at(i).at(jj);
			token->setText(replacement.str());
			replacement.str("");
		}
	}

	infile.createLinesFromTokens();
	m_humdrum_text << infile;
}



//////////////////////////////
//
// Tool_textdur::getTextSpineStarts --
//

void Tool_textdur::getTextSpineStarts(HumdrumFile& infile, vector<HTp>& starts) {
	starts.clear();
	vector<HTp> allSpineStarts;
	infile.getSpineStartList(allSpineStarts);
	for (int i=0; i<(int)allSpineStarts.size(); i++) {
		HTp token = allSpineStarts.at(i);
		if (*token == "**text") {
			starts.push_back(token);
			token->setValue("auto", "index", i);
		} else if (*token == "**sylba") {
			starts.push_back(token);
			token->setValue("auto", "index", i);
		}
	}

	// Setup m_track2column
	m_track2column.resize(infile.getMaxTrack() + 1);
	fill(m_track2column.begin(), m_track2column.end(), -1);
	for (int i=0; i<(int)starts.size(); i++) {
		int track = starts[i]->getTrack();
		m_track2column.at(track) = i;
	}
}



//////////////////////////////
//
// Tool_textdur::processTextSpine --
//

void Tool_textdur::processTextSpine(vector<HTp>& starts, int index) {
	HTp current = starts.at(index);
	current->getNextToken();
	while (current) {
		if (!current->isData()) {
			if (*current == "*-") {
				// store data terminator (for calculating duration of last note):
				current->setValue("auto", "index", to_string(m_syllables.at(index).size()));
				m_syllables.at(index).push_back(current);
				m_durations.at(index).push_back(-1000);
				m_melismas.at(index).push_back(-1000);
				break;
			}
			current = current->getNextToken();
			continue;
		}

		if (current->isNull()) {
			current = current->getNextToken();
			continue;
		}

		current->setValue("auto", "index", to_string(m_syllables.at(index).size()));
		m_syllables.at(index).push_back(current);
		m_durations.at(index).push_back(-1);        // store dummy duration
		m_melismas.at(index).push_back(-1);         // store dummy melisma


		current = current->getNextToken();
	}

	for (int i=0; i<(int)m_syllables.size(); i++) {
		for (int j=0; j<(int)m_syllables[i].size() - 1; j++) {
			if (m_melismaQ) {
				m_melismas.at(i).at(j) = getMelisma(m_syllables.at(i).at(j), m_syllables.at(i).at(j+1));
			}
			if (m_durationQ) {
				m_durations.at(i).at(j) = getDuration(m_syllables.at(i).at(j), m_syllables.at(i).at(j+1));
			}
		}
	}
}



//////////////////////////////
//
// Tool_textdur::getMelisma --  Not counting syllable starts on secondary tied notes.
//

int Tool_textdur::getMelisma(HTp tok1, HTp tok2) {
	int stopIndex = tok2->getLineIndex();
	HTp current = getTandemKernToken(tok1);
	if (!current) {
		return 0;
	}
	if (current->isNull()) {
		cerr << "Strange case for syllable " << tok1 << " on line " << tok1->getLineNumber();
		cerr << ", field " << tok1->getFieldNumber() <<" which does not start on a note" << endl;
		return 0;
	}
	int cline = current->getLineIndex();
	int counter = 0;
	while (current && cline < stopIndex) {
		if (!current->isData()) {
			current = current->getNextToken();
			if (current) {
				cline = current->getLineIndex();
			}
			continue;
		}
		if (current->isNull()) {
			current = current->getNextToken();
			if (current) {
				cline = current->getLineIndex();
			}
			continue;
		}
		if (current->isNoteAttack()) {
			counter++;
		}
		current = current->getNextToken();
		if (current) {
			cline = current->getLineIndex();
		}
	}

	return counter;
}



//////////////////////////////
//
// Tool_textdur::getTandemKernToken --  Search to the left for the
//    first **kern spine.  Returns NULL if none found.
//

HTp Tool_textdur::getTandemKernToken(HTp token) {
	HTp current = token->getPreviousFieldToken();
	while (current && !current->isKern()) {
		current = current->getPreviousFieldToken();
	}
	return current;
}



//////////////////////////////
//
// Tool_textdur::getDuration --  Not counting rests at end of first syllable.
//

HumNum Tool_textdur::getDuration(HTp tok1, HTp tok2) {
	int startIndex = tok1->getLineIndex();
	HTp current = getTandemKernToken(tok2);
	if (!current) {
		return 0;
	}
	if (current->isNull()) {
		cerr << "Strange case for syllable " << tok1 << " on line " << tok1->getLineNumber();
		cerr << ", field " << tok1->getFieldNumber() <<" which does not start on a note" << endl;
		return 0;
	}
	HTp lastNoteEnd = current;
	current = current->getPreviousToken();
	int cline = current->getLineIndex();
	while (current && cline > startIndex) {
		if (!current->isData()) {
			current = current->getPreviousToken();
			continue;
		}
		if (current->isNull()) {
			current = current->getPreviousToken();
			continue;
		}
		cline = current->getLineIndex();
		if (!current->isRest()) {
			break;
		} else {
			lastNoteEnd = current;
		}
		current = current->getPreviousToken();
	}

	if (!lastNoteEnd) {
		return 0;
	}

	HumNum duration = lastNoteEnd->getDurationFromStart() - tok1->getDurationFromStart();

	return duration;
}



// END_MERGE

} // end namespace hum



