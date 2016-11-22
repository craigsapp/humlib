//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Tue Nov 22 13:13:13 PST 2016
// Last Modified: Tue Nov 22 13:13:16 PST 2016
// Filename:      HumdrumFileContent-spine.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/HumdrumFileContent-tie.cpp
// Syntax:        C++11
// vim:           ts=3 noexpandtab
//
// Description:   Functions related to working with HumdrumFile spines.
//

#include "HumdrumFileContent.h"
#include <sstream>

using namespace std;

namespace hum {

// START_MERGE


//////////////////////////////
//
// HumdrumFileContent::prependDataSpine -- prepend a data spine
//     to the file.  Returns true if successful; false otherwise.
//
//     data == numeric or string data to print 
//     null == if the data is converted to a string is equal to this
//             string then set the data spine content to a null token, ".".
//             default is ".".
//     exinterp == the exterp string to use.  Default is "**data".
//

template <class DATATYPE>
bool HumdrumFileContent::prependDataSpine(vector<DATATYPE> data,
		const string& null, const string& exinterp) {

	if ((int)data.size() != getLineCount()) {
		return false;
	}

	string ex;
	if (exinterp.find("**") == 0) {
		ex = exinterp;
	} else if (exinterp.find("*") == 0) {
		ex = "*" + exinterp;
	} else {
		ex = "**" + exinterp;
	}
	if (ex.size() <= 2) {
		ex += "data";
	}

	stringstream ss;
	HumdrumFileContent& infile = *this;
	HumdrumLine* line;
	for (int i=0; i<infile.getLineCount(); i++) {
		line = infile.getLine(i);
		if (!line->hasSpines()) {
			continue;
		}
		if (line->isExclusive()) {
			line->insertToken(0, ex);
			continue;
		} else if (line->isTerminator()) {
			line->insertToken(0, "*-");
			continue;
		} else if (line->isInterpretation()) {
			line->insertToken(0, "*");
		} else if (line->isLocalComment()) {
			line->insertToken(0, "!");
		} else if (line->isBarline()) {
			line->insertToken(0, (string)*infile.token(i, 0));
		} else if (line->isData()) {
			ss.str(string());
			ss << data[i];
			if (ss.str() == null) {
				line->insertToken(0, ".");
			} else if (ss.str() == "") {
				line->insertToken(0, ".");
			} else {
				line->insertToken(0, ss.str());
			}
		} else{
			cerr << "!!strange error for line " << i+1 << ":\t" << line << endl;
		}
	}
	return true;
}



//////////////////////////////
//
// HumdrumFileContent::appendDataSpine -- prepend a data spine
//     to the file.  Returns true if successful; false otherwise.
//
//     data == numeric or string data to print 
//     null == if the data is converted to a string is equal to this
//             string then set the data spine content to a null token, ".".
//             default is ".".
//     exinterp == the exterp string to use.  Default is "**data".
//

template <class DATATYPE>
bool HumdrumFileContent::appendDataSpine(vector<DATATYPE> data,
		const string& null, const string& exinterp) {

	if ((int)data.size() != getLineCount()) {
		return false;
	}

	string ex;
	if (exinterp.find("**") == 0) {
		ex = exinterp;
	} else if (exinterp.find("*") == 0) {
		ex = "*" + exinterp;
	} else {
		ex = "**" + exinterp;
	}
	if (ex.size() <= 2) {
		ex += "data";
	}

	stringstream ss;
	HumdrumFileContent& infile = *this;
	HumdrumLine* line;
	for (int i=0; i<infile.getLineCount(); i++) {
		line = infile.getLine(i);
		if (!line->hasSpines()) {
			continue;
		}
		if (line->isExclusive()) {
			line->appendToken(ex);
			continue;
		} else if (line->isTerminator()) {
			line->appendToken("*-");
			continue;
		} else if (line->isInterpretation()) {
			line->appendToken("*");
		} else if (line->isLocalComment()) {
			line->appendToken("!");
		} else if (line->isBarline()) {
			line->appendToken((string)*infile.token(i, 0));
		} else if (line->isData()) {
			ss.str(string());
			ss << data[i];
			if (ss.str() == null) {
				line->appendToken(".");
			} else if (ss.str() == "") {
				line->appendToken(".");
			} else {
				line->appendToken(ss.str());
			}
		} else{
			cerr << "!!strange error for line " << i+1 << ":\t" << line << endl;
		}
	}
	return true;
}


// END_MERGE

} // end namespace hum



