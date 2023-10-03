//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Thu Jul 29 09:13:04 CEST 2021
// Last Modified: Wed Aug  4 08:25:57 CEST 2021
// Filename:      tool-gasparize.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/tool-gasparize.cpp
// Syntax:        C++11; humlib
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Refinements and corrections for Gaspar scores imported
//                from Finale.
//

#include "tool-gasparize.h"
#include "tool-shed.h"
#include "Convert.h"
#include "HumRegex.h"

#include <algorithm>

using namespace std;

namespace hum {

// START_MERGE


/////////////////////////////////
//
// Tool_gasparize::Tool_gasparize -- Set the recognized options for the tool.
//

Tool_gasparize::Tool_gasparize(void) {
	define("R|no-reference-records=b", "Do not add reference records");
	define("r|only-add-reference-records=b", "Only add reference records");

	define("B|do-not-delete-breaks=b", "Do not delete system/page break markers");
	define("b|only-delete-breaks=b", "only delete breaks");

	define("A|do-not-fix-instrument-abbreviations=b", "Do not fix instrument abbreviations");
	define("a|only-fix-instrument-abbreviations=b", "Only fix instrument abbreviations");

	define("E|do-not-fix-editorial-accidentals=b", "Do not fix instrument abbreviations");
	define("e|only-fix-editorial-accidentals=b", "Only fix editorial accidentals");

	define("T|do-not-add-terminal-longs=b", "Do not add terminal long markers");
	define("t|only-add-terminal-longs=b", "Only add terminal longs");

	define("no-ties=b", "Do not fix tied notes");

	define("N|do-not-remove-empty-transpositions=b", "Do not remove empty transposition instructions");
	define ("n|only-remove-empty-transpositions=b", "Only remove empty transpositions");
}



/////////////////////////////////
//
// Tool_gasparize::run -- Primary interfaces to the tool.
//

bool Tool_gasparize::run(HumdrumFileSet& infiles) {
	bool status = true;
	for (int i=0; i<infiles.getCount(); i++) {
		status &= run(infiles[i]);
	}
	return status;
}


bool Tool_gasparize::run(const string& indata, ostream& out) {
	HumdrumFile infile(indata);
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_gasparize::run(HumdrumFile& infile, ostream& out) {
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

bool Tool_gasparize::run(HumdrumFile& infile) {
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
// Tool_gasparize::processFile --
//

void Tool_gasparize::processFile(HumdrumFile& infile) {

	bool mensurationQ    = true;
	bool articulationsQ  = true;
	bool abbreviationsQ  = true;
	bool accidentalsQ    = true;
	bool referencesQ     = true;
	bool terminalsQ      = true;
	bool breaksQ         = true;
	bool transpositionsQ = true;
   bool tieQ            = true;
   bool teditQ          = true;
   bool instrumentQ     = true;
   bool removekeydesigQ = true;
   bool fixbarlinesQ    = true;
   bool parenthesesQ    = true;

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

	if (getBoolean("no-ties")) { tieQ = false; }

	if (getBoolean("only-remove-empty-transpositions")) {
		abbreviationsQ  = false;
		accidentalsQ    = false;
		referencesQ     = false;
		terminalsQ      = false;
		breaksQ         = false;
		transpositionsQ = true;
	}

	if (articulationsQ)  { removeArticulations(infile); }
	if (fixbarlinesQ)    { fixBarlines(infile); }
	if (tieQ)            { fixTies(infile); }
	if (abbreviationsQ)  { fixInstrumentAbbreviations(infile); }
	if (accidentalsQ)    { fixEditorialAccidentals(infile); }
	if (parenthesesQ)    { createJEditorialAccidentals(infile); }
	if (referencesQ)     { addBibliographicRecords(infile); }
	if (breaksQ)         { deleteBreaks(infile); }
	if (terminalsQ)      { addTerminalLongs(infile); }
	if (transpositionsQ) { deleteDummyTranspositions(infile); }
	if (mensurationQ)    { addMensurations(infile); }
	if (teditQ)          { createEditText(infile); }
   if (instrumentQ)     { adjustIntrumentNames(infile); }
   if (removekeydesigQ) { removeKeyDesignations(infile); }

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
// Tool_gasparize::removeArticulations --
//

void Tool_gasparize::removeArticulations(HumdrumFile& infile) {
	HumRegex hre;
	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isData()) {
			continue;
		}
		for (int j=0; j<infile[i].getFieldCount(); j++) {
			HTp token = infile.token(i, j);
			if (!token->isKern()) {
				continue;
			}
			if (token->isNull()) {
				continue;
			}
			bool changed = false;
			string text = token->getText();
			if (text.find("'") != string::npos) {
				// remove staccatos
				changed = true;
				hre.replaceDestructive(text, "", "'", "g");
			}
			if (text.find("~") != string::npos) {
				// remove tenutos
				changed = true;
				hre.replaceDestructive(text, "", "~", "g");
			}
			if (changed) {
				token->setText(text);
			}
		}
	}
}



//////////////////////////////
//
// Tool_gasparize::adjustSystemDecoration --
//    !!!system-decoration: [(s1)(s2)(s3)(s4)]
// to:
//    !!!system-decoration: [*]
//

void Tool_gasparize::adjustSystemDecoration(HumdrumFile& infile) {
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
// Tool_gasparize::deleteDummyTranspositions -- Somehow empty
//    transpositions that go to the same pitch can appear in the
//    MusicXML data, so remove them here.  Example:
// 		*Trd0c0
//

void Tool_gasparize::deleteDummyTranspositions(HumdrumFile& infile) {
	vector<int> ldel;
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
// Tool_gasparize::fixEditorialAccidentals -- checkDataLine() does
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

void Tool_gasparize::fixEditorialAccidentals(HumdrumFile& infile) {
	removeDoubledAccidentals(infile);

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
// Tool_gasparize::removeDoubledAccidentals -- Often caused by transposition
//    differences between parts in the MusicXML export from Finale.  Also some
//    strange double sharps appear randomly.
//

void Tool_gasparize::removeDoubledAccidentals(HumdrumFile& infile) {
	HumRegex hre;
	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isData()) {
			continue;
		}
		for (int j=0; j<infile[i].getFieldCount(); j++) {
			HTp token = infile.token(i, j);
			if (!token->isKern()) {
				continue;
			}
			if (token->isNull()) {
				continue;
			}
			if (token->isRest()) {
				continue;
			}
			if (token->find("--") != string::npos) {
				string text = *token;
				hre.replaceDestructive(text, "-", "--", "g");
			} else if (token->find("--") != string::npos) {
				string text = *token;
				hre.replaceDestructive(text, "#", "##", "g");
			}
		}
	}
}



//////////////////////////////
//
// Tool_gasparize::addTerminalLongs -- Convert all last notes to terminal longs
//    Also probably add terminal longs before double barlines as in JRP.
//

void Tool_gasparize::addTerminalLongs(HumdrumFile& infile) {
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
			if (cur->find("l") != string::npos) {
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
// Tool_gasparize::fixInstrumentAbbreviations --
//

void Tool_gasparize::fixInstrumentAbbreviations(HumdrumFile& infile) {
	int iline = -1;
	int aline = -1;

	vector<HTp> kerns = infile.getKernSpineStartList();
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
// Tool_gasparize::convertBreaks --
//

void Tool_gasparize::convertBreaks(HumdrumFile& infile) {
	HumRegex hre;
	for (int i=infile.getLineCount()-1; i>= 0; i--) {
		if (!infile[i].isGlobalComment()) {
			continue;
		}
		if (hre.search(*infile.token(i, 0), "linebreak\\s*:\\s*original")) {
			string text = "!!LO:LB:g=original";
			infile[i].setText(text);
		}
		else if (hre.search(*infile.token(i, 0), "pagebreak\\s*:\\s*original")) {
			string text = "!!LO:PB:g=original";
			infile[i].setText(text);
		}
	}
}



//////////////////////////////
//
// Tool_gasparize::deleteBreaks --
//

void Tool_gasparize::deleteBreaks(HumdrumFile& infile) {
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
// Tool_gasparize::addBibliographicRecords --
//
// !!!COM:
// !!!CDT:
// !!!OTL:
// !!!AGN:
// !!!SCT:
// !!!SCA:
// !!!voices:
//
// At end:
// !!!RDF**kern: l = terminal long
// !!!RDF**kern: i = editorial accidental
// !!!EED:
// !!!EEV: $DATE
//

void Tool_gasparize::addBibliographicRecords(HumdrumFile& infile) {
	vector<HLp> refinfo = infile.getReferenceRecords();
	map<string, HLp> refs;
	for (int i=0; i<(int)refinfo.size(); i++) {
		string key = refinfo[i]->getReferenceKey();
		refs[key] = refinfo[i];
	}

	// header records
	if (refs.find("voices") == refs.end()) {
		if (infile.token(0, 0)->find("!!!OTL") != string::npos) {
			infile.insertLine(1, "!!!voices:");
		} else {
			infile.insertLine(0, "!!!voices:");
		}
	}
	if (refs.find("SCA") == refs.end()) {
		if (infile.token(0, 0)->find("!!!OTL") != string::npos) {
			infile.insertLine(1, "!!!SCA:");
		} else {
			infile.insertLine(0, "!!!SCA:");
		}
	}
	if (refs.find("SCT") == refs.end()) {
		if (infile.token(0, 0)->find("!!!OTL") != string::npos) {
			infile.insertLine(1, "!!!SCT:");
		} else {
			infile.insertLine(0, "!!!SCT:");
		}
	}
	if (refs.find("AGN") == refs.end()) {
		if (infile.token(0, 0)->find("!!!OTL") != string::npos) {
			infile.insertLine(1, "!!!AGN:");
		} else {
			infile.insertLine(0, "!!!AGN:");
		}
	}

	if (refs.find("OTL") == refs.end()) {
		infile.insertLine(0, "!!!OTL:");
	}
	if (refs.find("CDT") == refs.end()) {
		infile.insertLine(0, "!!!CDT: ~1450-~1517");
	}
	if (refs.find("COM") == refs.end()) {
		infile.insertLine(0, "!!!COM: Gaspar van Weerbeke");
	}

	// trailer records
	bool foundi = false;
	bool foundj = false;
	bool foundl = false;
	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isReference()) {
			continue;
		}
		HTp token = infile.token(i, 0);
		if (token->find("!!!RDF**kern:") == string::npos) {
			continue;
		}
		if (token->find("terminal breve") != string::npos) {
			foundl = true;
		} else if (token->find("editorial accidental") != string::npos) {
			if (token->find("i =") != string::npos) {
				foundi = true;
			} else if (token->find("j =") != string::npos) {
				foundj = true;
			}
		}
	}
	if (!foundj) {
		infile.appendLine("!!!RDF**kern: j = editorial accidental, optional, paren up");
	}
	if (!foundi) {
		infile.appendLine("!!!RDF**kern: i = editorial accidental");
	}
	if (!foundl) {
		infile.appendLine("!!!RDF**kern: l = terminal long");
	}

	if (refs.find("PTL") == refs.end()) {
		infile.appendLine("!!!PTL: Gaspar van Weerbeke: Collected Works. V. Settings of Liturgical Texts, Songs, and Instrumental Works");
	}
	if (refs.find("PPR") == refs.end()) {
		infile.appendLine("!!!PPR: American Institute of Musicology");
	}
	if (refs.find("PC#") == refs.end()) {
		infile.appendLine("!!!PC#: Corpus Mensurabilis Musicae 106/V");
	}
	if (refs.find("PDT") == refs.end()) {
		infile.appendLine("!!!PDT: {YEAR}");
	}
	if (refs.find("PED") == refs.end()) {
		infile.appendLine("!!!PED: Kolb, Paul");
		infile.appendLine("!!!PED: Pavanello, Agnese");
	}
	if (refs.find("YEC") == refs.end()) {
		infile.appendLine("!!!YEC: Copyright {YEAR}, Kolb, Paul");
		infile.appendLine("!!!YEC: Copyright {YEAR}, Pavanello, Agnese");
	}
	if (refs.find("YEM") == refs.end()) {
		infile.appendLine("!!!YEM: CC-BY-SA 4.0 (https://creativecommons.org/licenses/by-nc/4.0/legalcode)");
	}
	if (refs.find("EED") == refs.end()) {
		infile.appendLine("!!!EED: Zybina, Karina");
		infile.appendLine("!!!EED: Mair-Gruber, Roland");
	}
	if (refs.find("EEV") == refs.end()) {
		string date = getDate();
		string line = "!!!EEV: " + date;
		infile.appendLine(line);
	}
}



////////////////////////////////
//
// Tool_gasparize::checkDataLine --
//

void Tool_gasparize::checkDataLine(HumdrumFile& infile, int lineindex) {
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
		if (token->find('j') != string::npos) {
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
			if (token->find("#") != string::npos) {
				hasaccid = true;
			} else if (token->find("-") != string::npos) {
				hasaccid = true;
			}
			int hashide = false;
			if (token->find("-y") != string::npos) {
				hashide = true;
			}
			else if (token->find("#y") != string::npos) {
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
		HumRegex hre;
		hre.replaceDestructive(text, "#", "##+", "g");
		hre.replaceDestructive(text, "-", "--+", "g");
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
// Tool_gasparize::updateKeySignatures -- Fill in the accidental
//    states for each diatonic pitch.
//

void Tool_gasparize::updateKeySignatures(HumdrumFile& infile, int lineindex) {
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
// Tool_gasparize::clearStates --
//

void Tool_gasparize::clearStates(void) {
	for (int i=0; i<(int)m_pstates.size(); i++) {
		fill(m_pstates[i].begin(), m_pstates[i].end(), 0);
	}
	for (int i=0; i<(int)m_estates.size(); i++) {
		fill(m_estates[i].begin(), m_estates[i].end(), false);
	}
}


//////////////////////////////
//
// Tool_gasparize::getDate --
//

string Tool_gasparize::getDate(void) {
	time_t t = time(NULL);
	tm* timeptr = localtime(&t);
	stringstream ss;
	int year = timeptr->tm_year + 1900;
	int month = timeptr->tm_mon + 1;
	int day = timeptr->tm_mday;
	ss << year << "/";
	if (month < 10) {
		ss << "0";
	}
	ss << month << "/";
	if (day < 10) {
		ss << "0";
	}
	ss << day;
	return ss.str();
}



//////////////////////////////
//
// Tool_gasparize::fixTies --
//    If a tie is unclosed or if a note is followed by an invisible rest, then fix.
//

void Tool_gasparize::fixTies(HumdrumFile& infile) {
	int strands = infile.getStrandCount();
	for (int i=0; i<strands; i++) {
		HTp sstart = infile.getStrandStart(i);
		if (!sstart) {
			continue;
		}
		if (!sstart->isKern()) {
			continue;
		}
		HTp send   = infile.getStrandEnd(i);
		fixTiesForStrand(sstart, send);
	}
	fixTieStartEnd(infile);
}



void Tool_gasparize::fixTieStartEnd(HumdrumFile& infile) {
	int strands = infile.getStrandCount();
	for (int i=0; i<strands; i++) {
		HTp sstart = infile.getStrandStart(i);
		if (!sstart) {
			continue;
		}
		if (!sstart->isKern()) {
			continue;
		}
		HTp send   = infile.getStrandEnd(i);
		fixTiesStartEnd(sstart, send);
	}
}



void Tool_gasparize::fixTiesStartEnd(HTp starts, HTp ends) {
	HTp current = starts;
	HumRegex hre;
	while (current) {
		if (!current->isData()) {
			current = current->getNextToken();
			continue;
		}
		if ((current->find('[') != string::npos) &&
				(current->find(']') != string::npos) &&
				(current->find(' ') == string::npos)) {
			string text = *current;
			hre.replaceDestructive(text, "", "\\[", "g");
			hre.replaceDestructive(text, "_", "\\]", "g");
			current->setText(text);
		}
		current = current->getNextToken();
	}
}


//////////////////////////////
//
// Tool_gasparize::fixTiesForStrand --
//

void Tool_gasparize::fixTiesForStrand(HTp sstart, HTp send) {
	if (!sstart) {
		return;
	}
	HTp current = sstart;
	HTp last = NULL;
	current = current->getNextToken();
	while (current) {
		if (!current->isData()) {
			current = current->getNextToken();
			continue;
		}
		if (current->isNull()) {
			current = current->getNextToken();
			continue;
		}
		if (last == NULL) {
			last = current;
			current = current->getNextToken();
			continue;
		}
		if (current->find("yy") != string::npos) {
			fixTieToInvisibleRest(last, current);
		} else if (((last->find("[") != string::npos) || (last->find("_") != string::npos))
				&& ((current->find("]") == string::npos) && (current->find("_") == string::npos))) {
			fixHangingTie(last, current);
		}
		last = current;
		current = current->getNextToken();
	}
}



//////////////////////////////
//
// Tool_gasparize::fixTieToInvisibleRest --
//

void Tool_gasparize::fixTieToInvisibleRest(HTp first, HTp second) {
	if (second->find("yy") == string::npos) {
		return;
	}
	if ((first->find("[") == string::npos) && (first->find("_") == string::npos)) {
		string ftext = *first;
		ftext = "[" + ftext;
		first->setText(ftext);
	}
	HumRegex hre;
	if (!hre.search(first, "([A-Ga-g#n-]+)")) {
		return;
	}
	string pitch = hre.getMatch(1);
	pitch += "]";
	string text = *second;
	hre.replaceDestructive(text, pitch, "ryy");
	second->setText(text);
}



//////////////////////////////
//
// Tool_gasparize::fixHangingTie -- Not dealing with chain of missing ties.
//

void Tool_gasparize::fixHangingTie(HTp first, HTp second) {
	string text = *second;
	text += "]";
	second->setText(text);
}



//////////////////////////////
//
// Tool_gasparize::addMensurations -- Add mensurations.
//

void Tool_gasparize::addMensurations(HumdrumFile& infile) {
	HumRegex hre;
	for (int i=infile.getLineCount() - 1; i>=0; i--) {
		if (!infile[i].isInterpretation()) {
			continue;
		}
		for (int j=0; j<infile[i].getFieldCount(); j++) {
			HTp token = infile.token(i, 0);
			if (hre.search(token, "^\\*M(\\d+)/(\\d+)")) {
				int value = hre.getMatchInt(1);
				addMensuration(value, infile, i);
			}
		}
	}
}



//////////////////////////////
//
// Tool_gasparize::addMensuration --
//

void Tool_gasparize::addMensuration(int top, HumdrumFile& infile, int index) {
	HTp checktoken = infile[index+1].token(0);
	if (!checktoken) {
		return;
	}
	if (checktoken->find("met") != string::npos) {
		return;
	}
	int fieldcount = infile[index].getFieldCount();
	string line = "*";
	HTp token = infile[index].token(0);
	if (token->isKern()) {
		if (top == 2) {
			line += "met(C|)";
		} else {
			line += "met(O)";
		}
	}
	for (int i=1; i<fieldcount; i++) {
		line += "\t*";
		HTp token = infile[index].token(i);
		if (token->isKern()) {
			if (top == 2) {
				line += "met(C|)";
			} else {
				line += "met(O)";
			}
		}
	}
	infile.insertLine(index+1, line);
}


///////////////////////////////
//
// Tool_gasparize::createEditText -- Convert <i> markers into *edit interps.
//

void Tool_gasparize::createEditText(HumdrumFile& infile) {
	// previous process manipulated the structure so reanalyze here for now:
	infile.analyzeBaseFromTokens();
	infile.analyzeStructureNoRhythm();

	int strands = infile.getStrandCount();
	for (int i=0; i<strands; i++) {
		HTp sstart = infile.getStrandStart(i);
		if (!sstart) {
			continue;
		}
		if (!sstart->isDataType("**text")) {
			continue;
		}
		HTp send   = infile.getStrandEnd(i);
		bool status = addEditStylingForText(infile, sstart, send);
		if (status) {
			infile.analyzeBaseFromTokens();
			infile.analyzeStructureNoRhythm();
		}
	}
}


//////////////////////////////
//
// Tool_gasparize::addEditStylingForText --
//

bool Tool_gasparize::addEditStylingForText(HumdrumFile& infile, HTp sstart, HTp send) {
	HTp current = send->getPreviousToken();
	bool output = false;
	string state = "";
	string laststate = "";
	HumRegex hre;
	HTp lastdata = NULL;
	bool italicQ = false;
	while (current && (current != sstart)) {
		if (!current->isData()) {
			current = current->getPreviousToken();
			continue;
		}
		if (current->isNull()) {
			current = current->getPreviousToken();
			continue;
		}
		italicQ = false;
		string text = current->getText();
		if (text.find("<i>") != string::npos) {
			italicQ = true;
			hre.replaceDestructive(text, "", "<i>", "g");
			hre.replaceDestructive(text, "", "</i>", "g");
			current->setText(text);
		} else {
}
		if (laststate == "") {
			if (italicQ) {
				laststate = "italic";
			} else {
				laststate = "regular";
			}
			current = current->getPreviousToken();
			continue;
		} else {
			if (italicQ) {
				state = "italic";
			} else {
				state = "regular";
			}
		}
		if (state != laststate) {
			if (lastdata && (laststate == "italic")) {
				output = true;
				if (!insertEditText("*edit", infile, lastdata->getLineIndex() - 1, lastdata->getFieldIndex())) {
					string line = getEditLine("*edit", lastdata->getFieldIndex(), lastdata->getOwner());
					infile.insertLine(lastdata->getLineIndex(), line);
				}
			} else if (lastdata && (laststate == "regular")) {
				output = true;
				if (!insertEditText("*Xedit", infile, lastdata->getLineIndex() - 1, lastdata->getFieldIndex())) {
					string line = getEditLine("*Xedit", lastdata->getFieldIndex(), lastdata->getOwner());
					infile.insertLine(lastdata->getLineIndex(), line);
				}
			}
		}
		laststate = state;
		lastdata = current;
		current = current->getPreviousToken();
	}

	if (lastdata && italicQ) {
		// add *edit before first syllable in **text.
		output = true;
		if (!insertEditText("*edit", infile, lastdata->getLineIndex() - 1, lastdata->getFieldIndex())) {
			string line = getEditLine("*edit", lastdata->getFieldIndex(), lastdata->getOwner());
			infile.insertLine(lastdata->getLineIndex(), line);
		}
	}

	return output;
}



//////////////////////////////
//
// Tool_gasparize::insertEditText --
//

bool Tool_gasparize::insertEditText(const string& text, HumdrumFile& infile, int line, int field) {
	if (!infile[line].isInterpretation()) {
		return false;
	}
	HTp token;
	for (int i=0; i<infile[line].getFieldCount(); i++) {
		token = infile.token(line, i);
		if (token->isNull()) {
			continue;
		}
		if (token->find("edit") != string::npos) {
			break;
		}
		return false;
	}
	token = infile.token(line, field);
	token->setText(text);

	return true;
}



/////////////////////
//
// Tool_gasparize::getEditLine --
//

string Tool_gasparize::getEditLine(const string& text, int fieldindex, HLp line) {
	string output;
	for (int i=0; i<fieldindex; i++) {
		output += "*";
		if (i < line->getFieldCount()) {
			output += "\t";
		}
	}
	output += text;
	if (fieldindex < line->getFieldCount()) {
		output += "\t";
	}
	for (int i=fieldindex+1; i<line->getFieldCount(); i++) {
		output += "*";
		if (i < line->getFieldCount()) {
			output += "\t";
		}
	}
	return output;
}



//////////////////////////////
//
// adjustIntrumentNames --
//

void Tool_gasparize::adjustIntrumentNames(HumdrumFile& infile) {
	int instrumentLine = -1;
	int abbrLine = -1;
	for (int i=0; i<infile.getLineCount(); i++) {
		if (infile[i].isData()) {
			break;
		}
		if (!infile[i].isInterpretation()) {
			continue;
		}
		for (int j=0; j<infile[i].getFieldCount(); j++) {
			HTp token = infile.token(i, j);
			if (token->compare(0, 3, "*I\"") == 0) {
				instrumentLine = i;
			}
			if (token->compare(0, 3, "*I'") == 0) {
				abbrLine = i;
			}
		}
	}
	if (instrumentLine < 0) {
		return;
	}
	for (int i=0; i<infile[instrumentLine].getFieldCount(); i++) {
		HTp token = infile.token(instrumentLine, i);
		if (*token == "*I\"CT I") {
			token->setText("*I\"Contratenor 1");
		} else if (*token == "*I\"CTI") {
			token->setText("*I\"Contratenor 1");
		} else if (*token == "*I\"CTII") {
			token->setText("*I\"Contratenor 2");
		} else if (*token == "*I\"CT II") {
			token->setText("*I\"Contratenor 2");
		} else if (*token == "*I\"CT") {
			token->setText("*I\"Contratenor");
		} else if (*token == "*I\"S") {
			token->setText("*I\"Superius");
		} else if (*token == "*I\"A") {
			token->setText("*I\"Altus");
		} else if (*token == "*I\"T") {
			token->setText("*I\"Tenor");
		} else if (*token == "*I\"B") {
			token->setText("*I\"Bassus");
		} else if (*token == "*I\"V") {
			token->setText("*I\"Quintus");
		} else if (*token == "*I\"VI") {
			token->setText("*I\"Sextus");
		}
	}
	if (abbrLine >= 0) {
		return;
	}
	string abbr;
	HumRegex hre;
	for (int i=0; i<infile[instrumentLine].getFieldCount(); i++) {
		HTp token = infile.token(instrumentLine, i);
		string text = *token;
		if (text == "*I\"Quintus") {
			abbr += "*I'V";
		} else if (text == "*I\"Contratenor") {
			abbr += "*I'Ct";
		} else if (text == "*I\"Sextus") {
			abbr += "*I'VI";
		} else if (text == "*I\"Contratenor 1") {
			abbr += "*I'Ct1";
		} else if (text == "*I\"Contratenor 2") {
			abbr += "*I'Ct2";
		} else if (hre.search(text, "^\\*I\"([A-Z])")) {
			abbr += "*I'";
			abbr += hre.getMatch(1);
		} else {
			abbr += "*";
		}
		if (i < infile[instrumentLine].getFieldCount() - 1) {
			abbr += "\t";
		}
	}
	infile.insertLine(instrumentLine+1, abbr);
	infile.analyzeBaseFromTokens();
	infile.analyzeStructureNoRhythm();
}


//////////////////////////////
//
// Tool_gaspar::removeKeyDesignations --
//

void Tool_gasparize::removeKeyDesignations(HumdrumFile& infile) {
	HumRegex hre;
	for (int i=infile.getLineCount() - 1; i>=0; i--) {
		if (!infile[i].isInterpretation()) {
			continue;
		}
		for (int j=0; j<infile[i].getFieldCount(); j++) {
			HTp token = infile.token(i, j);
			if (*token == "*") {
				continue;
			}
			if (!token->isKern()) {
				continue;
			}
			if (hre.search(token, "^\\*[A-Ga-g][#n-]*:$")) {
				// suppress the key desingation
				infile.deleteLine(i);
				break;
			}
		}
	}

}


//////////////////////////////
//
// Tool_gasparize::fixBarlines -- Add final double barline and convert
//    any intermediate final barlines to double barlines.
//

void Tool_gasparize::fixBarlines(HumdrumFile& infile) {
	fixFinalBarline(infile);
	HumRegex hre;

	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isBarline()) {
			continue;
		}
		if (infile[i].getDurationToEnd() == 0) {
			break;
		}
		for (int j=0; j<infile[i].getFieldCount(); j++) {
			HTp token = infile.token(i, j);
			if (token->find("==") == string::npos) {
				continue;
			}
			if (hre.search(token, "^==(\\d*)")) {
				string text = "=";
				text += hre.getMatch(1);
				text += "||";
				token->setText(text);
			}
		}
	}
}



//////////////////////////////
//
// Tool_gasparize::fixFinalBarline --
//

void Tool_gasparize::fixFinalBarline(HumdrumFile& infile) {
	for (int i=infile.getLineCount() - 1; i>=0; i--) {
		if (infile[i].isData()) {
			break;
		}
		if (!infile[i].isBarline()) {
			continue;
		}
		for (int j=0; j<infile[i].getFieldCount(); j++) {
			HTp token = infile.token(i, j);
			if (*token != "==") {
				token->setText("==");
			}
		}
	}
}



//////////////////////////////
//
// Tool_gasparize::createJEditorialAccidentals --
// convert
// 	!LO:TX:a:t=(    )
// 	4F#
//

void Tool_gasparize::createJEditorialAccidentals(HumdrumFile& infile) {
	int strands = infile.getStrandCount();
	for (int i=0; i<strands; i++) {
		HTp sstart = infile.getStrandStart(i);
		if (!sstart) {
			continue;
		}
		if (!sstart->isKern()) {
			continue;
		}
		HTp send   = infile.getStrandEnd(i);
		createJEditorialAccidentals(sstart, send);
	}
}

void Tool_gasparize::createJEditorialAccidentals(HTp sstart, HTp send) {
	HTp current = sstart->getNextToken();
	HumRegex hre;
	while (current && (current != send)) {
		if (!current->isCommentLocal()) {
			current = current->getNextToken();
			continue;
		}
		if (hre.search(current, "^!LO:TX:a:t=\\(\\s*\\)$")) {
			current->setText("!");
			convertNextNoteToJAccidental(current);
		}
		current = current->getNextToken();
	}
}

void Tool_gasparize::convertNextNoteToJAccidental(HTp current) {
	current = current->getNextToken();
	HumRegex hre;
	while (current) {
		if (!current->isData()) {
			// Does not handle LO for non-data.
			current = current->getNextToken();
			continue;
		}
		if (current->isNull()) {
			break;
		}
		if (current->isRest()) {
			break;
		}
		string text = *current;
		if (hre.search(text, "i")) {
			hre.replaceDestructive(text, "j", "i");
			current->setText(text);
			break;
		} else if (hre.search(text, "[-#n]")) {
			hre.replaceDestructive(text, "$1j", "(.*[-#n]+)");
			current->setText(text);
			break;
		} else {
			// Need to add a natural sign as well.
			hre.replaceDestructive(text, "$1nj", "(.*[A-Ga-g]+)");
			current->setText(text);
			break;
		}
		break;
	}
	current = current->getNextToken();
}



// END_MERGE

} // end namespace hum



