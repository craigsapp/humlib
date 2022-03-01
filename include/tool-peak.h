//
// Programmer:    Kiana Hu
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Feb 28 11:14:20 PST 2022
// Last Modified: Mon Feb 28 11:14:24 PST 2022
// Filename:      tool-peak.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-peak.h
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Analyze melodic high-points.
//

#ifndef _TOOL_PEAK_H
#define _TOOL_PEAK_H

#include "HumTool.h"
#include "HumdrumFile.h"

namespace hum {

// START_MERGE

class Tool_peak : public HumTool {
	public:
		                              Tool_peak          (void);
		                             ~Tool_peak          () {};

		bool                          run                (HumdrumFileSet& infiles);
		bool                          run                (HumdrumFile& infile);
		bool                          run                (const string& indata, ostream& out);
		bool                          run                (HumdrumFile& infile, ostream& out);

	protected:
		void                          processFile        (HumdrumFile& infile);
		void                          initialize         (void);
		void                          processFile        (HumdrumFile& infile, Options& options);
		void                          processSpine       (HTp startok);
		void                          identifyLocalPeaks (std::vector<bool>& peaknotes,
		                                                  std::vector<int>& notelist);
		void                          getLocalPeakNotes  (vector<vector<HTp>>& newnotelist,
		                                                  vector<vector<HTp>>& oldnotelist,
		                                                  vector<bool>& peaknotes);
		void                          identifyPeakSequence(vector<bool>& globalpeaknotes,
		                                                   vector<int>& peakmidinums,
		                                                   vector<vector<HTp>>& notes);
		std::vector<int>              getMidiNumbers     (std::vector<std::vector<HTp>>& notelist);
		std::vector<std::vector<HTp>> getNoteList        (HTp starting);
		void                          printData          (std::vector<std::vector<HTp>>& notelist,
		                                                  std::vector<int>& midinums,
		                                                  std::vector<bool>& peaknotes);
		void                          markNotesInScore   (vector<vector<HTp>>& peaknotelist,
		                                                  vector<bool>& ispeak);

	private:
		bool m_rawQ             = false;
		std::string m_marker    = "@";
		std::string m_color     = "red";
		double      m_smallRest = 4.0;   // Ignore rests that are 1 whole note or less.
		double      m_peakDur   = 24.0;  // 6 whole notes maximum between m_peakNum local maximums.
		double      m_peakNum   = 3;    // Number of local maximums in a row needed to mark in score.


};

// END_MERGE

} // end namespace hum

#endif /* _TOOL_PEAK_H */



