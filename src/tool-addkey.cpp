//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Thu Apr  4 23:50:20 PDT 2024
// Last Modified: Thu Apr  4 23:50:24 PDT 2024
// Filename:      tool-addkey.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/tool-addkey.cpp
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Insert key designation from -k option or from !!!key: reference
//                record.
//
// Options:
//                No option will insert the !!!key: reference value into a
//                key designation in the data or create an insert one if there
//                is no key designation line.
//                -k key == Insert the given key.
//                -K     == Insert the given key in a !!!key: reference
//                          record as well.
//
// Note: not all cases implemented yet
//

#include "tool-addkey.h"
#include "HumRegex.h"

#include <vector>

using namespace std;

namespace hum {

// START_MERGE


/////////////////////////////////
//
// Tool_addkey::Tool_addkey -- Set the recognized options for the tool.
//

Tool_addkey::Tool_addkey(void) {
	define("k|key=s",           "Add given key designtation to data");
	define("K|reference-key=b", "Update or add !!!key: designation, used with -k");
}



/////////////////////////////////
//
// Tool_addkey::run -- Do the main work of the tool.
//

bool Tool_addkey::run(HumdrumFileSet& infiles) {
	bool status = true;
	for (int i=0; i<infiles.getCount(); i++) {
		status &= run(infiles[i]);
	}
	return status;
}


bool Tool_addkey::run(const string& indata, ostream& out) {
	HumdrumFile infile(indata);
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_addkey::run(HumdrumFile& infile, ostream& out) {
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_addkey::run(HumdrumFile& infile) {
	processFile(infile);
	return true;
}



//////////////////////////////
//
// Tool_addkey::initialize --  Initializations that only have to be done once
//    for all HumdrumFile segments.
//

void Tool_addkey::initialize(void) {
	m_addKeyRefQ = getBoolean("reference-key");
	m_keyQ       = getBoolean("key");
	m_key        = getString("key");
	HumRegex hre;
	hre.replaceDestructive(m_key, "", ":$");
	hre.replaceDestructive(m_key, "", "^\\*");
}



//////////////////////////////
//
// Tool_addkey::processFile --
//

void Tool_addkey::processFile(HumdrumFile& infile) {
	initialize();
	if (m_keyQ) {
		addInputKey(infile);
	} else {
		insertReferenceKey(infile);
	}
}



//////////////////////////////
//
// Tool_addkey::addInputKey -- Insert key from -k option into score
//    rather than !!!key: entry.  Insert input into !!!key: entry if
//    -K option is given.
//

void Tool_addkey::addInputKey(HumdrumFile& infile) {
	getLineIndexes(infile);

	if (m_refKeyIndex != -1) {
		string text = "!!!key: " + m_key;
		infile[m_refKeyIndex].token(0)->setText(text);
	}

	HumRegex hre;
	string keyDesig = "*" + m_key;
	if (!hre.search(m_key, ":")) {
		keyDesig += ":";
	}
	insertKeyDesig(infile, keyDesig);

	// Update the reference key record if -K option is used:
	if (m_addKeyRefQ) {
		if (m_refKeyIndex != -1) {
			string text = "!!!key: " + m_key;
			infile[m_refKeyIndex].setText(text);
		}
		// Or print just before exinterp line later if not found,
		// but needs to be created.
	}
}



//////////////////////////////
//
// Tool_addkey::insertKeyDesig --
//

void Tool_addkey::insertKeyDesig(HumdrumFile& infile, const string& keyDesig) {
	// Replace the key designation if any are found in the header.
	// If not found, then store in key signature for printing later.
	for (int i=0; i<infile.getLineCount(); i++) {
		if (i >= m_dataStartIndex) {
			break;
		}
		if (!infile[i].isInterpretation()) {
			continue;
		}
		for (int j=0; j<infile[i].getFieldCount(); j++) {
			HTp token = infile.token(i, j);
			if (token->isKeyDesignation()) {
				token->setText(keyDesig);
			} else if ((m_keyDesigIndex == -1) && (token->isKeySignature())) {
				// Store keyDesig later to print:
				token->setValue("auto", "keyDesig", keyDesig);
			}
		}
	}
}



//////////////////////////////
//
// Tool_addkey::insertReferenceKey -- Take the !!!key: value and insert
// into key designations in the header.  Add key designation line if not
// present already in header.
//

void Tool_addkey::insertReferenceKey(HumdrumFile& infile) {
	getLineIndexes(infile);

	if (m_refKeyIndex == -1) {
		// Nothing to do, add later before exinterp line.
		return;
	}

	HumRegex hre;
	string keyValue = infile[m_refKeyIndex].getReferenceValue();
	if (!hre.search(keyValue, ":")) {
		keyValue += ":";
	}
	if (!hre.search(keyValue, "^\\*")) {
		hre.replaceDestructive(keyValue, "*", "^");
	}
	if (m_keyDesigIndex > 0) {
		for (int i=m_exinterpIndex+1; i<=m_keyDesigIndex; i++) {
			if (!infile[i].isInterpretation()) {
				continue;
			}
			for (int j=0; j<infile[i].getFieldCount(); j++) {
				HTp token = infile.token(i, j);
				if (!token->isKeyDesignation()) {
					continue;
				}
				string text = "*" + keyValue;
				token->setText(text);
			}
		}
		infile.generateLinesFromTokens();
		m_humdrum_text << infile;
	} else if (m_keySigIndex > 0) {
		printKeyDesig(infile, m_keySigIndex, keyValue, +1);
	} else if (m_dataStartIndex > 0) {
		printKeyDesig(infile, m_dataStartIndex, keyValue, -1);
	}
}



//////////////////////////////
//
// Tool_addkey::printKeyDesig --
//

void Tool_addkey::printKeyDesig(HumdrumFile& infile, int index, const string& desig, int direction) {
	int index2 = index + direction;
	for (int i=0; i<infile.getLineCount(); i++) {
		if (i != index2) {
			m_humdrum_text << infile[i] << endl;
		} else {
			if (index > index2) {
				m_humdrum_text << infile[i] << endl;
			}
			for (int j=0; j<infile[index].getFieldCount(); j++) {
				HTp token = infile.token(index, j);
				if (j > 0) {
					m_humdrum_text << "\t";
				}
				if (token->isKern()) {
					m_humdrum_text << desig;
				} else {
					m_humdrum_text << "*";
				}
			}
			m_humdrum_text << endl;
			if (index < index2) {
				m_humdrum_text << infile[i] << endl;
			}
		}
	}
}



//////////////////////////////
//
// Tool_addkey::getLineIndexes --
//

void Tool_addkey::getLineIndexes(HumdrumFile& infile) {
	m_refKeyIndex    = -1;
	m_keyDesigIndex  = -1;
	m_keySigIndex    = -1;
	m_dataStartIndex = -1;

	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].hasSpines()) {
			continue;
		}
		if (infile[i].isExclusiveInterpretation()) {
			m_exinterpIndex = i;
			continue;
		}
		if (infile[i].isData()) {
			m_dataStartIndex = i;
			break;
		}
		if (infile[i].isBarline()) {
			m_dataStartIndex = i;
			break;
		}
		if (infile[i].token(0)->compare(0, 7, "!!!key:") == 0) {
			m_refKeyIndex = i;
		}
		if (!infile[i].isInterpretation()) {
			continue;
		}
		for (int j=0; j<infile[i].getFieldCount(); j++) {
			HTp token = infile.token(i, j);
			if (token->isKeySignature()) {
				m_keySigIndex = i;
			} else if (token->isKeyDesignation()) {
				m_keyDesigIndex = i;
			}
		}
	}

	if (m_refKeyIndex == -1) {
		// !!!key: could be at bottom, so search backwards in file.
		for (int i=infile.getLineCount() - 1; i>=0; i--) {
			if (!infile[i].isReference()) {
				continue;
			}
			string key = infile[i].getReferenceKey();
			if (key == "key") {
				m_refKeyIndex   = i;
				break;
			}
		}
	}
}


// END_MERGE

} // end namespace hum



