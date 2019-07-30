//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Jul 29 16:48:22 CEST 2019
// Last Modified: Mon Jul 29 16:48:25 CEST 2019
// Filename:      tool-composite.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-composite.h
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Fill in kern measures that have no duration with rests.
//

#ifndef _TOOL_COMPOSITE_H
#define _TOOL_COMPOSITE_H

#include "HumTool.h"
#include "HumdrumFile.h"

namespace hum {

// START_MERGE

class Tool_composite : public HumTool {
	public:
		       	   Tool_composite      (void);
		       	  ~Tool_composite      () {};

		bool        run                (HumdrumFile& infile);
		bool        run                (const string& indata, ostream& out);
		bool        run                (HumdrumFile& infile, ostream& out);

	protected:
		void        processFile        (HumdrumFile& infile);
		void        initialize         (void);

	private:
		string      m_pitch = "e";

};

// END_MERGE

} // end namespace hum

#endif /* _TOOL_COMPOSITE_H */



