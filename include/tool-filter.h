//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Wed Dec 14 22:21:44 PST 2016
// Last Modified: Wed Dec 14 22:21:47 PST 2016
// Filename:      tool-filter.h
// URL:           https://github.com/craigsapp/minHumdrum/blob/master/include/tool-filter.h
// Syntax:        C++11
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Interface for filter tool.
//

#ifndef _TOOL_FILTER_H_INCLUDED
#define _TOOL_FILTER_H_INCLUDED

#include "HumTool.h"
#include "HumdrumFile.h"

namespace hum {

// START_MERGE

class Tool_filter : public HumTool {
	public:
		         Tool_filter   (void);
		        ~Tool_filter   () {};

		bool     run             (HumdrumFile& infile);
		bool     run             (const string& indata, ostream& out);
		bool     run             (HumdrumFile& infile, ostream& out);

	protected:
		void     getCommandList  (vector<pair<string, string> >& commands,
		                          HumdrumFile& infile);
		void     initialize      (HumdrumFile& infile);

	private:
		string   m_variant;        // used with -v option.
		bool     m_debugQ = false; // used with --debug option

};

// END_MERGE

} // end namespace hum

#endif /* _TOOL_FILTER_H_INCLUDED */



