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
		static HumNum    recipToDuration   (const string& recip, 
		                                    string separator = " ");
};



///////////////////////////////////////////////////////////////////////////
// Templates -- Templates for use with minHumdrum.
///////////////////////////////////////////////////////////////////////////



// Print contents of vectors as a tab-separated list:
template <typename A>
ostream& operator<<(ostream& out, const vector<A>& v) {
	for (unsigned int i=0; i<v.size(); i++) {
		out << v[i];
		if (i < v.size() - 1) {		
			out << '\t';
		}
	}
	return out;
}



// Print contents of pointer vectors as a tab-separated list:
template <typename A>
ostream& operator<<(ostream& out, const vector<A*>& v) {
	for (unsigned int i=0; i<v.size(); i++) {
		out << *v[i];
		if (i < v.size() - 1) {		
			out << '\t';
		}
	}
	return out;
}

// END_MERGE

#endif /* _CONVERT */



