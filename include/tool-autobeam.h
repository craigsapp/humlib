//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Wed Nov 30 20:36:38 PST 2016
// Last Modified: Wed Nov 30 20:36:41 PST 2016
// Filename:      tool-autobeam.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-autobeam.h
// Syntax:        C++11; humlib
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Interface for autobeam tool.
//

#ifndef _TOOL_AUTOBEAM_H_INCLUDED
#define _TOOL_AUTOBEAM_H_INCLUDED

#include "HumTool.h"
#include "HumdrumFile.h"

namespace hum {

// START_MERGE

class Tool_autobeam : public HumTool {
	public:
		         Tool_autobeam   (void);
		        ~Tool_autobeam   () {};

		bool     run             (HumdrumFile& infile);
		bool     run             (const string& indata, ostream& out);
		bool     run             (HumdrumFile& infile, ostream& out);

	protected:
		void     initialize      (HumdrumFile& infile);
		void     processStrand   (HTp strandstart, HTp strandend);
		void     processMeasure  (vector<HTp>& measure);
		void     addBeam         (HTp startnote, HTp endnote);
		void     addBeams        (HumdrumFile& infile);
		void     removeBeams     (HumdrumFile& infile);

	private:
		vector<vector<pair<int, HumNum> > > m_timesigs;
		vector<HTp> m_kernspines;
		bool        m_overwriteQ;
		int         m_track;

};

// END_MERGE

} // end namespace hum

#endif /* _TOOL_AUTOBEAM_H_INCLUDED */



