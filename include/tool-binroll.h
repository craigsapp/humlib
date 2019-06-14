//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sun Mar  4 21:15:15 PST 2018
// Last Modified: Sun Mar  4 21:15:19 PST 2018
// Filename:      tool-binroll.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-binroll.h
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Extract a binary pinao roll of note in a score.
//

#ifndef _TOOL_BINROLL_H
#define _TOOL_BINROLL_H

#include "HumTool.h"
#include "HumNum.h"
#include "HumdrumFile.h"

namespace hum {

// START_MERGE

class Tool_binroll : public HumTool {
	public:
		         Tool_binroll      (void);
		        ~Tool_binroll      () {};

		bool     run               (HumdrumFile& infile);
		bool     run               (const string& indata, ostream& out);
		bool     run               (HumdrumFile& infile, ostream& out);

	protected:
		void     processFile       (HumdrumFile& infile);
		void     processStrand     (vector<vector<char>>& roll, HTp starting,
		                            HTp ending);
		void     printAnalysis     (HumdrumFile& infile,
		                            vector<vector<char>>& roll);

	private:
		HumNum    m_duration;

};

// END_MERGE

} // end namespace hum

#endif /* _TOOL_BINROLL_H */



