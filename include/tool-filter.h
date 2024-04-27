//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Wed Dec 14 22:21:44 PST 2016
// Last Modified: Wed Dec 14 22:21:47 PST 2016
// Filename:      tool-filter.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-filter.h
// Syntax:        C++11; humlib
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Interface for filter tool.
//

#ifndef _TOOL_FILTER_H_INCLUDED
#define _TOOL_FILTER_H_INCLUDED

#include "HumTool.h"
#include "HumdrumFileSet.h"

#include <string>
#include <utility>
#include <vector>

namespace hum {

// START_MERGE

class Tool_filter : public HumTool {
	public:
		         Tool_filter        (void);
		        ~Tool_filter        () {};

		bool     run                (HumdrumFileSet& infiles);
		bool     run                (HumdrumFile& infile);
		bool     run                (const std::string& indata);

		bool     runUniversal       (HumdrumFileSet& infiles);

	protected:
		void     getCommandList     (std::vector<std::pair<std::string, std::string> >& commands,
		                             HumdrumFile& infile);
		void     getUniversalCommandList(std::vector<std::pair<std::string, std::string> >& commands,
		                             HumdrumFileSet& infiles);
		void     initialize         (HumdrumFile& infile);
		void     removeGlobalFilterLines    (HumdrumFile& infile);
		void     removeUniversalFilterLines (HumdrumFileSet& infiles);
		void     splitPipeline      (std::vector<std::string>& clist, const std::string& command);

	private:
		std::string   m_variant;        // used with -v option.
		bool     m_debugQ = false; // used with --debug option

};

// END_MERGE

} // end namespace hum

#endif /* _TOOL_FILTER_H_INCLUDED */



