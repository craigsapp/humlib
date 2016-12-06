//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Nov 28 08:55:15 PST 2016
// Last Modified: Mon Nov 28 08:55:38 PST 2016
// Filename:      HumTool.h
// URL:           https://github.com/craigsapp/minHumdrum/blob/master/include/HumTool.h
// Syntax:        C++11
// vim:           syntax=cpp ts=3 noexpandtab nowrap
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
		         HumTool              (void);
		        ~HumTool              ();

		bool     hasNonHumdrumOutput  (void);
		string   getTextOutput        (void);
		ostream& getTextOutput        (ostream& out);
		bool     hasError             (void);
		string   getError             (void);
		ostream& getError             (ostream& out);

	protected:
		stringstream m_text;   // output for non-humdrum output;
	  	stringstream m_error;  // output for error text;

};


///////////////////////////////////////////////////////////////////////////
//
// common command-line Interfaces
//

//////////////////////////////
//
// BASIC_INTERFACE -- Expects one Humdurm file, either from the
//    first command-line argument (left over after options have been
//    parsed out), or from standard input.
//
// function call that the interface must implement:
//  .run(HumdrumFile& infile, ostream& out)
//
//

#define BASIC_INTERFACE(CLASS)                 \
using namespace std;                           \
using namespace hum;                           \
int main(int argc, char** argv) {              \
	CLASS interface;                            \
	interface.process(argc, argv);              \
	HumdrumFile infile;                         \
	if (interface.getArgCount() > 0) {          \
		infile.read(interface.getArgument(1));   \
	} else {                                    \
		infile.read(cin);                        \
	}                                           \
	int status = interface.run(infile, cout);   \
	if (interface.hasError()) {                 \
		interface.getError(cerr);                \
	}                                           \
	return !status;                             \
}



//////////////////////////////
//
// STREAM_INTERFACE -- Expects one Humdurm file, either from the
//    first command-line argument (left over after options have been
//    parsed out), or from standard input.
//
// function call that the interface must implement:
//  .run(HumdrumFile& infile, ostream& out)
//
//

#define STREAM_INTERFACE(CLASS)                                  \
using namespace std;                                             \
using namespace hum;                                             \
int main(int argc, char** argv) {                                \
	CLASS interface;                                              \
	interface.process(argc, argv);                                \
	HumdrumFileStream streamer(static_cast<Options&>(interface)); \
	HumdrumFile infile;                                           \
	bool status = true;                                           \
	while (streamer.read(infile)) {                               \
		status &= interface.run(infile, cout);                     \
		if (interface.hasError()) {                                \
			interface.getError(cerr);                               \
		}                                                          \
	}                                                             \
	return !status;                                               \
}


// END_MERGE

} // end namespace hum

#endif /* _HUMTOOL_H_INCLUDED */



