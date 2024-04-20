//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Sep  2 12:02:01 CEST 2023
// Last Modified: Tue Sep  5 05:55:32 CEST 2023
// Filename:      tool-pline.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/tool-pline.cpp
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Poetic line processing tool.
//

#include "tool-pline.h"
#include "HumRegex.h"

using namespace std;

namespace hum {

// START_MERGE


/////////////////////////////////
//
// Tool_pline::Tool_pline -- Set the recognized options for the tool.
//

Tool_pline::Tool_pline(void) {
	define("c|color=b",   "color poetic lines (currently only by notes)");
	define("o|overlap=b", "do overlap analysis/markup");
}



/////////////////////////////////
//
// Tool_pline::run -- Do the main work of the tool.
//

bool Tool_pline::run(HumdrumFileSet& infiles) {
	bool status = true;
	for (int i=0; i<infiles.getCount(); i++) {
		status &= run(infiles[i]);
	}
	return status;
}


bool Tool_pline::run(const string& indata, ostream& out) {
	HumdrumFile infile(indata);
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_pline::run(HumdrumFile& infile, ostream& out) {
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_pline::run(HumdrumFile& infile) {
	initialize();
	processFile(infile);
	return true;
}


//////////////////////////////
//
// Tool_pline::initialize --
//

void Tool_pline::initialize(void) {
	m_colors.resize(14);

	m_colors[0] = "red";        // red
	m_colors[1] = "darkorange"; // orange
	m_colors[2] = "gold";       // yellow
	m_colors[3] = "limegreen";  // green
	m_colors[4] = "skyblue";    // light blue
	m_colors[5] = "mediumblue"; // dark blue
	m_colors[6] = "purple";     // purple

	m_colors[7]  = "darkred";        // red
	m_colors[8]  = "lightsalmon";    // orange
	m_colors[9]  = "darkgoldenrod";  // yellow
	m_colors[10] = "olivedrab";      // green
	m_colors[11] = "darkturquoise";  // light blue
	m_colors[12] = "darkblue";       // dark blue
	m_colors[13] = "indigo";         // purple

	// lighter colors for staff highlighting:
	//m_colors[0] = "#ffaaaa";  // red
	//m_colors[1] = "#ffbb00";  // orange
	//m_colors[2] = "#eeee00";  // yellow
	//m_colors[3] = "#99cc01";  // green
	//m_colors[4] = "#bbddff";  // light blue
	//m_colors[5] = "#88aaff";  // dark blue
	//m_colors[6] = "#cc88ff";  // purple

	m_colorQ = getBoolean("color");
	m_colorQ = true;  // default behavior for now.
}



//////////////////////////////
//
// Tool_pline::processFile --
//

void Tool_pline::processFile(HumdrumFile& infile) {
	getPlineInterpretations(infile, m_ptokens);
	fillLineInfo(infile, m_lineInfo);
	if (m_colorQ) {
		plineToColor(infile, m_ptokens);
	}
	infile.createLinesFromTokens();
	m_humdrum_text << infile;
	if (m_colorQ) {
		m_humdrum_text << "!!!RDF**kern: 😀 = marked note, color=black" << endl;
	}
}



//////////////////////////////
//
// Tool_pline::markRests --
//

void Tool_pline::markRests(HumdrumFile& infile) {
	vector<HTp> spinestops;
	infile.getSpineStopList(spinestops);
	for (int i=0; i<(int)spinestops.size(); i++) {
		if (!spinestops[i]->isKern()) {
			continue;
		}
		markSpineRests(spinestops[i]);
	}
}


//////////////////////////////
//
// Tool_pline::markSpineRests --
//

void Tool_pline::markSpineRests(HTp spineStop) {
	string marker = "😀";
	int track = spineStop->getTrack();
	int lastValue = -1;
	HTp current = spineStop->getPreviousToken();
	int  line;
	int  cvalue;
	while (current) {
		if (!current->isData()) {
			current = current->getPreviousToken();
			continue;
		}
		if (current->isNull()) {
			current = current->getPreviousToken();
			continue;
		}

		line = current->getLineIndex();
		cvalue = m_lineInfo.at(line).at(track);

		if (current->isRest() && (cvalue != lastValue)) {
			string text = *current;
			text += marker;
			current->setText(text);
		} else {
			lastValue = cvalue;
			string text = *current;
			text += "@" + to_string(cvalue);
			current->setText(text);
		}
		current = current->getPreviousToken();
	}
}



//////////////////////////////
//
// Tool_pline::fillLineInfo --
//

void Tool_pline::fillLineInfo(HumdrumFile& infile, vector<vector<int>>& lineinfo) {
	lineinfo.clear();
	lineinfo.resize(infile.getLineCount());
	int maxtrack = infile.getMaxTrack();
	HumRegex hre;
	for (int i=0; i<infile.getLineCount(); i++) {
		lineinfo[i].resize(maxtrack + 1);
		fill(lineinfo[i].begin(), lineinfo[i].end(), 0);
		if (!infile[i].isInterpretation()) {
			continue;
		}
		for (int j=0; j<infile[i].getFieldCount(); j++) {
			HTp token = infile.token(i, j);
			if (hre.search(token, "^\\*pline:\\s*(\\d+)")) {
				int digit = hre.getMatchInt(1);
				int track = token->getTrack();
				lineinfo[i][track] = digit;
			}
		}
	}

	for (int i=1; i<(int)lineinfo.size() - 1; i++) {
		for (int j=1; j<=maxtrack; j++) {
			if (lineinfo.at(i).at(j)) {
				continue;
			} else {
				lineinfo.at(i).at(j) = lineinfo.at(i-1).at(j);
			}
		}
	}

	// for (int i=0; i<(int)lineinfo.size() - 1; i++) {
	// 	for (int j=1; j<=maxtrack; j++) {
	// 		cerr << lineinfo[i][j] << "\t";
	// 	}
	// 	cerr << endl;
	// }

}



//////////////////////////////
//
// Tool_pline::plineToColor --
//

void Tool_pline::plineToColor(HumdrumFile& infile, vector<HTp>& tokens) {
	HumRegex hre;
	markRests(infile);
	for (int i=0; i<(int)tokens.size(); i++) {
		if (!hre.search(tokens[i], "^\\*pline:\\s*(\\d+)")) {
			continue;
		}
		int lineNum = hre.getMatchInt(1);
		int colorIndex = (lineNum - 1) % m_colors.size();
		string color = m_colors.at(colorIndex);
		string text = "*color:";
		text += color;
		tokens[i]->setText(text);
	}
}



//////////////////////////////
//
// Tool_pline::getPlineInterpretations --
//

void Tool_pline::getPlineInterpretations(HumdrumFile& infile, vector<HTp>& tokens) {
	HumRegex hre;
	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isInterpretation()) {
			continue;
		}
		for (int j=0; j<infile[i].getFieldCount(); j++) {
			HTp token = infile.token(i, j);
			if (!token->isKern()) {
				continue;
			}
			if (hre.search(token, "^\\*pline:\\s*(\\d+)")) {
				tokens.push_back(token);
			}
		}
	}
}



// END_MERGE

} // end namespace hum



