//
// Programmer:    Kiana Hu
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Feb 28 11:14:20 PST 2022
// Last Modified: Sat Jul  2 16:43:26 PDT 2022
// Filename:      tool-cmr.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-cmr.h
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Analyze conspicuous melodic repetitions.
//

#ifndef _TOOL_CMR_H
#define _TOOL_CMR_H

#include "HumTool.h"
#include "HumdrumFile.h"

namespace hum {

// START_MERGE

class cmr_group_info;


//////////////////////////////
//
// cmr_note_info -- Storage for a single CMR note.
//

class cmr_note_info {

	public:
		         cmr_note_info    (void);
		void     clear            (void);
		int      getMeasureBegin  (void);
		int      getMeasureEnd    (void);
		void     setMeasureBegin  (int measure);
		void     setMeasureEnd    (int measure);
		HumNum   getStartTime     (void);
		HumNum   getEndTime       (void);
		int      getMidiPitch     (void);
		string   getPitch         (void);
		HTp      getToken         (void);
		double   getNoteStrength  (void);
		bool     hasSyncopation   (void);
		bool     hasLeapBefore    (void);
		void     markNote         (const std::string& marker);
		std::ostream& printNote   (std::ostream& output = std::cout, const std::string& marker = "");

		static double getMetricLevel(HTp token);
		static bool   isSyncopated(HTp token);
		static bool   isLeapBefore(HTp token);

		static double m_syncopationWeight;
		static double m_leapWeight;

	private:
		std::vector<HTp> m_tokens;    // List of tokens for the notes (first entry is note attack);

		// location information:
		int   m_measureBegin;    // starting measure of note
		int   m_measureEnd;      // ending measure of tied note group

		// analysis information:
		int   m_hasSyncopation;  // is the note syncopated
		int   m_hasLeapBefore;   // is there a melodic leap before note

	friend class cmr_group_info;

};



//////////////////////////////
//
// cmr_group_info -- Storage for a CMR note group.
//


class cmr_group_info {
	public:
		        cmr_group_info     (void);
		void    clear              (void);
		int     getIndex           (void);
		int     getMeasureBegin    (void);
		int     getMeasureEnd      (void);
		int     getMidiPitch       (void);
		HTp     getNote            (int index);
		HTp     getFirstToken      (void);
		int     getNoteCount       (void);
		int     getTrack           (void);
		int     getStartFieldNumber(void);
		int     getStartLineNumber (void);
		void    addNote            (std::vector<HTp>& tiednotes, std::vector<int>& barnums);
		void    markNotes          (const std::string& marker);
		void    setSerial          (int serial);
		int     getSerial          (void);
		int     getDirection       (void);
		void    setDirectionUp     (void);
		void    setDirectionDown   (void);
		void    makeInvalid        (void);
		bool    isValid            (void);
		string  getPitch           (void);
		HumNum  getEndTime         (void);
		HumNum  getGroupDuration   (void);
		HumNum  getStartTime       (void);
		double  getGroupStrength   (void);
		bool    mergeGroup         (cmr_group_info& group);
		std::ostream& printNotes   (std::ostream& output = std::cout, const std::string& marker = "");

	private:
		int   m_serial;                     // used to keep track of mergers
		int   m_direction;                  // +1 = positive peak, -1 = negative peak
		std::vector<cmr_note_info> m_notes; // note info for each note in group.
};


///////////////////////////////////////////////////////////////////////////

class Tool_cmr : public HumTool {
	public:
		                 Tool_cmr                (void);
		                ~Tool_cmr                () {};

		bool             run                     (HumdrumFileSet& infiles);
		bool             run                     (HumdrumFile& infile);
		bool             run                     (const std::string& indata, std::ostream& out);
		bool             run                     (HumdrumFile& infile, std::ostream& out);
		void             finally                 (void);

	protected:
		void             processFile             (HumdrumFile& infile);
		void             initialize              (void);
		void             processFile             (HumdrumFile& infile, Options& options);
		void             processSpine            (HTp startok);
		void             processSpineFlipped     (HTp startok);
		void             identifyLocalPeaks      (std::vector<bool>& cmrnotes,
		                                          std::vector<int>& notelist);
		void             getDurations            (std::vector<double>& durations,
		                                          std::vector<std::vector<HTp>>& notelist);
		void             getBeat                 (std::vector<bool>& metpos,
		                                          std::vector<std::vector<HTp>>& notelist);
		bool             isMelodicallyAccented   (int index);
		bool             hasLeapBefore           (HTp token);
		bool             isSyncopated            (HTp token);
		void             getLocalPeakNotes       (std::vector<std::vector<HTp>>& newnotelist,
		                                          std::vector<std::vector<HTp>>& oldnotelist,
		                                          std::vector<bool>& cmrnotes);
		void             identifyPeakSequence    (std::vector<bool>& globalcmrnotes,
		                                          std::vector<int>& cmrmidinums,
		                                          std::vector<std::vector<HTp>>& notes);
		void             getMidiNumbers          (std::vector<int>& midinotes, std::vector<std::vector<HTp>>& notelist);
		void             getMetlev               (std::vector<double>& metlevs, std::vector<std::vector<HTp>>& notelist);
		void             getSyncopation          (std::vector<bool>& syncopation, std::vector<std::vector<HTp>>& notelist);
		void             getLeapBefore           (std::vector<bool>& leap, std::vector<int>& midinums);
		void             getNoteList             (std::vector<std::vector<HTp>>& notelist, HTp starting);
		void             printData               (std::vector<std::vector<HTp>>& notelist,
		                                          std::vector<int>& midinums,
		                                          std::vector<bool>& cmrnotes);
		void             markNotesInScore        (void);
		void             mergeOverlappingPeaks   (void);
		bool             checkGroupPairForMerger (cmr_group_info& index1, cmr_group_info& index2);
		int              countNotesInScore       (HumdrumFile& infile);
		void             flipMidiNumbers         (std::vector<int>& midinums);
		void             markNotes               (std::vector<std::vector<HTp>>& noteslist, std::vector<bool> cmrnotesQ, const std::string& marker);
		void             prepareHtmlReport       (void);
		void             printAnalysisData       (void);
		int              getGroupCount           (void);
		int              getGroupNoteCount       (void);
		int 						 getStrengthScore        (void);
		void             printStatistics         (HumdrumFile& infile);
		string           getComposer             (HumdrumFile& infile);
		void             printSummaryStatistics  (HumdrumFile& infile);
		void             storeVegaData           (HumdrumFile& infile);
		void             printVegaPlot           (void);
		void             printHtmlPlot           (void);
		void             printGroupStatistics    (HumdrumFile& infile);
		void             getPartNames            (std::vector<std::string>& partNames, HumdrumFile& infile);
		void             checkForCmr             (int index, int direction);
		bool             hasHigher               (int pitch, int tolerance,
		                                          std::vector<int>& midinums, 
		                                          std::vector<std::vector<HTp>>& notelist,
		                                          int index1, int index2);
		bool             hasGroupUp              (void);
		bool             hasGroupDown            (void);
		void             getVocalRange           (std::vector<std::string>& minpitch,
		                                          std::vector<std::string>& maxpitch,
		                                          std::vector<std::vector<HTp>>& notelist);
		std::string      getPitch                (HTp token);
		void             addGroupNumbersToScore  (HumdrumFile& infile);
		void             addGroupNumberToScore   (HumdrumFile& infile, HTp note, int number, int dir);
		void             adjustGroupSerials      (void);
		std::string      getLocalLabelToken      (int number, int dir);
		bool             isOnStrongBeat          (HTp token);

	private:
		// Command-line options:
		bool        m_rawQ        = false;       // don't print score (only analysis)
		bool        m_peaksQ      = false;       // analyze only positive cmrs (peaks)
		bool        m_npeaksQ     = false;       // analyze only negative cmrs (troughs)
		bool        m_naccentedQ  = false;       // analyze cmrs without melodic accentation
		bool        m_infoQ       = false;       // used with -i option: display info only
		bool        m_localQ      = false;       // used with -l option: mark all local peaks
		bool        m_localOnlyQ  = false;       // used with -L option: only mark local peaks, then exit before CMR analysis.
		bool        m_summaryQ    = false;       // used with -S option: summary statistics of multiple files
		bool        m_vegaQ       = false;       // used with -v option: output Vega-lite plot directly
		bool        m_htmlQ       = false;       // used with -V option: output Vega-lite plot in HTML file
		bool        m_vegaCountQ  = false;       // used with -w option: output Vega-lite plot for CMR count
		bool        m_vegaStrengthQ  = false;    // used with -W option: output Vega-lite plot with strength scores
		bool        m_notelistQ   = false;       // used with --notelist option
		bool        m_debugQ      = false;       // used with --debug option
		bool        m_numberQ     = false;       // used with -N option
		double      m_smallRest   = 4.0;         // Ignore rests that are 1 whole note or less
		double      m_cmrDur      = 24.0;        // 6 whole notes maximum between m_cmrNum local maximums
		double      m_cmrNum      = 3;           // number of local maximums in a row needed to mark in score
		int         m_noteCount   = 0;           // total number of notes in the score
		int         m_local_count = 0;           // used for coloring local peaks
		std::string m_colorUp     = "red";       // color to mark peak cmr notes
		std::string m_markerUp    = "+";         // marker to label peak cmr notes in score
		std::string m_colorDown   = "orange";    // color to mark antipeak cmr notes
		std::string m_markerDown  = "@";         // marker to label antipeak cmr notes in score
		std::string m_local_color = "limegreen"; // color to mark local peaks
		std::string m_local_marker = "N";        // marker for local peak notes
		std::string m_leap_color  = "purple";    // color to mark leap notes before peaks
		std::string m_leap_marker = "k";         // marker for leap notes

		// Negative peak markers:
		std::string m_local_color_n = "green";   // color to mark local peaks
		std::string m_local_marker_n = "K";      // marker for local peak notes
		int         m_local_count_n = 0;         // used for coloring local peaks


		// Analysis variables:
		std::vector<std::vector<HTp>> m_notelist; // **kern tokens (each entry is a tied group)
		std::vector<int>    m_barNum;            // starting bar number of lines in input score.

		// m_noteGroups == Storage for analized CMRs.
		std::vector<cmr_group_info> m_noteGroups;

		// m_partNames == Names of the parts (or prefferably abbreviations)
		std::vector<std::string> m_partNames;

		// m_track == Current track being processed.
		int m_track = 0;

		// m_showMergedQ == Show merged groups in output list.
		bool m_showMergedQ = false;

		// m_minPitch == minimum pitch indexed by track (scientific notation);
		std::vector<std::string> m_minPitch;

		// m_maxPitch == minimum pitch indexed by track (scientific notation);
		std::vector<std::string> m_maxPitch;

		// m_durUnit == duration unit for displaying durations in analysis table.
		std::string m_durUnit = "w";

		// m_halfQ == report durations in half note (minims).
		bool m_halfQ = false;

		// variables for doing CMR analysis (reset for each part)
		std::vector<int>         m_midinums;      // MIDI note for first entry for teach tied group
		std::vector<bool>        m_localpeaks;    // True if higher (or lower for negative search) than adjacent notes.
		std::vector<double>      m_metlevs;       // True if higher (or lower for negative search) than adjacent notes.
		std::vector<bool>        m_syncopation;   // True if note is syncopated.
		std::vector<bool>        m_leapbefore;    // True if note has a leap before it.

		// Summary statistics variables:
		std::vector<int>         m_cmrCount;       // number of CMRs in each input file
		std::vector<int>         m_cmrNoteCount;   // number of CMR notes in each input file
		std::vector<int>         m_scoreNoteCount; // number of note in each input file

		std::stringstream        m_vegaData;       // stores all data for Vega plot from each processFile
};


// END_MERGE

} // end namespace hum

#endif /* _TOOL_CMR */
