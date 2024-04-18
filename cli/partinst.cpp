//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sun Apr 23 20:34:59 PDT 2023
// Last Modified: Sun Apr 23 20:35:02 PDT 2023
// Filename:      partinst.cpp
// Syntax:        C++11
//
// Description:   Add or remove **kern instrument information
//                in secondary part spines.
//
// To do:         Add -r option to reverse the process.
//

/*

Example input:

**kern	**kern
*part1	*part1
*ICklav	*
*Iklav	*
*I"Piano	*
*I'Pno.	*
1C		1c
*-	*-

Example output:

**kern	**kern
*part1	*part1
*ICklav	*ICklav
*Iklav	*Iklav
*I"Piano	*I"Piano
*I'Pno.	*I'Pno.
1C		1c
*-	*-


*/

#include "humlib.h"
#include <fstream>
#include <iostream>
#include <string>

using namespace hum;
using namespace std;

// function definitions:
void processFile                 (HumdrumFile& infile);

bool m_changedQ = false;

///////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv) {
   Options options;
   options.process(argc, argv);

	HumdrumFileStream instream(options);
	HumdrumFile infile;
	while (instream.read(infile)) {
		processFile(infile);
		if (m_changedQ) {
			infile.generateLinesFromTokens();
		}
		cout << infile;
	}

   return 0;
}



//////////////////////////////
//
// processFile --
//

void processFile(HumdrumFile& infile) {
	vector<string> partByTrack(infile.getMaxTrack() + 1);
	for (int i=0; i<infile.getLineCount(); i++) {
		if (infile[i].isBarline()) {
			// Stop looking after the first barline in the data.
			break;
		}
		if (!infile[i].isInterpretation()) {
			continue;
		}

		// Identify the part number for kern spines:
		for (int j=0; j<infile[i].getFieldCount(); j++) {
			HTp token = infile.token(i, j);
			if (!token->isKern()) {
				continue;
			}
			if (token->compare(0, 5, "*part") == 0) {
				int track = token->getTrack();
				partByTrack.at(track) = *token;
			}
		}

		// If a **kern spine is for the same part but has empty
		// instrument information that the previous spine for the part
		// does have, then copy the instrument information.  If the
		// target token is not a null interpretation (and not the same
		// content as the previous part spine, then print a warning
		// and do not change the token.
		HTp ptok = NULL;
		for (int j=0; j<infile[i].getFieldCount(); j++) {
			HTp token = infile.token(i, j);
			if (!token->isKern()) {
				// Only process **kern spines.
				continue;
			}
			int track = token->getTrack();
			if (partByTrack[track].empty()) {
				// There is no part info, so don't try to do anything.
				ptok = token;
				continue;
			}
			if (!ptok) {
				// If this is the first **kern token, then nothing to do.
				ptok = token;
				continue;
			}
			if (partByTrack.at(track) != partByTrack.at(ptok->getTrack())) {
				// If the two spines are not for the same part, then skip.
				ptok = token;
				continue;
			}
			if (*ptok == "*") {
				// If the previous kern token is null, then nothing to do.
				ptok = token;
				continue;
			}
			if (*ptok == *token) {
				// If the two tokens are the same, then don't need to worry.
				ptok = token;
				continue;
			}

			if (ptok->isInstrumentGroup() || ptok->isInstrumentClass()
					|| ptok->isInstrumentCode() || ptok->isInstrumentName()
					|| ptok->isInstrumentAbbreviation()
					) {
				if (*token != "*") {
					cerr << "WARNING: NOT REPLACING " << token << " WITH ";
					cerr << ptok << " AT (" << (i+1) << ", " << (j+1) << ") IN ";
					cerr << infile.getFilename() << "\n";
					ptok = token;
					continue;
				} else {
					token->setText(*ptok);
					m_changedQ = 1;
				}
			}
			ptok = token;
			continue;
		}
	}
}




