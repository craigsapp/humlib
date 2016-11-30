//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Nov 28 08:55:15 PST 2016
// Last Modified: Mon Nov 28 08:55:38 PST 2016
// Filename:      HumTool.h
// URL:           https://github.com/craigsapp/minHumdrum/blob/master/include/tool-metlev.h
// Syntax:        C++11
// vim:           ts=3 noexpandtab
//
// Description:   Common interface for Humdrum tools.
//

#ifndef _TOOL_METLEV_H
#define _TOOL_METLEV_H

#include "HumTool.h"

namespace hum {

class Tool_metlev : public HumTool {
	public:
		      Tool_metlev        (void);
		     ~Tool_metlev        ();

		bool  run                (HumdrumFile& infile, ostream& out);

	protected:
		void  fillVoiceResults   (vector<vector<double> >& results, 
		                          HumdrumFile& infile,
		                          vector<double>& beatlev);

	private:
		vector<HTp> m_kernspines;


};

} // end namespace hum

#endif /* _TOOL_METLEV_H */



