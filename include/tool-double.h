//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Fri Jul 30 04:56:20 CEST 2021
// Last Modified: Fri Jul 30 04:56:23 CEST 2021
// Filename:      tool-double.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-double.h
// Syntax:        C++11; humlib
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Double durations of notes/rests/time signatures.
//

#ifndef _TOOL_DOUBLE_H_INCLUDED
#define _TOOL_DOUBLE_H_INCLUDED

#include "HumTool.h"
#include "HumdrumFile.h"

namespace hum {

// START_MERGE

class Tool_double : public HumTool {
	public:
		         Tool_double     (void);
		        ~Tool_double     () {};

		bool     run                (HumdrumFileSet& infiles);
		bool     run                (HumdrumFile& infile);
		bool     run                (const string& indata, ostream& out);
		bool     run                (HumdrumFile& infile, ostream& out);

	protected:
		void     initialize         (HumdrumFile& infile);
		void     processFile        (HumdrumFile& infile);
		void     doubleRhythms      (HumdrumFile& infile);
		void     terminalBreveToTerminalLong(HumdrumFile& infile);
		void     processBeamsForMeasure(vector<HTp>& notes);
		void     adjustBeams        (HumdrumFile& infile);
		void     adjustBeams        (HTp sstart, HTp send);

	private:

};

// END_MERGE

} // end namespace hum

#endif /* _TOOL_DOUBLE_H_INCLUDED */



