//
// Programmer:    Wolfgang Drescher <drescher.wolfgang@gmail.com>
// Creation Date: Mon Oct 14 19:24:00 UTC 2024
// Filename:      tool-notemark.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-notemark.h
// Syntax:        C++11; humlib
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Interface for notemark tool
//

#ifndef _TOOL_NOTEMARK_H
#define _TOOL_NOTEMARK_H

#include "HumTool.h"
#include "HumdrumFile.h"

using namespace std;

namespace hum {

// START_MERGE

class Tool_notemark : public HumTool {

	public:
		     Tool_notemark (void);
		     ~Tool_notemark() {};

		bool run           (HumdrumFileSet& infiles);
		bool run           (HumdrumFile& infile);
		bool run           (const string& indata, ostream& out);
		bool run           (HumdrumFile& infile, ostream& out);

	protected:
		void               initialize         (void);
        void               processFile        (HumdrumFile& infile);
		int                getStartLineNumber (void);
		int                getEndLineNumber   (void);
		std::vector<int>   setupSpineInfo     (HumdrumFile& infile);

	private:
		// m_kernSpines: list of all **kern spines found in file.
		std::vector<HTp> m_kernSpines;

		// m_selectedKernSpines: list of only the **kern spines that will be analyzed.
		std::vector<HTp> m_selectedKernSpines;

		std::string m_kernTracks;  // used with -s option
		std::string m_spineTracks; // used with -k option
		std::string m_lineRange;   // used with -l option
		std::string m_signifier;   // used with --signifier option
		std::string m_color;       // used with --color option
};

// END_MERGE

} // end namespace hum

#endif /* _TOOL_NOTEMARK_H */
