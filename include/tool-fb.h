//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Wed Mar  9 21:50:25 PST 2022
// Last Modified: Wed Mar  9 21:50:28 PST 2022
// Filename:      tool-fb.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-fb.h
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Extract figured bass numbers from musical content.
// Reference:     https://github.com/WolfgangDrescher/humdrum-figured-bass-filter-demo
//

#ifndef _TOOL_FB_H
#define _TOOL_FB_H

#include "HumTool.h"
#include "HumdrumFile.h"

namespace hum {

// START_MERGE

class Tool_fb : public HumTool {
	public:
		         Tool_fb           (void);
		        ~Tool_fb           () {};

		bool     run               (HumdrumFileSet& infiles);
		bool     run               (HumdrumFile& infile);
		bool     run               (const string& indata, ostream& out);
		bool     run               (HumdrumFile& infile, ostream& out);

	protected:
		void     processFile       (HumdrumFile& infile);
		void     initialize        (void);
		void     processLine       (HumdrumFile& infile, int index);
		void     setupScoreData    (HumdrumFile& infile);
		void     getAnalyses       (HumdrumFile& infile);
		void     getHarmonicIntervals(HumdrumFile& infile);
		void     calculateIntervals(vector<int>& intervals, vector<HTp>& tokens, int bassIndex);
		void     printOutput       (HumdrumFile& infile);
		void     printLineStyle3   (HumdrumFile& infile, int line);
		std::string getAnalysisTokenStyle3(HumdrumFile& infile, int line, int field);

	private:
		std::vector<HTp>              m_kernspines;
		std::vector<int>              m_kerntracks;
		std::vector<int>              m_track2index;
		std::vector<std::vector<int>> m_keyaccid;
		std::vector<std::vector<int>> m_intervals;
		const int m_rest = -1000;
		int       m_reference = 0; // currently fixed to bass
		int       m_debugQ = false;

};

// END_MERGE

} // end namespace hum

#endif /* _TOOL_FB_H */



