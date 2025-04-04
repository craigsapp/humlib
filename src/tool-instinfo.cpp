//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Apr 15 00:21:49 PDT 2024
// Last Modified: Thu May  2 01:08:42 PDT 2024
// Filename:      tool-instinfo.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/tool-instinfo.cpp
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Options:       -q # = set tempo in units of quarter notes per minute.
//                -q "#; m#:#; m#:#" = set three tempos, one at start, and two
//                at given measures.
//
// Description:   Add instrument info such as:
//                *ICstr           == instrument class (strings)
//                *Icello          == instrument code (Violoncello)
//                *I#2             == instrument number (second cello(s))
//                *I"Violoncello   == instrument name
//                *I'Vc.           == instrument abbreviation
//

#include "tool-instinfo.h"
#include "Convert.h"
#include "HumRegex.h"

#include <algorithm>
#include <tuple>
#include <utility>
#include <vector>

using namespace std;

namespace hum {

// START_MERGE


/////////////////////////////////
//
// Tool_instinfo::Tool_instinfo -- Set the recognized options for the tool.
//

Tool_instinfo::Tool_instinfo(void) {
	define("c|instrument-class=s",        "instrument class by kern spine");
	define("i|instrument-code=s",         "instrument codes by kern spine");
	define("m|instrument-number=s",       "instrument number by kern spine");
	define("n|instrument-name=s",         "instrument name by kern spine");
	define("a|instrument-abbreviation=s", "instrument class by kern spine");
}



/////////////////////////////////
//
// Tool_instinfo::run -- Do the main work of the tool.
//

bool Tool_instinfo::run(HumdrumFileSet& infiles) {
	bool status = true;
	for (int i=0; i<infiles.getCount(); i++) {
		status &= run(infiles[i]);
	}
	return status;
}


bool Tool_instinfo::run(const string& indata, ostream& out) {
	HumdrumFile infile(indata);
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_instinfo::run(HumdrumFile& infile, ostream& out) {
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_instinfo::run(HumdrumFile& infile) {
	processFile(infile);
	return true;
}



//////////////////////////////
//
// Tool_instinfo::initialize --  Initializations that only have to be done once
//    for all HumdrumFile segments.
//

void Tool_instinfo::initialize(HumdrumFile& infile) {
	HumRegex hre;

	vector<HTp> kernstarts;
	kernstarts = infile.getKernSpineStartList();
	int ksize = (int)kernstarts.size();


	// Store instrument class, such as *Iww
	// Separated by semicolons and/or spaces
	string iclass = Convert::trimWhiteSpace(getString("instrument-class"));
	vector<string> pieces;
	hre.split(pieces, iclass, "[\\s:;,]+");
	if ((iclass.find(":") == string::npos) && ((int)pieces.size() == ksize)) {
		for (int i=0; i<ksize; i++) {
			m_iclass[i] = pieces[i];
		}
	} else {
		while (hre.search(iclass, "\\s*k\\s*(\\d+)\\s*:\\s*([^\\s]+)\\s*;?\\s*")) {
			int k = hre.getMatchInt(1);
			string iiclass = hre.getMatch(2);
			m_iclass[k-1] = iiclass;
			hre.replaceDestructive(iclass, "", "\\s*k\\s*(\\d+)\\s*:\\s*([^\\s]+)\\s*;?\\s*");
		}
	}

	// Store instrument codes, such as *Iflt
	// Separated by semicolons and/or spaces
	string icode = Convert::trimWhiteSpace(getString("instrument-code"));
	hre.split(pieces, icode, "[\\s:;,]+");
	if ((icode.find(":") == string::npos) && ((int)pieces.size() == ksize)) {
		for (int i=0; i<ksize; i++) {
			m_icode[i] = pieces[i];
		}
	} else {
		while (hre.search(icode, "\\s*k\\s*(\\d+)\\s*:\\s*([^\\s]+)\\s*;?\\s*")) {
			int k = hre.getMatchInt(1);
			string code = hre.getMatch(2);
			m_icode[k-1] = code;
			hre.replaceDestructive(icode, "", "\\s*k\\s*(\\d+)\\s*:\\s*([^\\s]+)\\s*;?\\s*");
		}
	}

	// Store instrument number, such as *I#2
	// The # before number is optional (and will be added automatically)
	// Separated by semicolons and/or spaces
	string inum = Convert::trimWhiteSpace(getString("instrument-number"));
	hre.split(pieces, inum, "[\\s:;,]+");
	if ((inum.find(":") == string::npos) && ((int)pieces.size() == ksize)) {
		for (int i=0; i<ksize; i++) {
			string value = pieces[i];
			if (!value.empty()) {
				if (value[0] != '#') {
					value = "#" + value;
				}
			}
			m_inum[i] = value;
		}
	} else {
		while (hre.search(inum, "\\s*k\\s*(\\d+)\\s*:\\s*#?\\s*([\\d]+)\\s*;?\\s*")) {
			int k = hre.getMatchInt(1);
			string num = "#" + hre.getMatch(2);
			m_inum[k-1] = num;
			hre.replaceDestructive(inum, "", "\\s*k\\s*(\\d+)\\s*:\\s*#?\\s*([\\d]+)\\s*;?\\s*");
		}
	}

	// Store instrument name, such as *I"flt
	// The names must be separated by semicolons.
	string iname = Convert::trimWhiteSpace(getString("instrument-name"));
	hre.split(pieces, iname, "\\s*;\\s*");
	if ((!hre.search(iname, "k\\s*\\d+\\s*:")) && ((int)pieces.size() == ksize)) {
		for (int i=0; i<ksize; i++) {
			m_iname[i] = pieces[i];
		}
	} else {
		while (hre.search(iname, "\\s*k\\s*(\\d+)\\s*:\\s*([^;]+)\\s*(;|$)\\s*")) {
			int k = hre.getMatchInt(1);
			string name = hre.getMatch(2);
			m_iname[k-1] = name;
			hre.replaceDestructive(iname, "", "\\s*k\\s*(\\d+)\\s*:\\s*([^;]+)");
		}
	}

	// Store instrument abbreviation, such as *I'fl.
	// The abbreviations must be separated by semicolons.
	string iabbr = Convert::trimWhiteSpace(getString("instrument-abbreviation"));
	hre.split(pieces, iabbr, "\\s*;\\s*");
	if ((!hre.search(iabbr, "k\\s*\\d+\\s*:")) && ((int)pieces.size() == ksize)) {
		for (int i=0; i<ksize; i++) {
			m_iabbr[i] = pieces[i];
		}
	} else {
		while (hre.search(iabbr, "\\s*k\\s*(\\d+)\\s*:\\s*([^;]+)\\s*(;|$)\\s*")) {
			int k = hre.getMatchInt(1);
			string abbr = hre.getMatch(2);
			m_iabbr[k-1] = abbr;
			hre.replaceDestructive(iabbr, "", "\\s*k\\s*(\\d+)\\s*:\\s*([^;]+)");
		}
	}

}



//////////////////////////////
//
// Tool_instinfo::processFile --
//

void Tool_instinfo::processFile(HumdrumFile& infile) {
	initialize(infile);
	vector<HTp> kspines;
	kspines = infile.getKernSpineStartList();
	vector<int> ktracks(kspines.size(), -1);
	for (int i=0; i<(int)kspines.size(); i++) {
		ktracks[i] = kspines[i]->getTrack();
	}
	map<int, int> track2kindex;
	for (int i=0; i<(int)ktracks.size(); i++) {
		track2kindex[ktracks[i]] = i+1;
	}

	int gpsIndex      = -1;
	int exinterpIndex = -1;
	int iclassIndex   = -1;
	int icodeIndex    = -1;
	int inameIndex    = -1;
	int iabbrIndex    = -1;
	int inumIndex     = -1;

	HumRegex hre;

	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].hasSpines()) {
			continue;
		}
		if (infile[i].isExclusiveInterpretation()) {
			exinterpIndex = i;
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
			if (!token->isKern()) {
				continue;
			}
			if (hre.search(token, "^\\*staff\\d")) {
				gpsIndex = i;
			}
			if (hre.search(token, "^\\*part\\d")) {
				gpsIndex = i;
			}
			if (hre.search(token, "^\\*group\\d")) {
				gpsIndex = i;
			}
			if (token->isInstrumentClass()) {
				iclassIndex = i;
			}
			if (token->isInstrumentCode()) {
				icodeIndex = i;
			}
			if (token->isInstrumentName()) {
				inameIndex = i;
			}
			if (token->isInstrumentAbbreviation()) {
				iabbrIndex = i;
			}
			if (token->isInstrumentNumber()) {
				inumIndex = i;
			}
		}
	}

	if ((iclassIndex > 0) && !m_iclass.empty()) {
		updateInstrumentLine(infile, iclassIndex, m_iclass, track2kindex, "*IC");
	}
	if ((icodeIndex > 0) && !m_icode.empty()) {
		updateInstrumentLine(infile, icodeIndex, m_icode, track2kindex, "*I");
	}
	if ((inumIndex > 0) && !m_inum.empty()) {
		updateInstrumentLine(infile, inumIndex, m_inum, track2kindex, "*I");
	}
	if ((inameIndex > 0) && !m_iname.empty()) {
		updateInstrumentLine(infile, inameIndex, m_iname, track2kindex, "*I\"");
	}
	if ((iabbrIndex > 0) && !m_iabbr.empty()) {
		updateInstrumentLine(infile, iabbrIndex, m_iabbr, track2kindex, "*I'");
	}

	// Insertion line of instrument info after given line;
	// Add above given index:
	int aclassIndex = -1;
	int acodeIndex  = -1;
	int anumIndex   = -1;
	int anameIndex  = -1;
	int aabbrIndex  = -1;
	// Or add below given index:
	int bclassIndex = -1;
	int bcodeIndex  = -1;
	int bnumIndex   = -1;
	int bnameIndex  = -1;
	int babbrIndex  = -1;

	// Where to place instrument class:
	if ((iclassIndex < 0) && !m_iclass.empty()) {
		if (icodeIndex > 0) {
			aclassIndex = icodeIndex;
		} else if (inumIndex > 0) {
			aclassIndex = inumIndex;
		} else if (inameIndex > 0) {
			aclassIndex = inameIndex;
		} else if (iabbrIndex > 0) {
			aclassIndex = iabbrIndex;
		} else if (gpsIndex > 0) {
			bclassIndex = gpsIndex;
		} else if (exinterpIndex >= 0) {
			bclassIndex = exinterpIndex;
		}
	}

	// Where to place instrument code:
	if ((icodeIndex < 0) && !m_icode.empty()) {
		if (iclassIndex > 0) {
			bcodeIndex = iclassIndex;
		} else if (inumIndex > 0) {
			acodeIndex = inumIndex;
		} else if (inameIndex > 0) {
			acodeIndex = inameIndex;
		} else if (iabbrIndex > 0) {
			acodeIndex = iabbrIndex;
		} else if (gpsIndex > 0) {
			bcodeIndex = gpsIndex;
		} else if (exinterpIndex >= 0) {
			bcodeIndex = exinterpIndex;
		}
	}

	// Where to place instrument number:
	if ((inumIndex < 0) && !m_inum.empty()) {
		if (icodeIndex > 0) {
			bnumIndex = icodeIndex;
		} else if (iclassIndex > 0) {
			bnumIndex = iclassIndex;
		} else if (inameIndex > 0) {
			anumIndex = inameIndex;
		} else if (iabbrIndex > 0) {
			anumIndex = iabbrIndex;
		} else if (gpsIndex > 0) {
			bnumIndex = gpsIndex;
		} else if (exinterpIndex >= 0) {
			bnumIndex = exinterpIndex;
		}
	}

	// Where to place instrument name:
	if ((inameIndex < 0) && !m_iname.empty()) {
		if (inumIndex > 0) {
			bnameIndex = inumIndex;
		} else if (icodeIndex > 0) {
			bnameIndex = icodeIndex;
		} else if (iclassIndex > 0) {
			bnameIndex = iclassIndex;
		} else if (iabbrIndex > 0) {
			anameIndex = iabbrIndex;
		} else if (gpsIndex > 0) {
			bnameIndex = gpsIndex;
		} else if (exinterpIndex >= 0) {
			bnameIndex = exinterpIndex;
		}
	}

	// Where to place instrument abbreviation:
	if ((iabbrIndex < 0) && !m_iabbr.empty()) {
		if (inameIndex > 0) {
			babbrIndex = inameIndex;
		} else if (inumIndex > 0) {
			babbrIndex = inumIndex;
		} else if (icodeIndex > 0) {
			babbrIndex = icodeIndex;
		} else if (iclassIndex > 0) {
			babbrIndex = iclassIndex;
		} else if (gpsIndex > 0) {
			babbrIndex = gpsIndex;
		} else if (exinterpIndex >= 0) {
			babbrIndex = exinterpIndex;
		}
	}

	if (aclassIndex > 0) {
		insertInstrumentInfo(infile, aclassIndex, m_iclass, "*IC", "above-class", track2kindex);
	} else if (bclassIndex >= 0) {
		insertInstrumentInfo(infile, bclassIndex, m_iclass, "*IC", "below-class", track2kindex);
	}

	if (acodeIndex > 0) {
		insertInstrumentInfo(infile, acodeIndex, m_icode, "*I", "above-code", track2kindex);
	} else if (bcodeIndex >= 0) {
		insertInstrumentInfo(infile, bcodeIndex, m_icode, "*I", "below-code", track2kindex);
	}

	if (anumIndex > 0) {
		insertInstrumentInfo(infile, anumIndex, m_inum, "*I", "above-num", track2kindex);
	} else if (bnumIndex >= 0) {
		insertInstrumentInfo(infile, bnumIndex, m_inum, "*I", "below-num", track2kindex);
	}

	if (anameIndex > 0) {
		insertInstrumentInfo(infile, anameIndex, m_iname, "*I\"", "above-name", track2kindex);
	} else if (bnameIndex >= 0) {
		insertInstrumentInfo(infile, bnameIndex, m_iname, "*I\"", "below-name", track2kindex);
	}

	if (aabbrIndex > 0) {
		insertInstrumentInfo(infile, aabbrIndex, m_iabbr, "*I'", "above-abbr", track2kindex);
	} else if (babbrIndex >= 0) {
		insertInstrumentInfo(infile, babbrIndex, m_iabbr, "*I'", "below-abbr", track2kindex);
	}

	infile.createLinesFromTokens();

	bool dataQ = false;
	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].hasSpines()) {
			m_humdrum_text << infile[i] << endl;
			continue;
		}
		if (infile[i].isData()) {
			dataQ = true;
		}
		if (infile[i].isBarline()) {
			dataQ = true;
		}
		if (dataQ) {
			m_humdrum_text << infile[i] << endl;
			continue;
		}
		printLine(infile, i);
	}
}



/////////////////////////////
//
// Tool_instinfo::insertInstrumentInfo --
//

void Tool_instinfo::insertInstrumentInfo(HumdrumFile& infile, int index,
	map<int, string>& info, const string& prefix, const string& key, map<int, int>& track2kindex) {

	for (int j=0; j<infile[index].getFieldCount(); j++) {
		HTp token = infile.token(index, j);
		if (!token->isKern()) {
			token->setValue("auto", key, "*");
			continue;
		}
		int track = token->getTrack();
		int kindex = track2kindex[track] - 1;
		if (kindex < 0) {
			token->setValue("auto", key, "*");
			continue;
		}
		if (((key == "above-class") || (key == "below-class")) && info[kindex].empty()) {
			token->setValue("auto", key, "*");
			continue;
		}
		if (((key == "above-code") || (key == "below-code")) && info[kindex].empty()) {
			token->setValue("auto", key, "*");
			continue;
		}
		if (((key == "above-num") || (key == "below-num")) && info[kindex].empty()) {
			token->setValue("auto", key, "*");
			continue;
		}
		string newtext = prefix + info[kindex];
		token->setValue("auto", key, newtext);
	}
}



//////////////////////////////
//
// Tool_instinfo::printLine --
//

void Tool_instinfo::printLine(HumdrumFile& infile, int index) {
	HTp first = infile.token(index, 0);

	if (!first->getValue("auto", "above-class").empty()) {
		printLine(infile, index, "above-class");
	}
	if (!first->getValue("auto", "above-code").empty()) {
		printLine(infile, index, "above-code");
	}
	if (!first->getValue("auto", "above-num").empty()) {
		printLine(infile, index, "above-num");
	}
	if (!first->getValue("auto", "above-name").empty()) {
		printLine(infile, index, "above-name");
	}
	if (!first->getValue("auto", "above-abbr").empty()) {
		printLine(infile, index, "above-abbr");
	}

	m_humdrum_text << infile[index] << endl;

	if (!first->getValue("auto", "below-class").empty()) {
		printLine(infile, index, "below-class");
	}
	if (!first->getValue("auto", "below-code").empty()) {
		printLine(infile, index, "below-code");
	}
	if (!first->getValue("auto", "below-num").empty()) {
		printLine(infile, index, "below-num");
	}
	if (!first->getValue("auto", "below-name").empty()) {
		printLine(infile, index, "below-name");
	}
	if (!first->getValue("auto", "below-abbr").empty()) {
		printLine(infile, index, "below-abbr");
	}

}



//////////////////////////////
//
// Tool_instinfo::printLine --
//

void Tool_instinfo::printLine(HumdrumFile& infile, int index, const string& key) {
	for (int j=0; j<infile[index].getFieldCount(); j++) {
		HTp token = infile.token(index, j);
		string value = token->getValue("auto", key);
		if (value.empty()) {
			value = "*";
		}
		m_humdrum_text << value;
		if (j < infile[index].getFieldCount() - 1) {
			m_humdrum_text << "\t";
		}
	}
	m_humdrum_text << endl;
}



//////////////////////////////
//
// Tool_instinfo::updateInstrumentLine --
//

void Tool_instinfo::updateInstrumentLine(HumdrumFile& infile, int index,
		map<int, string>& values, map<int, int>& track2kindex,
		const string& prefix) {
	for (int j = 0; j<infile[index].getFieldCount(); j++) {
		HTp token = infile.token(index, j);
		if (!token->isKern()) {
			continue;
		}
		int track = token->getTrack();
		int kindex = track2kindex[track] - 1;
		if (kindex < 0) {
			continue;
		}
		string value = values[kindex];
		if (value.empty()) {
			continue;
		}
		value = prefix + value;
		token->setText(value);
	}
}


// END_MERGE

} // end namespace hum



