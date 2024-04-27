//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sun Oct 18 16:51:22 PDT 2020
// Last Modified: Sun Oct 18 16:51:25 PDT 2020
// Filename:      tool-strophe.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-strophe.h
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Switch between strophe encoding and corrected encoding.
//

#ifndef _TOOL_STROPHE_H
#define _TOOL_STROPHE_H

#include "HumTool.h"
#include "HumdrumFile.h"

#include <ostream>
#include <set>
#include <string>

namespace hum {

// START_MERGE

class Tool_strophe : public HumTool {
	public:
		         Tool_strophe       (void);
		        ~Tool_strophe       () {};

		bool     run               (HumdrumFileSet& infiles);
		bool     run               (HumdrumFile& infile);
		bool     run               (const std::string& indata, std::ostream& out);
		bool     run               (HumdrumFile& infile, std::ostream& out);

	protected:
		void     processFile       (HumdrumFile& infile);
		void     initialize        (void);
		void     displayStropheVariants(HumdrumFile& infile);
		void     markWithColor     (HumdrumFile& infile);
		int      markStrophe       (HTp strophestart, HTp stropheend);

	private:
		bool         m_listQ;      // boolean for showing a list of variants
		bool         m_markQ;      // boolean for marking strophes
		std::string  m_marker;     // character for marking strophes
		std::string  m_color;      // color for strphe notes/rests
      std::set<std::string> m_variants;  // used for --list option

};

// END_MERGE

} // end namespace hum

#endif /* _TOOL_STROPHE_H */



