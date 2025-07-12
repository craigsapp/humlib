//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon May  5 06:31:29 PDT 2025
// Last Modified: Wed Jul  9 08:20:06 CEST 2025
// Filename:      GotScore.cpp
// URL:           http://github.com/craigsapp/humlib/blob/master/src/GotScore.cpp
// Syntax:        C++17; humlib
// vim:           syntax=cpp ts=3 noexpandtab nowrap
// $Smake:        g++ -std=c++17 -Wall -Wextra -o mygot %b.cpp
//
// Description:   Convert from spreadsheet (TSV) GOT data to **gotr/**gotp
//                or **kern data.
//

#include "GotScore.h"

#include <algorithm>
#include <cctype>
#include <regex>
#include <set>


using namespace std;

namespace hum {

// START_MERGE


//////////////////////////////
//
// GotScore::Measure::print -- Print contents of Measure object.  This
//   is useful for debugging, and also this function is used to print
//   the measure contents when there is a parsing error when converting
//   to **kern data.
//

ostream& GotScore::Measure::print(ostream& output) {
	output << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
	output << "!!!BAR:\t" << m_barnum << endl;
	if (!m_error.empty()) {
		for (int i=0; i<m_error.size(); ++i) {
			output << "!!!ERROR: " << m_error[i] << endl;
		}
	}
	if (!m_text.empty()) {
		output << "!!!TEXT:\t" << m_text << endl;
	}

	for (int v=0; v<(int)m_rhythms.size(); v++) {
		output << "!!!rhythms-voice-" << v+1 << ":";
		for (int b=0; b<(int)m_rhythms.at(v).size(); b++) {
			output << "\t";
			output << m_rhythms.at(v).at(b);
		}
		output << endl;
	}

	for (int v=0; v<(int)m_pitches.size(); v++) {
		output << "!!!pitches-voice-" << v+1 << ":";
		for (int b=0; b<(int)m_pitches.at(v).size(); b++) {
			output << "\t";
			output << m_pitches.at(v).at(b);
		}
		output << endl;
	}

	for (int v=0; v<(int)m_splitRhythms.size(); v++) {
		output << "!!!kern-rhythms-voice-" << v+1 << ":";
		for (int b=0; b<(int)m_splitRhythms.at(v).size(); b++) {
			output << "\t";
			for (int p=0; p<(int)m_splitRhythms.at(v).at(b).size(); p++) {
				output << " " << m_splitRhythms.at(v).at(b).at(p);
			}
		}
		output << endl;
	}

	for (int v=0; v<(int)m_splitPitches.size(); v++) {
		output << "!!!kern-pitches-voice-" << v+1 << ":";
		for (int b=0; b<(int)m_splitPitches.at(v).size(); b++) {
			output << "\t";
			for (int p=0; p<(int)m_splitPitches.at(v).at(b).size(); p++) {
				if (p != 0) {
					output << " ";
				}
				output << m_splitPitches.at(v).at(b).at(p);
			}
		}
		output << endl;
	}

	for (int i=0; i<m_error.size(); ++i) {
		output << "!!LO:TX:t=P:problem=";
		for (int j=0; j<(int)m_error[i].size(); ++j) {
			if (m_error[i][j] == ':') {
				output << "&colon;";
			} else {
				output << m_error[i][j];
			}
		}
		output << endl;
	}
	// Print out dummy rests to make empty measure visible.
	for (int i=0; i<(int)m_pitches.size(); i++) {
		if (i > 0) {
			output << "\t";
		}
		output << "4ryy";
	}
	if (m_owner && m_owner->m_textQ) {
		output << "\t.";
	}
	output << endl;

	return output;
}



//////////////////////////////
//
// GotScore::GotScore -- Constructor: reads all lines from the
//     input stream or string, trims trailing whitespace, and
//     prepares internal TSV‐cell structures.
//

GotScore::GotScore(void) {
	// do nothing
}

GotScore::GotScore(stringstream& ss) {
	loadLines(ss);
}


GotScore::GotScore(const string& s) {
	stringstream ss(s);
	loadLines(ss);
}



//////////////////////////////
//
// GotScore::~GotScore -- Destructor: no special cleanup required.
//

GotScore::~GotScore() {
	// do nothing
}



//////////////////////////////
//
// GotScore::clear -- Resets all internal vectors and cached output
//    strings to empty (used when starting a new conversion).
//

void GotScore::clear(void) {
	m_lines.clear();
	m_cells.clear();
	m_measures.clear();
	m_got.clear();
	m_kern.clear();
	m_error.clear();
}



//////////////////////////////
//
// GotScore::printInputFile -- Prints the input data from m_lines
//    (with trailing whitespace removed).
//

ostream& GotScore::printInputFile(ostream& out) {
	for (int i=0; i<(int)m_lines.size(); i++) {
		out << (i+1) << ":\t" << m_lines[i] << endl;
	}
	return out;
}



//////////////////////////////
//
// GotScore::printCells -- Print cells extracted from lines (for debugging
//      purposes to verify input TSV data).
//

ostream& GotScore::printCells(ostream& out) {
	for (int i=0; i<(int)m_cells.size(); i++) {
		for (int j=0; j<(int)m_cells[i].size(); j++) {
			out << i << "," << j << ":\t" << m_cells[i][j] << endl;
		}
	}
	return out;
}



//////////////////////////////
//
// GotScore::printMeasures -- Print contenst of Measure objects (for debugging).
//

ostream& GotScore::printMeasures(ostream& out) {
	for (int i=0; i<(int)m_measures.size(); i++) {
		m_measures[i].print(out);
	}
	return out;
}


//////////////////////////////
//
// GotScore::loadLines -- Input data is split vector of strings
//    and then 2D vector of TSV cells.
//

void GotScore::loadLines(const string& s) {
	stringstream ss(s);
	loadLines(ss);
}


void GotScore::loadLines(stringstream& ss) {
	clear();
	string line;
	while (getline(ss, line)) {
		// Remove trailing spaces and tabs
		size_t end = line.find_last_not_of(" \n\r\t");
		if (end != string::npos) {
			line.erase(end + 1);
		} else {
			line.clear();
		}
		m_lines.push_back(line);
	}

	prepareCells();
}



//////////////////////////////
//
// GotScore::prepareCells -- Convert from a vector of lines
//   into a 2D vector of TSV values.
//

void GotScore::prepareCells() {
	m_cells.clear();
	m_cells.resize(m_lines.size());
	for (int i=0; i<(int)m_lines.size(); ++i) {
		const string& line = m_lines[i];
		vector<string> fields;
		size_t start = 0;
		size_t end;
		while ((end = line.find('\t', start)) != string::npos) {
			fields.emplace_back(line.substr(start, end - start));
			start = end + 1;
		}
		fields.emplace_back(line.substr(start));
		m_cells[i] = std::move(fields);
	}
	prepareMeasures(cerr);
}



//////////////////////////////
//
// GotScore::prepareMeasures -- Store m_cells data into m_measures.  Iterate
//     through all rows in the m_cells matrix, processing system data line groups
//     as they are found.  Systems start with "s# bar" in the first column
//     of m_cells.
//

bool GotScore::prepareMeasures(ostream& out) {
	regex re("^s(\\d+) bar");
	regex rebaronly("^\\s*bar\\s*$");
	std::smatch match;
	bool status = true;

	// Search for lines starting with "s# bar" which incidates system #.
	for (int i=0; i<(int)m_cells.size(); i++) {
		if (regex_search(m_cells[i][0], rebaronly)) {
			// Measure number lines should start with "s# bar" not "bar".
			out << "Error on line: " << (i+1) << ": " << " No system info for bar line." << endl;
			return false;
		}
		if (!regex_search(m_cells[i][0], match, re)) {
			// Skip lines that do not start with "s# bar"
			continue;
		}
		int system = stoi(match[1]);
		if (m_debugQ) {
			out << ">>>>>>>>>>>>>>>>>>> PROCESSING SYSTEM = " << system << endl;
		}
		status = processSystemMeasures(i, system, out);
		if (!status) {
			out << "Problems parsing system " << system << endl;
			return status;
		}
	}
	return status;
}



//////////////////////////////
//
// GotScore::processSystemMeasures --  Helper for prepareMeasures:
//   for each system, locate the pitch/rhythm for each voice and any
//   optional text lines and store in a new Measure object.
//
//   barIndex = index of the measure number line for the system.
//   system   = system number (should start at 1).
//   out      = where the conversion contents will be sent.
//
// Example data (tab separated cells), first two measures ownly
//    s1 bar       0                1
//    s1 v1 r      *met(C|) 1       22    1
//    s1 v1 p      *        g/      b/c// d//
//    s1 v2 r      *met(C|) 1       1     1
//    s1 v2 p      *        r       r     b/
//    s1 v3 r      *met(C|) 1       1     1
//    s1 v3 p      *        r       r     G
//    s1 text      E-               -go flos
//

bool GotScore::processSystemMeasures(int barIndex, int system, ostream& out) {
	vector<int> rIndex;
	vector<int> pIndex;
	int textIndex = -1;

	regex re("^s(\\d+)"); // system number
	regex rev("v(\\d+)"); // voice number
	regex rer("r\\s*$");  // rhythm information
	regex rep("p\\s*$");  // pitch information
	std::smatch match;

	// error checking
	regex badpitch("\\d"); // pitch cell cannot have a digit
	regex badrhythm("[a-gA-GrR#-]"); // rhythm cell cannot have a pitch letter

	// Identify the lines for each type of data for one system:
	// * line index for the bar numbers
	// * line index for each pair of rhythm/pitch
	// * line index for the text (optional).
	for (int i = barIndex + 1; i<(int)m_cells.size(); i++) {
		if (!regex_search(m_cells[i][0], match, re)) {
			continue;
		}
		int tsys = stoi(match[1]);
		if (system != tsys) {
			break;
		}
		if (m_cells[i][0].find("text") != string::npos) {
			textIndex = i;
			m_textQ = true;
			continue;
		}
		if (!regex_search(m_cells[i][0], match, rev)) {
			continue;
		}
		int voiceNumber = stoi(match[1]);
		if (voiceNumber > m_voices) {
			m_voices = voiceNumber;
		}
		int voiceIndex = voiceNumber - 1;
		if (regex_search(m_cells[i][0], rer)) {
			if ((int)rIndex.size() != voiceIndex) {
				out << "INVALID VOICE NUMBER FOR RHYTHMS ON ROW " << (i+1) << endl;
				return false;
			}
			rIndex.push_back(i);
		} else if (regex_search(m_cells[i][0], rep)) {
			if ((int)pIndex.size() != voiceIndex) {
				out << "INVALID VOICE NUMBER FOR PITCHES ON ROW " << (i+1) << endl;
				return false;
			}
			pIndex.push_back(i);
		}
	}

	if (m_debugQ) {
		cerr << "SYSTEM " << system << endl;
		if (rIndex.size() != pIndex.size()) {
			out << "\tRhythms and Pitches are not equal" << endl;
			return false;
		}
		cerr << "\tBARINDEX: " << barIndex << endl;
		if (textIndex >= 0) {
			cerr << "\tTEXTINDEX: " << textIndex << endl;
		}
		for (int j=0; j<(int)rIndex.size(); j++) {
			cerr << "\tVOICE " << j+1 << " RINDEX: " << rIndex[j] << " PINDEX: " << pIndex[j] << endl;
		}
	}

	// Now process the bar line and prepare measures for any bar
	// line cell that contains a number.

	regex renum("(\\d+)");

	for (int i=1; i<(int)m_cells[barIndex].size(); i++) {
		if (!regex_search(m_cells.at(barIndex).at(i), match, renum)) {
			continue;
		}
		int barnum = stoi(match[1]);

		if (m_debugQ) {
			cerr << "PROCESSING MEASURE " << barnum << endl;
		}

		// Add a new measure to the end of the list:
		m_measures.resize(m_measures.size()+1);
		GotScore::Measure& lm = m_measures.back();

		// Store the GotScore owner of the measure (for keeping
		// track of text existence in measures:
		lm.m_owner = this;

		// Store the bar number for the measure:
		lm.m_barnum = m_cells[barIndex].at(i);

		// Store the text for the measure:
		if (textIndex >= 0) {
			if (i < (int)m_cells[textIndex].size()) {
				lm.m_text = m_cells[textIndex].at(i);
			} else {
				lm.m_text = "";
			}
		}

		lm.m_rhythms.resize(rIndex.size());
		lm.m_pitches.resize(rIndex.size());
		for (int j=0; j<(int)rIndex.size(); ++j) {
			string& rhythm_cell = m_cells.at(rIndex.at(j)).at(i);
			lm.m_rhythms.at(j) = splitBySpaces(rhythm_cell);
			for (int k=0; k<(int)lm.m_rhythms.at(j).size(); ++k) {
				string& value = lm.m_rhythms.at(j).at(k);
				if (!value.empty()) {
					if (value.at(0) == '*') {
						continue;
					}
				}
				if (regex_search(value, badrhythm)) {
					stringstream ss;
					ss << "Measure " << lm.m_barnum << ", voice ";
					ss << j+1 << ": detected pitch characters in rhythm cell: " << value;
					lm.m_error.push_back(ss.str());
					break;
				}
			}
		}
		for (int j=0; j<(int)pIndex.size(); ++j) {
			string& pitch_cell = m_cells.at(pIndex.at(j)).at(i);
			lm.m_pitches.at(j) = splitBySpaces(pitch_cell);
			for (int k=0; k<(int)lm.m_pitches.at(j).size(); ++k) {
				string& value = lm.m_pitches.at(j).at(k);
				if (!value.empty()) {
					if (value.at(0) == '*') {
						continue;
					}
				}
				if (regex_search(value, badpitch)) {
					stringstream ss;
					ss << "Measure " << lm.m_barnum << ", voice ";
					ss << j+1 << ": detected rhythm characters in pitch cell: " << value;
					lm.m_error.push_back(ss.str());
					break;
				}
			}
		}
	}

	return true;
}



//////////////////////////////
//
// GotScore::cleanRhythmValues -- Convert "6" rhythms into "16" (sixteenths)
//

void GotScore::cleanRhythmValues(vector<vector<string>>& rhythms) {
	for (int i=0; i<(int)rhythms.size(); i++) {
		for (int j=0; j<(int)rhythms[i].size(); j++) {
			size_t loc = rhythms.at(i).at(j).find("6");
			if (loc != string::npos) {
				rhythms.at(i).at(j).replace(loc, 1, "16");
			}
		}
	}
}



//////////////////////////////
//
// GotScore::splitBySpaces -- Split a single TSV cell (string) into
//     an array by spaces.  Pitches and Rhytyms are groups into beats
//     by spaces (either 2 or 3 groupings per measure).
//


vector<string> GotScore::splitBySpaces(const string& input) {
	// 1) split on whitespace
	vector<string> parts;
	{
		stringstream stream(input);
		string word;
		while (stream >> word) {
			parts.push_back(word);
		}
	}

	// 2) merge any "." + digit-leading token into single ".<whatever>"
	vector<string> result;
	result.reserve(parts.size());
	for (size_t i = 0; i < parts.size(); ++i) {
		if (parts[i] == "." &&
			i + 1 < parts.size() &&
			!parts[i+1].empty() &&
			isdigit(static_cast<unsigned char>(parts[i+1][0])))
		{
			// merge "." and the next token
			result.push_back(parts[i] + parts[i+1]);
			++i;  // skip the next one
		} else {
			result.push_back(parts[i]);
		}
	}

	return result;
}




//////////////////////////////
//
// GotScore::getGotHumdrum -- Print input data by measure into
//   **gotr/**gotp (GOT rhythm/GOT pitch) columns, which is an
//   intermediate step before splitting into individual notes
//   on the way to **kern data.
//

string GotScore::getGotHumdrum(void) {
	if (!m_got.empty()) {
		return m_got;
	}

	stringstream out;

	string header = getHeaderInfo(0);
	out << header;

	// Print exclusive interpretation line:
	for (int i=0; i<m_voices; i++) {
		if (i>0) {
			out << "\t";
		}
		out << "**gotr\t**gotp";
	}
	if (m_textQ) {
		out << "\t**cdata";
	}
	out << "\n";

	// Print staff/part info:
	for (int i=0; i<m_voices; i++) {
		if (i>0) {
			out << "\t";
		}
		out << "*staff" << (i+1);
		out << "\t*staff" << (i+1);
	}
	if (m_textQ) {
		out << "\t*";
	}
	out << "\n";

	for (int i=0; i<(int)m_measures.size(); i++) {
		out << getGotHumdrumMeasure(m_measures.at(i));
	}

	// Print final barline:
	for (int i=0; i<m_voices; i++) {
		if (i>0) {
			out << "\t";
		}
		out << "==\t==";
	}
	if (m_textQ) {
		out << "\t==";
	}
	out << "\n";

	// Print termination interpretation:
	for (int i=0; i<m_voices; i++) {
		if (i>0) {
			out << "\t";
		}
		out << "*-\t*-";
	}
	if (m_textQ) {
		out << "\t*-";
	}
	out << "\n";

	string footer = getFooterInfo();
	out << footer;

	m_got = out.str();
	return m_got;
}



//////////////////////////////
//
// GotScore::getHeaderInfo -- Extract bibliographic information from the start of the data.
//

string GotScore::getHeaderInfo(int index) {
	stringstream out;
	for (int i=index; i<(int)m_cells.size(); ++i) {
		if (m_cells[i].empty()) {
			continue;
		}
		if (m_cells[i][0].compare(0, 2, "!!") == 0) {
			if (m_cells[i][0].compare(0, 3, "!!!") == 0) {
				if (m_cells[i][0].back() != ':') {
					out << m_cells[i][0] << ":";
				} else {
					out << m_cells[i][0];
				}
				if (m_cells[i].size() > 1) {
					out << " " << m_cells[i][1];
				}
				for (int j=2; j<(int)m_cells[i].size(); ++j) {
					out << "\t" << m_cells[i][j];
				}
				out << endl;
			} else {
				for (int j=0; j<(int)m_cells[i].size(); ++j) {
					if (j > 0) {
						out << "\t";
					}
					out << m_cells[i][j];
				}
				out << endl;
			}
		} else {
			break;
		}
	}
	return out.str();
}



//////////////////////////////
//
// GotScore::getFooterInfo -- Extract bibliographic information from the end of the data.
//

string GotScore::getFooterInfo(void) {
	stringstream out;
	out << "!!!RDF**kern: l = terminal long" << endl;

	if (m_cautionary) {
		out << "!!!RDF**kern: i = editorial accidental, paren" << endl;
	}

	int startIndex = -1;
	for (int i=(int)m_cells.size() - 1; i>=0; --i) {
		if (m_cells[i].empty()) {
			continue;
		} else if (m_cells[i][0].compare(0, 2, "!!") == 0) {
			startIndex = i;
			continue;
		} else if (m_cells[i][0] == "") {
			continue;
		} else {
			break;
		}
	}
	if (startIndex < 0) {
		return out.str();
	}
	out << getHeaderInfo(startIndex);
	return out.str();
}



//////////////////////////////
//
// GotScore::getGotHumdrumMeasure -- Print GOT system measure,
//   one **gotr/**gotp column for each voice from top to bottom
//   of system, also **cdata if lyrics are present.
//

string GotScore::getGotHumdrumMeasure(GotScore::Measure& mdata) {
	stringstream out;
	int mincount = -1;
	int maxcount = -1;

	for (int i=0; i<(int)mdata.m_rhythms.size(); i++) {
		if (mincount < 0) {
			mincount = (int)mdata.m_rhythms.at(i).size();
		}
		if (maxcount < 0) {
			maxcount = (int)mdata.m_rhythms.at(i).size();
		}
		if (mincount > (int)mdata.m_rhythms.at(i).size()) {
			mincount = (int)mdata.m_rhythms.at(i).size();
		}
		if (maxcount < (int)mdata.m_rhythms.at(i).size()) {
			maxcount = (int)mdata.m_rhythms.at(i).size();
		}
	}

	for (int i=0; i<(int)mdata.m_pitches.size(); i++) {
		if (mincount < 0) {
			mincount = (int)mdata.m_pitches.at(i).size();
		}
		if (maxcount < 0) {
			maxcount = (int)mdata.m_pitches.at(i).size();
		}
		if (mincount > (int)mdata.m_pitches.at(i).size()) {
			mincount = (int)mdata.m_pitches.at(i).size();
		}
		if (maxcount < (int)mdata.m_pitches.at(i).size()) {
			maxcount = (int)mdata.m_pitches.at(i).size();
		}
	}

	if (mincount != maxcount) {
		out << "!!Problem with measure " << mdata.m_barnum << endl;
		return "";
	}

	if (mdata.m_barnum != "0") {
		// Print starting barline, as long as it is not a pickup measure.
		for (int i=0; i<m_voices; i++) {
			if (i>0) {
				out << "\t";
			}
			out << "=" << mdata.m_barnum << "\t=" << mdata.m_barnum;
		}
		if (m_textQ) {
			out << "\t=" << mdata.m_barnum;
		}
		out << "\n";
	}

	for (int i=0; i<maxcount; i++) {
		for (int j=0; j<m_voices; j++) {
			if (j > 0) {
				out << "\t";
			}
			out << mdata.m_rhythms.at(j).at(i);
			out << "\t";
			out << mdata.m_pitches.at(j).at(i);
		}

		if (m_textQ) {
			out << "\t";
			if ((i>0) || mdata.m_text.empty()) {
				out << ".";
			} else {
				out << mdata.m_text;
			}
		}
		out << endl;
	}

	return out.str();
}



//////////////////////////////
//
// GotScore::getKernHumdrum -- Convert GOT score into **kern data
//     and **cdata for lyrics.  Splits GOT beat words into individual
//     note/rest tokens, applies ties, and concatenates measures
//     into a single string.
//


string GotScore::getKernHumdrum(void) {
	if (!m_kern.empty()) {
		return m_kern;   // return cached
	}

	// Tokenize each measure into individual rhythm & pitch tokens
	splitMeasureTokens();

	// Handle leading‐dot augmentation ties
	pairLeadingDots();

	// Handle half‐duration ties and pitch‐dot ties
	processDotTiedNotes();

	// Check for token alignment errors by measure, and then parse
	// underscore ties.  Underscore ties are "_" attached to the end of
	// a pitch that extends to the next note in the sequence.  "_" is converted
	// into "[" on the first note, and "]" is added to the next note.
	// Later when merging pitches and rhythms, if a pitch has both "[" and "]"
	// then convert them into "_" for a **kern tie-continuation symbol.
	vector<vector<string*>> pall;
	pall.resize(m_measures[0].m_splitRhythms.size());
	for (int i=0; i<(int)pall.size(); i++) {
		pall[i].reserve(m_measures.size() * 20);
	}

	// Apply underscore ties _ -> [ ... ] for each measure and voice
	for (auto& M : m_measures) {
		for (size_t v = 0; v < M.m_splitRhythms.size(); ++v) {
			// flatten all non‐interpretation rhythm tokens
			vector<string*> rr;
			for (auto& beat : M.m_splitRhythms[v]) {
				for (auto& tok : beat) {
					if (!tok.empty() && tok[0] != '*') {
						rr.push_back(&tok);
					}
				}
			}
			// flatten all non‐interpretation pitch tokens
			vector<string*> pp;
			for (auto& beat : M.m_splitPitches[v]) {
				for (auto& tok : beat) {
					if (!tok.empty() && tok[0] != '*') {
						pp.push_back(&tok);
                  pall.at(v).push_back(&tok);
					}
				}
			}

			// If rhythms.size() and pitches.size() are not equal then
			// generate an error for the measure.
			if (rr.size() != pp.size()) {
				string message = "Measure " + M.m_barnum;
				message += ", voice " + to_string(v+1);
				message += ": pitch and rhythm token counts are not the same.";
				M.m_error.push_back(message);
			}
		}
	}

	// Handle underscore ties.
	for (int i=0; i<(int)pall.size(); i++) {
		processUnderscoreTies(pall[i]);
	}

	// Add accidental analysis
	prepareAccidentals();

	// Build timed events (timestamps and durations)
	buildVoiceEvents();

	// Emit **kern interpretation & data
	stringstream out;

	string header = getHeaderInfo(0);
	out << header;

	// Exclusive interpretation line
	for (int i = 0; i < m_voices; ++i) {
		if (i > 0) out << "\t";
		out << "**kern";
	}
	if (m_textQ) {
		out << "\t**cdata";
	}
	out << "\n";

	// Staff/part info (reverse order for **kern)
	for (int i = 0; i < m_voices; ++i) {
		if (i > 0) out << "\t";
		out << "*staff" << (m_voices - i);
	}
	if (m_textQ) out << "\t*";
	out << "\n";

	// Calculate optimal clefs for each voice
	vector<string> clefs;
	clefs = generateVoiceClefs();
	for (int i=(int)clefs.size() - 1; i>= 0; --i) {
		if (i != (int)clefs.size() - 1) {
			out << "\t";
		}
		out << clefs[i];
	}
	if (m_textQ) {
		out << "\t*";
	}
	out << endl;

	// Data rows, by measure
	for (auto& M : m_measures) {
		out << getKernHumdrumMeasure(M);
	}

	// Final barline
	for (int i = 0; i < m_voices; ++i) {
		if (i > 0) out << "\t";
		out << "==";
	}
	if (m_textQ) out << "\t==";
	out << "\n";

	// Termination line
	for (int i = 0; i < m_voices; ++i) {
		if (i > 0) out << "\t";
		out << "*-";
	}
	if (m_textQ) out << "\t*-";
	out << "\n";

	string footer = getFooterInfo();
	out << footer;

	m_kern = out.str();
	return m_kern;
}



//////////////////////////////
//
// GotScore::generateVoiceClefs -- Calculate the clef that best fits the
//    pitch ranges of each voice.
//

vector<string> GotScore::generateVoiceClefs(void) {
	vector<double> psum(m_voices, 0);
	vector<double> pcount(m_voices, 0);
	vector<double> pmin(m_voices, 127);
	vector<double> pmax(m_voices, 0);
	for (int v=0; v<(int)m_pitch_hist.size(); ++v) {
		for (int i=0; i<(int)m_pitch_hist[v].size(); ++i) {
			double count = m_pitch_hist[v][i];
			if (count == 0) {
				continue;
			}
			psum[v] += i * count;
			pcount[v] += count;
			if (i < pmin[v]) {
				pmin[v] = i;
			}
			if (i > pmax[v]) {
				pmax[v] = i;
			}
		}
	}

	vector<double> pmean(m_voices, 0);
	for (int v=0; v<(int)pmean.size(); ++v) {
		pmean[v] = psum[v] / pcount[v];
	}
	vector<string> output(m_voices);;
	for (int v=0; v<(int)output.size(); ++v) {
		output[v] = chooseClef(pmean[v], pmin[v], pmax[v]);
	}

	return output;
}



//////////////////////////////
//
// GotScore::chooseClef -- Chose the clef for a voice based on the
//    mean pitch of the voice, and the min/max value.  Currently the
//    algorithm is to use tenor clef if the mean pitch is between A-flat3
//    and E4.
//

string GotScore::chooseClef(double mean, double min, double max) {
	if (mean > 64) {
		return "*clefG2";
	} else if (mean > 56) {
		return "*clefGv2";
	} else {
		return "*clefF4";
	}
}



//////////////////////////////
//
// GotScore::getKernHumdrumMeasure -- Formats one Measure
//     into kern columns (aligning by timestamp), and inserting
//     lyrics column if present.
//

string GotScore::getKernHumdrumMeasure(GotScore::Measure& mdata) {
	stringstream out;

	// Avoid barline at start of pickup measure.
	if (mdata.m_barnum != "0") {
		  mdata.printKernBarline(out, m_textQ);
	}

	// Print parsing error message and Measure contents
	// when there is a problem somewhere in the measure.
	if (!mdata.m_error.empty()) {
		mdata.print(out);
		return out.str();
	}

	auto aligned = alignEventsByTimestamp(mdata);
	bool textPrinted = false;

	for (const auto& e : aligned) {
		  // 1) print the **kern** columns
		  for (int i = (int)e.rhythms.size() - 1; i >= 0; --i) {
				if (i < (int)e.rhythms.size() - 1) out << "\t";
				const string& r = e.rhythms[i];
				const string& p = e.pitches[i];

				if (r.empty() || r == ".") {
					 out << ".";
				}
				else if (r[0] == '*') {
					 // interpretation token: only print r
					 out << r;
				}
				else {
					 // normal note
					 out << mergeRhythmAndPitchIntoNote(r, p);
				}
		  }

		  // 2) figure out if this row is an interpretation line
		  bool isInterpRow = false;
		  for (auto& r : e.rhythms) {
				if (!r.empty() && r[0] == '*') {
					 isInterpRow = true;
					 break;
				}
		  }

		  // 3) print the text spine
		  if (m_textQ) {
				out << "\t";
				if (isInterpRow) {
					 // always null-interpretation on met/measuresig rows
					 out << "*";
				}
				else if (!textPrinted && !mdata.m_text.empty()) {
					 // first real data row gets the lyric
					 out << mdata.m_text;
					 textPrinted = true;
				}
				else {
					 // all later rows null out
					 out << ".";
				}
		  }

		  out << "\n";
	}

	return out.str();
}



//////////////////////////////
//
// GotScore::mergeRhythmAndPitchIntoNote -- Take the two separate streams of
//    rhythms and pitches and merge into a single **kern token.
//

string GotScore::mergeRhythmAndPitchIntoNote(const string& r, const string& p) {
	string output;

	size_t loc = p.find('[');
	size_t loc2 = p.find(']');
	if (loc != string::npos) {
		if (loc2 == string::npos) {
			output += "[";
		}
	}

	bool hasFermata = false;
	for (int i=0; i<(int)r.size(); i++) {
		if (r[i] != '_') {
			if (r[i] == ';') {
				hasFermata = true;
			} else {
				output += r[i];
			}
		}
	}

	bool hasTieEnd = false;
	if (p.find(']') != string::npos) {
		hasTieEnd = true;
	}
	if (p.find('_') != string::npos) {
		hasTieEnd = true;
	}
	if (loc2 != string::npos) {
		hasTieEnd = true;
	}

	for (int i=0; i<(int)p.size(); i++) {
		if ((p[i] != '[') && p[i] != ']') {
			output += p[i];
			if (p[i] == '#' || p[i] == '-') {
				if (!hasTieEnd) {
					output += "X";
				}
			}
		}
	}

	if (loc != string::npos) {
		if (loc2 != string::npos) {
			output += "_";
		}
	} else {
		if (loc2 != string::npos) {
			output += "]";
		}
	}

	if (hasFermata) {
		output += ';';
	}

	return  output;
}



//////////////////////////////
//
// GotScore::prepareAccidentals -- Mark notes that become natural
//    within the measure with editorial accidentals (add the letter "i"
//    after the note, and add !!!RDF**kern: i = editorial accidental
//    in the footer later.
//

void GotScore::prepareAccidentals(void) {
	if (m_measures.empty()) {
		return;
	}
	if (m_measures.at(0).m_splitPitches.empty()) {
		// This function should only be run after filling in m_splitPitches
		// (as well as processing ties).
		return;
	}

	if (!m_no_editorialQ) {
		for (int i=0; i<(int)m_measures.size(); i++) {
			m_measures[i].m_diatonic.resize(m_measures[i].m_splitPitches.size());
			m_measures[i].m_accid.resize(m_measures[i].m_splitPitches.size());
			m_measures[i].m_accidState.resize(m_measures[i].m_splitPitches.size());
			m_measures[i].m_kerns.resize(m_measures[i].m_splitPitches.size());
			for (int v=0; v<(int)m_measures[i].m_splitPitches.size(); v++) {
				markEditorialAccidentals(m_measures[i], v);
			}
		}
	}

	if (m_cautionaryQ) {
		for (int i=1; i<(int)m_measures.size(); i++) {
			for (int v=0; v<(int)m_measures.at(0).m_splitPitches.size(); v++) {
				checkForCautionaryAccidentals(i, v);
			}
		}
	}
}



//////////////////////////////
//
// GotScore::checkForCautionaryAccidentals -- This is an optionally run function
//     that adds cautionary accidentals (for natural notes) based on the previous
//     measure.  Use the setCautionary() function to activate this function
//     which converting to **kern data.
//

void GotScore::checkForCautionaryAccidentals(int mindex, int vindex) {
	if (mindex < 1) {
		return;
	}
	vector<int> states = m_measures.at(mindex - 1).m_accidState.at(vindex);
	Measure& measure = m_measures.at(mindex);
	vector<string*>& kerns  = measure.m_kerns.at(vindex);
	vector<int>& diatonic = measure.m_diatonic.at(vindex);
	vector<int>& accid = measure.m_accid.at(vindex);

	for (int i=0; i<(int)kerns.size(); i++) {
		int dindex = diatonic.at(i);
		if (dindex < 0) {
			continue;
		}
		if (states.at(dindex) == 0) {
			continue;
		}
		if (states.at(dindex) > 100) {
			continue;
		}
		int acc = accid.at(i);
		if ((states.at(dindex) != 0) && (acc == 0)) {
			if (kerns.at(i)->find('i') == string::npos) {
				if (kerns.at(i)->find('n') == string::npos) {
					*kerns.at(i) += "nii";
				} else {
					*kerns.at(i) += "ii";
				}
			}
			m_cautionary = true;
		}
		states.at(dindex) = 1000;
	}
}



//////////////////////////////
//
// GotScore::markEditorialAccidentals -- When a natural pitch class follows an
//    chromatically altered one in the measure, mark it as an editorial natural
//    accidental.  This will add a parenthese around the note; otherwise, the
//    natural visual accidental will be added automatically for modern accidental
//    syntax.
//

void GotScore::markEditorialAccidentals(GotScore::Measure& measure, int voice) {
	vector<vector<string>>& pitches = measure.m_splitPitches.at(voice);
	vector<int>& diatonic = measure.m_diatonic.at(voice);
	vector<int>& accid = measure.m_accid.at(voice);
	diatonic.clear();
	accid.clear();
	vector<string*>& kerns = measure.m_kerns.at(voice);
	for (int i=0; i<(int)pitches.size(); i++) {
		for (int j=0; j<(int)pitches.at(i).size(); j++) {
			int d = -1;
			int a = 0;
			getDiatonicAccid(pitches.at(i).at(j), d, a);
			diatonic.push_back(d);
			accid.push_back(a);
			kerns.push_back(&pitches.at(i).at(j));
		}
	}

	// Now create the accidState for the voice, when the accid state changes
	// from non-zero to zero, add the letter i to the m_splitPitches value at the
	// same index to indicate a cautionary natural accidental.
	vector<int>& accidState = measure.m_accidState.at(voice);
	accidState.resize(7);
	std::fill(accidState.begin(), accidState.end(), 0);
	for (int i=0; i<(int)diatonic.size(); i++) {
		int dindex = diatonic[i];
		if (dindex < 0) {
			continue;
		}
		int acc = accid[i];
		if ((accidState.at(dindex) != 0) && (acc == 0)) {
			if (kerns.at(i)->find('i') == string::npos) {
				if (kerns.at(i)->find('n') == string::npos) {
					*kerns.at(i) += "ni";
				} else {
					*kerns.at(i) += "i";
				}
			}
			m_cautionary = true;
		}
		accidState.at(dindex) = acc;
	}
}



/////////////////////////////
//
// GotScore::getDiatonicAccid -- Return the diatonic pitch class and accidental
//       state of a **kern note.
//

void GotScore::getDiatonicAccid(const string& pitch, int& d, int& a) {
	d = -1;
	a = 0;
	if (pitch.find('r') != string::npos) {
		return;
	}
	for (int i=0; i<(int)pitch.size(); i++) {
		char c = std::tolower(pitch[i]);
		if (c >= 'a' && c <= 'g') {
			d = c - 'a';
		} else if (pitch[i] == '-') {
			a = -1;
		} else if (pitch[i] == '#') {
			a = +1;
		} else if (pitch[i] == 'n') {
			a = 0;
		}
	}
}



//////////////////////////////
//
// GotScore::splitMeasureTokens -- Iterates all measures and tokenizes
//     each rhythm+pitch word into individual **kern tokens.
//

void GotScore::splitMeasureTokens(void) {
	for (int i=0; i<(int)m_measures.size(); i++) {
		splitMeasureTokens(m_measures[i]);
	}

}


//
// Splits one measure’s rhythm/pitch strings into per-note tokens,
// converts GOT pitches to kern, and builds timed events for alignment
// across voices.
//

void GotScore::splitMeasureTokens(GotScore::Measure& mdata) {
	// --- rhythms --------------------------------------------------------
	mdata.m_splitRhythms.clear();
	mdata.m_splitRhythms.resize(mdata.m_rhythms.size());

	for (size_t v = 0; v < mdata.m_rhythms.size(); ++v) {
		mdata.m_splitRhythms[v].clear();
		mdata.m_splitRhythms[v].resize(mdata.m_rhythms[v].size());

		for (size_t w = 0; w < mdata.m_rhythms[v].size(); ++w) {
			string raw = mdata.m_rhythms[v][w];

			// handle leading dot
			bool hadDot = false;
			if (!raw.empty() && raw[0] == '.') {
				hadDot = true;
				raw.erase(0,1);
			}

			if (m_debugQ) {
				cerr << "RHYTHM RAW ["<<v<<"]["<<w<<"] : \""
						  << mdata.m_rhythms[v][w]
						  << "\" → tokenizer input \"" << raw << "\"\n";
			}

			// tokenize the stripped word
			vector<string> toks = tokenizeRhythmString(raw);

			// re-insert the dot if needed
			if (hadDot) toks.insert(toks.begin(), ".");

			if (m_debugQ) {
				cerr << "RHYTHM TOKENS["<<v<<"]["<<w<<"]:";
				for (auto& t : toks) {
					cerr << " \"" << t << "\"";
				}
				cerr << "\n";
			}

			mdata.m_splitRhythms[v][w] = std::move(toks);
		}

		// Expand "6" into "16" for rhythms:
		cleanRhythmValues(mdata.m_splitRhythms.at(v));
	}

	// --- pitches -------------------------------------------------------
	mdata.m_splitPitches.clear();
	mdata.m_splitPitches.resize(mdata.m_pitches.size());

	for (size_t v = 0; v < mdata.m_pitches.size(); ++v) {
		mdata.m_splitPitches[v].clear();
		mdata.m_splitPitches[v].resize(mdata.m_pitches[v].size());

		for (size_t w = 0; w < mdata.m_pitches[v].size(); ++w) {
			// 1) tokenize into GOT‐style pitch tokens
			vector<string> gotTokens = tokenizePitchString(mdata.m_pitches[v][w]);

			// 2) convert that lvalue into kern‐style pitches
			vector<string> kernTokens = convertGotToKernPitches(gotTokens);

			if (m_debugQ) {
				cerr << "PITCH TOKENS["<<v<<"]["<<w<<"]:";
				for (auto& t : kernTokens) cerr << " \"" << t << "\"";
				cerr << "\n";
			}

			mdata.m_splitPitches[v][w] = std::move(kernTokens);
		}
	}
}



//////////////////////////////
//
// GotScore::buildVoiceEvents --
//

void GotScore::buildVoiceEvents(void) {
	for (auto& mdata : m_measures) {
		if (!mdata.m_error.empty()) {
			continue;
		}
		mdata.m_voiceEvents.clear();
		mdata.m_voiceEvents.resize(mdata.m_splitRhythms.size());

		for (int v=0; v<(int)mdata.m_splitRhythms.size(); ++v) {
			double time = 0.0;
			auto& events = mdata.m_voiceEvents[v];
			const auto& rWords = mdata.m_splitRhythms[v];
			const auto& pWords = mdata.m_splitPitches[v];

			for (int w=0; w<(int)rWords.size(); ++w) {
				const auto& rhythms = rWords[w];
				const auto& pitches = pWords[w];
				for (int t=0; t<(int)rhythms.size(); ++t) {
					GotScore::Measure::TimedEvent evt;
					const string& r = rhythms[t];
					const string& p = (t < (int)pitches.size()) ? pitches[t] : ".";
					bool isInterp = (!r.empty() && r[0] == '*');

					evt.isInterpretation = isInterp;
					evt.rhythm           = r;
					evt.pitch            = isInterp ? "*" : p;
					evt.timestamp        = isInterp ? time - 0.0001 : time;
					evt.duration         = isInterp ? 0.0 : durationFromRhythmToken(r);
					events.push_back(evt);

					if (!isInterp && r != "." && evt.duration > 0.0) {
						time += evt.duration;
					}
				}
			}
		}
	}
}



//////////////////////////////
//
// GotScore::alignEventsByTimestamp -- Function to flatten all voices’
//     events by timestamp.  Merges all voices’ timed events into a single
//     time‐ordered list (EventAtTime), filling "." when a voice is silent.
//

vector<GotScore::EventAtTime> GotScore::alignEventsByTimestamp(const GotScore::Measure& mdata) {
	vector<EventAtTime> result;
	set<double> allTimes;

	for (const auto& voice : mdata.m_voiceEvents) {
		for (const auto& evt : voice) {
			allTimes.insert(evt.timestamp);
		}
	}

	for (double t : allTimes) {
		// bool isInterpRow = false;
		for (const auto& voice : mdata.m_voiceEvents) {
			auto it = std::find_if(voice.begin(), voice.end(), [&](const GotScore::Measure::TimedEvent& e) {
				return std::abs(e.timestamp - t) < 1e-6 && e.isInterpretation;
			});
			if (it != voice.end()) {
				// isInterpRow = true;
				break;
			}
		}

		EventAtTime row;
		row.timestamp = t;
		for (const auto& voice : mdata.m_voiceEvents) {
			auto it = std::find_if(voice.begin(), voice.end(), [&](const GotScore::Measure::TimedEvent& e) {
				return std::abs(e.timestamp - t) < 1e-6;
			});
			if (it != voice.end()) {
				row.rhythms.push_back(it->rhythm);
				row.pitches.push_back(it->pitch);
				// row.pitches.push_back(it->isInterpretation ? "*" : it->pitch);
			} else {
				row.rhythms.push_back(".");
				row.pitches.push_back(".");
			}
		}
		result.push_back(row);
	}

	return result;
}



//////////////////////////////
//
// GotScore::processDotTiedNotes -- 2_ becomes [2, and the next note
//     becomes #].  Also a "." by iteslf will change "." into *2 of the
//     previous note duration (hopefully no "." augmentation dots, but
//     check for those later), and then add '[' to the previous duration
//     and ']' to the substitute duration.
//

void GotScore::processDotTiedNotes(void) {
	// Build flattened per-voice lists of pointers to every non-interpretation token
	vector<vector<string*>> R(m_voices);
	vector<vector<string*>> P(m_voices);

	for (auto& M : m_measures) {
		if (!M.m_error.empty()) {
			continue;
		}
		for (int v=0; v<m_voices; ++v) {
			// collect rhythm tokens
			for (auto& beat : M.m_splitRhythms[v]) {
				for (auto& tok : beat) {
					if (!tok.empty() && tok[0] != '*') {
						R[v].push_back(&tok);
					}
				}
			}
			// collect pitch tokens
			for (auto& beat : M.m_splitPitches[v]) {
				for (auto& tok : beat) {
					if (!tok.empty() && tok[0] != '*') {
						P[v].push_back(&tok);
					}
				}
			}
		}
	}

	// Half‐duration ties (rhythm dots):
	for (int v = 0; v < m_voices; ++v) {
		processRhythmTies(R[v], P[v]);
	}

	// Pitch‐dot ties:
	for (int v = 0; v < m_voices; ++v) {
		processPitchDotsByVoice(P[v]);
	}

	// store pitches in histogram for later calculation of
	// clefs for each voice.
	storePitchHistograms(P);
}



//////////////////////////////
//
// GotScore::storePitchHistograms -- Convert notes for each voice to MIDI
//    pitches and store in pitch histograms by voice to be used later to calculate
//    the optimal clef (bass, treble, or vocal-tenor).
//

void GotScore::storePitchHistograms(vector<vector<string*>>& P) {
	m_pitch_hist.resize(P.size());
	for (int v=0; v<(int)P.size(); ++v) {
		m_pitch_hist.at(v).resize(127);
		std::fill(m_pitch_hist[v].begin(), m_pitch_hist[v].end(), 0);
	}

	for (int v=0; v<(int)P.size(); ++v) {
		for (int i=0; i<(int)P.at(v).size(); ++i) {
			string& p = *P.at(v).at(i);
			if (p.empty()) {
				continue;
			}
			if (p == ".") {
				continue;
			}
			if (p[0] == '*') {
				continue;
			}
			int midi = GotScore::kernToMidiNoteNumber(p);
			if (midi < 0) {
				continue;
			}
			m_pitch_hist.at(v).at(midi)++;
		}
	}
}



//////////////////////////////
//
// GotScore::processPitchDotsByVoice -- Implements "." in the pitch stream:
//     copy the previous pitch into a lone "." slot.
//

void GotScore::processPitchDotsByVoice(vector<string*>& pitches) {
	for (int i=1; i<(int)pitches.size(); i++) {
		if (*pitches.at(i) == ".") {
			string& current = *pitches.at(i);
			string& previous = *pitches.at(i-1);
			current = previous;
		}
	}
}



//////////////////////////////
//
// GotScore::processRhythmTies -- Implements "." in the rhythm stream:
//     replace with half duration and append "]" to the corresponding pitch.
//

void GotScore::processRhythmTies(vector<string*>& rhythms, vector<string*>& pitches) {
	for (int i=1; i<(int)rhythms.size(); ++i) {
		  if (*rhythms[i] == ".") {
				char d = (*rhythms[i-1])[0];
				char half = (d=='1' ? '2': d=='2' ? '4': d=='4' ? '8': d);
				*rhythms[i] = string(1, half);
				*pitches[i] = *pitches[i-1] + "]";
		  }
	}
}



//////////////////////////////
//
// GotScore::convertGotToKernPitches -- Converts a vector of GOT-style
//     pitch tokens into **kern-style pitch strings.
//

vector<string> GotScore::convertGotToKernPitches(vector<string>& gotpitch) {
	vector<string> output;
	output.resize(gotpitch.size());
	for (int i=0; i<(int)gotpitch.size(); i++) {
		output[i] = convertGotToKernPitch(gotpitch[i]);
	}
	return output;
}



//////////////////////////////
//
// GotScore::convertGotToKernPitch -- Converts one GOT pitch
//     (such as “c//” or “D#”) into the appropriate **kern pitch
//     (with octave markers).
//
//   GOT     PITCH   KERN
//   --------------------
//   C     = C2    = CC
//   c     = C3    = C
//   c/    = C4    = c
//   c//   = C5    = cc
//   c///  = C6    = ccc
//

string GotScore::convertGotToKernPitch(const string& gotpitch) {
	if (gotpitch == ".") {
		return ".";
	}
	if (gotpitch == "*") {
		return "*";
	}

	// Strip trailing underscore if present and replace later
	bool hasUnderscore = false;
	string gp = gotpitch;
	if (!gp.empty() && gp.back() == '_') {
		gp.pop_back();
		hasUnderscore = true;
	}

	int slashes  = 0;
	int sharps   = 0;
	int flats    = 0;
	char letter  = '\0';
	bool isUpper = false;
	bool isLower = false;

	// Gather slash‐count, accidentals, and base letter
	for (char c : gp) {
		if (c == '/') {
			slashes++;
		} else if (c >= 'a' && c <= 'h') {
			letter = c;
			isLower = true;
			isUpper = false;
		} else if (c >= 'A' && c <= 'H') {
			letter = c;
			isUpper = true;
			isLower = false;
		} else if (c == '#') {
			sharps++;
		} else if (c == 'r') {
			// rest
			return "r";
		}
	}

	int octave = 8;
	if (isUpper) {
		octave = 2;
	} else if (isLower) {
		octave = 3 + slashes;
	} else {
		cerr << "STRANGE PITCH: " << gotpitch << "\n";
		return "?";
	}

	if (octave == 3) {
		letter = toupper(letter);
	}

	// Adjust accidentals: D#/d# -> E-/e-, B/b -> B-/b-, H/h -> B/b
	if ((letter == 'D' || letter == 'd') && sharps) {
		letter = (letter == 'D' ? 'E' : 'e');
		sharps = 0; flats = 1;
	} else if (letter == 'b' || letter == 'B') {
		flats = 1;
	} else if (letter == 'h') {
		letter = 'b';
	} else if (letter == 'H') {
		letter = 'B';
	}

	// Build the **kern pitch by duplicating letters according to octave
	string out;
	int reps = (octave < 4 ? 3 - octave + 1 : octave - 4 + 1);
	for (int i = 0; i < reps; ++i) {
		out += letter;
	}
	if (sharps) {
		out += '#';
	} else if (flats) {
		out += '-';
	}

	if (hasUnderscore) {
		out += "_";
	}

	return out;
}



//////////////////////////////
//
// GotScore::tokenizePitchString -- Convert a GOT pitch string into
//      separate notes, such as:
// ".c//rD_" into ".", "c//", "r", "D_".
//

vector<string> GotScore::tokenizePitchString(const string& input) {
	vector<string> tokens;
	size_t i = 0;
	const size_t len = input.size();

	if (!input.empty()) {
		if (input[0] == '*') {
			tokens.push_back(input);
			return tokens;
		}
	}

	// Optional standalone dot at the start
	if (i < len && input[i] == '.') {
		tokens.emplace_back(".");
		++i;
	}

	string token;
	while (i < len) {
		char c = input[i];
		if (isalpha(c) && tolower(c) >= 'a' && tolower(c) <= 'h') {
			token += input[i++];  // add base letter

			// Optional #
			if (i < len && input[i] == '#') {
				token += input[i++];
			}

			// Zero or more slashes
			while (i < len && input[i] == '/') {
				token += input[i++];
			}

			// Optional _
			if (i < len && input[i] == '_') {
				token += input[i++];
			}

			tokens.push_back(token);
			token.clear();
		} else if (c == 'r') {
				token += input[i++];
				tokens.push_back(token);
				token.clear();
		} else {
			// Ignore all other characters
			++i;

		}
	}

	return tokens;
}



//////////////////////////////
//
// GotScore::tokenizeRhythmString -- Convert GOT rhythm string into separate
//      tokens.  Example "4.88888" -> "4.", "8", "8", "8", "8", "8".
//                       ".4x3"    -> ".", "4", "4", "4".
//

vector<string> GotScore::tokenizeRhythmString(const string& input) {
	if (m_debugQ) {
		// Show the exact string about to be parse
		cerr << "\nRHYTHM IN: \"" << input << "\"" << endl;
	}

	vector<string> tokens;
	size_t i = 0;

	// 1) Interpretation tokens pass through unchanged
	if (!input.empty() && input[0] == '*') {
		tokens.push_back(input);
	} else {
		while (i < input.size()) {
			char c = input[i];

			// Digit start: collect a number + any _ ; . suffixes, then handle xN
			if (std::isdigit(c)) {
				string tok;
				tok += c;
				++i;

				// grab any trailing '_', ';', or '.' attached to this digit
				while (i < input.size() && (input[i] == '_' || input[i] == ';' || input[i] == '.')) {
					tok += input[i++];
				}

				// handle repeat pattern xN
				if (i + 1 < input.size() && input[i] == 'x' && std::isdigit(input[i+1])) {
					++i;  // skip 'x'
					int rep = 0;
					while (i < input.size() && std::isdigit(input[i])) {
						rep = rep * 10 + (input[i++] - '0');
					}
					for (int r = 0; r < rep; ++r) {
						tokens.push_back(tok);
					}
				} else {
					tokens.push_back(tok);
				}

			// Dot always stands alone
			} else if (c == '.') {
				tokens.emplace_back(".");
				++i;

			// Anything else gets skipped
			} else {
				++i;
			}
		}
	}

	// Debug: show exactly what tokens we produced
	if (m_debugQ) {
		cerr << "RHYTHM OUT:";
		for (auto& t : tokens) {
			cerr << " \"" << t << "\"";
		}
		cerr << endl;
	}

	return tokens;
}



//////////////////////////////
//
// GotScore::pairLeadingDots -- Leading dots in GOT data indicates a
//     continuation of the previous note by 1/2 of the duration of the
//     previous note.  These will be converted into tied notes when
//     across barlines, but also possible across metric beats such as
//     the half measure (or 1/3 measure in sesquialtera).
//

void GotScore::pairLeadingDots(void) {
	for (int i=0; i<(int)m_measures.size(); i++) {
		processDotsForMeasure(m_measures[i]);
	}
}



//////////////////////////////
//
// GotScore::processDotsForMeasure --
//     Either the rhythms or the pitches may have a leading dot, or both
//     may have a leading dot.  If only one has a leading dot, then add a
//     dot to the other data type.  Later the dots will be converted to
//     tied notes.
//
//     Dots with time signatures (mensuration signs) before them in the
//     measure will cause problems.
//

void GotScore::processDotsForMeasure(GotScore::Measure& mdata) {
	if (!mdata.m_error.empty()) {
		return;
	}
	for (int voice=0; voice<(int)mdata.m_splitRhythms.size(); voice++) {
		for (int word=0; word<(int)mdata.m_splitRhythms.at(voice).size(); word++) {
			if (mdata.m_splitRhythms.at(voice).at(word).at(0) == ".") {
				if (mdata.m_splitPitches.at(voice).at(word).at(0) != ".") {
					// Add a leading dot to splitPitches:
					vector<string>& vec = mdata.m_splitPitches.at(voice).at(word);
					vec.insert(vec.begin(), ".");
				}
			} else if (mdata.m_splitPitches.at(voice).at(word).at(0) == ".") {
				// Add a leading dot to splitRhythms:
				vector<string>& vec = mdata.m_splitRhythms.at(voice).at(word);
				vec.insert(vec.begin(), ".");
			}
		}
	}
}



//////////////////////////////
//
// GotScore::Measure::printKernBarline -- Outputs a barline (e.g. "=1")
//     in **kern format for every voice (and text column if present).
//

void GotScore::Measure::printKernBarline(ostream& out, bool textQ) {
	int voices = (int)m_rhythms.size();
	for (int i=0; i<voices; i++) {
		if (i>0) {
			out << "\t";
		}
		out << "=" << m_barnum;
	}
	if (textQ) {
		out << "\t=" << m_barnum;
	}
	out << endl;
}



//////////////////////////////
//
// GotScore::durationFromRhythmToken -- Parses a rhythm token such as "4." or
//     "8" into a numeric duration in beats (handles augmentation dots).
//

double GotScore::durationFromRhythmToken(const std::string& token) {
    if (token.empty() || token[0]=='*' || token == ".") {
        return 0.0;
    }

    static const std::regex re(R"((\d+)(\.*))");
    std::smatch m;
    if (std::regex_search(token, m, re)) {
        int base    = std::stoi(m[1].str());
        double dur  = 1.0 / base;
        for (char c : m[2].str()) {
            if (c == '.') dur += dur/2.0;
        }
        return dur;
    }

    return 0.0;
}



//////////////////////////////
//
// GotScore::processUnderscoreTies -- Implements "_" ties: wrap the current
//     token with "[" and appends "]" to both rhythm and pitch on the next token.
//     If the a token has both "[" and "]" present then convert to "_"; otherwise,
//     "_" should be removed.
//


void GotScore::processUnderscoreTies(vector<string*>& pitches) {
	if (pitches.empty()) {
		return;
	}

	// Convert "_" into tie pairs
	for (int i=0; i<(int)pitches.size(); i++) {
		size_t loc = pitches[i]->find('_');
		if (loc != string::npos) {
			pitches[i]->at(loc) = '[';
			if (i < (int)pitches.size() - 1) {
				pitches[i+1]->push_back(']');
			}
		}
	}

	// Finish dealing with dot ties by opening the tie on the
	// previous note.
	for (int i=1; i<(int)pitches.size(); i++) {
		size_t loc = pitches[i]->find(']');
		if (loc != string::npos) {
			size_t loc2 = pitches[i-1]->find('[');
			if (loc2 == string::npos) {
				pitches[i-1]->push_back('[');
			}
		}
	}

	// add terminal long marker
	*pitches.back() += 'l';
}



//////////////////////////////
//
// GotScore::setNoEditorial -- do not display parentheses around natural signs that are
//    not in the original scores (for example an "E" following an "E-flat" does not have an
//    an explicit natural sign in the GOT notation.  With this option set, the natural will
//    still be shown, but no parenthese around it.
//

void GotScore::setNoEditorial(void) {
	m_no_editorialQ = true;
}



//////////////////////////////
//
// GotScore::setCautionary -- display cautionary accidentals based on accidental states
//    states in the previous measure.
//

void GotScore::setCautionary(void) {
	m_cautionaryQ = true;
}



//////////////////////////////
//
// GotScore::setNoForcedAccidentals -- Use modern accidental-style display by
//    not showing all accidentals.
//

void GotScore::setNoForcedAccidentals(void) {
	m_modern_accQ = true;
}



///////////////////////////////
//
// GotScore::kernToMidiNoteNumber -- Convert **kern to MIDI note number
//    (middle C = 60).  Middle C is assigned to octave 5 rather than
//    octave 4 for the kernToBase12() function.
//

int GotScore::kernToMidiNoteNumber(const string& kerndata) {
	int pc = GotScore::kernToBase12PC(kerndata);
	int octave = GotScore::kernToOctaveNumber(kerndata);
	return pc + 12 * (octave + 1);
}



//////////////////////////////
//
// GotScore::kernToOctaveNumber -- Convert a kern token into an octave number.
//    Middle C is the start of the 4th octave. -1000 is returned if there
//    is not pitch in the string.  Only the first subtoken in the string is
//    considered.
//

int GotScore::kernToOctaveNumber(const string& kerndata) {
	int uc = 0;
	int lc = 0;
	if (kerndata == ".") {
		return -1000;
	}
	for (int i=0; i<(int)kerndata.size(); i++) {
		if (kerndata[i] == ' ') {
			break;
		}
		if (kerndata[i] == 'r') {
			return -1000;
		}
		uc += ('A' <= kerndata[i]) && (kerndata[i] <= 'G') ? 1 : 0;
		lc += ('a' <= kerndata[i]) && (kerndata[i] <= 'g') ? 1 : 0;
	}
	if ((uc > 0) && (lc > 0)) {
		// invalid pitch description
		return -1000;
	}
	if (uc > 0) {
		return 4 - uc;
	} else if (lc > 0) {
		return 3 + lc;
	} else {
		return -1000;
	}
}



//////////////////////////////
//
// GotScore::kernToBase12PC -- Convert **kern pitch to a base-12 pitch-class.
//   C=0, C#/D-flat=1, D=2, etc.  Will return -1 instead of 11 for C-, and
//   will return 12 instead of 0 for B#.
//

int GotScore::kernToBase12PC(const string& kerndata) {
	int diatonic = GotScore::kernToDiatonicPC(kerndata);
	if (diatonic < 0) {
		return diatonic;
	}
	int accid    = GotScore::kernToAccidentalCount(kerndata);
	int output = -1000;
	switch (diatonic) {
		case 0: output =  0; break;
		case 1: output =  2; break;
		case 2: output =  4; break;
		case 3: output =  5; break;
		case 4: output =  7; break;
		case 5: output =  9; break;
		case 6: output = 11; break;
	}
	output += accid;
	return output;
}



//////////////////////////////
//
// GotScore::kernToAccidentalCount -- Convert a kern token into a count
//    of accidentals in the first subtoken.  Sharps are assigned to the
//    value +1 and flats to -1.  So a double sharp is +2 and a double
//    flat is -2.  Only the first subtoken in the string is considered.
//    Cases such as "#-" should not exist, but in this case the return
//    value will be 0.
//

int GotScore::kernToAccidentalCount(const string& kerndata) {
	int output = 0;
	for (int i=0; i<(int)kerndata.size(); i++) {
		if (kerndata[i] == ' ') {
			break;
		}
		if (kerndata[i] == '-') {
			output--;
		}
		if (kerndata[i] == '#') {
			output++;
		}
	}
	return output;
}



//////////////////////////////
//
// GotScore::kernToBase40PC -- Convert **kern pitch to a base-40 pitch class.
//    Will ignore subsequent pitches in a chord.
//

int GotScore::kernToBase40PC(const string& kerndata) {
	int diatonic = GotScore::kernToDiatonicPC(kerndata);
	if (diatonic < 0) {
		return diatonic;
	}
	int accid  = GotScore::kernToAccidentalCount(kerndata);
	int output = -1000;
	switch (diatonic) {
		case 0: output =  0; break;
		case 1: output =  6; break;
		case 2: output = 12; break;
		case 3: output = 17; break;
		case 4: output = 23; break;
		case 5: output = 29; break;
		case 6: output = 35; break;
	}
	output += accid;
	return output + 2;     // +2 to make c-flat-flat bottom of octave.
}



//////////////////////////////
//
// GotScore::kernToDiatonicPC -- Convert a kern token into a diatonic
//    note pitch-class where 0="C", 1="D", ..., 6="B".  -1000 is returned
//    if the note is rest, and -2000 if there is no pitch information in the
//    input string. Only the first subtoken in the string is considered.
//

int GotScore::kernToDiatonicPC(const string& kerndata) {
	for (int i=0; i<(int)kerndata.size(); i++) {
		if (kerndata[i] == ' ') {
			break;
		}
		if (kerndata[i] == 'r') {
			return -1000;
		}
		switch (kerndata[i]) {
			case 'A': case 'a': return 5;
			case 'B': case 'b': return 6;
			case 'C': case 'c': return 0;
			case 'D': case 'd': return 1;
			case 'E': case 'e': return 2;
			case 'F': case 'f': return 3;
			case 'G': case 'g': return 4;
		}
	}
	return -2000;
}



// END_MERGE

} // end namespace hum



