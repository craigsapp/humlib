//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sun Apr 23 16:44:17 PDT 2023
// Last Modified: Sun Apr 23 16:44:20 PDT 2023
// Filename:      kerninst.cpp
// Syntax:        C++11
//
// Description:   Add or remove **kern instrument information
///               from non-kern spines.
//

#include "humlib.h"
#include <fstream>
#include <iostream>
#include <string>

using namespace hum;
using namespace std;

// function definitions:
void processFile                 (HumdrumFile& infile);
void removeNonKernInstrumentInfo (HumdrumFile& infile);
void addNonKernInstrumentInfo    (HumdrumFile& infile);

// command-line options:
bool m_removeQ = false;
bool m_addQ    = false;
bool m_checkQ  = false;

///////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv) {
   Options options;
	options.define("a|add=b", "Add instrument information to **kern co-spines");
	options.define("r|remove=b", "Remove instrument information to **kern co-spines");
   options.process(argc, argv);

	m_addQ    = options.getBoolean("add");
	m_removeQ = options.getBoolean("remove");
	m_checkQ = !(m_addQ || m_removeQ);

	HumdrumFileStream instream(options);
	HumdrumFile infile;
	while (instream.read(infile)) {
		processFile(infile);
		infile.generateLinesFromTokens();
		if (m_removeQ || m_addQ) {
			cout << infile;
		}
	}

   return 0;
}



//////////////////////////////
//
// processFile --
//

void processFile(HumdrumFile& infile) {
	if (m_removeQ) {
		removeNonKernInstrumentInfo(infile);
	} else if (m_addQ) {
		addNonKernInstrumentInfo(infile);
	}
}



//////////////////////////////
//
// addNonKernInstrumentInfo -- Not yet implemented.
//

void addNonKernInstrumentInfo(HumdrumFile& infile) {
	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isInterpretation()) {
			continue;
		}
		for (int j=0; j<infile[i].getFieldCount(); j++) {
		}
	}
}



//////////////////////////////
//
// removeNonKernInstrumentInfo --
//

void removeNonKernInstrumentInfo(HumdrumFile& infile) {
	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isInterpretation()) {
			continue;
		}
		for (int j=0; j<infile[i].getFieldCount(); j++) {
			HTp token = infile.token(i, j);
			if (token->isKern()) {
				continue;
			}
			if (token->isInstrumentName()) {
				token->setText("*");
			} else if (token->isInstrumentAbbreviation()) {
				token->setText("*");
			} else if (token->isInstrumentCode()) {
				token->setText("*");
			} else if (token->isInstrumentClass()) {
				token->setText("*");
			} else if (token->isInstrumentGroup()) {
				token->setText("*");
			}
		}
	}
}



