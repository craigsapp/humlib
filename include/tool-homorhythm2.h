//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Aug 26 15:43:58 PDT 2019
// Last Modified: Mon Aug 26 15:44:01 PDT 2019
// Filename:      tool-homorhythm2.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-homorhythm2.h
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Identify homorhythm2 regions of music.
//

#ifndef _TOOL_HOMOPHONIC2_H
#define _TOOL_HOMOPHONIC2_H

#include "HumTool.h"
#include "HumdrumFileSet.h"

namespace hum {

// START_MERGE

class Tool_homorhythm2 : public HumTool {
	public:
		            Tool_homorhythm2    (void);
		           ~Tool_homorhythm2    () {};

		bool        run                (HumdrumFileSet& infiles);
		bool        run                (HumdrumFile& infile);
		bool        run                (const string& indata, ostream& out);
		bool        run                (HumdrumFile& infile, ostream& out);

	protected:
		void        processFile        (HumdrumFile& infile);
		void        initialize         (void);

	private:
		double      m_threshold = 0.6;
		double      m_threshold2 = 0.4;
		vector<double> m_score;
};

// END_MERGE

} // end namespace hum

#endif /* _TOOL_HOMOPHONIC2_H */



