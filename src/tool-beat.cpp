//
// Programmer:    Wolfgang Drescher <drescher.wolfgang@gmail.com>
// Creation Date: Fri Dez 13 08:08:00 UTC 2024
// Filename:      tool-beat.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/tool-beat.cpp
// Syntax:        C++11; humlib
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Analyzes **kern data and generates a rhythmic analysis
//

#include "tool-beat.h"
#include "Convert.h"
#include "HumRegex.h"

using namespace std;

namespace hum {

// START_MERGE

//////////////////////////////
//
// Tool_beat::Tool_beat -- Set the recognized options for the tool.
//

Tool_beat::Tool_beat(void) {
	define("s|spine-tracks|spine|spines|track|tracks=s", "process only the specified spines");
	define("k|kern-tracks=s",                            "process only the specified kern spines");
	define("d|duration|dur=b",                           "display duration for each slice");
	define("u|beatsize|rhythm=s:4",                      "beatsize (recip rhythmic value); default=4");
	define("f|float=b",                                  "floating-point beat values instead of rational numbers");
	define("D|digits=i:0",                               "number of digits after decimal point");
	define("c|comma=b",                                  "display decimal points as commas");
	define("t|tied|ties=b",                              "show beats of tied notes");
	define("r|rest=b",                                   "show beats of rests");
	define("full=b",                                     "show numbers on each slice");
}



//////////////////////////////
//
// Tool_beat::run -- Do the main work of the tool.
//

bool Tool_beat::run(HumdrumFileSet &infiles) {
	bool status = true;
	for (int i = 0; i < infiles.getCount(); i++) {
		status &= run(infiles[i]);
	}
	return status;
}

bool Tool_beat::run(const string &indata, ostream &out) {
	HumdrumFile infile(indata);
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}

bool Tool_beat::run(HumdrumFile &infile, ostream &out) {
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}

bool Tool_beat::run(HumdrumFile &infile) {
	initialize();
	processFile(infile);
	infile.createLinesFromTokens();
	return true;
}



//////////////////////////////
//
// Tool_beat::initialize --
//

void Tool_beat::initialize(void) {
	m_floatQ = getBoolean("float");
	m_digits = getInteger("digits");
	m_commaQ = getBoolean("comma");
	m_durationQ = getBoolean("duration");
	m_beatsize = getString("beatsize");
	m_restQ = getBoolean("rest");
	m_tiedQ = getBoolean("tied");
	m_fullQ = getBoolean("full");

	if (m_digits < 0) {
		m_digits = 0;
	}
	if (m_digits > 15) {
		m_digits = 15;
	}

	if (m_fullQ) {

	}

	if (getBoolean("spine-tracks")) {
		m_spineTracks = getString("spine-tracks");
	} else if (getBoolean("kern-tracks")) {
		m_kernTracks = getString("kern-tracks");
	}
}



//////////////////////////////
//
// Tool_beat::processFile --
//

void Tool_beat::processFile(HumdrumFile& infile) {

	vector<HTp> kernspines = infile.getKernLikeSpineStartList();

	int maxTrack = infile.getMaxTrack();

	m_selectedKernSpines.resize(maxTrack + 1); // +1 is needed since track=0 is not used

	// By default, process all tracks:
	fill(m_selectedKernSpines.begin(), m_selectedKernSpines.end(), true);
	// Otherwise, select which **kern track, or spine tracks to process selectively:

	// Calculate which input spines to process based on -s or -k option:
	if (!m_kernTracks.empty()) {
		vector<int> ktracks = Convert::extractIntegerList(m_kernTracks, maxTrack);
		fill(m_selectedKernSpines.begin(), m_selectedKernSpines.end(), false);
		for (int i=0; i<(int)ktracks.size(); i++) {
			int index = ktracks[i] - 1;
			if ((index < 0) || (index >= (int)kernspines.size())) {
				continue;
			}
			int track = kernspines.at(ktracks[i] - 1)->getTrack();
			m_selectedKernSpines.at(track) = true;
		}
	} else if (!m_spineTracks.empty()) {
		infile.makeBooleanTrackList(m_selectedKernSpines, m_spineTracks);
	}

	string suffix = m_durationQ ? "dur" : "absb";
	string exinterp = "**cdata-" + suffix;
	
	for (int i = 0; i < kernspines.size(); i++) {
		if (m_selectedKernSpines[i]) {
			vector<string> trackData = getTrackData(infile, kernspines[i]->getTrack());
			infile.insertDataSpineAfter(kernspines[i]->getTrack(), trackData, ".", exinterp);
		}
	}

	// Enables usage in verovio (`!!!filter: beat`)
	m_humdrum_text << infile;
}



//////////////////////////////
//
// Tool_beat::getTrackData -- Create beat spine data
//

vector<string> Tool_beat::getTrackData(HumdrumFile& infile, int track) {
	vector<string> trackData;
	trackData.resize(infile.getLineCount());

	for (int i = 0; i < infile.getLineCount(); i++) {
		HumNum num;
		HLp line = infile.getLine(i);
		if (!line->hasSpines()) continue;		
		HTp token = nullptr;

		// Get token for current track
		for (int j = 0; j < infile[i].getFieldCount(); j++) {
			if (track == infile.token(i, j)->getTrack()) {
				// TODO handle spine splits
				token = infile.token(i, j);
				break;
			}
		}

		if (token == nullptr) continue;

		// If -d isset use token or line duration (depending on --full) as num
		if (m_durationQ) {
			num = m_fullQ ? line->getDuration() : token->getDurationToNoteEnd();
		} else {
			// Use duration from start as num
			num = token->getDurationFromStart();
		}

		// Hide output if not useful or not configured in options
		if (num == -1) continue;
		if (token->isRest() && !m_restQ && !m_fullQ) continue;
		if (token->isNull() && !m_fullQ) continue;
		if (token->isSustainedNote() && !m_tiedQ && !m_fullQ) continue;

		// Use beatsize as basis for calculations
		HumNum beatsize = Convert::recipToDuration(m_beatsize);
		if (beatsize == 0) continue;
		num /= beatsize;

		string str = "";
		if (m_floatQ) {
			// Print floating-point beat values instead of rational numbers
			stringstream beatStream;
			if (m_digits) {
				beatStream << std::setprecision(m_digits + 1) << num.getFloat();
			} else {
				beatStream << num.getFloat();
			}
			str = beatStream.str();
			if (m_commaQ) {
				// Repalce . with , when using -f
				HumRegex hre;
				hre.replaceDestructive(str, ",", "\\.");
			}
		} else {
			// Print two part HumNum as default option
			stringstream beatStream;
			num.printTwoPart(beatStream);
			str = beatStream.str();
		}

		if (token->isRest()) {
			// Add "r" suffix for rests
			str = str + "r";
		}

		trackData[i] = str;
	}

	return trackData;
}


// END_MERGE

} // end namespace hum
