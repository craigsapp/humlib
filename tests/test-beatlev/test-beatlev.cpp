// Description: Example of extracting beat levels
// vim: ts=3
// Todo: meters such as 2+3/4
//       tuplets (may be hard)
//       pickups (particularly if they do not start on a beat
//       secondary partial measures

#include "humlib.h"
#include <cmath>

using namespace std;
using namespace hum;

// function declarations:

int main(int argc, char** argv) {

	// handle command-line options:
	Options opts;
	opts.define("a|append=b",          "append data analysis to input file");
	opts.define("p|prepend=b",         "prepend data analysis to input file");
	opts.define("i|integer=b",         "quantize metric levels to int values");
	opts.define("m|metric-position=b", "output metric position in measure");
	opts.define("b|beat-duration=b",   "output beat durations");
	opts.process(argc, argv);

	bool quantize = opts.getBoolean("integer");

	// read an inputfile from the first filename argument, or standard input
	HumdrumFile infile;
	if (opts.getArgCount() > 0) {
		infile.read(opts.getArgument(1));
	} else {
		infile.read(cin);
	}

	int lineCount = infile.getLineCount();
	vector<double> beatlev(lineCount, NAN);  // beat level analysis
	vector<HumNum> measurepos(lineCount);    // quarter notes since barline
	vector<HumNum> beatdurs(lineCount);      // duration of a beat


	// set the default meter if not given (maybe infer from duration of meausre)
	int top = 4;              // top number of meter signature
	int bot = 4;              // bottom number of meter signature
	HumNum beatdur(top, bot); // duration of a beat in the measure
	bool compoundQ = false;   // test for compound meters, such as 6/8
	HumNum adjmeasurepos;     // for adjusting the beat level in compout meters
	HumNum adjbeatdur;        // for adjusting the beat level in compout meters

	for (int i=0; i<lineCount; i++) {
		if (sscanf(infile.token(i,0)->c_str(), "*M%d/%d", &top, &bot)) {
			beatdur.setValue(1 * 4, bot); // converted to quarter-note units
			if ((top % 3 == 0) && (top != 3)) {
            // if the meter top is a multiple of 3 but not 3, then compound
				compoundQ = true;
				beatdur *= 3;
			} else {
				compoundQ = false;
			}
		}
		if (!infile[i].isData()) {
				continue;
		}
		beatdurs[i] = beatdur;
		measurepos[i] = infile[i].getDurationFromBarline();
		measurepos[i] /= beatdur;
		int denominator = measurepos[i].getDenominator();
		if (compoundQ) {
			beatlev[i] = log(denominator) / log(3.0);
			// avoid small deviations from integer values:
			double q = beatlev[i];
			if ((q + 0.00001) - int(q+0.00001)  < 0.0001) {
				q = (int)(q+0.00001);
			}
			if ((q != 0.0) && (q != 1.0)) {
				// if not the beat or first level, then calculate
				// levels above level 1.  In 6/8 this means
				// to move the 8th note level to be the "beat"
				// and then use binary levels for rhythmic levels
				// smaller than a beat.
				adjbeatdur.setValue(4,bot);
				adjmeasurepos = infile[i].getDurationFromBarline() / adjbeatdur;
				denominator = adjmeasurepos.getDenominator();
				beatlev[i] = 1.0 + log(denominator)/log(2.0);
			}
		} else {
			beatlev[i] = log(denominator) / log(2.0);
		}
		if (quantize) {
				beatlev[i] = floor(beatlev[i]);
		}
	}

	if (opts.getBoolean("append")) {
		infile.appendDataSpine(beatlev, "nan", "**blev");
		if (opts.getBoolean("metric-position")) {
			infile.appendDataSpine(measurepos, "nan", "**mpos");
		}
		if (opts.getBoolean("beat-duration")) {
			infile.appendDataSpine(beatdurs, "nan", "**bdur");
		}
		cout << infile;
	} else if (opts.getBoolean("prepend")) {
		if (opts.getBoolean("beat-duration")) {
			infile.prependDataSpine(beatdurs, "nan", "**bdur");
		}
		if (opts.getBoolean("metric-position")) {
			infile.prependDataSpine(measurepos, "nan", "**mpos");
		}
		infile.prependDataSpine(beatlev, "nan", "**blev");
		cout << infile;
	} else {
		infile.prependDataSpine(beatlev, "nan", "**blev");
		infile.printFieldIndex(0, cout);
	}

	return 0;
}


