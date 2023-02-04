//
// Programmer:    Wolfgang Drescher <drescher.wolfgang@gmail.com>
// Creation Date: Sa  4 Feb 2023 16:27:58 CET
// Filename:      tool-lyricsformatter.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-lyricsformatter.h
// Syntax:        C++11; humlib
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Interface for lyricsformatter tool, which formats lyrics of **text spines.
//

#ifndef _TOOL_LYRICSFORMATTER_H
#define _TOOL_LYRICSFORMATTER_H

#include "HumTool.h"
#include "HumdrumFile.h"

namespace hum {

// START_MERGE

class Tool_lyricsformatter : public HumTool {

	public:
		     Tool_lyricsformatter (void);
		     ~Tool_lyricsformatter() {};

		bool run(HumdrumFileSet& infiles);
		bool run(HumdrumFile& infile);
		bool run(const string& indata, ostream& out);
		bool run(HumdrumFile& infile, ostream& out);

	protected:
		void initialize      (void);
        void processFile     (HumdrumFile& infile);
		void addUnderlines   (vector<HTp> spineStartList);
		void removeUnderlines(vector<HTp> spineStartList);


	private:
		bool m_addUnderlineQ;
		bool m_removeUnderlineQ;

};

// END_MERGE

} // end namespace hum

#endif /* _TOOL_LYRICSFORMATTER_H */
