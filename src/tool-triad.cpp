//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Tue May  5 05:36:55 PDT 2026
// Last Modified: Sat May  9 21:02:00 PDT 2026
// Filename:      tool-triad.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/tool-triad.cpp
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Analyze triad encoding and corrected encoding.
//

#include "tool-triad.h"
#include "Convert.h"
#include "HumRegex.h"

using namespace std;

namespace hum {

// START_MERGE


/////////////////////////////////
//
// Tool_triad::Tool_triad -- Set the recognized options for the tool.
//

Tool_triad::Tool_triad(void) {
	define("a|append=b",       "Add analysis at the end of the line");
	define("c|pc|class|pitch-class=b",  "Display pitch classes");
	define("p|pitches=b",      "Display pitches");
	define("R|rest=b",         "Display rest rather than null token");
	define("s|summary=b",      "Display summary table");
	define("r|root=b",         "Display root");
}



/////////////////////////////////
//
// Tool_triad::run -- Do the main work of the tool.
//

bool Tool_triad::run(HumdrumFileSet& infiles) {
	bool status = true;
	for (int i=0; i<infiles.getCount(); i++) {
		status &= run(infiles[i]);
	}
	return status;
}


bool Tool_triad::run(const string& indata, ostream& out) {
	HumdrumFile infile(indata);
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_triad::run(HumdrumFile& infile, ostream& out) {
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}



bool Tool_triad::run(HumdrumFile& infile) {
	initialize();
	processFile(infile);
	return true;
}



//////////////////////////////
//
// Tool_triad::initialize --  Initializations that only have to be done once
//    for all HumdrumFile segments.
//

void Tool_triad::initialize(void) {
	m_appendQ   = getBoolean("append");
	m_prependQ  = !m_appendQ;
	m_summaryQ  = getBoolean("summary");
	m_classQ    = getBoolean("pitch-class");
	m_pitchesQ  = getBoolean("pitches");
	m_rootQ     = getBoolean("root");
}



/////////////////////////////
//
// Tool_triad::processFile --
//

void Tool_triad::processFile(HumdrumFile& infile) {
	string quality;
	string root;
	string inversion;

	for (int i=0; i<infile.getLineCount(); i++) {

		// Preserve global/local comments exactly.
		if (!infile[i].hasSpines()) {
			m_humdrum_text << infile[i] << endl;
			continue;
		}

		quality.clear();
		root.clear();
		inversion.clear();

		string token = infile[i].getTriadicQuality(
			infile,
			i,
			quality,
			root,
			inversion,
			m_pitchesQ,
			m_classQ,
			getBoolean("rest")
		);

		// Construct analysis token for data lines.
		if (token.empty()) {

			token = quality;

			if (!root.empty()) {
				token += "(";
				token += root;

				if (!inversion.empty()) {
					token += "/";
					token += inversion;
				}

				token += ")";
			}
		}

		// Ignore hidden comment marker.
		if (token == "!!") {
			m_humdrum_text << infile[i] << endl;
			continue;
		}

		// Prepend analysis spine.
		if (m_prependQ) {
			m_humdrum_text << token << "\t" << infile[i] << endl;
		}

		// Append analysis spine.
		else {
			m_humdrum_text << infile[i] << "\t" << token << endl;
		}
	}
}



// END_MERGE

} // end namespace hum
