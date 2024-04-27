//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Thu Jul 16 17:57:09 PDT 2020
// Last Modified: Thu Jul 16 19:05:33 PDT 2020
// Filename:      tool-tie.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-tie.h
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Interface for fixing adjusting chords (order of pitches,
//                rhythms, articulatios).
//

#ifndef _TOOL_TIE_H
#define _TOOL_TIE_H

#include "HumTool.h"
#include "HumdrumFile.h"

#include <ostream>
#include <string>

namespace hum {

// START_MERGE

class Tool_tie : public HumTool {
	public:
		         Tool_tie                (void);
		        ~Tool_tie                () {};

		bool     run                     (HumdrumFileSet& infiles);
		bool     run                     (HumdrumFile& infile);
		bool     run                     (const std::string& indata, std::ostream& out);
		bool     run                     (HumdrumFile& infile, std::ostream& out);

	protected:
		void     processFile             (HumdrumFile& infile);
		void     initialize              (void);
		void     mergeTies               (HumdrumFile& infile);
		void     mergeTie                (HTp token);
		int      markOverfills           (HumdrumFile& infile);
		bool     checkForOverfill        (HTp tok);
		bool     checkForInvisible       (HTp tok);
		void     markNextBarlineInvisible(HTp tok);
		void     splitOverfills          (HumdrumFile& infile);
		void     splitToken              (HTp tok);
		void     carryForwardLeftoverDuration(HumNum duration, HTp tok);
		HumNum   getDurationToNextVisibleBarline(HTp tok);
		HumNum   getDurationToNextBarline(HTp tok);

	private:
		bool          m_printQ         = false;
		bool          m_mergeQ         = false;
		bool          m_splitQ         = false;
		bool          m_markQ          = false;
		bool          m_invisibleQ     = false;
		bool          m_skipInvisibleQ = false;
		std::string   m_mark           = "@";

};

// END_MERGE

} // end namespace hum

#endif /* _TOOL_TIE_H */



