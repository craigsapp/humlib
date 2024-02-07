//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Fri May  6 19:30:42 PDT 2022
// Last Modified: Fri May  6 19:30:45 PDT 2022
// Filename:      tool-humtr.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/tool-humtr.cpp
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Modernize POPC2 project text.
//

#include "tool-humtr.h"
#include "HumRegex.h"

using namespace std;

namespace hum {

// START_MERGE


/////////////////////////////////
//
// Tool_humtr::Tool_humtr -- Set the recognized options for the tool.
//

Tool_humtr::Tool_humtr(void) {
	define("T|no-text|no-lyrics=b", "Do not convert lyrics in **text spines.");
	define("L|no-local=b", "Do not convert local LO t parameters.");
	define("G|no-global=b", "Do not convert global LO t parameters.");
	define("R|no-reference=b", "Do not convert reference record values.");

	define("t|text-only|lyrics-only=b", "convert only lyrics in **text spines.");
	define("l|local-only=b", "convert only local LO t parameters.");
	define("g|global-only=b", "convert only global LO t parameters.");
	define("r|reference-only=b", "convert only reference record values.");

	define("d|data-type=s", "process only given exclusive interpretations");
	define("s|spines=s", "spines to process");

	define("i|input=s", "Input characters to change");
	define("o|output=s", "Output characters to change to");

	define("m|replace-map=s", "Characters to change from and to");
	define("M|display-mapping=b", "Display character transliterations mappings");
	define("p|popc|popc2=b", "Add POPC2 character substitutions");
}



//////////////////////////////
//
// Tool_humtr::initialize --  Initializations that only have to be done once
//    for all HumdrumFile segments.
//

void Tool_humtr::initialize(void) {
	m_lyricsQ    = !getBoolean("T");
	m_localQ     = !getBoolean("L");
	m_globalQ    = !getBoolean("G");
	m_referenceQ = !getBoolean("R");

	m_lyricsOnlyQ    = getBoolean("t");
	m_localOnlyQ     = getBoolean("l");
	m_globalOnlyQ    = getBoolean("g");
	m_referenceOnlyQ = getBoolean("r");

	if (m_lyricsOnlyQ || m_localOnlyQ || m_globalOnlyQ || m_referenceOnlyQ) {
		m_lyricsQ = false;
		m_localQ = false;
		m_globalQ = false;
		m_referenceQ = false;
	}
	if (m_lyricsOnlyQ) {
		m_lyricsQ = true;
	}
	if (m_localOnlyQ) {
		m_localQ = true;
	}
	if (m_globalOnlyQ) {
		m_globalQ = true;
	}
	if (m_referenceOnlyQ) {
		m_referenceQ = true;
	}

	m_from.clear();
	m_to.clear();

	if (!getBoolean("replace-map")) {
		string replace = getString("replace-map");
		addFromToCombined(replace);
	}

	if (getBoolean("input") && getBoolean("output")) {
		string fromString = getString("input");
		string toString = getString("output");
		fillFromToPair(fromString, toString);
	}

	if (getBoolean("popc")) {
		addFromToCombined("ſ:s ʃ:s &#383;:s ν:u ί:í α:a ť:k ᴣ:z ʓ:z̨ ʒ̇:ż ʒ́:ź Ʒ̇:Ż Ʒ́:Ź ӡ:z Ʒ:Z Ӡ:Z æ:ae");
	}
}



/////////////////////////////////
//
// Tool_humtr::run -- Do the main work of the tool.
//

bool Tool_humtr::run(HumdrumFileSet& infiles) {
	bool status = true;
	for (int i=0; i<infiles.getCount(); i++) {
		status &= run(infiles[i]);
	}
	return status;
}



bool Tool_humtr::run(const string& indata, ostream& out) {
	HumdrumFile infile(indata);
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_humtr::run(HumdrumFile& infile, ostream& out) {
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_humtr::run(HumdrumFile& infile) {
	initialize();
	processFile(infile);
	if (getBoolean("display-mapping")) {
		int lengthF = (int)m_from.size();
		int lengthT = (int)m_to.size();
		int length = lengthF;
		if (length > lengthT) {
			length = lengthT;
		}
		for (int i=0; i<length; i++) {
			m_free_text << "FROM\t" << m_from[i] << "\tTO\t" << m_to[i] << endl;
		}
		return true;
	} else {
		infile.createLinesFromTokens();
		m_humdrum_text << infile;
	}
	return true;
}



//////////////////////////////
//
// Tool_humtr::processFile --
//

void Tool_humtr::processFile(HumdrumFile& infile) {
	if (m_lyricsQ) {
		convertTextSpines(infile);
	}
	if (m_localQ) {
		convertLocalLayoutText(infile);
	}
	if (m_globalQ) {
		convertGlobalLayoutText(infile);
	}
	if (m_referenceQ) {
		convertReferenceText(infile);
	}
}



//////////////////////////////
//
// Tool_humtr::convertTextSpines --
//

void Tool_humtr::convertTextSpines(HumdrumFile& infile) {
	int scount = infile.getStrandCount();
	for (int i=0; i<scount; i++) {
		HTp stok = infile.getStrandStart(i);
		if (!stok->isDataType("**text")) {
			continue;
		}
		HTp etok = infile.getStrandEnd(i);
		processTextStrand(stok, etok);
	}
}



//////////////////////////////
//
// Tool_humtr::processTextStrand --
//

void Tool_humtr::processTextStrand(HTp stok, HTp etok) {
	HTp current = stok;
	while (current && (current != etok)) {
		if (!current->isData()) {
			current = current->getNextToken();
			continue;
		}
		if (current->isNull()) {
			current = current->getNextToken();
			continue;
		}

		string text = transliterateText(*current);
		if (text != *current) {
			current->setText(text);
		}

		current = current->getNextToken();
	}
}



//////////////////////////////
//
// Tool_humtr::convertReferenceText --
//

void Tool_humtr::convertReferenceText(HumdrumFile& infile) {
	HumRegex hre;
	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isGlobalReference()) {
			continue;
		}

		HTp token = infile.token(i, 0);
		if (!hre.search(token, "^!!![^:]+:(.*)$")) {
			continue;
		}
		string oldcontents = hre.getMatch(1);
		if (oldcontents == "") {
			return;
		}
		string newcontents = transliterateText(oldcontents);
		if (oldcontents != newcontents) {
			string text = *token;
			hre.replaceDestructive(text, ":" + newcontents, ":" + oldcontents);
			token->setText(text);
		}
	}
}



//////////////////////////////
//
// Tool_humtr::convertGlobalLayoutText --
//

void Tool_humtr::convertGlobalLayoutText(HumdrumFile& infile) {
	HumRegex hre;
	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isCommentGlobal()) {
			continue;
		}

		HTp token = infile.token(i, 0);
		if (!hre.search(token, "^!!LO:.*:t=([^:]+)")) {
			continue;
		}
		string oldcontents = hre.getMatch(1);
		string newcontents = transliterateText(oldcontents);
		if (oldcontents != newcontents) {
			string text = *token;
			hre.replaceDestructive(text, ":t=" + newcontents, ":t=" + oldcontents);
			token->setText(text);
		}
	}
}



//////////////////////////////
//
// Tool_humtr::convertLocalLayoutText --
//

void Tool_humtr::convertLocalLayoutText(HumdrumFile& infile) {
	HumRegex hre;

	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isCommentLocal()) {
			continue;
		}
		for (int j=0; j<infile[i].getFieldCount(); j++) {
			HTp token = infile.token(i, j);
			if (*token == "!") {
				continue;
			}
			if (!hre.search(token, "^!LO:.*:t=([^:]+)")) {
				continue;
			}
			string oldcontents = hre.getMatch(1);
			string newcontents = transliterateText(oldcontents);
			if (oldcontents != newcontents) {
				string text = *token;
				hre.makeSafeDestructive(oldcontents);
				hre.replaceDestructive(text, ":t=" + newcontents, ":t=" + oldcontents);
				token->setText(text);
			}
		}
	}
}



//////////////////////////////
//
// Tool_humtr::addFromToCombined -- Add additional translations to
//     the from / to substitutions array.
//

void Tool_humtr::addFromToCombined(const string& value) {
	HumRegex hre;
	vector<string> entries;
	hre.split(entries, value, m_sep1);
	for (int i=0; i<(int)entries.size(); i++) {
		vector<string> mapping;
		hre.split(mapping, entries[i], m_sep2);
		if (mapping.size() != 2) {
			cerr << "Warning: ignoring bad character mapping: " << entries[i] << endl;
			continue;
		}
		m_from.push_back(mapping[0]);
		m_to.push_back(mapping[1]);
	}
}



//////////////////////////////
//
// Tool_humtr::fillFromToPair
//

void Tool_humtr::fillFromToPair(const string& from, const string& to) {
	vector<string> fromList = getUtf8CharacterArray(from);
	vector<string> toList   = getUtf8CharacterArray(to);
	if (fromList.size() != toList.size()) {
		cerr << "Error: String lengths to not match for " << from << "\tAND\t" << to << endl;
		cerr << "FROM LIST count: " << fromList.size() << endl;
		for (int i=0; i<(int)fromList.size(); i++) {
			cerr << "\t" << fromList[i] << endl;
		}
		cerr << endl;
		cerr << "TO LIST count: " << toList.size() << endl;
		for (int i=0; i<(int)toList.size(); i++) {
			cerr << "\t" << toList[i] << endl;
		}
		return;
	}
	for (int i=0; i<(int)fromList.size(); i++) {
		m_from.push_back(fromList[i]);
	}
	for (int i=0; i<(int)toList.size(); i++) {
		m_to.push_back(toList[i]);
	}
}



//////////////////////////////
//
// Tool_humtr::getUtf8CharacterArray --
//

vector<string> Tool_humtr::getUtf8CharacterArray(const string& value) {
	vector<string> output;
	string current;
	for (int i=0; i<(int)value.size(); i++) {
		current = "";
		char v = value[i];
		current.push_back(v);
		unsigned char u = (unsigned char)v;
		if (u < 0x80) {
			output.push_back(current);
			continue;
		}
		int count = 0;
		if (u >> 5 == 6) {
			count = 1;
		} else if (u >> 4 == 14) {
			count = 2;
		} else if (u >> 3 == 30) {
			count = 3;
		} else {
			cerr << "Error reading UTF-8 character in string " << value << endl;
			output.clear();
			return output;
		}
		for (int j=0; j<count; j++) {
			v = value[i+j];
			u = (unsigned char)v;
			if (v >> 6 != 2) {
				cerr << "Error in reading UTF-8 character of string " << endl;
				output.clear();
				return output;
			}
			current.push_back(v);
		}
		output.push_back(current);
	}

	if (output.empty()) {
		return output;
	}

	// Check for ASCII character ranges:
	vector<string> out2;
	out2.push_back(output[0]);
	for (int i=1; i<(int)output.size() - 1; i++) {
		if (output[i] != "-") {
			out2.push_back(output[i]);
			continue;
		}
		if ((output[i-1].size() > 1) || (output[i+1].size() > 1)) {
			// One or both of the adjacent characters are UTF-8, so
			// treat dash as regular character rather than range operator.
			out2.push_back(output[i]);
			continue;
		}

		// Insert a range of characters:
		unsigned int starting = (unsigned char)output[i-1][0];
		unsigned int ending   = (unsigned char)output[i+1][0];
		if (starting > 0xff) {
			cerr << "Strange error here " << starting << endl;
		}
		if (ending > 0xff) {
			cerr << "Strange error here " << starting << endl;
		}
		if (starting == ending) {
			continue;
		}
		int direction;
		if (starting > ending) {
			direction = -1;
			starting--;
			ending++;
		} else {
			direction = 1;
			starting++;
			ending--;
		}
		if (direction > 0) {
			for (unsigned int j=starting; j<=ending; j++) {
				string current = "";
				current.push_back((unsigned char)j);
				out2.push_back(current);
			}
		} else {
			for (unsigned int j=starting; j>=ending; j--) {
				string current = "";
				current.push_back((unsigned char)j);
				out2.push_back(current);
			}
		}
	}
	if (output.size() > 1) {
		out2.push_back(output.back());
	}

	return out2;
}



//////////////////////////////
//
// Tool_humtr::transliterateText --
//

string Tool_humtr::transliterateText(const string& input) {
	return transliterateTextNonOverlapping(input);
}



//////////////////////////////
//
// Tool_humtr::transliterateTextNonOverlapping --
//

string Tool_humtr::transliterateTextNonOverlapping(const string& input) {
	string output = input;
	HumRegex hre;
	for (int i=0; i<(int)m_from.size(); i++) {
		hre.replaceDestructive(output, m_to.at(i), m_from.at(i), "g");
	}
	return output;
}



//////////////////////////////
//
// Tool_humtr::transliterateTextOverlapping -- Only single-character mappings
//     are allowed (used particularly for character ranges).
//

string Tool_humtr::transliterateTextOverlapping(const string& input) {
	// not implemented yet
	return input;
}



// END_MERGE

} // end namespace hum



