//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Wed Apr 17 20:25:15 PDT 2024
// Last Modified: Wed Apr 17 20:25:18 PDT 2024
// Filename:      tool-addlabels.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/tool-addlabels.cpp
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Options:       -d "A,A,B,B"     Set the default expansion list.
//                -r "A,B"         Set the norep expansion list.
//                -l "m0:A; m9:B"  Place labels at given measures.
//
// Description:   Add labels and expansion lists to a Humdrum file.
//

#include "tool-addlabels.h"
#include "HumRegex.h"

#include <vector>

using namespace std;

namespace hum {

// START_MERGE


/////////////////////////////////
//
// Tool_addlabels::Tool_addlabels -- Set the recognized options for the tool.
//

Tool_addlabels::Tool_addlabels(void) {
	define("d|default=s:", "Default expansion list");
	define("r|norep=s:",   "norep expansion list");
	define("l|labels=s:",  "List of labels to insert");
}



/////////////////////////////////
//
// Tool_addlabels::run -- Do the main work of the tool.
//

bool Tool_addlabels::run(HumdrumFileSet& infiles) {
	bool status = true;
	for (int i=0; i<infiles.getCount(); i++) {
		status &= run(infiles[i]);
	}
	return status;
}


bool Tool_addlabels::run(const string& indata, ostream& out) {
	HumdrumFile infile(indata);
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_addlabels::run(HumdrumFile& infile, ostream& out) {
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_addlabels::run(HumdrumFile& infile) {
	processFile(infile);
	return true;
}



//////////////////////////////
//
// Tool_addlabels::initialize -- Process input options.
//

void Tool_addlabels::initialize(void) {
	m_default = getString("default");
	m_norep   = getString("norep");
	m_zeroth.clear();

	HumRegex hre;
	if (!m_default.empty()) {
		if (!hre.search(m_default, "^\\[")) {
			m_default = "[" + m_default;
		}
		if (!hre.search(m_default, "\\]$")) {
			m_default += "]";
		}
	}
	if (!m_norep.empty()) {
		if (!hre.search(m_norep, "^\\[")) {
			m_norep = "[" + m_norep;
		}
		if (!hre.search(m_norep, "\\]$")) {
			m_norep += "]";
		}
	}

	string value = getString("labels");
	hre.replaceDestructive(value, "", "^[\\s;,]+");
	hre.replaceDestructive(value, "", "[\\s;,]+$");

	vector<string> pieces;
	hre.split(pieces, value, "\\s*[;,]\\s*");
	for (int i=0; i<(int)pieces.size(); i++) {
		if (hre.search(pieces[i], "^\\s$")) {
			continue;
		}
		if (hre.search(pieces[i], "^\\s*m?\\s*(\\d+)([a-z]?)\\s*:\\s*(.+)\\s*$")) {
			int barnum = hre.getMatchInt(1);
			string sub = hre.getMatch(2);
			int subbar = 0;
			if (!sub.empty()) {
				subbar = sub[0] - 'a';
			}
			string label = hre.getMatch(3);

			if ((barnum <= 0) && (subbar <= 0)) {
				m_zeroth = label;
				continue;
			}
			m_barnums.push_back(barnum);
			m_subbarnums.push_back(subbar);
			m_labels.push_back(label);
		} else {
			cerr << "Error parsing label (ignoring): " << pieces[i] << endl;
		}
	}
}



//////////////////////////////
//
// Tool_addlabels::processFile --
//

void Tool_addlabels::processFile(HumdrumFile& infile) {
	initialize();

	vector<string> llist;
	assignLabels(llist, infile);

	m_defaultIndex = getExpansionIndex(infile);

	for (int i=0; i<infile.getLineCount(); i++) {
		if (i == m_defaultIndex) {
			printExpansionLists(infile, i);
			m_humdrum_text << infile[i] << endl;
			continue;
		}
		m_humdrum_text << infile[i] << endl;
		if (!llist.at(i).empty()) {
			for (int j=0; j<infile[i].getFieldCount(); j++) {
				m_humdrum_text << "*>" << llist.at(i);
				if (j < infile[i].getFieldCount() - 1) {
					m_humdrum_text << "\t";
				}
			}
			m_humdrum_text << endl;
		}
	}
}



//////////////////////////////
//
// Tool_addlabels::getExpsnsionIndex -- Return index that is where the
//    expansion labels and 0th label should be printed ABOVE.
//

int Tool_addlabels::getExpansionIndex(HumdrumFile& infile) {
	int staffIndex    = -1;
	int partIndex     = -1;
	int groupIndex    = -1;
	int instIndex     = -1;
	int abbrIndex     = -1;
	int clefIndex     = -1;
	int keySigIndex   = -1;
	int keyDesigIndex = -1;
	int exIndex       = -1;

	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].hasSpines()) {
			continue;
		}
		if (infile[i].isExclusiveInterpretation()) {
			exIndex = i;
			continue;
		}
		if (infile[i].isData()) {
			break;
		}
		if (infile[i].isBarline()) {
			break;
		}
		if (!infile[i].isInterpretation()) {
			continue;
		}
		for (int j=0; j<infile[i].getFieldCount(); j++) {
			HTp token = infile.token(i, j);
			if (*token == "*") {
				continue;
			}
			if ((clefIndex < 0) && token->isClef()) {
				clefIndex = i;
			}
			if ((instIndex < 0) && token->compare(0, 3, "*I\"") == 0) {
				instIndex = i;
			}
			if ((abbrIndex < 0) && token->compare(0, 3, "*I'") == 0) {
				abbrIndex = i;
			}
			if ((keySigIndex < 0) && token->isKeySignature()) {
				keySigIndex = i;
			}
			if ((keyDesigIndex < 0) && token->isKeyDesignation()) {
				keyDesigIndex = i;
			}
			if ((staffIndex != i) && (token->compare(0, 6, "*staff") == 0)) {
				staffIndex = i;
			}
			if ((partIndex != i) && (token->compare(0, 5, "*part") == 0)) {
				partIndex = i;
			}
			if ((groupIndex != i) && (token->compare(0, 6, "*group") == 0)) {
				groupIndex = i;
			}
		}
	}

	int spigaIndex = staffIndex;
	if (partIndex > spigaIndex) {
		spigaIndex = partIndex;
	}
	if (groupIndex > spigaIndex) {
		spigaIndex = groupIndex;
	}
	if (instIndex > spigaIndex) {
		spigaIndex = instIndex;
	}
	if (abbrIndex > spigaIndex) {
		spigaIndex = abbrIndex;
	}

	if (spigaIndex > 0) {
		return spigaIndex + 1;
	}

	int tindex = -1;

	if ((clefIndex > 0) && (tindex > 0)) {
		tindex = clefIndex;
	}

	if ((keySigIndex > 0) && (tindex > 0)) {
		if (keySigIndex < tindex) {
			tindex = keySigIndex;
		}
	}

	if ((keyDesigIndex > 0) && (tindex > 0)) {
		if (keyDesigIndex < tindex) {
			tindex = keyDesigIndex;
		}
	}
	if (tindex > 0) {
		if (exIndex < tindex - 1) {
			return tindex;
		}
	}
	return exIndex + 1;
}



//////////////////////////////
//
// Tool_addlabels::printExpansionLists -- printing above given line index
//   But use field count of next spined line in data if target line is
//   unspined.
//

void Tool_addlabels::printExpansionLists(HumdrumFile& infile, int index) {
	int ii = -1;
	for (int i=index; i<infile.getLineCount(); i++) {
		if (infile[i].hasSpines()) {
			ii = i;
			break;
		}
	}
	if (ii < 0) {
		cerr << "STRANGE ERROR ON LINE: " << infile[index] << endl;
	}

	if (!m_default.empty()) {
		for (int j=0; j<infile[ii].getFieldCount(); j++) {
			m_humdrum_text << "*>" << m_default;
			if (j < infile[ii].getFieldCount() - 1) {
				m_humdrum_text << "\t";
			}
		}
		m_humdrum_text << endl;
	}

	if (!m_norep.empty()) {
		for (int j=0; j<infile[ii].getFieldCount(); j++) {
			m_humdrum_text << "*>norep" << m_norep;
			if (j < infile[ii].getFieldCount() - 1) {
				m_humdrum_text << "\t";
			}
		}
		m_humdrum_text << endl;
	}

	if (!m_zeroth.empty()) {
		for (int j=0; j<infile[ii].getFieldCount(); j++) {
			m_humdrum_text << "*>" << m_zeroth;
			if (j < infile[ii].getFieldCount() - 1) {
				m_humdrum_text << "\t";
			}
		}
		m_humdrum_text << endl;
	}
}



//////////////////////////////
//
// Tool_addlabels::assignLabels -- assign labels to specific lines.
//

void Tool_addlabels::assignLabels(vector<string>& llist, HumdrumFile& infile) {
	llist.resize(infile.getLineCount());
	for (int i=0; i<(int)m_barnums.size(); i++) {
		addLabel(llist, infile, m_barnums.at(i), m_subbarnums.at(i), m_labels.at(i));
	}
}



//////////////////////////////
//
// Tool_addlabels::addLabel -- Add specified tempo to list.
//

void Tool_addlabels::addLabel(vector<string>& llist, HumdrumFile& infile,
		int barnum, int subbarnum, const string& label) {

	if (barnum <= 0) {
		return;
	}

	// find barnum index:
	int barIndex = -1;
	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isBarline()) {
			continue;
		}
		int bar = infile[i].getBarNumber();
		if (bar == barnum) {
			barIndex = i;
			break;
		}
	}
	if (barIndex < 0) {
		cerr << "WARNING: could not find measure number " << barnum << endl;
		return;
	}

	int counter = 0;
	if (subbarnum > 0) {
		for (int i=barIndex + 1; i<infile.getLineCount(); i++) {
			if (!infile[i].isBarline()) {
				continue;
			}
			counter++;
			if (counter >= subbarnum) {
				barIndex = i;
				break;
			}
		}
	}

	if (barIndex < 0) {
		return;
	}

	// insert at the next spined line (but figure that out later):
	llist.at(barIndex) = label;
}


// END_MERGE

} // end namespace hum



