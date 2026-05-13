//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon May  4 20:19:23 PDT 2026
// Last Modified: Mon May  4 20:19:27 PDT 2026
// Filename:      tool-triad.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-triad.h
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Extract **triad spines
//

#ifndef _TOOL_TRAD_H
#define _TOOL_TRAD_H

#include "HumTool.h"
#include "HumdrumFile.h"

#include <ostream>
#include <sstream>
#include <string>

namespace hum {

// START_MERGE

class Tool_triad : public HumTool {
	public:
		         Tool_triad          (void);
		        ~Tool_triad          () {};

		bool     run                  (HumdrumFileSet& infiles);
		bool     run                  (HumdrumFile& infile);
		bool     run                  (const std::string& indata, std::ostream& out);
		bool     run                  (HumdrumFile& infile, std::ostream& out);
		void     processFile          (HumdrumFile& infile);
		std::string fillInMajorMinor  (HumdrumFile& infile, int index);

	protected:
		void     initialize        (void);

	private:
		bool m_appendQ  = false; // -a
		bool m_classQ   = false; // -c
		bool m_lowQ     = false; // -l
		bool m_pitchesQ = false; // -p
		bool m_qualityQ = false; // -q
		bool m_restQ    = false; // -R
		bool m_rootQ    = false; // -r
		bool m_summaryQ = false; // not implemented
		bool m_unisonQ  = false; // -U

};

// END_MERGE

} // end namespace hum

#endif /* _TOOL_TRAD_H */



