//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Tue Jul 22 2025
// Filename:      tool-barnum.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/tool-barnum.cpp
// Syntax:        C++11; humlib
//

#include "tool-barnum.h"
#include "Convert.h"
#include <sstream>

using namespace std;

namespace hum {

// START_MERGE

Tool_barnum::Tool_barnum(void) {
	define("r|remove=b", "Remove bar numbers from the file");
	define("s|start=i:1", "Starting bar number");
	define("a|all=b",     "Print numbers on all barlines");
	define("debug=b",     "Print debug info");
}


bool Tool_barnum::run(HumdrumFileSet& infiles) {
	bool status = true;
	for (int i=0; i<infiles.getCount(); i++) {
		status &= run(infiles[i]);
	}
	return status;
}


bool Tool_barnum::run(HumdrumFile& infile) {
	initialize();
	if (m_removeQ) {
		removeBarNumbers(infile);
	} else {
		renumberBarNumbers(infile);
	}
	return true;
}


void Tool_barnum::initialize(void) {
	m_removeQ  = getBoolean("remove");
	m_startnum = getInteger("start");
	m_allQ     = getBoolean("all");
	m_debugQ   = getBoolean("debug");
}


void Tool_barnum::removeBarNumbers(HumdrumFile& infile) {
	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isBarline()) {
			m_humdrum_text << infile[i] << "\n";
			continue;
		}
		printWithoutBarNumbers(infile[i]);
	}
}


void Tool_barnum::printWithoutBarNumbers(HumdrumLine& line) {
	for (int i=0; i<line.getFieldCount(); i++) {
		string token = *line.token(i);
		stringstream out;
		if (token.empty() || token[0] != '=') {
			out << token;
		} else {
			for (char c : token) {
				if (!isdigit(c)) out << c;
			}
		}
		m_humdrum_text << out.str();
		if (i < line.getFieldCount()-1) m_humdrum_text << "\t";
	}
	m_humdrum_text << "\n";
}


void Tool_barnum::printWithBarNumbers(HumdrumLine& line, int number) {
	for (int i=0; i<line.getFieldCount(); i++) {
		string token = *line.token(i);
		stringstream out;
		bool numbered = false;
		for (size_t j=0; j<token.size(); j++) {
			if (token[j] == '=' && (j+1 == token.size() || !isdigit(token[j+1]))) {
				out << '=' << number;
				numbered = true;
			} else if (!isdigit(token[j]) || !numbered) {
				out << token[j];
			}
		}
		m_humdrum_text << out.str();
		if (i < line.getFieldCount()-1) m_humdrum_text << "\t";
	}
	m_humdrum_text << "\n";
}


void Tool_barnum::renumberBarNumbers(HumdrumFile& infile) {
	infile.analyzeRhythmStructure();
	vector<int> barlines;
	for (int i=0; i<infile.getLineCount(); i++) {
		if (infile[i].isBarline()) {
			barlines.push_back(i);
		}
	}

	int number = m_startnum;
	int index = 0;
	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isBarline()) {
			m_humdrum_text << infile[i] << "\n";
			continue;
		}
		if (m_allQ || index == 0 || index == (int)barlines.size()-1) {
			printWithBarNumbers(infile[i], number++);
		} else {
			printWithoutBarNumbers(infile[i]);
		}
		index++;
	}
}

// END_MERGE

} // end namespace hum

