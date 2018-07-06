//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Fri Jul  6 00:05:27 CEST 2018
// Last Modified: Fri Jul  6 00:05:32 CEST 2018
// Filename:      tool-slur.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/tool-slur.cpp
// Syntax:        C++11
// vim:           ts=3 noexpandtab
//
// Description:   Interface for fixing unmatched slurs.
//

#include "tool-slur.h"
#include "Convert.h"
#include "HumRegex.h"

using namespace std;

namespace hum {

// START_MERGE


/////////////////////////////////
//
// Tool_slur::Tool_slur -- Set the recognized options for the tool.
//

Tool_slur::Tool_slur(void) {
	// add options here
}



/////////////////////////////////
//
// Tool_slur::run -- Do the main work of the tool.
//

bool Tool_slur::run(const string& indata, ostream& out) {
	HumdrumFile infile(indata);
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_slur::run(HumdrumFile& infile, ostream& out) {
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_slur::run(HumdrumFile& infile) {
	initialize();
	processFile(infile);
	infile.createLinesFromTokens();
	return true;
}



//////////////////////////////
//
// Tool_slur::initialize --
//

void Tool_slur::initialize(void) {
}



//////////////////////////////
//
// Tool_slur::processFile --
//

void Tool_slur::processFile(HumdrumFile& infile) {
	infile.analyzeKernSlurs();
	int opencount = 0;
	int closecount = 0;
	for (int i=0; i<infile.getStrandCount(); i++) {
		HTp stok = infile.getStrandStart(i);
		if (!stok->isKern()) {
			continue;
		}
		HTp etok = infile.getStrandEnd(i);
		HTp tok = stok;
		while (tok && (tok != etok)) {
			if (!tok->isData()) {
				tok = tok->getNextToken();
				continue;
			}
			if (tok->isNull()) {
				tok = tok->getNextToken();
				continue;
			}
			string value = tok->getValue("auto", "hangingSlur");
			if (value == "true") {
				string side = tok->getValue("auto", "slurSide");
				if (side == "start") {
					opencount++;
					string data = *tok;
					data += "i";
					tok->setText(data);
					// cerr << "TOK " << tok << " has an unclosed slur opening" << endl;
				} else if (side == "stop") {
					closecount++;
					string data = *tok;
					data += "j";
					tok->setText(data);
					// cerr << "TOK " << tok << " has an unopened slur closing" << endl;
				}
			}
			tok = tok->getNextToken();
		}
	}

	if (opencount + closecount == 0) {
		return;
	}

	if (opencount) {
		infile.appendLine("!!!RDF**kern: i = marked note, color=\"hotpink\", text=\"unclosed slur\"");
	}

	if (closecount) {
		infile.appendLine("!!!RDF**kern: j = marked note, color=\"magenta\", text=\"unopened slur\"");
	}

	infile.createLinesFromTokens();
}



// END_MERGE

} // end namespace hum



