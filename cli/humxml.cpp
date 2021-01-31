//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Fri Jan 29 18:58:32 PST 2021
// Last Modified: Sun Jan 31 01:22:56 PST 2021
// Filename:      humxml.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/cli/humxml.cpp
// Syntax:        C++11
// vim:           ts=3 noexpandtab nowrap
//
// Description:   Reveal data structure of Humdrum file and internal parameters.
//                Optionally run various analyses of the data to calculate
//                content-based parameters.
//

#include "humlib.h"

using namespace std;
using namespace hum;

int main(int argc, char **argv) {
	Options options;
	options.define("A|all=b",              "do all analyses");
	options.define("a|accidentals=b",      "analyze visual accidental states");
	options.define("p|phrases=b",          "analyze phrases");
	options.define("r|rest-positions=b",   "analyze rest positions");
	options.define("s|slurs=b",            "analyze slurs");
	options.define("t|ties=b",             "analyze ties");
	options.define("x|text-repetitions=b", "analyze text repetitions");

	// Processing not run automatically with -A option:

	options.process(argc, argv);

	bool allQ = options.getBoolean("all");

	HumdrumFile infile;
	if (options.getArgCount() == 0) {
		infile.read(cin);
	} else {
		infile.read(options.getArg(1));
	}

	// Do content analysis of the data (mostly **kern data):
	if (allQ || options.getBoolean("accidentals")) {
		// This analysis calculates the visible state of
		// accidentals for **kern data notes for conversion
		// to graphical CMN music notation. **kern data
		// encodes the sounding accidentals.  These may
		// be shown in notation or hidden due to the 
		// key signature or previous notes in the measure,
		// or modified by !LO:N:acc parameters. An "X" after
		// an accidental will force it to be displayed, and
		// a "y" after an accidental will force it to
		// be hidden.
		infile.analyzeKernAccidentals();
	}
	if (allQ || options.getBoolean("phrases")) {
		// Link phrase endings to each other.  Phrase start
		// is "{" and phrase end is "}".  Phrase markings
		// are typically for analytic purposes, with slurs
		// usually used for phasing slurs.  Phrases that cross
		// other phrases are prefixed with one or more "&".
		infile.analyzeSlurs();
	}
	if (allQ || options.getBoolean("rest-positions")) {
		// Specify the vertical position of rests on the staff.
		infile.analyzeRestPositions();
	}
	if (allQ || options.getBoolean("slurs")) {
		// Link slur endings to each other.  Slur start is
		// "(" and slur end is ")".  Slurs that cross other
		// active slurs are prefixed with one or more "&".
		infile.analyzeSlurs();
	}
	if (allQ || options.getBoolean("ties")) {
		// Link tie endings to each other.  Tie starts
		// are "[", tie continuations (a end and a start on the
		// same note are "_", and tie ends are "]".  When the
		// tie character is doubled, then that is a discontinuous
		// tie, where the two tied notes are not melocially adjacent
		// (this can happen in music to simplify the notatation
		// of written out arpeggiations).
		// This analysis seems to not generate HumHash data, but
		// proably adjusted fixed variables for HumdrumTokens.
		infile.analyzeKernTies();
	}
	if (allQ || options.getBoolean("text-repetitions")) {
		// Something to do with **text text-repeat markers.
		infile.analyzeTextRepetition();
	}

	infile.printXml();
}



