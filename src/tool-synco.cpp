//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Apr 30 10:42:45 PDT 2022
// Last Modified: Wed May  4 01:03:46 PDT 2022
// Filename:      tool-synco.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/tool-synco.cpp
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Identify and mark syncopations.
//

#include "tool-synco.h"
#include "HumRegex.h"

using namespace std;

namespace hum {

// START_MERGE


/////////////////////////////////
//
// Tool_synco::Tool_synco -- Set the recognized options for the tool.
//

Tool_synco::Tool_synco(void) {
	define("c|color=s:skyblue", "SVG color to highlight syncopation notes");
	define("i|info=b", "Display only statistics info");
	define("f|filename=b", "Add filename to statistics info");
	define("a|all=b", "Average all statistics info");
}


/////////////////////////////////
//
// Tool_synco::run -- Do the main work of the tool.
//

bool Tool_synco::run(HumdrumFileSet& infiles) {
	bool status = true;
	for (int i=0; i<infiles.getCount(); i++) {
		status &= run(infiles[i]);
	}
	if (m_allQ) {
		m_free_text << m_scountTotal << "\t";
		m_free_text << m_notecountTotal << "\t";
		double percent =  (double)m_scountTotal / m_notecountTotal;
		percent = int(percent * 10000.0 + 0.5) / 100.0;
		m_free_text << percent << "\t";
		m_free_text << m_fileCount;
		if (m_fileCount == 1) {
			m_free_text << " file";
		} else {
			m_free_text << " files";
		}
		m_free_text << endl;
	}
	return status;
}



bool Tool_synco::run(const string& indata, ostream& out) {
	HumdrumFile infile(indata);
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_synco::run(HumdrumFile& infile, ostream& out) {
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_synco::run(HumdrumFile& infile) {
	initialize();
	processFile(infile);
	if (m_hasSyncoQ && !m_infoQ) {
		infile.createLinesFromTokens();
		m_humdrum_text << infile;
		m_humdrum_text << "!!!RDF**kern: | = marked note, color=" << m_color << endl;
	}
	double notecount = infile.getNoteCount();
	double density = m_scount / (double)notecount;
	double percent =  int(density * 10000.0 + 0.5) / 100.0;
	if (m_infoQ) {
		m_free_text << m_scount << "\t" << notecount << "\t" << percent << "%";
		if (m_fileQ) {
			m_free_text << "\t" << infile.getFilename();
		}
		m_free_text << endl;

		m_scountTotal    += m_scount;
		m_notecountTotal += notecount;
		m_fileCount++;
	} else {
		m_humdrum_text << "!!!syncopated_notes: " << m_scount << endl;
		m_humdrum_text << "!!!total_notes: " << notecount << endl;
		m_humdrum_text << "!!!syncopated_density: " << percent << "%" << endl;
	}

	return true;
}



//////////////////////////////
//
// Tool_synco::initialize --  Initializations that only have to be done once
//    for all HumdrumFile segments.
//

void Tool_synco::initialize(void) {
	m_infoQ = getBoolean("info");
	m_fileQ = getBoolean("filename");
	m_allQ  = getBoolean("all");
	m_color = getString("color");
}



//////////////////////////////
//
// Tool_synco::processFile --
//

void Tool_synco::processFile(HumdrumFile& infile) {
	int scount = infile.getStrandCount();
	m_scount = 0;
	for (int i=0; i<scount; i++) {
		HTp stok = infile.getStrandStart(i);
		if (!stok->isKern()) {
			continue;
		}
		HTp etok = infile.getStrandEnd(i);
		processStrand(stok, etok);
	}
}



//////////////////////////////
//
// Tool_synco::processStrand --
//

void Tool_synco::processStrand(HTp stok, HTp etok) {
	HTp current = stok;
	while (current && (current != etok)) {
		if (!current->isData()) {
			current = current->getNextToken();
			continue;
		}
		if (current->isNull()) {
			current = current->getNextToken();
			continue;
		}
		if (current->isRest()) {
			current = current->getNextToken();
			continue;
		}
		if (current->isSecondaryTiedNote()) {
			current = current->getNextToken();
			continue;
		}
		if (isSyncopated(current)) {
			m_hasSyncoQ = true;
			m_scount++;
			markNote(current);
		}
		current = current->getNextToken();
	}
}



//////////////////////////////
//
// Tool_synco::isSyncopated --
//

bool Tool_synco::isSyncopated(HTp token) {
	double metlev   = getMetricLevel(token);
	HumNum duration = token->getTiedDuration();
	double logDur   = log2(duration.getFloat());
	if (metlev == 2) {
		return false;
	}
	if (logDur > metlev) {
		return true;
	} else {
		return false;
	}
}



//////////////////////////////
//
// Tool_synco::getMetricLevel -- Assuming whole-note beats for now.
//

double Tool_synco::getMetricLevel(HTp token) {
	HumNum durbar = token->getDurationFromBarline();
	if (!durbar.isInteger()) {
		return -1.0;
	}
	if (durbar.getNumerator() % 4 == 0) {
		return 2.0;
	}
	if (durbar.getNumerator() % 2 == 0) {
		return 1.0;
	}
	return 0.0;
}



//////////////////////////////
//
// Tool_synco::markNote -- Currently ignoring chords.
//

void Tool_synco::markNote(HTp token) {
	token->setText(token->getText() + "|");
	if ((token->find('[') != string::npos) || (token->find('_') != string::npos)) {
		HTp current = token->getNextToken();
		while (current) {
			if (!current->isData()) {
				current = current->getNextToken();
				continue;
			}
			if (current->isNull()) {
				current = current->getNextToken();
				continue;
			}
			if (current->isRest()) {
				break;
			}
			if (current->find("_") != string::npos) {
				current->setText(current->getText() + "|");
			} else if (current->find("]") != string::npos) {
				current->setText(current->getText() + "|");
				break;
			}
			current = current->getNextToken();
		}
	}
}



// END_MERGE

} // end namespace hum



