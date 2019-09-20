//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sun Jul 14 01:03:07 CEST 2019
// Last Modified: Sun Jul 14 01:03:12 CEST 2019
// Filename:      tool-composite.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/tool-composite.cpp
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Generate composite rhythm spine for music.
//

#include "tool-composite.h"
#include "tool-extract.h"
#include "tool-autobeam.h"
#include "Convert.h"
#include "HumRegex.h"

using namespace std;

namespace hum {

// START_MERGE


/////////////////////////////////
//
// Tool_composite::Tool_composite -- Set the recognized options for the tool.
//

Tool_composite::Tool_composite(void) {
	define("pitch=s:e",    "pitch to display for composite rhythm");
	define("a|append=b",   "append data to end of line");
	define("p|prepend=b",  "prepend data to end of line");
	define("b|beam=b",     "apply automatic beaming");
	define("G|no-grace=b", "do not include grace notes");
}



/////////////////////////////////
//
// Tool_composite::run -- Do the main work of the tool.
//

bool Tool_composite::run(HumdrumFileSet& infiles) {
	bool status = true;
	for (int i=0; i<infiles.getCount(); i++) {
		status &= run(infiles[i]);
	}
	return status;
}


bool Tool_composite::run(const string& indata, ostream& out) {
	HumdrumFile infile;
	infile.readStringNoRhythm(indata);
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_composite::run(HumdrumFile& infile, ostream& out) {
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_composite::run(HumdrumFile& infile) {
	initialize();
	processFile(infile);
	infile.createLinesFromTokens();
	// need to convert to text for now:
	m_humdrum_text << infile;
	return true;
}



//////////////////////////////
//
// Tool_composite::initialize --
//

void Tool_composite::initialize(void) {
	m_pitch = getString("pitch");
}



//////////////////////////////
//
// Tool_composite::processFile --
//

void Tool_composite::processFile(HumdrumFile& infile) {
	Tool_extract extract;
	bool appendQ = getBoolean("append");
	bool prependQ = getBoolean("prepend");
	bool graceQ = !getBoolean("no-grace");

	if (appendQ) {
		extract.setModified("s", "1-$,0");
	} else {
		extract.setModified("s", "0,1-$");
	}

	vector<HumNum> durations(infile.getLineCount());
	for (int i=0; i<infile.getLineCount(); i++) {
		durations[i] = infile[i].getDuration();
	}

	vector<bool> isRest(infile.getLineCount(), false);
	for (int i=0; i<infile.getLineCount(); i++) {
		if (durations[i] == 0) {
			continue;
		}
		// bool allnull = true;
		bool allrest = true;
		for (int j=0; j<infile[i].getFieldCount(); j++) {
			HTp tok = infile.token(i, j);
			if (!tok->isKern()) {
				continue;
			}
			if (tok->isNote()) {
				// allnull = false;
				allrest = false;
				break;
			}
			if (tok->isRest()) {
				// allnull = false;
			}
		}
		if (allrest) {
			isRest[i] = true;
		}
	}

	string pstring = getString("pitch");

	HumRegex hre;

	extract.run(infile);
	infile.readString(extract.getAllText());
	HTp token;
	for (int i=0; i<infile.getLineCount(); i++) {
		if (infile[i].isInterpretation()) {
			if (appendQ) {
				token = infile.token(i, infile[i].getFieldCount() - 1);
			} else {
				token = infile.token(i, 0);
			}
			if (token->compare("**blank") == 0) {
				token->setText("**kern");
				continue;
			}
			// copy time signature and tempos
			for (int j=1; j<infile[i].getFieldCount(); j++) {
				HTp stok = infile.token(i, j);
				if (stok->isTempo()) {
					token->setText(*stok);
				} else if (stok->isTimeSignature()) {
					token->setText(*stok);
				} else if (stok->isMensurationSymbol()) {
					token->setText(*stok);
				} else if (stok->isKeySignature()) {
					token->setText(*stok);
				} else if (stok->isKeyDesignation()) {
					token->setText(*stok);
				}
			}
		}
		if (!infile[i].isData()) {
			continue;
		}
		if (infile[i].getDuration() == 0) {
			// Grace note data line (most likely)
			if (!graceQ) {
				continue;
			}
			// otherwise, borrow the view of the first grace note found on the line
			// (beaming, visual duration) and apply the target pitch to the grace note.
			for (int j=0; j<infile[i].getFieldCount(); j++) {
				HTp tok = infile.token(i, j);
				if (!tok->isKern()) {
					continue;
				}
				if (tok->isNull()) {
					continue;
				}
				if (tok->isGrace()) {
					string q;
					string beam;
					string recip;
					if (hre.search(tok, "(\\d+%?\\d*\\.*)")) {
						recip = hre.getMatch(1);
					}
					if (hre.search(tok, "([LJk]+)")) {
						beam = hre.getMatch(1);
					}
					if (hre.search(tok, "(q+)")) {
						q = hre.getMatch(1);
					}
					string full;
					full += recip;
					full += q;
					full += pstring;
					full += beam;
					HTp targettok = infile.token(i, 0);
					if (appendQ) {
						targettok = infile.token(i, infile[i].getFieldCount() - 1);
					}
					targettok->setText(full);
					break;
				}
			}
			continue;
		}
		string recip = Convert::durationToRecip(infile[i].getDuration());

		if (appendQ) {
			token = infile.token(i, infile[i].getFieldCount() - 1);
		} else {
			token = infile.token(i, 0);
		}
		if (isRest[i]) {
			recip += "r";
		} else {
			recip += pstring;
		}
		token->setText(recip);
	}

	if (!(appendQ || prependQ)) {
		Tool_extract extract2;
		extract2.setModified("s", "1");
		extract2.run(infile);
		infile.readString(extract2.getAllText());
	}

	if (getBoolean("beam")) {
		Tool_autobeam autobeam;
		if (appendQ) {
			int trackcount =  infile.getTrackCount();
			string tstring = to_string(trackcount);
			autobeam.setModified("t", tstring);
		} else {
			autobeam.setModified("t", "1");
		}
		// need to analyze structure for some reason:
		infile.analyzeStrands();
		autobeam.run(infile);
	}


}



// END_MERGE

} // end namespace hum



