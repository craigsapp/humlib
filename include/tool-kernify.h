//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Tue Mar 21 14:10:55 PDT 2023
// Last Modified: Tue Mar 21 14:10:58 PDT 2023
// Filename:      tool-kernify.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-kernify.h
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Add a dummy kern spine for cases where the input data
//                has no staff-like data spines (**kern, **mens).
//

#ifndef _TOOL_KERNIFY_H
#define _TOOL_KERNIFY_H

#include "HumTool.h"
#include "HumdrumFile.h"

namespace hum {

// START_MERGE

class Tool_kernify : public HumTool {
	public:
		         Tool_kernify (void);
		        ~Tool_kernify () {};

		bool     run          (HumdrumFileSet& infiles);
		bool     run          (HumdrumFile& infile);
		bool     run          (const string& indata, ostream& out);
		bool     run          (HumdrumFile& infile, ostream& out);

	protected:
		void        initialize             (void);
		void        processFile            (HumdrumFile& infile);
		void        generateDummyKernSpine (HumdrumFile& infile);
		std::string makeNullLine           (HumdrumLine& line);
		std::string makeReverseLine        (HumdrumLine& line);

	private:
		bool m_forceQ = false;  // used with -f option

};

// END_MERGE

} // end namespace hum

#endif /* _TOOL_KERNIFY_H */



