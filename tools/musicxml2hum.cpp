//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Aug  6 10:53:40 CEST 2016
// Last Modified: Sun Sep 18 13:11:26 PDT 2016
// Filename:      musicxml2hum.cpp
// URL:           https://github.com/craigsapp/musicxml2hum/blob/master/src/musicxml2hum.cpp
// Syntax:        C++11
// vim:           ts=3 noexpandtab
//
// Description:   Command-line interface for converting MusicXML files into
//                Humdrum files.
//

#include "tool-musicxml2hum.h"

#include <iostream>

using namespace std;

int main(int argc, char** argv) {
	hum::Tool_musicxml2hum converter;
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

