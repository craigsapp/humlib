//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sun Jul 14 01:03:07 CEST 2019
// Last Modified: Fri Jan  7 15:33:12 PST 2022
// Filename:      tool-composite.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/tool-composite.cpp
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Generate composite rhythm spine for music.
//
// To do:
// composite -F is not working.
// composite -x should allow beamaing.
// (1) Add ties that are missing from composite notation
// (2) Unmetered tremolos as ornaments in analyses
// (3) Grace notes counted as ornaments (deal with trill endings later)
// (5) Ties unmetered tremolo and grace notes (accent tool)
//

#include "tool-composite.h"
#include "tool-extract.h"
#include "tool-autobeam.h"
#include "Convert.h"
#include "HumRegex.h"

#include <strstream>

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
	define("debug=b",       "print debug statements");
	define("a|append=b",    "append data to end of line (top of system)");
	define("x|extract=b",   "only output composite rhythm spines");
	define("grace=b",       "include grace notes in composite rhythms");
	define("u|up-stem=b",   "force notes to be up-stem");

	define("o|only=s",      "output notes of given group (A or B)");

	define("F|no-full-composite=b", "Do not do full composite rhythm analysis");
	define("c|coincidence=b", "Do coincidence rhythm analysis");
	define("g|group|groups|grouping|groupings=b", "Do group rhythm analysis");
	define("m|mark=b",        "Mark coincidences in group analysis and input score");

	// Numeric analysis options:
	define("P|onsets=b",             "count number of note (pitch) onsets in feature");
	define("A|accents=b",            "count number of accents in feature");
	define("O|ornaments=b",          "count number of ornaments in feature");
	define("S|slurs=b",              "count number of slur beginnings/ending in feature");
	define("T|total=b",              "count total number of analysis features for each note");
	define("all|all-analyses=b",     "do all analyses");

	// Styling for numeric analyses;
	define("Z|no-zeros|no-zeroes=b", "do not show zeros in analyses.");
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

	m_fullCompositeQ = !getBoolean("no-full-composite");
	m_coincidenceQ   =  getBoolean("coincidence");
	m_groupsQ        =  getBoolean("groups");
	m_upstemQ        =  getBoolean("up-stem");

	// There must be at least one analysis being done (excluding -o options):
	if (!m_groupsQ && !m_coincidenceQ) {
		m_fullCompositeQ = true;
	}

	// Extract music in a specific group:
	m_onlyQ          = getBoolean("only");
	m_only           = getString("only");

	if (m_fullCompositeQ) {
		m_fullComposite.resize(infile.getLineCount());
	}

	m_groups.resize(2);
	m_groups[0].resize(infile.getLineCount());
	m_groups[1].resize(infile.getLineCount());

	m_analysisOnsetsQ    = getBoolean("onsets");
	m_analysisAccentsQ   = getBoolean("accents");
	m_analysisOrnamentsQ = getBoolean("ornaments");
	m_analysisSlursQ     = getBoolean("slurs");
	m_analysisTotalQ     = getBoolean("total");
	if (getBoolean("all")) {
		m_analysisOnsetsQ    = true;
		m_analysisAccentsQ   = true;
		m_analysisOrnamentsQ = true;
		m_analysisSlursQ     = true;
		m_analysisTotalQ     = true;
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

	if (m_coincidenceQ) {
		analyzeCoincidenceRhythms(infile);
	}
	if (m_fullCompositeQ) {
		analyzeFullCompositeRhythm(infile);
	}
	if (m_groupsQ) {
		analyzeGroupCompositeRhythms(infile);
	}
	if (m_analysesQ) {
		doNumericAnalyses(infile);
	}
	prepareOutput(infile);
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

	// Now either print the output data by itself, or
	// merge with input score:
	if (m_extractQ) {
		// output only analysis data
		m_humdrum_text << analysis.str();
	} else {
		// merge analysis data with input file
		HumdrumFile output;
		output.readString(analysis.str());
		if (m_beamQ) {
			Tool_autobeam autobeam;
			autobeam.run(output);
		}

		for (int i=0; i<infile.getLineCount(); i++) {

			if (m_verseLabelIndex && (m_verseLabelIndex == -i)) {
				m_humdrum_text << generateVerseLabelLine(output, infile, i);
				m_humdrum_text << endl;
			}

			if (m_striaIndex && (m_striaIndex == -i)) {
				m_humdrum_text << generateStriaLine(output, infile, i);
				m_humdrum_text << endl;
			}

			if (!infile[i].hasSpines()) {
				m_humdrum_text << infile[i];
			} else if (m_appendQ) {
				// analysis data at end of line
				m_humdrum_text << infile[i];
				m_humdrum_text << "\t";
				m_humdrum_text << output[i];
			} else if (m_prependQ) {
				// analysis data at start of line (default)
				m_humdrum_text << output[i];
				m_humdrum_text << "\t";
				m_humdrum_text << infile[i];
			} else {
				// output data only
				m_humdrum_text << output[i];
			}
			m_humdrum_text << endl;
		}
	}
	if (m_coinMarkQ) {
		m_humdrum_text << "!!!RDF**kern: " << m_coinMark;
		m_humdrum_text << " = marked note, color=\"";
		m_humdrum_text << "limegreen\"";
		m_humdrum_text << endl;
	}
}



//////////////////////////////
//
// Tool_composite::generateVerseLabelLine --
//

string Tool_composite::generateVerseLabelLine(HumdrumFile& output, HumdrumFile& input, int line) {
	string outstring;
	string inputBlanks;
	for (int i=0; i<input[line].getFieldCount(); i++) {
		inputBlanks += "*";
		if (i < input[line].getFieldCount() - 1) {
			inputBlanks += "\t";
		}
	}
	if (m_appendQ) {
		outstring += inputBlanks;
		outstring += "\t";
	}
	string outputLabels;
	for (int i=0; i<output[line].getFieldCount(); i++) {
		HTp token = output.token(line, i);
		string exinterp = token->getExInterp();
		if (exinterp.compare(0, 8, "**vdata-") != 0) {
			outputLabels += "*";
			if (output[line].getFieldCount()) {
				outputLabels += "\t";
			}
			continue;
		}
		string label = exinterp.substr(8);
		outputLabels += "*v:";
		outputLabels += label;
		if (i < output[line].getFieldCount() - 1) {
			outputLabels += "\t";
		}
	}
	outstring += outputLabels;
	if (m_prependQ) {
		outstring += "\t";
		outstring += inputBlanks;
	}
	return outstring;
}



//////////////////////////////
//
// Tool_composite::generateStriaLine --
//

string Tool_composite::generateStriaLine(HumdrumFile& output, HumdrumFile& input, int line) {
	string outstring;
	string inputBlanks;
	for (int i=0; i<input[line].getFieldCount(); i++) {
		inputBlanks += "*";
		if (i < input[line].getFieldCount() - 1) {
			inputBlanks += "\t";
		}
	}
	if (m_appendQ) {
		outstring += inputBlanks;
		outstring += "\t";
	}
	string outputStria;
	for (int i=0; i<output[line].getFieldCount(); i++) {
		HTp token = output.token(line, i);
		string exinterp = token->getExInterp();
		if (exinterp.compare(0, 6, "**kern") != 0) {
			outputStria += "*";
			if (output[line].getFieldCount()) {
				outputStria += "\t";
			}
			continue;
		}
		outputStria += "*stria1";
		if (i < output[line].getFieldCount() - 1) {
			outputStria += "\t";
		}
	}
	outstring += outputStria;
	if (m_prependQ) {
		outstring += "\t";
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
		tempout << getCoincidenceToken(infile, line);
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
			return "*I\"Coincidence";
		} else if (line == m_instrumentAbbrIndex) {
			return "*I'Coin.";
		} else if (line == m_timeSignatureIndex) {
			return getTimeSignature(infile, m_timeSignatureIndex, "");
		} else if (line == m_meterSymbolIndex) {
			return getMeterSymbol(infile, m_meterSymbolIndex, "");
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
			return "*I\"Composite";
		} else if (line == m_instrumentAbbrIndex) {
			return "*I'Comp.";
		} else if (line == m_timeSignatureIndex) {
			return getTimeSignature(infile, m_timeSignatureIndex, "");
		} else if (line == m_meterSymbolIndex) {
			return getMeterSymbol(infile, m_meterSymbolIndex, "");
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
// Tool_composite::needsCoincidenceMarker -- return composite marker if there
//     is a coincidence between two groups on the given line (from group analysis data).
//

bool Tool_composite::needsCoincidenceMarker(int line) {
	string group1 = m_groups.at(0).at(line);
	string group2 = m_groups.at(1).at(line);

	bool domark = true;
	if (!m_coinMarkQ) {
		return false;
	} else { 
		// coincidence if there are no ties or rests, or null tokens involved.
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
				return "*I\"Group A";
			} else {
				return "*I\"Group B";
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
			return getMeterSymbol(infile, m_meterSymbolIndex, tgroup);
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
// Tool_composite::getMeterSymbol --
//

string Tool_composite::getMeterSymbol(HumdrumFile& infile, int line, const string& group) {
	if (!infile[line].isInterpretation()) {
		return "*";
	}
	HTp lastToken = NULL;
	for (int j=0; j<infile[line].getFieldCount(); j++) {
		HTp token = infile.token(line, j);
		if (!token->isMeterSymbol()) {
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

	vector<bool> isRest(infile.getLineCount(), false);
	vector<bool> isNull(infile.getLineCount(), false);

	for (int i=0; i<infile.getLineCount(); i++) {
		if (durations[i] == 0) {
			continue;
		}
		bool allnull = true;
		bool allrest = true;
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
		}
		m_fullComposite[i] = output;
	}

//	removeAuxTremolosFromCompositeRhythm(infile);

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
		currValue = merged[i];
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

	for (int i=0; i<infile.getLineCount(); i++) {
		if (noteAttack[i]) {
			HumNum duration = infile[nextAttackIndex[i]].getDurationFromStart() -
				infile[i].getDurationFromStart();
			string recip = Convert::durationToRecip(duration);
			string text;
			if (noteAttack[i] == -2) {
				text = recip + "eR";
				// this is either a middle tie if next note is also a tie
				// or is a tie end if the next note is an attack (1) or rest (2)
				if (noteAttack[nextAttackIndex[i]] > 0) {
					// tie end
					m_coincidence[i] = text + "]";
				} else {
					// tie middle
					m_coincidence[i] = text + "_";
				}
			} else if (noteAttack[i] == 1) {
				text = recip + "eR";
				// Start a tie if the next note is a tied note
				if (noteAttack[nextAttackIndex[i]] == -2) {
					m_coincidence[i] = "[" + text;
				} else {
					m_coincidence[i] = text;
				}
			} else if (noteAttack[i] == 2) {
				// rest
				text = recip + "r";
				m_coincidence[i] = text;
			}
		}
	}

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
			bool sus2 = true;
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
					sus2 = false;
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

				if (sus2 && nullSustain) {
					states[i] = -1;
				} else if (nullSustain) {
					if (states[i] <= 0) {
						states[i] = -1;
					}
				} else if (sus2) {
					if (tieNote) {
						states[i] = -2;
					} else {
						states[i] = -1;
					}
				} else {
					states[i] = 1;
				}
			}
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

	vector<bool> isRest(infile.getLineCount(), false);
	vector<bool> isNull(infile.getLineCount(), false);

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

HumNum Tool_composite::getLineDuration(HumdrumFile& infile, int index, vector<bool>& isNull) {
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
//
// First index of m_analyses is rhythm type, and second index is analysis type, with "0" being onsets.
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
		// doOnsetAnalysisCoincidence(m_analyses.at(m_COINCIDENCE).at(m_ONSET), infile, m_analyses.at(0));
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
	m_striaIndex           = 0;
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

}


// END_MERGE

} // end namespace hum



