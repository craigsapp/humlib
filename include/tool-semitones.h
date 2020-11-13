//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Nov 28 08:55:15 PST 2016
// Last Modified: Mon Nov 28 08:55:38 PST 2016
// Filename:      tool-semitones.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-semitones.h
// Syntax:        C++11; humlib
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Interface for semitones tool.
//

#ifndef _TOOL_SEMITONES_H_INCLUDED
#define _TOOL_SEMITONES_H_INCLUDED

#include "HumTool.h"
#include "HumdrumFile.h"

namespace hum {

// START_MERGE

class Tool_semitones : public HumTool {
	public:
		      Tool_semitones   (void);
		     ~Tool_semitones   () {};

		bool  run              (HumdrumFileSet& infiles);
		bool  run              (HumdrumFile& infile);
		bool  run              (const string& indata, ostream& out);
		bool  run              (HumdrumFile& infile, ostream& out);

	protected:
		void   processFile(HumdrumFile& infile);
		void   initialize(void);
		void   analyzeLine(HumdrumFile& infile, int line);
		int    processKernSpines(HumdrumFile& infile, int line, int start);
		void   printTokens(const string& value, int count);
		string getTwelveToneIntervalString(HTp token);
		string getNextNoteAttack(HTp token);

	private:
		bool m_cdata = false; // used **cdata (to display in VHV notation)
		bool m_midi  = false; // give the MIDI note number rather than inteval.

};


// END_MERGE

} // end namespace hum

#endif /* _TOOL_SEMITONES_H_INCLUDED */



