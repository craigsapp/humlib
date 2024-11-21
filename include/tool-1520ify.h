//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Sep  9 08:07:21 PDT 2024
// Last Modified: Mon Sep  9 08:07:24 PDT 2024
// Filename:      tool-1520ify.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-1520ify.h
// Syntax:        C++11; humlib
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Interface for 1520ify tool.
//

#ifndef _TOOL_1520IFY_H_INCLUDED
#define _TOOL_1520IFY_H_INCLUDED

#include "HumTool.h"
#include "HumdrumFile.h"

#include <ostream>
#include <string>
#include <vector>

namespace hum {

// START_MERGE

class Tool_1520ify : public HumTool {
	public:
		            Tool_1520ify       (void);
		           ~Tool_1520ify       () {};

		bool        run                (HumdrumFileSet& infiles);
		bool        run                (HumdrumFile& infile);
		bool        run                (const std::string& indata, std::ostream& out);
		bool        run                (HumdrumFile& infile, std::ostream& out);

	protected:
		void        initialize         (HumdrumFile& infile);
		void        processFile        (HumdrumFile& infile);
		void        updateKeySignatures(HumdrumFile& infile, int lineindex);
		void        checkDataLine      (HumdrumFile& infile, int lineindex);
		void        clearStates        (void);
		void        addBibliographicRecords(HumdrumFile& infile);
		void        deleteBreaks       (HumdrumFile& infile);
		void        fixEditorialAccidentals(HumdrumFile& infile);
		void        fixInstrumentAbbreviations(HumdrumFile& infile);
		void        addTerminalLongs   (HumdrumFile& infile);
		void        deleteDummyTranspositions(HumdrumFile& infile);
		std::string getDate            (void);
		int         getYear            (void);
		void        adjustSystemDecoration(HumdrumFile& infile);

	private:
		std::vector<std::vector<int>>  m_pstates;
		std::vector<std::vector<int>>  m_kstates;
		std::vector<std::vector<bool>> m_estates;

};

// END_MERGE

} // end namespace hum

#endif /* _TOOL_1520IFY_H_INCLUDED */



