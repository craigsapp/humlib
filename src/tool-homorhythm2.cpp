//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Fri Aug  9 17:58:05 EDT 2019
// Last Modified: Fri Aug  9 17:58:08 EDT 2019
// Filename:      tool-homorhythm2.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/tool-homorhythm2.cpp
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Identify homorhythm2 regions of music.
//

#include "tool-homorhythm2.h"
#include "Convert.h"
#include "NoteGrid.h"

using namespace std;

namespace hum {

// START_MERGE


/////////////////////////////////
//
// Tool_homorhythm2::Tool_homorhythm -- Set the recognized options for the tool.
//

Tool_homorhythm2::Tool_homorhythm2(void) {
	define("t|threshold=d:1.6",  "threshold score sum required for homorhythm texture detection");
	define("u|threshold2=d:1.3", "threshold score sum required for semi-homorhythm texture detection");
	define("s|score=b",          "show numeric scores");
	define("n|length=i:4",       "sonority length to calculate");
	define("f|fraction=b",       "report fraction of music that is homorhythm");
}



/////////////////////////////////
//
// Tool_homorhythm2::run -- Do the main work of the tool.
//

bool Tool_homorhythm2::run(HumdrumFileSet& infiles) {
	bool status = true;
	for (int i=0; i<infiles.getCount(); i++) {
		status &= run(infiles[i]);
	}
	return status;
}


bool Tool_homorhythm2::run(const string& indata, ostream& out) {
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


bool Tool_homorhythm2::run(HumdrumFile& infile, ostream& out) {
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_homorhythm2::run(HumdrumFile& infile) {
	initialize();
	processFile(infile);
	return true;
}



//////////////////////////////
//
// Tool_homorhythm2::initialize --
//

void Tool_homorhythm2::initialize(void) {
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
// Tool_homorhythm2::processFile --
//

void Tool_homorhythm2::processFile(HumdrumFile& infile) {
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
		m_score[index] = score / count;
	}

	for (int i=grid.getSliceCount()-1; i>=wsize; i--) {
		score = 0;
		count = 0;
		for (int j=0; j<grid.getVoiceCount(); j++) {
			for (int k=j+1; k<grid.getVoiceCount(); k++) {
				for (int m=0; m<wsize; m++) {
					NoteCell* cell1 = grid.cell(j, i-m);
					if (cell1->isRest()) {
						continue;
					}
					NoteCell* cell2 = grid.cell(k, i-m);
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
		m_score[index] += score / count;
	}


	for (int i=0; i<(int)m_score.size(); i++) {
		m_score[i] = int(m_score[i] * 100.0 + 0.5) / 100.0;
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

	if (getBoolean("fraction")) {
		HumNum sum = 0;
		HumNum total = infile.getScoreDuration();
		for (int i=0; i<(int)m_score.size(); i++) {
			if (m_score[i] >= m_threshold2) {
				sum += infile[i].getDuration();
			}
		}
		HumNum fraction = sum / total;
		m_free_text << int(fraction.getFloat() * 1000.0 + 0.5) / 10.0 << endl;
	} else {
		if (getBoolean("score")) {
			infile.appendDataSpine(m_score, ".", "**cdata", false);
		}
		infile.appendDataSpine(color, ".", "**color", true);
		infile.createLinesFromTokens();

		// problem within emscripten-compiled version, so force to output as string:
		m_humdrum_text << infile;
	}

}




// END_MERGE

} // end namespace hum



