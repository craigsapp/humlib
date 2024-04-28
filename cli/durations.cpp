//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Mar 12 12:35:38 PST 2022
// Last Modified: Sat Mar 12 12:52:21 PST 2022
// Filename:      cli/durations.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/cli/durations.cpp
// Syntax:        C++11
// vim:           ts=3 noexpandtab nowrap
//
// Description:   Extract list of durations of notes in a score. Durations are in units
//                of quarter notes.  Duration qualifiers appended to output durations:
//                T = the duration contains a group of tied notes
//                R = the duration is for a rest
//                G = Grace note
//
// NB:            Chords are treated as a single entry, and chords with
//                a mix of tied untied notes are not considered separately
//                (the state of the first note in the chord list is the
//                only one considered).
//

#include "humlib.h"

using namespace std;
using namespace hum;

void processFile(HumdrumFile &infile);

int main(int argc, char **argv) {
  Options options;
  options.process(argc, argv);

  HumdrumFileStream instream(options);
  HumdrumFile infile;
  while (instream.read(infile)) {
	 processFile(infile);
  }

  return 0;
}



//////////////////////////////
//
// processFile --
//

void processFile(HumdrumFile &infile) {
	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isData()) {
			continue;
		}
		for (int j=0; j<infile[i].getFieldCount(); j++) {
			HTp token = infile.token(i, j);
			if (!token->isKern()) {
				continue;
			}
			if (token->isNull()) {
				continue;
			}
			if (token->isSecondaryTiedNote()) {
				continue;
			}
			string rest  = token->isRest()  ? "R" : "";
			string grace = token->isGrace() ? "G" : "";
			string tie   = (token->find("[") != string::npos) || token->isSecondaryTiedNote() ? "T" : "";
			HumNum duration;
			duration = token->getTiedDuration();
			cout << duration.getFloat() << rest << grace << tie << endl;
		}
	}
}



