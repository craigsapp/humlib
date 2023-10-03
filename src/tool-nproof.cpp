//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Wed Sep 20 13:08:15 PDT 2023
// Last Modified: Wed Sep 20 13:08:18 PDT 2023
// Filename:      tool-nproof.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/tool-nproof.cpp
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
// Documentation: https://doc.verovio.humdrum.org/filter/nproof
//
// Description:   Meter/beat data extraction tool.
//

#include "tool-nproof.h"
#include "Convert.h"
#include "HumRegex.h"

using namespace std;

namespace hum {

// START_MERGE


/////////////////////////////////
//
// Tool_nproof::Tool_nproof -- Set the recognized options for the tool.
//

Tool_nproof::Tool_nproof(void) {
	define("B|no-blank|no-blanks=b", "Do not check for blank lines.\n");
	define("b|only-blank|only-blanks=b", "Only check for blank lines.\n");

	define("I|no-instrument|no-instruments=b", "Do not check instrument interpretations.\n");
	define("i|only-instrument|only-instruments=b", "Only check instrument interpretations.\n");

	define("K|no-key=b", "Do not check for !!!key: manual initial key designation.\n");
	define("k|only-key=b", "Only check for !!!key: manual initial key designation.\n");

	define("R|no-reference=b", "Do not check for reference records.\n");
	define("r|only-reference=b", "Only check for reference records.\n");

	define("T|no-termination|no-terminations=b", "Do not check spine terminations.\n");
	define("t|only-termination|only-terminations=b", "Only check spine terminations.\n");

	define("file|filename=b", "Print filename with raw count (if available).\n");
	define("raw=b", "Only print error count.\n");
}



/////////////////////////////////
//
// Tool_nproof::run -- Do the main work of the tool.
//

bool Tool_nproof::run(HumdrumFileSet& infiles) {
	bool status = true;
	for (int i=0; i<infiles.getCount(); i++) {
		status &= run(infiles[i]);
	}
	return status;
}


bool Tool_nproof::run(const string& indata, ostream& out) {
	HumdrumFile infile(indata);
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_nproof::run(HumdrumFile& infile, ostream& out) {
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_nproof::run(HumdrumFile& infile) {
	initialize();
	processFile(infile);
	return true;
}



//////////////////////////////
//
// Tool_nproof::initialize --
//

void Tool_nproof::initialize(void) {
	m_noblankQ       = getBoolean("no-blank");
	m_noinstrumentQ  = getBoolean("no-instrument");
	m_nokeyQ         = getBoolean("no-key");
	m_noreferenceQ   = getBoolean("no-reference");
	m_noterminationQ = getBoolean("no-termination");

	bool onlyBlank       = getBoolean("only-blank");
	bool onlyInstrument  = getBoolean("only-instrument");
	bool onlyKey         = getBoolean("only-key");
	bool onlyReference   = getBoolean("only-reference");
	bool onlyTermination = getBoolean("only-termination");

	if (onlyBlank || onlyInstrument || onlyKey || onlyReference || onlyTermination) {
		m_noblankQ       = !onlyBlank;
		m_noinstrumentQ  = !onlyInstrument;
		m_nokeyQ         = !onlyKey;
		m_noreferenceQ   = !onlyReference;
		m_noterminationQ = !onlyTermination;
	}

	m_fileQ          = getBoolean("file");
	m_rawQ           = getBoolean("raw");
}



//////////////////////////////
//
// Tool_nproof::processFile --
//

void Tool_nproof::processFile(HumdrumFile& infile) {
	m_errorCount = 0;
	m_errorList = "";
	m_errorHtml = "";

	if (!m_noblankQ) {
		checkForBlankLines(infile);
	}
	if (!m_nokeyQ) {
		checkKeyInformation(infile);
	}
	if (!m_noinstrumentQ) {
		checkInstrumentInformation(infile);
	}
	if (!m_noreferenceQ) {
		checkReferenceRecords(infile);
	}
	if (!m_noterminationQ) {
		checkSpineTerminations(infile);
	}

	m_humdrum_text << infile;

	if (m_rawQ) {
		// print error count only.
		if (m_fileQ) {
			m_free_text << infile.getFilename() << "\t";
		}
		m_free_text << m_errorCount << endl;
		return;
	}

	if (m_errorCount > 0) {
		m_humdrum_text << m_errorList;
		m_humdrum_text << "!!!TOOL-nproof-error-count: " << m_errorCount << endl;
		m_humdrum_text << "!!@@BEGIN: PREHTML\n";
		m_humdrum_text << "!!@TOOL: nproof\n";
		m_humdrum_text << "!!@CONTENT:\n";
		m_humdrum_text << "!! <h2 style='color:red'> @{TOOL-nproof-error-count} problem";
		if (m_errorCount != 1) {
			m_humdrum_text << "s";
		}
		m_humdrum_text << " detected </h2>\n";
		m_humdrum_text << "!! <ul style='color:darkred'>\n";
		m_humdrum_text << m_errorHtml;
		m_humdrum_text << "!! </ul>\n";
		m_humdrum_text << "!!@@END: PREHTML\n";
	} else {
		m_humdrum_text << "!!@@BEGIN: PREHTML\n";
		m_humdrum_text << "!!@TOOL: nproof\n";
		m_humdrum_text << "!!@CONTENT:\n";
		m_humdrum_text << "!! <h2 style='color:red'> No problems detected </h2>\n";
		m_humdrum_text << "!!@@END: PREHTML\n";
	}
}



//////////////////////////////
//
// Tool_nproof::checkForBlankLines --
//

void Tool_nproof::checkForBlankLines(HumdrumFile& infile) {
	vector<int> blanks;
	// -1: Not checking for a blank line at the very end of the score.
	for (int i=0; i<infile.getLineCount() - 1; i++) {
		if (infile[i].hasSpines()) {
			continue;
		}
		HTp token = infile.token(i, 0);
		if (*token == "") {
			blanks.push_back(i+1);
		}
	}

	if (blanks.empty()) {
		return;
	}

	m_errorCount++;
	m_errorList += "!!!TOOL-nproof-error-" + to_string(m_errorCount) + ": Blank lines on row";
	if (blanks.size() != 1) {
		m_errorList += "s";
	}
	m_errorList += ": ";
	for (int i=0; i<(int)blanks.size(); i++) {
		m_errorList += to_string(blanks[i]);
		if (i < (int)blanks.size() - 1) {
			m_errorList += ", ";
		}
	}
	m_errorList += ".\n";
	m_errorHtml += "!! <li> @{TOOL-nproof-error-" + to_string(m_errorCount) + "} </li>\n";
}



//////////////////////////////
//
// Tool_nproof::checkForValidInstrumentCode --
//

void Tool_nproof::checkForValidInstrumentCode(HTp token,
		vector<pair<string, string>>& instrumentList) {

	if ((token->find("&") == string::npos) && (token->find("|") == string::npos)) {
		string code = token->substr(2);
		for (int i=0; i<(int)instrumentList.size(); i++) {
			if (instrumentList[i].first == code) {
				return;
			}
		}

		m_errorCount++;
		m_errorList += "!!!TOOL-nproof-error-" + to_string(m_errorCount) + ": Unknown instrument code \"" + code + "\" on line " + to_string(token->getLineNumber()) + ", field " + to_string(token->getFieldNumber()) + ". See list of codes at <a target='_blank' href='https://bit.ly/humdrum-instrument-codes'>https://bit.ly/humdrum-instrument-codes</a>.\n";
		m_errorHtml += "!! <li> @{TOOL-nproof-error-" + to_string(m_errorCount) + "} </li>\n";
		return;
	}

	bool found1 = false;
	bool found2 = false;
	string inst1;
	string inst2;
	HumRegex hre;
	if (hre.match(token, "^\\*I(.*)[&|](I.*)")) {
		inst1 = hre.getMatch(1);
		inst2 = hre.getMatch(2);

		for (int i=0; i<(int)instrumentList.size(); i++) {
			if (instrumentList[i].first == inst1) {
				found1 = true;
			}
			if (instrumentList[i].first == inst2) {
				found2 = true;
			}
		}
	}

	if (!found1) {
		m_errorCount++;
		m_errorList += "!!!TOOL-nproof-error-" + to_string(m_errorCount) + ": Unknown instrument code \"" + inst1 + "\" in token " + *token + " on line " + to_string(token->getLineNumber()) + ", field " + to_string(token->getFieldNumber()) + ". See list of codes at <a target='_blank' href='https://bit.ly/humdrum-instrument-codes'>https://bit.ly/humdrum-instrument-codes</a>.\n";
		m_errorHtml += "!! <li> @{TOOL-nproof-error-" + to_string(m_errorCount) + "} </li>\n";
	}

	if (!found2) {
		m_errorCount++;
		m_errorList += "!!!TOOL-nproof-error-" + to_string(m_errorCount) + ": Unknown instrument code \"" + inst2 + "\" in token " + *token + " on line " + to_string(token->getLineNumber()) + ", field " + to_string(token->getFieldNumber()) + ". See list of codes at <a target='_blank' href='https://bit.ly/humdrum-instrument-codes'>https://bit.ly/humdrum-instrument-codes</a>.\n";
		m_errorHtml += "!! <li> @{TOOL-nproof-error-" + to_string(m_errorCount) + "} </li>\n";
	}

}



//////////////////////////////
//
// Tool_nproof::checkInstrumentInformation --
//

void Tool_nproof::checkInstrumentInformation(HumdrumFile& infile) {
	int codeLine = -1;
	int classLine = -1;
	HumRegex hre;

	vector<pair<string, string>> instrumentList = Convert::getInstrumentList();

	for (int i=0; i<infile.getLineCount(); i++) {
		if (infile[i].isData()) {
			break;
		}
		if (!infile[i].isInterpretation()) {
			continue;
		}
		if (infile[i].isManipulator()) {
			continue;
		}
		for (int j=0; j<infile[i].getFieldCount(); j++) {
			HTp token = infile.token(i, j);
			if (!token->isKern()) {
				continue;
			}
			if (token->compare(0, 3, "*IC") == 0) {
				if (classLine < 0) {
					classLine = i;
				}
			} else if (hre.search(token, "^\\*I[a-z]")) {
				if (codeLine < 0) {
					codeLine = i;
				}
			}
		}
	}

	if (codeLine < 0) {
		m_errorCount++;
		m_errorList += "!!!TOOL-nproof-error-" + to_string(m_errorCount) + ": No instrument code line.\n";
		m_errorHtml += "!! <li> @{TOOL-nproof-error-" + to_string(m_errorCount) + "} </li>\n";
	} else {
		for (int i=0; i<infile[codeLine].getFieldCount(); i++) {
			HTp token = infile.token(codeLine, i);
			if (token->isKern()) {
				if (!hre.search(token, "^\\*I[a-z]")) {
					m_errorCount++;
					m_errorList += "!!!TOOL-nproof-error-" + to_string(m_errorCount) + ": expected instrument code on line " + to_string(token->getLineNumber()) + ", field " + to_string(token->getFieldNumber()) + ".\n";
					m_errorHtml += "!! <li> @{TOOL-nproof-error-" + to_string(m_errorCount) + "} </li>\n";
				} else {
					checkForValidInstrumentCode(token, instrumentList);
				}
			} else {
				if (*token != "*") {
					m_errorCount++;
					m_errorList += "!!!TOOL-nproof-error-" + to_string(m_errorCount) + ": Expected null interpretation on instrument code line " + to_string(token->getLineNumber()) + ", field " + to_string(token->getFieldNumber()) + ".\n";
					m_errorHtml += "!! <li> @{TOOL-nproof-error-" + to_string(m_errorCount) + "} </li>\n";
				}
			}
		}
	}

	if (classLine < 0) {
		m_errorCount++;
		m_errorList += "!!!TOOL-nproof-error-" + to_string(m_errorCount) + ": No instrument class line.\n";
		m_errorHtml += "!! <li> @{TOOL-nproof-error-" + to_string(m_errorCount) + "} </li>\n";
	} else {
		for (int i=0; i<infile[classLine].getFieldCount(); i++) {
			HTp token = infile.token(classLine, i);
			if (token->isKern()) {
				if (!hre.search(token, "^\\*IC[a-z]")) {
					m_errorCount++;
					m_errorList += "!!!TOOL-nproof-error-" + to_string(m_errorCount) + ": expected instrument class on line " + to_string(token->getLineNumber()) + ", field " + to_string(token->getFieldNumber()) + ".\n";
					m_errorHtml += "!! <li> @{TOOL-nproof-error-" + to_string(m_errorCount) + "} </li>\n";
				}
			} else {
				if (*token != "*") {
					m_errorCount++;
					m_errorList += "!!!TOOL-nproof-error-" + to_string(m_errorCount) + ": Expected null interpretation on instrument class line " + to_string(token->getLineNumber()) + ", field " + to_string(token->getFieldNumber()) + ".\n";
					m_errorHtml += "!! <li> @{TOOL-nproof-error-" + to_string(m_errorCount) + "} </li>\n";
				}
			}
		}
	}
}



//////////////////////////////
//
// Tool_nproof::checkReferenceRecords --
//

void Tool_nproof::checkReferenceRecords(HumdrumFile& infile) {
	vector<int> foundENC;  // Musescore encoder's name
	vector<int> foundEND;  // Musescore encdoer's date
	vector<int> foundEED;  // VHV editor's name
	vector<int> foundEEV;  // VHV editor's date

	HumRegex hre;
	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isReferenceRecord()) {
			continue;
		}
		string key = infile[i].getReferenceKey();

		if (hre.search(key, "^EED\\d*$")) {
			if (key == "EED") {
				foundEED.push_back(i);
			}
			string value = infile[i].getReferenceValue();
			if (hre.search(value, "^\\d\\d\\d\\d")) {
				m_errorCount++;
				m_errorList += "!!!TOOL-nproof-error-" + to_string(m_errorCount) + ": For EED (Electronic EDitor) record on line " + to_string(i+1) + ", found a date rather than a name: " + value + ".\n";
				m_errorHtml += "!! <li> @{TOOL-nproof-error-" + to_string(m_errorCount) + "} </li>\n";
			}
		}
		if (hre.search(key, "^EEV\\d*$")) {
			if (key == "EEV") {
				foundEEV.push_back(i);;
			}
			string value = infile[i].getReferenceValue();
			if (!hre.search(value, "^\\d\\d\\d\\d-\\d\\d-\\d\\d")) {
				m_errorCount++;
				m_errorList += "!!!TOOL-nproof-error-" + to_string(m_errorCount) + ": For EEV (ElEctronic Version) record on line " + to_string(i+1) + ", found a name rather than a date (or invalid date): " + value + ".\n";
				m_errorHtml += "!! <li> @{TOOL-nproof-error-" + to_string(m_errorCount) + "} </li>\n";
			}
		}
		if (hre.search(key, "^ENC\\d*(-modern|-iiif)?$")) {
			string value = infile[i].getReferenceValue();
			if (hre.search(value, "^\\d\\d\\d\\d-\\d\\d-\\d\\d")) {
				m_errorCount++;
				m_errorList += "!!!TOOL-nproof-error-" + to_string(m_errorCount) + ": For ENC (Electronic eNCoder) record on line " + to_string(i+1) + ", found a date rather than a name: " + value + ".\n";
				m_errorHtml += "!! <li> @{TOOL-nproof-error-" + to_string(m_errorCount) + "} </li>\n";
			}
		}
		if (hre.search(key, "^END\\d*(-modern|-iiif)?$")) {
			string value = infile[i].getReferenceValue();
			if (!hre.search(value, "^\\d\\d\\d\\d-\\d\\d-\\d\\d")) {
				m_errorCount++;
				m_errorList += "!!!TOOL-nproof-error-" + to_string(m_errorCount) + ": For END (Electronic eNcoding Date) record on line " + to_string(i+1) + ", found a name rather than a date (or an invalid date): " + value + ".\n";
				m_errorHtml += "!! <li> @{TOOL-nproof-error-" + to_string(m_errorCount) + "} </li>\n";
			}
		}
		if (hre.search(key, "^ENC(\\d*.*)$")) {
				if (key == "ENC") {
					foundENC.push_back(i);
				}
		}
		if (hre.search(key, "^ENC-(\\d+.*)$")) {
				string newvalue = "ENC" + hre.getMatch(1);
				m_errorCount++;
				m_errorList += "!!!TOOL-nproof-error-" + to_string(m_errorCount) + ": " + key + " reference record on line " + to_string(i+1) + " should not include a dash and instead be: " + newvalue + ".\n";
				m_errorHtml += "!! <li> @{TOOL-nproof-error-" + to_string(m_errorCount) + "} </li>\n";
		}
		if (hre.search(key, "END(\\d*.*)")) {
				if (key == "END") {
					foundEND.push_back(i);
				}
		}
		if (hre.search(key, "^END-(\\d+.*)$")) {
				string newvalue = "END" + hre.getMatch(1);
				m_errorCount++;
				m_errorList += "!!!TOOL-nproof-error-" + to_string(m_errorCount) + ": " + key + " reference record on line " + to_string(i+1) + " should not include a dash and instead be: " + newvalue + ".\n";
				m_errorHtml += "!! <li> @{TOOL-nproof-error-" + to_string(m_errorCount) + "} </li>\n";
		}
		if (key == "filter-") {
				m_errorCount++;
				m_errorList += "!!!TOOL-nproof-error-" + to_string(m_errorCount) + ": \"filter-\" reference record on line " + to_string(i+1) + " should probably be \"filter-modern\" instead.\n";
				m_errorHtml += "!! <li> @{TOOL-nproof-error-" + to_string(m_errorCount) + "} </li>\n";
		}
		if (key == "ENC-mod") {
				m_errorCount++;
				m_errorList += "!!!TOOL-nproof-error-" + to_string(m_errorCount) + ": ENC-mod reference record on line " + to_string(i+1) + " should be ENC-modern instead.\n";
				m_errorHtml += "!! <li> @{TOOL-nproof-error-" + to_string(m_errorCount) + "} </li>\n";
		}
		if (key == "END-mod") {
				m_errorCount++;
				m_errorList += "!!!TOOL-nproof-error-" + to_string(m_errorCount) + ": END-mod reference record on line " + to_string(i+1) + " should be END-modern instead.\n";
				m_errorHtml += "!! <li> @{TOOL-nproof-error-" + to_string(m_errorCount) + "} </li>\n";
		}
		if (key == "AIN-mod") {
				m_errorCount++;
				m_errorList += "!!!TOOL-nproof-error-" + to_string(m_errorCount) + ": AIN-mod reference record on line " + to_string(i+1) + " should be AIN-modern instead.\n";
				m_errorHtml += "!! <li> @{TOOL-nproof-error-" + to_string(m_errorCount) + "} </li>\n";
		}
		if (hre.search(key, "^(.*)-ori$")) {
				string piece = hre.getMatch(1);
				m_errorCount++;
				m_errorList += "!!!TOOL-nproof-error-" + to_string(m_errorCount) + ": " + key + " reference record on line " + to_string(i+1) + " should not be used (either use " + piece + "-mod or don't add -ori qualifier to " + piece + ").\n";
				m_errorHtml += "!! <li> @{TOOL-nproof-error-" + to_string(m_errorCount) + "} </li>\n";
		}
	}

	// vector<int> foundENC;  // Musescore encoder's name
	// vector<int> foundEND;  // Musescore encdoer's date
	// vector<int> foundEED;  // VHV editor's name
	// vector<int> foundEEV;  // VHV editor's date

	if (foundENC.empty()) {
		m_errorCount++;
		m_errorList += "!!!TOOL-nproof-error-" + to_string(m_errorCount) + ": Missing ENC (initial encoder's name) reference record.\n";
		m_errorHtml += "!! <li> @{TOOL-nproof-error-" + to_string(m_errorCount) + "} </li>\n";
	}
	if (foundEND.empty()) {
		m_errorCount++;
		m_errorList += "!!!TOOL-nproof-error-" + to_string(m_errorCount) + ": Missing END (initial encoding date) reference record.\n";
		m_errorHtml += "!! <li> @{TOOL-nproof-error-" + to_string(m_errorCount) + "} </li>\n";
	}
	if (foundEED.empty()) {
		m_errorCount++;
		m_errorList += "!!!TOOL-nproof-error-" + to_string(m_errorCount) + ": Missing EED (Humdrum electronic editor's name) reference record.\n";
		m_errorHtml += "!! <li> @{TOOL-nproof-error-" + to_string(m_errorCount) + "} </li>\n";
	}
	if (foundEEV.empty()) {
		m_errorCount++;
		m_errorList += "!!!TOOL-nproof-error-" + to_string(m_errorCount) + ": Missing EEV (Humdrum electronic edition date) reference record.\n";
		m_errorHtml += "!! <li> @{TOOL-nproof-error-" + to_string(m_errorCount) + "} </li>\n";
	}

	if ((foundENC.size() == 1) && (foundEED.size() == 1)) {
		if (foundENC[0] > foundEED[0]) {
			m_errorCount++;
			m_errorList += "!!!TOOL-nproof-error-" + to_string(m_errorCount) + ": ENC reference record on line " + to_string(foundENC[0]+1) + " should come before EED reference record on line " + to_string(foundEED[0]+1) + "\n";
			m_errorHtml += "!! <li> @{TOOL-nproof-error-" + to_string(m_errorCount) + "} </li>\n";
		}
	}

	if ((foundEND.size() == 1) && (foundEEV.size() == 1)) {
		if (foundEND[0] > foundEEV[0]) {
			m_errorCount++;
			m_errorList += "!!!TOOL-nproof-error-" + to_string(m_errorCount) + ": END reference record on line " + to_string(foundEND[0]+1) + " should come before EEV reference record on line " + to_string(foundEEV[0]+1) + "\n";
			m_errorHtml += "!! <li> @{TOOL-nproof-error-" + to_string(m_errorCount) + "} </li>\n";
		}
	}


	if ((foundENC.size() == 2) && (foundEED.size() == 0)) {
		string date1;
		string date2;
		if (foundEND.size() == 2) {
			date1 = infile[foundEND[0]].getReferenceValue();
			date2 = infile[foundEND[1]].getReferenceValue();
			hre.replaceDestructive(date1, "", "-", "g");
			hre.replaceDestructive(date2, "", "-", "g");
			int number1 = 0;
			int number2 = 0;
			if (hre.search(date1, "^(20\\d{6})$")) {
				number1 = hre.getMatchInt(1);
			}
			if (hre.search(date2, "^(20\\d{6})$")) {
				number2 = hre.getMatchInt(1);
			}
			if ((number1 > 0) && (number2 > 0)) {
				if (number1 > number2) {
					m_errorCount++;
					m_errorList += "!!!TOOL-nproof-error-" + to_string(m_errorCount) + ": Second ENC reference record on line " + to_string(foundENC[1]+1) + " should probably be changed to EED reference record (and second END reference record on line " + to_string(foundEND[1]+1) + " changed to EEV).\n";
					m_errorHtml += "!! <li> @{TOOL-nproof-error-" + to_string(m_errorCount) + "} </li>\n";
				}
			}
		} else {
			m_errorCount++;
			m_errorList += "!!!TOOL-nproof-error-" + to_string(m_errorCount) + ": There are two ENC records on lines " + to_string(foundENC[0]+1) + " and " + to_string(foundENC[1]+1) + ". The Humdrum editor's name should be changed to EED, and the editing date should be changed from END to EEV if necessary.\n";
			m_errorHtml += "!! <li> @{TOOL-nproof-error-" + to_string(m_errorCount) + "} </li>\n";
		}
	}

}



//////////////////////////////
//
// Tool_nproof::checkKeyInformation --
//

void Tool_nproof::checkKeyInformation(HumdrumFile& infile) {
	int foundKey = -1;
	for (int i=0; i<infile.getLineCount(); i++) {
		if (infile[i].hasSpines()) {
			continue;
		}
		HTp token = infile.token(i, 0);
		if (token->compare(0, 7, "!!!key:") == 0) {
			foundKey = i;
			break;
		}
	}

	if (foundKey < 0) {
		m_errorCount++;
		m_errorList += "!!!TOOL-nproof-error-" + to_string(m_errorCount) + ": No <tt>!!!key:</tt> reference record.\n";
		m_errorHtml += "!! <li> @{TOOL-nproof-error-" + to_string(m_errorCount) + "} </li>\n";
		return;
	}

	string value = infile[foundKey].getReferenceValue();
	if (value.empty()) {
		m_errorCount++;
		m_errorList += "!!!TOOL-nproof-error-" + to_string(m_errorCount) + ": <tt>!!!key:</tt> reference record on line " + to_string(foundKey+1) + " should not be empty.  If no key, then use \"none\".\n";
		m_errorHtml += "!! <li> @{TOOL-nproof-error-" + to_string(m_errorCount) + "} </li>\n";
		return;
	}

	HumRegex hre;
	if (hre.search(value, "^([a-gA-G][#-n]?):(dor|phr|lyd|mix|aeo|loc|ion)$")) {
		string tonic = hre.getMatch(1);
		string mode  = hre.getMatch(2);
		int major = 0;
		if ((mode == "lyd") || (mode == "mix") || (mode == "ion")) {
			major = 1;
		}
		int uppercase = isupper(tonic[0]);
		if ((major == 1) && (uppercase == 0)) {
			tonic[0] = toupper(tonic[0]);
			string correct = tonic + ":" + mode;
			m_errorCount++;
			m_errorList += "!!!TOOL-nproof-error-" + to_string(m_errorCount) + ": <tt>!!!key:</tt> reference record on line " + to_string(foundKey + 1) + " should be \"" + correct + "\".\n";
			m_errorHtml += "!! <li> @{TOOL-nproof-error-" + to_string(m_errorCount) + "} </li>\n";
		} else if ((major == 0) && (uppercase == 1)) {
			tonic[0] = tolower(tonic[0]);
			string correct = tonic + ":" + mode;
			m_errorCount++;
			m_errorList += "!!!TOOL-nproof-error-" + to_string(m_errorCount) + ": <tt>!!!key:</tt> reference record on line " + to_string(foundKey + 1) + " should be \"" + correct + "\".\n";
			m_errorHtml += "!! <li> @{TOOL-nproof-error-" + to_string(m_errorCount) + "} </li>\n";
		}
	} else if (hre.search(value, "([a-gA-G][#-n]?):(.+)")) {
		string mode = hre.getMatch(2);
		m_errorCount++;
		m_errorList += "!!!TOOL-nproof-error-" + to_string(m_errorCount) + ": Unknown mode in <tt>!!!key:</tt> reference record contents on line " + to_string(foundKey + 1) + ": \"" + mode + "\".\n";
		m_errorHtml += "!! <li> @{TOOL-nproof-error-" + to_string(m_errorCount) + "} </li>\n";
	} else if (!hre.search(value, "([a-gA-G][#-n]?):?")) {
		m_errorCount++;
		m_errorList += "!!!TOOL-nproof-error-" + to_string(m_errorCount) + ": Unknown key designation in <tt>!!!key:</tt> reference record contents on line " + to_string(foundKey + 1) + ": \"" + value + "\".\n";
		m_errorHtml += "!! <li> @{TOOL-nproof-error-" + to_string(m_errorCount) + "} </li>\n";
	}

}



//////////////////////////////
//
// Tool_nproof::checkSpineTerminations --
//

void Tool_nproof::checkSpineTerminations(HumdrumFile& infile) {
	int foundTerminal = 0;
	for (int i=infile.getLineCount() - 1; i>0; i--) {
		if (!infile[i].isInterpretation()) {
			continue;
		}
		HTp token = infile.token(i, 0);
		if (*token == "*-") {
			foundTerminal = i;
			break;
		}
	}

	if (!foundTerminal) {
		m_errorCount++;
		m_errorList += "!!!TOOL-nproof-error-" + to_string(m_errorCount) + ": No spine terminators.\n";
		m_errorHtml += "!! <li> @{TOOL-nproof-error-" + to_string(m_errorCount) + "} </li>\n";
		return;
	}

	bool problem = false;
	for (int i=0; i<infile[foundTerminal].getFieldCount(); i++) {
		HTp token = infile[foundTerminal].token(i);
		string value = token->getSpineInfo();
		if (value.find(" ") != string::npos) {
			problem = true;
			break;
		}
	}

	if (!problem) {
		return;
	}

	m_errorCount++;
	m_errorList += "!!!TOOL-nproof-error-" + to_string(m_errorCount) + ": Incorrect spine merger(s): ";
	for (int i=0; i<infile[foundTerminal].getFieldCount(); i++) {
		HTp token = infile[foundTerminal].token(i);
		m_errorList += "<" + token->getSpineInfo() + ">";
		if (i < infile[foundTerminal].getFieldCount() - 1) {
			m_errorList += " ";
		}
	}
	m_errorList += "\n";
	m_errorHtml += "!! <li> @{TOOL-nproof-error-" + to_string(m_errorCount) + "} </li>\n";
}



// END_MERGE

} // end namespace hum



