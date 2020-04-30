//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Tue 28 Apr 2020 08:54:49 AM PDT
// Last Modified: Tue 28 Apr 2020 08:54:52 AM PDT
// Filename:      naka.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/cli/naka.cpp
// Syntax:        C++11
// vim:           ts=3 noexpandtab nowrap
//
// Description:   Extra metric positions of notes.
//

#include "humlib.h"
#include <iostream>

using namespace hum;
using namespace std;

void   processFile         (HumdrumFile& infile, Options& options);


///////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv) {
	Options options;
	options.define("A|all=b", "extract all features");
	options.process(argc, argv);
	HumdrumFileStream instream(options);
	HumdrumFile infile;
	while (instream.read(infile)) {
		processFile(infile, options);
	}
	return 0;
}



//////////////////////////////
//
// processFile --
//

void processFile(HumdrumFile& infile, Options& options) {
	int allcount = 0;
	int measure = 0;
	vector<int> partcount(1000, 0);
	vector<HTp> spinestarts;
	infile.getKernSpineStartList(spinestarts);
	map<int, int> trackToPart;
	for (int i=0; i<(int)spinestarts.size(); i++) {
		int part = spinestarts.size() - i;
		int track = spinestarts[i]->getTrack();
		trackToPart[track] = part;
	}

	int tpq = infile.tpq();
	cout << "# Divisions per quarter = " << tpq << endl;
	cout << "#tick\tpitch\tabsbeat\tbeat\tid\n";

	for (int i=0; i<infile.getLineCount(); i++) {
		if (infile[i].isBarline()) {
			fill(partcount.begin(), partcount.end(), 0);
			HumRegex hre;
			if (hre.search(infile.token(i, 0), "=(\\d+)")) {
				measure = hre.getMatchInt(1);
			}
		}
		if (!infile[i].isData()) {
			continue;
		}

		for (int j = infile[i].getFieldCount() - 1; j >= 0; j--) {
			HTp token = infile.token(i, j);
			if (!token->isKern()) {
				continue;
			}
			if (token->isNull()) {
				continue;
			}
			int track = token->getTrack();
			int part = trackToPart[track];
			if (token->isRest()) {
				// update note counter for part then skip
				partcount[part]++;
				continue;
			}
			vector<std::string> subtokens = token->getSubtokens(); 
			for (int k=0; k<(int)subtokens.size(); k++) {
				if (subtokens[k].find('_') != string::npos) {
					// ignore tie continuations
					continue;
				}
				if (subtokens[k].find(']') != string::npos) {
					// ignore tie endings
					continue;
				}

				int b40 = Convert::kernToBase40(subtokens[k]);
				int octave = b40 / 40;
				int accid = Convert::base40ToAccidental(b40);
				int diatonic = Convert::base40ToDiatonic(b40);
				diatonic %= 7;


				HumNum absbeat = token->getDurationFromStart();
				HumNum barpos = token->getDurationFromBarline();
				HumNum ticks = absbeat * tpq;
				if (!ticks.isInteger()) {
					cerr << "Strange problem with ticks: " << ticks << endl;
				}

				allcount++;
				cout << ticks << "\t";
				char pitch = 'C';
				if (diatonic < 5) {
					pitch += diatonic;
				} else {
					pitch += diatonic - 7;
				}
				cout << pitch;
				if (accid > 0) {
					for (int m=0; m<accid; m++) {
						cout << "#";
					}
				} else if (accid < 0) {
					for (int m=0; m<-accid; m++) {
						cout << "b";
					}
				}
				cout << octave;

				cout << "\t";
				cout << absbeat.getFloat();

				cout << "\t";
				cout << barpos.getFloat() + 1.0;

				cout << "\t";
				cout << "P" << part << "-";
				cout << measure << "-";
				int ncount = ++partcount[part];
				cout << ncount;
				cout << endl;
			}
		}
	}
}



