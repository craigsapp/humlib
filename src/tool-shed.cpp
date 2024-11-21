//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sun Oct 13 11:41:16 PDT 2019
// Last Modified: Thu May 13 09:42:30 PDT 2021
// Filename:      tool-shed.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/tool-shed.cpp
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   sed-like tool for Humdrum data.
//

#include "tool-shed.h"
#include "Convert.h"
#include "HumRegex.h"

using namespace std;

namespace hum {

// START_MERGE


/////////////////////////////////
//
// Tool_shed::Tool_shed -- Set the recognized options for the tool.
//

Tool_shed::Tool_shed(void) {
	define("s|spine|spines=s",              "list of spines to process");
	define("e|expression=s",                "regular expression");
	define("E|exclusion-expression=s",      "regular expression to skip");
	define("x|exclusive-interpretations=s", "apply only to spine types in list");
	define("k|kern=b",                      "apply only to **kern data");
	define("X=s",                           "defineable exclusive interpretation x");
	define("Y=s",                           "defineable exclusive interpretation y");
	define("Z=s",                           "defineable exclusive interpretation z");
}



/////////////////////////////////
//
// Tool_shed::run -- Do the main work of the tool.
//

bool Tool_shed::run(HumdrumFileSet& infiles) {
	bool status = true;
	for (int i=0; i<infiles.getCount(); i++) {
		status &= run(infiles[i]);
	}
	return status;
}


bool Tool_shed::run(const string& indata, ostream& out) {
	HumdrumFile infile(indata);
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_shed::run(HumdrumFile& infile, ostream& out) {
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_shed::run(HumdrumFile& infile) {
	initialize();
	initializeSegment(infile);
	if (m_options.empty()) {
		cerr << "Error: -e option is required" << endl;
		return false;
	}
	processFile(infile);
	return true;
}



//////////////////////////////
//
// Tool_shed::prepareSearch --
//

void Tool_shed::prepareSearch(int index) {
	// deal with command-line options (seprately for each search):
	m_exinterps.clear();

	if (getBoolean("kern")) {
		m_exinterps.push_back("**kern");
	} else if (getBoolean("exclusive-interpretations")) {
		vector<string> extra = addToExInterpList();
		for (int i=0; i<(int)extra.size(); i++) {
			m_exinterps.push_back(extra[i]);
		}
	}

	m_search  = m_searches.at(index);
	m_replace = m_replaces.at(index);
	m_option  = m_options.at(index);

	m_grepoptions = "";
	if (m_option.find("i") != std::string::npos) {
		m_grepoptions += "i";
	}
	if (m_option.find("g") != std::string::npos) {
		m_grepoptions += "g";
	}

	if (m_option.find("X") != std::string::npos) {
		if (m_xInterp != "") {
			m_exinterps.push_back(m_xInterp);
		}
	}
	if (m_option.find("Y") != std::string::npos) {
		if (m_yInterp != "") {
			m_exinterps.push_back(m_yInterp);
		}
	}
	if (m_option.find("Z") != std::string::npos) {
		if (m_zInterp != "") {
			m_exinterps.push_back(m_zInterp);
		}
	}

	m_data = true;             // process data
	m_barline = false;         // process barline
	m_exinterp = false;        // process exclusive interpretations
	m_interpretation = false;  // process interpretations (other than exinterp
	                           //     and spine manipulators).

	if (m_option.find("I") != std::string::npos) {
		m_interpretation = true;
		m_data = false;
	}
	if (m_option.find("X") != std::string::npos) {
		m_exinterp = true;
		m_data = false;
	}
	if (m_option.find("B") != std::string::npos) {
		m_barline = true;
		m_data = false;
	}
	if (m_option.find("M") != std::string::npos) {
		// measure is an alias for barline
		m_barline = true;
		m_data = false;
	}
	if (m_option.find("L") != std::string::npos) {
		m_localcomment = true;
		m_data = false;
	}
	if (m_option.find("G") != std::string::npos) {
		m_globalcomment = true;
		m_data = false;
	}
	if (m_option.find("K") != std::string::npos) {
		m_referencekey = true;
		m_data = false;
	}
	if (m_option.find("V") != std::string::npos) {
		m_referencevalue = true;
		m_data = false;
	}
	if (m_option.find("R") != std::string::npos) {
		m_reference = true;
		m_referencekey = false;
		m_referencevalue = false;
		m_data = false;
	}
	if (m_option.find("D") != std::string::npos) {
		m_data = true;
	}

}



//////////////////////////////
//
// Tool_shed::initialize --  Initializations that only have to be done once
//    for all HumdrumFile segments.
//

void Tool_shed::initialize(void) {
	if (getBoolean("expression")) {
		string value = getString("expression");
		parseExpression(value);
	}
	m_exclusion = getString("exclusion-expression");

	if (getBoolean("X")) {
		m_xInterp = getExInterp(getString("X"));
	}
	if (getBoolean("Y")) {
		m_yInterp = getExInterp(getString("Y"));
	}
	if (getBoolean("Z")) {
		m_zInterp = getExInterp(getString("Z"));
	}
}



//////////////////////////////
//
// Tool_shed::getExInterp --
//

string Tool_shed::getExInterp(const string& value) {
	if (value == "") {
		return "**";
	}
	if (value == "*") {
		return "**";
	}
	if (value.compare(0, 2, "**") == 0) {
		return value;
	}
	if (value.compare(0, 1, "*") == 0) {
		return "*" + value;
	}
	return "**" + value;
}



//////////////////////////////
//
// Tool_shed::parseExpression --
//     Form of string:
//        s/search/replace/options; s/search2/replace2/options2
//
//

void Tool_shed::parseExpression(const string& expression) {
	int state = 0;

	m_searches.clear();
	m_replaces.clear();
	m_options.clear();

	char divchar = '/';

	for (int i=0; i<(int)expression.size(); i++) {
		if (state == 0) {  // start of expression
			if (isspace(expression[i])) {
				continue;
			} else if (expression[i] == 's') {
				if (i >= (int)expression.size() - 1) {
					cerr << "Error: spurious s at end of expression: "
					     << expression << endl;
					return;
				} else {
					divchar = expression[i+1];
					i++;
					state++;
					m_searches.push_back("");
				}
			} else {
				cerr << "Error at position " << i
				     << " in expression: " << expression << endl;
				return;
			}
		} else if (state == 1) { // search string
			if (expression[i] == divchar) {
				state++;
				m_replaces.push_back("");
				continue;
			} if (expression[i] == '\\') {
				if (i >= (int)expression.size() - 1) {
					cerr << "Error: expression ends too soon: "
					     << expression << endl;
					return;
				} else {
					m_searches.back() += '\\';
					m_searches.back() += expression[i+1];
					i++;
				}
			} else {
				m_searches.back() += expression[i];
			}
		} else if (state == 2) { // replace string
			if (expression[i] == divchar) {
				state++;
				m_options.push_back("");
				continue;
			} if (expression[i] == '\\') {
				if (i >= (int)expression.size() - 1) {
					cerr << "Error: expression ends too soon: "
					     << expression << endl;
					return;
				} else {
					m_replaces.back() += '\\';
					m_replaces.back() += expression[i+1];
					i++;
				}
			} else {
				m_replaces.back() += expression[i];
			}
		} else if (state == 3) { // regular expression options
			if (expression[i] == ';') {
				state++;
			} else if (isspace(expression[i])) {
				state++;
			} else {
				m_options.back() += expression[i];
			}
		}
		if (state == 4) {
			state = 0;
		}
	}
}



//////////////////////////////
//
// Tool_shed::initializeSegment -- Recalculate variables for each Humdrum
//      input segment.
//

void Tool_shed::initializeSegment(HumdrumFile& infile) {
	m_spines.clear();
	if (getBoolean("spines")) {
		int maxtrack = infile.getMaxTrack();
		Convert::makeBooleanTrackList(m_spines, getString("spines"), maxtrack);
	}
}



//////////////////////////////
//
// Tool_shed::addToExInterpList --
//

vector<string> Tool_shed::addToExInterpList(void) {
	string elist = getString("exclusive-interpretations");
	elist = Convert::trimWhiteSpace(elist);
	HumRegex hre;
	hre.replaceDestructive(elist, "", "^[,;\\s*]+");
	hre.replaceDestructive(elist, "", "[,;\\s*]+$");
	vector<string> pieces;
	hre.split(pieces, elist, "[,;\\s*]+");

	vector<string> output;
	for (int i=0; i<(int)pieces.size(); i++) {
		if (pieces[i].empty()) {
			continue;
		}
		output.push_back("**" + pieces[i]);
	}
	return output;
}



//////////////////////////////
//
// Tool_shed::processFile --
//

void Tool_shed::processFile(HumdrumFile& infile) {
	for (int i=0; i<(int)m_options.size(); i++) {
		prepareSearch(i);
		processExpression(infile);
	}
	m_humdrum_text << infile;
}



//////////////////////////////
//
// Tool_shed::processExpression --
//

void Tool_shed::processExpression(HumdrumFile& infile) {
	if (m_search == "") {
		// nothing to do
		return;
	}
	m_modified = false;

	if (m_interpretation) {
		searchAndReplaceInterpretation(infile);
	}

	if (m_localcomment) {
		searchAndReplaceLocalComment(infile);
	}

	if (m_globalcomment) {
		searchAndReplaceGlobalComment(infile);
	}

	if (m_reference) {
		searchAndReplaceReferenceRecords(infile);
	}

	if (m_referencekey) {
		searchAndReplaceReferenceKeys(infile);
	}

	if (m_referencevalue) {
		searchAndReplaceReferenceValues(infile);
	}

	if (m_exinterp) {
		searchAndReplaceExinterp(infile);
	}

	if (m_barline) {
		searchAndReplaceBarline(infile);
	}

	if (m_data) {
		searchAndReplaceData(infile);
	}

	if (m_modified) {
		infile.createLinesFromTokens();
	}
}



//////////////////////////////
//
// Tool_shed::searchAndReplaceBarline --
//

void Tool_shed::searchAndReplaceBarline(HumdrumFile& infile) {
	string isearch;
	if (m_search[0] == '^') {
		isearch = "^=" + m_search.substr(1);
	} else {
		isearch = "^=.*" + m_search;
	}
	HumRegex hre;
	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isBarline()) {
			continue;
		}
		for (int j=0; j<infile[i].getFieldCount(); j++) {
			HTp token = infile.token(i, j);
			if (token->isNull()) {
				// Don't mess with null interpretations
				continue;
			}
			if (!isValid(token)) {
				continue;
			}
			if (hre.search(token, isearch, m_grepoptions)) {
				string text = token->getText().substr(1);
				hre.replaceDestructive(text, m_replace, m_search, m_grepoptions);
				hre.replaceDestructive(text, "", "^=+");
				text = "=" + text;
				token->setText(text);
				m_modified = true;
			}
		}
	}
}



//////////////////////////////
//
// Tool_shed::searchAndReplaceInterpretation --
//

void Tool_shed::searchAndReplaceInterpretation(HumdrumFile& infile) {
	string isearch;
	if (m_search[0] == '^') {
		isearch = "^\\*" + m_search.substr(1);
	} else {
		isearch = "^\\*.*" + m_search;
	}
	HumRegex hre;
	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isInterpretation()) {
			continue;
		} else if (infile[i].isExclusiveInterpretation()) {
			continue;
		} else if (infile[i].isManipulator()) {
			continue;
		}
		for (int j=0; j<infile[i].getFieldCount(); j++) {
			HTp token = infile.token(i, j);
			if (token->isNull()) {
				// Don't mess with null interpretations
				continue;
			}
			if (!isValid(token)) {
				continue;
			}
			if (hre.search(token, isearch, m_grepoptions)) {
				string text = token->getText().substr(1);
				hre.replaceDestructive(text, m_replace, m_search, m_grepoptions);
				hre.replaceDestructive(text, "", "^\\*+");
				text = "*" + text;
				token->setText(text);
				m_modified = true;
			}
		}
	}
}



//////////////////////////////
//
// Tool_shed::searchAndReplaceLocalComment --
//

void Tool_shed::searchAndReplaceLocalComment(HumdrumFile& infile) {
	string isearch;
	if (m_search[0] == '^') {
		isearch = "^!" + m_search.substr(1);
	} else {
		isearch = "^!.*" + m_search;
	}
	HumRegex hre;
	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isLocalComment()) {
			continue;
		}
		for (int j=0; j<infile[i].getFieldCount(); j++) {
			HTp token = infile.token(i, j);
			if (token->isNull()) {
				// Don't mess with null interpretations
				continue;
			}
			if (!isValid(token)) {
				continue;
			}
			if (hre.search(token, isearch, m_grepoptions)) {
				string text = token->getText().substr(1);
				hre.replaceDestructive(text, m_replace, m_search, m_grepoptions);
				hre.replaceDestructive(text, "", "^!+");
				text = "!" + text;
				token->setText(text);
				m_modified = true;
			}
		}
	}
}



//////////////////////////////
//
// Tool_shed::searchAndReplaceGlobalComment --
//

void Tool_shed::searchAndReplaceGlobalComment(HumdrumFile& infile) {
	string isearch;
	if (m_search[0] == '^') {
		isearch = "^!!" + m_search.substr(1);
	} else {
		isearch = "^!!.*" + m_search;
	}
	HumRegex hre;
	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isGlobalComment()) {
			continue;
		}
		HTp token = infile.token(i, 0);
		if (token->size() < 3) {
			// Don't mess with null comments
			continue;
		}
		if (hre.search(token, isearch, m_grepoptions)) {
			string text = token->getText().substr(2);
			hre.replaceDestructive(text, m_replace, m_search, m_grepoptions);
			hre.replaceDestructive(text, "", "^!+");
			text = "!!" + text;
			token->setText(text);
			m_modified = true;
		}
	}
}



//////////////////////////////
//
// Tool_shed::searchAndReplaceReferenceRecords --
//

void Tool_shed::searchAndReplaceReferenceRecords(HumdrumFile& infile) {
	string isearch;
	if (m_search[0] == '^') {
		isearch = "^!!!" + m_search.substr(1);
	} else {
		isearch = "^!!!.*" + m_search;
	}
	HumRegex hre;
	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isGlobalReference()) {
			continue;
		}
		HTp token = infile.token(i, 0);
		if (hre.search(token, isearch, m_grepoptions)) {
			string text = token->getText().substr(1);
			hre.replaceDestructive(text, m_replace, m_search, m_grepoptions);
			hre.replaceDestructive(text, "", "^!+");
			text = "!!!" + text;
			token->setText(text);
			m_modified = true;
		}
	}
}



//////////////////////////////
//
// Tool_shed::searchAndReplaceReferenceKeys --
//

void Tool_shed::searchAndReplaceReferenceKeys(HumdrumFile& infile) {
	string isearch = m_search;
	HumRegex hre;
	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isGlobalReference()) {
			continue;
		}
		HTp token = infile.token(i, 0);
		string key = infile[i].getReferenceKey();
		if (hre.search(key, isearch, m_grepoptions)) {
			hre.replaceDestructive(key, m_replace, m_search, m_grepoptions);
			hre.replaceDestructive(key, "", "^!+");
			hre.replaceDestructive(key, "", ":+$");
			string value = infile[i].getReferenceValue();
			string text = "!!!" + key + ": " + value;
			token->setText(text);
			m_modified = true;
		}
	}
}



//////////////////////////////
//
// Tool_shed::searchAndReplaceReferenceValues --
//

void Tool_shed::searchAndReplaceReferenceValues(HumdrumFile& infile) {
	string isearch = m_search;
	HumRegex hre;
	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isGlobalReference()) {
			continue;
		}
		HTp token = infile.token(i, 0);
		string value = infile[i].getReferenceValue();
		if (hre.search(value, isearch, m_grepoptions)) {
			hre.replaceDestructive(value, m_replace, m_search, m_grepoptions);
			hre.replaceDestructive(value, "", "^!+");
			hre.replaceDestructive(value, "", ":+$");
			string key = infile[i].getReferenceKey();
			string text = "!!!" + key + ": " + value;
			token->setText(text);
			m_modified = true;
		}
	}
}



//////////////////////////////
//
// Tool_shed::searchAndReplaceExinterp --
//

void Tool_shed::searchAndReplaceExinterp(HumdrumFile& infile) {
	string isearch;
	if (m_search[0] == '^') {
		isearch = "^\\*\\*" + m_search.substr(1);
	} else {
		isearch = "^\\*\\*.*" + m_search;
	}
	HumRegex hre;
	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isInterpretation()) {
			continue;
		} else if (!infile[i].isExclusiveInterpretation()) {
			// assuming a single line for all exclusive interpretations
			continue;
		}
		for (int j=0; j<infile[i].getFieldCount(); j++) {
			HTp token = infile.token(i, j);
			if (token->isNull()) {
				// Don't mess with null interpretations
				continue;
			}
			if (!isValid(token)) {
				continue;
			}
			if (hre.search(token, isearch, m_grepoptions)) {
				string text = token->getText().substr(2);
				hre.replaceDestructive(text, m_replace, m_search, m_grepoptions);
				hre.replaceDestructive(text, "", "^\\*+");
				text = "**" + text;
				token->setText(text);
				m_modified = true;
			}
		}
	}
}



//////////////////////////////
//
// Tool_shed::searchAndReplaceData --
//

void Tool_shed::searchAndReplaceData(HumdrumFile& infile) {
	string dsearch = m_search;

	HumRegex hre;
	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isData()) {
			continue;
		}
		for (int j=0; j<infile[i].getFieldCount(); j++) {
			HTp token = infile.token(i, j);
			if (token->isNull()) {
				// Don't mess with null interpretations
				continue;
			}
			if (!isValid(token)) {
				continue;
			}
			if (hre.search(token, dsearch, m_grepoptions)) {
				string text = token->getText();
				hre.replaceDestructive(text, m_replace, dsearch, m_grepoptions);
				if (text == "") {
					text = ".";
				}
				token->setText(text);
				m_modified = true;
			}
		}
	}
}



//////////////////////////////
//
// Tool_shed::isValidDataType -- usar with -x and -k options.
//

bool Tool_shed::isValidDataType(HTp token) {
	if (m_exinterps.empty()) {
		return true;
	}
	string datatype = token->getDataType();
	for (int i=0; i<(int)m_exinterps.size(); i++) {
		if (datatype == m_exinterps[i]) {
			return true;
		}
	}
	return false;
}



//////////////////////////////
//
// Tool_shed::isValidSpine -- used with -s option.
//

bool Tool_shed::isValidSpine(HTp token) {
	if (m_spines.empty()) {
		return true;
	}
	int track = token->getTrack();
	return m_spines.at(track);
}



//////////////////////////////
//
// Tool_shed::isValid --
//

bool Tool_shed::isValid(HTp token) {
	if (!m_exclusion.empty()) {
		HumRegex hre;
		if (hre.search(token, m_exclusion)) {
			return false;
		}
	}
	if (isValidDataType(token) && isValidSpine(token)) {
		return true;
	}
	return false;
}



// END_MERGE

} // end namespace hum



