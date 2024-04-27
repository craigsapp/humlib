//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Wed Apr 12 19:03:03 PDT 2023
// Last Modified: Wed Apr 12 19:03:06 PDT 2023
// Filename:      tool-grep.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-grep.h
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   C++ implementation of the grep command (sort of).
//

#ifndef _TOOL_GREP_H
#define _TOOL_GREP_H

#include "HumTool.h"
#include "HumdrumFile.h"

#include <ostream>
#include <string>

namespace hum {

// START_MERGE

class Tool_grep : public HumTool {
	public:
		         Tool_grep         (void);
		        ~Tool_grep         () {};

		bool     run               (HumdrumFileSet& infiles);
		bool     run               (HumdrumFile& infile);
		bool     run               (const std::string& indata, std::ostream& out);
		bool     run               (HumdrumFile& infile, std::ostream& out);

	protected:
		void      processFile         (HumdrumFile& infile);
		void      initialize          (void);

	private:
		bool        m_negateQ;    // for the -v option
		std::string m_regex;      // for the -e option

};

// END_MERGE

} // end namespace hum

#endif /* _TOOL_GREP_H */



