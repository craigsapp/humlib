//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Oct 23 20:39:21 PDT 2021
// Last Modified: Sat Oct 23 20:39:24 PDT 2021
// Filename:      scapeinfo.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/cli/scapeinfo.cpp
// Syntax:        C++11
// vim:           ts=3 noexpandtab nowrap
//
// Description:   Calculate score location info for pixel columns in keyscape images.
//

#include "humlib.h"
#include <iostream>

using namespace hum;
using namespace std;

void  processFile   (HumdrumFile& infile, Options& options);
void  getBarnums    (vector<int>& barnums, HumdrumFile& infile);


///////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv) {
	Options options;
	options.define("s|segments=i:300", "number of equally spaced segements in analysis");
	options.process(argc, argv);
	HumdrumFileStream instream(options);
	HumdrumFile infile;
	while (instream.read(infile)) {
		processFile(infile, options);
		// Only allow one file at a time for now:
		break;
	}
	return 0;
}



//////////////////////////////
//
// processFile --
//

void processFile(HumdrumFile& infile, Options& options) {
	int debugQ = 0;

	int segments = options.getInteger("segments");
	if (segments <= 0) {
		segments = 300;
	}

	vector<int> barnums;
	getBarnums(barnums, infile);

	if (debugQ) {
		for (int i=0; i<infile.getLineCount(); i++) {	
			cout << barnums[i] << "\t" << infile[i] << endl;
		}
	}

	vector<int> datalines;
	datalines.reserve(infile.getLineCount());
	for (int i=0; i<infile.getLineCount(); i++) {
		if (infile[i].isData()) {
			HumNum linedur = infile[i].getDuration();
			if (linedur > 0) {
				datalines.push_back(i);
			}
		}
	}

	vector<double> qtimes(datalines.size(), -1);
	for (int i=0; i<(int)datalines.size(); i++) {
		qtimes[i] = infile[datalines[i]].getDurationFromStart().getFloat();
	}

	double totaldur = infile.getScoreDuration().getFloat();
	double increment = totaldur / segments;

	cout << "[\n";
	for (int i=0; i<segments; i++) {
		double qstart = increment * i;
		double qend   = increment * (i + 1.0);
		int starttargetindex = -1;
		for (int j=0; j<(int)qtimes.size(); j++) {
			if (qtimes[j] >= qstart) {
				starttargetindex = j;
				break;
			}
		}
		int endtargetindex = -1;
		for (int j=(int)qtimes.size()-1; j>=0; j--) {
			if (qtimes[j] <= qstart) {
				endtargetindex = j;
				break;
			}
		}

		cout << "\t{";
		cout << "\"qstart\":" <<  qstart;
		cout << ", \"qend\":" <<  qend;
		if (endtargetindex >= 0) {
			cout << ", \"startbar\":" <<  barnums.at(datalines.at(endtargetindex));
		}
		if (starttargetindex >= 0) {
			cout << ", \"endbar\":" <<  barnums.at(datalines.at(starttargetindex));
		}

		cout << "}";
		if (i < segments) {
			cout << ",";
		}
		cout << "\n";
		
	}
	cout << "]\n";
	
}



//////////////////////////////
//
// getBarnums --
//

void getBarnums(vector<int>& barnums, HumdrumFile& infile) {
	barnums.resize(infile.getLineCount());
	fill(barnums.begin(), barnums.end(), -1);
	int current = -1;
	int firstbar = -1;
	for (int i=0; i<infile.getLineCount(); i++) {
		if (infile[i].isBarline()) {
			int value = infile[i].getBarNumber();
			if (value >= 0) {
				current = value;
				if (firstbar == -1) {
					firstbar = i;
				}
			}
		}
		barnums[i] = current;
	}

	// go back and fill in the barnumber before first numbered bar:
	for (int i=0; i<firstbar; i++) {
		barnums[i] = barnums[firstbar] - 1;
	}
}



