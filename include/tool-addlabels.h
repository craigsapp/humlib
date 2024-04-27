//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Apr 15 00:16:09 PDT 2024
// Last Modified: Mon Apr 15 00:16:13 PDT 2024
// Filename:      tool-addlabels.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-addlabels.h
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Options:       -q # = set tempo in units of quarter notes per minute.
//                -q "#; m#:#; m#:#" = set three tempos, one at start, and two
//                at given measures.
//
// Description:   Insert tempo interpretations from -q option.
//                example
//                    addlabels -t 60.24
//                Add *MM60.24 in **kern spines at start of music (preferably
//                right after key signature / metric symbol).
//                    addlabels -t "60; m12:132; m24:92"
//                Add *MM60 at start of music, *MM132 at measure 12, and
//                *MM92 at measure 24.
//

#ifndef _TOOL_ADDLABELS_H
#define _TOOL_ADDLABELS_H

#include "HumTool.h"
#include "HumdrumFile.h"

#include <ostream>
#include <string>
#include <vector>

namespace hum {

// START_MERGE

class Tool_addlabels : public HumTool {
	public:
		         Tool_addlabels     (void);
		        ~Tool_addlabels     () {};

		bool     run               (HumdrumFileSet& infiles);
		bool     run               (HumdrumFile& infile);
		bool     run               (const std::string& indata, std::ostream& out);
		bool     run               (HumdrumFile& infile, std::ostream& out);

	protected:
		void    processFile        (HumdrumFile& infile);
		void    initialize         (void);
		int     getExpansionIndex  (HumdrumFile& infile);
		void    printExpansionLists(HumdrumFile& infile, int index);
		void    assignLabels       (std::vector<std::string>& llist, HumdrumFile& infile);
		void    addLabel           (std::vector<std::string>& llist, HumdrumFile& infile,
		                            int barnum, int subbarnum, const std::string& label);
	private:
		std::string m_default;             // set with the -d option
		std::string m_norep;               // set with the -r option
		std::string m_zeroth;              // m0 label (right after default and norep expansion lists)
		int m_defaultIndex = -1;           // line to place default expansions list above (and then norep)
		std::vector<int> m_barnums;        // set with -l option
		std::vector<int> m_subbarnums;     // set with -l option
		std::vector<std::string> m_labels; // set with -l option
};

// END_MERGE

} // end namespace hum

#endif /* _TOOL_ADDLABELS_H */



