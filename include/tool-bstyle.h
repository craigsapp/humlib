//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Wed Nov 13 09:02:20 PST 2024
// Last Edited:   Wed Nov 13 09:02:24 PST 2024
// Filename:      tool-bstyle.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-bstyle.h
// Syntax:        C++11; humlib
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Notate hypermeters.
//							*bstyle:dash=3
//                      solid bar every three barlines, dashes for other two
//							*bstyle:dash=2
//                      solid bar every two barlines, dashes for other one
//							*bstyle:dot=2
//                      solid bar every two barlines, dots for other one
//							*bstyle:invis=2
//                      solid bar every two barlines, invisible for other one
//                   *bstyle:stop
//                      turn off last styling message
//

#ifndef _TOOL_BSTYLE_H
#define _TOOL_BSTYLE_H

#include "HumTool.h"
#include "HumdrumFile.h"

namespace hum {

// START_MERGE

class Tool_bstyle : public HumTool {

	public:
		     Tool_bstyle (void);
		     ~Tool_bstyle() {};

		bool run     (HumdrumFileSet& infiles);
		bool run     (HumdrumFile& infile);
		bool run     (const std::string& indata, std::ostream& out);
		bool run     (HumdrumFile& infile, std::ostream& out);

	protected:
		void         initialize   (void);
      void         processFile  (HumdrumFile& infile);
		void         removeBarStylings(HumdrumFile& infile);
		void         removeBarStylings(HTp spine);
		void         applyBarStylings(HumdrumFile& infile);
		void         applyBarStylings(HTp spine);

	private:
		bool  m_removeQ = false;  // used with -r option

};

// END_MERGE

} // end namespace hum

#endif /* _TOOL_BSTYLE_H */



