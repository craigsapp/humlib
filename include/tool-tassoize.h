//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Wed Nov 30 20:36:38 PST 2016
// Last Modified: Wed Nov 30 20:36:41 PST 2016
// Filename:      tool-tassoize.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-tassoize.h
// Syntax:        C++11
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Interface for tassoize tool.
//

#ifndef _TOOL_TASSOIZE_H_INCLUDED
#define _TOOL_TASSOIZE_H_INCLUDED

#include "HumTool.h"
#include "HumdrumFile.h"

namespace hum {

// START_MERGE

class Tool_tassoize : public HumTool {
	public:
		         Tool_tassoize   (void);
		        ~Tool_tassoize   () {};

		bool     run                (HumdrumFile& infile);
		bool     run                (const string& indata, ostream& out);
		bool     run                (HumdrumFile& infile, ostream& out);

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
		string   getDate            (void);

	private:
		vector<vector<int>> m_pstates;
		vector<vector<int>> m_kstates;
		vector<vector<bool>> m_estates;

};

// END_MERGE

} // end namespace hum

#endif /* _TOOL_TASSOIZE_H_INCLUDED */



