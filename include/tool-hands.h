//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Thu Jun  6 23:06:39 PDT 2024
// Last Modified: Thu Jun  6 23:06:42 PDT 2024
// Filename:      tool-hands.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-hands.h
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Analysis of hands for keyboard music.  *LH for left hand and *RH for right hand.
//                In chords with different hands from the prevailing *LH/*RH use:
//                   !LO:N:LH:n=2
//                for the second note in the chord being left hand, while the other note in *RH
//                being for the right hand (not implemented yet).
//

#ifndef _TOOL_HANDS_H
#define _TOOL_HANDS_H

#include "HumTool.h"
#include "HumdrumFile.h"

#include <ostream>
#include <string>
#include <vector>

namespace hum {

// START_MERGE

class Tool_hands : public HumTool {
	public:
		            Tool_hands        (void);
		           ~Tool_hands        () {};

		bool        run               (HumdrumFileSet& infiles);
		bool        run               (HumdrumFile& infile);
		bool        run               (const std::string& indata, std::ostream& out);
		bool        run               (HumdrumFile& infile, std::ostream& out);

	protected:
		void        processFile       (HumdrumFile& infile);
		void        initialize        (void);
		void        colorHands        (HumdrumFile& infile);
		void        doHandAnalysis    (HumdrumFile& infile);
		void        doHandAnalysis    (HTp startSpine);
		void        markNotes         (HumdrumFile& infile);
		void        markNotes         (HTp sstart, HTp send);
		void        removeNotes       (HumdrumFile& infile, const std::string& htype);
		void        removeNotes       (HTp sstart, HTp send, const std::string& htype);

	private:
		bool        m_colorQ      = false;        // used with -c option
		std::string m_leftColor   = "dodgerblue"; // used with --left-color option
		std::string m_rightColor  = "crimson";    // used with --right-color option
		bool        m_markQ       = false;        // used with -m option
		std::string m_leftMarker  = "🟦";         //
		std::string m_rightMarker = "🟥";         //
		bool        m_leftOnlyQ   = false;        // used with -l option
		bool        m_rightOnlyQ  = false;        // used with -r option
		bool        m_attacksOnlyQ = false;       // used with -a option

		std::vector<int> m_trackHasHandMarkup;    // given track number uses hand labels

};

// END_MERGE

} // end namespace hum

#endif /* _TOOL_HANDS_H */



