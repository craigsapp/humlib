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
#include "Convert.h"
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
	define("m|mark|marker=s:@", "marker to add to score");
	define("p|pitch|pitches=s", "list of pitches to mark");
	define("i|interval=s", "musical interval of marked pitches");
	define("I|is-sounding=s", "musical score is in sounding format for marks");
	define("c|chromatic=i:0", "chromatic interval of marked pitches");
	define("d|diatonic=i:0", "diatonic interval of marked pitches");
	define("color=s", "color marked pitches");
	define("string=s", "string number");
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
	m_pitches.clear();
	m_marker = getString("mark");
	m_IQ = getBoolean("I");
	m_color = getString("color");
	if (getBoolean("pitches")) {
		m_pitches = parsePitches(getString("pitches"));
	}
	m_cd = getBoolean("diatonic") && getBoolean("chromatic");
	m_interval.clear();
	if (m_cd) {
		m_diatonic = getInteger("diatonic");
		m_chromatic = getInteger("chromatic");
	} else {
		if (getBoolean("interval")) {
			m_interval = getString("interval");
		}
	}
	if ((abs(m_diatonic) > 28) || (abs(m_chromatic) > 48)) {
		m_diatonic = 0;
		m_chromatic = 0;
		m_cd = false;
	}
	if (!m_pitches.empty()) {
		prepareTranspositionInterval();
	}
	m_string = getString("string");
}



//////////////////////////////
//
// Tool_scordatura::processFile --
//

void Tool_scordatura::processFile(HumdrumFile& infile) {
	m_modifiedQ = false;

	if (!m_pitches.empty()) {
		markPitches(infile);
		if (m_modifiedQ) {
			addMarkerRdf(infile);
		}
	}

	if (m_writtenQ || m_soundingQ) {
		vector<HTp> rdfs;
		getScordaturaRdfs(rdfs, infile);
		if (!rdfs.empty()) {
			processScordaturas(infile, rdfs);
		}
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



//////////////////////////////
//
// Tool_scordatura::parsePitches --
//

set<int> Tool_scordatura::parsePitches(const string& input) {
	HumRegex hre;
	string value = input;
	hre.replaceDestructive(value, "-", "\\s*-\\s*", "g");

	vector<string> pieces;
	hre.split(pieces, value, "[^A-Ga-g0-9-]+");

	HumPitch pitcher;
	set<int> output;
	string p1;
	string p2;
	int d1;
	int d2;
	for (int i=0; i<(int)pieces.size(); i++) {
		if (hre.search(pieces[i], "(.*)-(.*)")) {
			// pitch range
			p1 = hre.getMatch(1);
			p2 = hre.getMatch(2);
			d1 = Convert::kernToBase7(p1);
			d2 = Convert::kernToBase7(p2);
			if ((d1 < 0) || (d2 < 0) || (d1 > d2) || (d1 > 127) || (d2 > 127)) {
				continue;
			}
			for (int j=d1; j<=d2; j++) {
				output.insert(j);
			}
		} else {
			// single pitch
			d1 = Convert::kernToBase7(pieces[i]);
			if ((d1 < 0) || (d1 > 127)) {
				continue;
			}
			output.insert(d1);
		}
	}
	return output;
}



//////////////////////////////
//
// Tool_scordatura::markPitches --
//

void Tool_scordatura::markPitches(HumdrumFile& infile) {
	for (int i=0; i<infile.getStrandCount(); i++) {
		HTp sstart = infile.getStrandStart(i);
		if (!sstart->isKern()) {
			continue;
		}
		HTp sstop = infile.getStrandStop(i);
		markPitches(sstart, sstop);
	}
}


void Tool_scordatura::markPitches(HTp sstart, HTp sstop) {
	HTp current = sstart;
	while (current && (current != sstop)) {
		if (current->isNull() || current->isRest()) {
			current = current->getNextToken();
			continue;
		}
		markPitches(current);
		current = current->getNextToken();
	}
}


void Tool_scordatura::markPitches(HTp token) {
	vector<string> subtokens = token->getSubtokens();
	int counter = 0;
	for (int i=0; i<(int)subtokens.size(); i++) {
		int dia = Convert::kernToBase7(subtokens[i]);
		if (m_pitches.find(dia) != m_pitches.end()) {
			counter++;
			subtokens[i] += m_marker;
		}
	}
	if (counter == 0) {
		return;
	}
	string newtoken;
	for (int i=0; i<(int)subtokens.size(); i++) {
		newtoken += subtokens[i];
		if (i < (int)subtokens.size() - 1) {
			newtoken += ' ';
		}
	}
	token->setText(newtoken);
	m_modifiedQ = true;
}



//////////////////////////////
//
// Tool_scordatura::addMarkerRdf --
//

void Tool_scordatura::addMarkerRdf(HumdrumFile& infile) {
	string line = "!!!RDF**kern: ";
	line += m_marker;
	line += " = ";
	if (!m_string.empty()) {
		line += "string=";
		line += m_string;
		line += " ";
	}
	line += "scordatura=";
	if (m_IQ) {
		line += "I";
	}
	line += "Tr";
	if (m_transposition.empty()) {
		line += "XXX";
	} else {
		line += m_transposition;
	}
	if (!m_color.empty()) {
		line += ", color=";
		line += m_color;
	}
	infile.appendLine(line);
	m_modifiedQ = true;
}



//////////////////////////////
//
// Tool_scordatura::prepareTranspositionInterval --
//

void Tool_scordatura::prepareTranspositionInterval(void) {
	m_transposition.clear();
	if (m_cd) {
		m_transposition = "d";
		m_transposition += to_string(m_diatonic);
		m_transposition += "c";
		m_transposition += to_string(m_chromatic);
		return;
	}

	if (m_interval.empty()) {
		return;
	}

	HumTransposer trans;
	trans.intervalToDiatonicChromatic(m_diatonic, m_chromatic, m_interval);
	m_transposition = "d";
	m_transposition += to_string(m_diatonic);
	m_transposition += "c";
	m_transposition += to_string(m_chromatic);
}



// END_MERGE

} // end namespace hum



