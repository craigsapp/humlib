//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Wed Sep 25 19:35:54 PDT 2019
// Last Modified: Mon Feb 21 18:31:59 PST 2022
// Filename:      cli/musedata2hum.cpp
// URL:           https://github.com/craigsapp/musedata2hum/blob/master/src/musedata2hum.cpp
// Syntax:        C++11
// vim:           ts=3 noexpandtab
//
// Description:   Command-line interface for converting MusicXML files into
//                Humdrum files.
//

#include "humlib.h"

#include <iostream>

using namespace std;
using namespace hum;

int main(int argc, char** argv) {
	hum::Tool_musedata2hum converter;
	if (!converter.process(argc, argv)) {
		converter.getError(cerr);
		return -1;
	}

	MuseDataSet infile;
	string filename;
	if (converter.getArgCount() == 0) {
		filename = "<STDIN>";
		infile.read(cin);
	} else {
		filename = converter.getArg(1);
		infile.readFile(filename);
	}
	int partcount = infile.getFileCount();
	if (partcount == 0) {
		cerr << "Error: no parts found in file" << endl;
		return 1;
	}

	stringstream out;
	bool status = converter.convert(out, infile);
	if (!status) {
		cerr << "Error converting file: " << filename << endl;
	}
	cout << out.str();

	return 0;
}

