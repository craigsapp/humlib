//
// Programmer:    Martha Thomae
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Wed Dec 16 11:16:33 PST 2020
// Last Modified: Sun Sep 24 17:17:23 PDT 2023
// Filename:      tool-kern2mens.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/tool-kern2mens.cpp
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Converts **kerns data into **mens.
//

#include "tool-kern2mens.h"
#include "Convert.h"
#include "HumRegex.h"

using namespace std;

namespace hum {

// START_MERGE


/////////////////////////////////
//
// Tool_kern2mens::Tool_kern2mens -- Set the recognized options for the tool.
//

Tool_kern2mens::Tool_kern2mens(void) {
	define("N|no-measure-numbers=b",                "remove measure numbers");
	define("M|no-measures=b",                       "remove measures ");
	define("I|not-invisible=b",                     "keep measures visible");
	define("D|no-double-bar=b",                     "keep thick final barlines");
	define("c|clef=s",                              "clef to use in mensural notation");
	define("V|no-verovio=b",                        "don't add verovio styling");
	define("e|evenNoteSpacing|even-note-spacing=b", "add evenNoteSpacing option");
}



/////////////////////////////////
//
// Tool_kern2mens::run -- Do the main work of the tool.
//

bool Tool_kern2mens::run(HumdrumFileSet& infiles) {
	bool status = true;
	for (int i=0; i<infiles.getCount(); i++) {
		status &= run(infiles[i]);
	}
	return status;
}

bool Tool_kern2mens::run(const string& indata, ostream& out) {
	HumdrumFile infile(indata);
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_kern2mens::run(HumdrumFile& infile, ostream& out) {
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_kern2mens::run(HumdrumFile& infile) {
	m_numbersQ   = !getBoolean("no-measure-numbers");
	m_measuresQ  = !getBoolean("no-measures");
	m_invisibleQ = !getBoolean("not-invisible");
	m_doublebarQ = !getBoolean("no-double-bar");
	m_noverovioQ =  getBoolean("no-verovio");
	m_clef       =  getString("clef");
	m_evenNoteSpacingQ = getBoolean("even-note-spacing");
	storeKernEditorialAccidental(infile);
	storeKernTerminalLong(infile);
	convertToMens(infile);
	return true;
}



//////////////////////////////
//
// Tool_kern2mens::convertToMens --
//

void Tool_kern2mens::convertToMens(HumdrumFile& infile) {
	analyzeColoration(infile);
	int maxtrack = infile.getMaxTrack();
	for (int i=0; i<infile.getLineCount(); i++) {
		if (infile[i].isBarline()) {
			printBarline(infile, i);
			continue;
		}
		if (!infile[i].hasSpines()) {
			if (i == m_kernEdAccLineIndex) {
				m_humdrum_text << m_mensEdAccLine << endl;
			} else if (i == m_kernTerminalLongIndex) {
				m_humdrum_text << m_mensTerminalLongLine << endl;
			} else {
				m_humdrum_text << infile[i] << "\n";
			}
			continue;
		}
		if ((maxtrack == 1) && infile[i].isAllNull()) {
			continue;
		}
		for (int j=0; j<infile[i].getFieldCount(); j++) {
			HTp token = infile.token(i, j);
			m_humdrum_text << convertKernTokenToMens(token);
			if (j < infile[i].getFieldCount() - 1) {
				m_humdrum_text << "\t";
			}
		}
		m_humdrum_text << "\n";
	}
	if (!m_noverovioQ) {
		addVerovioStyling(infile);
	}
}



//////////////////////////////
//
// Tool_kern2mens::addVerovioStyling --  Add a spacing of 0.3, 0.5 if no
//   spacing is already found in the data.  Use the -V option to no add any
//   spacing options (to use the defaults in verovio).
//

void Tool_kern2mens::addVerovioStyling(HumdrumFile& infile) {
	HumRegex hre;
	for (int i=0; i<infile.getLineCount(); i++) {
		if (infile[i].hasSpines()) {
			continue;
		}
		HTp token = infile[i].token(0);
		if (hre.search(token, "!!!verovio:\\s*evenNoteSpacing")) {
			return;
		}
		if (!m_evenNoteSpacingQ) {
			if (hre.search(token, "!!!verovio:\\s*spacingLinear")) {
				return;
			}
			if (hre.search(token, "!!!verovio:\\s*spacingNonLinear")) {
				return;
			}
		}
	}
	if (m_evenNoteSpacingQ) {
		m_humdrum_text << "!!!verovio: evenNoteSpacing\n";
	} else {
		m_humdrum_text << "!!!verovio: spacingLinear 0.3\n";
		m_humdrum_text << "!!!verovio: spacingNonLinear 0.5\n";
	}
}



//////////////////////////////
//
// Tool_kern2mens::convertKernTokenToMens --
//

string Tool_kern2mens::convertKernTokenToMens(HTp token) {
	string data;
	HumRegex hre;
	if (!token->isKern()) {
		return *token;
	}
	if (token->isNull()) {
		return *token;
	}
	if (token->isExclusiveInterpretation()) {
		return "**mens";
	}
	if (token->isInterpretation()) {
		if (!m_clef.empty()) {
			if (hre.search(token, "^\\*clef")) {
				data = "*clef";
				data += m_clef;
				return data;
			}
		} else if (hre.search(token, "^\\*[mo]?clef")) {
			string value = getClefConversion(token);
			return value;
		}
	}
	if (!token->isData()) {
		return *token;
	}
	if (token->isSecondaryTiedNote()) {
		return ".";
	}
	data = *token;
	// remove uninteresting characters (beams, articulations, etc).
	// keeping pitches, accidentals, rests, slurs, durations, ties
	hre.replaceDestructive(data, "", "[^A-Gnra-g#\\(\\)\\[\\]0-9%.-]", "g");
	// but keep editorial accidental (probably i)
	HumNum dur;
	if (token->find("[") != std::string::npos) {
		dur = token->getTiedDuration();
		hre.replaceDestructive(data, "", "\\[");
	} else {
		dur = token->getDuration();
	}
	string rhythm = Convert::durationToRecip(dur);
	bool perfect = false;
	if (rhythm.find('.') != std::string::npos) {
		perfect = true;
	}
	hre.replaceDestructive(data, rhythm, "\\d+%?\\d*\\.*");
	hre.replaceDestructive(data, "S", "3\\%4");
	hre.replaceDestructive(data, "s", "3\\%2");
	hre.replaceDestructive(data, "M", "3");
	hre.replaceDestructive(data, "X", "000");
	hre.replaceDestructive(data, "L", "00");
	hre.replaceDestructive(data, "S", "0");
	hre.replaceDestructive(data, "u", "16");
	hre.replaceDestructive(data, "M", "2");
	hre.replaceDestructive(data, "m", "4");
	hre.replaceDestructive(data, "U", "8");
	hre.replaceDestructive(data, "s", "1");
	hre.replaceDestructive(data, ":", "\\.");
	if (perfect) {
		hre.replaceDestructive(data, "$1p", "([XLSsMmUu]+)");
	} else {
		hre.replaceDestructive(data, "$1i", "([XLSsMmUu]+)");
	}

	// transfer editorial accidental
	if (!m_kernEditorialAccidental.empty()) {
		if (token->find(m_kernEditorialAccidental) != string::npos) {
			hre.replaceDestructive(data, "$1z", "([#-n]+)", "g");
		}
	}

	// transfer terminal longs
	if (!m_kernTerminalLong.empty()) {
		string searchTerm = "(" + m_kernTerminalLong + "+)";
		if (hre.search(token, searchTerm)) {
			data += hre.getMatch(1);
		}
	}

	bool coloration = token->getValueBool("auto", "coloration");
	if (coloration) {
		data += "~";
	}

	return data;
}



//////////////////////////////
//
// Tool_kern2mens::printBarline --
//

void Tool_kern2mens::printBarline(HumdrumFile& infile, int line) {
	bool doubleQ = false;
	// keeping double barlines and final barlines.
	if (infile.token(line, 0)->find("==") != std::string::npos) {
		doubleQ = true;
	} else if (infile.token(line, 0)->find("||") != std::string::npos) {
		doubleQ = true;
	} else if (!m_measuresQ) {
		return;
	}

	HumRegex hre;
	int dataline = line+1;
	while (dataline < infile.getLineCount()) {
		if (infile[dataline].isData()) {
			break;
		}
		dataline++;
	}
	if (dataline >= infile.getLineCount()) {
		dataline = infile.getLineCount() - 1;
	}
	if (infile[dataline].isData()) {
		int attacks = true;
		for (int j=0; j<infile[dataline].getFieldCount(); j++) {
			HTp token = infile.token(dataline, j);
			if (!token->isKern()) {
				continue;
			}
			if (token->isSecondaryTiedNote()) {
				attacks = false;
				break;
			}
		}
		if ((!doubleQ) && (!attacks)) {
			return;
		}
	}

	for (int j=0; j<infile[line].getFieldCount(); j++) {
		doubleQ = false;
		string token = *infile.token(line, j);
		if (m_doublebarQ && (token.find("==") != std::string::npos)) {
			hre.replaceDestructive(token, "=||", "=+");
			doubleQ = true;
		}
		if (m_doublebarQ && (token.find("||") != std::string::npos)) {
			doubleQ = true;
		}
		if (!m_numbersQ) {
			hre.replaceDestructive(token, "", "\\d+");
		}
		if (token.find("-") != std::string::npos) {
			m_humdrum_text << token;
		} else {
			if ((!doubleQ) && m_invisibleQ) {
				m_humdrum_text << token << "-";
			} else {
				m_humdrum_text << token;
			}
		}
		if (j < infile[line].getFieldCount() - 1) {
			m_humdrum_text << "\t";
		}
	}
	m_humdrum_text << "\n";
}



//////////////////////////////
//
// Tool_kern2mens::getClefConversion --
//    If token is *oclef and there is an adjacent *clef,
//        convert *oclef to *clef and *clef to *mclef; otherwise,
//        return the given clef.
//
//

string Tool_kern2mens::getClefConversion(HTp token) {

	vector<HTp> clefs;
	vector<HTp> oclefs;
	vector<HTp> mclefs;

	HumRegex hre;

	HTp current = token->getNextToken();
	while (current) {
		if (current->isData()) {
			break;
		}
		if (current->compare(0, 5, "*clef") == 0) {
			clefs.push_back(current);
		}
		if (current->compare(0, 6, "*oclef") == 0) {
			oclefs.push_back(current);
		}
		if (current->compare(0, 6, "*mclef") == 0) {
			mclefs.push_back(current);
		}
		current = current->getNextToken();
	}

	current = token->getPreviousToken();
	while (current) {
		if (current->isData()) {
			break;
		}
		if (current->compare(0, 5, "*clef") == 0) {
			clefs.push_back(current);
		}
		if (current->compare(0, 6, "*oclef") == 0) {
			oclefs.push_back(current);
		}
		if (current->compare(0, 6, "*mclef") == 0) {
			mclefs.push_back(current);
		}
		current = current->getPreviousToken();
	}

	if (token->compare(0, 5, "*clef") == 0) {
		if (oclefs.size() > 0) {
			string value = *token;
			hre.replaceDestructive(value, "mclef", "clef");
			return value;
		}
	}

	if (token->compare(0, 6, "*oclef") == 0) {
		if (clefs.size() > 0) {
			string value = *token;
			hre.replaceDestructive(value, "clef", "oclef");
			return value;
		}
	}

	return *token;
}



//////////////////////////////
//
// Tool_kern2mens::storeKernEditorialAccidental --
//

void Tool_kern2mens::storeKernEditorialAccidental(HumdrumFile& infile) {
	for (int i=infile.getLineCount() - 1; i>= 0; i--) {
		if (infile[i].hasSpines()) {
			continue;
		}
		if (!infile[i].isReferenceRecord()) {
			continue;
		}
		string key = infile[i].getReferenceKey();
		if (key != "RDF**kern") {
			continue;
		}
		HumRegex hre;
		string value = infile[i].getReferenceValue();
		if (hre.search(value, "^\\s*([^\\s]+)\\s*=\\s*(.*)\\s*$")) {
			string signifier = hre.getMatch(1);
			string definition = hre.getMatch(2);
			if (hre.search(definition, "editorial\\s+accidental")) {
				m_kernEditorialAccidental = signifier;
				m_kernEdAccLineIndex = i;
				m_mensEdAccLine = "!!!RDF**mens: z = ";
				m_mensEdAccLine += definition;
				break;
			}
		}
	}
}



//////////////////////////////
//
// Tool_kern2mens::storeKernTerminalLong --
//

void Tool_kern2mens::storeKernTerminalLong(HumdrumFile& infile) {
	for (int i=infile.getLineCount() - 1; i>= 0; i--) {
		if (infile[i].hasSpines()) {
			continue;
		}
		if (!infile[i].isReferenceRecord()) {
			continue;
		}
		string key = infile[i].getReferenceKey();
		if (key != "RDF**kern") {
			continue;
		}
		HumRegex hre;
		string value = infile[i].getReferenceValue();
		if (hre.search(value, "^\\s*([^\\s]+)\\s*=\\s*(.*)\\s*$")) {
			string signifier = hre.getMatch(1);
			string definition = hre.getMatch(2);

			if (hre.search(definition, "terminal\\s+long")) {
				m_kernTerminalLong = signifier;
				m_kernTerminalLongIndex = i;
				m_mensTerminalLongLine = "!!!RDF**mens: " + signifier + " = ";
				m_mensTerminalLongLine += definition;
				break;
			} else if (hre.search(definition, "long\\s+note")) {
				m_kernTerminalLong = signifier;
				m_kernTerminalLongIndex = i;
				m_mensTerminalLongLine = "!!!RDF**mens: " + signifier + " = ";
				m_mensTerminalLongLine += definition;
				break;
			}

		}
	}
}



//////////////////////////////
//
// Tool_kern2mens::analyzeColoration --
//

void Tool_kern2mens::analyzeColoration(HumdrumFile& infile) {
	vector<HTp> spinestarts = infile.getKernSpineStartList();
	for (int i=0; i<(int)spinestarts.size(); i++) {
		analyzeColoration(spinestarts[i]);
	}
}

void Tool_kern2mens::analyzeColoration(HTp stok) {
	HTp current = stok->getNextToken();
	bool coloration = false;
	while (current) {
		if (current->isInterpretation()) {
			if (*current == "*col") {
				coloration = true;
			} else if (*current == "*Xcol") {
				coloration = false;
			}
		}
		if (!current->isData()) {
			current = current->getNextToken();
			continue;
		}
		if (current->isNull()) {
			current = current->getNextToken();
			continue;
		}
		if (coloration) {
			current->setValue("auto", "coloration", 1);
		}
		current = current->getNextToken();
	}
}



// END_MERGE

} // end namespace hum



