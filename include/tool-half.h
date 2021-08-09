//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Fri Jul 30 07:06:48 CEST 2021
// Last Modified: Fri Jul 30 07:06:51 CEST 2021
// Filename:      tool-half.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-half.h
// Syntax:        C++11; humlib
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Double durations of notes/rests/time signatures.
//

#ifndef _TOOL_HALF_H_INCLUDED
#define _TOOL_HALF_H_INCLUDED

#include "HumTool.h"
#include "HumdrumFile.h"

namespace hum {

// START_MERGE

class Tool_half : public HumTool {
	public:
		         Tool_half     (void);
		        ~Tool_half     () {};

		bool     run           (HumdrumFileSet& infiles);
		bool     run           (HumdrumFile& infile);
		bool     run           (const string& indata, ostream& out);
		bool     run           (HumdrumFile& infile, ostream& out);

	protected:
		void     initialize    (HumdrumFile& infile);
		void     processFile   (HumdrumFile& infile);
		void     halfRhythms   (HumdrumFile& infile);
		void     terminalLongToTerminalBreve(HumdrumFile& infile);
		void     adjustBeams   (HumdrumFile& infile);

	private:
		bool     m_lyricBreakQ = false;  // used with -l option

};

// END_MERGE

} // end namespace hum

#endif /* _TOOL_HALF_H_INCLUDED */



