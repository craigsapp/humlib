//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Nov 28 08:55:15 PST 2016
// Last Modified: Mon Nov 28 08:55:38 PST 2016
// Filename:      tool-testgrid.h
// URL:           https://github.com/craigsapp/minHumdrum/blob/master/include/tool-testgrid.h
// Syntax:        C++11
// vim:           ts=3 noexpandtab
//
// Description:   Interface for testgrid tool.
//

#ifndef _TOOL_TESTGRID_H
#define _TOOL_TESTGRID_H

#include "HumTool.h"
#include "HumdrumFile.h"
#include "NoteGrid.h"

namespace hum {

// START_MERGE

class Tool_testgrid : public HumTool {
	public:
		         Tool_testgrid   (void);
		        ~Tool_testgrid   () {};

		bool     run             (HumdrumFile& infile, ostream& out);

	protected:
		void    doAnalysis  (vector<vector<string> >& results, NoteGrid& grid,
		                     bool debug);
		void    doAnalysisA (vector<string>& results, NoteGrid& grid, int vindex,
		                     bool debug);
		void    doAnalysisB (vector<string>& results, NoteGrid& grid, int vindex,
		                     bool debug);

	private:
		vector<HTp> m_kernspines;

};

// END_MERGE

} // end namespace hum

#endif /* _TOOL_TESTGIRD_H */



