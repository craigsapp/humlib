//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Fri Aug  9 17:57:41 EDT 2019
// Last Modified: Wed Aug 14 07:49:56 EDT 2019
// Filename:      tool-homorhythm.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-homorhythm.h
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Identify homorhythm regions of music.
//

#ifndef _TOOL_HOMOPHONIC_H
#define _TOOL_HOMOPHONIC_H

#include "HumTool.h"
#include "HumdrumFileSet.h"

#include <ostream>
#include <string>
#include <vector>

namespace hum {

// START_MERGE

class HPNote {
	public:
		int track = -1;
		int line = -1;
		int field = -1;
		int subfield = -1;
		HTp token = NULL;
		HumNum duration = 0;
		std::string text;
		bool attack = false;
		bool nullQ = false;
};

class Tool_homorhythm : public HumTool {
	public:
		            Tool_homorhythm    (void);
		           ~Tool_homorhythm    () {};

		bool        run                (HumdrumFileSet& infiles);
		bool        run                (HumdrumFile& infile);
		bool        run                (const std::string& indata, std::ostream& out);
		bool        run                (HumdrumFile& infile, std::ostream& out);

	protected:
		void        processFile        (HumdrumFile& infile);
		void        analyzeLine        (HumdrumFile& infile, int line);
		void        initialize         (void);
		void        markHomophonicNotes(void);
		int         getExtantVoiceCount(HumdrumFile& infile);
		int         getOriginalVoiceCount(HumdrumFile& infile);
		void        addRawAnalysis     (HumdrumFile& infile, std::vector<double>& raw);
		void        addAccumulatedScores(HumdrumFile& infile, std::vector<double>& score);
		void        addAttacks         (HumdrumFile& infile, std::vector<int>& attacks);
		void        addFractionAnalysis(HumdrumFile& infile, std::vector<double>& score);

	private:
		std::vector<std::string> m_homorhythm;
		std::vector<int> m_notecount;
		std::vector<int> m_attacks;
		std::vector<std::vector<HPNote>> m_notes;
		double m_threshold = 4.0;
		double m_score = 1.0;
		double m_intermediate_score = 0.5;
		int m_voice_count = 0;
		bool m_letterQ = false;
};

// END_MERGE

} // end namespace hum

#endif /* _TOOL_HOMOPHONIC_H */



