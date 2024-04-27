//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon May  3 10:01:46 PDT 2021
// Last Modified: Mon May  3 10:01:49 PDT 2021
// Filename:      tool-timebase.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-timebase.h
// Syntax:        C++11; humlib
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Interface for timebase tool, which causes data lines to
//                have the same duration.
//

#ifndef _TOOL_TIMEBASE_H_INCLUDED
#define _TOOL_TIMEBASE_H_INCLUDED

#include "HumTool.h"
#include "HumdrumFile.h"

#include <ostream>
#include <string>
#include <vector>

namespace hum {

// START_MERGE

class Tool_timebase : public HumTool {
	public:
		      Tool_timebase       (void);
		     ~Tool_timebase       () {};

		bool  run                 (HumdrumFileSet& infiles);
		bool  run                 (HumdrumFile& infile);
		bool  run                 (const std::string& indata, std::ostream& out);
		bool  run                 (HumdrumFile& infile, std::ostream& out);

	protected:
		void   processFile         (HumdrumFile& infile);
		HumNum getMinimumTime      (HumdrumFile& infile);
		void   expandScore         (HumdrumFile& infile, HumNum mindur);

	private:
		bool   m_grace   = false;
		bool   m_quiet   = false;
		HumNum m_basedur = false;

};


// END_MERGE

} // end namespace hum

#endif /* _TOOL_TIMEBASE_H_INCLUDED */



