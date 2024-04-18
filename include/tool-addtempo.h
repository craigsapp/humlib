//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Apr 15 00:16:09 PDT 2024
// Last Modified: Mon Apr 15 00:16:13 PDT 2024
// Filename:      tool-addtempo.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-addtempo.h
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Options:       -q # = set tempo in units of quarter notes per minute.
//                -q "#; m#:#; m#:#" = set three tempos, one at start, and two
//                at given measures.
//
// Description:   Insert tempo interpretations from -q option.
//                example
//                    addtempo -t 60.24
//                Add *MM60.24 in **kern spines at start of music (preferably
//                right after key signature / metric symbol).
//                    addtempo -t "60; m12:132; m24:92"
//                Add *MM60 at start of music, *MM132 at measure 12, and
//                *MM92 at measure 24.
//

#ifndef _TOOL_ADDTEMPO_H
#define _TOOL_ADDTEMPO_H

#include "HumTool.h"
#include "HumdrumFile.h"

namespace hum {

// START_MERGE

class Tool_addtempo : public HumTool {
	public:
		         Tool_addtempo     (void);
		        ~Tool_addtempo     () {};

		bool     run               (HumdrumFileSet& infiles);
		bool     run               (HumdrumFile& infile);
		bool     run               (const string& indata, std::ostream& out);
		bool     run               (HumdrumFile& infile, std::ostream& out);

	protected:
		void    processFile        (HumdrumFile& infile);
		void    initialize         (void);
		void    assignTempoChanges (std::vector<double>& tlist,
		                            HumdrumFile& infile);
		void    addTempo           (vector<double>& tlist,
		                            HumdrumFile& infile,
		                            int measure, double tempo);
		void   addTempoToStart     (vector<double>& tlist,
		                            HumdrumFile& infile, double tempo);

	private:
		std::vector<std::pair<int, double>> m_tempos;

};

// END_MERGE

} // end namespace hum

#endif /* _TOOL_ADDTEMPO_H */



