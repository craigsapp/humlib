//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Tue May 19 22:12:33 PDT 2020
// Last Modified: Tue May 19 22:12:36 PDT 2020
// Filename:      tool-chantize.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/tool-chantize.cpp
// Syntax:        C++11; humlib
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Convert raw MusicXML entry of chant notation into
//                more direct chant representation.
//

#include "tool-chantize.h"

#include "Convert.h"
#include "HumRegex.h"
#include "tool-shed.h"

#include <algorithm>

using namespace std;

namespace hum {

// START_MERGE


/////////////////////////////////
//
// Tool_chantize::Tool_chantize -- Set the recognized options for the tool.
//

Tool_chantize::Tool_chantize(void) {
	define("R|no-reference-records=b", "Do not add reference records");
	define("r|only-add-reference-records=b", "Only add reference records");

	define("B|do-not-delete-breaks=b", "Do not delete system/page break markers");
	define("b|only-delete-breaks=b", "only delete breaks");

	define("A|do-not-fix-instrument-abbreviations=b", "Do not fix instrument abbreviations");
	define("a|only-fix-instrument-abbreviations=b", "Only fix instrument abbreviations");

	define("E|do-not-fix-editorial-accidentals=b", "Do not fix instrument abbreviations");
	define("e|only-fix-editorial-accidentals=b", "Only fix editorial accidentals");

	define("N|do-not-remove-empty-transpositions=b", "Do not remove empty transposition instructions");
	define ("n|only-remove-empty-transpositions=b", "Only remove empty transpositions");
}



/////////////////////////////////
//
// Tool_chantize::run -- Primary interfaces to the tool.
//

bool Tool_chantize::run(HumdrumFileSet& infiles) {
	bool status = true;
	for (int i=0; i<infiles.getCount(); i++) {
		status &= run(infiles[i]);
	}
	return status;
}


bool Tool_chantize::run(const string& indata, ostream& out) {
	HumdrumFile infile(indata);
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_chantize::run(HumdrumFile& infile, ostream& out) {
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

bool Tool_chantize::run(HumdrumFile& infile) {
	processFile(infile);

	// Re-load the text for each line from their tokens.
	infile.createLinesFromTokens();
	outputFile(infile);
	return true;
}



//////////////////////////////
//
// Tool_chantize::outputFile --
//    * remove time signature,
//    * remove barlines, except double barlines
//    * convert rests to double barlines (except at the end of the
//         music).
//

void Tool_chantize::outputFile(HumdrumFile& infile) {
	vector<bool> terminalRest = getTerminalRestStates(infile);
	HTp token;
	bool restQ = false;
	for (int i=0; i<infile.getLineCount(); i++) {
		if (infile[i].isInterpretation()) {
			token = infile.token(i, 0);
			if (token->isTimeSignature()) {
				// suppress time signatures
				continue;
			}
		} else if (infile[i].isCommentGlobal()) {
			// Convert line breaks into invisible barlines.
			token = infile.token(i, 0);
			if (token->find("original") != string::npos) {
				int cols = 2;
				for (int j=0; j<cols; j++) {
					m_humdrum_text << "=-";
					if (j < cols - 1) {
						m_humdrum_text << "\t";
					}
				}
				m_humdrum_text << "\n";
				continue;
			}
		} else if (infile[i].isBarline()) {
			// suppress barlines
			token = infile.token(i, 0);
			// but do not suppress double barlines
			if (token->find("||") == string::npos) {
				continue;
			}
		} else if (infile[i].isData()) {
			token = infile.token(i, 0);
			if (token->isRest()) {
				if (terminalRest[i]) {
					continue;
				}
				if (!restQ) {
					restQ = true;
					// convert rest into double barline
					// but suppress rests at end of music
					//or just suppress
					// for (int j=0; j<infile[i].getFieldCount(); j++) {
					// 	m_humdrum_text  << "=-";
					// 	if (j < infile[i].getFieldCount() - 1) {
					// 		m_humdrum_text << "\t";
					// 	}
					// }
					// m_humdrum_text << "\n";
				}
				continue;
			} else {
				restQ = false;
			}
		}
		m_humdrum_text << infile[i] << "\n";
	}
	if (m_diamondQ) {
		m_humdrum_text << "!!!RDF**kern: j = diamond note, color=blue\n";
	}
	m_humdrum_text << "!!!RDF**kern: {} = ligature\n";
}



//////////////////////////////
//
// Tool_chantize::processFile --
//

void Tool_chantize::processFile(HumdrumFile& infile) {

	bool abbreviationsQ  = true;
	bool accidentalsQ    = true;
	bool referencesQ     = true;
	// bool breaksQ         = true;
	bool transpositionsQ = true;

	if (getBoolean("no-reference-records")) { referencesQ = false; }
	if (getBoolean("only-add-reference-records")) {
		abbreviationsQ  = false;
		accidentalsQ    = false;
		referencesQ     = true;
		// breaksQ         = false;
		transpositionsQ = false;
	}

	if (getBoolean("do-not-delete-breaks")) {
		//	breaksQ = false;
	}
	if (getBoolean("only-delete-breaks")) {
		abbreviationsQ  = false;
		accidentalsQ    = false;
		referencesQ     = false;
		// breaksQ         = true;
		transpositionsQ = false;
	}

	if (getBoolean("do-not-fix-instrument-abbreviations")) { abbreviationsQ = false; }
	if (getBoolean("only-fix-instrument-abbreviations")) {
		abbreviationsQ  = true;
		accidentalsQ    = false;
		referencesQ     = false;
		// breaksQ         = false;
		transpositionsQ = false;
	}

	if (getBoolean("do-not-fix-editorial-accidentals")) { accidentalsQ = false; }
	if (getBoolean("only-fix-editorial-accidentals")) {
		abbreviationsQ  = false;
		accidentalsQ    = true;
		referencesQ     = false;
		// breaksQ         = false;
		transpositionsQ = false;
	}

	if (getBoolean("do-not-remove-empty-transpositions")) { transpositionsQ = false; }
	if (getBoolean("only-remove-empty-transpositions")) {
		abbreviationsQ  = false;
		accidentalsQ    = false;
		referencesQ     = false;
		// breaksQ         = false;
		transpositionsQ = true;
	}

	m_diamondQ = hasDiamondNotes(infile);

	if (abbreviationsQ)  { fixInstrumentAbbreviations(infile); }
	if (accidentalsQ)    { fixEditorialAccidentals(infile); }
	if (referencesQ)     { addBibliographicRecords(infile); }
//	if (breaksQ)         { deleteBreaks(infile); }
	if (transpositionsQ) { deleteDummyTranspositions(infile); }

	// Remove rhythms from notes
	Tool_shed shed;
	vector<string> argv;
	argv.push_back("shed");
	argv.push_back("-x");     // only apply to **text spines
	argv.push_back("kern");
	argv.push_back("-e");
	// convert 8th notes into diamond notes
	// remove rhythms and beams
	argv.push_back("s/8/j/g; s/[0-9LJ]//g; s/\\(/{/g; s/\\)/}/g");
	shed.process(argv);
	shed.run(infile);
}



//////////////////////////////
//
// Tool_chantize::deleteDummyTranspositions -- Somehow empty
//    transpositions that go to the same pitch can appear in the
//    MusicXML data, so remove them here.  Example:
// 		*Trd0c0
//

void Tool_chantize::deleteDummyTranspositions(HumdrumFile& infile) {
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
// Tool_chantize::fixEditorialAccidentals -- checkDataLine() does
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

void Tool_chantize::fixEditorialAccidentals(HumdrumFile& infile) {
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
// Tool_chantize::fixInstrumentAbbreviations --
//

void Tool_chantize::fixInstrumentAbbreviations(HumdrumFile& infile) {
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
// Tool_chantize::deleteBreaks --
//

void Tool_chantize::deleteBreaks(HumdrumFile& infile) {
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
// Tool_chantize::addBibliographicRecords --
//
// !!!folio:
// !!!system:
// !!!SMS:
//
// At end:
// !!!SMS-url:
// !!!AGN: Chant
// !!!ENC: [Encoder's name]
// !!!END: [Encoding date]
// !!!EEV: $DATE
//

void Tool_chantize::addBibliographicRecords(HumdrumFile& infile) {
	std::vector<HLp> refinfo = infile.getReferenceRecords();
	std::map<string, HLp> refs;
	for (int i=0; i<(int)refinfo.size(); i++) {
		string key = refinfo[i]->getReferenceKey();
		refs[key] = refinfo[i];
	}

	// header records

	if (refs.find("system") == refs.end()) {
		infile.insertLine(0, "!!!system:");
	}

	if (refs.find("folio") == refs.end()) {
		infile.insertLine(0, "!!!folio:");
	}

	if (refs.find("SMS") == refs.end()) {
		infile.insertLine(0, "!!!SMS:");
	}

	if (refs.find("OTL") == refs.end()) {
		if (infile.token(0, 0)->find("!!!OTL") != std::string::npos) {
			// do nothing
		} else {
			infile.insertLine(0, "!!!OTL:");
		}
	}

	// trailer records

	if (refs.find("SMS-url") == refs.end()) {
		infile.appendLine("!!!SMS-url:");
	}

	if (refs.find("AGN") == refs.end()) {
		infile.appendLine("!!!AGN: Chant");
	}

	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isReference()) {
			continue;
		}
		if (infile.token(i, 0)->find("!!!RDF**kern:") == std::string::npos) {
			continue;
		}
	}

	if (refs.find("ENC") == refs.end()) {
		infile.appendLine("!!!ENC: [Encoder's name]");
	}
	if (refs.find("END") == refs.end()) {
		string date = getDate();
		string line = "!!!EED: " + date;
		infile.appendLine(line);
	}
	if (refs.find("EEV") == refs.end()) {
		string date = getDate();
		string line = "!!!EEV: " + date;
		infile.appendLine(line);
	}

}



////////////////////////////////
//
// Tool_chantize::checkDataLine --
//

void Tool_chantize::checkDataLine(HumdrumFile& infile, int lineindex) {
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
// Tool_chantize::updateKeySignatures -- Fill in the accidental
//    states for each diatonic pitch.
//

void Tool_chantize::updateKeySignatures(HumdrumFile& infile, int lineindex) {
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
// Tool_chantize::clearStates --
//

void Tool_chantize::clearStates(void) {
	for (int i=0; i<(int)m_pstates.size(); i++) {
		fill(m_pstates[i].begin(), m_pstates[i].end(), 0);
	}
	for (int i=0; i<(int)m_estates.size(); i++) {
		fill(m_estates[i].begin(), m_estates[i].end(), false);
	}
}


//////////////////////////////
//
// Tool_chantize::getDate --
//

string Tool_chantize::getDate(void) {
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
// Tool_chantize::getTerminalRestStates -- return a vector of each line,
//    setting true if the line is a data line containing a rest at the end of the
//    music after the last note.
//

vector<bool> Tool_chantize::getTerminalRestStates(HumdrumFile& infile) {
	vector<bool> output(infile.getLineCount(), false);

	for (int i=infile.getLineCount() - 1; i>=0; i--) {
		if (!infile[i].isData()) {
			continue;
		}
		HTp token = infile.token(i, 0);
		if (token->isRest()) {
			output[i] = true;
		} else {
			break;
		}
	}

	return output;

}



//////////////////////////////
//
// Tool_chantize::hasDiamondNotes -- True if the duration of any line is less than a quarter note.
//

bool Tool_chantize::hasDiamondNotes(HumdrumFile& infile) {
	for (int i=0; i<infile.getLineCount(); i++) {
		HumNum duration = infile[i].getDuration();
		if (duration < 1) {
			return true;
		}
	}
	return false;
}


// END_MERGE

} // end namespace hum



