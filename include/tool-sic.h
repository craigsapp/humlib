//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Thu Jan 30 22:31:31 PST 2020
// Last Modified: Thu Jan 30 22:31:35 PST 2020
// Filename:      tool-sic.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-sic.h
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Switch between sic encoding and corrected encoding.
//

#ifndef _TOOL_SIC_H
#define _TOOL_SIC_H

#include "HumTool.h"
#include "HumdrumFile.h"

namespace hum {

// START_MERGE

class Tool_sic : public HumTool {
	public:
		         Tool_sic       (void);
		        ~Tool_sic       () {};

		bool     run               (HumdrumFileSet& infiles);
		bool     run               (HumdrumFile& infile);
		bool     run               (const string& indata, ostream& out);
		bool     run               (HumdrumFile& infile, ostream& out);

	protected:
		void     processFile       (HumdrumFile& infile);
		void     initialize        (void);
		void     insertOriginalToken(HTp sictok);
		void     insertSubstitutionToken(HTp sictok);
		HTp      getTargetToken     (HTp stok);
		void     addVerboseParameter(HTp token);
		void     removeVerboseParameter(HTp token);

	private:
		bool     m_substituteQ = false;
		bool     m_originalQ   = false;
		bool     m_removeQ     = false;
		bool     m_verboseQ    = false;
		bool     m_quietQ      = false;
		bool     m_modifiedQ   = false;

};

// END_MERGE

} // end namespace hum

#endif /* _TOOL_SIC_H */



