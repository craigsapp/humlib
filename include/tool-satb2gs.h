//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sun Feb  6 14:33:36 PST 2011
// Last Modified: Wed Nov 30 20:36:41 PST 2016
// Filename:      tool-satb2gs.h
// URL:           https://github.com/craigsapp/minHumdrum/blob/master/include/tool-satb2gs.h
// Syntax:        C++11
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Interface for satb2gs tool.
//

#ifndef _TOOL_AUTOBEAM_H_INCLUDED
#define _TOOL_AUTOBEAM_H_INCLUDED

#include "HumTool.h"
#include "HumdrumFile.h"

#include <vector>

namespace hum {

// START_MERGE

class Tool_satb2gs : public HumTool {
	public:
		         Tool_satb2gs    (void);
		        ~Tool_satb2gs    () {};

		bool     run             (HumdrumFile& infile);
		bool     run             (const string& indata, ostream& out);
		bool     run             (HumdrumFile& infile, ostream& out);

	protected:
		void     initialize      (HumdrumFile& infile);
		void     processFile     (HumdrumFile& infile);
		void     example         (void);
		void     usage           (const string& command);
		void     convertData     (HumdrumFile& infile);
		int      getSatbTracks   (vector<int>& tracks, HumdrumFile& infile);
		void     printSpine      (HumdrumFile& infile, int row, int col, 
		                          vector<int>& satbtracks);
		void     printExInterp   (HumdrumFile& infile, int line, 
		                          vector<int>& tracks);
		void     printLastLine   (HumdrumFile& infile, int line, 
		                          vector<int>& tracks);
	private:
		int    debugQ    = 0;             // used with --debug option
};


// END_MERGE

} // end namespace hum

#endif /* _TOOL_AUTOBEAM_H_INCLUDED */



