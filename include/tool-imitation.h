//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Jun 17 20:18:23 CEST 2017
// Last Modified: Sat Jun 17 22:40:59 CEST 2017
// Filename:      tool-imitation.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-imitation.h
// Syntax:        C++11
// vim:           ts=3 noexpandtab
//
// Description:   Interface for imitation tool.
//

#ifndef _TOOL_DISSONANT_H
#define _TOOL_DISSONANT_H

#include "HumTool.h"
#include "HumdrumFile.h"
#include "NoteGrid.h"

namespace hum {

// START_MERGE

class Tool_imitation : public HumTool {
	public:
		         Tool_imitation    (void);
		        ~Tool_imitation    () {};

		bool     run               (HumdrumFile& infile);
		bool     run               (const string& indata, ostream& out);
		bool     run               (HumdrumFile& infile, ostream& out);

	protected:
		void    doAnalysis         (vector<vector<string>>& results, NoteGrid& grid,
		                            vector<vector<NoteCell*>>& attacks,
		                            vector<vector<double>>& intervals,
		                            HumdrumFile& infile, bool debug);
		void    analyzeImmitation  (vector<vector<string>>& results,
		                            vector<vector<NoteCell*>>& attacks,
		                            vector<vector<double>>& intervals,
		                            int v1, int v2);
		void    getIntervals       (vector<double>& intervals,
		                            vector<NoteCell*>& attacks);
		int     compareSequences   (vector<NoteCell*>& attack1, vector<double>& seq1,
		                            int i1, vector<NoteCell*>& attack2,
		                            vector<double>& seq2, int i2);

	private:
	 	vector<HTp> m_kernspines;
		int m_threshold;
		bool m_duration;
		bool m_rest;
		bool m_rest2;
		bool m_mark;
		char m_marker = '@';
		static int Enumerator;
};

// END_MERGE

} // end namespace hum

#endif /* _TOOL_DISSONANT_H */



