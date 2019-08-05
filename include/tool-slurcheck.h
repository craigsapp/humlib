//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Fri Jul  6 00:16:03 CEST 2018
// Last Modified: Fri Jul  6 00:16:06 CEST 2018
// Filename:      tool-slurcheck.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-slurcheck.h
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Interface for fixing adjusting chords (order of pitches,
//                rhythms, articulatios).
//

#ifndef _TOOL_SLURCHECK_H
#define _TOOL_SLURCHECK_H

#include "HumTool.h"
#include "HumdrumFile.h"

namespace hum {

// START_MERGE

class Tool_slurcheck : public HumTool {
	public:
		         Tool_slurcheck    (void);
		        ~Tool_slurcheck    () {};

		bool     run               (HumdrumFileSet& infiles);
		bool     run               (HumdrumFile& infile);
		bool     run               (const string& indata, ostream& out);
		bool     run               (HumdrumFile& infile, ostream& out);

	protected:
		void     processFile       (HumdrumFile& infile);
		void     initialize        (void);

	private:

};

// END_MERGE

} // end namespace hum

#endif /* _TOOL_SLURCHECK_H */



