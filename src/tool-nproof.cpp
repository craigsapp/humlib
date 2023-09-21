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
#include "HumRegex.h"
#include "Convert.h"

using namespace std;

namespace hum {

// START_MERGE


/////////////////////////////////
//
// Tool_nproof::Tool_nproof -- Set the recognized options for the tool.
//

Tool_nproof::Tool_nproof(void) {
	define("K|no-key=b", "Do not check for !!!key: reference record.\n");
	define("I|no-instrument|no-instruments=b", "Do not check instrument interpretations.\n");
	define("T|no-termination=b", "Do not check spine termination.\n");
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
	m_nokeyQ         = getBoolean("no-key");
	m_noinstrumentQ  = getBoolean("no-instrument");
	m_noterminationQ = getBoolean("no-termination");
}



//////////////////////////////
//
// Tool_nproof::processFile --
//

void Tool_nproof::processFile(HumdrumFile& infile) {
	m_errorCount = 0;
	m_errorList = "";
	m_errorHtml = "";

	if (!m_nokeyQ) {
		checkKeyInformation(infile);
	}
	if (!m_noinstrumentQ) {
		checkInstrumentInformation(infile);
	}
	if (!m_noterminationQ) {
		checkSpineTerminations(infile);
	}

	m_humdrum_text << infile;

	if (m_errorCount > 0) {
		m_humdrum_text << m_errorList;
		m_humdrum_text << "!!!TOOL-nproof-error-count: " << m_errorCount << endl;
		m_humdrum_text << "!!@@BEGIN: PREHTML\n";
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
		m_humdrum_text << "!!@CONTENTS:\n";
		m_humdrum_text << "!! <h2 style='color:red'> No problems detected </h2>\n";
		m_humdrum_text << "!!@@END: PREHTML\n";
	}
}


//////////////////////////////
//
// Tool_nproof::checkForValidInstrumentCode --
//

void Tool_nproof::checkForValidInstrumentCode(HTp token,
		vector<pair<string, string>>& instrumentList) {
	string code = token->substr(2);
	for (int i=0; i<(int)instrumentList.size(); i++) {
		if (instrumentList[i].first == code) {
			return;
		}
	}

	m_errorCount++;
	m_errorList += "!!!TOOL-nproof-error-" + to_string(m_errorCount) + ": Unknown instrument code \"" + code + "\" on line " + to_string(token->getLineNumber()) + ", field " + to_string(token->getFieldNumber()) + ".\n";
	m_errorHtml += "!! <li> @{TOOL-nproof-error-" + to_string(m_errorCount) + "} </li>\n";
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
			if (token->compare("*IC") == 0) {
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

	if (foundKey >= 0) {
		return;
	}

	m_errorCount++;
	m_errorList += "!!!TOOL-nproof-error-" + to_string(m_errorCount) + ": No <tt>!!!key:</tt> reference record.\n";
	m_errorHtml += "!! <li> @{TOOL-nproof-error-" + to_string(m_errorCount) + "} </li>\n";
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



