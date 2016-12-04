//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Nov 28 08:55:15 PST 2016
// Last Modified: Mon Nov 28 08:55:38 PST 2016
// Filename:      HumTool.h
// URL:           https://github.com/craigsapp/minHumdrum/blob/master/include/HumTool.h
// Syntax:        C++11
// vim:           ts=3 noexpandtab
//
// Description:   Common interface for Humdrum tools.
//

#ifndef _HUMTOOL_H_INCLUDED
#define _HUMTOOL_H_INCLUDED

#include "Options.h"
#include <sstream>

namespace hum {

// START_MERGE

class HumTool : public Options {
	public:
		       HumTool        (void);
		      ~HumTool        ();

		bool   hasError       (void);
		string getError       (void);

	protected:
		stringstream m_error;

};


///////////////////////////////////////////////////////////////////////////
//
// common command-line Interfaces
//

//
// BASIC_INTERFACE -- .run(HumdrumFile& infile, ostream& out)
//

#define BASIC_INTERFACE(CLASS)                 \
                                               \
using namespace std;                           \
using namespace hum;                           \
                                               \
int main(int argc, char** argv) {              \
	CLASS interface;                            \
	interface.process(argc, argv);              \
                                               \
	HumdrumFile infile;                         \
	if (interface.getArgCount() > 0) {          \
		infile.read(interface.getArgument(1));   \
	} else {                                    \
		infile.read(cin);                        \
	}                                           \
                                               \
	int status = interface.run(infile, cout);   \
	if (interface.hasError()) {                 \
		cerr << interface.getError();            \
	}                                           \
                                               \
	return !status;                             \
}


// END_MERGE

} // end namespace hum

#endif /* _HUMTOOL_H_INCLUDED */


