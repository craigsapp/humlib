//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Oct 16 21:44:03 PDT 2000
// Last Modified: Fri Jun 14 11:45:17 CEST 2019 Updated for humlib.
// Filename:      tool-spinetrace.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-spinetrace.h
// Syntax:        C++11; humlib
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Identifies data field spine memberships
//

#ifndef _TOOL_SPINETRACE_H_INCLUDED
#define _TOOL_SPINETRACE_H_INCLUDED

#include "HumTool.h"
#include "HumdrumFile.h"

namespace hum {

// START_MERGE

class Tool_spinetrace : public HumTool {
	public:
		      Tool_spinetrace          (void);
		     ~Tool_spinetrace          () {};

		bool  run                      (HumdrumFile& infile);
		bool  run                      (const string& indata, ostream& out);
		bool  run                      (HumdrumFile& infile, ostream& out);

	protected:
		void  initialize               (HumdrumFile& infile);
		void  processFile              (HumdrumFile& infile);

	private:

};


// END_MERGE

} // end namespace hum

#endif /* _TOOL_SPINETRACE_H_INCLUDED */



