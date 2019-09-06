//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Jun 17 20:18:23 CEST 2017
// Last Modified: Sat Jun 17 22:40:59 CEST 2017
// Filename:      tool-imitation.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-imitation.h
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Interface for imitation tool.
//

#ifndef _TOOL_IMITATION_H
#define _TOOL_IMITATION_H

#include "HumTool.h"
#include "HumdrumFile.h"
#include "NoteGrid.h"

namespace hum {

// START_MERGE

class Tool_imitation : public HumTool {
	public:
		         Tool_imitation    (void);
		        ~Tool_imitation    () {};

		bool     run               (HumdrumFileSet& infiles);
		bool     run               (HumdrumFile& infile);
		bool     run               (const string& indata, ostream& out);
		bool     run               (HumdrumFile& infile, ostream& out);

	protected:
		void    doAnalysis         (vector<vector<string>>& results, NoteGrid& grid,
		                            vector<vector<NoteCell*>>& attacks,
		                            vector<vector<double>>& intervals,
		                            HumdrumFile& infile, bool debug);
		void    analyzeImitation  (vector<vector<string>>& results,
		                            vector<vector<NoteCell*>>& attacks,
		                            vector<vector<double>>& intervals,
		                            int v1, int v2);
		void    getIntervals       (vector<double>& intervals,
		                            vector<NoteCell*>& attacks);
		int     compareSequences   (vector<NoteCell*>& attack1, vector<double>& seq1,
		                            int i1, vector<NoteCell*>& attack2,
		                            vector<double>& seq2, int i2);
		int     checkForIntervalSequence(vector<int>& m_intervals,
		                            vector<double>& v1i, int starti, int count);
		void    markedTiedNotes    (vector<HTp>& tokens);

	private:
	 	vector<HTp> m_kernspines;
		int m_threshold;
		bool m_duration;
		bool m_rest;
		bool m_rest2;
		double m_maxdistance;
		bool m_maxdistanceQ;
		vector<int> m_intervals;
		bool m_mark;
		char m_marker = '@';
		bool m_single = false;
		static int Enumerator;
		bool m_first = false;
		bool m_nozero = false;
		bool m_onlyzero = false;
		bool m_measure = false;
		bool m_beat    = false;
		bool m_length  = false;

		bool m_noInfo = false;

		bool m_noN    = false;
		bool m_noC    = false;
		bool m_noD    = false;
		bool m_noI    = false;

		bool m_noNN   = false;
		bool m_noCC   = false;
		bool m_noDD   = false;
		bool m_noII   = false;

		bool m_addsearches  = false;
		bool m_inversion  = false;
		bool m_retrograde = false;

		vector<int> m_barlines;
};

// END_MERGE

} // end namespace hum

#endif /* _TOOL_IMITATION_H */



