//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sun Feb  6 14:33:36 PST 2011
// Last Modified: Mon May 24 03:44:28 PDT 2021
// Filename:      tool-satb2gs.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/tool-satb2gs.cpp
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Converts Soprano/Alto/Tenor/Bass staves into
//                Grand-staff style.
//

#include "tool-satb2gs.h"
#include "Convert.h"
#include "HumRegex.h"

#include <vector>

using namespace std;

namespace hum {

// START_MERGE


/////////////////////////////////
//
// Tool_satb2gs::Tool_satb2gs -- Set the recognized options for the tool.
//

Tool_satb2gs::Tool_satb2gs(void) {
	// no options
}



/////////////////////////////////
//
// Tool_satb2gs::run -- Do the main work of the tool.
//

bool Tool_satb2gs::run(HumdrumFileSet& infiles) {
	bool status = true;
	for (int i=0; i<infiles.getCount(); i++) {
		status &= run(infiles[i]);
	}
	return status;
}


bool Tool_satb2gs::run(const string& indata, ostream& out) {
	HumdrumFile infile(indata);
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_satb2gs::run(HumdrumFile& infile, ostream& out) {
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_satb2gs::run(HumdrumFile& infile) {
	initialize();
	processFile(infile);
	return true;
}



//////////////////////////////
//
// Tool_satb2gs::initialize --  Initializations that only have to be done once
//    for all HumdrumFile segments.
//

void Tool_satb2gs::initialize(void) {
	// do nothing
}



//////////////////////////////
//
// Tool_satb2gs::processFile --
//

void Tool_satb2gs::processFile(HumdrumFile& infile) {
	vector<vector<int>> tracks;
	getTrackInfo(tracks, infile);

	if ((tracks[1].size() != 2) || (tracks[3].size() != 2)) {
		cerr << "Warning: not processing data since there must be at least four **kern spines" << endl;
		return;
	}

	bool goodHeader = validateHeader(infile);
	if (!goodHeader) {
		cerr << "Warning: no spine manipulations allows within header, not processing file" << endl;
		return;
	}

	bool dataQ = false;
	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].hasSpines()) {
			m_humdrum_text << infile[i] << endl;
			continue;
		}
		if (infile[i].isData()) {
			if (!dataQ) {
				printSpineSplitLine(tracks);
			}
			dataQ = true;
		}
		if (!dataQ) {
			printHeaderLine(infile, i, tracks);
			continue;
		}
		HTp token = infile.token(i, 0);
		if (*token == "*-") {
			printSpineMergeLine(tracks);
			printTerminatorLine(tracks);
			continue;
		}
		printRegularLine(infile, i, tracks);
	}
}



//////////////////////////////
//
// Tool_satb2gs::printRegularLine -- print a regular line
//   (between first data line and before terminator line).
//

void Tool_satb2gs::printRegularLine(HumdrumFile& infile, int line,
		vector<vector<int>>& tracks) {

	int spinecount = infile[line].getFieldCount();
	int track;
	HTp token;
	vector<vector<vector<HTp>>> tokens;
	tokens.resize(5);
	for (int i=0; i<(int)tracks.size(); i++) {
		tokens[i].resize(tracks[i].size());
	}

	// store tokens in output order:
	for (int i=0; i<(int)tracks.size(); i++) {
		for (int j=0; j<(int)tracks[i].size(); j++) {
			int target = tracks[i][j];
			for (int k=0; k<spinecount; k++) {
				token = infile.token(line, k);
				track = token->getTrack();
				if (track != target) {
					continue;
				}
				tokens[i][j].push_back(token);
			}
		}
	}

	int counter = 0;
	HTp top;
	HTp bot;
	HTp inner;
	HTp outer;
	bool suppressQ;

	// now print in output order, but hide fermatas
	// in the alto and tenor parts if there are fermatas
	// int the soprano and bass parts respectively.
	for (int i=0; i<(int)tokens.size(); i++) {
		for (int j=0; j<(int)tokens[i].size(); j++) {
			switch (i) {
				case 0:
				case 2:
				case 4:
					// non-kern spines
					for (int k=0; k<(int)tokens[i][j].size(); k++) {
						m_humdrum_text << tokens[i][j][k];
						counter++;
						if (counter < spinecount) {
							m_humdrum_text << "\t";
						}
					}
					break;

				case 1:
				case 3:
					top = tokens[i][0][0];
					bot = tokens[i][1][0];
					if (i == 1) {
						// tenor: top is inner
						inner = top;
						outer = bot;
					} else {
						// alto: bottom is inner
						inner = bot;
						outer = top;
					}
					if (inner->hasFermata() && outer->hasFermata()) {
						suppressQ = true;
					} else {
						suppressQ = false;
					}

					for (int k=0; k<(int)tokens[i][j].size(); k++) {
						token = tokens[i][j][k];
						if (suppressQ && ((void*)token == (void*)inner)) {
							string value = *token;
							// Make fermata invisible by adding 'y' after it:
							for (int m=0; m<(int)value.size(); m++) {
								m_humdrum_text << value[m];
								if (value[m] == ';') {
									if (m < (int)value.size() - 1) {
										if (value.at(m+1) != 'y') {
											m_humdrum_text << 'y';
										}
									} else {
											m_humdrum_text << 'y';
									}
								}
							}
						} else {
							m_humdrum_text << token;
						}
						counter++;
						if (counter < spinecount) {
							m_humdrum_text << "\t";
						}
					}
					break;
			}
		}
	}

	m_humdrum_text << endl;
}



//////////////////////////////
//
// Tool_satb2gs::printTerminatorLine --  Print the terminator line in the
//   output data.
//

void Tool_satb2gs::printTerminatorLine(vector<vector<int>>& tracks) {
	int count = getNewTrackCount(tracks);
	for (int i=0; i<count; i++) {
		m_humdrum_text << "*-";
		if (i < count - 1) {
			m_humdrum_text << "\t";
		}
	}
	m_humdrum_text << endl;
}



//////////////////////////////
//
// Tool_satb2gs::printSpineSplitLine --
//

void Tool_satb2gs::printSpineSplitLine(vector<vector<int>>& tracks) {
	int count = getNewTrackCount(tracks);
	int counter = 0;

	for (int i=0; i<(int)tracks.size(); i++) {
		switch (i) {
			case 0:
			case 2:
			case 4:
				for (int j=0; j<(int)tracks[i].size(); j++) {
					m_humdrum_text << "*";
					counter++;
					if (counter < count) {
						m_humdrum_text << "\t";
					}
				}
				break;
			case 1:
			case 3:
				m_humdrum_text << "*^";
				counter++;
				if (counter < count) {
					m_humdrum_text << "\t";
				}
				break;
		}
	}
	m_humdrum_text << endl;
}



//////////////////////////////
//
// Tool_satb2gs::printSpineMergeLine --
//

void Tool_satb2gs::printSpineMergeLine(vector<vector<int>>& tracks) {
	int count = getNewTrackCount(tracks);
	count += 2;
	int counter;

	if (!tracks[2].empty()) {
		// do not need to place merges on separate lines since they are
		// separated by non-kern spine(s) between bass and soprano subspines.

		counter = 0;
		for (int i=0; i<(int)tracks.size(); i++) {
			switch (i) {
				case 0:
				case 2:
				case 4:
					for (int j=0; j<(int)tracks[i].size(); j++) {
						m_humdrum_text << "*";
						counter++;
						if (counter < count) {
							m_humdrum_text << "\t";
						}
					}
					break;
				case 1:
				case 3:
					for (int j=0; j<(int)tracks[i].size(); j++) {
						m_humdrum_text << "*v";
						counter++;
						if (counter < count) {
							m_humdrum_text << "\t";
						}
					}
					break;
			}
		}
		m_humdrum_text << endl;

	} else {
		// Merges for tenor/bass and soprano/alto need to be placed
		// on separate lines.

		// First merge tenor/bass (tracks[1])
		counter = 0;
		for (int i=0; i<(int)tracks.size(); i++) {
			switch (i) {
				case 0:
				case 2:
				case 3:
				case 4:
					for (int j=0; j<(int)tracks[i].size(); j++) {
						m_humdrum_text << "*";
						counter++;
						if (counter < count) {
							m_humdrum_text << "\t";
						}
					}
					break;
				case 1:
					for (int j=0; j<(int)tracks[i].size(); j++) {
						m_humdrum_text << "*v";
						counter++;
						if (counter < count) {
							m_humdrum_text << "\t";
						}
					}
					break;
			}
		}
		m_humdrum_text << endl;

		// Now merge soprano/alto (tracks[3])
		count--;
		counter = 0;
		for (int i=0; i<(int)tracks.size(); i++) {
			switch (i) {
				case 0:
				case 2:
				case 4:
					for (int j=0; j<(int)tracks[i].size(); j++) {
						m_humdrum_text << "*";
						counter++;
						if (counter < count) {
							m_humdrum_text << "\t";
						}
					}
					break;
				case 1:
					m_humdrum_text << "*";
					m_humdrum_text << "\t";
					counter++;
					break;
				case 3:
					for (int j=0; j<(int)tracks[i].size(); j++) {
						m_humdrum_text << "*v";
						counter++;
						if (counter < count) {
							m_humdrum_text << "\t";
						}
					}
					break;
			}
		}
		m_humdrum_text << endl;
	}
}



//////////////////////////////
//
// Tool_satb2gs::getNewTrackCount -- Return the number of tracks (spines)
//   in the output data (not counting subspines).
//

int Tool_satb2gs::getNewTrackCount(vector<vector<int>>& tracks) {
	int sum = 0;
	for (int i=0; i<(int)tracks.size(); i++) {
		for (int j=0; j<(int)tracks[i].size(); j++) {
			sum++;
		}
	}
	// remove two spines that were merged into two others:
	sum -= 2;
	return sum;
}



//////////////////////////////
//
// Tool_satb2gs::printHeaderLine --
//

void Tool_satb2gs::printHeaderLine(HumdrumFile& infile, int line,
		vector<vector<int>>& tracks) {
	int count = infile.getMaxTrack() - 2;

	HTp token;
	int counter = 0;
	for (int i=0; i<(int)tracks.size(); i++) {
		switch (i) {
			case 0:
			case 2:
			case 4:
				for (int j=0; j<(int)tracks[i].size(); j++) {
					token = infile.token(line, tracks[i][j]-1);
					m_humdrum_text << token;
					counter++;
					if (counter < count) {
						m_humdrum_text << "\t";
					}
				}
				break;

			case 1:
			case 3:
				token = infile.token(line, tracks[i][0]-1);
				if (token->isInstrumentName()) {
					// suppress instrument names, but keep blank name
					// to force indent.
					m_humdrum_text << "*I\"";
				} else if (token->isInstrumentAbbreviation()) {
					// suppress instrument abbreviations
					m_humdrum_text << "*";
				} else if (token->isInstrumentDesignation()) {
					// suppress instrument designations (such as *Itenor)
					m_humdrum_text << "*";
				} else if (token->isClef()) {
					vector<HTp> clefs = getClefs(infile, line);
					if (i == 1) {
						if (clefs.size() == 4) {
							m_humdrum_text << clefs[0];
						} else {
							m_humdrum_text << "*clefF4";
						}
					} else {
						if (clefs.size() == 4) {
							m_humdrum_text << clefs.back();
						} else {
							m_humdrum_text << "*clefG2";
						}
					}
				} else {
					m_humdrum_text << token;
				}
				counter++;
				if (counter < count) {
					m_humdrum_text << "\t";
				}
				break;
		}
	}
	m_humdrum_text << endl;
}



//////////////////////////////
//
// Tool_satb2gs::getClefs -- get a list of the clefs on the current line.
//

vector<HTp> Tool_satb2gs::getClefs(HumdrumFile& infile, int line) {
	vector<HTp> output;
	for (int i=0; i<infile[line].getFieldCount(); i++) {
		HTp token = infile[line].token(i);
		if (!token->isKern()) {
			continue;
		}
		if (token->isClef()) {
			output.push_back(token);
		}
	}
	return output;
}



//////////////////////////////
//
// Tool_satb2gs::getTrackInfo --
//     tracks 0 = list of spines before bass **kern spine
//     tracks 1 = tenor and then bass **kern track numbers
//     tracks 2 = aux. spines after after tenor and then after bass
//     tracks 3 = soprano and then alto **kern track numbers
//     tracks 4 = aux. spines after after soprano and then after alto
//

void Tool_satb2gs::getTrackInfo(vector<vector<int>>& tracks, HumdrumFile& infile) {
	tracks.resize(5);
	for (int i=0; i<(int)tracks.size(); i++) {
		tracks[i].clear();
	}
	vector<HTp> sstarts;
	infile.getSpineStartList(sstarts);
	int track;

	// fill in tracks[0]: spines before first **kern spine
	for (int i=0; i<(int)sstarts.size(); i++) {
		if (sstarts[i]->isKern()) {
			break;
		}
		track = sstarts[i]->getTrack();
		tracks[0].push_back(track);
	}

	int kcount = 0;

	kcount = 0;
	// Store tracks related to the tenor part:
	for (int i=0; i<(int)sstarts.size(); i++) {
		if (sstarts[i]->isKern()) {
			kcount++;
		}
		if (kcount > 2) {
			break;
		}
		if (kcount < 2) {
			continue;
		}
		track = sstarts[i]->getTrack();
		if (sstarts[i]->isKern()) {
			tracks[1].push_back(track);
		} else {
			tracks[2].push_back(track);
		}
	}

	kcount = 0;
	// Store tracks related to the bass part:
	for (int i=0; i<(int)sstarts.size(); i++) {
		if (sstarts[i]->isKern()) {
			kcount++;
		}
		if (kcount > 1) {
			break;
		}
		if (kcount < 1) {
			continue;
		}
		track = sstarts[i]->getTrack();
		if (sstarts[i]->isKern()) {
			tracks[1].push_back(track);
		} else {
			tracks[2].push_back(track);
		}
	}

	kcount = 0;
	// Store tracks related to the soprano part:
	for (int i=0; i<(int)sstarts.size(); i++) {
		if (sstarts[i]->isKern()) {
			kcount++;
		}
		if (kcount > 4) {
			break;
		}
		if (kcount < 4) {
			continue;
		}
		track = sstarts[i]->getTrack();
		if (sstarts[i]->isKern()) {
			tracks[3].push_back(track);
		} else {
			tracks[4].push_back(track);
		}
	}

	kcount = 0;
	// Store tracks related to the alto part:
	for (int i=0; i<(int)sstarts.size(); i++) {
		if (sstarts[i]->isKern()) {
			kcount++;
		}
		if (kcount > 3) {
			break;
		}
		if (kcount < 3) {
			continue;
		}
		track = sstarts[i]->getTrack();
		if (sstarts[i]->isKern()) {
			tracks[3].push_back(track);
		} else {
			tracks[4].push_back(track);
		}
	}
}



//////////////////////////////
//
// Tool_satb2gs::validateHeader -- Header cannot contain
//   spine manipulators.
//

bool Tool_satb2gs::validateHeader(HumdrumFile& infile) {
	for (int i=0; i<infile.getLineCount(); i++) {
		if (infile[i].isData()) {
			break;
		}
		if (!infile[i].isInterpretation()) {
			continue;
		}
		HTp token = infile.token(i, 0);
		if (token->isExclusive()) {
			continue;
		}
		if (infile[i].isManipulator()) {
			return false;
		}
	}

	return true;
}



// END_MERGE

} // end namespace hum



