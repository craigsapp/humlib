//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Fri Jan 29 18:58:32 PST 2021
// Last Modified: Fri Jan 29 18:58:35 PST 2021
// Filename:      humxml.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/cli/humxml.cpp
// Syntax:        C++11
// vim:           ts=3 noexpandtab nowrap
//
// Description:   Reveal data structure of Humdrum file and internal parameters.
//

#include "humlib.h"

using namespace std;
using namespace hum;

int main(int argc, char **argv) {
	Options options;
	options.define("s|slurs=b", "analyze slurs");
	options.process(argc, argv);

	HumdrumFile infile;
	if (options.getArgCount() == 0) {
		infile.read(cin);
	} else {
		infile.read(options.getArg(1));
	}
	if (options.getBoolean("slurs")) {
		infile.analyzeSlurs();
	}
	infile.printXml();
}



