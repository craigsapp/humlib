//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Oct  7 20:59:36 PDT 2024
// Last Modified: Mon Oct  7 20:59:39 PDT 2024
// Filename:      cli/tempo2time.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/cli/tempo2time.cpp
// Syntax:        C++11
// vim:           ts=3 noexpandtab nowrap
//

#include "humlib.h"
#include <iostream>

using namespace hum;
using namespace std;

void processFile   (HumdrumFile& infile);
void getTimings    (vector<double>& timings, HumdrumFile& infile);
void printInputWithTimings(HumdrumFile& infile, vector<double>& timings);

bool secondsQ = false;  // used with -s option
bool roundQ   = false;  // used with -r option

///////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv) {
	Options options;
	options.define("s|sec|seconds=b", "Output time in seconds");
	options.define("r|round=b", "Round output time when milliseconds");
	options.process(argc, argv);
	secondsQ = options.getBoolean("seconds");
	roundQ   = options.getBoolean("round");
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

void processFile(HumdrumFile& infile) {
	vector<double> timings;
	getTimings(timings, infile);
	printInputWithTimings(infile, timings);
}



//////////////////////////////
//
// printInputWithTimings --
//

void printInputWithTimings(HumdrumFile& infile, vector<double>& timings) {
	for (int i=0; i<infile.getLineCount(); i++) {
		if (infile[i].isEmpty()) {
			cout << endl;
			continue;
		}
		if (!infile[i].hasSpines()) {
			cout << infile[i] << endl;
			continue;
		}
		if (infile[i].isManipulator()) {
			HTp token = infile.token(i, 0);
			if (*token == "*-") {
				cout << token << "\t" << infile[i] << endl;
				continue;
			} else if (token->compare(0, 2, "**") == 0) {
				if (secondsQ) {
					cout << "**stime";
				} else {
					cout << "**mtime";
				}
				cout << "\t";
				cout << infile[i] << endl;
				continue;
			} else {
				cout << "*\t" << infile[i] << endl;
			}
		}
		if (infile[i].isInterpretation()) {
			cout << "*\t" << infile[i] << endl;
			continue;
		}
		if (infile[i].isComment()) {
			cout << "!\t" << infile[i] << endl;
			continue;
		}
		if (infile[i].isData()) {
			if (secondsQ) {
				cout << timings[i] << "\t" << infile[i] << endl;
			} else {
				double value = timings[i] * 1000.0;
				if (roundQ) {
					value = int(value + 0.5);
				}
				cout << value << "\t" << infile[i] << endl;
			}
			continue;
		}
		if (infile[i].isBarline()) {
			HTp token = infile.token(i, 0);
			cout << token << "\t" << infile[i] << endl;
			continue;
		}
		cerr << "!!ERROR: Should not get here, line is: " << infile[i] << endl;
	}
}



//////////////////////////////
//
// getTimings --
//

void getTimings(vector<double>& timings, HumdrumFile& infile) {

	vector<double> tempos(infile.getLineCount(), 0.0);
	tempos[0] = 120.0;
	double tempo = 120.0;

	HumRegex hre;
	for (int i=0; i<infile.getLineCount(); i++) {
		if (infile[i].isInterpretation()) {
			for (int j=infile[i].getFieldCount()-1; j>=0; j--) {
				HTp token = infile.token(i, j);
				if (!token->isKern()) {
					continue;
				}
				if (!token->isTempo()) {
					continue;
				}
				if (hre.search(token, "(\\d+\\.?\\d*)")) {
					tempo = hre.getMatchDouble(1);
					tempos[i] = tempo;
				}
			}
			continue;
		}
	}

	HumNum sum = 0;
	HumNum last = 0;
	vector<HumNum> sumFromLastTempo(infile.getLineCount(), 0);

	for (int i=0; i<infile.getLineCount(); i++) {
		sum += last;
		sumFromLastTempo[i] = sum;
		last = infile[i].getDuration();
		if (tempos[i] > 0.0) {
			sum = 0;
		}
	}

	timings.resize(infile.getLineCount());
	std::fill(timings.begin(), timings.end(), 0.0);

	double lastTime = 0.0;

	for (int i=0; i<infile.getLineCount(); i++) {
		timings[i] = lastTime + 60.0 / tempo * sumFromLastTempo[i].getFloat();
		if (tempos[i] > 0.0){
			lastTime = timings[i];
		}
	}
}



