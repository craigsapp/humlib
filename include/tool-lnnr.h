//
// Programmer:    Wolfgang Drescher <drescher.wolfgang@gmail.com>
// Creation Date: Sat Jul 13 2024 04:44:00 CET
// Filename:      tool-lnnr.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-lnnr.h
// Syntax:        C++11; humlib
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Interface for lnnr tool
//

#ifndef _TOOL_LNNR_H
#define _TOOL_LNNR_H

#include "HumTool.h"
#include "HumdrumFile.h"

using namespace std;

namespace hum {

// START_MERGE

class Tool_lnnr : public HumTool {

	public:
		     Tool_lnnr (void);
		     ~Tool_lnnr() {};

		bool run       (HumdrumFileSet& infiles);
		bool run       (HumdrumFile& infile);
		bool run       (const string& indata, ostream& out);
		bool run       (HumdrumFile& infile, ostream& out);

	protected:
		void           initialize  (void);
        void           processFile (HumdrumFile& infile);
		vector<string> getTrackData(HumdrumFile& infile);

	private:
		bool m_indexQ   = false;
		bool m_prependQ = false;
};

// END_MERGE

} // end namespace hum

#endif /* _TOOL_LNNR_H */
