//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Aug 17 02:39:28 PDT 2015
// Last Modified: Mon Aug 17 02:39:32 PDT 2015
// Filename:      HumdrumFileStructure.h
// URL:           https://github.com/craigsapp/minHumdrum/blob/master/include/HumdrumFileStructure.h
// Syntax:        C++11
// vim:           ts=3
//
// Description:   Used to add structural analysis to HumdrumFileBase class.
//

#ifndef _HUMDRUMFILESTRUCTURE_H
#define _HUMDRUMFILESTRUCTURE_H

#include "HumdrumFileBase.h"

using namespace std;

namespace minHumdrum {

// START_MERGE

class HumdrumFileStructure : public HumdrumFileBase {
	public:
		              HumdrumFileStructure         (void);
		             ~HumdrumFileStructure         ();
};


// END_MERGE

} // end namespace std;

#endif /* _HUMDRUMFILESTRUCTURE_H */



