//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Mar 13 23:45:00 PDT 2023
// Last Modified: Fri Mar 17 12:36:19 PDT 2023
// Filename:      tool-ordergps.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/tool-ordergps.cpp
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Make *group, *part, and *staff interpretations adjacent and in
//                that order.  Only looking before the first data, manipulator or
//                local comment line.
//

#include "tool-ordergps.h"

using namespace std;

namespace hum {

// START_MERGE


/////////////////////////////////
//
// Tool_ordergps::Tool_ordergps -- Set the recognized options for the tool.
//

Tool_ordergps::Tool_ordergps(void) {
	define("e|empty=b",   "list files that have no group/part/staff (used with -p option).");
	define("f|file=b",    "list input files only.");
	define("l|list=b",    "list files that will be changed.");
	define("p|problem=b", "list files that have mixed content for *group, *part, *staff info.");
	define("r|reverse=b", "order *staff, *part, *group");
	define("s|staff=b",   "Add staff line if none present already in score.");
	define("t|top=b",     "Place group/part/staff lines first after exinterp.");
}



/////////////////////////////////
//
// Tool_ordergps::run -- Do the main work of the tool.
//

bool Tool_ordergps::run(HumdrumFileSet& infiles) {
	bool status = true;
	for (int i=0; i<infiles.getCount(); i++) {
		status &= run(infiles[i]);
	}
	return status;
}


bool Tool_ordergps::run(const string& indata, ostream& out) {
	HumdrumFile infile(indata);
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_ordergps::run(HumdrumFile& infile, ostream& out) {
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_ordergps::run(HumdrumFile& infile) {
	initialize();
	processFile(infile);
	return true;
}



//////////////////////////////
//
// Tool_ordergps::initialize -- Setup to do before processing a file.
//

void Tool_ordergps::initialize(void) {
	m_emptyQ   = getBoolean("empty");
	m_fileQ    = getBoolean("file");
	m_listQ    = getBoolean("list");
	m_problemQ = getBoolean("problem");
	m_reverseQ = getBoolean("reverse");
	m_staffQ   = getBoolean("staff");
	m_topQ     = getBoolean("top");
}



//////////////////////////////
//
// Tool_ordergps::processFile -- Analyze an input file.
//

void Tool_ordergps::processFile(HumdrumFile& infile) {
	vector<int> groupIndex;
	vector<int> partIndex;
	vector<int> staffIndex;
	bool foundProblem = false;

	for (int i=0; i<infile.getLineCount(); i++) {
		if (infile[i].isData()) {
			break;
		}
		if (infile[i].isManipulator()) {
			// Don't deall with spine splits/mergers/exchanges/additions.
			if (!infile[i].isExclusiveInterpretation()) {
				break;
			}
		}
		if (infile[i].isCommentLocal()) {
			// Don't process after local comments.   The file header
			// is too complex perhaps, so do not alter anything
			// after the local comment.  This can be related to modori
			// assignment for groups, for example.
			break;
		}
		if (!infile[i].hasSpines()) {
			continue;
		}
		if (infile[i].isExclusiveInterpretation()) {
			continue;
		}
		if (!infile[i].isInterpretation()) {
			continue;
		}
		int hasGroup = false;
		int hasPart  = false;
		int hasStaff = false;
		int hasOther = false;
		for (int j=0; j<infile[i].getFieldCount(); j++) {
			HTp token = infile.token(i, j);
			if (*token == "*") {
				continue;
			}
			if (token->compare(0, 6, "*group") == 0) {
				hasGroup = true;
			} else if (token->compare(0, 5, "*part") == 0) {
				hasPart = true;
			} else if (token->compare(0, 6, "*staff") == 0) {
				hasStaff = true;
			} else {
				hasOther = true;
			}
		}

		if (hasOther && hasGroup) {
			foundProblem = true;
			if (m_problemQ) {
				cerr << infile.getFilename() << " HAS MIXED GROUP LINE:" << endl;
				cerr << "\t" << infile[i] << endl;
			}
		}

		if (hasOther && hasPart) {
			foundProblem = true;
			if (m_problemQ) {
				cerr << infile.getFilename() << " HAS MIXED PART LINE:" << endl;
				cerr << "\t" << infile[i] << endl;
			}
		}

		if (hasOther && hasStaff) {
			foundProblem = true;
			if (m_problemQ) {
				cerr << infile.getFilename() << " HAS MIXED STAFF LINE:" << endl;
				cerr << "\t" << infile[i] << endl;
			}
		}

		if (hasOther) {
			continue;
		}

		if (hasGroup && hasPart) {
			foundProblem = true;
			if (m_problemQ) {
				cerr << infile.getFilename() << " HAS MIXED GROUP AND PART LINE:" << endl;
				cerr << "\t" << infile[i] << endl;
			}
		}

		if (hasGroup && hasStaff) {
			foundProblem = true;
			if (m_problemQ) {
				cerr << infile.getFilename() << " HAS MIXED GROUP AND STAFF LINE:" << endl;
				cerr << "\t" << infile[i] << endl;
			}
		}

		if (hasPart && hasStaff) {
			foundProblem = true;
			if (m_problemQ) {
				cerr << infile.getFilename() << " HAS MIXED PART AND STAFF LINE:" << endl;
				cerr << "\t" << infile[i] << endl;
			}
		}

		if (hasGroup) {
			groupIndex.push_back(i);
		}

		if (hasPart)  {
			partIndex.push_back(i);
		}

		if (hasStaff) {
			staffIndex.push_back(i);
		}
	}

	if (groupIndex.size() > 1) {
		foundProblem = true;
		if (m_problemQ) {
			cerr << infile.getFilename() << " HAS MORE THAN ONE GROUP LINE:" << endl;
			for (int i=0; i<(int)groupIndex.size(); i++) {
				cerr << "\t" << infile[groupIndex[i]] << endl;
			}
		}
	}

	if (partIndex.size() > 1) {
		foundProblem = true;
		if (m_problemQ) {
			cerr << infile.getFilename() << " HAS MORE THAN ONE PART LINE:" << endl;
			for (int i=0; i<(int)partIndex.size(); i++) {
				cerr << "\t" << infile[partIndex[i]] << endl;
			}
		}
	}

	if (staffIndex.size() > 1) {
		foundProblem = true;
		if (m_problemQ) {
			cerr << infile.getFilename() << " HAS MORE THAN ONE STAFF LINE:" << endl;
			for (int i=0; i<(int)staffIndex.size(); i++) {
				cerr << "\t" << infile[staffIndex[i]] << endl;
			}
		}
	}

	if (m_problemQ) {
		if (m_emptyQ) {
			if (groupIndex.empty() && partIndex.empty() && staffIndex.empty()) {
				cerr << infile.getFilename() << " HAS NO GROUP/PART/STAFF INFO" << endl;
			}
		}
	} else {
		if (foundProblem) {
			// Don try to fix anything, just echo the input:
			m_humdrum_text << infile;
		} else {
			if (m_staffQ && groupIndex.empty() && partIndex.empty() && staffIndex.empty()) {
				printStaffLine(infile);
			} else {
				// Process further here
				// Check the order of the group/part/staff lines.
				int gindex = groupIndex.empty() ? -1 : groupIndex.at(0);
				int pindex = partIndex.empty() ? -1 : partIndex.at(0);
				int sindex = staffIndex.empty() ? -1 : staffIndex.at(0);
				if (m_topQ) {
					printFileTop(infile, gindex, pindex, sindex);
				} else {
					printFile(infile, gindex, pindex, sindex);
				}
			}
		}
	}
}



//////////////////////////////
//
// Tool_ordergps::printFileTop -- Print group/part/staff first after exclusive
//     interpretations.
//

void Tool_ordergps::printFileTop(HumdrumFile& infile, int gindex, int pindex, int sindex) {
	for (int i=0; i<infile.getLineCount(); i++) {
		if (i == gindex) {
			continue;
		} else if (i == pindex) {
			continue;
		} else if (i == sindex) {
			continue;
		} else if (infile[i].isExclusiveInterpretation()) {
			m_humdrum_text << infile[i] << endl;
			if (m_reverseQ) {
				if (sindex >= 0) {
					m_humdrum_text << infile[sindex] << endl;
				}
				if (pindex >= 0) {
					m_humdrum_text << infile[pindex] << endl;
				}
				if (gindex >= 0) {
					m_humdrum_text << infile[gindex] << endl;
				}
			} else {
				if (gindex >= 0) {
					m_humdrum_text << infile[gindex] << endl;
				}
				if (pindex >= 0) {
					m_humdrum_text << infile[pindex] << endl;
				}
				if (sindex >= 0) {
					m_humdrum_text << infile[sindex] << endl;
				}
			}
		} else {
			m_humdrum_text << infile[i] << endl;
		}
	}
}



//////////////////////////////
//
// Tool_ordergps::printFile -- Check to see if the group/part/staff
//    lines need to be adjusted, and the print the file.  Lines
//    will be ordered group/part/staff, placing the lines where
//    the first of group/part/staff is found.
//

void Tool_ordergps::printFile(HumdrumFile& infile, int gindex, int pindex, int sindex) {
	int startIndex = gindex;
	if (pindex >= 0) {
		if (startIndex < 0) {
			startIndex = pindex;
		} else if (pindex < startIndex) {
			startIndex = pindex;
		}
	}
	if (sindex >= 0) {
		if (startIndex < 0) {
			startIndex = sindex;
		} else if (sindex < startIndex) {
			startIndex = sindex;
		}
	}
	if (startIndex < 0) {
		// no group/part/staff lines in file, so just print it:
		m_humdrum_text << infile;
		return;
	}

	for (int i=0; i<infile.getLineCount(); i++) {
		if (i == startIndex) {
			if (m_reverseQ) {
				if (sindex >= 0) {
					m_humdrum_text << infile[sindex] << endl;
				}
				if (pindex >= 0) {
					m_humdrum_text << infile[pindex] << endl;
				}
				if (gindex >= 0) {
					m_humdrum_text << infile[gindex] << endl;
				}
			} else {
				if (gindex >= 0) {
					m_humdrum_text << infile[gindex] << endl;
				}
				if (pindex >= 0) {
					m_humdrum_text << infile[pindex] << endl;
				}
				if (sindex >= 0) {
					m_humdrum_text << infile[sindex] << endl;
				}
			}
		} else if (i == gindex) {
			continue;
		} else if (i == pindex) {
			continue;
		} else if (i == sindex) {
			continue;
		} else {
			m_humdrum_text << infile[i] << endl;
		}
	}
}



//////////////////////////////
//
// Tool_ordergps::printStaffLine --  Add a *staff at the start of the
//     data since none was detected.  Does not label staff-like spines
//     other than **kern (such as **kernyy, **kern-mod, **mens).

void Tool_ordergps::printStaffLine(HumdrumFile& infile) {
	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isExclusiveInterpretation()) {
			m_humdrum_text << infile[i] << endl;
			continue;
		}
		m_humdrum_text << infile[i] << endl;
		vector<string> staffLine(infile[i].getFieldCount(), "*");
		int counter = 0;
		for (int j=infile[i].getFieldCount() - 1; j>=0; j--) {
			HTp token = infile.token(i, j);
			if (token->isKern()) {
				counter++;
				string text = "*staff" + to_string(counter);
				staffLine.at(j) = text;
			}
		}
		for (int j=0; j<(int)staffLine.size(); j++) {
			m_humdrum_text << staffLine[j];
			if (j < (int)staffLine.size() - 1) {
				m_humdrum_text << '\t';
			}
		}
		m_humdrum_text << endl;
	}
}


// END_MERGE

} // end namespace hum



