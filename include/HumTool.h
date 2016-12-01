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

#ifndef _HUMTOOL_H
#define _HUMTOOL_H

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

// END_MERGE

} // end namespace hum

#endif /* _HUMTOOL_H */



