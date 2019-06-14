//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Thu Aug 10 08:25:50 EDT 2017
// Last Modified: Thu Aug 10 08:25:53 EDT 2017
// Filename:      tool-ruthfix.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-ruthfix.h
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Interface for fixing early-music MusicXML import where a tied note over a barline
//                is represented as an invisible rest.  This occurs in MusicXML output from Sibelius.
//

#ifndef _TOOL_RUTHFIX_H
#define _TOOL_RUTHFIX_H

#include "HumTool.h"
#include "HumdrumFile.h"
#include "NoteGrid.h"

namespace hum {

// START_MERGE

class Tool_ruthfix : public HumTool {
	public:
		         Tool_ruthfix      (void);
		        ~Tool_ruthfix      () {};

		bool     run               (HumdrumFile& infile);
		bool     run               (const string& indata, ostream& out);
		bool     run               (HumdrumFile& infile, ostream& out);

	protected:
		void    insertCrossBarTies (HumdrumFile& infile);
		void    insertCrossBarTies (HumdrumFile& infile, int strand);
		void    createTiedNote     (HTp left, HTp right);

};

// END_MERGE

} // end namespace hum

#endif /* _TOOL_RUTHFIX_H */



