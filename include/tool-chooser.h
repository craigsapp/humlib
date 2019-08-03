//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Thu Aug  1 10:41:57 CEST 2019
// Last Modified: Thu Aug  1 10:42:01 CEST 2019
// Filename:      tool-chooser.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-chooser.h
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Select particular sections of an input stream.
//

#ifndef _TOOL_COMPOSITE_H
#define _TOOL_COMPOSITE_H

#include "HumTool.h"
#include "HumdrumFileSet.h"

namespace hum {

// START_MERGE

class Tool_chooser : public HumTool {
	public:
		       	   Tool_chooser       (void);
		       	  ~Tool_chooser       () {};

		bool        run                (const string& indata);
		bool        run                (HumdrumFileStream& instream);

	protected:
		void        processFiles       (HumdrumFileSet& infiles);
		void        initialize         (void);

		void        expandSegmentList  (vector<int>& field, string& fieldstring,
		                                int maximum);
		void        processSegmentEntry(vector<int>& field,
		                                const string& astring, int maximum);
		void        removeDollarsFromString(string& buffer, int maximum);

	private:

};

// END_MERGE

} // end namespace hum

#endif /* _TOOL_COMPOSITE_H */



