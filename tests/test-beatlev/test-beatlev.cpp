// Description: Example of extracting beat levels
// a score for dissonance analysis.
// vim: ts=3
// Todo: meters such as 2+3/4
//       tuplets (may be hard)
//       pickups (particularly if they do not start on a beat
//       secondary partial measures

#include "humlib.h"
#include <cmath>
#include <algorithm>

using namespace std;
using namespace hum;

// function declarations:
void fillResults(vector<vector<double> >& results, HumdrumFile& infile,
		vector<double>& beatlev);

bool quantizeQ = false;  // used with -i option
bool attacksQ  = false;  // used with -x option
bool nograceQ  = false;  // used with -G option


int main(int argc, char** argv) {

	// handle command-line options:
	Options opts;
	opts.define("a|append=b",          "append data analysis to input file");
	opts.define("p|prepend=b",         "prepend data analysis to input file");
	opts.define("i|integer=b",         "quantize metric levels to int values");
	opts.define("m|metric-position=b", "output metric position in measure");
	opts.define("b|beat-duration=b",   "output beat durations");
	opts.define("x|attacks-only=b",    "only mark lines with note attacks");
	opts.define("G|no-grace-notes=b",  "do not mark grace note lines");
	opts.define("k|kern-spine=i:1",    "analyze only given kern spine");
	opts.define("K|all-spines=b",      "analyze each kern spine separately");
	opts.process(argc, argv);

	quantizeQ = opts.getBoolean("integer");
	attacksQ  = opts.getBoolean("attacks-only");
	nograceQ  = opts.getBoolean("no-grace-notes");

	// read an inputfile from the first filename argument, or standard input
	HumdrumFile infile;
	if (opts.getArgCount() > 0) {
		infile.read(opts.getArgument(1));
	} else {
		infile.read(cin);
	}

	int lineCount = infile.getLineCount();
	vector<double> beatlev(lineCount, NAN);      // beat level analysis
	vector<HumNum> measurepos(lineCount, -1000); // quarter notes since barline
	vector<HumNum> beatdurs(lineCount, -1000);   // duration of a beat

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
		if (nograceQ && (infile[i].getDuration() == 0)) {
			continue;
		}
		if (attacksQ) {
			if (!infile[i].getKernNoteAttacks()) {
				continue;
			}
		}
		beatdurs[i] = beatdur;
		measurepos[i] = infile[i].getDurationFromBarline();
		measurepos[i] /= beatdur;
		int denominator = measurepos[i].getDenominator();
		if (compoundQ) {
			beatlev[i] = Convert::nearIntQuantize(log(denominator) / log(3.0));
			if ((beatlev[i] != 0.0) && (beatlev[i] != 1.0)) {
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
			beatlev[i] = Convert::nearIntQuantize(log(denominator) / log(2.0));
		}
		if (beatlev[i] - (int)beatlev[i] != 0.0) {
			beatlev[i] = Convert::significantDigits(beatlev[i], 2);
		}
		if (quantizeQ) {
				beatlev[i] = floor(beatlev[i]);
		}
	}

	// print the analysis results:
	if (opts.getBoolean("kern-spine")) {
		int kspine = opts.getInteger("kern-spine") - 1;
		vector<HTp> kernspines = infile.getKernSpineStartList();
		if ((kspine >= 0) && (kspine < kernspines.size())) {
			vector<vector<double> > results;
			fillResults(results, infile, beatlev);
			if (kspine == (int)kernspines.size() - 1) {
				infile.appendDataSpine(results.back(), "nan", "**blev");
			} else {
				int track = kernspines[kspine+1]->getTrack();
				infile.insertDataSpineBefore(track, results[kspine],
						"nan", "**blev");
			}
			cout << infile;
		}
	} else if (opts.getBoolean("all-spines")) {
		vector<HTp> kernspines = infile.getKernSpineStartList();
		vector<vector<double> > results;
		fillResults(results, infile, beatlev);
		infile.appendDataSpine(results.back(), "nan", "**blev");
		for (int i = (int)results.size()-1; i>0; i--) {
			int track = kernspines[i]->getTrack();
			infile.insertDataSpineBefore(track, results[i-1], "nan", "**blev");
		}
		cout << infile;
	} else if (opts.getBoolean("append")) {
		infile.appendDataSpine(beatlev, "nan", "**blev");
		if (opts.getBoolean("metric-position")) {
			infile.appendDataSpine(measurepos, "-1000", "**mpos");
		}
		if (opts.getBoolean("beat-duration")) {
			infile.appendDataSpine(beatdurs, "-1000", "**bdur");
		}
		cout << infile;
	} else if (opts.getBoolean("prepend")) {
		if (opts.getBoolean("beat-duration")) {
			infile.prependDataSpine(beatdurs, "-1000", "**bdur");
		}
		if (opts.getBoolean("metric-position")) {
			infile.prependDataSpine(measurepos, "-1000", "**mpos");
		}
		infile.prependDataSpine(beatlev, "nan", "**blev");
		cout << infile;
	} else {
		infile.prependDataSpine(beatlev, "nan", "**blev");
		infile.printFieldIndex(0, cout);
	}

	return 0;
}



//////////////////////////////
//
// fillResults --
//

void fillResults(vector<vector<double> >& results, HumdrumFile& infile,
		vector<double>& beatlev) {
	vector<HTp> kernspines = infile.getKernSpineStartList();
	results.resize(kernspines.size());
	for (int i=0; i<(int)results.size(); i++) {
		results[i].resize(beatlev.size());
		fill(results[i].begin(), results[i].end(), NAN);
	}
	int track;
	vector<int> rtracks(infile.getTrackCount() + 1, -1);
	for (int i=0; i<(int)kernspines.size(); i++) {
		int track = kernspines[i]->getTrack();
		rtracks[track] = i;
	}

	vector<int> nonnullcount(kernspines.size(), 0);
	vector<int> attackcount(kernspines.size(), 0);
	HTp token;
	int voice;
	int i, j;
	for (i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isData()) {
			continue;
		}
		for (j=0; j<infile[i].getFieldCount(); j++) {
			token = infile.token(i, j);
			if (!token->isKern()) {
				continue;
			}
			if (token->isNull()) {
				continue;
			}
			track = token->getTrack();
			voice = rtracks[track];
			nonnullcount[voice]++;
			if (token->isNoteAttack()) {
				attackcount[voice]++;
			}
		}
		for (int v=0; v<(int)kernspines.size(); v++) {
			if (attacksQ) {
				if (attackcount[v]) {
					results[v][i] = beatlev[i];
					attackcount[v] = 0;
				}
			} else {
				if (nonnullcount[v]) {
					results[v][i] = beatlev[i];
				}
				nonnullcount[v] = 0;
			}
		}
	}
}



