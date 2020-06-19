//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Thu 18 Jun 2020 10:23:13 PM PDT
// Last Modified: Thu 18 Jun 2020 10:23:16 PM PDT
// Filename:      tool-scordatura.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/tool-scordatura.cpp
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Convert between written and sounding scores for scordatura.
//

#include "tool-scordatura.h"
#include "HumRegex.h"

using namespace std;

namespace hum {

// START_MERGE


/////////////////////////////////
//
// Tool_scordatura::Tool_scordatura -- Set the recognized options for the tool.
//

Tool_scordatura::Tool_scordatura(void) {
	define("s|sounding=b", "generate sounding score");
	define("w|written=b", "generate written score");
}



/////////////////////////////////
//
// Tool_scordatura::run -- Do the main work of the tool.
//

bool Tool_scordatura::run(HumdrumFileSet& infiles) {
	bool status = true;
	for (int i=0; i<infiles.getCount(); i++) {
		status &= run(infiles[i]);
	}
	return status;
}


bool Tool_scordatura::run(const string& indata, ostream& out) {
	HumdrumFile infile(indata);
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_scordatura::run(HumdrumFile& infile, ostream& out) {
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_scordatura::run(HumdrumFile& infile) {
	initialize();
	processFile(infile);
	return true;
}



//////////////////////////////
//
// Tool_scordatura::initialize --  Initializations that only have to be done once
//    for all HumdrumFile segments.
//

void Tool_scordatura::initialize(void) {
	m_writtenQ  = getBoolean("written");
	m_soundingQ = getBoolean("sounding");
}



//////////////////////////////
//
// Tool_scordatura::processFile --
//

void Tool_scordatura::processFile(HumdrumFile& infile) {
	m_modifiedQ = false;

	vector<HTp> rdfs;
	getScordaturaRdfs(rdfs, infile);
	if (!rdfs.empty()) {
		processScordaturas(infile, rdfs);
	}
	if (m_modifiedQ) {
		infile.createLinesFromTokens();
	}
}



//////////////////////////////
//
// Tool_scordatura::processScoredaturas --
//

void Tool_scordatura::processScordaturas(HumdrumFile& infile, vector<HTp>& rdfs) {
	for (int i=0; i<(int)rdfs.size(); i++) {
		processScordatura(infile, rdfs[i]);
	}
}



//////////////////////////////
//
// Tool_scordatura::processScordatura --
//

void Tool_scordatura::processScordatura(HumdrumFile& infile, HTp reference) {
	HumRegex hre;

	if (m_writtenQ) {
		if (!hre.search(reference, "^!!!RDF\\*\\*kern\\s*:\\s*([^\\s]+)\\s*=.*\\bscordatura\\s*=\\s*[\"']?\\s*ITrd(-?\\d+)c(-?\\d+)\\b")) {
			return;
		}
	} else if (m_soundingQ) {
		if (!hre.search(reference, "^!!!RDF\\*\\*kern\\s*:\\s*([^\\s]+)\\s*=.*\\bscordatura\\s*=\\s*[\"']?\\s*Trd(-?\\d+)c(-?\\d+)\\b")) {
			return;
		}
	}

	string marker = hre.getMatch(1);
	int diatonic = hre.getMatchInt(2);
	int chromatic = hre.getMatchInt(3);

	if (diatonic == 0 && chromatic == 0) {
		// nothing to do
		return;
	}

	flipScordaturaInfo(reference, diatonic, chromatic);
	transposeMarker(infile, marker, diatonic, chromatic);
}



//////////////////////////////
//
// Tool_scordatura::transposeMarker --
//


void Tool_scordatura::transposeMarker(HumdrumFile& infile, const string& marker, int diatonic, int chromatic) {
	m_transposer.setTranspositionDC(diatonic, chromatic);
	for (int i=0; i<infile.getStrandCount(); i++) {
		HTp sstart = infile.getStrandBegin(i);
		if (!sstart->isKern()) {
			continue;
		}
		HTp sstop = infile.getStrandEnd(i);
		transposeStrand(sstart, sstop, marker);
	}
}



//////////////////////////////
//
// Tool_scordatura::transposeStrand --
//

void Tool_scordatura::transposeStrand(HTp sstart, HTp sstop, const string& marker) {
	HTp current = sstart;
	while (current && current != sstop) {
		if (!current->isData()) {
			current = current->getNextToken();
			continue;
		}
		if (current->isNull() || current->isRest()) {
			current = current->getNextToken();
			continue;
		}
		if (current->find(marker) != string::npos) {
			transposeChord(current, marker);
		}
		current = current->getNextToken();
	}
}



//////////////////////////////
//
// Tool_scordatura::transposeChord --
//

void Tool_scordatura::transposeChord(HTp token, const string& marker) {
	int scount = token->getSubtokenCount();
	if (scount == 1) {
		string inputnote = *token;
		string newtoken;
		newtoken = transposeNote(inputnote);
		token->setText(newtoken);
		return;
	}
	vector<string> subtokens;
	subtokens = token->getSubtokens();
	for (int i=0; i<(int)subtokens.size(); i++) {
		if (subtokens[i].find(marker) == string::npos) {
			continue;
		}
		string newtoken = transposeNote(subtokens[i]);
		subtokens[i] = newtoken;
	}
	string newchord;
	for (int i=0; i<(int)subtokens.size(); i++) {
		newchord += subtokens[i];
		if (i<(int)subtokens.size() - 1) {
			newchord += ' ';
		}
	}
	token->setText(newchord);
}



//////////////////////////////
//
// Tool_scordatura::transposeNote --
//

string Tool_scordatura::transposeNote(const string& note) {
	HumRegex hre;
	if (!hre.search(note, "(.*?)([A-Ga-g]+[-#]*)(.*)")) {
		return note;
	}
	string pre = hre.getMatch(1);
	string pitch = hre.getMatch(2);
	string post = hre.getMatch(3);
	HumPitch hpitch;
	hpitch.setKernPitch(pitch);
	m_transposer.transpose(hpitch);
	string output;
	output = pre;
	output += hpitch.getKernPitch();
	output += post;
	return output;
}



//////////////////////////////
//
// Tool_scordatura::flipScordaturaInfo --
//

void Tool_scordatura::flipScordaturaInfo(HTp reference, int diatonic, int chromatic) {
	diatonic *= -1;
	chromatic *= -1;
	string output;
	if (m_writtenQ) {
		output = "Trd";
		output += to_string(diatonic);
		output += "c";
		output += to_string(chromatic);
	} else if (m_soundingQ) {
		output = "ITrd";
		output += to_string(diatonic);
		output += "c";
		output += to_string(chromatic);
	} else {
		return;
	}
	HumRegex hre;
	string token = *reference;
	hre.replaceDestructive(token, output, "I?Trd-?\\dc-?\\d");
	if (token != *reference) {
		m_modifiedQ = true;
		reference->setText(token);
	}
}



//////////////////////////////
//
// Tool_scordatura::getScoredaturaRdfs --
//

void Tool_scordatura::getScordaturaRdfs(vector<HTp>& rdfs, HumdrumFile& infile) {
	rdfs.clear();
	HumRegex hre;
	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isReference()) {
			continue;
		}
		HTp reference = infile.token(i, 0);
		if (m_writtenQ) {
			if (hre.search(reference, "^!!!RDF\\*\\*kern\\s*:\\s*[^\\s]+\\s*=.*\\bscordatura\\s*=\\s*[\"']?\\s*ITrd-?\\d+c-?\\d+\\b")) {
				rdfs.push_back(reference);
			}
		} else if (m_soundingQ) {
			if (hre.search(reference, "^!!!RDF\\*\\*kern\\s*:\\s*[^\\s]+\\s*=.*\\bscordatura\\s*=\\s*[\"']?\\s*Trd-?\\d+c-?\\d+\\b")) {
				rdfs.push_back(reference);
			}
		}
	}
}


// END_MERGE

} // end namespace hum



