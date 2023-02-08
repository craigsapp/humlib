//
// Programmer:    Wolfgang Drescher <drescher.wolfgang@gmail.com>
// Creation Date: Sa  4 Feb 2023 16:27:58 CET
// Filename:      tool-worex.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-worex.h
// Syntax:        C++11; humlib
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Interface for worex tool, which adds word extenders to the lyrics.
//

#ifndef _TOOL_WOREX_H
#define _TOOL_WOREX_H

#include "HumTool.h"
#include "HumdrumFile.h"

namespace hum {

// START_MERGE

class Tool_worex : public HumTool {

	public:
		     Tool_worex (void);
		     ~Tool_worex() {};

		bool run(HumdrumFileSet& infiles);
		bool run(HumdrumFile& infile);
		bool run(const string& indata, ostream& out);
		bool run(HumdrumFile& infile, ostream& out);

	protected:
		void initialize         (void);
        void processFile        (HumdrumFile& infile);
		void addWordExtenders   (vector<HTp> spineStartList);
		void removeWordExtenders(vector<HTp> spineStartList);
		HTp  getParallelNote    (HTp token);


	private:
		bool m_removeWordExtenderQ;

};

// END_MERGE

} // end namespace hum

#endif /* _TOOL_WOREX_H */
