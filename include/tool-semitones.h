//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Fri Nov 13 02:39:49 PST 2020
// Last Modified: Sat Nov 14 21:25:27 PST 2020
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
		bool  run              (const std::string& indata, std::ostream& out);
		bool  run              (HumdrumFile& infile, std::ostream& out);

	protected:
		void        processFile(HumdrumFile& infile);
		void        initialize(void);
		void        analyzeLine(HumdrumFile& infile, int line);
		int         processKernSpines(HumdrumFile& infile, int line, int start, int kspine);
		void        printTokens(const std::string& value, int count);
		std::string getTwelveToneIntervalString(HTp token);
		std::string getNextNoteAttack(HTp token);
		void        markInterval(HTp token);
		HTp         markNote(HTp token, bool markQ);
		void        addMarker(HTp token);
		void        showCount(void);
		bool        filterData(HTp token);
		std::vector<HTp> getTieGroup(HTp token);
		HTp         getNextNote(HTp token);
		bool        hasTieContinue(const string& value);

	private:

		bool        m_cdataQ      = false; // used **cdata (to display in VHV notation)
		bool        m_downQ       = false; // mark/count notes in downward interval
		bool        m_firstQ      = false; // mark only first note in interval
		bool        m_leapQ       = false; // mark/count notes in leap motion
		bool        m_midiQ       = false; // give the MIDI note number rather than inteval.
		bool        m_noanalysisQ = false; // do not print analysis spines
		bool        m_noinputQ    = false; // do not print input data
		bool        m_nomarkQ     = false; // do not mark notes (just count intervals)
		bool        m_norestsQ    = false; // ignore rests
		bool        m_notiesQ     = false; // do not mark secondary tied notes
		bool        m_repeatQ     = false; // make/count notes that repeat
		bool        m_secondQ     = false; // mark only second note in interval
		bool        m_stepQ       = false; // mark/count notes in stepwise motion
		bool        m_upQ         = false; // mark/count notes in upward interval
      bool        m_count       = false; // count the number of intervals being marked

		int         m_markCount = 0;
		int         m_leap      = 3;

      std::string m_marker  = "@";
		std::string m_color   = "red";
		std::string m_include;
		std::string m_exclude;

};


// END_MERGE

} // end namespace hum

#endif /* _TOOL_SEMITONES_H_INCLUDED */



