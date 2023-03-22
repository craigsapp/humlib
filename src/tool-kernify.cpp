//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Tue Mar 21 14:41:24 PDT 2023
// Last Modified: Tue Mar 21 14:41:30 PDT 2023
// Filename:      tool-kernify.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/tool-kernify.cpp
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Add invisible staff when no staff-like spine (**kern, **mens)
//                is present in the input data.
//

#include "tool-kernify.h"

using namespace std;

namespace hum {

// START_MERGE


/////////////////////////////////
//
// Tool_kernify::Tool_kernify -- Set the recognized options for the tool.
//

Tool_kernify::Tool_kernify(void) {
	// add options here
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
	processFile(infile);
	return true;
}



//////////////////////////////
//
// Tool_kernify::initialize -- Setup to do before processing a file.
//

void Tool_kernify::initialize(void) {
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
			// No need for a dummy kern spine, so do nothing.
			// later an option can be used to force a dummy
			// kern spine even if there already exists
			return;
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

	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].hasSpines()) {
			m_humdrum_text << infile[i];
		} else if (infile[i].isExclusiveInterpretation()) {
			m_humdrum_text << "**kern";
			for (int j=0; j<infile[i].getFieldCount(); j++) {
				HTp token = infile.token(i, j);
				if (*token == "**recip") {
					m_humdrum_text << "\t**xrecip";
				} else if (token->find("**cdata") == std::string::npos) {
					m_humdrum_text << "\t**cdata-" << token->substr(2);
				} else {
					m_humdrum_text << "\t" << infile[i];
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
			m_humdrum_text << "\t" << infile[i];
		} else if (infile[i].isBarline()) {
			m_humdrum_text << infile[i].token(0) << "\t" << infile[i];
		} else if (infile[i].isData()) {
			if (hasRecip) {
				for (int j=0; j<infile[i].getFieldCount(); j++) {
					HTp token = infile.token(i, j);
					if (!token->isDataType("**recip")) {
						continue;
					}
					m_humdrum_text << token << "ryy\t" << infile[i];
					break;
				}
			} else {
				m_humdrum_text << "4ryy" << "\t" << infile[i];
			}
		} else if (infile[i].isCommentLocal()) {
				m_humdrum_text << "!" << "\t" << infile[i];
		} else if (infile[i].isInterpretation()) {
				if (striaIndex == i) {
					m_humdrum_text << "*stria0" << "\t" << infile[i];
					striaIndex = -1;
				} else if (clefIndex == i) {
					m_humdrum_text << "*clefXyy" << "\t" << infile[i];
					clefIndex = -1;
				} else {
					m_humdrum_text << "*" << "\t" << infile[i];
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



// END_MERGE

} // end namespace hum



