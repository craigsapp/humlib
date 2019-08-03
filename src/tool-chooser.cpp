//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sun Jul 14 01:03:07 CEST 2019
// Last Modified: Sun Jul 14 01:03:12 CEST 2019
// Filename:      tool-chooser.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/tool-chooser.cpp
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Generate chooser rhythm spine for music.
//

#include "tool-chooser.h"
#include "HumRegex.h"
#include "Convert.h"

using namespace std;

namespace hum {

// START_MERGE


/////////////////////////////////
//
// Tool_chooser::Tool_chooser -- Set the recognized options for the tool.
//

Tool_chooser::Tool_chooser(void) {
	define("s|segment=s",  "segments to pass to output");
}



/////////////////////////////////
//
// Tool_chooser::run -- Do the main work of the tool.
//


bool Tool_chooser::run(const string& indata) {
	initialize();
	HumdrumFileStream instream;
	instream.loadString(indata);
	HumdrumFileSet infiles;
	infiles.read(instream);
	processFiles(infiles);
	return true;
}



bool Tool_chooser::run(HumdrumFileStream& instream) {
	initialize();
	HumdrumFileSet infiles;
	infiles.read(instream);
	processFiles(infiles);
	return true;
}



//////////////////////////////
//
// Tool_chooser::initialize --
//

void Tool_chooser::initialize(void) {
	// do nothing
}



//////////////////////////////
//
// Tool_chooser::processFiles --
//

void Tool_chooser::processFiles(HumdrumFileSet& infiles) {
	int maximum = infiles.getCount();
	string expansion = getString("segment");
	vector<int> outlist = Convert::extractIntegerList(expansion, maximum);

	for (int i=0; i<(int)outlist.size(); i++) {
		m_humdrum_text << infiles[outlist[i]-1];
	}
}


// END_MERGE

} // end namespace hum



