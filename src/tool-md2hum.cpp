//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Wed Sep 25 19:23:06 PDT 2019
// Last Modified: Wed Sep 25 19:23:08 PDT 2019
// Filename:      md2hum.cpp
// URL:           https://github.com/craigsapp/hum2ly/blob/master/src/md2hum.cpp
// Syntax:        C++11; humlib
// vim:           ts=3:noexpandtab
//
// Description:   Convert a MusicXML file into a Humdrum file.
//

#include "tool-md2hum.h"
#include "Convert.h"
#include "HumGrid.h"

using namespace std;
using namespace pugi;

namespace hum {

// START_MERGE

//////////////////////////////
//
// Tool_md2hum::Tool_md2hum --
//

Tool_md2hum::Tool_md2hum(void) {
	// Options& options = m_options;
	// options.define("k|kern=b","display corresponding **kern data");

	define("r|recip=b", "output **recip spine");
	define("s|stems=b", "include stems in output");
}



//////////////////////////////
//
// initialize --
//

void Tool_md2hum::initialize(void) {
	m_stemsQ = getBoolean("stems");
	m_recipQ = getBoolean("recip");
}



//////////////////////////////
//
// Tool_md2hum::setOptions --
//

void Tool_md2hum::setOptions(int argc, char** argv) {
	m_options.process(argc, argv);
}


void Tool_md2hum::setOptions(const vector<string>& argvlist) {
    m_options.process(argvlist);
}



//////////////////////////////
//
// Tool_md2hum::getOptionDefinitions -- Used to avoid
//     duplicating the definitions in the test main() function.
//

Options Tool_md2hum::getOptionDefinitions(void) {
	return m_options;
}



//////////////////////////////
//
// Tool_md2hum::convert -- Convert a MusicXML file into
//     Humdrum content.
//

bool Tool_md2hum::convertFile(ostream& out, const string& filename) {
	MuseDataSet mds;
	int result = mds.readFile(filename);
	if (!result) {
		cerr << "\nMuseData file [" << filename << "] has syntax errors\n";
		cerr << "Error description:\t" << mds.getError() << "\n";
		exit(1);
	}

	return convert(out, mds);
}


bool Tool_md2hum::convert(ostream& out, istream& input) {
	MuseDataSet mds;
	mds.read(input);
	return convert(out, mds);
}


bool Tool_md2hum::convertString(ostream& out, const string& input) {
	MuseDataSet mds;
	int result = mds.readString(input);
	if (!result) {
		cout << "\nXML content has syntax errors\n";
		cout << "Error description:\t" << mds.getError() << "\n";
		exit(1);
	}
	return convert(out, mds);
}



bool Tool_md2hum::convert(ostream& out, MuseDataSet& mds) {
	initialize();
	int status = 1;

	// int partcount = mds.getPartCount();
	// only first part in file for now:
	MuseData& md = mds[0];
	int tpq = md.getInitialTpq();

	out << "**kern\n";
	for (int i=0; i<md.getLineCount(); i++) {
		printKernInfo(out, md[i], tpq);
	}
	out << "*-\n";

	return status;
}



//////////////////////////////
//
// Tool_md2hum::printKernInfo --
//

void Tool_md2hum::printKernInfo(ostream& out, MuseRecord& mr, int tpq) {
	if (mr.isBarline()) {
		out << mr.getKernMeasureStyle();
		out << endl;
	} else if (mr.isNote()) {
		out << mr.getKernNoteStyle(1, 1);
		out << endl;
	} else if (mr.isRest()) {
		out << mr.getKernRestStyle(tpq);
		out << endl;
	}
}


// END_MERGE

} // end namespace hum


