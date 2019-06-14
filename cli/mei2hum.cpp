//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Wed Sep 13 14:53:40 PDT 2017
// Last Modified: Wed Sep 13 14:53:42 PDT 2017
// Filename:      mei2hum.cpp
// URL:           https://github.com/craigsapp/musicxml2hum/blob/master/src/mei2hum.cpp
// Syntax:        C++11
// vim:           ts=3 noexpandtab
//
// Description:   Command-line interface for converting MEI files into
//                Humdrum files.
//

#include "humlib.h"

#include <iostream>

using namespace std;

int main(int argc, char** argv) {
	hum::Tool_mei2hum converter;
	if (!converter.process(argc, argv)) {
		converter.getError(cerr);
		return -1;
	}
	// hum::Options options(converter.getOptionDefinitions());
	// options.process(argc, argv);

	pugi::xml_document infile;
	string filename;
	if (converter.getArgCount() == 0) {
		filename = "<STDIN>";
		infile.load(cin);
	} else {
		filename = converter.getArg(1);
		infile.load_file(filename.c_str());
	}

	//converter.setOptions(argc, argv);
	stringstream out;
	bool status = converter.convert(out, infile);
	if (!status) {
		cerr << "Error converting file: " << filename << endl;
	}
	cout << out.str();

	return 0;
}

