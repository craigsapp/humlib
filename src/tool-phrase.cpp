//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Thu May 31 16:41:35 PDT 2018
// Last Modified: Thu May 31 16:41:39 PDT 2018
// Filename:      tool-phrase.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-phrase.cpp
// Syntax:        C++11; humlib
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Analyses phrases
//

#include "tool-phrase.h"
#include "Convert.h"
#include "HumRegex.h"

#include <algorithm>
#include <cmath>

using namespace std;

namespace hum {

// START_MERGE


/////////////////////////////////
//
// Tool_gridtest::Tool_phrase -- Set the recognized options for the tool.
//

Tool_phrase::Tool_phrase(void) {
	define("A|no-average=b", "do not do average phrase-length analysis");
	define("R|remove2=b", "remove phrase boundaries in data and do not do analysis");
	define("m|mark=b", "mark phrase boundaries based on rests");
	define("r|remove=b", "remove phrase boundaries in data");
	define("c|color=s", "display color of analysis data");
}



///////////////////////////////
//
// Tool_phrase::run -- Primary interfaces to the tool.
//

bool Tool_phrase::run(const string& indata, ostream& out) {
	HumdrumFile infile(indata);
	return run(infile, out);
}


bool Tool_phrase::run(HumdrumFile& infile, ostream& out) {
	bool status = run(infile);
	return status;
}


bool Tool_phrase::run(HumdrumFile& infile) {
	initialize(infile);
	for (int i=0; i<(int)m_starts.size(); i++) {
		if (m_removeQ) {
			removePhraseMarks(m_starts[i]);
		}
		if (m_remove2Q) {
			continue;
		}
		if (hasPhraseMarks(m_starts[i])) {
			analyzeSpineByPhrase(i);
		} else {
			analyzeSpineByRests(i);
		}
	}
	if (!m_remove2Q) {
		prepareAnalysis(infile);
	}
	infile.createLinesFromTokens();
	return true;
}



//////////////////////////////
//
// Tool_phrase::prepareAnalysis --
//

void Tool_phrase::prepareAnalysis(HumdrumFile& infile) {
	string exinterp = "**cdata";
	infile.appendDataSpine(m_results.back(), "", exinterp);
	for (int i = (int)m_results.size()-1; i>0; i--) {
		int track = m_starts[i]->getTrack();
		infile.insertDataSpineBefore(track, m_results[i-1], "", exinterp);
	}
	if (m_averageQ) {
		addAverageLines(infile);
	}
	if (!m_color.empty()) {
		int insertline = -1;
		for (int i=0; i<infile.getLineCount(); i++) {
			if (infile[i].isData() || infile[i].isBarline()) {
				insertline = i;
				break;
			}
		}
		if (insertline > 0) {
			stringstream ss;
			int fsize = infile[insertline].getFieldCount();
			for (int j=0; j<fsize; j++) {
				ss << "*";
				HTp token = infile.token(insertline, j);
				string dt = token->getDataType();
				if (dt.empty() || (dt == "**cdata")) {
					ss << "color:" << m_color;
				}
				if (j < fsize  - 1) {
					ss << "\t";
				}
			}
			string output = ss.str();
			infile.insertLine(insertline, output);
		}
	}
}



///////////////////////////////
//
// Tool_pharse::addAverageLines --
//

void Tool_phrase::addAverageLines(HumdrumFile& infile) {
	vector<string> averages;
	averages.resize(m_starts.size()+1);
	int tcount = 0;
	HumNum tsum = 0;
	double average;
	stringstream ss;
	for (int i=0; i<(int)m_starts.size(); i++) {
		if (m_pcount[i] > 0) {
			average = m_psum[i].getFloat() / m_pcount[i];
		} else {
			average = 0.0;
		}
		ss.str("");
		ss.clear();
		ss << "!!average-phrase-length-k" << i+1 << ":\t" << average;
		averages[i+1] = ss.str();
		tcount += m_pcount[i];
		tsum += m_psum[i];
	}
	average = tsum.getFloat() / tcount;
	ss.str("");
	ss.clear();
	ss << "!!average-phrase-length:\t" << average;
	averages[0] = ss.str();

	for (int i=0; i<(int)averages.size(); i++) {
		infile.appendLine(averages[i]);
	}
}



///////////////////////////////
//
// Tool_phrase::initialize --
//

void Tool_phrase::initialize(HumdrumFile& infile) {
	m_starts = infile.getKernSpineStartList();
	m_results.resize(m_starts.size());
	int lines = infile.getLineCount();
	for (int i=0; i<(int)m_results.size(); i++) {
		m_results[i].resize(lines);
	}
	m_pcount.resize(m_starts.size());
	m_psum.resize(m_starts.size());
	std::fill(m_pcount.begin(), m_pcount.end(), 0);
	std::fill(m_psum.begin(), m_psum.end(), 0);
	m_markQ = getBoolean("mark");
	m_removeQ = getBoolean("remove");
	m_averageQ = !getBoolean("no-average");
	m_remove2Q = getBoolean("remove2");
	if (getBoolean("color")) {
		m_color = getString("color");
	}
}



///////////////////////////////
//
// Tool_phrase::analyzeSpineByRests --
//

void Tool_phrase::analyzeSpineByRests(int index) {
	HTp start    = m_starts[index];
	HTp current  = start;
	HTp lastnote = NULL;   // last note to be processed
	HTp pstart   = NULL;   // phrase start;
	HumNum dur;
	stringstream ss;
	while (current) {
		if (current->isBarline()) {
			if (current->find("||") != std::string::npos) {
				if (pstart) {
					dur = current->getDurationFromStart()
							- pstart->getDurationFromStart();
					ss.str("");
					ss.clear();
					ss << dur.getFloat();
					m_psum[index] += dur;
					m_pcount[index]++;
					m_results[index][pstart->getLineIndex()] = ss.str();
					pstart = NULL;
					if (m_markQ && lastnote) {
						lastnote->setText(lastnote->getText() + "}");
					}
				}
			}
		}
		if (!current->isData()) {
			current = current->getNextToken();
			continue;
		}
		if (current->isNull()) {
			current = current->getNextToken();
			continue;
		}
		if (pstart && current->isRest()) {
			if (lastnote) {
				dur = current->getDurationFromStart()
						- pstart->getDurationFromStart();
				ss.str("");
				ss.clear();
				ss << dur.getFloat();
				m_psum[index] += dur;
				m_pcount[index]++;
				m_results[index][pstart->getLineIndex()] = ss.str();
				if (m_markQ) {
					lastnote->setText(lastnote->getText() + "}");
				}
			}
			pstart = NULL;
			lastnote = NULL;
			current = current->getNextToken();
			continue;
		}
		if (current->isRest()) {
			current = current->getNextToken();
			continue;
		}
		if (current->isNote()) {
			lastnote = current;
		}
		if (pstart && current->isNote() && (current->find(";") != std::string::npos)) {
			// fermata at end of phrase.
			dur = current->getDurationFromStart() + current->getDuration()
					- pstart->getDurationFromStart();
			ss.str("");
			ss.clear();
			ss << dur.getFloat();
			m_psum[index] += dur;
			m_pcount[index]++;
			m_results[index][pstart->getLineIndex()] = ss.str();
			if (m_markQ) {
				current->setText(current->getText() + "}");
			}
			current = current->getNextToken();
			pstart = NULL;
			continue;
		}
		if (current->isNote() && pstart == NULL) {
			pstart = current;
			if (m_markQ) {
				current->setText("{" + current->getText());
			}
		}
		current = current->getNextToken();
	}
	if (pstart) {
		dur = start->getOwner()->getOwner()->getScoreDuration()
				- pstart->getDurationFromStart();
		ss.str("");
		ss.clear();
		ss << dur.getFloat();
		m_psum[index] += dur;
		m_pcount[index]++;
		m_results[index][pstart->getLineIndex()] = ss.str();
		if (m_markQ && lastnote) {
			lastnote->setText(lastnote->getText() + "}");
		}
	}
}



///////////////////////////////
//
// Tool_phrase::analyzeSpineByPhrase --
//

void Tool_phrase::analyzeSpineByPhrase(int index) {
	HTp start    = m_starts[index];
	HTp current  = start;
	HTp pstart   = NULL;   // phrase start;
	HumNum dur;
	stringstream ss;
	while (current) {
		if (!current->isData()) {
			current = current->getNextToken();
			continue;
		}
		if (current->isNull()) {
			current = current->getNextToken();
			continue;
		}
		if (current->find("{") != std::string::npos) {
			pstart = current;
			current = current->getNextToken();
			continue;
		}
		if (current->find("}") != std::string::npos) {
			if (pstart) {
				dur = current->getDurationFromStart() + current->getDuration()
						- pstart->getDurationFromStart();
				ss.str("");
				ss.clear();
				ss << dur.getFloat();
				m_psum[index] += dur;
				m_pcount[index]++;
				m_results[index][pstart->getLineIndex()] = ss.str();
			}
			current = current->getNextToken();
			continue;
		}
		current = current->getNextToken();
	}
}



//////////////////////////////
//
// Tool_phrase::removePhraseMarks -- Remvoe { and } characters from **kern data.
//

void Tool_phrase::removePhraseMarks(HTp start) {
	HTp current = start;
	HumRegex hre;
	while (current) {
		if (!current->isData()) {
			current = current->getNextToken();
			continue;
		}
		if (current->isNull()) {
			current = current->getNextToken();
			continue;
		}
		if (current->find("{") != std::string::npos) {
			string data = *current;
			hre.replaceDestructive(data, "", "\\{", "g");
			current->setText(data);
		}
		if (current->find("}") != std::string::npos) {
			string data = *current;
			hre.replaceDestructive(data, "", "\\}", "g");
			current->setText(data);
		}
		current = current->getNextToken();
	}
}



//////////////////////////////
//
// Tool_phrase::hasPhraseMarks -- True if **kern data spine (primary layer), has
//   "{" (or "}", but this is not checked) characters (phrase markers).
//

bool Tool_phrase::hasPhraseMarks(HTp start) {
	HTp current = start;
	while (current) {
		if (!current->isData()) {
			current = current->getNextToken();
			continue;
		}
		if (current->find("{") != std::string::npos) {
			return true;
		}
		current = current->getNextToken();
	}
	return false;
}



// END_MERGE

} // end namespace hum



