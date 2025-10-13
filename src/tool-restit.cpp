//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Fri Oct 10 09:56:34 PDT 2025
// Last Modified: Mon Oct 13 09:07:29 PDT 2025
// Filename:      tool-restit.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/tool-restit.cpp
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Switch between restit encoding and corrected encoding.
//

#include "tool-restit.h"
#include "Convert.h"
#include "HumRegex.h"

using namespace std;

namespace hum {

// START_MERGE


/////////////////////////////////
//
// Tool_restit::Tool_restit -- Set the recognized options for the tool.
//

Tool_restit::Tool_restit(void) {
	define("k|kern=s", "process only the given spines");

}



/////////////////////////////////
//
// Tool_restit::run -- Do the main work of the tool.
//

bool Tool_restit::run(HumdrumFileSet& infiles) {
	bool status = true;
	for (int i=0; i<infiles.getCount(); i++) {
		status &= run(infiles[i]);
	}
	return status;
}


bool Tool_restit::run(const string& indata, ostream& out) {
	HumdrumFile infile(indata);
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_restit::run(HumdrumFile& infile, ostream& out) {
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_restit::run(HumdrumFile& infile) {
	initialize(infile);
	processFile(infile);
	return true;
}



//////////////////////////////
//
// Tool_restit::initialize --  Initializations that only have to be done once
//    for all HumdrumFile segments.
//

void Tool_restit::initialize(HumdrumFile& infile) {
cerr << "one" << endl;
	if (getBoolean("kern")) {
cerr << "two" << endl;
      string klist = getString("kern");
cerr << "three" << endl;
		cout << "KLIST = " << klist << endl;
cerr << "four" << endl;
   }
cerr << "five" << endl;

}



//////////////////////////////
//
// Tool_restit::processFile --
//

void Tool_restit::processFile(HumdrumFile& infile) {
	vector<HTp> starts;
	infile.getSpineStartList(starts);
	for (int i=0; i<(int)starts.size(); i++) {
		cout << starts[i] << endl;
	}

	if (m_modifiedQ) {
		infile.createLinesFromTokens();
	}
	m_humdrum_text << infile;
}




// END_MERGE

} // end namespace hum



