//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Wed Jun 12 12:15:23 CEST 2019
// Last Modified: Wed Jun 12 12:15:26 CEST 2019
// Filename:      tool-tabber.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-tabber.h
// Syntax:        C++11
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Add/remove spacing tabs between spines.
//

#ifndef _TOOL_TABBER_H_INCLUDED
#define _TOOL_TABBER_H_INCLUDED

#include "HumTool.h"
#include "HumdrumFile.h"

namespace hum {

// START_MERGE

class Tool_tabber : public HumTool {
	public:
		      Tool_tabber              (void);
		     ~Tool_tabber              () {};

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

#endif /* _TOOL_TABBER_H_INCLUDED */



