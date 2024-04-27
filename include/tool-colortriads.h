//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Wed Sep  9 21:59:29 PDT 2020
// Last Modified: Wed Sep  9 21:59:32 PDT 2020
// Filename:      tool-colortriads.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-colortriads.h
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Color diatonic triadic sonorities.
//

#ifndef _TOOL_COLORTRIADS_H
#define _TOOL_COLORTRIADS_H

#include "HumTool.h"
#include "HumdrumFile.h"

#include <ostream>
#include <string>
#include <vector>

namespace hum {

// START_MERGE

class Tool_colortriads : public HumTool {
	public:
		         Tool_colortriads  (void);
		        ~Tool_colortriads  () {};

		bool     run               (HumdrumFileSet& infiles);
		bool     run               (HumdrumFile& infile);
		bool     run               (const std::string& indata, std::ostream& out);
		bool     run               (HumdrumFile& infile, std::ostream& out);

	protected:
		void     processFile       (HumdrumFile& infile);
		void     initialize        (void);
		int      getDiatonicTransposition(HumdrumFile& infile);

	private:
		std::vector<bool> m_colorState;
		std::vector<std::string> m_color;
		std::vector<std::string> m_searches;
		std::vector<std::string> m_marks;
		bool m_filtersQ  = false;
		bool m_commandsQ = false;
		bool m_relativeQ = false;
		std::string m_key;

};

// END_MERGE

} // end namespace hum

#endif /* _TOOL_COLORTRIADS_H */



