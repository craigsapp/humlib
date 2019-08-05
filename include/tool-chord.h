//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Fri Feb  9 21:19:09 PST 2018
// Last Modified: Fri Feb  9 21:19:13 PST 2018
// Filename:      tool-chord.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-chord.h
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Interface for fixing adjusting chords (order of pitches,
//                rhythms, articulatios).
//

#ifndef _TOOL_CHORD_H
#define _TOOL_CHORD_H

#include "HumTool.h"
#include "HumdrumFile.h"

namespace hum {

// START_MERGE

class Tool_chord : public HumTool {
	public:
		         Tool_chord      (void);
		        ~Tool_chord      () {};

		bool     run               (HumdrumFileSet& infiles);
		bool     run               (HumdrumFile& infile);
		bool     run               (const string& indata, ostream& out);
		bool     run               (HumdrumFile& infile, ostream& out);

	protected:
		void     processFile       (HumdrumFile& infile, int direction);
		void     processChord      (HTp tok, int direction);
		void     initialize        (void);
		void     minimizeChordPitches(vector<string>& notes, vector<pair<int,int>>& pitches);
		void     maximizeChordPitches(vector<string>& notes, vector<pair<int,int>>& pitches);

	private:
		int       m_direction = 0;
		int       m_spine     = -1;
		int       m_primary   = 0;

};

// END_MERGE

} // end namespace hum

#endif /* _TOOL_CHORD_H */



