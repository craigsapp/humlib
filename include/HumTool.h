//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Nov 28 08:55:15 PST 2016
// Last Modified: Mon Nov 28 08:55:38 PST 2016
// Filename:      HumTool.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/HumTool.h
// Syntax:        C++11; humlib
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Common interface for Humdrum tools.
//

#ifndef _HUMTOOL_H_INCLUDED
#define _HUMTOOL_H_INCLUDED

#include "Options.h"

#include <sstream>
#include <string>

namespace hum {

// START_MERGE

class HumTool : public Options {
	public:
		              HumTool         (void);
		             ~HumTool         ();

		void          clearOutput     (void);

		bool          hasAnyText      (void);
		std::string   getAllText      (void);
		ostream&      getAllText      (ostream& out);

		bool          hasHumdrumText  (void);
		std::string   getHumdrumText  (void);
		ostream&      getHumdrumText  (ostream& out);
		void          suppressHumdrumFileOutput(void);

		bool          hasJsonText     (void);
		std::string   getJsonText     (void);
		ostream&      getJsonText     (ostream& out);

		bool          hasFreeText     (void);
		std::string   getFreeText     (void);
		ostream&      getFreeText     (ostream& out);

		bool          hasWarning      (void);
		std::string   getWarning      (void);
		ostream&      getWarning      (ostream& out);

		bool          hasError        (void);
		std::string   getError        (void);
		ostream&      getError        (ostream& out);

	protected:
		std::stringstream m_humdrum_text;  // output text in Humdrum syntax.
		std::stringstream m_json_text;     // output text in JSON syntax.
		std::stringstream m_free_text;     // output for plain text content.
	  	std::stringstream m_warning_text;  // output for warning messages;
	  	std::stringstream m_error_text;    // output for error messages;

		bool m_suppress = false;

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

#define BASIC_INTERFACE(CLASS)                         \
using namespace std;                                   \
using namespace hum;                                   \
int main(int argc, char** argv) {                      \
	CLASS interface;                                    \
	if (!interface.process(argc, argv)) {               \
		interface.getError(cerr);                        \
		return -1;                                       \
	}                                                   \
	HumdrumFile infile;                                 \
	if (interface.getArgCount() > 0) {                  \
		infile.readNoRhythm(interface.getArgument(1));   \
	} else {                                            \
		infile.readNoRhythm(cin);                        \
	}                                                   \
	int status = interface.run(infile, cout);           \
	if (interface.hasWarning()) {                       \
		interface.getWarning(cerr);                      \
		return 0;                                        \
	}                                                   \
	if (interface.hasError()) {                         \
		interface.getError(cerr);                        \
		return -1;                                       \
	}                                                   \
	return !status;                                     \
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
	if (!interface.process(argc, argv)) {                         \
		interface.getError(cerr);                                  \
		return -1;                                                 \
	}                                                             \
	HumdrumFileStream streamer(static_cast<Options&>(interface)); \
	HumdrumFile infile;                                           \
	bool status = true;                                           \
	while (streamer.read(infile)) {                               \
		status &= interface.run(infile);                           \
		if (interface.hasWarning()) {                              \
			interface.getWarning(cerr);                             \
		}                                                          \
		if (interface.hasAnyText()) {                              \
		   interface.getAllText(cout);                             \
		}                                                          \
		if (interface.hasError()) {                                \
			interface.getError(cerr);                               \
         return -1;                                              \
		}                                                          \
		if (!interface.hasAnyText()) {                             \
			cout << infile;                                         \
		}                                                          \
		interface.clearOutput();                                   \
	}                                                             \
	return !status;                                               \
}



//////////////////////////////
//
// STREAM_INTERFACE2 -- Expects two Humdurm files, either from the
//    first two command-line arguments (left over after options have
//    been parsed out), or from standard input.
//
// function call that the interface must implement:
//  .run(HumdrumFile& infile1, HumdrumFile& infile2, ostream& out)
//
//

#define STREAM_INTERFACE2(CLASS)                                 \
using namespace std;                                             \
using namespace hum;                                             \
int main(int argc, char** argv) {                                \
	CLASS interface;                                              \
	if (!interface.process(argc, argv)) {                         \
		interface.getError(cerr);                                  \
		return -1;                                                 \
	}                                                             \
	HumdrumFileStream streamer(static_cast<Options&>(interface)); \
	HumdrumFile infile1;                                          \
	HumdrumFile infile2;                                          \
	bool status = true;                                           \
	streamer.read(infile1);                                       \
	streamer.read(infile2);                                       \
	status &= interface.run(infile1, infile2);                    \
	if (interface.hasWarning()) {                                 \
		interface.getWarning(cerr);                                \
	}                                                             \
	if (interface.hasAnyText()) {                                 \
	   interface.getAllText(cout);                                \
	}                                                             \
	if (interface.hasError()) {                                   \
		interface.getError(cerr);                                  \
        return -1;                                               \
	}                                                             \
	if (!interface.hasAnyText()) {                                \
		cout << infile1;                                           \
		cout << infile2;                                           \
	}                                                             \
	interface.clearOutput();                                      \
	return !status;                                               \
}


// END_MERGE

} // end namespace hum

#endif /* _HUMTOOL_H_INCLUDED */



