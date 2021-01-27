//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Jul 29 16:48:22 CEST 2019
// Last Modified: Mon Jan 18 11:30:01 PST 2021
// Filename:      tool-composite.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-composite.h
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Fill in kern measures that have no duration with rests.
//

#ifndef _TOOL_COMPOSITE_H
#define _TOOL_COMPOSITE_H

#include "HumTool.h"
#include "HumdrumFile.h"

namespace hum {

// START_MERGE

class Tool_composite : public HumTool {
	public:
		       	   Tool_composite      (void);
		       	  ~Tool_composite      () {};

		bool        run                (HumdrumFileSet& infiles);
		bool        run                (HumdrumFile& infile);
		bool        run                (const string& indata, ostream& out);
		bool        run                (HumdrumFile& infile, ostream& out);

	protected:
		void        processFile        (HumdrumFile& infile);
		void        initialize         (void);
		HumNum      getLineDuration    (HumdrumFile& infile, int index, vector<bool>& isNull);
		void        setupGrouping      (vector<vector<string>>& grouping, HumdrumFile& infile);
		void        printGroupingInfo  (vector<vector<string>>& gouping);
		string      getGroup           (vector<vector<string>>& current, int spine, int subspine);
		bool        hasGroup           (vector<vector<string>>& grouping, HumdrumFile& infile, int line,
		                                const string& group);
		int         getGroupNoteType   (vector<vector<string>>& grouping, HumdrumFile& infile,
		                                int line, const string& group);

	private:
		string      m_pitch = "e";

};

// END_MERGE

} // end namespace hum

#endif /* _TOOL_COMPOSITE_H */



