//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Thu Sep  9 21:30:46 PDT 2004
// Last Modified: Tue Jul 22 11:44:06 CEST 2025
// Filename:      tool-barnum.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-barnum.h
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Number, renumber, or remove measure numbers from Humdrum files.
//

#ifndef _TOOL_BARNUM_H
#define _TOOL_BARNUM_H

#include "HumTool.h"
#include "HumdrumFile.h"

#include <ostream>
#include <string>
#include <vector>

namespace hum {

// START_MERGE

class Tool_barnum : public HumTool {
	public:
		         Tool_barnum          (void);
		        ~Tool_barnum          () {};

		bool     run                  (HumdrumFileSet& infiles);
		bool     run                  (HumdrumFile& infile);
		bool     run                  (const std::string& indata, std::ostream& out);
		bool     run                  (HumdrumFile& infile, std::ostream& out);

	protected:
		void     initialize           (void);
		void     removeBarNumbers     (HumdrumFile& infile);
		void     renumberBarNumbers   (HumdrumFile& infile);
		void     printWithoutBarNumbers(HumdrumLine& line);
		void     printWithBarNumbers  (HumdrumLine& line, int measurenum);

	private:
		bool     m_removeQ  = false;   // -r: remove bar numbers
		int      m_startnum = 1;       // -s: starting bar number
		bool     m_allQ     = false;   // -a: number all barlines
		bool     m_debugQ   = false;   // --debug
};

// END_MERGE

} // end namespace hum

#endif /* _TOOL_BARNUM_H */



