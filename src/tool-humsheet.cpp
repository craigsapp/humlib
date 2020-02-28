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

using namespace std;

namespace hum {

// START_MERGE


/////////////////////////////////
//
// Tool_humsheet::Tool_humsheet -- Set the recognized options for the tool.
//

Tool_humsheet::Tool_humsheet(void) {
	define("h|H|html|HTML=b", "output table in HTML wrapper");
	define("z|zebra=b", "add zebra striping by spine to style");
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
	m_htmlQ = getBoolean("html");
	m_zebraQ = getBoolean("zebra");
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
	analyzeTabIndex(infile);
	m_free_text << "<table class=\"humdrum\"";
	m_free_text << " data-spine-count=\"" << infile.getMaxTrack() << "\"";
	m_free_text << ">\n";
	for (int i=0; i<infile.getLineCount(); i++) {
		m_free_text << "<tr";
		printRowClasses(infile, i);
		m_free_text << ">";
		printRowContents(infile, i);
		m_free_text << "</tr>\n";
	}
	m_free_text << "</table>";
	if (m_htmlQ) {
		printHtmlFooter();
	}
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
	HumdrumLine* hl = &infile[row];
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
	}
	if (hl->isLocalComment()) {
		classes += "lcomment ";
	}

	if (hl->isUniversalReference()) {
		classes += "ureference ";
	} else if (hl->isCommentUniversal()) {
		classes += "ucomment ";
	} else if (hl->isReference()) {
		classes += "reference ";
	} else if (hl->isGlobalComment()) {
		classes += "gcomment ";
	}

	if (hl->isBarline()) {
		classes += "barline ";
	}
	if (hl->isManipulator()) {
		classes += "manip ";
	}
	if (!classes.empty()) {
      // remove space.
		classes.resize((int)classes.size() - 1);
		m_free_text << " class=\"" << classes << "\"";
	}
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
		printId(token);
		printCellClasses(token);
		printSpineData(token);
		printSubspineData(token);
		printTabIndex(token);
		printColSpan(token);
		m_free_text << " contenteditable=\"true\">";
		printToken(token);
		m_free_text << "</td>";
	}
}



//////////////////////////////
//
// Tool_humsheet::printSpineData --
//

void Tool_humsheet::printSpineData(HTp token) {
	int spine = token->getTrack() - 1;
	m_free_text << " data-spine=\"" << spine << "\"";
}



//////////////////////////////
//
// Tool_humsheet::printSubspineData --
//

void Tool_humsheet::printSubspineData(HTp token) {
	int spine = token->getSubtrack();
	if (spine > 0) {
		spine--;
	}
	m_free_text << " data-subspine=\"" << spine << "\"";
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
	if (track % 2 == 0) {
		classlist = "zebra";
	}
	if (!classlist.empty()) {
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
	m_free_text << "table.humdrum td {\n";
	m_free_text << "	outline: none;\n";
	m_free_text << "}\n";
	m_free_text << "tr.ucomment {\n";
	m_free_text << "	color: chocolate;\n";
	m_free_text << "}\n";
	m_free_text << "tr.ureference {\n";
	m_free_text << "	color: chocolate;\n";
	m_free_text << "	background: rgb(255,99,71,0.25);\n";
	m_free_text << "}\n";
	m_free_text << "tr.reference {\n";
	m_free_text << "	color: green;\n";
	m_free_text << "}\n";
	m_free_text << "tr.interp {\n";
	m_free_text << "	color: darkviolet;\n";
	m_free_text << "}\n";
	m_free_text << "tr.barline {\n";
	m_free_text << "	color: gray;\n";
	m_free_text << "	background: rgba(0, 0, 0, 0.06);\n";
	m_free_text << "}\n";
	if (m_zebraQ) {
		m_free_text << ".zebra {\n";
		m_free_text << "	background: #ccccff33;\n";
		m_free_text << "}\n";
	}
	m_free_text << "</style>\n";
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



