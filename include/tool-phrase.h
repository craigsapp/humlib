//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Thu May 31 16:56:54 PDT 2018
// Last Modified: Thu May 31 16:56:57 PDT 2018
// Filename:      tool-phrase.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-phrase.h
// Syntax:        C++11
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Interface for phrase tool.
//

#ifndef _TOOL_PRASE_H_INCLUDED
#define _TOOL_PRASE_H_INCLUDED

#include "HumTool.h"
#include "HumdrumFile.h"

#include <vector>
#include <string>

namespace hum {

// START_MERGE

class Tool_phrase : public HumTool {
	public:
		      Tool_phrase      (void);
		     ~Tool_phrase      () {};

		bool  run              (HumdrumFile& infile);
		bool  run              (const string& indata, ostream& out);
		bool  run              (HumdrumFile& infile, ostream& out);

	protected:
		void  analyzeSpine     (int index);
		void  initialize       (HumdrumFile& infile);
		void  prepareAnalysis  (HumdrumFile& infile);
		void  addAverageLines  (HumdrumFile& infile);

	private:
		vector<vector<string>> m_results;
		vector<HTp>            m_starts;
		HumdrumFile            m_infile;
		vector<int>            m_pcount;
		vector<HumNum>         m_psum;

};


// END_MERGE

} // end namespace hum

#endif /* _TOOL_PRASE_H_INCLUDED */



