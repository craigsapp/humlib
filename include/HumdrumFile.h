//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Aug 17 02:39:28 PDT 2015
// Last Modified: Mon Aug 17 02:39:32 PDT 2015
// Filename:      HumdrumFile.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/HumdrumFile.h
// Syntax:        C++11
// vim:           ts=3 noexpandtab
//
// Description:   Placeholder class to serve as interface to HumdrumFileBase,
//                HumdrumFileStructure and HumdrumFileContent.
//

#ifndef _HUMDRUMFILE_H
#define _HUMDRUMFILE_H

#include "HumdrumFileContent.h"

using namespace std;

namespace hum {

// START_MERGE

#ifndef HUMDRUMFILE_PARENT
	#define HUMDRUMFILE_PARENT HumdrumFileContent
#endif

class HumdrumFile : public HUMDRUMFILE_PARENT {
	public:
		              HumdrumFile         (void);
		              HumdrumFile         (const string& filename);
		              HumdrumFile         (istream& filename);
		             ~HumdrumFile         ();

		ostream&      printXml            (ostream& out = cout, int level = 0,
		                                   const string& indent = "\t");
};


// END_MERGE

} // end namespace hum

#endif /* _HUMDRUMFILE_H */



