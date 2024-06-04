//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sun Jul 14 01:03:07 CEST 2019
// Last Modified: Thu Jun  9 15:23:15 PDT 2022
// Filename:      tool-composite.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/tool-composite.cpp
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Generate composite rhythm spine for music.
//
// To do:
// (1) Add ties that are missing from composite notation
// (2) Unmetered tremolos as ornaments in analyses
// (3) Grace notes counted as ornaments (deal with trill endings later)
// (5) Ties unmetered tremolo and grace notes (accent tool)
// (R130) has a problem in coincidence (ties)
//

#include "tool-autobeam.h"
#include "tool-composite.h"
#include "tool-extract.h"
#include "Convert.h"
#include "HumRegex.h"

#include <sstream>

using namespace std;

namespace hum {

// START_MERGE


// Note state variables for grouping:
#define TYPE_UNDEFINED           9 /* for inactive groupings */
#define TYPE_NoteSustainAttack   3
#define TYPE_NoteAttack          2
#define TYPE_RestAttack          1
#define TYPE_NONE                0
#define TYPE_RestSustain        -1
#define TYPE_NoteSustain        -2
#define TYPE_NoteSustainSustain -3

#define COMPOSITE_TREMOLO_MARKER "||"


/////////////////////////////////
//
// Tool_composite::Tool_composite -- Set the recognized options for the tool.
//

Tool_composite::Tool_composite(void) {
	define("debug=b",                  "print debug statements");
	define("a|append=b",               "append data to end of line (top of system)");
	define("x|extract=b",              "only output composite rhythm spines");
	define("grace=b",                  "include grace notes in composite rhythms");
	define("u|up-stem=b",              "force notes to be up-stem");
	define("C|color-full-composite=b", "color full composite rhythm if score has groups");
	define("l|score-size=d:100.0",     "set staff size of input score (percent)");
	define("L|analysis-size=d:100.0",  "set staff size of analysis staves (percent)");
	define("o|only=s",                 "output notes of given group (A or B)");
	define("r|rhythms=b",              "convert input score to rhythms only.");
	define("e|events=b",               "show event counts on analysis staves.");
	define("F|no-full-composite=b",    "do not do full composite rhythm analysis");
	define("c|coincidence=b",          "do coincidence rhythm analysis");
	define("g|group|groups=b",         "do group rhythm analysis");
	define("m|mark=b",                 "mark coincidences in group analysis and input score");
	define("M|mark-input=b",           "mark coincidences in input score");

	// Numeric analysis options:
	define("A|analysis|analyses=s",    "list of numeric analysis features to extract");

	// Styling for numeric analyses;
	define("Z|no-zeros|no-zeroes=b",   "do not show zeros in analyses.");
}



//////////////////////////////
//
// Tool_composite::initialize -- Prepare interface variables.
//

void Tool_composite::initialize(HumdrumFile& infile) {
	m_debugQ   = getBoolean("debug");
	m_appendQ  = getBoolean("append");
	m_extractQ = getBoolean("extract");
	if (m_extractQ) {
		m_appendQ = false;
		m_prependQ = false;
	}
	m_graceQ   = getBoolean("grace");

	m_hasGroupsQ = hasGroupInterpretations(infile);

	m_colorFullCompositeQ =  getBoolean("color-full-composite");
	m_fullCompositeQ      = !getBoolean("no-full-composite");
	m_coincidenceQ        =  getBoolean("coincidence");
	m_groupsQ             =  getBoolean("groups");
	m_upstemQ             =  getBoolean("up-stem");
	m_rhythmQ             =  getBoolean("rhythms");
	m_eventQ              =  getBoolean("events");

	// There must be at least one analysis being done (excluding -o options):
	if (!m_groupsQ && !m_coincidenceQ) {
		m_fullCompositeQ = true;
	}

	// Extract music in a specific group:
	m_onlyQ          = getBoolean("only");
	m_only           = getString("only");

	m_scoreSize      = getDouble("score-size");
	m_analysisSize   = getDouble("analysis-size");

	if (m_fullCompositeQ) {
		m_fullComposite.resize(infile.getLineCount());
	}

	m_groups.resize(2);
	m_groups[0].resize(infile.getLineCount());
	m_groups[1].resize(infile.getLineCount());

	m_analysisOnsetsQ    = false;
	m_analysisAccentsQ   = false;
	m_analysisOrnamentsQ = false;
	m_analysisSlursQ     = false;
	m_analysisTotalQ     = false;

	if (getBoolean("analyses")) {
		string argument = getString("analyses");
		if (argument == "all") {
			m_analysisOnsetsQ    = true;
			m_analysisAccentsQ   = true;
			m_analysisOrnamentsQ = true;
			m_analysisSlursQ     = true;
			m_analysisTotalQ     = true;
		} else {
			if (argument.find("n") != string::npos) {
				m_analysisOnsetsQ = true;
			}
			if (argument.find("a") != string::npos) {
				m_analysisAccentsQ = true;
			}
			if (argument.find("o") != string::npos) {
				m_analysisOrnamentsQ = true;
			}
			if (argument.find("s") != string::npos) {
				m_analysisSlursQ = true;
			}
			if (argument.find("t") != string::npos) {
				m_analysisTotalQ = true;
			}
		}
	}

	m_analysisIndex.resize(5);
	m_analysisIndex[0] = m_analysisOnsetsQ;
	m_analysisIndex[1] = m_analysisAccentsQ;
	m_analysisIndex[2] = m_analysisOrnamentsQ;
	m_analysisIndex[3] = m_analysisSlursQ;
	m_analysisIndex[4] = m_analysisTotalQ;

	m_nozerosQ         = getBoolean("no-zeros");
	m_numericAnalysisSpineCount = 0;
	m_analysesQ = false;
	for (int i=0; i<(int)m_analysisIndex.size(); i++) {
		if (m_analysisIndex[i]) {
			m_analysesQ = true;
			m_numericAnalysisSpineCount++;
		}
	}

	initializeNumericAnalyses(infile);
	m_assignedQ = false;
	m_coinMarkQ = getBoolean("mark");
	if (getBoolean("mark-input")) {
		m_coinMarkQ = true;
		m_extractInputQ = true;
	}
}



//////////////////////////////
//
// Tool_composite::initializeNumericAnalyses --
//

void Tool_composite::initializeNumericAnalyses(HumdrumFile& infile) {
	double initValue = 0;
	m_analyses.clear();
	m_analyses.resize(m_ANALYSES_DIM1);  // first index is composite rhythm type
	for (int i=0; i<(int)m_analyses.size(); i++) {
		m_analyses.at(i).resize(m_ANALYSES_DIM2);  // second index is analysis type
		if (m_analysisOnsetsQ) {
			m_analyses.at(i).at(m_ONSET).resize(infile.getLineCount());
		}
		if (m_analysisAccentsQ) {
			m_analyses.at(i).at(m_ACCENT).resize(infile.getLineCount());
		}
		if (m_analysisOrnamentsQ) {
			m_analyses.at(i).at(m_ORNAMENT).resize(infile.getLineCount());
		}
		if (m_analysisSlursQ) {
			m_analyses.at(i).at(m_SLUR).resize(infile.getLineCount());
		}
		if (m_analysisTotalQ) {
			m_analyses.at(i).at(m_TOTAL).resize(infile.getLineCount());
		}
		// third index is the line number in input file:
		for (int j=0; j<(int)m_analysisIndex.size(); j++) {
			if (m_analysisIndex[j]) {
				m_analyses.at(i).at(j).resize(infile.getLineCount());
				fill(m_analyses.at(i).at(j).begin(), m_analyses.at(i).at(j).end(), initValue);
			} else {
				m_analyses.at(i).at(j).clear();
			}
		}
	}

}



/////////////////////////////////
//
// Tool_composite::run -- Do the main work of the tool.
//

bool Tool_composite::run(HumdrumFileSet& infiles) {
	bool status = true;
	for (int i=0; i<infiles.getCount(); i++) {
		status &= run(infiles[i]);
	}
	return status;
}


bool Tool_composite::run(const string& indata, ostream& out) {
	HumdrumFile infile;
	infile.readStringNoRhythm(indata);
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_composite::run(HumdrumFile& infile, ostream& out) {
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_composite::run(HumdrumFile& infile) {
	processFile(infile);
	return true;
}



//////////////////////////////
//
// Tool_composite::processFile --
//

void Tool_composite::processFile(HumdrumFile& infile) {
	initialize(infile);
	if (m_onlyQ) {
		if (!m_assignedQ) {
			assignGroups(infile);
		}
		analyzeLineGroups(infile);
		extractGroup(infile, m_only);
		return;
	}

	if (m_groupsQ) {
		checkForAutomaticGrouping(infile);
	}

	if (m_coincidenceQ) {
		analyzeCoincidenceRhythms(infile);
	}
	if (m_fullCompositeQ) {
		analyzeFullCompositeRhythm(infile);
	}
	analyzeGroupCompositeRhythms(infile);
	if (m_analysesQ) {
		doNumericAnalyses(infile);
	}
	prepareOutput(infile);
}



//////////////////////////////
//
// checkForAutomaticGrouping --
//

void Tool_composite::checkForAutomaticGrouping(HumdrumFile& infile) {

	bool hasGroups = false;
	int interpline = -1;
	int manipline = -1;
	int dataline = -1;
	int barline = -1;

	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].hasSpines()) {
			continue;
		}
		if ((barline < 0) && infile[i].isBarline()) {
			barline = i;
		}
		if ((dataline < 0) && infile[i].isData()) {
			dataline = i;
		}
		if ((manipline < 0) && infile[i].isManipulator()) {
			HTp token = infile.token(i, 0);
			if ((!token->isExclusiveInterpretation()) && (!token->isTerminator())) {
				manipline = i;
			}
			continue;
		}
		if (!infile[i].isInterpretation()) {
			continue;
		}
		if ((dataline < 0) && (barline < 0) && (manipline < 0)) {
			// Put below last interp line
			HTp current = infile.token(i, 0);
			current = current->getNextToken();
			if (current) {
				interpline = current->getLineIndex();
			} else {
				interpline = i;
			}
		}
		for (int j=0; j<infile[i].getFieldCount(); j++) {
			HTp token = infile.token(i, j);
			if (!token->isKern()) {
				continue;
			}
			if (token->compare(0, 5, "*grp:") == 0) {
				hasGroups = 1;
				break;
			}
		}
		if (hasGroups) {
			break;
		}
	}
	if (hasGroups) {
		return;
	}

	// No Groupings found in score, so add *grp:A to the first kern spine
	// and *grp:B to the second kern spine (from left-to-right).

	vector<HTp> sstarts;
	infile.getSpineStartList(sstarts, "**kern");
	if (sstarts.size() != 2) {
		return;
	}

	// Add a new interpretation line before the first data line, but also before
	// the first barline and also before the first manipline.
	int addline = dataline;
	if (addline < 0) {
		// no data?
		return;
	}
	if ((addline > barline) && (barline > 0)) {
		addline = barline;
	}
	if ((addline > manipline) && (manipline > 0)) {
		addline = manipline;
	}
	if ((addline > interpline) && (interpline > 0)) {
		addline = interpline;
	}

	if (addline < 0) {
		// something strange
		return;
	}

	infile.insertNullInterpretationLineAboveIndex(addline);
	for (int i=0; i< (int)sstarts.size(); i++) {
		int track = sstarts[i]->getTrack();
		for (int j=0; j<infile[addline].getFieldCount(); j++) {
			HTp token = infile.token(addline, j);
			int jtrack = token->getTrack();
			if (track != jtrack) {
				continue;
			}
			if (i == 0) {
				token->setText("*grp:A");
			} else if (i == 1) {
				token->setText("*grp:B");
			}
			break;
		}
	}
	infile[addline].createLineFromTokens();
}



//////////////////////////////
//
// Tool_composite::hasGroupInterpretations --
//

bool Tool_composite::hasGroupInterpretations(HumdrumFile& infile) {
	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isInterpretation()) {
			continue;
		}
		for (int j=0; j<infile[i].getFieldCount(); j++) {
			HTp token = infile.token(i, j);
			if (!token->isKern()) {
				continue;
			}
			if (token->compare(0, 5, "*grp:") == 0) {
				return true;
			}
		}
	}
	return false;
}



//////////////////////////////
//
// Tool_composite::addCoincidenceMarks --
//

void Tool_composite::addCoincidenceMarks(HumdrumFile& infile) {
	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isData()) {
			continue;
		}
		bool domark = needsCoincidenceMarker(i);
		if (!domark) {
			continue;
		}
		for (int j=0; j<infile[i].getFieldCount(); j++) {
			HTp token = infile.token(i, j);
			if (!token->isKern()) {
				continue;
			}
			if (token->isNull()) {
				continue;
			}
			if (token->isRest()) {
				continue;
			}
			if (token->isNoteAttack()) {
				string text = *token;
				text += m_coinMark;
				token->setText(text);
			}
		}
	}
}



//////////////////////////////
//
// Tool_composite::prepareOutput --
//

void Tool_composite::prepareOutput(HumdrumFile& infile) {
	analyzeOutputVariables(infile);

	if (m_coinMarkQ) {
		addCoincidenceMarks(infile);
		infile.generateLinesFromTokens();
	}

	if (m_rhythmQ) {
		convertNotesToRhythms(infile);
	}

	stringstream analysis;

	// Prepare the output data:
	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].hasSpines()) {
			analysis << infile[i];
		} else {
			getAnalysisOutputLine(analysis, infile, i);
		}
		analysis << endl;
	}

	HumdrumFile output;
	output.readString(analysis.str());
	stringstream tempout;

	addStaffInfo(output, infile);
	addTimeSignatureChanges(output, infile);
	addMeterSignatureChanges(output, infile);
	adjustBadCoincidenceRests(output, infile);
	for (int i=0; i<output.getLineCount(); i++) {
		output[i].createLineFromTokens();
	}

	HumRegex hre;

	for (int i=0; i<infile.getLineCount(); i++) {
		if (m_verseLabelIndex && (m_verseLabelIndex == -i)) {
			string labelLine = generateVerseLabelLine(output, infile, i);
			if (!labelLine.empty()) {
				if (!hre.search(labelLine, "^[*\t]+$")) {
					tempout << labelLine;
					tempout << endl;
				}
			}
		}

		if (m_striaIndex && (m_striaIndex == -i)) {
			string striaLine =  generateStriaLine(output, infile, i);
			if (!striaLine.empty()) {
				tempout << striaLine;
				tempout << endl;
			}
		}

		if (((m_scoreSize != 100.0) || (m_analysisSize != 100.0)) && m_sizeIndex && (m_sizeIndex == -i)) {
			string sizeLine = generateSizeLine(output, infile, i);
			if (!sizeLine.empty()) {
				tempout << sizeLine;
				tempout << endl;
			}
		}

		if (!infile[i].hasSpines()) {
			tempout << output[i];
		} else if (m_appendQ) {
			// analysis data at end of line
			if (m_extractInputQ || !m_extractQ) {
				tempout << infile[i];
			}
			if (!(m_extractQ || m_extractInputQ)) {
				tempout << "\t";
			}
			if (m_extractQ || !m_extractInputQ) {
				tempout << output[i];
			}
		} else if (m_prependQ) {
			// analysis data at start of line (default)
			if (!m_extractInputQ || m_extractQ) {
				tempout << output[i];
			}
			if (!(m_extractQ || m_extractInputQ)) {
				tempout << "\t";
			}
			if (!m_extractQ || m_extractInputQ) {
				tempout << infile[i];
			}
		} else {
			// output data only
			tempout << output[i];
		}
		tempout << endl;
	}

	if (m_beamQ) {
	/*
		string spinelist = "1";
		int otracks = output.getMaxTrack();
		if (otracks > 1) {
			spinelist += "-" + to_string(otracks);
		}
		string soption = "-s " + spinelist;
		vector<string> argv;
		argv.push_back(soption);    // only beam analysis spines.
		argv.push_back("-g");       // beam adjacent grace notes

		Tool_autobeam autobeam;
		autobeam.process(argv);
		HumdrumFile finaloutput;
		finaloutput.readString(tempout.str());
		autobeam.run(finaloutput);
		for (int i=0; i<finaloutput.getLineCount(); i++) {
			finaloutput[i].createLineFromTokens();
		}
		m_humdrum_text << finaloutput;
	*/

		HumdrumFile finaloutput;
		finaloutput.readString(tempout.str());
		Tool_autobeam autobeam;
		autobeam.run(finaloutput);
		m_humdrum_text << finaloutput;

	} else {
		m_humdrum_text << tempout.str();
	}

	if (m_coinMarkQ) {
		m_humdrum_text << "!!!RDF**kern: " << m_coinMark;
		m_humdrum_text << " = marked note, coincidence note, color=\"";
		m_humdrum_text << m_coinMarkColor << "\"" << endl;
	}
	if (m_colorFullCompositeQ) {
		m_humdrum_text << "!!!RDF**kern: " << m_AMark;
      m_humdrum_text << " = marked note, polyrhythm group A, color=\"";
		m_humdrum_text << m_AMarkColor << "\"" << endl;
		m_humdrum_text << "!!!RDF**kern: " << m_BMark;
      m_humdrum_text << " = marked note, polyrhythm group B, color=\"";
		m_humdrum_text << m_BMarkColor << "\"" << endl;
		if (!m_coinMarkQ) {
			m_humdrum_text << "!!!RDF**kern: " << m_coinMark;
			m_humdrum_text << " = marked note, coincidence note, color=\"";
			m_humdrum_text << m_coinMarkColor << "\"" << endl;
		}
	}

	if (m_groupBEventCount >= 0) {
		m_humdrum_text << "!!!group-b-event-count: " << m_groupBEventCount << endl;
	}
	if (m_groupAEventCount >= 0) {
		m_humdrum_text << "!!!group-a-event-count: " << m_groupAEventCount << endl;
	}
	if (m_fullCompositeEventCount >= 0) {
		m_humdrum_text << "!!!composite-event-count: " << m_fullCompositeEventCount << endl;
	}
	if (m_coincidenceEventCount >= 0) {
		m_humdrum_text << "!!!coincidence-event-count: " << m_coincidenceEventCount << endl;
	}
}



//////////////////////////////
//
// Tool_composite::generateVerseLabelLine --
//

string Tool_composite::generateVerseLabelLine(HumdrumFile& output, HumdrumFile& input, int line) {

	if (m_extractInputQ) {
		return "";
	}

	string outstring;
	string inputBlanks;
	if (!m_extractQ) {
		for (int i=0; i<input[line].getFieldCount(); i++) {
			inputBlanks += "*";
			if (i < input[line].getFieldCount() - 1) {
				inputBlanks += "\t";
			}
		}
	}
	if (!(m_extractQ || m_extractInputQ)) {
		if (m_appendQ) {
			outstring += inputBlanks;
			outstring += "\t";
		}
	}
	string outputLabels;
	if (!m_extractInputQ) {
		for (int i=0; i<output[line].getFieldCount(); i++) {
			HTp token = output.token(line, i);
			string exinterp = token->getExInterp();
			if (exinterp.compare(0, 8, "**vdata-") != 0) {
				outputLabels += "*";
				if (i < output[line].getFieldCount() - 1) {
					outputLabels += "\t";
				}
				continue;
			}
			string label = exinterp.substr(8);
			outputLabels += "*v:";
			outputLabels += label;
			outputLabels += ":";
			if (i < output[line].getFieldCount() - 1) {
				outputLabels += "\t";
			}
		}
	}
	outstring += outputLabels;
	if (m_prependQ || m_extractQ) {
		if (!(m_extractQ || m_extractInputQ)) {
			outstring += "\t";
		}
		outstring += inputBlanks;
	}

	return outstring;
}



//////////////////////////////
//
// Tool_composite::generateStriaLine --
// m_extractQ      == output only
// m_extractInputQ == input only
//

string Tool_composite::generateStriaLine(HumdrumFile& output, HumdrumFile& input, int line) {

	if (m_extractInputQ) {
		return "";
	}

	string outstring;
	string inputBlanks;
	if (!m_extractQ) {
		for (int i=0; i<input[line].getFieldCount(); i++) {
			inputBlanks += "*";
			if (i < input[line].getFieldCount() - 1) {
				inputBlanks += "\t";
			}
		}
		if (m_appendQ) {
			outstring += inputBlanks;
			if (!m_extractInputQ) {
				outstring += "\t";
			}
		}
	}

	string outputStria;
	if (!m_extractInputQ) {
		for (int i=0; i<output[line].getFieldCount(); i++) {
			HTp token = output.token(line, i);
			string exinterp = token->getExInterp();
			if (exinterp.compare(0, 6, "**kern") != 0) {
				outputStria += "*";
				if (i < output[line].getFieldCount() - 1) {
					outputStria += "\t";
				}
				continue;
			}
			outputStria += "*stria1";
			if (i < output[line].getFieldCount() - 1) {
				outputStria += "\t";
			}
		}
	}

	outstring += outputStria;
	if (m_prependQ || m_extractQ) {
		if (!(m_extractQ || m_extractInputQ)) {
			outstring += "\t";
		}
		outstring += inputBlanks;
	}

	return outstring;
}



//////////////////////////////
//
// Tool_composite::generateSizeLine --
// m_extractQ      == output only
// m_extractInputQ == input only
//

string Tool_composite::generateSizeLine(HumdrumFile& output, HumdrumFile& input, int line) {

	if (m_extractInputQ) {
		return "";
	}

	string outstring;
	string inputBlanks;
	if (!m_extractQ) {
		for (int i=0; i<input[line].getFieldCount(); i++) {
			HTp token = input.token(line, i);
			inputBlanks += "*";
			if (token->isKernLike() && (m_scoreSize != 100.0)) {
				stringstream value;
				value.str("");
				value << m_scoreSize;
				inputBlanks += "size:";
				inputBlanks += value.str();
				inputBlanks += "%";
			}
			if (i < input[line].getFieldCount() - 1) {
				inputBlanks += "\t";
			}
		}
		if (m_appendQ) {
			outstring += inputBlanks;
			if (!m_extractInputQ) {
				outstring += "\t";
			}
		}
	}

	string outputSize;
	if (!m_extractInputQ) {
		for (int i=0; i<output[line].getFieldCount(); i++) {
			HTp token = output.token(line, i);
			string exinterp = token->getExInterp();
			if (exinterp.compare(0, 6, "**kern") != 0) {
				outputSize += "*";
				if (output[line].getFieldCount()) {
					outputSize += "\t";
				}
				continue;
			}
			outputSize += "*";
			if (m_analysisSize != 100.0) {
				outputSize += "size:";
				stringstream value;
				value.str("");
				value << m_analysisSize;
				outputSize += value.str();
				outputSize += "%";
			}
			if (i < output[line].getFieldCount() - 1) {
				outputSize += "\t";
			}
		}
	}

	outstring += outputSize;
	if (m_prependQ || m_extractQ) {
		if (!(m_extractQ || m_extractInputQ)) {
			outstring += "\t";
		}
		outstring += inputBlanks;
	}

	return outstring;
}



//////////////////////////////
//
// Tool_composite::getAnalysisOutputLine --
//

void Tool_composite::getAnalysisOutputLine(ostream& output, HumdrumFile& infile, int line) {
	if (!infile[line].hasSpines()) {
		return;
	}
	bool processedQ = false;

	stringstream tempout;

	if (m_coincidenceQ) {
		string value = getCoincidenceToken(infile, line);
		tempout << value;
		if (m_upstemQ) {
			if (value.find("R") != string::npos) {
				tempout << "/";
			}
		}
		if (processedQ) {
			tempout << "\t";
		}
		processedQ = true;
		// print coincidence spine data here
		if (m_numericAnalysisSpineCount) {
			addNumericAnalyses(tempout, infile, line, m_analyses[m_COINCIDENCE]);
		}
	}

	if (m_fullCompositeQ) {
		if (processedQ) {
			tempout << "\t";
		}
		processedQ = true;
		tempout << getFullCompositeToken(infile, line);
		if (m_numericAnalysisSpineCount) {
			addNumericAnalyses(tempout, infile, line, m_analyses[m_COMPOSITE_FULL]);
		}
	}

	if (m_groupsQ) {
		if (processedQ) {
			tempout << "\t";
		}
		processedQ = true;
		tempout << getGroupCompositeToken(infile, line, 0);
		if (m_numericAnalysisSpineCount) {
			addNumericAnalyses(tempout, infile, line, m_analyses[m_COMPOSITE_A]);
		}
		tempout << "\t";
		tempout << getGroupCompositeToken(infile, line, 1);
		if (m_numericAnalysisSpineCount) {
			addNumericAnalyses(tempout, infile, line, m_analyses[m_COMPOSITE_B]);
		}
	}
	output << tempout.str();
}



//////////////////////////////
//
// Tool_composite::addNumericAnalyses -- For a composite rhythm spine, add numeric
//     analyses spines after it as necessary.
//

void Tool_composite::addNumericAnalyses(ostream& output, HumdrumFile& infile, int line,
		vector<vector<double>>& numericAnalyses) {
	if (!infile[line].hasSpines()) {
		return;
	}
	if (infile[line].isCommentLocal()) {
		for (int i=0; i<(int)m_analysisIndex.size(); i++) {
			if (m_analysisIndex[i]) {
				output << "\t" << "!";
			}
		}
	} else if (infile[line].isBarline()) {
		HTp token = infile.token(line, 0);
		for (int i=0; i<(int)m_analysisIndex.size(); i++) {
			if (m_analysisIndex[i]) {
				output << "\t" << token;
			}
		}
	} else if (infile[line].isInterpretation()) {
		HTp token = infile.token(line, 0);
		for (int i=0; i<(int)m_analysisIndex.size(); i++) {
			if (m_analysisIndex[i]) {
				output << "\t";
				if (*token == "*-") {
					output << token;
				} else if (token->compare(0, 2, "**") == 0) {
					switch (i) {
						case 0: output << "**vdata-onsets";    break;
						case 1: output << "**vdata-accents";   break;
						case 2: output << "**vdata-ornaments"; break;
						case 3: output << "**vdata-slurs";     break;
						case 4: output << "**vdata-total";     break;
					}
				} else {
					output << "*";
				}
			}
		}
	} else if (infile[line].isData()) {
		for (int i=0; i<(int)m_analysisIndex.size(); i++) {
			if (m_analysisIndex[i]) {
				double value = numericAnalyses.at(i).at(line);
				output << "\t";
				if (value < 0) {
					output << ".";
				} else {
					output << value;
				}
			}
		}
	} else {
		for (int i=0; i<(int)m_analysisIndex.size(); i++) {
			if (m_analysisIndex[i]) {
				output << "\t" << "PROBLEM";
			}
		}
	}
}



//////////////////////////////
//
// Tool_composite::getCoincidenceToken --
//

string Tool_composite::getCoincidenceToken(HumdrumFile& infile, int line) {
	if (infile[line].isData()) {
		if (m_coincidence[line] != "") {
			return m_coincidence[line];
		} else {
			return ".";
		}
	} else if (infile[line].isInterpretation()) {
		HTp token = infile.token(line, 0);
		if (*token == "*-") {
			return "*-";
		} else if (token->compare(0, 2, "**") == 0) {
			return "**kern-coin";
		} else if (line == m_clefIndex) {
			return "*clefX";
		} else if (line == m_instrumentNameIndex) {
			string output = "*I\"Coincidence";
			if (m_eventQ) {
				m_coincidenceEventCount = getEventCount(m_coincidence);
				stringstream value;
				value.str("");
				value << "\\n(" << m_coincidenceEventCount << " event";
				if (m_coincidenceEventCount != 1) {
					value << "s";
				}
				value << ")";
				output += value.str();
			}
			return output;
		} else if (line == m_instrumentAbbrIndex) {
			return "*I'Coin.";
		} else if (line == m_timeSignatureIndex) {
			return getTimeSignature(infile, m_timeSignatureIndex, "");
		} else if (line == m_meterSymbolIndex) {
			return getMetricSymbol(infile, m_meterSymbolIndex, "");
		} else {
			return "*";
		}
	} else if (infile[line].isCommentLocal()) {
		return "!";
	} else if (infile[line].isBarline()) {
		HTp token = infile.token(line, 0);
		return *token;
	} else {
		return "PROBLEM";
	}
}



//////////////////////////////
//
// Tool_composite::getFullCompositeToken --
//

string Tool_composite::getFullCompositeToken(HumdrumFile& infile, int line) {
	if (infile[line].isData()) {
		if (m_fullComposite[line] != "") {
			bool domark = needsCoincidenceMarker(line);
			string output = m_fullComposite[line];
			if (domark) {
				output += m_coinMark;
			} else if (m_colorFullCompositeQ) {
				output += getFullCompositeMarker(line);
			}
			return output;
		} else {
			return ".";
		}
	} else if (infile[line].isInterpretation()) {
		HTp token = infile.token(line, 0);
		if (*token == "*-") {
			return "*-";
		} else if (token->compare(0, 2, "**") == 0) {
			return "**kern-comp";
		} else if (line == m_clefIndex) {
			return "*clefX";
		} else if (line == m_instrumentNameIndex) {
			string output = "*I\"Composite";
			if (m_eventQ) {
				m_fullCompositeEventCount = getEventCount(m_fullComposite);
				stringstream value;
				value.str("");
				value << "\\n(" << m_fullCompositeEventCount << " event";
				if (m_fullCompositeEventCount != 1) {
					value << "s";
				}
				value << ")";
				output += value.str();
			}
			return output;
		} else if (line == m_instrumentAbbrIndex) {
			return "*I'Comp.";
		} else if (line == m_timeSignatureIndex) {
			return getTimeSignature(infile, m_timeSignatureIndex, "");
		} else if (line == m_meterSymbolIndex) {
			return getMetricSymbol(infile, m_meterSymbolIndex, "");
		} else {
			return "*";
		}
	} else if (infile[line].isCommentLocal()) {
		return "!";
	} else if (infile[line].isBarline()) {
		HTp token = infile.token(line, 0);
		return *token;
	} else {
		return "PROBLEM";
	}
}



//////////////////////////////
//
// Tool_composite::getFullCompositeMarker --
//

string Tool_composite::getFullCompositeMarker(int line) {

	bool domark = needsCoincidenceMarker(line, true);
	if (domark) {
		return m_coinMark;
	}

	string Avalue = m_groups.at(0).at(line);
	string Bvalue = m_groups.at(1).at(line);

	if ((Avalue == ".") && (Bvalue == ".")) {
		return "";
	}

	// bool Arest = Avalue.find("r") != string::npos;
	// bool Brest = Bvalue.find("r") != string::npos;
	bool Anote = Avalue.find("R") != string::npos;
	bool Bnote = Bvalue.find("R") != string::npos;
	// bool Anull = Avalue == ".";
	// bool Bnull = Bvalue == ".";

	// deal with tied note sustains?
	if (Anote) {
		return m_AMark;
	}
	if (Bnote) {
		return m_BMark;
	}

	return "";
}



//////////////////////////////
//
// Tool_composite::needsCoincidenceMarker -- return composite marker if there
//     is a coincidence between two groups on the given line (from group analysis data).
//

bool Tool_composite::needsCoincidenceMarker(int line, bool forceQ) {
	string group1 = m_groups.at(0).at(line);
	string group2 = m_groups.at(1).at(line);

	if (!m_coinMarkQ) {
		if (!forceQ) {
			return false;
		}
	}

	// Coincidence if there are no ties or rests, or null tokens involved.
	bool domark = true;
	if (group1 == "") {
		domark = false;
	} else if (group2 == "") {
		domark = false;
	} else if (group1.find("r") != string::npos) {
		domark = false;
	} else if (group2.find("r") != string::npos) {
		domark = false;
	} else if (group1.find("_") != string::npos) {
		domark = false;
	} else if (group2.find("_") != string::npos) {
		domark = false;
	} else if (group1.find("]") != string::npos) {
		domark = false;
	} else if (group2.find("]") != string::npos) {
		domark = false;
	} else if (group1 == ".") {
		domark = false;
	} else if (group2 == ".") {
		domark = false;
	}

	return domark;
}



//////////////////////////////
//
// Tool_composite::getGroupCompositeToken --
//    group 0 == "A"
//    group 1 == "B"
//

string Tool_composite::getGroupCompositeToken(HumdrumFile& infile, int line, int group) {
	string tgroup = (group == 0) ? "A" : "B";

	if (infile[line].isData()) {
		if (m_groups.at(group).at(line) != "") {
			string output = m_groups.at(group).at(line);
			bool domark = needsCoincidenceMarker(line);
			if (domark) {
				output += m_coinMark;
			}
			return output;
		} else {
			return ".";
		}
	} else if (infile[line].isInterpretation()) {
		HTp token = infile.token(line, 0);
		if (*token == "*-") {
			return "*-";
		} else if (token->compare(0, 2, "**") == 0) {
			if (group == 0) {
				return "**kern-grpA";
			} else {
				return "**kern-grpB";
			}
		} else if (line == m_clefIndex) {
			return "*clefX";
		} else if (line == m_instrumentNameIndex) {
			if (group == 0) {
				string output = "*I\"Group A";
				if (m_eventQ) {
					m_groupAEventCount = getEventCount(m_groups.at(0));
					stringstream value;
					value.str("");
					value << "\\n(" << m_groupAEventCount << " event";
					if (m_groupAEventCount != 1) {
						value << "s";
					}
					value << ")";
					output += value.str();
				}
				return output;
			} else {
				string output = "*I\"Group B";
				if (m_eventQ) {
					m_groupBEventCount = getEventCount(m_groups.at(1));
					stringstream value;
					value.str("");
					value << "\\n(" << m_groupBEventCount << " event";
					if (m_groupBEventCount != 1) {
						value << "s";
					}
					value << ")";
					output += value.str();
				}
				return output;
			}
		} else if (line == m_instrumentAbbrIndex) {
			if (group == 0) {
				return "*I'Grp. A.";
			} else {
				return "*I'Grp. B.";
			}
		} else if (line == m_groupAssignmentIndex) {
			string grp = "*grp:";
			if (group == 0) {
				grp += "A";
			} else if (group == 1) {
				grp += "B";
			}
			return grp;
		} else if (line == m_timeSignatureIndex) {
			return getTimeSignature(infile, m_timeSignatureIndex, tgroup);
		} else if (line == m_meterSymbolIndex) {
			return getMetricSymbol(infile, m_meterSymbolIndex, tgroup);
		} else {
			return "*";
		}
	} else if (infile[line].isCommentLocal()) {
		return "!";
	} else if (infile[line].isBarline()) {
		HTp token = infile.token(line, 0);
		return *token;
	} else {
		return "PROBLEM";
	}

}



//////////////////////////////
//
// Tool_composite::getTimeSignature --
//

string Tool_composite::getTimeSignature(HumdrumFile& infile, int line, const string& group) {
	if (!infile[line].isInterpretation()) {
		return "*";
	}
	HTp lastToken = NULL;
	for (int j=0; j<infile[line].getFieldCount(); j++) {
		HTp token = infile.token(line, j);
		if (!token->isTimeSignature()) {
			continue;
		}
		lastToken = token;
		if (group != "") {
			string sgroup = token->getValue("auto", "group");
			if (sgroup != group) {
				continue;
			}
		}
		return *token;
	}
	if (lastToken) {
		return *lastToken;
	}
	return "*";
}



//////////////////////////////
//
// Tool_composite::getMetricSymbol --
//

string Tool_composite::getMetricSymbol(HumdrumFile& infile, int line, const string& group) {
	if (!infile[line].isInterpretation()) {
		return "*";
	}
	HTp lastToken = NULL;
	for (int j=0; j<infile[line].getFieldCount(); j++) {
		HTp token = infile.token(line, j);
		if (!token->isMetricSymbol()) {
			continue;
		}
		lastToken = token;
		if (group != "") {
			string sgroup = token->getValue("auto", "group");
			if (sgroup != group) {
				continue;
			}
		}
		return *token;
	}
	if (lastToken) {
		return *lastToken;
	}
	return "*";
}



//////////////////////////////
//
// Tool_composite::analyzeFullCompositeRhythm --
//

void Tool_composite::analyzeFullCompositeRhythm(HumdrumFile& infile) {

	vector<HumNum> durations(infile.getLineCount());
	for (int i=0; i<infile.getLineCount(); i++) {
		durations[i] = infile[i].getDuration();
	}

	vector<int> isRest(infile.getLineCount(), false);
	vector<int> isNull(infile.getLineCount(), false);
	vector<int> isSustain(infile.getLineCount(), false);

	for (int i=0; i<infile.getLineCount(); i++) {
		if (durations[i] == 0) {
			continue;
		}
		bool allnull = true;
		bool allrest = true;
		bool allsustain = true;
		for (int j=0; j<infile[i].getFieldCount(); j++) {
			HTp tok = infile.token(i, j);
			if (tok->isNull()) {
				continue;
			}
			allnull = false;
			if (!tok->isKern()) {
				continue;
			}
			if (tok->isNote()) {
				allrest = false;
				allnull = false;
				if ((tok->find("_") == string::npos) &&
				    (tok->find("]") == string::npos)) {
					allsustain = false;
				} else {
				// 	cerr << "NOTE THAT IS SUSTAIN: " << tok << endl;
				}
			} else {
				// cerr << "TOKEN IS NOT NOTE " << tok << endl;
			}
			if (tok->isRest()) {
				allnull = false;
			}
		}
		isRest[i]    = allrest    ? true : false;
		isNull[i]    = allnull    ? true : false;
		isSustain[i] = allsustain ? true : false;
	}

	string pstring = m_pitch;
	if (m_upstemQ) {
		pstring += "/";
	}

	HumRegex hre;
	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isData()) {
			continue;
		}

		if (infile[i].getDuration() == 0) {
			// Grace note data line (most likely)
			if (!m_graceQ) {
				continue;
			}
			// otherwise, borrow the view of the first grace note found on the line
			// (beaming, visual duration) and apply the target pitch to the grace note.
			for (int j=0; j<infile[i].getFieldCount(); j++) {
				HTp tok = infile.token(i, j);
				if (!tok->isKern()) {
					continue;
				}
				if (tok->isNull()) {
					continue;
				}
				if (tok->isGrace()) {
					string q;
					string beam;
					string recip;
					if (hre.search(tok, "(\\d+%?\\d*\\.*)")) {
						recip = hre.getMatch(1);
					}
					if (hre.search(tok, "([LJk]+)")) {
						beam = hre.getMatch(1);
					}
					if (hre.search(tok, "(q+)")) {
						q = hre.getMatch(1);
					}
					string full;
					full += recip;
					full += q;
					full += pstring;
					full += beam;
					m_fullComposite[i] = full;
					break;
				}
			}
			continue;
		}

		HumNum duration = getLineDuration(infile, i, isNull);
		string output;
		if (isNull[i]) {
			m_fullComposite[i] = ".";
			continue;
		} else {
			output = Convert::durationToRecip(duration);
		}

//		if (onlyAuxTremoloNotes(infile, i)) {
//			// mark auxiliary notes so that they can be merged
//			// with a preceding note later.
//			output += COMPOSITE_TREMOLO_MARKER;
//		}

		if (isRest[i]) {
			output += "r";
		} else {
			output += pstring;
			if (isSustain.at(i)) {
				output += "]";
			}
		}
		m_fullComposite[i] = output;
		// cerr << output << "\tS:" << isSustain[i] << "\tR:" << isRest[i] << "\tN:" << isNull[i] << endl;
	}


	fixTiedNotes(m_fullComposite, infile);

//	removeAuxTremolosFromCompositeRhythm(infile);

}



//////////////////////////////
//
// Tool_composite::fixTiedNotes --
//

void Tool_composite::fixTiedNotes(vector<string>& data, HumdrumFile& infile) {
	bool intie = false;
	HumRegex hre;
	for (int i=(int)data.size() - 1; i>=0; i--) {
		if (!infile[i].isData()) {
			continue;
		}
		if (infile[i].isGraceLine()) {
			// ignore grace-note lines
			continue;
		}
		if (data.at(i) == ".") {
			continue;
		}
		if (data.at(i) == "") {
			continue;
		}
		if (intie) {
			if (data.at(i).find("[") != string::npos) {
				intie = false;
			} else if (data.at(i).find("]") != string::npos) {
				hre.replaceDestructive(data.at(i), "_", "[]]");
			} else if (data.at(i).find("_") != string::npos) {
				// do nothing
			} else {
				data.at(i) = "[" + data.at(i);
				intie = false;
			}
		} else {
			if (data.at(i).find("]") != string::npos) {
				intie = true;
			} else if (data.at(i).find("_") != string::npos) {
				intie = true;
			}
		}
	}
}



//////////////////////////////
//
// Tool_composite::analyzeCoincidenceRhythms --
//

void Tool_composite::analyzeCoincidenceRhythms(HumdrumFile& infile) {
	if (!m_assignedQ) {
		assignGroups(infile);
	}
	vector<int> groupAstates;
	vector<int> groupBstates;

	// -2 = sustain tied note -1 = sustain, 0 = rest (or non-data), +1 = note attack
	getNumericGroupStates(groupAstates, infile, "A");
	getNumericGroupStates(groupBstates, infile, "B");

	vector<HumNum> timestamps(infile.getLineCount(), 0);
	for (int i=0; i<infile.getLineCount(); i++) {
		timestamps[i] = infile[i].getDurationFromStart();
	}

	vector<int> merged(infile.getLineCount(), 0);
	for (int i=0; i<infile.getLineCount(); i++) {
		if (groupAstates.at(i) == groupBstates.at(i)) {
			merged[i] = groupAstates.at(i);
		} else if (groupAstates[i] == 0 || (groupBstates[i] == 0)) {
			merged[i] = 0;
		} else if ((groupAstates[i] > 0) && (groupBstates[i] < 0)) {
			merged[i] = groupBstates[i];
		} else if ((groupAstates[i] < 0) && (groupBstates[i] > 0)) {
			merged[i] = groupAstates[i];
		} else {
			merged[i] = -1;
		}
	}

	bool foundFirstAttackA = false;
	bool foundFirstAttackB = false;

	m_coincidence.resize(infile.getLineCount());
	for (int i=0; i<infile.getLineCount(); i++) {
		m_coincidence[i] = "";
	}

	vector<int> noteAttack(infile.getLineCount(), 0);
	int currValue = -1000;
	int lastValue = -1000;
	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isData()) {
			continue;
		}
		lastValue = currValue;
		if (groupAstates[i] > 0) {
			foundFirstAttackA = true;
		}
		if (groupBstates[i] > 0) {
			foundFirstAttackB = true;
		}
		int value = merged[i];
		if (!foundFirstAttackA) {
			value = 0;
		}
		if (!foundFirstAttackB) {
			value = 0;
		}
		currValue = value;

		if (currValue == 1) {
			noteAttack[i] = 1;
		} else if (currValue == -2) {
			// sustained note attack
			noteAttack[i] = -2;
		} else if ((currValue == 0) && (lastValue != 0)) {
			noteAttack[i] = 2; // 2 means a "rest" note.
		}
	}

	// Need to split rests across barlines (at least non-invisible ones).
	// Also split rests if they generate an unprintable rhythm...

	vector<int> nextAttackIndex(infile.getLineCount(), -1);
	vector<int> prevAttackIndex(infile.getLineCount(), -1);
	int lasti = 0;
	for (int i=0; i<infile.getLineCount(); i++) {
		if (noteAttack[i] != 0) {
			prevAttackIndex[i] = lasti;
			lasti = i;
		}
	}
	int nexti = infile.getLineCount() - 1;
	for (int i=infile.getLineCount()-1; i>=0; i--) {
		if (noteAttack[i] != 0) {
			nextAttackIndex[i] = nexti;
			nexti = i;
		}
	}

	string lastnote = "";
	HumNum remainder;
	bool barline = false;
	for (int i=0; i<infile.getLineCount(); i++) {
		if (infile[i].isBarline()) {
			barline = true;
		}
		if (barline && infile[i].isData() && !infile[i].isGraceLine()) {
			barline = false;
			if (remainder > 0) {
				// add remainder rest at start of new barline
				HumNum durtobar = infile[i].getDurationToBarline();
				if (remainder > durtobar) {
					// Presuming a need for a rest at the start of the measure that
					// is an overflow from the previous measure.
					remainder = remainder - durtobar;
					string recip = Convert::durationToRecip(durtobar);
					string text = recip + "r";
					lastnote = text;
					m_coincidence[i] = text;
				} else {
					// Remainder rest stops by the next bar.
					string recip = Convert::durationToRecip(remainder);
					remainder = 0;
					string text = recip + "r";
					lastnote = text;
					m_coincidence[i] = text;
				}
			}
		}
		if (noteAttack[i]) {
			HumNum duration = infile[nextAttackIndex[i]].getDurationFromStart() -
				infile[i].getDurationFromStart();
			HumNum durtobar = infile[i].getDurationToBarline();
			if (duration > durtobar) {
				// clip the duration to the duration of the rest of the measure
				remainder = duration - durtobar;
				duration = durtobar;
			}

			string recip = Convert::durationToRecip(duration);
			string text;
			if (noteAttack[i] == -2) {
				text = recip + "eR";
				// this is either a middle tie if next note is also a tie
				// or is a tie end if the next note is an attack (1) or rest (2)
				if (noteAttack[nextAttackIndex[i]] > 0) {
					// tie end
					if ((lastnote.find("[") == string::npos) && (lastnote.find("_") == string::npos)) {
						m_coincidence[i] = recip + "r";
						// previous note is not a tie, so switch this note to a rest from
						// a sustained tied note

					} else {
						m_coincidence[i] = text;
						m_coincidence[i] += "]";
					}
					lastnote = m_coincidence[i];
				} else {
					// tie middle
					if ((lastnote.find("[") == string::npos) && (lastnote.find("_") == string::npos)) {
						m_coincidence[i] = text + "[";
					} else {
						m_coincidence[i] = text + "_";
					}
					lastnote = m_coincidence[i];
				}
			} else if (noteAttack[i] == 1) {
				text = recip + "eR";
				// Start a tie if the next note is a tied note
				if (noteAttack[nextAttackIndex[i]] == -2) {
					m_coincidence[i] = "[" + text;
					lastnote = m_coincidence[i];
				} else {
					m_coincidence[i] = text;
					lastnote = m_coincidence[i];
				}
			} else if (noteAttack[i] == 2) {
				// rest
				text = recip + "r";
				m_coincidence[i] = text;
				lastnote = m_coincidence[i];
			}
		}
	}

	// fixTiedNotes(m_coincidence, infile);

	if (m_debugQ) {
		cerr << "MERGED Coincidence states:" << endl;
		cerr << "TS\tA\tB\tMerged\tAttack\tIndex\tNext\tPrev\tCoin\tInput\n";
		for (int i=0; i<(int)merged.size(); i++) {
			cerr << timestamps[i] << "\t";
			cerr << groupAstates[i] << "\t" << groupBstates[i];
			cerr << "\t" << merged[i] << "\t" << noteAttack[i];
			cerr << "\t" << i;
			cerr << "\t" << nextAttackIndex[i] << "\t" << prevAttackIndex[i];
			cerr << "\t" << m_coincidence[i];
			cerr << "\t" << infile[i] << endl;
		}
		cerr << "==================================" << endl;
	}

}



//////////////////////////////
//
// Tool_composite::getNumericGroupStates -- return +1 if there is a note
//     attack in a given group, -1 if there is a sustained note in the group,
//     or 0 if there is a rest in the group.
//

void Tool_composite::getNumericGroupStates(vector<int>& states,
		HumdrumFile& infile, const string& tgroup) {
	states.resize(infile.getLineCount());
	fill(states.begin(), states.end(), 0);
	bool nullSustain = false;
	bool tieNote = false;
	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isData()) {
			continue;
		}
		vector<int> linestates;
		linestates.clear();
		for (int j=0; j<infile[i].getFieldCount(); j++) {
			HTp token = infile.token(i, j);
			if (!token->isKern()) {
				continue;
			}
			string group = token->getValue("auto", "group");
			if (group != tgroup) {
				// need to check if null tokens have groups
				continue;
			}
			nullSustain = false;
			if (token->isNull()) {
				nullSustain = true;
				token = token->resolveNull();
				if (!token) {
					continue;
				}
			}
			if (token->isRest()) {
				// resting is the default state.
				continue;
			}
			vector<string> subtoks = token->getSubtokens();
			bool suschord = true;
			bool attackchord = false;
			for (int k=0; k<(int)subtoks.size(); k++) {
				if (subtoks[k].find("r") != string::npos) {
					continue;
				}
				if (subtoks[k] == ".") {
					// Strange null subtoken.
					continue;
				}
				if ((subtoks[k].find("]") == string::npos) &&
						(subtoks[k].find("_") == string::npos)) {
					suschord = false;
					attackchord = true;
				}
				if (!nullSustain) {
					if ((subtoks[k].find("]") != string::npos) ||
						 	(subtoks[k].find("_") != string::npos)) {
						tieNote = true;
					} else {
						tieNote = false;
					}
				} else {
					tieNote = false;
				}
			} // end of chord analysis
			if (attackchord) {
				suschord = false;
			}
			if (suschord && nullSustain) {
				linestates.push_back(-1);
			} else if (nullSustain) {
				linestates.push_back(-1);
			} else if (suschord) {
				if (tieNote) {
					linestates.push_back(-2);
				} else {
					linestates.push_back(-1);
				}
			} else {
				linestates.push_back(1);
			}
		} // end of line analysis

		bool allZero = true;
		for (int m=0; m<(int)linestates.size(); m++) {
			if (linestates[m] != 0) {
				allZero = false;
				break;
			}
		}
		bool hasAttack = false;
		for (int m=0; m<(int)linestates.size(); m++) {
			if (linestates[m] > 0) {
				hasAttack = true;
				break;
			}
		}
		if (hasAttack) {
			states[i] = 1;
		} else if (allZero) {
			states[i] = 0;
		} else {
			states[i] = -1;
		}
	}
}



//////////////////////////////
//
// Tool_composite::analyzeGroupCompositeRhythms --
//

void Tool_composite::analyzeGroupCompositeRhythms(HumdrumFile& infile) {
	vector<HumNum> durations(infile.getLineCount());
	for (int i=0; i<infile.getLineCount(); i++) {
		durations[i] = infile[i].getDuration();
	}

	vector<int> isRest(infile.getLineCount(), false);
	vector<int> isNull(infile.getLineCount(), false);

	for (int i=0; i<infile.getLineCount(); i++) {
		if (durations[i] == 0) {
			continue;
		}
		bool allnull = true;
		bool allrest = true;
		for (int j=0; j<infile[i].getFieldCount(); j++) {
			HTp tok = infile.token(i, j);
			string value = tok->getValue("auto", "ignoreTremoloNote");
			if (value == "1") {
				continue;
			}
			if (tok->isNull()) {
				continue;
			}
			allnull = false;
			if (!tok->isKern()) {
				continue;
			}
			if (tok->isNote()) {
				allrest = false;
				break;
			}
			if (tok->isRest()) {
				allnull = false;
			}
		}
		isRest[i] = allrest ? true : false;
		isNull[i] = allnull ? true : false;
	}

	string pstring = m_pitch;
	if (m_upstemQ) {
		pstring += "/";
	}

	HumRegex hre;

	if (!m_assignedQ) {
		assignGroups(infile);
	}
	analyzeLineGroups(infile);
	if (m_debugQ) {
		printGroupAssignments(infile);
	}

	vector<vector<int>> groupstates;
	getGroupStates(groupstates, infile);

	vector<vector<HumNum>> groupdurs;
	getGroupDurations(groupdurs, groupstates, infile);

	vector<vector<string>> rhythms;
	getGroupRhythms(rhythms, groupdurs, groupstates, infile);

	string curtimesigA;
	string curtimesigB;

	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isData()) {
			continue;
		}
		if (infile[i].getDuration() == 0) {
			// Grace note data line (most likely)
			if (!m_graceQ) {
				continue;
			}
			// otherwise, borrow the view of the first grace note found on the line
			// (beaming, visual duration) and apply the target pitch to the grace note.
			for (int j=0; j<infile[i].getFieldCount(); j++) {
				HTp tok = infile.token(i, j);
				if (!tok->isKern()) {
					continue;
				}
				if (tok->isNull()) {
					continue;
				}
				if (tok->isGrace()) {
					string q;
					string beam;
					string recip;
					if (hre.search(tok, "(\\d+%?\\d*\\.*)")) {
						recip = hre.getMatch(1);
					}
					if (hre.search(tok, "([LJk]+)")) {
						beam = hre.getMatch(1);
					}
					if (hre.search(tok, "(q+)")) {
						q = hre.getMatch(1);
					}
					string full;
					full += recip;
					full += q;
					full += pstring;
					full += beam;
					// HTp targettok = NULL;
					// HTp targettok2 = NULL;
					//if (m_appendQ) {
					//	targettok = infile.token(i, infile[i].getFieldCount()-2);
					//	targettok2 = infile.token(i, infile[i].getFieldCount()-1);
					//} else {
					//	if (m_coincidenceQ) {
					//		targettok = infile.token(i, 1);
					//		targettok2 = infile.token(i, 2);
					//	} else {
					//		targettok = infile.token(i, 0);
					//		targettok2 = infile.token(i, 1);
					//	}
					//}

					string group = infile.token(i, j)->getValue("auto", "group");
					if (group == "A") {
						m_groups[0][i] = full;
					} else if (group == "B") {
						m_groups[1][i] = full;
					}
					break;
				}
			}
			continue;
		}

		// dealing with a non-zero data line:
		HumNum duration = getLineDuration(infile, i, isNull);
		string recip = rhythms[0][i];
		string recip2 = rhythms[1][i];
		if (recip.empty()) {
			recip = ".";
		} else {
			if (groupstates[0][i] == TYPE_RestAttack) {
				recip += "rR";
			}
			else if (groupstates[0][i] == TYPE_UNDEFINED) {
				// make invisible rest (rest not part of group)
				recip += "ryy";
			} else {
				recip += pstring;
			}
		}
		if (recip2.empty()) {
			// null group: add invisible rest to rhythm
			// HumNum linedur = infile[i].getDuration();
			// recip2 = Convert::durationToRecip(linedur);
			recip2 += ".";
		} else {
			if (groupstates[1][i] == TYPE_RestAttack) {
				recip2 += "rR";
			}
			else if (groupstates[1][i] == TYPE_UNDEFINED) {
				// make invisible rest (rest not part of group)
				recip2 += "ryy";
			} else {
				recip2 += pstring;
			}
		}
		m_groups[0][i] = recip;
		m_groups[1][i] = recip2;
	}

//	if (!m_together.empty()) {
//		if (m_appendQ) {
//			markTogether(infile, -2);
//		} else {
//			markTogether(infile, 2);
//		}
//	}
}



//////////////////////////////
//
// Tool_compare2::getGrouprhythms --
//

void Tool_composite::getGroupRhythms(vector<vector<string>>& rhythms,
		vector<vector<HumNum>>& groupdurs, vector<vector<int>>& groupstates,
		HumdrumFile& infile) {
	rhythms.resize(groupdurs.size());
	for (int i=0; i<(int)rhythms.size(); i++) {
		getGroupRhythms(rhythms[i], groupdurs[i], groupstates[i], infile);
	}
}

void Tool_composite::getGroupRhythms(vector<string>& rhythms, vector<HumNum>& durs,
		vector<int>& states, HumdrumFile& infile) {
	rhythms.clear();
	rhythms.resize(durs.size());
	int lastnotei = -1;
	for (int i=0; i<(int)rhythms.size(); i++) {
		if (states[i] <= 0) {
			continue;
		}
		string prefix = "";
		string postfix = "";
		for (int j=i+1; j<(int)rhythms.size(); j++) {
			if ((states[j]) > 0 && (states[j] < 5)) {
				if ((states[i] == TYPE_NoteAttack) && (states[j] == TYPE_NoteSustainAttack)) {
					prefix = "[";
				} else if ((states[i] == TYPE_NoteSustainAttack) && (states[j] == TYPE_NoteSustainAttack)) {
					postfix = "_";
				} else if ((states[i] == TYPE_NoteSustainAttack) && (states[j] == TYPE_NoteAttack)) {
					postfix = "]";
				} else if ((states[i] == TYPE_NoteSustainAttack) && (states[j] == TYPE_RestAttack)) {
					postfix = "]";
				}
				lastnotei = j;
				break;
			}
		}
		string value = Convert::durationToRecip(durs[i]);
		rhythms[i] = prefix + value + postfix;
	}
	if (lastnotei >= 0) {
		if (states[lastnotei] == TYPE_NoteSustainAttack) {
			rhythms[lastnotei] = rhythms[lastnotei] + "]";
		}
	}

	if (m_debugQ) {
		cerr << "=========================================" << endl;
		cerr << "RECIP FOR GROUP: " << endl;
		for (int i=0; i<(int)rhythms.size(); i++) {
			cerr << rhythms[i] << "\t" << durs[i] << "\t" << states[i] << "\t" << infile[i] << endl;
		}
		cerr << "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^" << endl;
	}
}



//////////////////////////////
//
// Tool_composite::getGroupDurations --
//

void Tool_composite::getGroupDurations(vector<vector<HumNum>>& groupdurs,
		vector<vector<int>>& groupstates, HumdrumFile& infile) {
	groupdurs.resize(groupstates.size());
	for (int i=0; i<(int)groupstates.size(); i++) {
		getGroupDurations(groupdurs[i], groupstates[i], infile);
	}
}

void Tool_composite::getGroupDurations(vector<HumNum>& groupdurs,
		vector<int>& groupstates, HumdrumFile& infile) {
	HumNum enddur = infile.getScoreDuration();
	groupdurs.resize(groupstates.size());
	fill(groupdurs.begin(), groupdurs.end(), -1);
	int eventi = -1;
	HumNum lasttime = 0;
	for (int i=0; i<(int)groupdurs.size(); i++) {
		if (groupstates[i] > 0) {
			if (eventi >= 0) {
				HumNum eventtime = infile[i].getDurationFromStart();
				HumNum duration = eventtime - lasttime;
				groupdurs[eventi] = duration;
				lasttime = eventtime;
				eventi = i;
				continue;
			} else {
				eventi = i;
			}
		}
	}
	if (eventi >= 0) {
		HumNum duration = enddur - lasttime;
		groupdurs[eventi] = duration;
	}
}



/////////////////////////////
//
// Tool_composite::getGroupStates -- Pull out the group note states for each
//    composite rhytm stream.
//
//    group:A:type = "note"   if there is at least one note attack in group A on the line.
//    group:A:type = "ncont"  if there is no attack but at least one note sustain in group A.
//    group:A:type = "snote"  there is a printed note which is part of a tie group sustain note.
//    group:A:type = "scont"  continuation of a tie group sutain note.
//    group:A:type = "rest"   if there is no note attack or sustain but there is a rest start.
//    group:A:type = "rcont"  if there is a rest continuing in group A on the line.
//    group:A:type = "none"   if there is no activity for group A on the line.
//
//    Numeric equivalents:
//     9 = TYPE_UNDEFINED           = "undefined"
//     3 = TYPE_NoteSustainAttack   = "snote"
//     2 = TYPE_NoteAttack          = "note"
//     1 = TYPE_RestAttack          = "rest"
//     0 = TYPE_NONE                = "none"
//    -1 = TYPE_RestSustain         = "rcont"
//    -2 = TYPE_NoteSustain         = "ncont"
//    -3 = TYPE_NoteSustainSustain  = "scont"
//

void Tool_composite::getGroupStates(vector<vector<int>>& groupstates, HumdrumFile& infile) {
	groupstates.resize(2);
	groupstates[0].resize(infile.getLineCount());
	groupstates[1].resize(infile.getLineCount());
	fill(groupstates[0].begin(), groupstates[0].end(), 0);
	fill(groupstates[1].begin(), groupstates[1].end(), 0);

	for (int i=0; i<infile.getLineCount(); i++) {
		for (int j=0; j<(int)groupstates.size(); j++) {
			char groupname = 'A' + j;
			string name;
			name.clear();
			name += groupname;
			string state = infile[i].getValue("group", name, "type");
			int typenum = typeStringToInt(state);
			groupstates[j][i] = typenum;
		}
	}
}



//////////////////////////////
//
// Tool_composite::getLineDuration -- Return the duration of the line, but return
//    0 if the line only contains nulls.  Also add the duration of any subsequent
//    lines that are null lines before any data content lines.

HumNum Tool_composite::getLineDuration(HumdrumFile& infile, int index, vector<int>& isNull) {
	if (isNull[index]) {
		return 0;
	}
	if (!infile[index].isData()) {
		return 0;
	}
	HumNum output = infile[index].getDuration();
	for (int i=index+1; i<infile.getLineCount(); i++) {
		if (!infile[i].isData()) {
			continue;
		}
		if (isNull[i]) {
			output += infile[i].getDuration();
		} else {
			break;
		}
	}
	return output;
}



//////////////////////////////
//
// Tool_composite::assignGroups -- Add a parameter
//   auto:grouping = "A" or "B" depending on the group.  This
//   can be generalized later to more letters, or arbitrary
//   strings perhaps.  This comes from an interpretation such
//   as *grp:A or *grp:B in the data.  If *grp: is found without
//   a letter, than that group will be null group.


void Tool_composite::assignGroups(HumdrumFile& infile) {
	m_assignedGroups = true;

	int maxtrack = infile.getMaxTrack();
	vector<vector<string>> curgroup;
	curgroup.resize(maxtrack + 1);
	for (int i=0; i<(int)curgroup.size(); i++) {
		curgroup[i].resize(100);
	}

	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].hasSpines()) {
			continue;
		}
		for (int j=0; j<infile[i].getFieldCount(); j++) {
			// checking all spines (not just **kern data).
			HTp token = infile.token(i, j);
			int track = token->getTrack();
			int subtrack = token->getSubtrack();
			if (subtrack > 99) {
				cerr << "Too many subspines!" << endl;
				continue;
			}

			if (*token == "*grp:A") {
				curgroup.at(track).at(subtrack) = "A";
				if (subtrack == 0) {
					for (int k=1; k<(int)curgroup.at(track).size(); k++) {
						curgroup.at(track).at(k) = "A";
					}
				}
				backfillGroup(curgroup, infile, i, track, subtrack, "A");
			}
			if (*token == "*grp:B") {
				curgroup.at(track).at(subtrack) = "B";
				if (subtrack == 0) {
					for (int k=1; k<(int)curgroup.at(track).size(); k++) {
						curgroup.at(track).at(k) = "B";
					}
				}
				backfillGroup(curgroup, infile, i, track, subtrack, "B");
			}
			// could expanded to other groups here
			if (*token == "*grp:") {
				// clear a group:
				curgroup.at(track).at(subtrack) = "";
				if (subtrack == 0) {
					for (int k=1; k<(int)curgroup.at(track).size(); k++) {
						curgroup.at(track).at(k) = "";
					}
				}
				backfillGroup(curgroup, infile, i, track, subtrack, "");
			}

			string group = curgroup.at(track).at(subtrack);
			token->setValue("auto", "group", group);
		}
	}
	m_assignedQ = true;
}



//////////////////////////////
//
// Tool_composite::backfillGroup -- Go back and reassign a group to all lines
//   before *grp:A or *grp:B so that time signatures and the like are used as
//   desired even if they come before a new group definition.
//

void Tool_composite::backfillGroup(vector<vector<string>>& curgroup, HumdrumFile& infile,
		int line, int track, int subtrack, const string& group) {
	int lastline = -1;
	for (int i=line-1; i>=0; i--) {
		if (infile[i].isData()) {
			lastline = i+1;
			break;
		}
		curgroup.at(track).at(subtrack) = group;
		if (subtrack == 0) {
			for (int k=1; k<(int)curgroup.at(track).size(); k++) {
				curgroup.at(track).at(k) = group;
			}
		}
	}
	if (lastline < 0) {
		lastline = 0;
	}
	for (int i=lastline; i<line; i++) {
		if (infile[i].isData()) {
			break;
		}
		for (int j=0; j<infile[i].getFieldCount(); j++) {
			HTp token = infile.token(i, j);
			int ttrack = token->getTrack();
			if (ttrack != track) {
				continue;
			}
			int tsubtrack = token->getSubtrack();
			if (tsubtrack != subtrack) {
				continue;
			}
			string group = curgroup.at(track).at(subtrack);
			token->setValue("auto", "group", group);
		}
	}
}



//////////////////////////////
//
// Tool_composite::analyzeLineGroups -- Look at each line for Group A and B and determine if
//    And one of five activity types are possible for the line:
//        group:A:type = "note"   if there is at least one note attack in group A on the line.
//        group:A:type = "ncont"  if there is no attack but at least one note sustain in group A.
//        group:A:type = "rest"   if there is no note attack or sustain but there is a rest start.
//        group:A:type = "rcont"  if there is a rest continuing in group A on the line.
//        group:A:type = "empty"  if there is no activity for group A on the line.
//

void Tool_composite::analyzeLineGroups(HumdrumFile& infile) {
	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].hasSpines()) {
			continue;
		}
		if (!infile[i].data()) {
			continue;
		}
		analyzeLineGroup(infile, i, "A");
		analyzeLineGroup(infile, i, "B");
	}
}



//////////////////////////////
//
// Tool_composite::analyzeLineGroup --
//
//     9 = TYPE_UNDEFINED           = "undefined"
//     3 = TYPE_NoteSustainAttack   = "snote"
//     2 = TYPE_NoteAttack          = "note"
//     1 = TYPE_RestAttack          = "rest"
//     0 = TYPE_NONE                = "none"
//    -1 = TYPE_RestSustain         = "rcont"
//    -2 = TYPE_NoteSustain         = "ncont"
//    -3 = TYPE_NoteSustainSustain  = "scont"
//

void Tool_composite::analyzeLineGroup(HumdrumFile& infile, int line, const string& target) {
	int groupstate = getGroupNoteType(infile, line, target);
	switch (groupstate) {
		case TYPE_NoteSustainAttack:
			infile[line].setValue("group", target, "type", "snote");
			break;
		case TYPE_NoteAttack:
			infile[line].setValue("group", target, "type", "note");
			break;
		case TYPE_RestAttack:
			infile[line].setValue("group", target, "type", "rest");
			break;
		case TYPE_RestSustain:
			infile[line].setValue("group", target, "type", "rcont");
			break;
		case TYPE_NoteSustain:
			infile[line].setValue("group", target, "type", "ncont");
			break;
		case TYPE_NoteSustainSustain:
			infile[line].setValue("group", target, "type", "scont");
			break;
		case TYPE_NONE:
			infile[line].setValue("group", target, "type", "none");
			break;
		default:
			infile[line].setValue("group", target, "type", "undefined");
			break;
	}
}



//////////////////////////////
//
// Tool_composite::extractGroup --
//

void Tool_composite::extractGroup(HumdrumFile& infile, const string &target) {
	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isData()) {
			m_humdrum_text << infile[i] << endl;
			continue;
		}
		for (int j=0; j<infile[i].getFieldCount(); j++) {
			HTp token = infile[i].token(j);
			if ((!token->isData()) || token->isNull()) {
				m_humdrum_text << token;
				if (j < infile[i].getFieldCount() - 1) {
					m_humdrum_text << "\t";
				}
				continue;
			}
			string group = token->getValue("auto", "group");
			if (group == target) {
				m_humdrum_text << token;
			} else {
				if (token->isRest()) {
					m_humdrum_text << token << "yy";
				} else {
					HumRegex hre;
					string rhythm = "4";
					if (hre.search(token, "(\\d+%?\\d*\\.*)")) {
						rhythm = hre.getMatch(1);
					}
					m_humdrum_text << rhythm << "ryy";
				}
			}
			if (j < infile[i].getFieldCount() - 1) {
				m_humdrum_text << "\t";
			}
		}
		m_humdrum_text << endl;
	}
}



//////////////////////////////
//
// Tool_composite::getGroupNoteType --
//
//  9 = TYPE_UNDEFINED
//  3 = TYPE_NoteSustainAttack
//  2 = TYPE_NoteAttack
//  1 = TYPE_RestAttack
//  0 = TYPE_NONE
// -1 = TYPE_RestSustain
// -2 = TYPE_NoteSustain
// -3 = TYPE_NoteSustainSustain
//

int Tool_composite::getGroupNoteType(HumdrumFile& infile, int line, const string& group) {
	if (!infile[line].isData()) {
		return TYPE_NONE;
	}

	vector<HTp> grouptokens;
	for (int i=0; i<infile[line].getFieldCount(); i++) {
		HTp token = infile.token(line, i);
		if (!token->isKern()) {
			continue;
		}
		string tgroup = token->getValue("auto", "group");
		if (group == tgroup) {
			grouptokens.push_back(token);
		}
	}

	if (grouptokens.empty()) {
		return TYPE_UNDEFINED;
	}

	bool hasRestAttack    = false;
	bool hasRestSustain   = false;
	bool hasNoteAttack    = false;
	bool hasNoteSustain   = false;
	bool hasNoteSAttack   = false;
	bool hasNoteSSustain  = false;

	for (int i=0; i<(int)grouptokens.size(); i++) {
		HTp token = grouptokens[i];
		string value = token->getValue("auto", "ignoreTremoloNote");
		if (value == "1") {
			hasNoteSustain = true;
			// need to check for tie on head note...
			continue;
		}
		if (token->isNull()) {
			HTp resolved = token->resolveNull();
			if (resolved && !resolved->isNull()) {
				if (resolved->isRest()) {
					hasRestSustain = true;
				} else {
					if (resolved->isNoteAttack()) {
						hasNoteSustain = true;
					} else if (resolved->isNoteSustain()) {
						hasNoteSSustain = true;
					}
				}
			}
			continue;
		}
		if (token->isRest()) {
			hasRestAttack = true;
			continue;
		}
		if (token->isNoteAttack()) {
			string value = token->getValue("auto", "ignoreTremoloNote");
			if (value != "1") {
				hasNoteAttack = true;
			}
			continue;
		}
		if (token->isNoteSustain()) {
			hasNoteSAttack = true;
		}
	}

	//  3 = TYPE_NoteSustainAttack
	//  2 = TYPE_NoteAttack
	//  1 = TYPE_RestAttack
	//  0 = TYPE_NONE
	// -1 = TYPE_RestSustain
	// -2 = TYPE_NoteSustain
	// -3 = TYPE_NoteSustainSustain

	if (hasNoteAttack) {
		return TYPE_NoteAttack;
	}
	if (hasNoteSAttack) {
		return TYPE_NoteSustainAttack;
	}
	if (hasNoteSustain) {
		return TYPE_NoteSustain;
	}
	if (hasNoteSSustain) {
		return TYPE_NoteSustainSustain;
	}
	if (hasRestAttack) {
		return TYPE_RestAttack;
	}
	if (hasRestSustain) {
		return TYPE_RestSustain;
	}

	cerr << "Warning: no category for line " << infile[line] << endl;

	return 0;
}



//////////////////////////////
//
// Tool_composite::printGroupAssignments -- for debugging of group assignments.
//

void Tool_composite::printGroupAssignments(HumdrumFile& infile) {
	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].hasSpines()) {
			cerr << infile[i] << endl;
			continue;
		}
		for (int j=0; j<infile[i].getFieldCount(); j++) {
			HTp token = infile.token(i, j);
			string value = token->getValue("auto", "group");
			cerr << token;
			if (!value.empty()) {
				cerr << "{" << value << "}";
			}
			if (j < infile[i].getFieldCount() - 1) {
				cerr << "\t";
			}
		}
		cerr << endl;
	}
}



//////////////////////////////
//
// Tool_composite::typeStringToInt -- Convert between numeric and string state forms.
//

int Tool_composite::typeStringToInt(const string& value) {
	if (value == "snote") { return TYPE_NoteSustainAttack;  }
	if (value == "note")  { return TYPE_NoteAttack;         }
	if (value == "rest")  { return TYPE_RestAttack;         }
	if (value == "none")  { return TYPE_NONE;               }
	if (value == "rcont") { return TYPE_RestSustain;        }
	if (value == "ncont") { return TYPE_NoteSustain;        }
	if (value == "scont") { return TYPE_NoteSustainSustain; }
	return TYPE_UNDEFINED;
}


//////////////////////////////
//
// Tool_composite::doNumericAnalyses --
//

void Tool_composite::doNumericAnalyses(HumdrumFile& infile) {
	if (m_analysisOnsetsQ) {
		doOnsetAnalyses(infile);
	}
	if (m_analysisAccentsQ) {
		doAccentAnalyses(infile);
	}
	if (m_analysisOrnamentsQ) {
		doOrnamentAnalyses(infile);
	}
	if (m_analysisAccentsQ) {
		doSlurAnalyses(infile);
	}
	if (m_analysisTotalQ) {
		doTotalAnalyses(infile);
	}
}



//////////////////////////////
//
// Tool_composite::doOnsetAnalyses -- targetGroup == "" means full composite onsets.
//     First index of m_analyses is rhythm type, and second index is analysis type,
//     with "0" being onsets.
//

void Tool_composite::doOnsetAnalyses(HumdrumFile& infile) {
	if (m_analyses.at(m_COMPOSITE_FULL).at(m_ONSET).size() > 0) {
		doOnsetAnalysis(m_analyses.at(m_COMPOSITE_FULL).at(m_ONSET), infile, "");
	}
	if (m_analyses.at(m_COMPOSITE_A).at(m_ONSET).size() > 0) {
		doOnsetAnalysis(m_analyses.at(m_COMPOSITE_A).at(m_ONSET), infile, "A");
	}
	if (m_analyses.at(m_COMPOSITE_A).at(m_ONSET).size() > 0) {
		doOnsetAnalysis(m_analyses.at(m_COMPOSITE_B).at(m_ONSET), infile, "B");
	}

	// Coincidence onset analysis must come after other onset analyses since it
	// relies on group A + group B.
	if (m_analyses.at(m_COINCIDENCE).at(m_ONSET).size() > 0) {
		doOnsetAnalysisCoincidence(m_analyses.at(m_COINCIDENCE).at(m_ONSET),
		                           m_analyses.at(m_COMPOSITE_A).at(m_ONSET),
		                           m_analyses.at(m_COMPOSITE_B).at(m_ONSET));
	}
}



//////////////////////////////
//
// Tool_composite::doOnsetAnalysisCoincidence -- Note onset analysis for coincidence analysis.
//

void Tool_composite::doOnsetAnalysisCoincidence(vector<double>& output,
		vector<double>& inputA, vector<double>& inputB) {
	fill(output.begin(), output.end(), 0);
	for (int i=0; i<(int)inputA.size(); i++) {
		if ((inputA[i] > 0) && (inputB[i] > 0)) {
			output[i] = inputA[i] + inputB[i];
		}
	}
}



//////////////////////////////
//
// Tool_composite::doOnsetAnalysis -- Count all of the notes on each line
//   (that are in **kern spines, ignoring other kern-like spines).
//

void Tool_composite::doOnsetAnalysis(vector<double>& analysis, HumdrumFile& infile,
		const string& targetGroup) {
	// analysis should already be sized to the number of lines in infile,
	// and initialized to zero.

	bool fullQ = (targetGroup == "") ? true : false;
	string group;

	for (int i=0; i<(int)infile.getLineCount(); i++) {
		if (!infile[i].isData()) {
			continue;
		}
		int csum = 0;
		for (int j=0; j<infile[i].getFieldCount(); j++) {
			HTp token = infile.token(i, j);
			if (!token->isKern()) {
				continue;
			}
			if (!fullQ) {
				string group = token->getValue("auto", "group");
				if (group != targetGroup) {
					continue;
				}
			}
			csum += countNoteOnsets(token);
		}
		analysis.at(i) = csum;
	}
}



//////////////////////////////
//
// Tool_composite::countNoteOnsets --
//

int Tool_composite::countNoteOnsets(HTp token) {
   vector<string> subtoks;
   subtoks = token->getSubtokens();
   int sum = 0;
   if (*token == ".") {
      return sum;
   }
   for (int i=0; i<(int)subtoks.size(); i++) {
      if (subtoks[i].find('r') != string::npos) {
         continue;
      }
      if (subtoks[i].find('_') != string::npos) {
         continue;
      }
      if (subtoks[i].find(']') != string::npos) {
         continue;
      }
      sum++;
   }
   return sum;
}



//////////////////////////////
//
// Tool_composite::doAccentAnalyses --
//

void Tool_composite::doAccentAnalyses(HumdrumFile& infile) {
	cerr << "DOING ACCENT ANALYSES" << endl;
}



//////////////////////////////
//
// Tool_composite::doOrnamentAnalyses --
//

void Tool_composite::doOrnamentAnalyses(HumdrumFile& infile) {
	cerr << "DOING ORNAMENT ANALYSES" << endl;
}



//////////////////////////////
//
// Tool_composite::doSlurAnalyses --
//

void Tool_composite::doSlurAnalyses(HumdrumFile& infile) {
	cerr << "DOING SLUR ANALYSES" << endl;
}



//////////////////////////////
//
// Tool_composite::doTotalAnalyses -- The totals analysis is the last
//    index in the m_analysis array.  It is the sum of all other analyses
//    (provided that they are non-negative).
//

void Tool_composite::doTotalAnalyses(HumdrumFile& infile) {
	if (m_numericAnalysisSpineCount <= 1) {
		// nothing to do.
		return;
	}

	// first index is rhythm analysis type
	// second index is numeric analysis type
	// third index is row in data (if analysis type is active)

	for (int i=0; i<(int)m_analyses.size(); i++) {
		for (int k=0; k<infile.getLineCount(); k++) {
			double sum = 0.0;
			for (int j=0; j<(int)m_analyses.at(i).size()-1; j++) {
				if (k < (int)m_analyses.at(i).at(j).size()) {
					sum += m_analyses.at(i).at(j).at(k);
				}
			}
			m_analyses.at(i).back().at(k) = sum;
		}
	}
}



//////////////////////////////
//
// Tool_composite::analyzeOutputVariables --
//

void Tool_composite::analyzeOutputVariables(HumdrumFile& infile) {

	m_instrumentNameIndex  = 0;
	m_instrumentAbbrIndex  = 0;
	m_timeSignatureIndex   = 0;
	m_meterSymbolIndex     = 0;
	m_groupAssignmentIndex = 0;
	m_firstDataIndex       = 0;
	m_verseLabelIndex      = 0;
	m_striaIndex           = 0;  // just before clef line in score
	m_sizeIndex            = 0;
	m_clefIndex            = 0;

	int barlineIndex = 0;
	int lastInterpBeforeBarline = 0;

	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].hasSpines()) {
			continue;
		}
		if (infile[i].isData()) {
			m_firstDataIndex = i;
			break;
		}
		if (infile[i].isBarline()) {
			barlineIndex = i;
		}
		if (!infile[i].isInterpretation()) {
			continue;
		}
		if (infile[i].isManipulator()) {
			continue;
		}
		if (!barlineIndex) {
			lastInterpBeforeBarline = i;
		}
		for (int j=0; j<infile[i].getFieldCount(); j++) {
			HTp token = infile.token(i, j);
			if (!token->isKernLike()) {
				continue;
			}
			if (token->isClef()) {
				m_clefIndex = i;
			}
			if (token->isInstrumentName()) {
				m_instrumentNameIndex = i;
			}
			if (token->isInstrumentAbbreviation()) {
				m_instrumentAbbrIndex = i;
			}
			if (token->isTimeSignature()) {
				m_timeSignatureIndex = i;
			}
			if (token->isMeterSymbol()) {
				m_meterSymbolIndex = i;
			}
			if (token->isStria()) {
				m_striaIndex = i;
			}
			if (token->compare(0, 6, "*size:") == 0) {
				m_sizeIndex = i;
			}
			if (token->compare(0, 5, "*grp:") == 0) {
				if (m_groupAssignmentIndex <= 0) {
					m_groupAssignmentIndex = i;
				}
			}
			if (token->compare(0, 4, "*v:") == 0) {
				if (!barlineIndex) {
					// do not add after a barline (but that would probably be OK).
					if (m_verseLabelIndex <= 0) {
						m_verseLabelIndex = i;
					}
				}
			}
		}
	}

	// If any variable is 0, then decide on a location to insert.  Maybe
	// store as a negative value to indicate that the line needs to be added.

	if (!m_verseLabelIndex) {
		// In rare cases there could be a LO parameter for the last interpretation
		// line (such as a layout parameter for a clef).  Currently ignore this problem.
		if (lastInterpBeforeBarline > 0) {
			m_verseLabelIndex = -lastInterpBeforeBarline;
		}
	}

	if (!m_striaIndex) {
		// In rare cases there could be a LO parameter for the last interpretation
		// line (such as a layout parameter for a clef).  Currently ignore this problem.
		if (m_clefIndex > 0) {
			m_striaIndex = -m_clefIndex;
		}
	}

	if (!m_sizeIndex) {
		// In rare cases there could be a LO parameter for the last interpretation
		// line (such as a layout parameter for a clef).  Currently ignore this problem.
		if (m_clefIndex > 0) {
			m_sizeIndex = -m_clefIndex;
		}
	}

}



//////////////////////////////
//
// Tool_composite::addStaffInfo -- Find staff number line and add staff line to output
//     if found.
//

void Tool_composite::addStaffInfo(HumdrumFile& output, HumdrumFile& infile) {
	int staffindex = -1;
	HumRegex hre;
	int lastStaff = -1;
	for (int i=0; i<infile.getLineCount(); i++) {
		if (infile[i].isData()) {
			break;
		}
		if (!infile[i].isInterpretation()) {
			continue;
		}
		for (int j=0; j<infile[i].getFieldCount(); j++) {
			HTp token = infile.token(i, j);
			if (!token->isKernLike()) {
				continue;
			}
			if (hre.search(token, "^\\*staff(\\d+)")) {
				lastStaff = hre.getMatchInt(1);
				staffindex = i;
				break;
			}
		}

		if (staffindex > 0) {
			break;
		}
	}
	if (staffindex < 0) {
		return;
	}
	if (lastStaff < 0) {
		return;
	}

	int currentStaff = lastStaff;
	for (int j=output[staffindex].getFieldCount() - 1; j>=0; j--) {
		HTp token = output.token(staffindex, j);
		string staffnum = "*staff" + to_string(++currentStaff);
		token->setText(staffnum);
	}

	output[staffindex].createLineFromTokens();

	int beginStaff = lastStaff + 1;
	int endStaff = lastStaff + output[staffindex].getFieldCount();
	int staffcount = output[staffindex].getFieldCount();
	string decoadd;
	if (staffcount > 1) {
		decoadd = "[(";
		for (int i=beginStaff; i<=endStaff; i++) {
			decoadd += "s" + to_string(i);
		}
		decoadd += ")]";
	} else {
		decoadd = "s" + to_string(beginStaff);
	}

	for (int i=output.getLineCount() - 1; i>=0; i--) {
		if (!output[i].isGlobalReference()) {
			continue;
		}
		HTp token = output.token(i, 0);
		if (hre.search(token, "!!!system-decoration:(\\s*)(.*)(\\s*)$")) {
			string prevalue = hre.getMatch(1);
			string original = hre.getMatch(2);
			string postvalue = hre.getMatch(3);
			string text = "!!!system-decoration:"+ prevalue + original + decoadd + postvalue;
			token->setText(text);
			output[i].createLineFromTokens();
			break;
		}
	}

}



///////////////////////////////
//
// Tool_composite::addTimeSignatureChanges --
//

void Tool_composite::addTimeSignatureChanges(HumdrumFile& output, HumdrumFile& infile) {
	string timesig;
	string groupAsig;
	string groupBsig;

	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isInterpretation()) {
			continue;
		}
		timesig    = "*";
		groupAsig  = "*";
		groupBsig  = "*";

		bool foundtime = false;

		for (int j=0; j<infile[i].getFieldCount(); j++) {
			HTp token = infile.token(i, j);
			if (token->isTimeSignature()) {
				string group = token->getValue("auto", "group");
				if (group == "") {
					timesig = *token;
					foundtime = true;
				} else if (group == "A") {
					groupAsig = *token;
					foundtime = true;
				} else if (group == "B") {
					groupBsig = *token;
					foundtime = true;
				} else {
					timesig = *token;
					foundtime = true;
				}
			}
		}

		if (!foundtime) {
			continue;
		}

		for (int j=0; j<output[i].getFieldCount(); j++) {
			HTp token = output.token(i, j);
			string spinetype = token->getDataType();
			if ((spinetype == "**kern-coin") || (spinetype == "**kern-comp")) {
				if (timesig != "") {
					token->setText(timesig);
				} else if (groupAsig != "") {
					token->setText(groupAsig);
				} else if (groupBsig != "") {
					token->setText(groupAsig);
				}
			} else if (spinetype == "**kern-grpA") {
				if (groupAsig != "") {
					token->setText(groupAsig);
				}
			} else if (spinetype == "**kern-grpB") {
				if (groupBsig != "") {
					token->setText(groupBsig);
				}
			}
		}
		output[i].createLineFromTokens();

	}
}



///////////////////////////////
//
// Tool_composite::addMeterSignatureChanges --
//

void Tool_composite::addMeterSignatureChanges(HumdrumFile& output, HumdrumFile& infile) {
	string metersig;
	string groupAsig;
	string groupBsig;

	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isInterpretation()) {
			continue;
		}
		metersig   = "";
		groupAsig  = "";
		groupBsig  = "";

		bool foundtime = false;

		for (int j=0; j<infile[i].getFieldCount(); j++) {
			HTp token = infile.token(i, j);
			if (token->isMetricSymbol()) {
				string group = token->getValue("auto", "group");
				if (group == "") {
					metersig = *token;
					foundtime = true;
				} else if (group == "A") {
					groupAsig = *token;
					foundtime = true;
				} else if (group == "B") {
					groupBsig = *token;
					foundtime = true;
				} else {
					metersig = *token;
					foundtime = true;
				}
			}
		}

		if (!foundtime) {
			continue;
		}

		for (int j=0; j<output[i].getFieldCount(); j++) {
			HTp token = output.token(i, j);
			string spinetype = token->getDataType();
			if ((spinetype == "**kern-coin") || (spinetype == "**kern-comp")) {
				if (metersig != "") {
					token->setText(metersig);
				} else if (groupAsig != "") {
					token->setText(groupAsig);
				} else if (groupBsig != "") {
					token->setText(groupAsig);
				}
			} else if (spinetype == "**kern-grpA") {
				if (groupAsig != "") {
					token->setText(groupAsig);
				}
			} else if (spinetype == "**kern-grpB") {
				if (groupBsig != "") {
					token->setText(groupBsig);
				}
			}
		}
		output[i].createLineFromTokens();

	}
}



//////////////////////////////
//
// adjustBadCoincidenceRests --  Sometimes coincidence rests are not so great, particularly
//    when they are long and there is a small note that will add to it to fill in a measure
//    (such as a 5 eighth-note rest in 6/8).  Try to simplify such case in this function
//    (more can be added on a case-by-case basis).
//
//    3... => 5 eighth notes (split according to meter).
//

void Tool_composite::adjustBadCoincidenceRests(HumdrumFile& output, HumdrumFile& infile) {
	vector<HTp> sstarts;
	output.getSpineStartList(sstarts, "**kern-coin");
	if (sstarts.empty()) {
		// no coincidence spine to process
		return;
	}

	HumRegex hre;
	vector<HumNum> timesigtop(output.getLineCount(), 4);
	vector<HumNum> timesigbot(output.getLineCount(), 4);
	HumNum tts = 4;
	HumNum bts = 4;

	for (int i=0; i<output.getLineCount(); i++) {
		if (!output[i].isInterpretation()) {
			timesigtop[i] = tts;
			timesigbot[i] = bts;
			continue;
		}
		for (int j=0; j<output[i].getFieldCount(); j++) {
			HTp token = output.token(i, j);
			if (token->getDataType() != "**kern-coin") {
				continue;
			}
			if (token->isTimeSignature()) {
				if (hre.search(token, "^\\*M(\\d+)/(\\d+)")) {
					tts = hre.getMatch(1);
					bts = hre.getMatch(2);
				}
			}
			break;
		}
		timesigtop[i] = tts;
		timesigbot[i] = bts;
	}

	HTp current = sstarts.at(0);
	string rhythm;
	while (current) {
		if (!current->isData()) {
			current = current->getNextToken();
			continue;
		}
		if (current->isNull()) {
			current = current->getNextToken();
			continue;
		}
		if (!current->isRest()) {
			// should check pitches as well
			current = current->getNextToken();
			continue;
		}
		if (hre.search(current, "(\\d+%?\\d*\\.*)")) {
			rhythm = hre.getMatch(1);
			if (rhythm == "3...") {
				int lindex = current->getLineIndex();
				current = fixBadRestRhythm(current, rhythm, timesigtop[lindex], timesigbot[lindex]);
			}
		}
		current = current->getNextToken();
	}

}


//////////////////////////////
//
// Tool_composite::fixBadRestRhythm --
//

HTp Tool_composite::fixBadRestRhythm(HTp token, string& rhythm, HumNum tstop, HumNum tsbot) {
	HumNum duration = Convert::recipToDuration(rhythm);
	if (rhythm == "3...") {
		duration = 5;
		duration /= 8;
		duration *= 4;
	}
	HumRegex hre;
	vector<HTp> tokens;
	HTp current = token;
	bool compound = false;
	HumNum testval = tstop / 3;
	if ((testval > 1) && testval.isInteger()) {
		compound = true;
	}
	tokens.push_back(token);
	current = current->getNextToken();
	while (current) {
		if (current->isBarline()) {
			tokens.push_back(current);
			break;
		}
		if (current->isData()) {
			tokens.push_back(current);
			if (!current->isNull()) {
				break;
			}
		}
		current = current->getNextToken();
	}

	vector<HumNum> beatfrac;
	for (int i=0; i<(int)tokens.size(); i++) {
		HumNum value = tokens[i]->getDurationFromBarline();
		if (compound) {
			value /= 3;
		}
		value *= tsbot;
		value /= 4;
		double dval = value.getFloat();
		int intval = (int)dval;
		value -= intval;
		beatfrac.push_back(value);
	}

	for (int i=1; i<(int)tokens.size() - 1; i++) {
		if (beatfrac[i] == 0) {
			// split rest at a beat boundary
			HumNum pos1 = token->getDurationFromStart();
			HumNum pos2 = tokens[i]->getDurationFromStart();
			HumNum predur = pos2 - pos1;
			HumNum postdur = duration - predur;
			string newrhy = Convert::durationToRecip(predur);
			string toktext = *token;
			string text2 = *token;
			hre.replaceDestructive(toktext, newrhy, rhythm);
			token->setText(toktext);
			newrhy = Convert::durationToRecip(postdur);
			hre.replaceDestructive(text2, newrhy, rhythm);
			tokens[i]->setText(text2);
			// doing only once for now
			break;
		}
	}

	if (tokens.back()->isBarline()) {
		return tokens.back();
	}
	if (tokens.size() == 1) {
		return tokens.back();
	}
	if (tokens.size() > 1) {
		return tokens.at((int)tokens.size() - 2);
	}
	// shouldn't get here
	return NULL;
}



//////////////////////////////
//
// Tool_composite::convertNotesToRhythms --
//

void Tool_composite::convertNotesToRhythms(HumdrumFile& infile) {
	HumRegex hre;
	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isData()) {
			continue;
		}
		for (int j=0; j<infile[i].getFieldCount(); j++) {
			HTp token = infile.token(i, j);
			if (!token->isKern()) {
				continue;
			}
			if (token->isRest()) {
				continue;
			}
			if (token->isNull()) {
				continue;
			}
			string text = *token;
			hre.replaceDestructive(text, "eR", "[A-Ga-g]+[#n-]*", "g");
			token->setText(text);
		}
		infile[i].createLineFromTokens();
	}
}



//////////////////////////////
//
// getEventCount -- Return the number of note attacks.
//

int Tool_composite::getEventCount(vector<string>& data) {
	int output = 0;
	for (int i=0; i<(int)data.size(); i++) {
		if (data[i] == "") {
			continue;
		}
		if (data[i] == ".") {
			continue;
		}
		if (data[i].find("*") != string::npos) {
			continue;
		}
		if (data[i].find("!") != string::npos) {
			continue;
		}
		if (data[i].find("r") != string::npos) {
			continue;
		}
		if (data[i].find("_") != string::npos) {
			continue;
		}
		if (data[i].find("]") != string::npos) {
			continue;
		}
		output++;
	}
	return output;
}



// END_MERGE

} // end namespace hum



