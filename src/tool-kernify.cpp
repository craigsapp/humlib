//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Tue Mar 21 14:41:24 PDT 2023
// Last Modified: Fri Aug  2 21:29:19 PDT 2024
// Filename:      tool-kernify.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/tool-kernify.cpp
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Add invisible staff when no staff-like spine (**kern, **mens)
//                is present in the input data.
//

#include "tool-kernify.h"
#include "HumRegex.h"
#include "Convert.h"

using namespace std;

namespace hum {

// START_MERGE


/////////////////////////////////
//
// Tool_kernify::Tool_kernify -- Set the recognized options for the tool.
//

Tool_kernify::Tool_kernify(void) {
	define("f|force=b", "force staff-like spines to be displayed as text");
}



/////////////////////////////////
//
// Tool_kernify::run -- Do the main work of the tool.
//

bool Tool_kernify::run(HumdrumFileSet& infiles) {
	bool status = true;
	for (int i=0; i<infiles.getCount(); i++) {
		status &= run(infiles[i]);
	}
	return status;
}


bool Tool_kernify::run(const string& indata, ostream& out) {
	HumdrumFile infile(indata);
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_kernify::run(HumdrumFile& infile, ostream& out) {
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_kernify::run(HumdrumFile& infile) {
	initialize();
	m_hasDataInterpretations = prepareDataSpines(infile);
	processFile(infile);
	return true;
}



//////////////////////////////
//
// Tool_kernify::initialize -- Setup to do before processing a file.
//

void Tool_kernify::initialize(void) {
	if (getBoolean("force")) {
		m_forceQ = true;
	}
	// do nothing
}



//////////////////////////////
//
// Tool_kernify::processFile -- Analyze an input file.
//

void Tool_kernify::processFile(HumdrumFile& infile) {
	generateDummyKernSpine(infile);
}



//////////////////////////////
//
// Tool_kernify::prepareDataSpines --
// 

bool Tool_kernify::prepareDataSpines(HumdrumFile& infile) {
	vector<HTp> spinestarts;
	infile.getSpineStartList(spinestarts);
	bool output = false;
	for (int i=0; i<(int)spinestarts.size(); i++) {
		output |= prepareDataSpine(spinestarts[i]);
	}
	return output;
}



//////////////////////////////
//
// Tool_kernify::prepareDataSpine -- check to see if *[abcv]data interpretation and if so process
//    and return true;
//

bool Tool_kernify::prepareDataSpine(HTp spinestart) {
	HumRegex hre;
	if (hre.search(spinestart, "^\\*\\*[abcv]data-")) {
		return false;
	}

	string prefix;
	HTp current = spinestart->getNextToken();
	while (current && !current->isData()) {
		if (current->isInterpretation()) {
			if (*current == "*adata") {
				prefix = "adata";
				break;
			} else if (*current == "*bdata") {
				prefix = "bdata";
				break;
			} else if (*current == "*cdata") {
				prefix = "cdata";
				break;
			} else if (*current == "*vdata") {
				prefix = "vdata";
				break;
			}
		}
		current = current->getNextToken();
	}

	if (prefix.empty()) {
		return false;
	}

	string text = "**";
	text += prefix;
	text += "-";
	text += spinestart->substr(2);
	spinestart->setText(text);

	return true;
}



//////////////////////////////
//
// Tool_kernify::generateDummyKernSpine --
//

void Tool_kernify::generateDummyKernSpine(HumdrumFile& infile) {
	vector<HTp> spineStarts;
	infile.getSpineStartList(spineStarts);
	bool hasRecip = false;
	if (spineStarts.empty()) {
		// no spines, so nothing to do
		return;
	}
	for (int i=0; i<(int)spineStarts.size(); i++) {
		if (spineStarts[i]->isStaffLike()) {
			if (!m_forceQ) {
				// No need for a dummy kern spine, so do nothing.
				// later an option can be used to force a dummy
				// kern spine even if there already exists
				return;
			}
		}
		if (spineStarts[i]->isDataType("**recip")) {
			hasRecip = true;
		}
	}

	int striaIndex = -1;
	int clefIndex = -1;
	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].hasSpines()) {
			continue;
		}
		if (infile[i].isData()) {
			break;
		}
		if (!infile[i].isInterpretation()) {
			continue;
		}
		for (int j=0; j<infile[i].getFieldCount(); j++) {
			HTp token = infile.token(i, j);
			if (striaIndex < 0) {
				if (token->compare(0, 6, "*stria") == 0) {
					striaIndex = i;
				}
			}
			if (clefIndex < 0) {
				if (token->compare(0, 6, "*clef") == 0) {
					clefIndex = i;
				}
			}
		}
	}
	if (striaIndex == clefIndex) {
		// Don't show on the same data line.
		striaIndex = -1;
	}

	bool hasDuration = infile.getScoreDuration() > 0 ? true : false;

	HumRegex hre;
	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].hasSpines()) {
			m_humdrum_text << infile[i];
		} else if (infile[i].isExclusiveInterpretation()) {
			m_humdrum_text << "**kern";
			for (int j=infile[i].getFieldCount()-1; j>=0; j--) {
				HTp token = infile.token(i, j);
				if (*token == "**recip") {
					m_humdrum_text << "\t**xrecip";
				} else if (token->find("**kern") != std::string::npos) {
					string value = *token;
					hre.replaceDestructive(value, "nrek", "kern", "g");
					hre.replaceDestructive(value, "**zcdata-", "^\\*\\*");
					m_humdrum_text << "\t" << value;
				} else if (token->find("**mens") != std::string::npos) {
					string value = *token;
					hre.replaceDestructive(value, "snem", "mens", "g");
					hre.replaceDestructive(value, "**ycdata-", "^\\*\\*");
					m_humdrum_text << "\t" << value;
				} else if (token->find("**cdata") == std::string::npos) {
					if (!m_hasDataInterpretations) {
						string value = token->substr(2);
						hre.replaceDestructive(value, "nrek", "kern", "g");
						hre.replaceDestructive(value, "snem", "snem", "g");
						m_humdrum_text << "\t**xcdata-" << value;
					} else {
						m_humdrum_text << "\t" << token;
					}
				} else {
					m_humdrum_text << "\t" << token;
				}
			}
			if (striaIndex < 0) {
				m_humdrum_text << endl << "*stria0" << "\t" << makeNullLine(infile[i]);
			}
			if (clefIndex < 0) {
				m_humdrum_text << endl << "*clefXyy" << "\t" << makeNullLine(infile[i]);
			}
		} else if (infile[i].isManipulator()) {
			if (*infile[i].token(0) == "*-") {
				m_humdrum_text << "*-";
			} else {
				m_humdrum_text << "*";
			}
			m_humdrum_text << "\t" << makeReverseLine(infile[i]);
		} else if (infile[i].isBarline()) {
			m_humdrum_text << infile[i].token(0) << "\t" << makeReverseLine(infile[i]);
		} else if (infile[i].isData()) {
			if (hasRecip) {
				for (int j=0; j<infile[i].getFieldCount(); j++) {
					HTp token = infile.token(i, j);
					if (!token->isDataType("**recip")) {
						continue;
					}
					m_humdrum_text << token << "ryy\t" << makeReverseLine(infile[i]);
					break;
				}
			} else {
				if (!hasDuration) {
					m_humdrum_text << "4ryy" << "\t" << makeReverseLine(infile[i]);
				} else {
					HumNum duration = infile[i].getDuration();
					string recip;
					if (duration == 0) {
						recip = "q";
					} else {
						recip = Convert::durationToRecip(duration);
					}

					m_humdrum_text << recip << "ryy\t" << makeReverseLine(infile[i]);
				}
			}
		} else if (infile[i].isCommentLocal()) {
				m_humdrum_text << "!" << "\t" << makeReverseLine(infile[i]);
		} else if (infile[i].isInterpretation()) {
				if (striaIndex == i) {
					m_humdrum_text << "*stria0" << "\t" << makeReverseLine(infile[i]);
					striaIndex = -1;
				} else if (clefIndex == i) {
					m_humdrum_text << "*clefXyy" << "\t" << makeReverseLine(infile[i]);
					clefIndex = -1;
				} else {
					HTp token = infile[i].token(0);
					if (token->compare(0, 2, "*>") == 0) {
						m_humdrum_text << token << "\t" << makeReverseLine(infile[i]);
					} else {
						m_humdrum_text << "*" << "\t" << makeReverseLine(infile[i]);
					}
				}
		} else {
			m_humdrum_text << "!!UNKNONWN LINE TYPE FOR LINE " << i+1 << ":\t" << infile[i];
		}
		m_humdrum_text << endl;
	}
}



//////////////////////////////
//
// Tool_kernify::makeNullLine --
//

string Tool_kernify::makeNullLine(HumdrumLine& line) {
	string output;
	for (int i=0; i<line.getFieldCount(); i++) {
		output += "*";
		if (i < line.getFieldCount() - 1) {
			output += "\t";
		}
	}
	return output;
}



//////////////////////////////
//
// Tool_kernify::makeReverseLine --
//

string Tool_kernify::makeReverseLine(HumdrumLine& line) {
	string output;
	for (int i=line.getFieldCount() - 1; i>= 0; i--) {
		output += *line.token(i);
		if (i > 0) {
			output += "\t";
		}
	}
	return output;
}



// END_MERGE

} // end namespace hum



