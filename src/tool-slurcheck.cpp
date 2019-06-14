//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Fri Jul  6 00:05:27 CEST 2018
// Last Modified: Fri Jul  6 22:06:26 CEST 2018
// Filename:      tool-slurcheck.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/tool-slurcheck.cpp
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Interface for fixing unmatched slurs.
//

#include "tool-slurcheck.h"
#include "Convert.h"
#include "HumRegex.h"

using namespace std;

namespace hum {

// START_MERGE


/////////////////////////////////
//
// Tool_slurcheck::Tool_slurcheck -- Set the recognized options for the tool.
//

Tool_slurcheck::Tool_slurcheck(void) {
	// add options here
	define("l|list=b", "list locations of unclosed slur endings");
	define("c|count=b", "count unclosed slur endings");
	define("Z|no-zeros=b", "do not list files that have zero unclosed slurs in counts");
	define("f|filename=b", "print filename for list and count options");
}



/////////////////////////////////
//
// Tool_slurcheck::run -- Do the main work of the tool.
//

bool Tool_slurcheck::run(const string& indata, ostream& out) {
	HumdrumFile infile(indata);
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_slurcheck::run(HumdrumFile& infile, ostream& out) {
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_slurcheck::run(HumdrumFile& infile) {
	initialize();
	processFile(infile);
	infile.createLinesFromTokens();
	return true;
}



//////////////////////////////
//
// Tool_slurcheck::initialize --
//

void Tool_slurcheck::initialize(void) {
}



//////////////////////////////
//
// Tool_slurcheck::processFile --
//

void Tool_slurcheck::processFile(HumdrumFile& infile) {
	infile.analyzeKernSlurs();
	int opencount = 0;
	int closecount = 0;
	int listQ  = getBoolean("list");
	int countQ = getBoolean("count");
	int zeroQ = !getBoolean("no-zeros");
	int filenameQ  = getBoolean("filename");
	if (listQ || countQ) {
		suppressHumdrumFileOutput();
	}
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
					if (listQ) {
						if (filenameQ) {
							m_free_text << infile.getFilename() << ":\t";
						}
						m_free_text << "UNCLOSED SLUR\tline:" << tok->getLineIndex()+1 
								<< "\tfield:" << tok->getFieldIndex()+1 << "\ttoken:" << tok << endl;
					} else if (!countQ) {
						string data = *tok;
						data += "i";
						tok->setText(data);
					}
				} else if (side == "stop") {
					closecount++;
					if (listQ) {
						if (filenameQ) {
							m_free_text << infile.getFilename() << ":\t";
						}
						m_free_text << "UNOPENED SLUR\tline:" << tok->getLineIndex()+1 
								<< "\tfield:" << tok->getFieldIndex()+1 << "\ttoken:" << tok << endl;
					} else if (!countQ) {
						string data = *tok;
						data += "j";
						tok->setText(data);
					}
				}
			}
			tok = tok->getNextToken();
		}
	}

	if (countQ) {
		int sum = opencount + closecount;
		if ((!zeroQ) && (sum == 0)) {
			return;
		}
		if (filenameQ) {
			m_free_text << infile.getFilename() << ":\t";
		}
		m_free_text << (opencount + closecount) << "\t(:" << opencount << "\t):" << closecount << endl;
	}

	if (countQ || listQ) {
		return;
	}

	if (opencount + closecount == 0) {
		return;
	}

	if (opencount) {
		infile.appendLine("!!!RDF**kern: i = marked note, color=\"hotpink\", text=\"extra(\"");
	}

	if (closecount) {
		infile.appendLine("!!!RDF**kern: j = marked note, color=\"magenta\", text=\"extra)\"");
	}

	infile.createLinesFromTokens();
}



// END_MERGE

} // end namespace hum



