//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Aug 17 02:39:28 PDT 2015
// Last Modified: Mon Aug 17 02:39:32 PDT 2015
// Filename:      HumdrumFile.h
// URL:           https://github.com/craigsapp/minHumdrum/blob/master/include/HumdrumFile.h
// Syntax:        C++11
// vim:           ts=3 noexpandtab
//
// Description:   Used to add structural analysis to HumdrumFileBase class.
//

#ifndef _HUMDRUMFILE_H
#define _HUMDRUMFILE_H

#include "HumdrumFileContent.h"

using namespace std;

namespace minHumdrum {

// START_MERGE

class HumdrumFile : public HumdrumFileContent {
	public:
		              HumdrumFile         (void);
		             ~HumdrumFile         ();
};


// END_MERGE

} // end namespace std;

#endif /* _HUMDRUMFILE_H */



