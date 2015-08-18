//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Aug 17 02:39:28 PDT 2015
// Last Modified: Mon Aug 17 02:39:32 PDT 2015
// Filename:      HumdrumFileContent.h
// URL:           https://github.com/craigsapp/minHumdrum/blob/master/include/HumdrumFileContent.h
// Syntax:        C++11
// vim:           ts=3 noexpandtab
//
// Description:   Used to add structural analysis to HumdrumFileBase class.
//

#ifndef _HUMDRUMFILECONTENT_H
#define _HUMDRUMFILECONTENT_H

#include "HumdrumFileStructure.h"

using namespace std;

namespace minHumdrum {

// START_MERGE

class HumdrumFileContent : public HumdrumFileStructure {
	public:
		              HumdrumFileContent         (void);
		             ~HumdrumFileContent         ();
};


// END_MERGE

} // end namespace std;

#endif /* _HUMDRUMFILECONTENT_H */



