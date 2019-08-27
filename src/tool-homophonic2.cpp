//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Fri Aug  9 17:58:05 EDT 2019
// Last Modified: Fri Aug  9 17:58:08 EDT 2019
// Filename:      tool-homophonic2.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/tool-homophonic2.cpp
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Identify homophonic2 regions of music.
//

#include "tool-homophonic2.h"
#include "Convert.h"
#include "NoteGrid.h"

using namespace std;

namespace hum {

// START_MERGE


/////////////////////////////////
//
// Tool_homophonic2::Tool_homophonic -- Set the recognized options for the tool.
//

Tool_homophonic2::Tool_homophonic2(void) {
	define("t|threshold=d:0.6", "Threshold score sum required for homophonic texture detection");
	define("u|threshold2=d:0.4", "Threshold score sum required for semi-homophonic texture detection");
	define("s|score=b", "Show numeric scores");
	define("n|length=i:5", "Sonority length to calculate");
}



/////////////////////////////////
//
// Tool_homophonic2::run -- Do the main work of the tool.
//

bool Tool_homophonic2::run(HumdrumFileSet& infiles) {
	bool status = true;
	for (int i=0; i<infiles.getCount(); i++) {
		status &= run(infiles[i]);
	}
	return status;
}


bool Tool_homophonic2::run(const string& indata, ostream& out) {
	HumdrumFile infile;
	infile.read(indata);
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_homophonic2::run(HumdrumFile& infile, ostream& out) {
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_homophonic2::run(HumdrumFile& infile) {
	initialize();
	processFile(infile);
	infile.createLinesFromTokens();
	return true;
}



//////////////////////////////
//
// Tool_homophonic2::initialize --
//

void Tool_homophonic2::initialize(void) {
	m_threshold = getDouble("threshold");
	if (m_threshold < 0.0) {
		m_threshold = 0.0;
	}
	m_threshold2 = getDouble("threshold2");
	if (m_threshold2 < 0.0) {
		m_threshold2 = 0.0;
	}
	if (m_threshold < m_threshold2) {
		double temp = m_threshold;
		m_threshold = m_threshold2;
		m_threshold2 = temp;
	}

}



//////////////////////////////
//
// Tool_homophonic2::processFile --
//

void Tool_homophonic2::processFile(HumdrumFile& infile) {
	infile.analyzeStructure();
	NoteGrid grid(infile);
	m_score.resize(infile.getLineCount());
	fill(m_score.begin(), m_score.end(), 0.0);

	double score;
	int count;
	int wsize = getInteger("length");
	for (int i=0; i<grid.getSliceCount()-wsize; i++) {
		score = 0;
		count = 0;
		for (int j=0; j<grid.getVoiceCount(); j++) {
			for (int k=j+1; k<grid.getVoiceCount(); k++) {
				for (int m=0; m<wsize; m++) {
					NoteCell* cell1 = grid.cell(j, i+m);
					if (cell1->isRest()) {
						continue;
					}
					NoteCell* cell2 = grid.cell(k, i+m);
					if (cell2->isRest()) {
						continue;
					}
					count++;
					if (cell1->isAttack() && cell2->isAttack()) {
						score += 1.0;
					}
				}
			}
		}
		int index = grid.getLineIndex(i);
		m_score[index] = int(score / count * 100.0 + 0.5) / 100.0;
	}

	vector<string> color(infile.getLineCount());;
	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isData()) {
			continue;
		}
		if (m_score[i] >= m_threshold) {
			color[i] = "red";
		} else if (m_score[i] >= m_threshold2) {
			color[i] = "orange";
		} else {
			color[i] = "black";
		}
	}

	if (getBoolean("score")) {
		infile.appendDataSpine(m_score, ".", "**cdata", false);
	}
	infile.appendDataSpine(color, ".", "**color", true);

/*
	for (int i=0; i<grid.getSliceCount(); i++) {
		for (int j=0; j<grid.getVoiceCount(); j++) {
			if (grid.cell(j, i)->isRest()) {
				m_free_text << "R\t";
			} else if (grid.cell(j, i)->isAttack()) {
				m_free_text << "1\t";
			} else {
				m_free_text << "0\t";
			}
		}
		m_free_text << endl;
	}
*/
	

}




// END_MERGE

} // end namespace hum



