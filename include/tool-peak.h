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
		void 													processSpineFlipped(HTp startok);
		void                          identifyLocalPeaks (std::vector<bool>& peaknotes,
		                                                  std::vector<int>& notelist);
		void                          getDurations       (vector<double>& durations,
		                                                  vector<vector<HTp>>& notelist);
		void                          getBeat            (vector<bool>& metpos,
		                                                  vector<vector<HTp>>& notelist);
		int                           getMetricLevel     (HTp token);
		bool                          isSyncopated       (HTp token);
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
		void                          mergeOverlappingPeaks(void);
		bool                          checkGroupPairForMerger(int index1, int index2);
    int                           countNotesInScore   (HumdrumFile& infile);
		std::vector<int> 							flipMidiNumbers     (vector<int>& midinums);

	private:
		bool m_rawQ             = false;         // don't print score (only analysis)
		bool m_peakQ            = false;         // analyze only peaks
		bool m_npeakQ           = false;         // analyze only negative peaks (troughs)
		std::string m_marker    = "@";           // marker to label peak notes in score
		std::string m_color     = "red";         // color to mark peak notes
		double      m_smallRest = 4.0;           // Ignore rests that are 1 whole note or less
		double      m_peakDur   = 24.0;          // 6 whole notes maximum between m_peakNum local maximums
		double      m_peakNum   = 3;             // number of local maximums in a row needed to mark in score

		bool        m_infoQ     = false;         // used with -i option
		int         m_count     = 0;             // number of peak sequences in score
		int         m_noteCount = 0;             // total number of notes in the score

		std::vector<int>    m_barNum;            // storage for identify start/end measures of peak groups

		std::vector<int>    m_peakMeasureBegin;  // starting measure of peak group
		std::vector<int>    m_peakMeasureEnd;    // starting measure of peak group
		std::vector<HumNum> m_peakDuration;      // between first peak note and last peak note.
		std::vector<std::vector<HTp>> m_peakPitch; // pitches of the peak sequence (excluding tied notes)
		std::vector<int>    m_peakPeakCount;     // how many notes in a peak sequence

		// Merging variables for peak groups:
		std::vector<int>    m_peakIndex;         // used to keep track of mergers
		std::vector<int>    m_peakTrack;         // used to keep track of mergers
		std::vector<HumNum> m_startTime;         // starting time of first note in group
		std::vector<HumNum> m_endTime;           // ending time of last note in group

};

// END_MERGE

} // end namespace hum

#endif /* _TOOL_PEAK_H */
