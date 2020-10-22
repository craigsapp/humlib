//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Wed Oct 21 21:18:45 PDT 2020
// Last Modified: Wed Oct 21 21:18:49 PDT 2020
// Filename:      tool-strophe.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/tool-strophe.cpp
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Switch between strophe encoding and corrected encoding.
//

#include "tool-strophe.h"
#include "Convert.h"
#include "HumRegex.h"

using namespace std;

namespace hum {

// START_MERGE


/////////////////////////////////
//
// Tool_strophe::Tool_strophe -- Set the recognized options for the tool.
//

Tool_strophe::Tool_strophe(void) {
	define("l|list=b",         "list all possible variants");
	define("m=b",              "mark strophe music");
	define("mark|marker=s:@",  "character to mark with");
	define("c|color=s:red",    "character to mark with");
}



/////////////////////////////////
//
// Tool_strophe::run -- Do the main work of the tool.
//

bool Tool_strophe::run(HumdrumFileSet& infiles) {
	bool status = true;
	for (int i=0; i<infiles.getCount(); i++) {
		status &= run(infiles[i]);
	}
	for (auto it = m_variants.begin(); it != m_variants.end(); ++it) {
		m_free_text << *it << endl;
	}
	return status;
}


bool Tool_strophe::run(const string& indata, ostream& out) {
	HumdrumFile infile(indata);
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else if (!m_listQ) {
		out << infile;
	}
	return status;
}


bool Tool_strophe::run(HumdrumFile& infile, ostream& out) {
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else if (!m_listQ) {
		out << infile;
	}
	return status;
}


bool Tool_strophe::run(HumdrumFile& infile) {
	initialize();
	processFile(infile);
	return true;
}



//////////////////////////////
//
// Tool_strophe::initialize --  Initializations that only have to be done once
//    for all HumdrumFile segments.
//

void Tool_strophe::initialize(void) {
	m_listQ     = getBoolean("list");
	m_markQ     = getBoolean("m");
	m_marker    = getString("marker");
	m_color     = getString("color");
}



//////////////////////////////
//
// Tool_strophe::processFile --
//

void Tool_strophe::processFile(HumdrumFile& infile) {
	infile.analyzeStrophes();
	if (m_listQ) {
		displayStropheVariants(infile);
	} else {
		markWithColor(infile);
	}
}



//////////////////////////////
//
// Tool_strophe::markWithColor --  Maybe give different colors
//     to different variants.  Currently only marking the primary
//     strophe.
//

void Tool_strophe::markWithColor(HumdrumFile& infile) {
	int counter = 0;
	for (int i=0; i<infile.getStropheCount(); i++) {
		HTp strophestart = infile.getStropheStart(i);
		HTp stropheend = infile.getStropheEnd(i);
		counter += markStrophe(strophestart, stropheend);
	}
	if (counter) {
		string rdf = "!!!RDF**kern: ";
		rdf += m_marker;
		rdf += " = marked note, strophe";
		if (m_color != "red") {
			rdf += ", color=\"";
			rdf += m_color;
			rdf += "\"";
		}
		infile.appendLine(rdf);
		infile.createLinesFromTokens();
	}
}



//////////////////////////////
//
// Tool_strophe::markStrophe -- Returns the number of marked notes/rests.
//

int Tool_strophe::markStrophe(HTp strophestart, HTp stropheend) {
	HTp current = strophestart;
	int output = 0;
	while (current && current != stropheend) {
		if (current->isData() && !current->isNull()) {
			// Think about multiple marking for individual notes in chords.
			string value = current->getText();
			value += m_marker;
			current->setText(value);
			output++;
		}
		current = current->getNextToken();
	}
	return output;
}



//////////////////////////////
//
// displayStropheVariants --
//

void Tool_strophe::displayStropheVariants(HumdrumFile& infile) {
	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isInterpretation()) {
			continue;
		}
		for (int j=0; j<infile[i].getFieldCount(); j++) {
			HTp token = infile.token(i, j);
			if (token->compare(0, 3, "*S/") != 0) {
				continue;
			}
			string variant = token->substr(3);
			m_variants.insert(variant);
		}
	}
}





// END_MERGE

} // end namespace hum



