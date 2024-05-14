//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Apr 15 00:21:49 PDT 2024
// Last Modified: Thu May  2 01:08:42 PDT 2024
// Filename:      tool-addtempo.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/tool-addtempo.cpp
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Options:       -q # = set tempo in units of quarter notes per minute.
//                -q "#; m#:#; m#:#" = set three tempos, one at start, and two
//                at given measures.
//
// Description:   Insert tempo interpretations from -q option.
//                example
//                    addtempo -t 60.24
//                Add *MM60.24 in **kern spines at start of music (preferrably
//                right after key signature / metric symbol).
//                    addtempo -t "60; m12:132; m24:92"
//                Add *MM60 at start of music, *MM132 at measure 12, and
//                *MM92 at measure 24.
//
// Example:       Adding mutiple tempo changes:
//                    addtempo -q "126; m7:52; m7b:126; m14:52; m15:126"
//                m7b: "b" is an offset, meaning the next measure after the (first) labeled measure 7.
//


#include "tool-addtempo.h"
#include "HumRegex.h"

#include <algorithm>
#include <tuple>
#include <utility>
#include <vector>

using namespace std;

namespace hum {

// START_MERGE


/////////////////////////////////
//
// Tool_addtempo::Tool_addtempo -- Set the recognized options for the tool.
//

Tool_addtempo::Tool_addtempo(void) {
	define("q|quarter-notes-per-minute=d:120.0", "Quarter notes per minute (or list by measure)");
}



/////////////////////////////////
//
// Tool_addtempo::run -- Do the main work of the tool.
//

bool Tool_addtempo::run(HumdrumFileSet& infiles) {
	bool status = true;
	for (int i=0; i<infiles.getCount(); i++) {
		status &= run(infiles[i]);
	}
	return status;
}


bool Tool_addtempo::run(const string& indata, ostream& out) {
	HumdrumFile infile(indata);
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_addtempo::run(HumdrumFile& infile, ostream& out) {
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_addtempo::run(HumdrumFile& infile) {
	processFile(infile);
	return true;
}



//////////////////////////////
//
// Tool_addtempo::initialize --  Initializations that only have to be done once
//    for all HumdrumFile segments.
//

void Tool_addtempo::initialize(void) {
	string value = getString("quarter-notes-per-minute");
	HumRegex hre;
	m_tempos.clear();
	vector<string> pieces;
	hre.split(pieces, value, "\\s*;\\s*");
	for (int i=0; i<(int)pieces.size(); i++) {
		if (hre.search(pieces[i], "^\\s$")) {
			continue;
		}
		if (hre.search(pieces[i], "^\\s*m\\s*(\\d+)\\s*([a-z]?)\\s*:\\s*([\\d.]+)\\s*$")) {
			int measure = hre.getMatchInt(1);
			string soffset = hre.getMatch(2);
			int offset = 0;
			if (!soffset.empty()) {
				offset = soffset.at(0) - 'a';
			}
			double tempo = hre.getMatchDouble(3);
			m_tempos.emplace_back(measure, tempo, offset);
		} else if (hre.search(pieces[i], "^\\s*([\\d.]+)\\s*$")) {
			int measure = 0;
			int offset = 0;
			double tempo = hre.getMatchDouble(1);
			m_tempos.emplace_back(measure, tempo, offset);
		}
	}

	auto compareByFirst = [](const std::tuple<int, double, int>& a, const std::tuple<int, double, int>& b) {
		return std::get<0>(a) < std::get<0>(b);
	};
	std::sort(m_tempos.begin(), m_tempos.end(), compareByFirst);
}



//////////////////////////////
//
// Tool_addtempo::processFile --
//

void Tool_addtempo::processFile(HumdrumFile& infile) {
	initialize();

	vector<double> tlist;
	assignTempoChanges(tlist, infile);

	for (int i=0; i<infile.getLineCount(); i++) {
		if (tlist.at(i) > 0.0) {
			for (int j=0; j<infile[i].getFieldCount(); j++) {
				HTp token = infile.token(i, j);
				if (token->isKern()) {
					m_humdrum_text << "*MM" << tlist.at(i);
				} else {
					m_humdrum_text << "*";
				}
				if (j < infile[i].getFieldCount() - 1) {
					m_humdrum_text << "\t";
				}
			}
			m_humdrum_text << endl;
		}
		m_humdrum_text << infile[i] << endl;
	}
}



//////////////////////////////
//
// Tool_addtempo::assignTempoChanges -- add non-zero
//    tempo when it should change.
//

void Tool_addtempo::assignTempoChanges(vector<double>& tlist, HumdrumFile& infile) {
	tlist.resize(infile.getLineCount());
	std::fill(tlist.begin(), tlist.end(), 0.0);
	for (int i=0; i<(int)m_tempos.size(); i++) {
		addTempo(tlist, infile, std::get<0>(m_tempos[i]), std::get<1>(m_tempos[i]), std::get<2>(m_tempos[i]));
	}
}



//////////////////////////////
//
// Tool_addtempo::addTempo -- Add specified tempo to list.
//

void Tool_addtempo::addTempo(vector<double>& tlist, HumdrumFile& infile,
		int measure, double tempo, int offset) {

	if (measure == 0) {
		addTempoToStart(tlist, infile, tempo);
		return;
	}

	// find measure index:
	int barIndex = -1;
	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isBarline()) {
			continue;
		}
		int bar = infile[i].getBarNumber();
		if (bar == measure) {
			if (offset == 0) {
				barIndex = i;
				break;
			}
			int counter = 0;
			for (int j=i+1; j<infile.getLineCount(); j++) {
				if (infile[j].isBarline()) {
					counter++;
					if (counter == offset) {
						barIndex = j;
						break;
					}
				}
			}
			break;
		}
	}
	if (barIndex < 0) {
		return;
	}
	int sigIndex = -1;
	int symIndex = -1;
	int dataIndex = -1;
	for (int i=barIndex+1; i<infile.getLineCount(); i++) {
		if (infile[i].isData()) {
			dataIndex = i;
			break;
		}
		if (infile[i].isBarline()) {
			break;
		}
		if (!infile[i].isInterpretation()) {
			continue;
		}
		for (int j=0; j<infile[i].getFieldCount(); j++) {
			HTp token = infile.token(i, j);
			if (!token->isKern()) {
				continue;
			}
			if (token->isTimeSignature()) {
				sigIndex = i;
			} else if (token->isMetricSymbol()) {
				symIndex = i;
			}
		}
	}

	if (dataIndex < 0) {
		return;
	}

	if ((sigIndex >= 0) && (symIndex >= 0)) {
		if (sigIndex > symIndex) {
			tlist.at(sigIndex+1) = tempo;
		} else {
			tlist.at(symIndex+1) = tempo;
		}
		return;
	} else if (sigIndex >= 0) {
		tlist.at(sigIndex+1) = tempo;
		return;
	} else if (symIndex >= 0) {
		return;
	} else if (dataIndex >= 0) {
		int localIndex = dataIndex - 1;
		int lastSpineIndex = localIndex;
		if (infile[dataIndex-1].isLocalComment()) {
			while (infile[localIndex].isLocalComment() || !infile[localIndex].hasSpines()) {
				if (!infile[localIndex].hasSpines()) {
					localIndex--;
					continue;
				} else {
					lastSpineIndex = localIndex;
				}
				if (infile[localIndex].isLocalComment()) {
					lastSpineIndex = localIndex;
					localIndex--;
					continue;
				}
			}
			tlist.at(lastSpineIndex) = tempo;
			return;
		} else {
			tlist.at(dataIndex) = tempo;
		}
	}

}



//////////////////////////////
//
// Tool_addtempo::addTempoToStart -- Can't use letter postfix for 0 measure for now.
//

void Tool_addtempo::addTempoToStart(vector<double>& tlist,
		HumdrumFile& infile, double tempo) {

	// find first measure and data line indexes:
	int barIndex  = -1;
	int dataIndex = -1;
	int sigIndex  = -1;
	int symIndex  = -1;

	for (int i=0; i<infile.getLineCount(); i++) {
		if (infile[i].isData()) {
			dataIndex = i;
			break;
		}

		if (infile[i].isBarline()) {
			if (barIndex < 0) {
				barIndex = i;
			}
			continue;
		}

		if (!infile[i].isInterpretation()) {
			continue;
		}
		for (int j=0; j<infile[i].getFieldCount(); j++) {
			HTp token = infile.token(i, j);
			if (!token->isKern()) {
				continue;
			}
			if (token->isTimeSignature()) {
				sigIndex = i;
			} else if (token->isMetricSymbol()) {
				symIndex = i;
			}
		}
	}

	if (dataIndex < 0) {
		return;
	}

	if ((sigIndex >= 0) && (symIndex >= 0)) {
		if (sigIndex > symIndex) {
			tlist.at(sigIndex+1) = tempo;
		} else {
			tlist.at(symIndex+1) = tempo;
		}
		return;
	} else if (sigIndex >= 0) {
		tlist.at(sigIndex+1) = tempo;
		return;
	} else if (symIndex >= 0) {
		return;
	} else if (dataIndex >= 0) {
		if (infile[dataIndex-1].isLocalComment()) {
			int localIndex = dataIndex - 1;
			int lastSpineIndex = localIndex;
			while (infile[localIndex].isLocalComment() || !infile[localIndex].hasSpines()) {
				if (!infile[localIndex].hasSpines()) {
					localIndex--;
					continue;
				} else {
					lastSpineIndex = localIndex;
				}
				if (infile[localIndex].isLocalComment()) {
					lastSpineIndex = localIndex;
					localIndex--;
					continue;
				}
			}
			tlist.at(lastSpineIndex) = tempo;
			return;
		} else {
			tlist.at(dataIndex) = tempo;
		}
	}

}



// END_MERGE

} // end namespace hum



