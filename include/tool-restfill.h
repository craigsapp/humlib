//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sun Jul 14 01:00:53 CEST 2019
// Last Modified: Sun Jul 14 01:00:56 CEST 2019
// Filename:      tool-restfill.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-restfill.h
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Fill in kern measures that have no duration with rests.
//

#ifndef _TOOL_RESTFILL_H
#define _TOOL_RESTFILL_H

#include "HumTool.h"
#include "HumdrumFile.h"

namespace hum {

// START_MERGE

class Tool_restfill : public HumTool {
	public:
		         Tool_restfill         (void);
		        ~Tool_restfill         () {};

		bool        run                (HumdrumFile& infile);
		bool        run                (const string& indata, ostream& out);
		bool        run                (HumdrumFile& infile, ostream& out);

	protected:
		void        processFile        (HumdrumFile& infile);
		void        initialize         (void);
		bool        hasBlankMeasure    (HTp start);
		void        fillInRests        (HTp start);
		void        addRest            (HTp cell, HumNum duration);
		HumNum      getNextTime        (HTp token);

	private:
		bool        m_hiddenQ  = false;
		std::string m_exinterp = "**kern";

};

// END_MERGE

} // end namespace hum

#endif /* _TOOL_RESTFILL_H */



