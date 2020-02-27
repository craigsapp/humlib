//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Wed Feb 26 10:31:49 PST 2020
// Last Modified: Wed Feb 26 10:31:53 PST 2020
// Filename:      tool-humsheet.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-humsheet.h
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Generate spread-sheet like HTML table for music.
//

#ifndef _TOOL_HUMSHEET_H
#define _TOOL_HUMSHEET_H

#include "HumTool.h"
#include "HumdrumFile.h"

namespace hum {

// START_MERGE

class Tool_humsheet : public HumTool {
	public:
		         Tool_humsheet       (void);
		        ~Tool_humsheet       () {};

		bool     run               (HumdrumFileSet& infiles);
		bool     run               (HumdrumFile& infile);
		bool     run               (const string& indata, ostream& out);
		bool     run               (HumdrumFile& infile, ostream& out);

		void     printRowClasses   (HumdrumFile& infile, int row);
		void     printRowContents  (HumdrumFile& infile, int row);
      void     printCellClasses  (HTp token);
      void     printHtmlHeader   (void);
      void     printHtmlFooter   (void);
      void     printStyle        (void);

	protected:
		void     processFile       (HumdrumFile& infile);
		void     initialize        (void);
		void     analyzeTracks     (HumdrumFile& infile);
		void     printColSpan      (HTp token);

	private:
		bool             m_htmlQ           = false;
		std::vector<int> m_max_subtrack;
		int              m_max_track       = 0;
		int              m_max_field       = 0;

};

// END_MERGE

} // end namespace hum

#endif /* _TOOL_HUMSHEET_H */



