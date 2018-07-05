//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Fri Jul  6 00:16:03 CEST 2018
// Last Modified: Fri Jul  6 00:16:06 CEST 2018
// Filename:      tool-slur.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-slur.h
// Syntax:        C++11
// vim:           ts=3 noexpandtab
//
// Description:   Interface for fixing adjusting chords (order of pitches,
//                rhythms, articulatios).
//

#ifndef _TOOL_SLUR_H
#define _TOOL_SLUR_H

#include "HumTool.h"
#include "HumdrumFile.h"

namespace hum {

// START_MERGE

class Tool_slur : public HumTool {
	public:
		         Tool_slur         (void);
		        ~Tool_slur         () {};

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

#endif /* _TOOL_SLUR_H */



