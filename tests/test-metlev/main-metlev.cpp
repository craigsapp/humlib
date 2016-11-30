//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Aug  8 12:24:49 PDT 2015
// Last Modified: Tue Nov 29 01:03:06 PST 2016
// Filename:      tool-metlev.h
// URL:           tbd
// Syntax:        C++11
// vim:           ts=3 noexpandtab
//
// Description:   Command-line interface to metlev tool.
//                Extracts metric levels from score.
//

#include "humlib.h"

// delete these lines when the files are placed properly
#include "HumTool.cpp"
#include "tool-metlev.cpp"

using namespace std;
using namespace hum;

int main(int argc, char** argv) {
	Tool_metlev interface;
	interface.process(argc, argv);

	// read an inputfile from the first filename argument, or standard input
	HumdrumFile infile;
	if (interface.getArgCount() > 0) {
		infile.read(interface.getArgument(1));
	} else {
		infile.read(cin);
	}

	int status = interface.run(infile, cout);
	if (status == 0) {
		cerr << interface.getError();
	} else {
		if (interface.hasError()) {
			cerr << interface.getError();
		}
	}

	return 0;
}

