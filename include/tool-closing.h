//
// Programmer:    Alexander Morgan
// Creation Date: Sun Aug 16 2026
// Filename:      tool-closing.h
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Count how many voices stop sounding at each observation point
//                of a score, which is used to evaluate potential cadence
//                points.
//

#ifndef _TOOL_CLOSING_H
#define _TOOL_CLOSING_H

#include "HumTool.h"
#include "HumdrumFileSet.h"

#include <ostream>
#include <string>
#include <vector>

namespace hum {

// START_MERGE

class Tool_closing : public HumTool {
	public:
		         Tool_closing        (void);
		        ~Tool_closing        () {};

		bool     run                 (HumdrumFileSet& infiles);
		bool     run                 (HumdrumFile& infile);
		bool     run                 (const std::string& indata, std::ostream& out);
		bool     run                 (HumdrumFile& infile, std::ostream& out);

	protected:
		void     initialize          (void);
		void     processFile         (HumdrumFile& infile);
		void     countClosingVoices  (HumdrumFile& infile);
		void     markClosingEvents   (HumdrumFile& infile);
		void     addAnalysisSpine    (HumdrumFile& infile);

	private:
		// m_counts: closing voice count for each line, or -1 for lines that get
		// no analysis value (such as non-data lines).
		std::vector<int> m_counts;
		bool        m_markQ        = false;
		std::string m_attackMarker = "@";
		std::string m_restMarker   = "N";
		std::string m_attackColor  = "dodgerblue";
		std::string m_restColor    = "orange";
};

// END_MERGE

} // end namespace hum

#endif /* _TOOL_CLOSING_H */
