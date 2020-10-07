//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Wed Sep  9 21:59:29 PDT 2020
// Last Modified: Wed Sep  9 21:59:32 PDT 2020
// Filename:      tool-colorgroups.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-colorgroups.h
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Color diatonic triadic sonorities.
//

#ifndef _TOOL_COLORGROUPS_H
#define _TOOL_COLORGROUPS_H

#include "HumTool.h"
#include "HumdrumFile.h"

namespace hum {

// START_MERGE

class Tool_colorgroups : public HumTool {
	public:
		         Tool_colorgroups  (void);
		        ~Tool_colorgroups  () {};

		bool     run               (HumdrumFileSet& infiles);
		bool     run               (HumdrumFile& infile);
		bool     run               (const string& indata, ostream& out);
		bool     run               (HumdrumFile& infile, ostream& out);

	protected:
		void     processFile       (HumdrumFile& infile);
		void     initialize        (void);

};

// END_MERGE

} // end namespace hum

#endif /* _TOOL_COLORGROUPS_H */



