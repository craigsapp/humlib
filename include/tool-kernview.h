//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Thu Jan 30 22:31:31 PST 2020
// Last Modified: Thu Jan 30 22:31:35 PST 2020
// Filename:      tool-kernview.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-kernview.h
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Suppress or show certain staves when rendering notation with verovio.
//

#ifndef _TOOL_KERNVIEW_H
#define _TOOL_KERNVIEW_H

#include "HumTool.h"
#include "HumdrumFile.h"

namespace hum {

// START_MERGE

class Tool_kernview : public HumTool {
	public:
		         Tool_kernview      (void);
		        ~Tool_kernview      () {};

		bool     run               (HumdrumFileSet& infiles);
		bool     run               (HumdrumFile& infile);
		bool     run               (const string& indata, ostream& out);
		bool     run               (HumdrumFile& infile, ostream& out);

	protected:
		void     processFile       (HumdrumFile& infile);
		void     initialize        (void);

	private:
		std::string m_view_string;
		std::string m_hide_string;

};

// END_MERGE

} // end namespace hum

#endif /* _TOOL_KERNVIEW_H */



