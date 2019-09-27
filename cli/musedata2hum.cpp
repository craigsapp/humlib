//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Wed Sep 25 19:35:54 PDT 2019
// Last Modified: Wed Sep 25 19:35:57 PDT 2019
// Filename:      musedata2hum.cpp
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

	stringstream out;
	bool status = converter.convert(out, infile);
	if (!status) {
		cerr << "Error converting file: " << filename << endl;
	}
	cout << out.str();

	return 0;
}

