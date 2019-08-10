//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Fri Aug  9 17:57:41 EDT 2019
// Last Modified: Fri Aug  9 17:57:45 EDT 2019
// Filename:      tool-homophonic.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-homophonic.h
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Identify homophonic regions of music.
//

#ifndef _TOOL_HOMOPHONIC_H
#define _TOOL_HOMOPHONIC_H

#include "HumTool.h"
#include "HumdrumFileSet.h"

namespace hum {

// START_MERGE

class HPNote {
	public:
		int track = -1;
		int line = -1;
		int field = -1;
		int subfield = -1;
		HTp token = NULL;
		std::string text;
		bool attack = false;
		bool nullQ = false;
};

class Tool_homophonic : public HumTool {
	public:
		            Tool_homophonic    (void);
		           ~Tool_homophonic    () {};

		bool        run                (HumdrumFileSet& infiles);
		bool        run                (HumdrumFile& infile);
		bool        run                (const string& indata, ostream& out);
		bool        run                (HumdrumFile& infile, ostream& out);

	protected:
		void        processFile        (HumdrumFile& infile);
		void        analyzeLine        (HumdrumFile& infile, int line);
		void        initialize         (void);
		void        markHomophonicNotes(void);

	private:
		std::vector<std::string> m_homophonic;
		std::vector<int> m_notecount;
		std::vector<int> m_attacks;
		std::vector<std::vector<HPNote>> m_notes;
		int m_count = 4;

};

// END_MERGE

} // end namespace hum

#endif /* _TOOL_HOMOPHONIC_H */



