//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Sep  2 11:58:11 CEST 2023
// Last Modified: Mon Sep  4 10:23:13 CEST 2023
// Filename:      tool-pline.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-pline.h
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Poetic line processing tool.
//

#ifndef _TOOL_PLINE_H
#define _TOOL_PLINE_H

#include "HumTool.h"
#include "HumdrumFile.h"

namespace hum {

// START_MERGE

class Tool_pline : public HumTool {
	public:
		         Tool_pline        (void);
		        ~Tool_pline        () {};

		bool     run               (HumdrumFileSet& infiles);
		bool     run               (HumdrumFile& infile);
		bool     run               (const string& indata, ostream& out);
		bool     run               (HumdrumFile& infile, ostream& out);


	protected:
		void     initialize        (void);
		void     processFile       (HumdrumFile& infile);
		void     getPlineInterpretations(HumdrumFile& infile, vector<HTp>& tokens);
		void     plineToColor      (HumdrumFile& infile, vector<HTp>& tokens);
		void     markRests         (HumdrumFile& infile);
		void     markSpineRests    (HTp spineStop);
		void     fillLineInfo      (HumdrumFile& infile, std::vector<std::vector<int>>& lineInfo);

	private:
		bool     m_colorQ   = false;  // used with -c option
		// bool     m_overlapQ = false;  // used with -o option

		std::vector<std::string> m_colors;
		std::vector<HTp> m_ptokens;
		std::vector<std::vector<int>> m_lineInfo;


};

// END_MERGE

} // end namespace hum

#endif /* _TOOL_PLINE_H */



