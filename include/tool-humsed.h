//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sun Oct 13 11:42:59 PDT 2019
// Last Modified: Sun Oct 13 11:43:02 PDT 2019
// Filename:      tool-humsed.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-humsed.h
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Sort data spines in a Humdrum file.
//

#ifndef _TOOL_HUMSED_H
#define _TOOL_HUMSED_H

#include "HumTool.h"
#include "HumdrumFile.h"

namespace hum {

// START_MERGE

class Tool_humsed : public HumTool {
	public:
		         Tool_humsed       (void);
		        ~Tool_humsed       () {};

		bool     run               (HumdrumFileSet& infiles);
		bool     run               (HumdrumFile& infile);
		bool     run               (const string& indata, ostream& out);
		bool     run               (HumdrumFile& infile, ostream& out);

	protected:
		void    processFile        (HumdrumFile& infile);
		void    searchAndReplaceInterpretation(HumdrumFile& infile);

	private:
		std::string m_search;      // search string
		std::string m_replace;     // replace string
		bool        m_interpretation = false; // process only interpretation records
		bool        m_modified = false;

};

// END_MERGE

} // end namespace hum

#endif /* _TOOL_HUMSED_H */



