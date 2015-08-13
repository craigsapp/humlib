//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Aug  8 12:24:49 PDT 2015
// Last Modified: Sun Aug  9 21:03:12 PDT 2015
// Filename:      Convert.h
// URL:           https://github.com/craigsapp/minHumdrum/blob/master/include/Convert.h
// Syntax:        C++11
// vim:           ts=3
//
// Description:   Convert between various data representations.
//

#ifndef _CONVERT_H
#define _CONVERT_H

#include <iostream>
#include <vector>

#include "HumNum.h"

using namespace std;

// START_MERGE

class Convert {
	public:
		static HumNum    recipToDuration  (const string& recip, HumNum scale = 4,
		                                    string separator = " ");
};



// END_MERGE

#endif /* _CONVERT */



