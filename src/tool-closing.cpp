//
// Programmer:    Alexander Morgan
// Creation Date: Sun Aug 16 2026
// Filename:      tool-closing.cpp
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Count how many voices stop sounding at each observation point
//                of a score in order to evaluate potential cadence points.
//
//                A voice is counted when its event there is a closingRest (the
//                voice falls silent at that point) or a closingAttack (the voice
//                rests, or the piece ends, at its next attack), both marked by
//                HumdrumFileContent::analyzeClosingRests().  Counts range from 0
//                to the number of voices, and a high count means that many of
//                the voices sounding just before that point drop out at it,
//                which is typical of a cadence.  Every data line is an
//                observation point, including lines where all voices are
//                resting.
//

#include "tool-closing.h"

#include <algorithm>
#include <string>

using namespace std;

namespace hum {

// START_MERGE


/////////////////////////////////
//
// Tool_closing::Tool_closing -- Set the recognized options for the tool.
//

Tool_closing::Tool_closing(void) {
	define("p|prepend=b",    "prepend analysis spine instead of appending it");
	define("m|mark=b",       "mark closing attacks and closing rests in the score");
	define("r|raw=b",        "print analysis as a plain text table");
	define("n|minimum=i:0",  "only report counts of at least this size");
}



/////////////////////////////////
//
// Tool_closing::run -- Do the main work of the tool.
//

bool Tool_closing::run(HumdrumFileSet& infiles) {
	bool status = true;
	for (int i=0; i<infiles.getCount(); i++) {
		status &= run(infiles[i]);
	}
	return status;
}


bool Tool_closing::run(const string& indata, ostream& out) {
	HumdrumFile infile;
	infile.readString(indata);
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_closing::run(HumdrumFile& infile, ostream& out) {
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_closing::run(HumdrumFile& infile) {
	initialize();
	processFile(infile);
	return true;
}



//////////////////////////////
//
// Tool_closing::initialize --
//

void Tool_closing::initialize(void) {
	m_prependQ = getBoolean("prepend");
	m_markQ    = getBoolean("mark");
	m_rawQ     = getBoolean("raw");
	m_minimum  = getInteger("minimum");
	if (m_minimum < 0) {
		// keep non-negative so that the -1 of non-data lines is never printed
		m_minimum = 0;
	}
}



//////////////////////////////
//
// Tool_closing::processFile --
//

void Tool_closing::processFile(HumdrumFile& infile) {
	infile.analyzeClosingRests();
	countClosingVoices(infile);

	if (m_rawQ) {
		printRawAnalysis(infile);
		return;
	}

	if (m_markQ) {
		markClosingEvents(infile);
	}
	addAnalysisSpine(infile);
	m_humdrum_text << infile;
}



//////////////////////////////
//
// Tool_closing::countClosingVoices -- For each observation point in the
//     composite rhythm of all of the parts, count the voices whose event at
//     that point is a closingAttack or a closingRest.  Every data line is an
//     observation point, including lines where all of the voices are resting.
//

void Tool_closing::countClosingVoices(HumdrumFile& infile) {
	m_counts.clear();
	m_counts.resize(infile.getLineCount(), -1);

	// A staff with more than one layer can have several closing events on the
	// same line, but it is a single voice, so count each track only once.
	vector<bool> counted(infile.getTrackCount() + 1, false);

	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isData()) {
			continue;
		}
		fill(counted.begin(), counted.end(), false);
		int sum = 0;
		for (int j=0; j<infile[i].getFieldCount(); j++) {
			HTp token = infile.token(i, j);
			if (!infile.isClosingEvent(token)) {
				continue;
			}
			int track = token->getTrack();
			if (counted[track]) {
				continue;
			}
			counted[track] = true;
			sum++;
		}
		m_counts[i] = sum;
	}
}



//////////////////////////////
//
// Tool_closing::markClosingEvents -- Mark each closing attack and each closing
//     rest so that they can be seen in the notation.
//

void Tool_closing::markClosingEvents(HumdrumFile& infile) {
	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isData()) {
			continue;
		}
		for (int j=0; j<infile[i].getFieldCount(); j++) {
			HTp token = infile.token(i, j);
			if (infile.isClosingAttack(token)) {
				token->setText(*token + m_attackMarker);
			} else if (infile.isClosingRest(token)) {
				token->setText(*token + m_restMarker);
			}
		}
	}
	infile.createLinesFromTokens();

	infile.appendLine("!!!RDF**kern: " + m_attackMarker
			+ " = marked note, closing attack, color=\"" + m_attackColor + "\"");
	infile.appendLine("!!!RDF**kern: " + m_restMarker
			+ " = marked note, closing rest, color=\"" + m_restColor + "\"");
}



//////////////////////////////
//
// Tool_closing::addAnalysisSpine -- Add a **closing spine containing the number
//     of closing voices at each observation point.
//

void Tool_closing::addAnalysisSpine(HumdrumFile& infile) {
	vector<string> data(infile.getLineCount());
	for (int i=0; i<infile.getLineCount(); i++) {
		if (m_counts[i] < m_minimum) {
			continue;
		}
		data[i] = to_string(m_counts[i]);
	}
	if (m_prependQ) {
		infile.prependDataSpine(data, "", "**closing");
	} else {
		infile.appendDataSpine(data, "", "**closing");
	}
}



//////////////////////////////
//
// Tool_closing::printRawAnalysis -- Print one line for each reported observation
//     point: its line number in the input, its time in quarter notes from the
//     start of the score, and the number of closing voices.
//

void Tool_closing::printRawAnalysis(HumdrumFile& infile) {
	m_humdrum_text << "!!!voice-count: " << infile.getKernSpineStartList().size() << endl;
	m_humdrum_text << "**line\t**qbeat\t**closing" << endl;
	for (int i=0; i<infile.getLineCount(); i++) {
		if (m_counts[i] < m_minimum) {
			continue;
		}
		m_humdrum_text << i+1
		               << "\t" << infile[i].getDurationFromStart().getFloat()
		               << "\t" << m_counts[i]
		               << endl;
	}
	m_humdrum_text << "*-\t*-\t*-" << endl;
}


// END_MERGE

} // end namespace hum
