//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Aug 12 10:58:43 PDT 2024
// Last Modified: Sun Aug 18 00:19:45 PDT 2024
// Filename:      cli/tandeminfo.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/cli/tandeminfo.cpp
// Syntax:        C++11
// vim:           ts=3 noexpandtab nowrap
//
// Description:   Analyze tandem interpretations.
//

#include "tool-tandeminfo.h"
#include "Convert.h"
#include "HumRegex.h"
#include "HumInstrument.h"

using namespace std;

namespace hum {

// START_MERGE


/////////////////////////////////
//
// Tool_tandeminfo::Tool_tandeminfo -- Set the recognized options for the tool.
//

Tool_tandeminfo::Tool_tandeminfo(void) {

	define("c|count=b",                               "show only unique list of interpretations with counts");
	define("d|description|m|meaning=b",               "give description of tandem interpretation");
	define("f|filename=b",                            "show filename");
	define("h|header-only=b",                         "only process interpretations before first data line");
	define("H|body-only=b",                           "only process interpretations after first data line");
	define("l|location=b",                            "show location of interpretation in file (row, column)");
	define("n|sort-by-count=b",                       "sort entries by unique counts from low to high (when -c is used)");
	define("N|sort-by-reverse-count=b",               "sort entries by unique counts from high to low (when -c is used)");
	define("s|sort=b",                                "sort entries alphabetically by tandem interpretation");
	define("t|table=b",                               "embed analysis withing input data");
	define("u|unknown-tandem-interpretations-only=b", "only list unknown interpretations");
	define("x|exclusive-interpretations=b",           "show exclusive interpretation context");
	define("z|zero-indexed-locations=b",              "locations are 0-indexed");
	define("close=b",                                 "close <details> by default in HTML output");
	m_entries.reserve(1000);
}



/////////////////////////////////
//
// Tool_tandeminfo::run -- Do the main work of the tool.
//

bool Tool_tandeminfo::run(HumdrumFileSet& infiles) {
	bool status = true;
	for (int i=0; i<infiles.getCount(); i++) {
		status &= run(infiles[i]);
	}
	return status;
}


bool Tool_tandeminfo::run(const string& indata, ostream& out) {
	HumdrumFile infile(indata);
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_tandeminfo::run(HumdrumFile& infile, ostream& out) {
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_tandeminfo::run(HumdrumFile& infile) {
	initialize();
	processFile(infile);
	return true;
}


//////////////////////////////
//
// Tool_tandeminfo::initialize --
//

void Tool_tandeminfo::initialize(void) {
	m_exclusiveQ  = getBoolean("exclusive-interpretations");
	m_unknownQ    = getBoolean("unknown-tandem-interpretations-only");
	m_filenameQ   = getBoolean("filename");
	m_locationQ   = getBoolean("location");
	m_countQ      = getBoolean("count");
	m_tableQ      = getBoolean("table");
	m_zeroQ       = getBoolean("zero-indexed-locations");
	m_closeQ      = getBoolean("close");
	m_sortQ       = getBoolean("sort");
	m_headerOnlyQ = getBoolean("header-only");
	m_bodyOnlyQ   = getBoolean("body-only");

	m_sortByCountQ = getBoolean("sort-by-count");
	m_sortByReverseCountQ = getBoolean("sort-by-reverse-count");

	if (m_headerOnlyQ && m_bodyOnlyQ) {
		m_headerOnlyQ = 0;
		m_bodyOnlyQ   = 0;
	}

	if (m_countQ && m_locationQ) {
		m_locationQ = false;
	}

	#ifndef __EMSCRIPTEN__
		m_descriptionQ = getBoolean("description");
		m_tableQ       = getBoolean("table");
	#else
		m_descriptionQ = !getBoolean("description");
		m_tableQ       = !getBoolean("table");
	#endif
}



//////////////////////////////
//
// Tool_tandeminfo::processFile --
//

void Tool_tandeminfo::processFile(HumdrumFile& infile) {
	m_entries.clear();
	m_count.clear();

	bool foundDataQ = false;
	for (int i=0; i<infile.getLineCount(); i++) {
		if (infile[i].isManipulator()) {
			continue;
		}
		if (infile[i].isData()) {
			foundDataQ = true;
		}
		if (!infile[i].isInterpretation()) {
			continue;
		}
		if (foundDataQ && m_headerOnlyQ) {
			break;
		}
		if (!foundDataQ && m_bodyOnlyQ) {
			continue;
		}
		for (int j=0; j<infile[i].getFieldCount(); j++) {
			HTp token = infile.token(i, j);
			if (*token == "*") {
				continue;
			}
			string description;
			if (m_descriptionQ) {
				description = getDescription(token);
				if (m_unknownQ) {
					HumRegex hre;
					if (!hre.search(description, m_unknown)) {
						continue;
					}
				}
			}
			m_entries.resize(m_entries.size() + 1);
			m_entries.back().token = token;
			m_entries.back().description = description;
		}
	}

	printEntries(infile);
}



//////////////////////////////
//
// Tool_tandeminfo::printEntries --
//

void Tool_tandeminfo::printEntries(HumdrumFile& infile) {
	if (m_sortQ) {
		sort(m_entries.begin(), m_entries.end(), [](const Entry &a, const Entry &b) {
			string aa = a.token->getText();
			string bb = b.token->getText();
			std::transform(aa.begin(), aa.end(), aa.begin(), ::tolower);
			std::transform(bb.begin(), bb.end(), bb.begin(), ::tolower);
			return (aa < bb);
		});
	}
	if (m_countQ) {
		doCountAnalysis();
	}

	if (m_sortByCountQ) {

		sort(m_entries.begin(), m_entries.end(), [](const Entry &a, const Entry &b) {
			int anum = a.count;
			int bnum = b.count;
			if (anum != bnum) {
				return anum < bnum;
			}
			string aa = a.token->getText();
			string bb = b.token->getText();
			std::transform(aa.begin(), aa.end(), aa.begin(), ::tolower);
			std::transform(bb.begin(), bb.end(), bb.begin(), ::tolower);
			return (aa < bb);
		});

	} else if (m_sortByReverseCountQ) {

		sort(m_entries.begin(), m_entries.end(), [](const Entry &a, const Entry &b) {
			int anum = a.count;
			int bnum = b.count;
			if (anum != bnum) {
				return anum > bnum;
			}
			string aa = a.token->getText();
			string bb = b.token->getText();
			std::transform(aa.begin(), aa.end(), aa.begin(), ::tolower);
			std::transform(bb.begin(), bb.end(), bb.begin(), ::tolower);
			return (aa < bb);
		});

	}

	if (m_tableQ) {
		printEntriesHtml(infile);
	} else {
		printEntriesText(infile);
	}
}



//////////////////////////////
//
// Tool_tandeminfo::doCountAnalysis --
//

void Tool_tandeminfo::doCountAnalysis(void) {
	m_count.clear();
	for (int i=0; i<(int)m_entries.size(); i++) {
		m_count[m_entries[i].token->getText()] += 1;
	}

	// store counts in entries:
	for (int i=0; i<(int)m_entries.size(); i++) {
		m_entries[i].count = m_count[m_entries[i].token->getText()];
	}
}



//////////////////////////////
//
// Tool_tandeminfo::printEntiesHtml -- Print as embedded HTML code at end of
//    input score.
//

void Tool_tandeminfo::printEntriesHtml(HumdrumFile& infile) {
	map<string, bool> processed;  // used for -c option

	m_humdrum_text << infile;

	m_humdrum_text << "!!@@BEGIN: PREHTML" << endl;

	m_humdrum_text << "!!@SCRIPT:" << endl;
	m_humdrum_text << "!!function gotoEditorCoordinate(row, col) {" << endl;
	m_humdrum_text << "!!   if ((typeof EDITOR == 'undefined') || !EDITOR) {" << endl;
	m_humdrum_text << "!!      return;" << endl;
	m_humdrum_text << "!!   }" << endl;
	m_humdrum_text << "!!   gotoLineFieldInEditor(row, col);" << endl;
	m_humdrum_text << "!!}" << endl;
	m_humdrum_text << "!!@CONTENT:" << endl;

	m_humdrum_text << "!!<style>" << endl;
	m_humdrum_text << "!!.PREHTML table.tandeminfo { border: 1px solid black; border-collapse: collapse; max-width: 98%; width:98%; }" << endl;
	m_humdrum_text << "!!.PREHTML table.tandeminfo th, .PREHTML table.tandeminfo td { vertical-align: top; padding-right: 10px; } " << endl;
	m_humdrum_text << "!!.PREHTML table.tandeminfo tr:hover td { background-color: #eee; } " << endl;
	m_humdrum_text << "!!.PREHTML table.tandeminfo th.tandem, .PREHTML table.tandeminfo td.tandem { white-space: nowrap; padding-right: 30px; } " << endl;
	m_humdrum_text << "!!.PREHTML table.tandeminfo td.tandem { font-family:\"Courier New\", Courier, monospace; }" << endl;
	m_humdrum_text << "!!.PREHTML table.tandeminfo td.exclusive { font-family:\"Courier New\", Courier, monospace; } " << endl;
	m_humdrum_text << "!!.PREHTML table.tandeminfo th.location, .PREHTML table.tandeminfo td.location { padding-right: 30px; } " << endl;
	m_humdrum_text << "!!.PREHTML table.tandeminfo th.count { padding-left: 20px; padding-right: 10px; } " << endl;
	m_humdrum_text << "!!.PREHTML table.tandeminfo td.count { text-align: right; padding-right: 30px; } " << endl;
	m_humdrum_text << "!!.PREHTML table.tandeminfo th.exclusive, .PREHTML table.tandeminfo td.exclusive { padding-right: 30px; } " << endl;
	m_humdrum_text << "!!.PREHTML table.tandeminfo tr th:last-child, .PREHTML table.tandeminfo tr td:last-child { width:100%; padding-right: 0; } " << endl;
	m_humdrum_text << "!!.PREHTML table.tandeminfo tr th:last-child, .PREHTML table.tandeminfo tr td:last-child { width:100%; padding-right: 0; } " << endl;
	m_humdrum_text << "!!.PREHTML table.tandeminfo tr th, .PREHTML table.tandeminfo tr td { padding:1px; padding-left:3px; } " << endl;
	m_humdrum_text << "!!.PREHTML span.unknown { color:crimson; font-weight:bold; }" << endl;
	m_humdrum_text << "!!.PREHTML td.squeeze { letter-spacing:-0.5px; }" << endl;

	m_humdrum_text << "!!.PREHTML details { position: relative; padding-left: 20px; }" << endl;
	m_humdrum_text << "!!.PREHTML summary { font-size: 1.5rem; cursor: pointer; list-style: none; }" << endl;
	m_humdrum_text << "!!.PREHTML summary::before { content: '▶'; display: inline-block; width: 2em; margin-left: -1.5em; text-align: center; }" << endl;
	m_humdrum_text << "!!.PREHTML details[open] summary::before { content: '▼'; }" << endl;

	m_humdrum_text << "!!</style>" << endl;

	m_humdrum_text << "!!<details class='tandeminfo' ";
	if (!m_closeQ) {
		m_humdrum_text << "open";
	}
	m_humdrum_text << ">" << endl;;
	m_humdrum_text << "!!<summary class='tandeminfo'>Tandem interpretation information</summary>" << endl;;
	if (!m_entries.empty()) {
		m_humdrum_text << "!!<table class='tandeminfo'>" << endl;

		// print table header
		m_humdrum_text << "!!<tr>" << endl;
		if (m_locationQ) {
			m_humdrum_text << "!!<th class='location'>Location</th>" << endl;
		} else if (m_countQ) {
			m_humdrum_text << "!!<th class='count'>Count</th>" << endl;
		}
		if (m_exclusiveQ) {
			m_humdrum_text << "!!<th class='exclusive'>Exclusive</th>" << endl;
		}
		m_humdrum_text << "!!<th class='tandem' >Tandem</th>" << endl;
		m_humdrum_text << "!!<th class='description'>Description</th>" << endl;
		m_humdrum_text << "!!</tr>" << endl;

		// print table entries
		for (int i=0; i<(int)m_entries.size(); i++) {
			HTp token = m_entries[i].token;
			if (processed[token->getText()]) {
				continue;
			}
			processed[token->getText()] = true;
			m_humdrum_text << "!!<tr" << " onclick='gotoEditorCoordinate(" << token->getLineNumber() << ", " << token->getFieldNumber() << ")'>" << endl;

			if (m_locationQ) {
				m_humdrum_text << "!!<td class='location'>" << endl;
				m_humdrum_text << "!!(" << token->getLineNumber() << ", " << token->getFieldNumber() << ")" << endl;
				m_humdrum_text << "!!</td>" << endl;
			} else if (m_countQ) {
				m_humdrum_text << "!!<td class='count'>";
				m_humdrum_text << m_count[token->getText()];
				m_humdrum_text << "</td>" << endl;
			}

			if (m_exclusiveQ) {
				m_humdrum_text << "!!<td class='exclusive'>" << endl;
				m_humdrum_text << "!!" << token->getDataType() << endl;
				m_humdrum_text << "!!</td>" << endl;
			}

			m_humdrum_text << "!!<td class='tandem";
			if (m_entries[i].token->size() > 15) {
				m_humdrum_text << " squeeze";
			}
			m_humdrum_text << "'>" << endl;
			m_humdrum_text << "!!" << m_entries[i].token << endl;
			m_humdrum_text << "!!</td>" << endl;

			HumRegex hre;
			m_humdrum_text << "!!<td class='description'>" << endl;
			string description = m_entries[i].description;
			hre.replaceDestructive(description, "&lt;", "<", "g");
			hre.replaceDestructive(description, "&gt;", ">", "g");
			hre.replaceDestructive(description, "<span class='tandeminfo unknown'>unknown</span>", "unknown");
			m_humdrum_text << "!!" << description << endl;
			m_humdrum_text << "!!</td>" << endl;

			m_humdrum_text << "!!</tr>" << endl;
		}

		m_humdrum_text << "!!</table>" << endl;
	}

	// print relevant settings
	vector<string> settings;
	if (m_entries.empty()) {
		settings.push_back("No interpretations found");
	}
	if (m_headerOnlyQ) {
		settings.push_back("Only processing header interpretations");
	}
	if (m_bodyOnlyQ) {
		settings.push_back("Only processing body interpretations");
	}
	if (m_unknownQ) {
		settings.push_back("Displaying only unknown interpretations");
	}
	if ((!m_countQ) && m_sortQ && !m_entries.empty()) {
		settings.push_back("List sorted alphabetically by interpretation");
	} else if (m_countQ && m_sortByCountQ) {
		settings.push_back("List sorted low to high by count");
	} else if (m_countQ && m_sortByReverseCountQ) {
		settings.push_back("List sorted high to low by count");
	}
	if (!settings.empty()) {
		m_humdrum_text << "!!<ul>" << endl;
		for (int i=0; i<(int)settings.size(); i++) {
			m_humdrum_text << "!!<li>";
			m_humdrum_text << settings[i];
			m_humdrum_text << "</li>" << endl;
		}
		m_humdrum_text << "!!</ul>" << endl;
	}



	m_humdrum_text << "!!</details>" << endl;
	m_humdrum_text << "!!@@END: PREHTML" << endl;
}



//////////////////////////////
//
// Tool_tandeminfo::printEntiesText --
//

void Tool_tandeminfo::printEntriesText(HumdrumFile& infile) {
	for (int i=0; i<(int)m_entries.size(); i++) {
		HTp token          = m_entries[i].token;
		string description = m_entries[i].description;

		if (m_filenameQ) {
			m_free_text << infile.getFilename() << "\t";
		}
		if (m_locationQ) {
			if (m_zeroQ) {
				int row = token->getLineIndex();
				int col = token->getFieldIndex();
				m_free_text << "(" << row << ", " << col << ")" << "\t";
			} else {
				int row = token->getLineNumber();
				int col = token->getFieldNumber();
				m_free_text << "(" << row << ", " << col << ")" << "\t";
			}
		}
		if (m_exclusiveQ) {
			m_free_text << token->getDataType() << "\t";
		}
		m_free_text << token;
		if (m_descriptionQ) {
			m_free_text << "\t" << description;
		}
		m_free_text << endl;
	}
}



//////////////////////////////
//
// Tool_tandeminfo::getDescription -- Return description of the input token; otherwise, return m_unknown.
//

string Tool_tandeminfo::getDescription(HTp token) {
	string tok = token->substr(1);
	string description;

	description = checkForKeySignature(tok);
	if (description != m_unknown) {
		return description;
	}

	description = checkForKeyDesignation(tok);
	if (description != m_unknown) {
		return description;
	}

	description = checkForInstrumentInfo(tok);
	if (description != m_unknown) {
		return description;
	}

	description = checkForLabelInfo(tok);
	if (description != m_unknown) {
		return description;
	}

	description = checkForTimeSignature(tok);
	if (description != m_unknown) {
		return description;
	}

	description = checkForMeter(tok);
	if (description != m_unknown) {
		return description;
	}

	description = checkForTempoMarking(tok);
	if (description != m_unknown) {
		return description;
	}

	description = checkForClef(tok);
	if (description != m_unknown) {
		return description;
	}

	description = checkForStaffPartGroup(tok);
	if (description != m_unknown) {
		return description;
	}

	description = checkForTuplet(tok);
	if (description != m_unknown) {
		return description;
	}

	description = checkForHands(tok);
	if (description != m_unknown) {
		return description;
	}

	description = checkForPosition(tok);
	if (description != m_unknown) {
		return description;
	}

	description = checkForCue(tok);
	if (description != m_unknown) {
		return description;
	}

	description = checkForFlip(tok);
	if (description != m_unknown) {
		return description;
	}

	description = checkForTremolo(tok);
	if (description != m_unknown) {
		return description;
	}

	description = checkForOttava(tok);
	if (description != m_unknown) {
		return description;
	}

	description = checkForPedal(tok);
	if (description != m_unknown) {
		return description;
	}

	description = checkForBracket(tok);
	if (description != m_unknown) {
		return description;
	}

	description = checkForRscale(tok);
	if (description != m_unknown) {
		return description;
	}

	description = checkForTimebase(tok);
	if (description != m_unknown) {
		return description;
	}

	description = checkForTransposition(tok);
	if (description != m_unknown) {
		return description;
	}

	description = checkForGrp(tok);
	if (description != m_unknown) {
		return description;
	}

	description = checkForStria(tok);
	if (description != m_unknown) {
		return description;
	}

	description = checkForFont(tok);
	if (description != m_unknown) {
		return description;
	}

	description = checkForVerseLabels(tok);
	if (description != m_unknown) {
		return description;
	}

	description = checkForLanguage(tok);
	if (description != m_unknown) {
		return description;
	}

	description = checkForStemInfo(tok);
	if (description != m_unknown) {
		return description;
	}

	description = checkForXywh(tok);
	if (description != m_unknown) {
		return description;
	}

	description = checkForCustos(tok);
	if (description != m_unknown) {
		return description;
	}

	description = checkForTextInterps(tok);
	if (description != m_unknown) {
		return description;
	}

	description = checkForRep(tok);
	if (description != m_unknown) {
		return description;
	}

	description = checkForPline(tok);
	if (description != m_unknown) {
		return description;
	}

	description = checkForTacet(tok);
	if (description != m_unknown) {
		return description;
	}

	description = checkForFb(tok);
	if (description != m_unknown) {
		return description;
	}

	description = checkForColor(tok);
	if (description != m_unknown) {
		return description;
	}

	description = checkForThru(tok);
	if (description != m_unknown) {
		return description;
	}

	HumRegex hre;
	if (hre.search(token, "\\s+$")) {
		return "unknown (space at end of interpretation may be the problem)";
	} else {
		return m_unknown;
	}
}



//////////////////////////////
//
// Tool_tandeminfo::checkForThru -- Humdrum Toolkit interpretations related to thru command.
//

string Tool_tandeminfo::checkForThru(const string& tok) {
	if (tok == "thru") {
		return "data processed by thru command (expansion lists processed)";
	}

	return m_unknown;
}



//////////////////////////////
//
// Tool_tandeminfo::checkForColor -- Extended interprerations for coloring notes in **kern data.
//     Used in verovio.
//

string Tool_tandeminfo::checkForColor(const string& tok) {
	HumRegex hre;
	if (hre.search(tok, "^color:(.*)")) {
		string color = hre.getMatch(1);
		string output;
		if (hre.search(tok, "^#[0-9A-Fa-f]{3}$")) {
			output = "3-digit hex ";
		} else if (hre.search(tok, "^#[0-9A-Fa-f]{6}$")) {
			output = "6-digit hex ";
		} else if (hre.search(tok, "^#[0-9A-Fa-f]{8}$")) {
			output = "8-digit hex  (RGB + transparency)";
		} else if (hre.search(tok, "^rgb(\\d+\\s*,\\s*\\d+\\s*,\\s*\\d+)$")) {
			output = "RGB integer";
		} else if (hre.search(tok, "^rgb(\\d+\\s*,\\s*\\d+\\s*,\\s*\\d+\\s*,[\\d.]+)$")) {
			output = "RGB integer with alpha";
		} else if (hre.search(tok, "^hsl(\\d+\\s*,\\s*\\d+%\\s*,\\s*\\d+%)$")) {
			output = "HSL";
		} else if (hre.search(tok, "^hsl(\\d+\\s*,\\s*\\d+%\\s*,\\s*\\d+%,\\s*[\\d.]+)$")) {
			output = "HSL with alpha";
		} else if (hre.search(tok, "^[a-z]+$")) {
			output = "named ";
		}
		output += " color";
	}

	return m_unknown;
}



//////////////////////////////
//
// Tool_tandeminfo::checkForFb -- Extended interprerations especially for **fb (**fa) exclusive
//     interpretations.
//

string Tool_tandeminfo::checkForFb(const string& tok) {
	if (tok == "reverse") {
		return "reverse order of accidental and number in figured bass";
	}
	if (tok == "Xreverse") {
		return "stop reversing order of accidental and number in figured bass";
	}

	return m_unknown;
}



//////////////////////////////
//
// Tool_tandeminfo::checkForTacet -- Extended interprerations for marking parts that are not
//     playing (rests only) in a movement/movement subsection.
//

string Tool_tandeminfo::checkForTacet(const string& tok) {
	if (tok == "tacet") {
		return "part is tacet in movement/section";
	}
	if (tok == "Xtacet") {
		return "end of part tacet";
	}

	return m_unknown;
}



//////////////////////////////
//
// Tool_tandeminfo::checkForRep -- Extended interprerations for poetic line analysis related to pline tool.
//

string Tool_tandeminfo::checkForPline(const string& tok) {
	HumRegex hre;
	if (hre.search(tok, "^pline:(\\d+)([abcr]*)$")) {
		string number = hre.getMatch(1);
		string info = hre.getMatch(2);
		string output = "poetic line markup: " + number + info;
		return output;
	}

	return m_unknown;
}



//////////////////////////////
//
// Tool_tandeminfo::checkForRep -- Extended interprerations for adding repeat sign shorthand for
//     repeated music.
//

string Tool_tandeminfo::checkForRep(const string& tok) {
	if (tok == "rep") {
		return "start of repeat sign replacing notes/rests";
	}
	if (tok == "Xrep") {
		return "end of repeat sign replacing notes/rests";
	}

	return m_unknown;
}



//////////////////////////////
//
// Tool_tandeminfo::checkForTextInterps -- Extended interprerations for **text and **silbe
//

string Tool_tandeminfo::checkForTextInterps(const string& tok) {
	if (tok == "ij") {
		return "start of text repeat region";
	}
	if (tok == "Xij") {
		return "end of text repeat region";
	}
	if (tok == "edit") {
		return "start of editorial text region";
	}
	if (tok == "Xedit") {
		return "end of editorial text region";
	}

	return m_unknown;
}



//////////////////////////////
//
// Tool_tandeminfo::checkForCustos -- Extended interprerations for marker
//     at end of system for next note in part.
//

string Tool_tandeminfo::checkForCustos(const string& tok) {
	HumRegex hre;

	if (tok == "custos") {
		return "custos, pitch unspecified";
	}

	if (tok == "custos:") {
		return "custos, pitch unspecified";
	}

	if (hre.search(tok, "^custos:([A-G]+|[a-g]+)(#+|-+|n)?$")) {
		// also deal with chord custos
		string pitch = hre.getMatch(1);
		string accid = hre.getMatch(2);
		string output = "custos on pitch " + pitch + accid;
		return output;
	}

	return m_unknown;
}



//////////////////////////////
//
// Tool_tandeminfo::checkForStemInfo -- Extended interprerations
//      for visual display of stems (on left or right side of notes).
//

string Tool_tandeminfo::checkForXywh(const string& tok) {
	HumRegex hre;
	if (hre.search(tok, "^xywh-([^:\\s]+):(\\d+),(\\d+),(\\d+),(\\d+)$")) {
		string page = hre.getMatch(1);
		string x = hre.getMatch(2);
		string y = hre.getMatch(3);
		string w = hre.getMatch(4);
		string h = hre.getMatch(5);
		string output = "IIIF bounding box, page=";
		output += page;
		output += ", x=" + x;
		output += ", y=" + y;
		output += ", w=" + w;
		output += ", h=" + h;
		return output;
	}

	return m_unknown;
}



//////////////////////////////
//
// Tool_tandeminfo::checkForStemInfo -- Extended interprerations
//      for visual display of stems (on left or right side of notes).
//

string Tool_tandeminfo::checkForStemInfo(const string& tok) {
	HumRegex hre;

	if (hre.search(tok, "^(\\d+)/left$")) {
		string rhythm = hre.getMatch(1);
		string output = rhythm + "-rhythm notes always have stem up on the left";
		return output;
	}

	if (hre.search(tok, "^(\\d+)\\\\left$")) {
		string rhythm = hre.getMatch(1);
		string output = rhythm + "-rhythm notes always have stem down on the left";
		return output;
	}

	if (hre.search(tok, "^(\\d+)/right$")) {
		string rhythm = hre.getMatch(1);
		string output = rhythm + "-rhythm notes always have stem up on the right";
		return output;
	}

	if (hre.search(tok, "^(\\d+)\\\\right$")) {
		string rhythm = hre.getMatch(1);
		string output = rhythm + "-rhythm notes always have stem down on the right";
		return output;
	}

	if (tok == "all/right") {
		string output = "all notes always have stem up on the right";
		return output;
	}

	if (tok == "all\\right") {
		string output = "all notes always have stem down on the right";
		return output;
	}

	if (tok == "all/left") {
		string output = "all notes always have stem up on the left";
		return output;
	}

	if (tok == "all\\left") {
		string output = "all notes always have stem down on the left";
		return output;
	}

	if (tok == "all/center") {
		string output = "all notes always have stem up on notehead center";
		return output;
	}

	if (tok == "all\\center") {
		string output = "all notes always have stem down on notehead center";
		return output;
	}
	// there is also "middle" which is the same as "center";

	return m_unknown;
}



//////////////////////////////
//
// Tool_tandeminfo::checkForLanguage -- Humdrum Toolkit and extended interprerations
//      for langauages (for **text and **silbe).
//

string Tool_tandeminfo::checkForLanguage(const string& tok) {
	HumRegex hre;

	if (hre.search(tok, "^L([A-Z][^\\s]+)$")) {
		string language = hre.getMatch(1);
		string output = "Language, old style: " + language;
		return output;
	}

	if (hre.search(tok, "^lang:([A-Z]{2,3})$")) {
		string code = hre.getMatch(1);
		string name = Convert::getLanguageName(code);
		if (name.empty()) {
			return "language code " + code +  " (unknown)";
		}
		string output = "language code";
		if (code.size() == 2) {
			output = "ISO 639-3 two-letter language code (";
		} else if (code.size() == 3) {
			output = "ISO 639-3 three-letter language code (";
		}
		output += name;
		output += ")";
		return output;
	}

	return m_unknown;
}



//////////////////////////////
//
// Tool_tandeminfo::checkForVerseLabels -- Extended tandem interpretations (used by verovio
//      for visual rendeing of notation).
//

string Tool_tandeminfo::checkForVerseLabels(const string& tok) {
	HumRegex hre;
	if (hre.search(tok, "^v:(.*)$")) {
		string output = "verse label \"" + hre.getMatch(1) + "\"";
		return output;
	}
	if (hre.search(tok, "^vv:(.*)$")) {
		string output = "verse label \"" + hre.getMatch(1) + "\", repeated after each system break";
		return output;
	}

	return m_unknown;
}



//////////////////////////////
//
// Tool_tandeminfo::checkForFont -- Extended interprtations for styling **text and **silbe.
//

string Tool_tandeminfo::checkForFont(const string& tok) {
	if (tok == "italic") {
		return "use italic font style";
	}
	if (tok == "Xitalic") {
		return "stop using italic font style";
	}
	if (tok == "bold") {
		return "use bold font style";
	}
	if (tok == "Xbold") {
		return "stop using bold font style";
	}

	return m_unknown;
}



//////////////////////////////
//
// Tool_tandeminfo::checkForStria -- Humdrum Toolkit interpretation.
//

string Tool_tandeminfo::checkForStria(const string& tok) {
	HumRegex hre;
	if (hre.search(tok, "^stria(\\d+)$")) {
		string output = "number of staff lines:" + hre.getMatch(1);
		return output;
	}

	return m_unknown;
}



//////////////////////////////
//
// Tool_tandeminfo::checkForGrp -- Polyrhythm project interpretations for
//      polyrhythm group assignments.  Related to humlib composite tool.
//

string Tool_tandeminfo::checkForGrp(const string& tok) {
	HumRegex hre;
	if (hre.search(tok, "^grp:([AB])$")) {
		string output = "composite rhythm grouping label " + hre.getMatch(1);
		return output;
	}

	return m_unknown;
}



//////////////////////////////
//
// Tool_tandeminfo::checkForTransposition -- Humdrum Toolkit interpretations related
//      to pitch transposition.
//

string Tool_tandeminfo::checkForTransposition(const string& tok) {
	HumRegex hre;

	if (hre.search(tok, "ITrd(-?\\d+)c(-?\\d+)$")) {
		string diatonic = hre.getMatch(1);
		string chromatic = hre.getMatch(2);
		string output = "transposition for written part, diatonic: ";
		output += diatonic;
		output += ", chromatic: ";
		output += chromatic;
		return output;
	}

	if (hre.search(tok, "Trd(-?\\d+)c(-?\\d+)$")) {
		string diatonic = hre.getMatch(1);
		string chromatic = hre.getMatch(2);
		string output = "transposed by diatonic: ";
		output += diatonic;
		output += ", chromatic: ";
		output += chromatic;
		return output;
	}

	return m_unknown;
}



//////////////////////////////
//
// Tool_tandeminfo::checkForTimebase -- Humdrum Toolkit interpretations related
//      to the timebase tool.
//

string Tool_tandeminfo::checkForTimebase(const string& tok) {
	HumRegex hre;
	if (hre.search(tok, "^tb(\\d+)$")) {
		string number = hre.getMatch(1);
		string output = "timebase: all data lines (should) have a duration of " + number;
		return output;
	}

	return m_unknown;
}



//////////////////////////////
//
// Tool_tandeminfo::checkForRscale -- Extended interpretation for adjusting the visual
//     display of note durations when they do not match the logical
//     note durations (such as show a quarter note as if it were a
//     half note, which would be indicated by "*rscale:2". Or a
//     half note as if it were a quarter note with "*rscale:1/2".
//     Also related to the rscale tool from Humdrum Extras and humlib.
//     Used in verovio.
//

string Tool_tandeminfo::checkForRscale(const string& tok) {
	HumRegex hre;
	if (hre.search(tok, "^rscale:(\\d+)(/\\d+)?$")) {
		string fraction = hre.getMatch(1) + hre.getMatch(2);
		string output = "visual rhythmic scaling factor " + fraction;
		return output;
	}

	return m_unknown;
}



//////////////////////////////
//
// Tool_tandeminfo::checkForBracket -- Extended interpretations for displaying
//     various bracket lines in visual music notation.
//

string Tool_tandeminfo::checkForBracket(const string& tok) {
	// Coloration
	if (tok == "col") {
		return "start of coloration bracket";
	}
	if (tok == "Xcol") {
		return "end of coloration bracket";
	}

	// Ligatures
	if (tok == "lig") {
		return "start of ligature bracket";
	}
	if (tok == "Xlig") {
		return "end of ligature bracket";
	}

	// Schoenberg
	if (tok == "haupt") {
		return "start of Hauptstimme bracket";
	}
	if (tok == "Xhaupt") {
		return "end of Hauptstimme bracket";
	}
	if (tok == "neben") {
		return "start of Nebenstimme bracket";
	}
	if (tok == "Xneben") {
		return "end of Nebenstimme bracket";
	}
	if (tok == "rhaupt") {
		return "start of Hauptrhythm bracket";
	}
	if (tok == "Xrhaupt") {
		return "end of Hauptrhythm bracket";
	}

	return m_unknown;
}



//////////////////////////////
//
// Tool_tandeminfo::checkForPedal -- Extended interpretations for displaying
//     ottava lines in music notation.
//

string Tool_tandeminfo::checkForPedal(const string& tok) {
	if (tok == "ped") {
		return "sustain pedal down";
	}
	if (tok == "Xped") {
		return "sustain pedal up";
	}
	return m_unknown;
}



//////////////////////////////
//
// Tool_tandeminfo::checkForOttava -- Extended interpretations for displaying
//     ottava lines in music notation.
//

string Tool_tandeminfo::checkForOttava(const string& tok) {
	if (tok == "8va") {
		return "start of 8va line";
	}
	if (tok == "X8va") {
		return "end of 8va line";
	}
	if (tok == "8ba") {
		return "start of 8ba (ottava basso) line";
	}
	if (tok == "X8ba") {
		return "end of 8ba (ottava basso) line";
	}
	if (tok == "15ma") {
		return "start of 15ma line";
	}
	if (tok == "X15ma") {
		return "end of 15ma line";
	}
	if (tok == "coll8ba") {
		return "coll ottava basso start";
	}
	if (tok == "Xcoll8ba") {
		return "coll ottava basso end";
	}
	return m_unknown;
}




//////////////////////////////
//
// Tool_tandeminfo::checkForTremolo -- Extended interpretations for collapsing
//     repeated notes into tremolos in music notation rendering.
//     Used specifically by verovio.
//

string Tool_tandeminfo::checkForTremolo(const string& tok) {
	if (tok == "tremolo") {
		return "start of tremolo rendering of repeated notes";
	}
	if (tok == "Xtremolo") {
		return "end of tremolo rendering of repeated notes";
	}
	return m_unknown;
}


//////////////////////////////
//
// Tool_tandeminfo::checkForFlip -- Extended interpretations for use with the
//     flipper humlib command.
//

string Tool_tandeminfo::checkForFlip(const string& tok) {
	if (tok == "flip") {
		return "switch order of subspines, specific to flipper tool";
	}
	if (tok == "Xflip") {
		return "cancel flipping of subspine, specific to flipper tool";
	}
	return m_unknown;
}



//////////////////////////////
//
// Tool_tandeminfo::checkForCue -- Extended interpretations for visual rendering
//     *cue means display as cue-sized notes.  Probably change
//     this so that *cue means following notes are cue notes
//     and add *cuesz for cue-sized notes (that are not cues
//     from other instruments).
//

string Tool_tandeminfo::checkForCue(const string& tok) {
	if (tok == "cue") {
		return "cue-sized notation follows";
	}
	if (tok == "Xcue") {
		return "cancel cue-sized notation";
	}
	return m_unknown;
}



//////////////////////////////
//
// Tool_tandeminfo::checkForPosition -- Extended interpretations for visual rendering
//     data above/below staff.  Useful in particular for **dynam.
//     Staff number in part (relative to top staff) can be given
//     as a number following a colon after the placement.
//

string Tool_tandeminfo::checkForPosition(const string& tok) {
	if (tok == "above") {
		return "place items above staff";
	}
	if (tok == "above:1") {
		return "place items above first staff of part";
	}
	if (tok == "above:2") {
		return "place items above second staff of part";
	}
	if (tok == "below") {
		return "place items below staff";
	}
	if (tok == "below:1") {
		return "place items below first staff of part";
	}
	if (tok == "below:2") {
		return "place items below second staff of part";
	}
	if (tok == "center") {
		return "centered items between two staves";
	}
	return m_unknown;
}



//////////////////////////////
//
// Tool_tandeminfo::checkForHands -- Extended interpretations to indicate which
//     hand is playing the notes (for grand-staff keyboard in particular).
//

string Tool_tandeminfo::checkForHands(const string& tok) {
	if (tok == "LH") {
		return "notes played by left hand";
	}
	if (tok == "RH") {
		return "notes played by right hand";
	}
	return m_unknown;
}



//////////////////////////////
//
// Tool_tandeminfo::checkForTuplet -- Extended interpretations for **kern data to control
//     visual stylings of tuplet numbers and brackets.
//

string Tool_tandeminfo::checkForTuplet(const string& tok) {
	if (tok == "Xbrackettup") {
		return "suppress brackets for tuplets";
	}
	if (tok == "brackettup") {
		return "do not suppress brackets for tuplets (default)";
	}
	if (tok == "tuplet") {
		return "show tuplet numbers (default)";
	}
	if (tok == "Xtuplet") {
		return "do not show tuplet numbers";
	}
	if (tok == "tupbreak") {
		return "break tuplet at this point";
	}
	

	return m_unknown;
}



//////////////////////////////
//
// Tool_tandeminfo::checkForStaffPartGroup -- Humdrum Toolkit interpretation (*staff), and
//    extensions to *part to group multiple staves into a single part as
//    well as *group for grouping staves/parts into instrument class
//    groups (useful for controlling connecting barlines across multiple
//    staves).
//

string Tool_tandeminfo::checkForStaffPartGroup (const string& tok) {
	HumRegex hre;

	if (hre.search(tok, "^staff(\\d+)(/\\d+)*$")) {
		string number = hre.getMatch(1);
		string second = hre.getMatch(2);
		string output;
		if (second.empty()) {
			output = "staff " + number;
			return output;
		}
		output = "staves " + tok.substr(5);
		return output;
	}

	if (hre.search(tok, "^part(\\d+)(/\\d+)*$")) {
		string number = hre.getMatch(1);
		string second = hre.getMatch(2);
		string output;
		if (second.empty()) {
			output = "part " + number;
			return output;
		}
		output = "parts " + tok.substr(5);
		return output;
	}

	if (hre.search(tok, "^group(\\d+)(/\\d+)*$")) {
		string number = hre.getMatch(1);
		string second = hre.getMatch(2);
		string output;
		if (second.empty()) {
			output = "group " + number;
			return output;
		}
		output = "groups " + tok.substr(5);
		return output;
	}

	return m_unknown;
}



//////////////////////////////
//
// Tool_tandeminfo::checkForClef -- Humdrum Toolkit interpretations.  Extension
//     is "*clefX" for percussion clef (checked for below),
//     and *clefG2yy for an invisible clef (not visually rendered).
//

string Tool_tandeminfo::checkForClef(const string& tok) {
	HumRegex hre;
	if (hre.search(tok, "^(m|o)?clef([GFCX])(.*?)([12345])?(yy)?$")) {
		string modori = hre.getMatch(1);
		string ctype = hre.getMatch(2);
		string octave = hre.getMatch(3);
		string line = hre.getMatch(4);
		string invisible = hre.getMatch(5);
		string output = "clef: ";
		if (ctype == "X") {
			output += "percussion";
			if (!line.empty()) {
				output += ", line=" + line;
			}
			if (!octave.empty()) {
				return m_unknown;
			}
		} else {
			output += ctype;
			if (line.empty()) {
				return m_unknown;
			}
			output += ", line=" + line;
			if (!octave.empty()) {
				if (hre.search(octave, "^v+$")) {
					output += ", octave displacement -" + to_string(octave.size());
				} else if (hre.search(octave, "^\\^+$")) {
					output += ", octave displacement +" + to_string(octave.size());
				}
			}
		}
		if (!invisible.empty()) {
			output += ", invisible (not displayed in music rendering)";
		}
		return output;
	}

	return m_unknown;
}



//////////////////////////////
//
// Tool_tandeminfo::checkForTimeSignature -- Humdrum Toolkit interpretations.
//      Extended for use with rare non-notatable rhythm bases, such as
//      *M3/3%2 for three triplet whole notes to the measure (this
//      is equivalent in duration to *M2/1 but gives a more refined
//      version of what the beat is.  Maybe also allow "*M2/4." which
//      would be equivalent to an explicit compound *M6/8 time signature.
//      Other extensions could also be done such as *M4/4yy for an invisible
//      time signature.  And another extension could be *M2/8+3/8 for *M5/8
//      split into 2 + 3 beat groupings.
//

string Tool_tandeminfo::checkForTimeSignature(const string& tok) {
	HumRegex hre;
	if (tok == "MX") {
		return "unmeasured music time signature";
	}
	if (hre.search(tok, "^MX/(\\d+)(%\\d+)?(yy)?")) {
		string output = "unmeasured music with beat " + hre.getMatch(1) + hre.getMatch(2);
		if (hre.getMatch(3) == "yy") {
			output += ", invisible";
			return output;
		}
	}
	if (hre.search(tok, "^M(\\d+)/(\\d+)(%\\d+)?(yy)?$")) {
		string top = hre.getMatch(1);
		string bot = hre.getMatch(2) + hre.getMatch(3);
		string invisible = hre.getMatch(4);
		string output = "time signature: top=" + top + ", bottom=" + bot;
		if (invisible == "yy") {
			output += ", invisible";
		}
		return output;
	}

	return m_unknown;
}



//////////////////////////////
//
// Tool_tandeminfo::checkForMeter -- Humdrum Toolkit interpretations. Extended for use
//    with mensural signs.
//

string Tool_tandeminfo::checkForMeter(const string& tok) {
	HumRegex hre;
	if (hre.search(tok, "^(m|o)?met\\((.*?)\\)$")) {
		string modori = hre.getMatch(1);
		string meter = hre.getMatch(2);
		if (meter == "c") {
			return "meter: common time";
		}
		if (meter == "c\\|") {
			return "meter: cut time";
		}
		if (meter == "") {
			return "meter: empty";
		}
		string output = "mensuration sign: " + meter;
		return output;
	}

	return m_unknown;
}



//////////////////////////////
//
// Tool_tandeminfo::checkForTempoMarking -- Humdrum Toolit interpretations.
//

string Tool_tandeminfo::checkForTempoMarking(const string& tok) {
	HumRegex hre;
	if (hre.search(tok, "^MM(\\d+)(\\.\\d*)?$")) {
		string tempo = hre.getMatch(1) + hre.getMatch(2);
		string output = "tempo: " + tempo + " quarter notes per minute";
		return output;
	}

	if (hre.search(tok, "^MM\\[(.*?)\\]$")) {
		string text = hre.getMatch(1);
		string output = "text-based tempo: " + text;
		return output;
	}

	return m_unknown;
}



//////////////////////////////
//
// Tool_tandeminfo::checkForLabelInfo -- Humdrum Toolkit interpretations.
//     Used by the thru command.
//

string Tool_tandeminfo::checkForLabelInfo(const string& tok) {
	HumRegex hre;
	if (!hre.search(tok, "^>")) {
		return m_unknown;
	}

	if (hre.search(tok, "^>(\\[.*\\]$)")) {
		string list = hre.getMatch(1);
		string output = "default expansion list: " + list;
		return output;
	}

	if (hre.search(tok, "^>([^[\\[\\]]+)(\\[.*\\]$)")) {
		string expansionName = hre.getMatch(1);
		string list = hre.getMatch(2);
		string output = "alternate expansion list: label: " + expansionName;
		output += ", expansion list: " + list;
		return output;
	}

	if (hre.search(tok, "^>([^\\[\\]]+)$")) {
		string label = hre.getMatch(1);
		string output = "expansion label: " + label;
		return output;
	}

	return m_unknown;

}



//////////////////////////////
//
// Tool_tandeminfo::checkForInstrumentInfo -- Humdrum Toolkit and extended interpretations.
//     Humdrum Tookit:
//         instrument group	 *IG
//         instrument class	*IC
//         instrument code	*I
//     Extended:
//         instrument name *I"
//         instrument number *I#
//         instrument abbreviation *I'
//
//     modori tool extensions:
//         *mI == modernized
//         *oI == original
//

string Tool_tandeminfo::checkForInstrumentInfo(const string& tok) {
	HumRegex hre;

	if (hre.search(tok, "^(m|o)?I\"(.*)$")) {
		string modori = hre.getMatch(1);
		string name = hre.getMatch(2);
		string output = "text to display in fromt of staff on first system (usually instrument name): \"";
		output += name;
		output += "\"";
		if (modori == "o") {
			output += " (original)";
		} else if (modori == "m") {
			output += " (modern)";
		}
		if (hre.search(tok, "\\\\n")) {
			output += ", \"\\n\" means a line break";
		}
		return output;
	}

	if (hre.search(tok, "^(m|o)?I'(.*)$")) {
		string modori = hre.getMatch(1);
		string abbr = hre.getMatch(2);
		string output = "text to display in front of staff on secondary systems (usually instrument abbreviation): \"";
		output += abbr;
		output += "\"";
		if (modori == "o") {
			output += " (original)";
		} else if (modori == "m") {
			output += " (modern)";
		}
		if (hre.search(tok, "\\\\n")) {
			output += ", \"\\n\" means a line break";
		}
		return output;
	}


	if (hre.search(tok, "^(m|o)?IC([^\\s]*)$")) {
		string modori = hre.getMatch(1);
		string iclass = hre.getMatch(2);
		bool andy = false;
		bool ory  = false;
		vector<string> iclasses;
		string tok2 = tok;
		hre.replaceDestructive(tok2, "", "IC", "g");
		if (hre.search(tok2, "&")) {
			hre.split(iclasses, tok2, "&+");
			andy = true;
		} else if (hre.search(tok2, "\\|")) {
			hre.split(iclasses, tok2, "\\++");
			ory = true;
		} else {
			iclasses.push_back(tok2);
		}
		string output;
		if (modori == "o") {
			output += "(original) ";
		} else if (modori == "m") {
			output += "(modern) ";
		}
		output += "instrument class";
		if (iclasses.size() != 1) {
			output += "es";
		}
		output += ":";
		for (int i=0; i<(int)iclasses.size(); i++) {
			output += " ";
			output += iclasses[i];
			HumInstrument inst;
			inst.setHumdrum(iclasses[i]);
			string name;
			if (iclasses[i] == "bras") {
				name = "brass";
			} else if (iclasses[i] == "idio") {
				name = "percussion";
			} else if (iclasses[i] == "klav") {
				name = "keyboards";
			} else if (iclasses[i] == "str") {
				name = "strings";
			} else if (iclasses[i] == "vox") {
				name = "voices";
			} else if (iclasses[i] == "ww") {
				name = "woodwinds";
			} else if (iclasses[i] != "") {
				name = "unknown";
			}
			if (!name.empty()) {
				output += "=\"" + name + "\"";
			}
			if (i < (int)iclasses.size() - 1) {
				if (andy) {
					output += " and ";
				} else if (ory) {
					output += " or ";
				}
			}
		}
		if (modori == "o") {
			output += " (original)";
		} else if (modori == "m") {
			output += " (modern)";
		}
		return output;
	}


	if (hre.search(tok, "^(m|o)?IG([^\\s]*)$")) {
		string modori = hre.getMatch(1);
		string group = hre.getMatch(2);
		bool andy = false;
		bool ory  = false;
		vector<string> groups;
		string tok2 = tok;
		hre.replaceDestructive(tok2, "", "IG", "g");
		if (hre.search(tok2, "&")) {
			hre.split(groups, tok2, "&+");
			andy = true;
		} else if (hre.search(tok2, "\\|")) {
			hre.split(groups, tok2, "\\++");
			ory = true;
		} else {
			groups.push_back(tok2);
		}
		string output;
		if (modori == "o") {
			output += "(original) ";
		} else if (modori == "m") {
			output += "(modern) ";
		}
		output += "instrument group";
		if (groups.size() != 1) {
			output += "s";
		}
		output += ":";
		for (int i=0; i<(int)groups.size(); i++) {
			output += " ";
			output += groups[i];
			HumInstrument inst;
			inst.setHumdrum(groups[i]);
			string name;
			if (groups[i] == "acmp") {
				name = "=accompaniment";
			} else if (groups[i] == "solo") {
				name = "=solo";
			} else if (groups[i] == "cont") {
				name = "=basso-continuo";
			} else if (groups[i] == "ripn") {
				name = "=ripieno";
			} else if (groups[i] == "conc") {
				name = "=concertino";
			} else if (groups[i] != "") {
				name = "=unknown";
			}
			if (!name.empty()) {
				output += "=\"" + name + "\"";
			}
			if (i < (int)groups.size() - 1) {
				if (andy) {
					output += " and ";
				} else if (ory) {
					output += " or ";
				}
			}
		}
		if (modori == "o") {
			output += " (original)";
		} else if (modori == "m") {
			output += " (modern)";
		}
		return output;
	}

	if (hre.search(tok, "^(m|o)?I#(\\d+)$")) {
		string modori = hre.getMatch(1);
		string number = hre.getMatch(2);
		string output = "sub-instrument number: ";
		output += number;
		if (modori == "o") {
			output += " (original)";
		} else if (modori == "m") {
			output += " (modern)";
		}
		return output;
	}

	if (hre.search(tok, "^(m|o)?I([a-z][a-zA-Z0-9_|&-]+)$")) {
		string modori = hre.getMatch(1);
		string code = hre.getMatch(2);
		bool andy = false;
		bool ory  = false;
		vector<string> codes;
		string tok2 = tok;
		hre.replaceDestructive(tok2, "", "I", "g");
		if (hre.search(tok2, "&")) {
			hre.split(codes, tok2, "&+");
			andy = true;
		} else if (hre.search(tok2, "\\|")) {
			hre.split(codes, tok2, "\\++");
			ory = true;
		} else {
			codes.push_back(tok2);
		}
		string output;
		if (modori == "o") {
			output += "(original) ";
		} else if (modori == "m") {
			output += "(modern) ";
		}
		output += "instrument code";
		if (codes.size() != 1) {
			output += "s";
		}
		output += ":";
		for (int i=0; i<(int)codes.size(); i++) {
			output += " ";
			output += codes[i];
			HumInstrument inst;
			inst.setHumdrum(codes[i]);
			string text = inst.getName();
			if (!text.empty()) {
				output += "= \"" + text + "\"";
			} else {
				output += "= unknown code";
			}
			output += "";
			if (i < (int)codes.size() - 1) {
				if (andy) {
					output += " and ";
				} else if (ory) {
					output += " or ";
				}
			}
		}

		return output;
	}

	return m_unknown;
}



//////////////////////////////
//
// Tool_tandeminfo::checkForKeySignature -- Standard Humdrum Toolkit interpretations.
//     Extended key signatures are possible (and detected by this function),
//     but typically the standard ones are in circle-of-fifths orderings.
//     This function also allows double sharps/flats in the key signature
//     which are very uncommon in real music.  Standard key signatures:
//
//     *k[f#c#g#d#a#e#b#]
//     *k[f#c#g#d#a#e#]
//     *k[f#c#g#d#a#]
//     *k[f#c#g#d#]
//     *k[f#c#g#]
//     *k[f#c#]
//     *k[f#]
//     *k[]
//     *k[b-]
//     *k[b-e-]
//     *k[b-e-a-]
//     *k[b-e-a-d-]
//     *k[b-e-a-d-g-]
//     *k[b-e-a-d-g-c-]
//     *k[b-e-a-d-g-c-f-]
//

string Tool_tandeminfo::checkForKeySignature(const string& tok) {

	// visual styling interpretations for key signatures:
	if (tok == "kcancel") {
		return "show cancellation naturals when changing key signatures";
	}
	if (tok == "Xkcancel") {
		return "do not show cancellation naturals when changing key signatures (default)";
	}

	if (tok == "k[]") {
		return "key signature: no sharps or flats";
	}
	if (tok == "ok[]") {
		return "original key signature: no sharps or flats";
	}
	if (tok == "mk[]") {
		return "modern key signature: no sharps or flats";
	}

	HumRegex hre;
	string modori;
	if (hre.search(tok, "^([m|o])k\\[")) {
		modori = hre.getMatch(1);
	}

	if (hre.search(tok, "^(?:m|o)?k\\[(([a-gA-G]+[n#-]{1,2})+)\\]$")) {
		string modori;
		string pcs = hre.getMatch(1);
		bool standardQ = false;
		if (pcs == "f#") {
			standardQ = true;
		} else if (pcs == "b-") {
			standardQ = true;
		} else if (pcs == "f#c#") {
			standardQ = true;
		} else if (pcs == "b-e-") {
			standardQ = true;
		} else if (pcs == "f#c#g#") {
			standardQ = true;
		} else if (pcs == "b-e-a-") {
			standardQ = true;
		} else if (pcs == "f#c#g#d#") {
			standardQ = true;
		} else if (pcs == "b-e-a-d-") {
			standardQ = true;
		} else if (pcs == "f#c#g#d#a#") {
			standardQ = true;
		} else if (pcs == "b-e-a-d-g-") {
			standardQ = true;
		} else if (pcs == "f#c#g#d#a#e#") {
			standardQ = true;
		} else if (pcs == "b-e-a-d-g-c-") {
			standardQ = true;
		} else if (pcs == "f#c#g#d#a#e#b#") {
			standardQ = true;
		} else if (pcs == "b-e-a-d-g-c-f-") {
			standardQ = true;
		}

		string output;
		if (modori == "m") {
			output = "modern ";
		} else if (modori == "o") {
			output = "original ";
		}
		output += "key signature";
		if (!standardQ) {
			output += " (<span class='unknown'>non-standard</span>)";
		}
		output += ": ";
		int flats = 0;
		int sharps = 0;
		int naturals = 0;
		int doubleflats = 0;
		int doublesharps = 0;
		vector<string> accidentals;
		hre.split(accidentals, pcs, "[a-gA-G]");
		for (int i=0; i<(int)accidentals.size(); i++) {
			if (accidentals[i] == "##") {
				doublesharps++;
			} else if (accidentals[i] == "--") {
				doubleflats++;
			} else if (accidentals[i] == "#") {
				sharps++;
			} else if (accidentals[i] == "-") {
				flats++;
			} else if (accidentals[i] == "n") {
				naturals++;
			}
		}

		bool foundQ = false;
		if (sharps) {
			if (foundQ) {
				output += ", ";
			}
			foundQ = true;
			if (sharps == 1) {
				output += "1 sharp";
			} else {
				output += to_string(sharps) + " sharps";
			}
		}

		if (flats) {
			if (foundQ) {
				output += ", ";
			}
			foundQ = true;
			if (flats == 1) {
				output += "1 flat";
			} else {
				output += to_string(flats) + " flats";
			}
		}

		if (naturals) {
			if (foundQ) {
				output += ", ";
			}
			foundQ = true;
			if (naturals == 1) {
				output += "1 natural";
			} else {
				output += to_string(naturals) + " naturals";
			}
		}

		if (doublesharps) {
			if (foundQ) {
				output += ", ";
			}
			foundQ = true;
			if (doublesharps == 1) {
				output += "1 double sharp";
			} else {
				output += to_string(doublesharps) + " double sharps";
			}
		}

		if (doubleflats) {
			if (foundQ) {
				output += ", ";
			}
			foundQ = true;
			if (doubleflats == 1) {
				output += "1 double flat";
			} else {
				output += to_string(doubleflats) + " double flats";
			}
		}

		return output;
	}
	return m_unknown;
}



//////////////////////////////
//
// Tool_tandeminfo::checkForKeyDesignation -- Standard Humdrum Toolkit interpretations, plus
//     modal extensions by Brett Arden.  Typically only used in **kern data.
//

string Tool_tandeminfo::checkForKeyDesignation(const string& tok) {
	HumRegex hre;
	if (tok == "?:") {
		return "key designation, unknown/unassigned key";
	}
	if (hre.search(tok, "^([a-gA-G])([-#]*):(ion|dor|phr|lyd|mix|aeo|loc)?$")) {
		string tonic = hre.getMatch(1);
		string accid = hre.getMatch(2);
		string mode  = hre.getMatch(3);
		bool isUpper = isupper(tonic[0]);
		string output = "key designation: ";
		if (mode.empty()) {
			output += toupper(tonic[0]);

			if (accid == "") {
				// do nothing
			} else if (accid == "#") {
				output += "-sharp";
			} else if (accid == "-") {
				output += "-flat";
			} else if (accid == "##") {
				output += "-double-sharp";
			} else if (accid == "--") {
				output += "-double-flat";
			} else if (accid == "###") {
				output += "-triple-sharp";
			} else if (accid == "---") {
				output += "-triple-flat";
			} else {
				return m_unknown;
			}

			if (isUpper) {
				output += " major";
			} else {
				output += " minor";
			}
			return output;
		} else {
			// Modal key
			if (isUpper && ((mode == "dor") || (mode == "phr") || (mode == "aeo") || (mode == "loc"))) {
				// need a lower-case letter for these modes (minor third above tonic)
				return m_unknown;
			}
			if ((!isUpper) && ((mode == "ion") || (mode == "lyd") || (mode == "mix"))) {
				// need an upper-case letter for these modes (major third above tonic)
				return m_unknown;
			}
			output += toupper(tonic[0]);

			if (accid == "") {
				// do nothing
			} else if (accid == "#") {
				output += "-sharp";
			} else if (accid == "-") {
				output += "-flat";
			} else if (accid == "##") {
				output += "-double-sharp";
			} else if (accid == "--") {
				output += "-double-flat";
			} else if (accid == "###") {
				output += "-triple-sharp";
			} else if (accid == "---") {
				output += "-triple-flat";
			} else {
				return m_unknown;
			}

			if (mode == "ion") {
				output += " ionian";
			} else if (mode == "dor") {
				output += " dorian";
			} else if (mode == "phr") {
				output += " phrygian";
			} else if (mode == "lyd") {
				output += " lydian";
			} else if (mode == "mix") {
				output += " mixolydian";
			} else if (mode == "aeo") {
				output += " aeolian";
			} else if (mode == "loc") {
				output += " locrian";
			} else {
				return m_unknown;
			}
			return output;
		}
	}

	return m_unknown;
}


// END_MERGE

} // end namespace hum



