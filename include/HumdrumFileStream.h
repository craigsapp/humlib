//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Tue Dec 11 16:03:43 PST 2012
// Last Modified: Tue Dec 11 16:03:46 PST 2012
// Last Modified: Fri Mar 11 21:25:24 PST 2016 Changed to STL
// Last Modified: Fri Dec  2 19:26:01 PST 2016 Ported to humlib
// Filename:      HumdrumFileStream.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/HumdrumFileStream.h
// Syntax:        C++11
// vim:           ts=3 noexpandtab
//
// Description:   Multi-movement manager for Humdrum files.  The class
//                will accept files, standard input, URLs, URIs which
//                have more than one data start/stop sequence.  This usually
//                indicates multiple movements if stored in one file, or
//                multiple works if coming in from standard input.
//

#ifndef _HUMDRUMFILESTREAM_H_INCLUDED
#define _HUMDRUMFILESTREAM_H_INCLUDED

#include "HumdrumFile.h"
#include "Options.h"

#include <vector>
#include <sstream>
#include <fstream>
#include <iostream>

namespace hum {

// START_MERGE

class HumdrumFileStream {
	public:
		                HumdrumFileStream  (void);
		                HumdrumFileStream  (char** list);
		                HumdrumFileStream  (const vector<string>& list);
		                HumdrumFileStream  (Options& options);

		int             setFileList        (char** list);
		int             setFileList        (const vector<string>& list);

		void            clear              (void);
		int             eof                (void);

		int             getFile            (HumdrumFile& infile);
		int             read               (HumdrumFile& infile);

	protected:
		ifstream        m_instream;       // used to read from list of files.
		stringstream    m_urlbuffer;      // used to read data over internet.
		string          m_newfilebuffer;  // used to keep track of !!!!segment:
		                                  // records.

		vector<string>  m_filelist;       // used when not using cin
		int             m_curfile;        // index into filelist

		vector<string>  m_universals;     // storage for universal comments

		// Automatic URL downloading of data from internet in read():
		void     fillUrlBuffer            (stringstream& uribuffer,
		                                   const string& uriname);

};

// END_MERGE

} // end namespace hum

#endif /* _HUMDRUMFILESTREAM_H */



