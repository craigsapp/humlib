//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Fri Aug  9 17:57:41 EDT 2019
// Last Modified: Wed Aug 14 07:49:56 EDT 2019
// Filename:      tool-homophonic.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-homophonic.h
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Identify homophonic regions of music.
//

#ifndef _TOOL_HOMOPHONIC_H
#define _TOOL_HOMOPHONIC_H

#include "HumTool.h"
#include "HumdrumFileSet.h"

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

class Tool_homophonic : public HumTool {
	public:
		            Tool_homophonic    (void);
		           ~Tool_homophonic    () {};

		bool        run                (HumdrumFileSet& infiles);
		bool        run                (HumdrumFile& infile);
		bool        run                (const string& indata, ostream& out);
		bool        run                (HumdrumFile& infile, ostream& out);

	protected:
		void        processFile        (HumdrumFile& infile);
		void        analyzeLine        (HumdrumFile& infile, int line);
		void        initialize         (void);
		void        markHomophonicNotes(void);
		void        printFractionAnalysis(HumdrumFile& infile, std::vector<double>& score);
		int         getExtantVoiceCount(HumdrumFile& infile);
		int         getOriginalVoiceCount(HumdrumFile& infile);
		void        printRawAnalysis   (HumdrumFile& infile, vector<double>& raw);
		void        printAccumulatedScores(HumdrumFile& infile, vector<double>& score);
		void        printAttacks(HumdrumFile& infile, vector<int>& attacks);

	private:
		std::vector<std::string> m_homophonic;
		std::vector<int> m_notecount;
		std::vector<int> m_attacks;
		std::vector<std::vector<HPNote>> m_notes;
		double m_threshold = 4.0;
		double m_score = 1.0;
		double m_intermediate_score = 0.5;
		int m_voice_count = 0;
};

// END_MERGE

} // end namespace hum

#endif /* _TOOL_HOMOPHONIC_H */



