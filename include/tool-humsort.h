//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Wed Jan 16 22:54:51 EST 2019
// Last Modified: Wed Jan 16 22:54:53 EST 2019
// Filename:      tool-humsort.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-humsort.h
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Sort data spines in a Humdrum file.
//

#ifndef _TOOL_HUMSORT_H
#define _TOOL_HUMSORT_H

#include "HumTool.h"
#include "HumdrumFile.h"

namespace hum {

// START_MERGE

class Tool_humsort : public HumTool {
	public:
		         Tool_humsort      (void);
		        ~Tool_humsort      () {};

		bool     run               (HumdrumFileSet& infiles);
		bool     run               (HumdrumFile& infile);
		bool     run               (const string& indata, ostream& out);
		bool     run               (HumdrumFile& infile, ostream& out);

	protected:
		void    processFile        (HumdrumFile& infile);

};

// END_MERGE

} // end namespace hum

#endif /* _TOOL_HUMSORT_H */



