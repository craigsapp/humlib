// Description: Test searching in HumRegex.
// vim: ts=3
// $Smake: g++ -O3 -o %b %f HumRegex.cpp
//

#include "HumRegex.h"

#include <iostream>

using namespace std;
using namespace hum;

int main(int argc, char** argv) {
	if (argc < 3) {
		cerr << "Usage: " << argv[0] << " exp string(s)" << endl;
	}
	HumRegex hre;
	for (int i=2; i<argc; i++) {
		bool status = hre.search(argv[i], argv[1]);
		cout << "String: " << argv[i] << endl;
		if (status) {
			cout << "\t";
			cout << hre.getMatchCount();
			if (hre.getMatchCount() == 1) {
				cout << " MATCH";
			} else {
				cout << " MATCHES";
			}
			cout << endl;
			cout << "\tprefix:\t" << hre.getPrefix() << endl;
			cout << "\tsuffix:\t" << hre.getSuffix() << endl;
			for (int j=0; j<hre.getMatchCount(); j++) {
			cout << "\tmatch " << j << ":\t\t" << hre.getMatch(j) << endl;
			cout << "\t\tstartpos:\t" << hre.getMatchStartIndex(j) << endl;
			cout << "\t\tendpos:\t\t" << hre.getMatchEndIndex(j) << endl;
			cout << "\t\tlength:\t\t" << hre.getMatchLength(j) << endl;
			}
		} else {
			cout << "\tDOES NOT MATCH" << endl;
		}
	}
	return 0;
}

