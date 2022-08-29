//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Aug 29 08:39:30 CEST 2022
// Last Modified: Mon Aug 29 08:39:41 CEST 2022
// Filename:      spineclose.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/cli/spineclose.cpp
// Syntax:        C++11
// vim:           ts=3 noexpandtab nowrap
//
// Description:   Add spacing tabs between primary spines to align them.
//

#include "humlib.h"

using namespace hum;
using namespace std;

void   processFile  (HumdrumFile& infile, Options& options);
string getMergeLine (vector<string>& info, int index);


///////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv) {
	Options options;
	options.define("i|info=b", "Show list of input scores that need closing");
	options.define("v|verbose=b", "Add additional text to -i option.");
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
	int endi = -1;
	bool foundQ = false;
	bool problemQ = false;
	bool infoQ = options.getBoolean("info");
	bool verboseQ = options.getBoolean("verbose");
	for (int i=infile.getLineCount() - 1; i>=0; i--) {
		if (infile[i].isInterpretation()) {
			endi = i;
			break;
		}
	}

	if (endi < 0) {
		if (!infoQ) {
			cout << infile;
		}
		return;
	}

	// Store the spine info strings for the termination line:
	int tokcount = infile[endi].getFieldCount();
	int newtokcount = tokcount;
	vector<string> info(tokcount);
	for (int j=0; j<infile[endi].getFieldCount(); j++) {
		HTp token = infile.token(endi, j);
		info.at(j) = token->getSpineInfo();
		if (info.at(j).find(' ') != string::npos) {
			problemQ = true;
		} else if (info.at(j) == "") {
			problemQ = true;
		}
	}

	// Insert mergers for adjacent pairs of spines that should
	// merge (not checking for complicated cases such as three
	// spines that should merge into one (pass the data through
	// this program again to merge more complicated cases).
	HumRegex hrea;
	HumRegex hreb;
	for (int i=0; i<info.size() - 1; i++) {
		string a = info.at(i);
		string b = info.at(i+1);
		if (!hrea.search(a, "\\(.*\\)a")) {
			continue;
		}
		if (!hreb.search(b, "\\(.*\\)b")) {
			continue;
		}
		string aval = hrea.getMatch(1);
		string bval = hreb.getMatch(1);
		if (aval != bval) {
			continue;
		}
		// Add a merging interpretation line just above
		// the termination line
		string output = getMergeLine(info, i);
		infile.insertLine(endi, output);
		endi++;
		newtokcount--;
		foundQ = true;
	}

	// Adjust the number of *- tokens on the termination spine:
	if (newtokcount != tokcount) {
		string newend;
		for (int i=0; i<newtokcount; i++) {
			newend += "*-";
			if (i < newtokcount - 1) {
				newend += "\t";
			}
		}
		infile[endi].setText(newend);
	}

	if (infoQ && problemQ) {
		if (verboseQ) {
			cout << "SPINE ERROR IN:\t";
		}
		cout << infile.getFilename() << endl;
	}
	if (infoQ && foundQ) {
		if (verboseQ) {
			cout << "SPINES NEED CLOSING IN:\t";
		}
		cout << infile.getFilename() << endl;
	} 
	if (!infoQ) {
		cout << infile;
	}

}



//////////////////////////////
//
// getMergeLine --
//

string getMergeLine(vector<string>& info, int index) {
	if (info.empty()) {
		return "";
	}
	string output;
	for (int i=0; i<index; i++) {
		output += "*\t";
	}
	output += "*v\t*v";
	for (int i=index+2; i<info.size(); i++) {
		output += "\t*";
	}

	// remove empty spot in info line:
	info.at(index) = "x";
	for (int i=index+2; i<info.size(); i++) {
		info.at(i-1) = info.at(i);
	}
	info.resize(info.size() - 1);

	return output;
}



