//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Nov 28 08:55:15 PST 2016
// Last Modified: Mon Nov 28 08:55:38 PST 2016
// Filename:      tool-metlev.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-metlev.h
// Syntax:        C++11; humlib
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Interface for metlev tool.
//

#ifndef _TOOL_METLEV_H_INCLUDED
#define _TOOL_METLEV_H_INCLUDED

#include "HumTool.h"
#include "HumdrumFile.h"

#include <ostream>
#include <string>
#include <vector>

namespace hum {

// START_MERGE

class Tool_metlev : public HumTool {
	public:
		      Tool_metlev      (void);
		     ~Tool_metlev      () {};

		bool  run              (HumdrumFileSet& infiles);
		bool  run              (HumdrumFile& infile);
		bool  run              (const std::string& indata, std::ostream& out);
		bool  run              (HumdrumFile& infile, std::ostream& out);

	protected:
		void  fillVoiceResults (std::vector<std::vector<double> >& results,
		                        HumdrumFile& infile,
		                        std::vector<double>& beatlev);

	private:
		std::vector<HTp> m_kernspines;

};


// END_MERGE

} // end namespace hum

#endif /* _TOOL_METLEV_H_INCLUDED */



