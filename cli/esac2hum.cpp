//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Thu Aug 22 18:35:34 PDT 2024
// Last Modified: Thu Aug 22 18:35:37 PDT 2024
// Filename:      cli/esac2hum.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/cli/esac2hum.cpp
// Syntax:        C++11
// vim:           ts=3 noexpandtab nowrap
//
// Description:   Converter from EsAC to Humdrum.
//

#include "humlib.h"

using namespace std;
using namespace hum;

int main(int argc, char** argv) {
	Tool_esac2hum interface;
	if (!interface.process(argc, argv)) {
		interface.getError(cerr);
		return -1;
	}
	HumdrumFile infile;
	int status = 1;
	if (interface.getArgCount() > 0) {
		status = interface.convertFile(cout, interface.getArgument(1));
	} else {
		status = interface.convert(cout, cin);
	}
	if (interface.hasWarning()) {
		interface.getWarning(cerr);
		return 0;
	}
	if (interface.hasError()) {
		interface.getError(cerr);
		return -1;
	}
	return !status;
}



