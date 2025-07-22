//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Thu Sep  9 21:30:46 PDT 2004
// Last Modified: Tue Jul 22 19:47:39 CEST 2025
// Filename:      tool-barnum.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-barnum.h
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Process measure numbers on barlines in a Humdrum file.
//

#ifndef _TOOL_BARNUM_H
#define _TOOL_BARNUM_H

#include "HumTool.h"
#include "HumdrumFile.h"

#include <ostream>
#include <string>

namespace hum {

// START_MERGE

class Tool_barnum : public HumTool {
	public:
		         Tool_barnum          (void);
		        ~Tool_barnum          () {};

		bool     run               (HumdrumFileSet& infiles);
		bool     run               (HumdrumFile& infile);
		bool     run               (const std::string& indata, std::ostream& out);
		bool     run               (HumdrumFile& infile, std::ostream& out);

	protected:

		void     processFile             (HumdrumFile& infile);
		void     initialize              (void);
		void     removeBarNumbers        (HumdrumFile& infile);
		void     printWithoutBarNumbers  (HumdrumLine& humline);
		void     printWithBarNumbers     (HumdrumLine& humline, int measurenum);
		void     printSingleBarNumber    (const std::string& astring, int measurenum);
		int      getEndingBarline        (HumdrumFile& infile);


	private:
   	bool m_removeQ;
   	int m_startnum;
   	bool m_debugQ;
   	bool m_allQ;

};

// END_MERGE

} // end namespace hum

#endif /* _TOOL_BARNUM_H */



