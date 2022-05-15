//
// Programmer:    Kiana Hu
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Feb 28 11:14:20 PST 2022
// Last Modified: Sat May 14 12:47:09 PDT 2022
// Filename:      tool-cmr.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-cmr.h
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Analyze conspicuous melodic repetitions.
//

#ifndef _TOOL_PEAK_H
#define _TOOL_PEAK_H

#include "HumTool.h"
#include "HumdrumFile.h"

namespace hum {

// START_MERGE

class Tool_cmr : public HumTool {
	public:
		                 Tool_cmr                (void);
		                ~Tool_cmr                () {};

		bool             run                     (HumdrumFileSet& infiles);
		bool             run                     (HumdrumFile& infile);
		bool             run                     (const string& indata, ostream& out);
		bool             run                     (HumdrumFile& infile, ostream& out);

	protected:
		void             processFile             (HumdrumFile& infile);
		void             initialize              (void);
		void             processFile             (HumdrumFile& infile, Options& options);
		void             processSpine            (HTp startok);
		void             processSpineFlipped     (HTp startok);
		void             identifyLocalPeaks      (std::vector<bool>& cmrnotes,
		                                          std::vector<int>& notelist);
		void             getDurations            (vector<double>& durations,
		                                          vector<vector<HTp>>& notelist);
		void             getBeat                 (vector<bool>& metpos,
		                                          vector<vector<HTp>>& notelist);
		int              getMetricLevel          (HTp token);
		bool             isMelodicallyAccented   (HTp token);
		bool             hasLeapBefore           (HTp token);
		bool             isSyncopated            (HTp token);
		void             getLocalPeakNotes       (vector<vector<HTp>>& newnotelist,
		                                          vector<vector<HTp>>& oldnotelist,
		                                          vector<bool>& cmrnotes);

		void             identifyPeakSequence    (vector<bool>& globalcmrnotes,
		                                          vector<int>& cmrmidinums,
		                                          vector<vector<HTp>>& notes);
		std::vector<int> getMidiNumbers          (std::vector<std::vector<HTp>>& notelist);
		std::vector<std::vector<HTp>> getNoteList(HTp starting);
		void             printData               (std::vector<std::vector<HTp>>& notelist,
		                                          std::vector<int>& midinums,
		                                          std::vector<bool>& cmrnotes);
		void             markNotesInScore        (vector<vector<HTp>>& cmrnotelist,
		                                          vector<bool>& iscmr);
		void             mergeOverlappingPeaks   (void);
		bool             checkGroupPairForMerger (int index1, int index2);
		int              countNotesInScore       (HumdrumFile& infile);
		std::vector<int> flipMidiNumbers         (vector<int>& midinums);
		void             markNotes               (vector<vector<HTp>>& notelist, vector<bool>& cmrnotesQ,
		                                          const string& marker);
		void             postProcessAnalysis     (HumdrumFile& infile);

	private:
		bool    m_rawQ        = false;           // don't print score (only analysis)
		bool    m_cmrQ        = false;           // analyze only cmrs
		bool    m_ncmrQ       = false;           // analyze only negative cmrs (troughs)
		bool    m_naccentedQ  = false;           // analyze cmrs without melodic accentation
		bool    m_infoQ       = false;           // used with -i option: display info only
		bool    m_localQ      = false;           // used with -l option: mark all local peaks
		bool    m_localOnlyQ  = false;           // used with -L option: only mark local peaks, then exit before CMR analysis.

		double  m_smallRest   = 4.0;             // Ignore rests that are 1 whole note or less
		double  m_cmrDur      = 24.0;            // 6 whole notes maximum between m_cmrNum local maximums
		double  m_cmrNum      = 3;               // number of local maximums in a row needed to mark in score

		int     m_count       = 0;               // number of cmr sequences in score
		int     m_noteCount   = 0;               // total number of notes in the score

		std::string m_color     = "red";         // color to mark cmr notes
		std::string m_marker    = "@";           // marker to label cmr notes in score

		std::string m_local_color = "limegreen"; // color to mark local peaks
		std::string m_local_marker = "N";        // marker for local peak notes
		int         m_local_count = 0;           // used for coloring local peaks

		// negative peak markers:
		std::string m_local_color_n = "green";   // color to mark local peaks
		std::string m_local_marker_n = "K";      // marker for local peak notes
		int         m_local_count_n = 0;         // used for coloring local peaks

		std::string m_leap_color  = "purple";    // color to mark leap notes before peaks
		std::string m_leap_marker = "k";         // marker for leap notes

		std::vector<int>    m_barNum;            // storage for identify start/end measures of cmr groups

		std::vector<int>    m_cmrMeasureBegin;   // starting measure of cmr group
		std::vector<int>    m_cmrMeasureEnd;     // starting measure of cmr group
		std::vector<HumNum> m_cmrDuration;       // between first cmr note and last cmr note.
		std::vector<std::vector<HTp>> m_cmrPitch;// pitches of the cmr sequence (excluding tied notes)
		std::vector<int>    m_cmrPeakCount;      // how many notes in a cmr sequence

		// Merging variables for cmr groups:
		std::vector<int>    m_cmrIndex;          // used to keep track of mergers
		std::vector<int>    m_cmrTrack;          // used to keep track of mergers
		std::vector<HumNum> m_startTime;         // starting time of first note in group
		std::vector<HumNum> m_endTime;           // ending time of last note in group

};

// END_MERGE

} // end namespace hum

#endif /* _TOOL_PEAK_H */
