//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Wed Nov 13 09:02:20 PST 2024
// Last Edited:   Wed Nov 13 09:02:24 PST 2024
// Filename:      tool-bardash.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-bardash.h
// Syntax:        C++11; humlib
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Notate hypermeters.
//							*bar:dash=3
//                      solid bar every three barlines, dashes for other two
//							*bar:dash=2
//                      solid bar every two barlines, dashes for other one
//							*bar:dot=2
//                      solid bar every two barlines, dots for other one
//							*bar:invis=2
//                      solid bar every two barlines, invisible for other one
//

#ifndef _TOOL_BARDASH_H
#define _TOOL_BARDASH_H

#include "HumTool.h"
#include "HumdrumFile.h"

namespace hum {

// START_MERGE

class Tool_bardash : public HumTool {

	public:
		     Tool_bardash (void);
		     ~Tool_bardash() {};

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

#endif /* _TOOL_BARDASH_H */



