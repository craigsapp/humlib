//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Apr 30 10:41:35 PDT 2022
// Last Modified: Sat Apr 30 10:41:38 PDT 2022
// Filename:      tool-synco.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-synco.h
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Identify and mark syncopations.
//

#ifndef _TOOL_SYNCO_H
#define _TOOL_SYNCO_H

#include "HumTool.h"
#include "HumdrumFile.h"

namespace hum {

// START_MERGE

class Tool_synco : public HumTool {
	public:
		         Tool_synco        (void);
		        ~Tool_synco        () {};

		bool     run               (HumdrumFileSet& infiles);
		bool     run               (HumdrumFile& infile);
		bool     run               (const string& indata, ostream& out);
		bool     run               (HumdrumFile& infile, ostream& out);

	protected:
		void      processFile      (HumdrumFile& infile);
		void      initialize       (void);

		void      processStrand    (HTp stok, HTp etok);
		bool      isSyncopated     (HTp token);
		double    getMetricLevel   (HTp token);
		void      markNote         (HTp token);

	private:
		bool      m_hasSyncoQ = false;
		int       m_scount    = 0;

};

// END_MERGE

} // end namespace hum

#endif /* _TOOL_SYNCO_H */



