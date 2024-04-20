//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Jun 17 15:24:23 CEST 2017
// Last Modified: Sat Jul  8 17:17:21 CEST 2017
// Filename:      tool-humsort.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/tool-humsort.cpp
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Sort data spines in a Humdrum file.
//

#include "tool-humsort.h"
#include "Convert.h"
#include "HumRegex.h"

#include <algorithm>
#include <cmath>
#include <functional>

using namespace std;

namespace hum {

// START_MERGE


/////////////////////////////////
//
// Tool_humsort::Tool_humsort -- Set the recognized options for the tool.
//

Tool_humsort::Tool_humsort(void) {
	// add options here
	define("n|numeric=b",                             "sort numerically");
	define("r|reverse=b",                             "sort in reversed order");
	define("s|spine=i:1",                             "spine to sort (1-indexed)");
	define("I|do-not-ignore-case=b",                  "do not ignore case when sorting alphabetically");
	define("i|e|x|interp|exclusive-interpretation=s", "exclusive interpretation to sort");
}



/////////////////////////////////
//
// Tool_humsort::run -- Do the main work of the tool.
//

bool Tool_humsort::run(HumdrumFileSet& infiles) {
	bool status = true;
	for (int i=0; i<infiles.getCount(); i++) {
		status &= run(infiles[i]);
	}
	return status;
}


bool Tool_humsort::run(const string& indata, ostream& out) {
	HumdrumFile infile(indata);
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_humsort::run(HumdrumFile& infile, ostream& out) {
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_humsort::run(HumdrumFile& infile) {
	processFile(infile);
	return true;
}



//////////////////////////////
//
// Tool_humsort::processFile --
//

void Tool_humsort::processFile(HumdrumFile& infile) {
	vector<HTp> sstarts;
	infile.getSpineStartList(sstarts);
	int spine = getInteger("spine");
	if (getBoolean("exclusive-interpretation")) {
		string datatype = getString("exclusive-interpretation");
		if (datatype.compare(0, 2, "**")) {
			datatype = "**" + datatype;
		} else if (datatype.compare(0, 1, "*")) {
			datatype = "*" + datatype;
		}
		for (int i=0; i<(int)sstarts.size(); i++) {
			if (sstarts[i]->isDataType(datatype)) {
				spine = sstarts[i]->getTrack();
				break;
			}
		}
	}
	vector<HTp> data;
	data.reserve(infile.getLineCount());
	HTp current = sstarts.at(spine-1);
	current = current->getNextToken();
	while (current) {
		if (current->isData()) {
			data.push_back(current);
		}
		current = current->getNextToken();
	}

	if (getBoolean("numeric")) {
		std::sort(data.begin(), data.end(),
			[](HTp a, HTp b) {
				if (*a == *b) {
					return 0;
				}
				if (*a == ".") {
					return -1;
				}
				if (*b == ".") {
					return 0;
				}
				char cha = a->at(0);
				char chb = b->at(0);
				if ((isdigit(cha) || cha == '-' || cha == '+' || cha == '.') &&
				    (isdigit(chb) || chb == '-' || chb == '+' || chb == '.')) {
					int A = stod(*a);
					int B = stod(*b);
					if (A < B) {
						return -1;
					} else {
						return 0;
					}
				}
				// one value is not a number for some reason, so compare as string
				return *a < *b ? -1 : 0;
			});
	} else {
		// alphabetic sorting
		if (!getBoolean("do-not-ignore-case")) {
			std::sort(data.begin(), data.end(), [](HTp a, HTp b) {
					string A = *a;
					string B = *b;
					std::transform(A.begin(), A.end(), A.begin(), ::tolower);
					std::transform(B.begin(), B.end(), B.begin(), ::tolower);
					return A < B;
			});
		} else {
			std::sort(data.begin(), data.end(),
				[](HTp a, HTp b) { return *a < *b; });
		}
	}

	for (int i=0; i<infile.getLineCount(); i++) {
		if (infile[i].hasSpines()) {
			m_humdrum_text << infile[i] << endl;
			break;
		}
		m_humdrum_text << infile[i] << endl;
	}
	if (getBoolean("reverse")) {
		for (int i=(int)data.size()-1; i>=0; i--) {
			m_humdrum_text << data[i]->getOwner() << endl;
		}
	} else {
		for (int i=0; i<(int)data.size(); i++) {
			m_humdrum_text << data[i]->getOwner() << endl;
		}
	}
	for (int i=0; i<infile.getLineCount(); i++) {
		if (*infile[i].token(0) != "*-") {
			continue;
		}
		for (int j=i; j<infile.getLineCount(); j++) {
			m_humdrum_text << infile[j] << endl;
		}
	}
}





// END_MERGE

} // end namespace hum



