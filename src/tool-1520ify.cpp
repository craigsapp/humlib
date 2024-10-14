//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Wed Oct 18 13:40:23 PDT 2017
// Last Modified: Wed Oct 18 13:40:26 PDT 2017
// Filename:      tool-1520ify.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/tool-1520ify.cpp
// Syntax:        C++11; humlib
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Refinements and corrections for TiM scores imported
//                from Finale/Sibelius/MuseScore.
//

#include "tool-1520ify.h"
#include "tool-shed.h"
#include "Convert.h"
#include "HumRegex.h"

#include <algorithm>
#include <chrono>
#include <iomanip>

using namespace std;

namespace hum {

// START_MERGE


/////////////////////////////////
//
// Tool_1520ify::Tool_1520ify -- Set the recognized options for the tool.
//

Tool_1520ify::Tool_1520ify(void) {
	define("R|no-reference-records=b",                "do not add reference records");
	define("r|only-add-reference-records=b",          "only add reference records");

	define("B|do-not-delete-breaks=b",                "do not delete system/page break markers");
	define("b|only-delete-breaks=b",                  "only delete breaks");

	define("A|do-not-fix-instrument-abbreviations=b", "do not fix instrument abbreviations");
	define("a|only-fix-instrument-abbreviations=b",   "only fix instrument abbreviations");

	define("E|do-not-fix-editorial-accidentals=b",    "do not fix instrument abbreviations");
	define("e|only-fix-editorial-accidentals=b",      "only fix editorial accidentals");

	define("T|do-not-add-terminal-longs=b",           "do not add terminal long markers");
	define("t|only-add-terminal-longs=b",             "only add terminal longs");

	define("N|do-not-remove-empty-transpositions=b",  "do not remove empty transposition instructions");
	define ("n|only-remove-empty-transpositions=b",   "only remove empty transpositions");
}



/////////////////////////////////
//
// Tool_1520ify::run -- Primary interfaces to the tool.
//

bool Tool_1520ify::run(HumdrumFileSet& infiles) {
	bool status = true;
	for (int i=0; i<infiles.getCount(); i++) {
		status &= run(infiles[i]);
	}
	return status;
}


bool Tool_1520ify::run(const string& indata, ostream& out) {
	HumdrumFile infile(indata);
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_1520ify::run(HumdrumFile& infile, ostream& out) {
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}

//
// In-place processing of file:
//

bool Tool_1520ify::run(HumdrumFile& infile) {
	processFile(infile);

	// Re-load the text for each line from their tokens.
	infile.createLinesFromTokens();

	// Need to adjust the line numbers for tokens for later
	// processing.
	m_humdrum_text << infile;
	return true;
}



//////////////////////////////
//
// Tool_1520ify::processFile --
//

void Tool_1520ify::processFile(HumdrumFile& infile) {

	bool abbreviationsQ  = true;
	bool accidentalsQ    = true;
	bool referencesQ     = true;
	bool terminalsQ      = true;
	bool breaksQ         = true;
	bool transpositionsQ = true;

	if (getBoolean("no-reference-records")) { referencesQ = false; }
	if (getBoolean("only-add-reference-records")) {
		abbreviationsQ  = false;
		accidentalsQ    = false;
		referencesQ     = true;
		terminalsQ      = false;
		breaksQ         = false;
		transpositionsQ = false;
	}

	if (getBoolean("do-not-delete-breaks")) { breaksQ = false; }
	if (getBoolean("only-delete-breaks")) {
		abbreviationsQ  = false;
		accidentalsQ    = false;
		referencesQ     = false;
		terminalsQ      = false;
		breaksQ         = true;
		transpositionsQ = false;
	}

	if (getBoolean("do-not-fix-instrument-abbreviations")) { abbreviationsQ = false; }
	if (getBoolean("only-fix-instrument-abbreviations")) {
		abbreviationsQ  = true;
		accidentalsQ    = false;
		referencesQ     = false;
		terminalsQ      = false;
		breaksQ         = false;
		transpositionsQ = false;
	}

	if (getBoolean("do-not-fix-editorial-accidentals")) { accidentalsQ = false; }
	if (getBoolean("only-fix-editorial-accidentals")) {
		abbreviationsQ  = false;
		accidentalsQ    = true;
		referencesQ     = false;
		terminalsQ      = false;
		breaksQ         = false;
		transpositionsQ = false;
	}

	if (getBoolean("do-not-add-terminal-longs")) { terminalsQ = false; }
	if (getBoolean("only-add-terminal-longs")) {
		abbreviationsQ  = false;
		accidentalsQ    = false;
		referencesQ     = false;
		terminalsQ      = true;
		breaksQ         = false;
		transpositionsQ = false;
	}

	if (getBoolean("do-not-remove-empty-transpositions")) { transpositionsQ = false; }
	if (getBoolean("only-remove-empty-transpositions")) {
		abbreviationsQ  = false;
		accidentalsQ    = false;
		referencesQ     = false;
		terminalsQ      = false;
		breaksQ         = false;
		transpositionsQ = true;
	}

	if (abbreviationsQ)  { fixInstrumentAbbreviations(infile); }
	if (accidentalsQ)    { fixEditorialAccidentals(infile); }
	if (referencesQ)     { addBibliographicRecords(infile); }
	if (terminalsQ)      { addTerminalLongs(infile); }
	if (breaksQ)         { deleteBreaks(infile); }
	if (transpositionsQ) { deleteDummyTranspositions(infile); }

	adjustSystemDecoration(infile);

	// Input lyrics may contain "=" signs which are to be converted into
	// spaces in **text data, and into elisions when displaying with verovio.
	Tool_shed shed;
	vector<string> argv;
	argv.push_back("shed");
	argv.push_back("-x");     // only apply to **text spines
	argv.push_back("text");
	argv.push_back("-e");
	argv.push_back("s/=/ /g");
	shed.process(argv);
	shed.run(infile);
}



//////////////////////////////
//
// Tool_1520ify::adjustSystemDecoration --
//    !!!system-decoration: [(s1)(s2)(s3)(s4)]
// to:
//    !!!system-decoration: [*]
//

void Tool_1520ify::adjustSystemDecoration(HumdrumFile& infile) {
	for (int i=infile.getLineCount() - 1; i>=0; i--) {
		if (!infile[i].isReference()) {
			continue;
		}
		HTp token = infile.token(i, 0);
		if (token->compare(0, 21, "!!!system-decoration:") == 0) {
			token->setText("!!!system-decoration: [*]");
			break;
		}
	}
}



//////////////////////////////
//
// Tool_1520ify::deleteDummyTranspositions -- Somehow empty
//    transpositions that go to the same pitch can appear in the
//    MusicXML data, so remove them here.  Example:
// 		*Trd0c0
//

void Tool_1520ify::deleteDummyTranspositions(HumdrumFile& infile) {
	std::vector<int> ldel;
	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].hasSpines()) {
			continue;
		}
		if (!infile[i].isInterpretation()) {
			continue;
		}
		bool empty = true;
		for (int j=0; j<infile[i].getFieldCount(); j++) {
			HTp token = infile.token(i, j);
			if (*token == "*") {
				continue;
			}
			if (!token->isKern()) {
				empty = false;
				continue;
			}
			if (*token == "*Trd0c0") {
				token->setText("*");
			} else {
				empty = false;
			}
		}
		if (empty) {
			ldel.push_back(i);
		}
	}

	if (ldel.size() == 1) {
		infile.deleteLine(ldel[0]);
	} else if (ldel.size() > 1) {
		cerr << "Warning: multiple transposition lines, not deleting them" << endl;
	}

}


//////////////////////////////
//
// Tool_1520ify::fixEditorialAccidentals -- checkDataLine() does
//       all of the work for this function, which only manages
//       key signature and barline processing.
//    Rules for accidentals in Tasso in Music Project:
//    (1) Only note accidentals printed in the source editions
//        are displayed as regular accidentals.  These accidentals
//        are postfixed with an "X" in the **kern data.
//    (2) Editorial accidentals are given an "i" marker but not
//        a "X" marker in the **kern data.  This editorial accidental
//        is displayed above the note.
//    This algorithm makes adjustments to the input data because
//    Sibelius will drop editorial information after the frist
//    editorial accidental on that pitch in the measure.
//    (3) If a note is the same pitch as a previous note in the
//        measure and the previous note has an editorial accidental,
//        then make the note an editorial note.  However, if the
//        accidental state of the note matches the key-signature,
//        then do not add an editorial accidental, and there will be
//        no accidental displayed on the note.  In that case, add a "y"
//        after the accidental to indicate that it is interpreted
//        and not visible in the original score.
//

void Tool_1520ify::fixEditorialAccidentals(HumdrumFile& infile) {
	m_pstates.resize(infile.getMaxTrack() + 1);
	m_estates.resize(infile.getMaxTrack() + 1);
	m_kstates.resize(infile.getMaxTrack() + 1);

	for (int i=0; i<(int)m_pstates.size(); i++) {
		m_pstates[i].resize(70);
		fill(m_pstates[i].begin(), m_pstates[i].end(), 0);
		m_kstates[i].resize(70);
		fill(m_kstates[i].begin(), m_kstates[i].end(), 0);
		m_estates[i].resize(70);
		fill(m_estates[i].begin(), m_estates[i].end(), false);
	}

	for (int i=0; i<infile.getLineCount(); i++) {
		if (infile[i].isInterpretation()) {
			updateKeySignatures(infile, i);
			continue;
		} else if (infile[i].isBarline()) {
			clearStates();
			continue;
		} else if (infile[i].isData()) {
			checkDataLine(infile, i);
		}
	}
}



//////////////////////////////
//
// Tool_1520ify::addTerminalLongs -- Convert all last notes to terminal longs
//    Also probably add terminal longs before double barlines as in JRP.
//

void Tool_1520ify::addTerminalLongs(HumdrumFile& infile) {
	int scount = infile.getStrandCount();
	for (int i=0; i<scount; i++) {
		HTp cur = infile.getStrandEnd(i);
		if (*cur != "*-") {
			continue;
		}
		if (!cur->isKern()) {
			continue;
		}
		while (cur) {
			if (!cur->isData()) {
				cur = cur->getPreviousToken();
				continue;
			}
			if (cur->isNull()) {
				cur = cur->getPreviousToken();
				continue;
			}
			if (cur->isRest()) {
				cur = cur->getPreviousToken();
				continue;
			}
			if (cur->isSecondaryTiedNote()) {
				cur = cur->getPreviousToken();
				continue;
			}
			if (cur->find("l") != std::string::npos) {
				// already marked so do not do it again
				break;
			}
			// mark this note with "l"
			string newtext = *cur;
			newtext += "l";
			cur->setText(newtext);
			break;
		}
	}
}



//////////////////////////////
//
// Tool_1520ify::fixInstrumentAbbreviations --
//

void Tool_1520ify::fixInstrumentAbbreviations(HumdrumFile& infile) {
	int iline = -1;
	int aline = -1;

	std::vector<HTp> kerns = infile.getKernSpineStartList();
	if (kerns.empty()) {
		return;
	}

	HTp cur = kerns[0];
	while (cur) {
		if (cur->isData()) {
			break;
		}
		if (cur->compare(0, 3, "*I\"") == 0) {
			iline = cur->getLineIndex();
		} else if (cur->compare(0, 3, "*I'") == 0) {
			aline = cur->getLineIndex();
		}
		cur = cur->getNextToken();
	}

	if (iline < 0) {
		// no names to create abbreviations for
		return;
	}
	if (aline < 0) {
		// not creating a new abbreviation for now
		// (could add later).
		return;
	}
	if (infile[iline].getFieldCount() != infile[aline].getFieldCount()) {
		// no spine splitting between the two lines.
		return;
	}
	// Maybe also require them to be adjacent to each other.
	HumRegex hre;
	for (int j=0; j<(int)infile[iline].getFieldCount(); j++) {
		if (!infile.token(iline, j)->isKern()) {
			continue;
		}
		if (!hre.search(*infile.token(iline, j), "([A-Za-z][A-Za-z .0-9]+)")) {
			continue;
		}
		string name = hre.getMatch(1);
		string abbr = "*I'";
		if (name == "Basso Continuo") {
			abbr += "BC";
		} else if (name == "Basso continuo") {
			abbr += "BC";
		} else if (name == "basso continuo") {
			abbr += "BC";
		} else {
			abbr += toupper(name[0]);
		}
		// check for numbers after the end of the name and add to abbreviation
		infile.token(aline, j)->setText(abbr);
	}
}



//////////////////////////////
//
// Tool_1520ify::deleteBreaks --
//

void Tool_1520ify::deleteBreaks(HumdrumFile& infile) {
	HumRegex hre;
	for (int i=infile.getLineCount()-1; i>= 0; i--) {
		if (!infile[i].isGlobalComment()) {
			continue;
		}
		if (hre.search(*infile.token(i, 0), "linebreak\\s*:\\s*original")) {
			infile.deleteLine(i);
		}
		else if (hre.search(*infile.token(i, 0), "pagebreak\\s*:\\s*original")) {
			infile.deleteLine(i);
		}
	}
}



////////////////////////////////
//
// Tool_1520ify::addBibliographicRecords --
//
// !!!!SEGMENT:
// !!!renid:
// !!!AGN:
// !!!voices:
// !!!COM:
// !!!CDT:
// !!!OTL:
// !!!OPR:
//
// At end:
// !!!RDF**kern: l = terminal long        (if needed)
// !!!RDF**kern: i = editorial accidental (if needed)
// !!!ENC: Benjamin Ory
// !!!END: 
// !!!EED: Benjamin Ory
// !!!EEV: <DATE>
// !!!YEC: Copyright <YEAR> Benjamin Ory, All Rights Reserved
// !!!ONB: Translated from MusicXML and edited on $DATE
//

void Tool_1520ify::addBibliographicRecords(HumdrumFile& infile) {
	std::vector<HLp> refinfo = infile.getReferenceRecords();
	std::map<string, HLp> refs;
	for (int i=0; i<(int)refinfo.size(); i++) {
		string key = refinfo[i]->getReferenceKey();
		refs[key] = refinfo[i];
	}

	// header records (inserted in reverse order)
	// automatic placement before/after existing records
	// could be improved later.
	if (refs.find("OPR") == refs.end()) {
		if (infile.token(0, 0)->find("!!!OPR") != std::string::npos) {
			infile.insertLine(1, "!!!OPR:");
		} else {
			infile.insertLine(0, "!!!OPR:");
		}
	}

	if (refs.find("OTL") == refs.end()) {
		if (infile.token(0, 0)->find("!!!OTL") != std::string::npos) {
			infile.insertLine(1, "!!!OTL:");
		} else {
			infile.insertLine(0, "!!!OTL:");
		}
	}

	if (refs.find("CDT") == refs.end()) {
		if (infile.token(0, 0)->find("!!!CDT") != std::string::npos) {
			infile.insertLine(1, "!!!CDT:");
		} else {
			infile.insertLine(0, "!!!CDT:");
		}
	}

	if (refs.find("COM") == refs.end()) {
		if (infile.token(0, 0)->find("!!!COM") != std::string::npos) {
			infile.insertLine(1, "!!!COM:");
		} else {
			infile.insertLine(0, "!!!COM:");
		}
	}

	if (refs.find("voices") == refs.end()) {
		if (infile.token(0, 0)->find("!!!voices") != std::string::npos) {
			infile.insertLine(1, "!!!voices:");
		} else {
			infile.insertLine(0, "!!!voices:");
		}
	}

	if (refs.find("AGN") == refs.end()) {
		if (infile.token(0, 0)->find("!!!AGN") != std::string::npos) {
			infile.insertLine(1, "!!!AGN:");
		} else {
			infile.insertLine(0, "!!!AGN:");
		}
	}

	if (refs.find("renid") == refs.end()) {
		if (infile.token(0, 0)->find("!!!renid") != std::string::npos) {
			infile.insertLine(1, "!!!renid:");
		} else {
			infile.insertLine(0, "!!!renid:");
		}
	}

	infile.insertLine(0, "!!!!SEGMENT:");

	int year = getYear();
	string date = getDate();



	// trailer records
	bool foundi = false;
	bool foundl = false;
	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isReference()) {
			continue;
		}
		if (infile.token(i, 0)->find("!!!RDF**kern:") == std::string::npos) {
			continue;
		}
		if (infile.token(i, 0)->find("terminal long") != std::string::npos) {
			foundl = true;
		} else if (infile.token(i, 0)->find("editorial accidental") != std::string::npos) {
			foundi = true;
		}
	}
	if (!foundi) {
		infile.appendLine("!!!RDF**kern: i = editorial accidental");
	}
	if (!foundl) {
		infile.appendLine("!!!RDF**kern: l = terminal long");
	}

	if (refs.find("ENC") == refs.end()) {
		infile.appendLine("!!!ENC: Benjamin Ory");
	}
	if (refs.find("END") == refs.end()) {
		infile.appendLine("!!!END:");
	}
	if (refs.find("EED") == refs.end()) {
		infile.appendLine("!!!EED: Benjamin Ory");
	}
	if (refs.find("EEV") == refs.end()) {
		string line = "!!!EEV: " + date;
		infile.appendLine(line);
	}
	if (refs.find("YEC") == refs.end()) {
		string line = "!!!YEC: Copyright ";
		line += to_string(year);
		line += " Benjamin Ory, All Rights Reserved";
		infile.appendLine(line);
	}
	if (refs.find("ONB") == refs.end()) {
		string line = "!!!ONB: Translated from MusicXML on " + date;
		infile.appendLine(line);
	}

}



////////////////////////////////
//
// Tool_1520ify::checkDataLine --
//

void Tool_1520ify::checkDataLine(HumdrumFile& infile, int lineindex) {
	HumdrumLine& line = infile[lineindex];

	HumRegex hre;
	HTp token;
	bool haseditQ;
	int base7;
	int accid;
	int track;
	bool removeQ;
	for (int i=0; i<line.getFieldCount(); i++) {
		token = line.token(i);
		track = token->getTrack();
		if (!token->isKern()) {
			continue;
		}
		if (token->isNull()) {
			continue;
		}
		if (token->isRest()) {
			continue;
		}
		if (token->isSecondaryTiedNote()) {
			continue;
		}

		base7 = Convert::kernToBase7(token);
		accid = Convert::kernToAccidentalCount(token);
		haseditQ = false;
		removeQ = false;

		// Hard-wired to "i" as editorial accidental marker
		if (token->find("ni") != string::npos) {
			haseditQ = true;
		} else if (token->find("-i") != string::npos) {
			haseditQ = true;
		} else if (token->find("#i") != string::npos) {
			haseditQ = true;
		} else if (token->find("nXi") != string::npos) {
			haseditQ = true;
			removeQ = true;
		} else if (token->find("-Xi") != string::npos) {
			haseditQ = true;
			removeQ = true;
		} else if (token->find("#Xi") != string::npos) {
			haseditQ = true;
			removeQ = true;
		}

		if (removeQ) {
			string temp = *token;
			hre.replaceDestructive(temp, "", "X");
			token->setText(temp);
		}

		bool explicitQ = false;
		if (token->find("#X") != string::npos) {
			explicitQ = true;
		} else if (token->find("-X") != string::npos) {
			explicitQ = true;
		} else if (token->find("nX") != string::npos) {
			explicitQ = true;
		} else if (token->find("n") != string::npos) {
			// add an explicit accidental marker
			explicitQ = true;
			string text = *token;
			hre.replaceDestructive(text, "nX", "n");
			token->setText(text);
		}

		if (haseditQ) {
			// Store new editorial pitch state.
			m_estates.at(track).at(base7) = true;
			m_pstates.at(track).at(base7) = accid;
			continue;
		}

		if (explicitQ) {
			// No need to make editorial since it is visible.
			m_estates.at(track).at(base7) = false;
			m_pstates.at(track).at(base7) = accid;
			continue;
		}

		if (accid == m_kstates.at(track).at(base7)) {
			// 	!m_estates.at(track).at(base7)) {
			// add !m_estates.at(track).at(base) as a condition if
			// you want editorial accidentals to be added to return the
			// note to the accidental in the key.
			//
			// The accidental matches the key-signature state,
			// so it should not be made editorial eventhough
			// it is not visible.
			m_pstates.at(track).at(base7) = accid;

			// Add a "y" marker of there is an interpreted accidental
			// state (flat or sharp) that is part of the key signature.
			int hasaccid = false;
			if (token->find("#") != std::string::npos) {
				hasaccid = true;
			} else if (token->find("-") != std::string::npos) {
				hasaccid = true;
			}
			int hashide = false;
			if (token->find("-y") != std::string::npos) {
				hashide = true;
			}
			else if (token->find("#y") != std::string::npos) {
				hashide = true;
			}
			if (hasaccid && !hashide) {
				string text = *token;
				hre.replaceDestructive(text, "#y", "#");
				hre.replaceDestructive(text, "-y", "-");
				token->setText(text);
			}

			continue;
		}

		// At this point the previous note with this pitch class
		// had an editorial accidental, and this note also has the
		// same accidental, or there was a previous visual accidental
		// outside of the key signature that will cause this note to have
		// an editorial accidental mark applied (Sibelius will drop
		// secondary editorial accidentals in a measure when exporting,
		// MusicXML, which is why this function is needed).

		m_estates[track][base7] = true;
		m_pstates[track][base7] = accid;

		string text = token->getText();
		string output = "";
		bool foundQ = false;
		for (int j=0; j<(int)text.size(); j++) {
			if (text[j] == 'n') {
				output += "ni";
				foundQ = true;
			} else if (text[j] == '#') {
				output += "#i";
				foundQ = true;
			} else if (text[j] == '-') {
				output += "-i";
				foundQ = true;
			} else {
				output += text[j];
			}
		}

		if (foundQ) {
			token->setText(output);
			continue;
		}

		// The note is natural, but has no natural sign.
		// add the natural sign and editorial mark.
		for (int j=(int)output.size()-1; j>=0; j--) {
			if ((tolower(output[j]) >= 'a') && (tolower(output[j]) <= 'g')) {
				output.insert(j+1, "ni");
				break;
			}
		}
		token->setText(output);
	}
}



////////////////////////////////
//
// Tool_1520ify::updateKeySignatures -- Fill in the accidental
//    states for each diatonic pitch.
//

void Tool_1520ify::updateKeySignatures(HumdrumFile& infile, int lineindex) {
	HumdrumLine& line = infile[lineindex];
	int track;
	for (int i=0; i<line.getFieldCount(); i++) {
		if (!line.token(i)->isKeySignature()) {
			continue;
		}
		HTp token = line.token(i);
		track = token->getTrack();
		string text = token->getText();
		fill(m_kstates[track].begin(), m_kstates[track].end(), 0);
		for (int j=3; j<(int)text.size()-1; j++) {
			if (text[j] == ']') {
				break;
			}
			switch (text[j]) {
				case 'a': case 'A':
					switch (text[j+1]) {
						case '#': m_kstates[track][5] = +1;
						break;
						case '-': m_kstates[track][5] = -1;
						break;
					}
					break;

				case 'b': case 'B':
					switch (text[j+1]) {
						case '#': m_kstates[track][6] = +1;
						break;
						case '-': m_kstates[track][6] = -1;
						break;
					}
					break;

				case 'c': case 'C':
					switch (text[j+1]) {
						case '#': m_kstates[track][0] = +1;
						break;
						case '-': m_kstates[track][0] = -1;
						break;
					}
					break;

				case 'd': case 'D':
					switch (text[j+1]) {
						case '#': m_kstates[track][1] = +1;
						break;
						case '-': m_kstates[track][1] = -1;
						break;
					}
					break;

				case 'e': case 'E':
					switch (text[j+1]) {
						case '#': m_kstates[track][2] = +1;
						break;
						case '-': m_kstates[track][2] = -1;
						break;
					}
					break;

				case 'f': case 'F':
					switch (text[j+1]) {
						case '#': m_kstates[track][3] = +1;
						break;
						case '-': m_kstates[track][3] = -1;
						break;
					}
					break;

				case 'g': case 'G':
					switch (text[j+1]) {
						case '#': m_kstates[track][4] = +1;
						break;
						case '-': m_kstates[track][4] = -1;
						break;
					}
					break;
			}
			for (int j=0; j<7; j++) {
				if (m_kstates[track][j] == 0) {
					continue;
				}
				for (int k=1; k<10; k++) {
					m_kstates[track][j+k*7] = m_kstates[track][j];
				}
			}
		}
	}

	// initialize m_pstates with contents of m_kstates
	for (int i=0; i<(int)m_kstates.size(); i++) {
		for (int j=0; j<(int)m_kstates[i].size(); j++) {
			m_pstates[i][j] = m_kstates[i][j];
		}
	}

}



////////////////////////////////
//
// Tool_1520ify::clearStates --
//

void Tool_1520ify::clearStates(void) {
	for (int i=0; i<(int)m_pstates.size(); i++) {
		fill(m_pstates[i].begin(), m_pstates[i].end(), 0);
	}
	for (int i=0; i<(int)m_estates.size(); i++) {
		fill(m_estates[i].begin(), m_estates[i].end(), false);
	}
}


//////////////////////////////
//
// Tool_1520ify::getDate --
//

string Tool_1520ify::getDate(void) {
	auto now = std::chrono::system_clock::now();
	std::time_t now_time = std::chrono::system_clock::to_time_t(now);
	std::tm* local_time = std::localtime(&now_time);
	int year = local_time->tm_year + 1900;
	int month = local_time->tm_mon + 1; 
	int day = local_time->tm_mday;
	stringstream ss;
	ss << year << "/";
	ss << std::setw(2) << std::setfill('0') << month << "/";
	ss << std::setw(2) << std::setfill('0') << day;
	return ss.str();
}



//////////////////////////////
//
// Tool_1520ify::getYear --
//

int Tool_1520ify::getYear(void) {
	auto now = std::chrono::system_clock::now();
	std::time_t now_time = std::chrono::system_clock::to_time_t(now);
	std::tm* local_time = std::localtime(&now_time);
	int year = local_time->tm_year + 1900;
	return year;
}



// END_MERGE

} // end namespace hum



