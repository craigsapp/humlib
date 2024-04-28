//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Apr  1 01:06:00 PDT 2024
// Last Modified: Mon Apr  1 23:38:33 PDT 2024
// Filename:      tool-humbreak.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-humbreak.h
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Insert line/system breaks and page breaks before input measures.
//

#ifndef _TOOL_HUMBREAK_H
#define _TOOL_HUMBREAK_H

#include "HumTool.h"
#include "HumdrumFile.h"

#include <map>
#include <ostream>
#include <string>

namespace hum {

// START_MERGE

class Tool_humbreak : public HumTool {
	public:
		         Tool_humbreak     (void);
		        ~Tool_humbreak     () {};

		bool     run               (HumdrumFileSet& infiles);
		bool     run               (HumdrumFile& infile);
		bool     run               (const std::string& indata, std::ostream& out);
		bool     run               (HumdrumFile& infile, std::ostream& out);

	protected:
		void    processFile        (HumdrumFile& infile);
		void    initialize         (void);
		void    addBreaks          (HumdrumFile& infile);
		void    removeBreaks       (HumdrumFile& infile);
		void    convertPageToLine  (HumdrumFile& infile);
		void    markLineBreakMeasures(HumdrumFile& infile);

	private:
		std::map<int, int> m_lineMeasures;  // list of measures to add line breaks to
		std::map<int, int> m_pageMeasures;  // list of measures to add page breaks to

		std::map<int, int> m_lineOffset;  // measure offsets for measure breaks
		std::map<int, int> m_pageOffset;  // measure offsets for page breaks

		std::string m_group = "original";
		bool m_removeQ = false;
		bool m_page2lineQ = false;

};

// END_MERGE

} // end namespace hum

#endif /* _TOOL_HUMBREAK_H */



