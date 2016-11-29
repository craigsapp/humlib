//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Nov 28 21:19:11 PST 2016
// Last Modified: Mon Nov 28 21:40:41 PST 2016
// Filename:      HumdrumFileContent-metlev.h
// URL:           https://github.com/craigsapp/humlib/blob/master/src/HumdrumFileContent-metlev.cpp
// Syntax:        C++11
// vim:           ts=3 noexpandtab
//
// Description:   
//   Extracts metric level of rhythmic values in Humdrum files.
//   Beat levels are log2 based, with 0 being the beat.  In 4/4 (and other
//   simple meters), the beat level is the quarter note.  The 8th-note level
//   is 1, the 16th-note level is 2, the 32nd-note level is 3, and so on.
//   Compound meters such as 6/8, use log3 for the first level, and then
//   log2 for smallert rhythmic value levels.  Metric positions above the 
//   beat level have yet to be implemented.
//

#include "HumdrumFileContent.h"
#include "Convert.h"

#include <algorithm>
#include <string.h>

using namespace std;

namespace hum {

// START_MERGE

//////////////////////////////
//
// HumdrumFileStructure::HumdrumFileContent -- HumdrumFileStructure
//     constructor.  Each line in the output vector matches to the
//     line of the metric analysis data.  undefined is the value to
//     represent undefined analysis data (for non-data spines).
//
//     default value: track = 0: 0 means use the time signature
//         of the first **kern spines in the file; otherwise, use the
//         time signatures found in the given track (indexed from 1
//         for the first spine on a line).
//     default value: undefined = NAN
//

void HumdrumFileContent::getMetricLevels(vector<double>& output,
		int track, double undefined) {

	HumdrumFileStructure& infile = *this;
	int lineCount = infile.getLineCount();
	output.resize(lineCount);
	fill(output.begin(), output.end(), undefined);
	vector<HTp> kernspines = infile.getKernSpineStartList();
	if ((track == 0) && (kernspines.size() > 0)) {
		track = kernspines[0]->getTrack();
	}
	if (track == 0) {
		track = 1;
	}

	int top = 1;                // top number of time signature (0 for no meter)
	int bot = 4;                // bottom number of time signature
   bool compoundQ = false;     // test for compound meters, such as 6/8
   HumNum beatdur(1 * 4, bot); // duration of a beat in the measure
	HumNum measurepos;          // quarter notes from the start of barline
   HumNum combeatdur;          // for adjusting beat level in compound meters
   HumNum commeasurepos;       // for adjusting beat level in compound meters

	for (int i=0; i<lineCount; i++) {
		if (infile[i].isInterpretation()) {
			// check for time signature:
			HumdrumLine& line = *infile.getLine(i);
			for (int j=0; j<line.getFieldCount(); j++) {
				int ttrack = line.token(j)->getTrack();
				if (ttrack != track) {
					continue;
				}
				if (sscanf(infile.token(i,j)->c_str(), "*M%d/%d", &top, &bot)) {
					beatdur.setValue(1*4, bot); // converted to quarter-note units
					if ((top % 3 == 0) && (top != 3)) {
						// if meter top is a multiple of 3 but not 3, then compound
						// such as 6/8, 9/8, 6/4, but not 3/8, 3/4.
						compoundQ = true;
						beatdur *= 3;
					} else {
						compoundQ = false;
					}
					break;
				}
			}
		}
		if (!infile[i].isData()) {
				continue;
		}

		measurepos = infile[i].getDurationFromBarline();
		// Might want to handle cases where the time signature changes in 
		// the middle or a measure...
		measurepos /= beatdur;
		int denominator = measurepos.getDenominator();
		if (compoundQ) {
			output[i] = Convert::nearIntQuantize(log(denominator) / log(3.0));
			if ((output[i] != 0.0) && (output[i] != 1.0)) {
				// if not the beat or first level, then calculate
				// levels above level 1.  In 6/8 this means
				// to move the 8th note level to be the "beat"
				// and then use binary levels for rhythmic levels
				// smaller than a beat.
				combeatdur.setValue(4,bot);
				commeasurepos = infile[i].getDurationFromBarline() / combeatdur;
				denominator = commeasurepos.getDenominator();
				output[i] = 1.0 + log(denominator)/log(2.0);
			}
		} else {
			output[i] = Convert::nearIntQuantize(log(denominator) / log(2.0));
		}
	}
}



// END_MERGE

} // end namespace hum



