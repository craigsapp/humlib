//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Wed Nov 30 20:36:38 PST 2016
// Last Modified: Wed Nov 30 20:36:41 PST 2016
// Filename:      tool-tassoize.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-tassoize.h
// Syntax:        C++11; humlib
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Interface for tassoize tool.
//

#ifndef _TOOL_TASSOIZE_H_INCLUDED
#define _TOOL_TASSOIZE_H_INCLUDED

#include "HumTool.h"
#include "HumdrumFile.h"

#include <ostream>
#include <string>
#include <vector>

namespace hum {

// START_MERGE

class Tool_tassoize : public HumTool {
	public:
		         Tool_tassoize   (void);
		        ~Tool_tassoize   () {};

		bool     run                (HumdrumFileSet& infiles);
		bool     run                (HumdrumFile& infile);
		bool     run                (const std::string& indata, std::ostream& out);
		bool     run                (HumdrumFile& infile, std::ostream& out);

	protected:
		void     initialize         (HumdrumFile& infile);
		void     processFile        (HumdrumFile& infile);
		void     updateKeySignatures(HumdrumFile& infile, int lineindex);
		void     checkDataLine      (HumdrumFile& infile, int lineindex);
		void     clearStates        (void);
		void     addBibliographicRecords(HumdrumFile& infile);
		void     deleteBreaks       (HumdrumFile& infile);
		void     fixEditorialAccidentals(HumdrumFile& infile);
		void     fixInstrumentAbbreviations(HumdrumFile& infile);
		void     addTerminalLongs   (HumdrumFile& infile);
		void     deleteDummyTranspositions(HumdrumFile& infile);
		std::string   getDate            (void);
		void     adjustSystemDecoration(HumdrumFile& infile);

	private:
		std::vector<std::vector<int>> m_pstates;
		std::vector<std::vector<int>> m_kstates;
		std::vector<std::vector<bool>> m_estates;

};

// END_MERGE

} // end namespace hum

#endif /* _TOOL_TASSOIZE_H_INCLUDED */



