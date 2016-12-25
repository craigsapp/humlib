//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Nov 28 08:55:15 PST 2016
// Last Modified: Mon Nov 28 08:55:38 PST 2016
// Filename:      tool-dissonant.h
// URL:           https://github.com/craigsapp/minHumdrum/blob/master/include/tool-dissonant.h
// Syntax:        C++11
// vim:           ts=3 noexpandtab
//
// Description:   Interface for dissonant tool.
//

#ifndef _TOOL_DISSONANT_H
#define _TOOL_DISSONANT_H

#include "HumTool.h"
#include "HumdrumFile.h"
#include "NoteGrid.h"

namespace hum {

// START_MERGE

class Tool_dissonant : public HumTool {
	public:
		         Tool_dissonant    (void);
		        ~Tool_dissonant    () {};

		bool     run               (HumdrumFile& infile);
		bool     run               (const string& indata, ostream& out);
		bool     run               (HumdrumFile& infile, ostream& out);

	protected:
		void    doAnalysis         (vector<vector<string> >& results,
		                            NoteGrid& grid,
		                            bool debug);
		void    doAnalysisForVoice (vector<string>& results, NoteGrid& grid,
		                            int vindex, bool debug);

	private:
	 	vector<HTp> m_kernspines;
		bool diss2Q = false;
		bool diss7Q = false;
		bool diss4Q = false;

};

// END_MERGE

} // end namespace hum

#endif /* _TOOL_DISSONANT_H */



