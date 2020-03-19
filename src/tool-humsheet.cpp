//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Wed Feb 26 09:49:14 PST 2020
// Last Modified: Wed Feb 26 09:49:17 PST 2020
// Filename:      tool-humsheet.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/tool-humsheet.cpp
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Switch between humsheet encoding and corrected encoding.
//

#include "tool-humsheet.h"
#include "Convert.h"

using namespace std;

namespace hum {

// START_MERGE


/////////////////////////////////
//
// Tool_humsheet::Tool_humsheet -- Set the recognized options for the tool.
//

Tool_humsheet::Tool_humsheet(void) {
	define("h|H|html|HTML=b", "output table in HTML wrapper");
	define("i|id|ID=b", "include ID for each cell");
	define("z|zebra=b", "add zebra striping by spine to style");
	define("y|z2|zebra2|zebra-2=b", "zebra striping by data type");
	define("t|tab-index=b", "vertical tab indexing");
	define("X|no-exinterp=b", "do not embed exclusive interp data");
	define("J|no-javascript=b", "do not embed javascript code");
	define("S|no-style=b", "do not embed CSS style element");
}



/////////////////////////////////
//
// Tool_humsheet::run -- Do the main work of the tool.
//

bool Tool_humsheet::run(HumdrumFileSet& infiles) {
	bool status = true;
	for (int i=0; i<infiles.getCount(); i++) {
		status &= run(infiles[i]);
	}
	return status;
}


bool Tool_humsheet::run(const string& indata, ostream& out) {
	HumdrumFile infile(indata);
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_humsheet::run(HumdrumFile& infile, ostream& out) {
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_humsheet::run(HumdrumFile& infile) {
	initialize();
	processFile(infile);
	return true;
}



//////////////////////////////
//
// Tool_humsheet::initialize --  Initializations that only have to be done once
//    for all HumdrumFile segments.
//

void Tool_humsheet::initialize(void) {
	m_idQ         = getBoolean("id");
	m_htmlQ       = getBoolean("html");
	m_zebraQ      = getBoolean("zebra");
	m_zebra2Q     = getBoolean("zebra2");
	m_exinterpQ   = !getBoolean("no-exinterp");
	m_javascriptQ = !getBoolean("no-javascript");
	m_tabindexQ   = getBoolean("tab-index");
}



//////////////////////////////
//
// Tool_humsheet::processFile --
//

void Tool_humsheet::processFile(HumdrumFile& infile) {
	analyzeTracks(infile);
	if (m_htmlQ) {
		printHtmlHeader();
		printStyle(infile);
	}
	if (m_tabindexQ) {
		analyzeTabIndex(infile);
	}
	m_free_text << "<table class=\"humdrum\"";
	m_free_text << " data-spine-count=\"" << infile.getMaxTrack() << "\"";
	m_free_text << ">\n";
	for (int i=0; i<infile.getLineCount(); i++) {
		m_free_text << "<tr";
		printRowClasses(infile, i);
		printRowData(infile, i);
		printTitle(infile, i);
		m_free_text << ">";
		printRowContents(infile, i);
		m_free_text << "</tr>\n";
	}
	m_free_text << "</table>";
	if (m_htmlQ) {
		if (m_javascriptQ) {
			printJavascript();
		}
		printHtmlFooter();
	}
}



//////////////////////////////
//
// Tool_humsheet::printTitle --
//

void Tool_humsheet::printTitle(HumdrumFile& infile, int line) {
	if (!infile[line].isReference()) {
		return;
	}
	string meaning = Convert::getReferenceKeyMeaning(infile[line].token(0));
	if (!meaning.empty()) {
		m_free_text << " title=\"" << meaning << "\"";
	}
}



//////////////////////////////
//
// Tool_humsheet::printRowData --
//

void Tool_humsheet::printRowData(HumdrumFile& infile, int line) {
	m_free_text << " data-line=\"" << line << "\"";
}



///////////////////////////////
//
// printHtmlHeader --
//

void Tool_humsheet::printHtmlHeader(void) {
	m_free_text << "<html>\n";
	m_free_text << "<head>\n";
	m_free_text << "<title>\n";
	m_free_text << "UNTITLED\n";
	m_free_text << "</title>\n";
	m_free_text << "</head>\n";
	m_free_text << "<body>\n";
}



///////////////////////////////
//
// printHtmlFooter --
//

void Tool_humsheet::printHtmlFooter(void) {
	m_free_text << "</body>\n";
	m_free_text << "</html>\n";
}



///////////////////////////////
//
// printRowClasses --
//

void Tool_humsheet::printRowClasses(HumdrumFile& infile, int row) {
	string classes;
	HLp hl = &infile[row];
	if (hl->hasSpines()) {
		classes += "spined ";
	}
	if (hl->isEmpty()) {
		classes += "empty ";
	}
	if (hl->isData()) {
		classes += "data ";
	}
	if (hl->isInterpretation()) {
		classes += "interp ";
		HTp token = hl->token(0);
		if (token->compare(0, 2, "*>") == 0) {
			classes += "label ";
		}
	}
	if (hl->isLocalComment()) {
		classes += "lcomment ";
		if (isLayout(hl)) {
			classes += "layout ";
		}
	}
	HTp token = hl->token(0);
	if (token->compare(0, 2, "!!") == 0) {
		if ((token->size() == 2) || (token->at(3) != '!')) {
			classes += "gcommet ";
		}
	}

	if (hl->isUniversalReference()) {
		if (token->compare(0, 11, "!!!!filter:") == 0) {
			classes += "ufilter ";
		} else if (token->compare(0, 12, "!!!!Xfilter:") == 0) {
			classes += "usedufilter ";
		} else {
			classes += "ureference ";
			if (token->compare(0, 12, "!!!!SEGMENT:") == 0) {
				classes += "segment ";
			}
		}
	} else if (hl->isCommentUniversal()) {
		classes += "ucomment ";
	} else if (hl->isReference()) {
		classes += "reference ";
	} else if (hl->isGlobalComment()) {
		HTp token = hl->token(0);
		if (token->compare(0, 10, "!!!filter:") == 0) {
			classes += "filter ";
		} else if (token->compare(0, 11, "!!!Xfilter:") == 0) {
			classes += "usedfilter ";
		} else {
			classes += "gcomment ";
			if (isLayout(hl)) {
				classes += "layout ";
			}
		}
	}

	if (hl->isBarline()) {
		classes += "barline ";
	}
	if (hl->isManipulator()) {
		HTp token = hl->token(0);
		if (token->compare(0, 2, "**") == 0) {
			classes += "exinterp ";
		} else {
			classes += "manip ";
		}
	}
	if (!classes.empty()) {
      // remove space.
		classes.resize((int)classes.size() - 1);
		m_free_text << " class=\"" << classes << "\"";
	}
}



//////////////////////////////
//
// Tool_humsheet::isLayout -- check to see if any cell 
//    starts with "!LO:".
//

bool Tool_humsheet::isLayout(HLp line) {
	if (line->hasSpines()) {
		if (!line->isCommentLocal()) {
			return false;
		}
		for (int i=0; i<line->getFieldCount(); i++) {
			HTp token = line->token(i);
			if (token->compare(0, 4, "!LO:") == 0) {
				return true;
			}
		}
	} else {
		HTp token = line->token(0);
		if (token->compare(0, 5, "!!LO:") == 0) {
			return true;
		}
	}
	return false;
}



///////////////////////////////
//
// Tool_humsheet::printRowContents --
//

void Tool_humsheet::printRowContents(HumdrumFile& infile, int row) {
	int fieldcount = infile[row].getFieldCount();
	for (int i=0; i<fieldcount; i++) {
		HTp token = infile.token(row, i);
		m_free_text << "<td";
		if (m_idQ) {
			printId(token);
		}
		printCellClasses(token);
		printCellData(token);
		if (m_tabindexQ) {
			printTabIndex(token);
		}
		printColSpan(token);
		if (!infile[row].isManipulator()) {
			// do not allow manipulators to be edited
			m_free_text << " contenteditable=\"true\"";
		} else if (infile[row].isExclusive()) {
			// but allow exclusive interpretation to be edited
			m_free_text << " contenteditable=\"true\"";
		}
		m_free_text << ">";
		printToken(token);
		m_free_text << "</td>";
	}
}



//////////////////////////////
//
// Tool_humsheet::printCellData --
//

void Tool_humsheet::printCellData(HTp token) {
	int field = token->getFieldIndex();
	m_free_text << " data-field=\"" << field << "\"";


	if (token->getOwner()->hasSpines()) {
		int spine = token->getTrack() - 1;
		m_free_text << " data-spine=\"" << spine << "\"";

		int subspine = token->getSubtrack();
		if (subspine > 0) {
			m_free_text << " data-subspine=\"" << subspine << "\"";
		}

		string exinterp = token->getDataType().substr(2);
		if (m_exinterpQ && !exinterp.empty()) {
			m_free_text << " data-x=\"" << exinterp << "\"";
		}
	}
}



//////////////////////////////
//
// Tool_humsheet::printToken --
//

void Tool_humsheet::printToken(HTp token) {
	for (int i=0; i<(int)token->size(); i++) {
		switch (token->at(i)) {
			case '>':
				m_free_text << "&gt;";
				break;
			case '<':
				m_free_text << "&lt;";
				break;
			default:
				m_free_text << token->at(i);
		}
	}
}



///////////////////////////////
//
// Tool_humsheet::printId --
//

void Tool_humsheet::printId(HTp token) {
	int line = token->getLineNumber();
	int field = token->getFieldNumber();
	string id = "tok-L";
	id += to_string(line);
	id += "F";
	id += to_string(field);
	m_free_text << " id=\"" << id << "\"";
}



///////////////////////////////
//
// Tool_humsheet::printTabIndex --
//

void Tool_humsheet::printTabIndex(HTp token) {
	string number = token->getValue("auto", "tabindex");
	if (number.empty()) {
		return;
	}
	m_free_text << " tabindex=\"" << number << "\"";
}



//////////////////////////////
//
// Tool_humsheet::printColspan -- print any necessary colspan values for
//    token (to align by primary spines)
//

void Tool_humsheet::printColSpan(HTp token) {
	if (!token->getOwner()->hasSpines()) {
		m_free_text << " colspan=\"" << m_max_field << "\"";
		return;
	}
	int track = token->getTrack() - 1;
	int scount = m_max_subtrack.at(track);
	int subtrack = token->getSubtrack();
	if (subtrack > 1) {
		subtrack--;
	}
	HTp nexttok = token->getNextFieldToken();
	int ntrack = -1;
	if (nexttok) {
		ntrack = nexttok->getTrack() - 1;
	}
	if ((ntrack < 0) || (ntrack != track)) {
		// at the end of a primary spine, so do a colspan with the remaining subtracks
		if (subtrack < scount-1) {
			int colspan = scount - subtrack;
			m_free_text << " colspan=\"" << colspan << "\"";
		}
	} else {
		// do nothing
	}
}



///////////////////////////////
//
// printCellClasses --
//

void Tool_humsheet::printCellClasses(HTp token) {
	int track = token->getTrack();
	string classlist;

	if (m_zebraQ) {
		if (track % 2 == 0) {
			classlist = "zebra ";
		}
	}

	if (token->getOwner()->hasSpines()) {
		int length = (int)token->size();
		if (length > 20) {
			classlist += "long ";
		}
	}

	if (!classlist.empty()) {
		classlist.resize((int)classlist.size() - 1);
		m_free_text << " class=\"" << classlist << "\"";
	}

}



//////////////////////////////
//
// Tool_humsheet::printStyle --
//

void Tool_humsheet::printStyle(HumdrumFile& infile) {

	m_free_text << "<style>\n";
	m_free_text << "body {\n";
	m_free_text << "	padding: 20px;\n";
	m_free_text << "}\n";
	m_free_text << "table.humdrum {\n";
	m_free_text << "	border-collapse: collapse;\n";
	m_free_text << "}\n";
	m_free_text << "table.humdrum td:focus {\n";
	m_free_text << "	background: #ff000033 !important;\n";
	m_free_text << "}\n";
	m_free_text << "table.humdrum td {\n";
	m_free_text << "	outline: none;\n";
	m_free_text << "}\n";
	m_free_text << "table.humdrum td[data-subspine='1'],\n";
	m_free_text << "table.humdrum td[data-subspine='2'],\n";
	m_free_text << "table.humdrum td[data-subspine='3'],\n";
	m_free_text << "table.humdrum td[data-subspine='4'],\n";
	m_free_text << "table.humdrum td[data-subspine='5'],\n";
	m_free_text << "table.humdrum td[data-subspine='6'],\n";
	m_free_text << "table.humdrum td[data-subspine='7'],\n";
	m_free_text << "table.humdrum td[data-subspine='8'],\n";
	m_free_text << "table.humdrum td[data-subspine='9'] {\n";
	m_free_text << "	border-right: solid #0000000A 1px;\n";
	m_free_text << "	padding-left: 3px;\n";
	m_free_text << "}\n";
	m_free_text << "table.humdrum tr.ucomment {\n";
	m_free_text << "	color: chocolate;\n";
	m_free_text << "}\n";
	m_free_text << "table.humdrum tr.segment {\n";
	m_free_text << "	color: chocolate;\n";
	m_free_text << "	background: rgb(255,99,71,0.25);\n";
	m_free_text << "}\n";
	m_free_text << "table.humdrum tr.ureference {\n";
	m_free_text << "	color: chocolate;\n";
	m_free_text << "}\n";
	m_free_text << "table.humdrum tr.reference {\n";
	m_free_text << "	color: green;\n";
	m_free_text << "}\n";
	m_free_text << "table.humdrum tr.gcomment {\n";
	m_free_text << "	color: blue;\n";
	m_free_text << "}\n";
	m_free_text << "table.humdrum tr.ucomment {\n";
	m_free_text << "	color: violet;\n";
	m_free_text << "}\n";
	m_free_text << "table.humdrum tr.lcomment {\n";
	m_free_text << "	color: $#2fc584;\n";
	m_free_text << "}\n";
	m_free_text << "table.humdrum tr.interp.manip {\n";
	m_free_text << "	color: magenta;\n";
	m_free_text << "}\n";
	m_free_text << "table.humdrum tr.interp.exinterp {\n";
	m_free_text << "	color: red;\n";
	m_free_text << "}\n";
	m_free_text << "table.humdrum tr.interp {\n";
	m_free_text << "	color: darkviolet;\n";
	m_free_text << "}\n";
	m_free_text << "table.humdrum tr.filter {\n";
	m_free_text << "	color: limegreen;\n";
	m_free_text << "}\n";
	m_free_text << "table.humdrum tr.usedfilter {\n";
	m_free_text << "	color: olive;\n";
	m_free_text << "}\n";
	m_free_text << "table.humdrum tr.ufilter {\n";
	m_free_text << "	color: limegreen;\n";
	m_free_text << "	background: rgba(0,0,aa,0.3);\n";
	m_free_text << "}\n";
	m_free_text << "table.humdrum tr.usedufilter {\n";
	m_free_text << "	color: olive;\n";
	m_free_text << "	background: rgba(0,0,aa,0.3);\n";
	m_free_text << "}\n";
	m_free_text << "table.humdrum tr.interp.label {\n";
	m_free_text << "	background: rgba(75,0,130,0.3);\n";
	m_free_text << "}\n";
	m_free_text << "table.humdrum tr.layout {\n";
	m_free_text << "	color: orange;\n";
	m_free_text << "}\n";
	m_free_text << "table.humdrum tr.barline {\n";
	m_free_text << "	color: gray;\n";
	m_free_text << "	background: rgba(0, 0, 0, 0.06);\n";
	m_free_text << "}\n";
	m_free_text << "table.humdrum td.long {\n";
	m_free_text << "	white-space: nowrap;\n";
	m_free_text << "	max-width: 200px;\n";
	m_free_text << "	background-image: linear-gradient(to right, cornsilk 95%, crimson 100%);\n";
	m_free_text << "	overflow: scroll;\n";
	m_free_text << "}\n";

	if (m_zebraQ) {
		m_free_text << "table.humdrum .zebra {\n";
		m_free_text << "	background: #ccccff33;\n";
		m_free_text << "}\n";
	} else if (m_zebra2Q) {
		m_free_text << "table.humdrum td[data-x='kern'] {\n";
		m_free_text << "	background: #ffcccc33;\n";
		m_free_text << "}\n";
		m_free_text << "table.humdrum td[data-x='dynam'] {\n";
		m_free_text << "	background: #ccccff33;\n";
		m_free_text << "}\n";
		m_free_text << "table.humdrum td[data-x='text'] {\n";
		m_free_text << "	background: #ccffcc33;\n";
		m_free_text << "}\n";
	}

	m_free_text << "</style>\n";
}



//////////////////////////////
//
// Tool_humsheet::printJavascript --
//

void Tool_humsheet::printJavascript(void) {

	m_free_text << "<script>\n";
	m_free_text << "\n";
	m_free_text << "var AKey      = 65;\n";
	m_free_text << "var BKey      = 66;\n";
	m_free_text << "var CKey      = 67;\n";
	m_free_text << "var DKey      = 68;\n";
	m_free_text << "var EKey      = 69;\n";
	m_free_text << "var FKey      = 70;\n";
	m_free_text << "var GKey      = 71;\n";
	m_free_text << "var HKey      = 72;\n";
	m_free_text << "var IKey      = 73;\n";
	m_free_text << "var JKey      = 74;\n";
	m_free_text << "var KKey      = 75;\n";
	m_free_text << "var LKey      = 76;\n";
	m_free_text << "var MKey      = 77;\n";
	m_free_text << "var NKey      = 78;\n";
	m_free_text << "var OKey      = 79;\n";
	m_free_text << "var PKey      = 80;\n";
	m_free_text << "var QKey      = 81;\n";
	m_free_text << "var RKey      = 82;\n";
	m_free_text << "var SKey      = 83;\n";
	m_free_text << "var TKey      = 84;\n";
	m_free_text << "var UKey      = 85;\n";
	m_free_text << "var VKey      = 86;\n";
	m_free_text << "var WKey      = 87;\n";
	m_free_text << "var XKey      = 88;\n";
	m_free_text << "var YKey      = 89;\n";
	m_free_text << "var ZKey      = 90;\n";
	m_free_text << "var ZeroKey   = 48;\n";
	m_free_text << "var OneKey    = 49;\n";
	m_free_text << "var TwoKey    = 50;\n";
	m_free_text << "var ThreeKey  = 51;\n";
	m_free_text << "var FourKey   = 52;\n";
	m_free_text << "var FiveKey   = 53;\n";
	m_free_text << "var SixKey    = 54;\n";
	m_free_text << "var SevenKey  = 55;\n";
	m_free_text << "var EightKey  = 56;\n";
	m_free_text << "var NineKey   = 57;\n";
	m_free_text << "var PgUpKey   = 33;\n";
	m_free_text << "var PgDnKey   = 34;\n";
	m_free_text << "var EndKey    = 35;\n";
	m_free_text << "var HomeKey   = 36;\n";
	m_free_text << "var LeftKey   = 37;\n";
	m_free_text << "var UpKey     = 38;\n";
	m_free_text << "var RightKey  = 39;\n";
	m_free_text << "var DownKey   = 40;\n";
	m_free_text << "var EnterKey  = 13;\n";
	m_free_text << "var SpaceKey  = 32;\n";
	m_free_text << "var SlashKey  = 191;\n";
	m_free_text << "var EscKey    = 27;\n";
	m_free_text << "var BackKey   = 8;\n";
	m_free_text << "var CommaKey  = 188;\n";
	m_free_text << "var MinusKey  = 189;\n";
	m_free_text << "var DotKey    = 190;\n";
	m_free_text << "var SemiColonKey = 186;\n";
	m_free_text << "var BackQuoteKey   = 192;\n";
	m_free_text << "var SingleQuoteKey = 222;\n";
	m_free_text << "\n";
	m_free_text << "var TARGET_SPINE    = 0;\n";
	m_free_text << "var TARGET_SUBSPINE = 0;\n";
	m_free_text << "\n";
	m_free_text << "window.addEventListener('keydown', processKey, true);\n";
	m_free_text << "\n";
	m_free_text << "function processKey(event) {\n";
	m_free_text << "	var target;\n";
	m_free_text << "	var spine;\n";
	m_free_text << "	var subspine;\n";
	m_free_text << "	var rent;\n";
	m_free_text << "	var nextline;\n";
	m_free_text << "	var line;\n";
	m_free_text << "	var nexttr;\n";
	m_free_text << "	var newtd;\n";
	m_free_text << "\n";
	m_free_text << "	if (!event.preventDefault) {\n";
	m_free_text << "		event.preventDefault = function() { };\n";
	m_free_text << "	}\n";
	m_free_text << "\n";
	m_free_text << "	if (event.metaKey) {\n";
	m_free_text << "		return;\n";
	m_free_text << "	}\n";
	m_free_text << "\n";
	m_free_text << "	switch (event.keyCode) {\n";
	m_free_text << "\n";
	m_free_text << "		case EnterKey: // Move to next lower row in same spine\n";
	m_free_text << "			if (event.shiftKey) {\n";
	m_free_text << "				moveLine(event.target, -1);\n";
	m_free_text << "			} else {\n";
	m_free_text << "				moveLine(event.target, +1);\n";
	m_free_text << "			}\n";
	m_free_text << "			event.preventDefault();\n";
	m_free_text << "			break;\n";
	m_free_text << "\n";
	m_free_text << "		case DownKey:  // Move to next lower row in same spine\n";
	m_free_text << "			moveLine(event.target, +1);\n";
	m_free_text << "			event.preventDefault();\n";
	m_free_text << "			break;\n";
	m_free_text << "\n";
	m_free_text << "		case UpKey:\n";
	m_free_text << "			moveLine(event.target, -1);\n";
	m_free_text << "			event.preventDefault();\n";
	m_free_text << "			break;\n";
	m_free_text << "\n";
	m_free_text << "		case RightKey:\n";
	m_free_text << "			if (event.shiftKey) {\n";
	m_free_text << "				moveField(event.target, +1);\n";
	m_free_text << "				event.preventDefault();\n";
	m_free_text << "			}\n";
	m_free_text << "			 break;\n";
	m_free_text << "\n";
	m_free_text << "		case LeftKey:\n";
	m_free_text << "			if (event.shiftKey) {\n";
	m_free_text << "				moveField(event.target, -1);\n";
	m_free_text << "				event.preventDefault();\n";
	m_free_text << "			}\n";
	m_free_text << "			break;\n";
	m_free_text << "\n";
	m_free_text << "	}\n";
	m_free_text << "}\n";
	m_free_text << "\n";
	m_free_text << "\n";
	m_free_text << "//////////////////////////////\n";
	m_free_text << "//\n";
	m_free_text << "// moveField -- Move the to the next or previous td on a row.\n";
	m_free_text << "//\n";
	m_free_text << "\n";
	m_free_text << "function moveField(target, direction) {\n";
	m_free_text << "	if (target.nodeName !== 'TD') {\n";
	m_free_text << "		console.log('TARGET IS NOT TD');\n";
	m_free_text << "		return;\n";
	m_free_text << "	}\n";
	m_free_text << "	rent = target.parentNode;\n";
	m_free_text << "	if (rent.nodeName !== 'TR') {\n";
	m_free_text << "		console.log('PARENT IS NOT TR');\n";
	m_free_text << "		return;\n";
	m_free_text << "	}\n";
	m_free_text << "	var tds = rent.querySelectorAll('TD');\n";
	m_free_text << "	for (var i=0; i<tds.length; i++) {\n";
	m_free_text << "		if (target !== tds[i]) {\n";
	m_free_text << "			console.log('TARGET', target, 'NOT THE SAME AS', tds[i]);\n";
	m_free_text << "			continue;\n";
	m_free_text << "		}\n";
	m_free_text << "		var newi = i + direction;\n";
	m_free_text << "		if (newi < 0) {\n";
	m_free_text << "			// Don't do anything since there are no more\n";
	m_free_text << "			// cells to the left of the current cell.\n";
	m_free_text << "			return;\n";
	m_free_text << "		}\n";
	m_free_text << "		if (newi >= tds.length) {\n";
	m_free_text << "			// Don't do anything since there are no more\n";
	m_free_text << "			// cells to the right of the current cell.\n";
	m_free_text << "			return;\n";
	m_free_text << "		}\n";
	m_free_text << "		var newtd = tds[newi];\n";
	m_free_text << "		newtd.focus();\n";
	m_free_text << "		return;\n";
	m_free_text << "	}\n";
	m_free_text << "}\n";
	m_free_text << "\n";
	m_free_text << "\n";
	m_free_text << "\n";
	m_free_text << "//////////////////////////////\n";
	m_free_text << "//\n";
	m_free_text << "// moveLine --\n";
	m_free_text << "//\n";
	m_free_text << "\n";
	m_free_text << "function moveLine(target, direction) {\n";
	m_free_text << "	if (target.nodeName !== 'TD') {\n";
	m_free_text << "		console.log('TARGET IS NOT TD');\n";
	m_free_text << "		return;\n";
	m_free_text << "	}\n";
	m_free_text << "	spine = parseInt(target.dataset.spine || -1);\n";
	m_free_text << "	subspine = parseInt(target.dataset.subspine || -1);\n";
	m_free_text << "	rent = target.parentNode;\n";
	m_free_text << "	if (rent.nodeName !== 'TR') {\n";
	m_free_text << "		console.log('PARENT IS NOT TR');\n";
	m_free_text << "		return;\n";
	m_free_text << "	}\n";
	m_free_text << "	line = parseInt(rent.dataset.line || -1);\n";
	m_free_text << "	nextline = line + direction;\n";
	m_free_text << "\n";
	m_free_text << "	nexttr = document.querySelector('tr[data-line=\"' + nextline + '\"]');\n";
	m_free_text << "\n";
	m_free_text << "	if (nexttr && nexttr.className.match(/\\bmanip\\b/)) {\n";
	m_free_text << "		nextline = line + direction * 2;\n";
	m_free_text << "		nexttr = document.querySelector('tr[data-line=\"' + nextline + '\"]');\n";
	m_free_text << "	}\n";
	m_free_text << "	if (nexttr && nexttr.className.match(/\\bmanip\\b/)) {\n";
	m_free_text << "		nextline = line + direction * 3;\n";
	m_free_text << "		nexttr = document.querySelector('tr[data-line=\"' + nextline + '\"]');\n";
	m_free_text << "	}\n";
	m_free_text << "	if (nexttr && nexttr.className.match(/\\bmanip\\b/)) {\n";
	m_free_text << "		nextline = line + direction * 4;\n";
	m_free_text << "		nexttr = document.querySelector('tr[data-line=\"' + nextline + '\"]');\n";
	m_free_text << "	}\n";
	m_free_text << "\n";
	m_free_text << "	if (!nexttr) {\n";
	m_free_text << "		// nexttr does not exist so do nothing\n";
	m_free_text << "		return;\n";
	m_free_text << "	}\n";
	m_free_text << "	newtd = getNewTd(nexttr, spine, subspine);\n";
	m_free_text << "	if (!newtd) {\n";
	m_free_text << "		console.log('CANNOT FIND NEW TD');\n";
	m_free_text << "		return;\n";
	m_free_text << "	}\n";
	m_free_text << "	newtd.focus();\n";
	m_free_text << "}\n";
	m_free_text << "\n";
	m_free_text << "\n";
	m_free_text << "\n";
	m_free_text << "//////////////////////////////\n";
	m_free_text << "//\n";
	m_free_text << "// getNewTd -- Find the td element with the matching spine and subspine\n";
	m_free_text << "//        numbers as the starting cell.  If they do not match, then find\n";
	m_free_text << "//        one that has a matching spine.  If that cannot be found, then\n";
	m_free_text << "//        store the spine and subspine for recovering the spine position\n";
	m_free_text << "//        after passing through a global comment.\n";
	m_free_text << "//\n";
	m_free_text << "\n";
	m_free_text << "function getNewTd(tr, spine, subspine) {\n";
	m_free_text << "	if (spine < 0) {\n";
	m_free_text << "		spine = TARGET_SPINE;\n";
	m_free_text << "		subspine = TARGET_SUBSPINE;\n";
	m_free_text << "	}\n";
	m_free_text << "	var tds = tr.querySelectorAll('td');\n";
	m_free_text << "	if (tds.length == 1) {\n";
	m_free_text << "		return tds[0];\n";
	m_free_text << "	} else if (tds.length == 0) {\n";
	m_free_text << "		console.log('DID NOT FIND ANY TDS');\n";
	m_free_text << "		return null;\n";
	m_free_text << "	}\n";
	m_free_text << "	var list = [];\n";
	m_free_text << "	var obj;\n";
	m_free_text << "	var i;\n";
	m_free_text << "	for (i=0; i<tds.length; i++) {\n";
	m_free_text << "		obj = {};\n";
	m_free_text << "		obj.spine = parseInt(tds[i].dataset.spine);\n";
	m_free_text << "		obj.subspine = parseInt(tds[i].dataset.subspine || -1);\n";
	m_free_text << "		obj.td = tds[i];\n";
	m_free_text << "		list.push(obj);\n";
	m_free_text << "	}\n";
	m_free_text << "	for (i=0; i<list.length; i++) {\n";
	m_free_text << "		if (list[i].spine != spine) {\n";
	m_free_text << "			continue;\n";
	m_free_text << "		}\n";
	m_free_text << "		if (list[i].subspine != subspine) {\n";
	m_free_text << "			continue;\n";
	m_free_text << "		}\n";
	m_free_text << "		return list[i].td;\n";
	m_free_text << "	}\n";
	m_free_text << "	// did not find the exact spine/subspine, so go to the first \n";
	m_free_text << "	// spine that matches (backwards if subspine is not 0).\n";
	m_free_text << "	if (subspine < 0) {\n";
	m_free_text << "		for (i=0; i<list.length - 1; i++) {\n";
	m_free_text << "			if (list[i].spine != spine) {\n";
	m_free_text << "				continue;\n";
	m_free_text << "			}\n";
	m_free_text << "			return list[i].td;\n";
	m_free_text << "		}\n";
	m_free_text << "	} else {\n";
	m_free_text << "		for (i=list.length - 1; i>=0; i--) {\n";
	m_free_text << "			if (list[i].spine != spine) {\n";
	m_free_text << "				continue;\n";
	m_free_text << "			}\n";
	m_free_text << "			return list[i].td;\n";
	m_free_text << "		}\n";
	m_free_text << "	}\n";
	m_free_text << "	if (list.length == 1) {\n";
	m_free_text << "		return list[0].td;\n";
	m_free_text << "	}\n";
	m_free_text << "	console.log('DID NOT FIND NEW TD FOR', spine, subspine);\n";
	m_free_text << "	return null;\n";
	m_free_text << "}\n";
	m_free_text << "\n";
	m_free_text << "\n";
	m_free_text << "\n";
	m_free_text << "//////////////////////////////\n";
	m_free_text << "//\n";
	m_free_text << "// focusin eventListener -- When a cell is focused on, and the cell\n";
	m_free_text << "//     is spined, then store its spine/subspine values in TARGET_SPINE\n";
	m_free_text << "//     and TARGET_SUBSPINE for navigating through global records.\n";
	m_free_text << "//\n";
	m_free_text << "\n";
	m_free_text << "document.addEventListener('focusin', function(event) {\n";
	m_free_text << "	var target = event.target;\n";
	m_free_text << "	if (target.nodeName !== 'TD') {\n";
	m_free_text << "		return;\n";
	m_free_text << "	}\n";
	m_free_text << "	moveCursorToEndOfText(target);\n";
	m_free_text << "\n";
	m_free_text << "	var tr = target.parentNode;\n";
	m_free_text << "	if (!tr) {\n";
	m_free_text << "		return;\n";
	m_free_text << "	}\n";
	m_free_text << "	if (tr.nodeName !== 'TR') {\n";
	m_free_text << "		return;\n";
	m_free_text << "	}\n";
	m_free_text << "	if (!tr.className.match(/\\bspined\\b/)) {\n";
	m_free_text << "		return;\n";
	m_free_text << "	}\n";
	m_free_text << "	var spine = parseInt(target.dataset.spine || -1);\n";
	m_free_text << "	var subspine = parseInt(target.dataset.subspine || -1);\n";
	m_free_text << "	TARGET_SPINE = spine;\n";
	m_free_text << "	TARGET_SUBSPINE = subspine;\n";
	m_free_text << "});\n";
	m_free_text << "\n";
	m_free_text << "\n";
	m_free_text << "\n";
	m_free_text << "//////////////////////////////\n";
	m_free_text << "//\n";
	m_free_text << "// focusout eventListener -- When leaving a cell, check that\n";
	m_free_text << "//    its contents are syntactically correct.\n";
	m_free_text << "//\n";
	m_free_text << "\n";
	m_free_text << "document.addEventListener('focusout', function(event) {\n";
	m_free_text << "	var target = event.target;\n";
	m_free_text << "	if (target.nodeName !== 'TD') {\n";
	m_free_text << "		return;\n";
	m_free_text << "	}\n";
	m_free_text << "\n";
	m_free_text << "	var tr = target.parentNode;\n";
	m_free_text << "	if (!tr) {\n";
	m_free_text << "		return;\n";
	m_free_text << "	}\n";
	m_free_text << "	if (tr.nodeName !== 'TR') {\n";
	m_free_text << "		return;\n";
	m_free_text << "	}\n";
	m_free_text << "	var empty = '.';\n";
	m_free_text << "	var classes = tr.className;\n";
	m_free_text << "	if (classes.match(/\\bmanip\\b/)) {\n";
	m_free_text << "		empty = '*';\n";
	m_free_text << "	} if (classes.match(/\\binterp\\b/)) {\n";
	m_free_text << "		empty = '*';\n";
	m_free_text << "	} else if (classes.match(/\\blcomment\\b/)) {\n";
	m_free_text << "		empty = '!';\n";
	m_free_text << "	} else if (classes.match(/comment\\b/)) {\n";
	m_free_text << "		empty = '!!';\n";
	m_free_text << "	} else if (classes.match(/reference\\b/)) {\n";
	m_free_text << "		empty = '!';\n";
	m_free_text << "	} else if (classes.match(/\\bbarline\\b/)) {\n";
	m_free_text << "		empty = '=';\n";
	m_free_text << "	}\n";
	m_free_text << "	\n";
	m_free_text << "	var contents = target.textContent;\n";
	m_free_text << "	if (contents === '') {\n";
	m_free_text << "		target.textContent = empty;\n";
	m_free_text << "		return;\n";
	m_free_text << "	}\n";
	m_free_text << "	\n";
	m_free_text << "	var firstchar = contents.charAt(0);\n";
	m_free_text << "	if ((empty === '!') && (firstchar !== '!')) {\n";
	m_free_text << "		target.textContent = '!' + contents;\n";
	m_free_text << "		return;\n";
	m_free_text << "	}\n";
	m_free_text << "	else if ((empty === '*') && (firstchar !== '*')) {\n";
	m_free_text << "		target.textContent = '*' + contents;\n";
	m_free_text << "		return;\n";
	m_free_text << "	}\n";
	m_free_text << "	else if ((empty === '=') && (firstchar !== '=')) {\n";
	m_free_text << "		target.textContent = '=' + contents;\n";
	m_free_text << "		return;\n";
	m_free_text << "	}\n";
	m_free_text << "	else if ((empty === '.') && (firstchar === '!')) {\n";
	m_free_text << "		contents = contents.replace(/^[!*=]*/, '');\n";
	m_free_text << "		target.textContent = contents;\n";
	m_free_text << "		return;\n";
	m_free_text << "	}\n";
	m_free_text << "	else if ((empty === '.') && (firstchar === '*')) {\n";
	m_free_text << "		contents = contents.replace(/^[!*=]*/, '');\n";
	m_free_text << "		target.textContent = contents;\n";
	m_free_text << "		return;\n";
	m_free_text << "	}\n";
	m_free_text << "	else if ((empty === '.') && (firstchar === '=')) {\n";
	m_free_text << "		contents = contents.replace(/^[!*=]*/, '');\n";
	m_free_text << "		target.textContent = contents;\n";
	m_free_text << "		return;\n";
	m_free_text << "	}\n";
	m_free_text << "});\n";
	m_free_text << "\n";
	m_free_text << "\n";
	m_free_text << "\n";
	m_free_text << "//////////////////////////////\n";
	m_free_text << "//\n";
	m_free_text << "// moveCursorToEndOfText --\n";
	m_free_text << "//\n";
	m_free_text << "\n";
	m_free_text << "function moveCursorToEndOfText(element) {\n";
	m_free_text << "	var range;\n";
	m_free_text << "	var selection;\n";
	m_free_text << "	if (document.createRange) {\n";
	m_free_text << "		range = document.createRange();\n";
	m_free_text << "		range.selectNodeContents(element);\n";
	m_free_text << "		range.collapse(false);\n";
	m_free_text << "		selection = window.getSelection();\n";
	m_free_text << "		selection.removeAllRanges();\n";
	m_free_text << "		selection.addRange(range);\n";
	m_free_text << "	}\n";
	m_free_text << "}\n";
	m_free_text << "\n";
	m_free_text << "</script>\n";

}



//////////////////////////////
//
// Tool_humsheet::analyzeTracks --
//

void Tool_humsheet::analyzeTracks(HumdrumFile& infile) {
	m_max_track = infile.getMaxTrack();
	m_max_subtrack.resize(m_max_track);
	std::fill(m_max_subtrack.begin(), m_max_subtrack.end(), 0);
	vector<int> current(m_max_track, 0);
	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].hasSpines()) {
			continue;
		}
		fill(current.begin(), current.end(), 0);
		for (int j=0; j<infile[i].getFieldCount(); j++) {
			HTp token = infile.token(i, j);
			int track = token->getTrack();
			track--;  // 0-indexing tracks
			current.at(track)++;
			if (current.at(track) > m_max_subtrack.at(track)) {
				m_max_subtrack[track] = current[track];
			}
		}
	}

	m_max_field = 0;
	for (int i=0; i<(int)m_max_subtrack.size(); i++) {
		m_max_field += m_max_subtrack[i];
	}
}



//////////////////////////////
//
// Tool_humsheet::analyzeTabIndex --
//

void Tool_humsheet::analyzeTabIndex(HumdrumFile& infile) {
	int scount = infile.getStrandCount();
	int counter = 1;
	for (int i=0; i<scount; i++) {
		HTp start = infile.getStrandStart(i);
		HTp stop = infile.getStrandEnd(i);
		HTp current = start;
		while (current && (current != stop)) {
			string number = to_string(counter++);
			current->setValue("auto", "tabindex", number);
			current = current->getNextToken();
		}
	}
}



// END_MERGE

} // end namespace hum



