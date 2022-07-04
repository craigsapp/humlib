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
		double   getNoteStrength  (void);
		bool     hasSyncopation   (void);
		bool     hasLeapBefore    (void);
		void     markNote         (const std::string& marker);
		std::ostream& printNote   (std::ostream& output = std::cout);

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
		int     getNoteCount       (void);
		int     getTrack           (void);
		int     getStartFieldNumber(void);
		int     getStartLineNumber (void);
		void    addNote            (std::vector<HTp>& tiednotes, std::vector<int>& barnums);
		bool    isValid            (void);
		void    markNotes          (const std::string& marker);
		void    setSerial          (int serial);
		int     getSerial          (void);
		string   getPitch          (void);
		HumNum  getEndTime         (void);
		HumNum  getGroupDuration   (void);
		HumNum  getStartTime       (void);
		double  getGroupStrength   (void);
		bool    mergeGroup         (cmr_group_info& group);
		std::ostream& printNotes   (std::ostream& output = std::cout);

	private:
		int   m_serial;                     // used to keep track of mergers
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
		bool             isMelodicallyAccented   (HTp token);
		bool             hasLeapBefore           (HTp token);
		bool             isSyncopated            (HTp token);
		void             getLocalPeakNotes       (std::vector<std::vector<HTp>>& newnotelist,
		                                          std::vector<std::vector<HTp>>& oldnotelist,
		                                          std::vector<bool>& cmrnotes);

		void             identifyPeakSequence    (std::vector<bool>& globalcmrnotes,
		                                          std::vector<int>& cmrmidinums,
		                                          std::vector<std::vector<HTp>>& notes);
		void             getMidiNumbers          (std::vector<int>& midinotes, std::vector<std::vector<HTp>>& notelist);
		void             getNoteList             (std::vector<std::vector<HTp>>& notelist, HTp starting);
		void             printData               (std::vector<std::vector<HTp>>& notelist,
		                                          std::vector<int>& midinums,
		                                          std::vector<bool>& cmrnotes);
		void             markNotesInScore        (void);
		void             mergeOverlappingPeaks   (void);
		bool             checkGroupPairForMerger (cmr_group_info& index1, cmr_group_info& index2);
		int              countNotesInScore       (HumdrumFile& infile);
		std::vector<int> flipMidiNumbers         (std::vector<int>& midinums);
		void             markNotes               (std::vector<std::vector<HTp>>& noteslist, std::vector<bool> cmrnotesQ, const std::string& marker);
		void             postProcessAnalysis     (HumdrumFile& infile);
		void             prepareHtmlReport       (void);
		void             printNoteList           (std::vector<std::vector<HTp>>& notelist);
		int              getGroupCount           (void);
		int              getGroupNoteCount       (void);
		void             printStatistics         (HumdrumFile& infile);
		void             printGroupStatistics    (HumdrumFile& infile);
		void             getPartNames            (std::vector<std::string>& partNames, HumdrumFile& infile);

	private:
		// Command-line options:
		bool        m_rawQ        = false;       // don't print score (only analysis)
		bool        m_cmrQ        = false;       // analyze only cmrs
		bool        m_ncmrQ       = false;       // analyze only negative cmrs (troughs)
		bool        m_naccentedQ  = false;       // analyze cmrs without melodic accentation
		bool        m_infoQ       = false;       // used with -i option: display info only
		bool        m_localQ      = false;       // used with -l option: mark all local peaks
		bool        m_localOnlyQ  = false;       // used with -L option: only mark local peaks, then exit before CMR analysis.
		bool        m_notelistQ   = false;       // uwsed with --notelist option
		double      m_smallRest   = 4.0;         // Ignore rests that are 1 whole note or less
		double      m_cmrDur      = 24.0;        // 6 whole notes maximum between m_cmrNum local maximums
		double      m_cmrNum      = 3;           // number of local maximums in a row needed to mark in score
		int         m_noteCount   = 0;           // total number of notes in the score
		int         m_local_count = 0;           // used for coloring local peaks
		std::string m_color       = "red";       // color to mark cmr notes
		std::string m_marker      = "@";         // marker to label cmr notes in score
		std::string m_local_color = "limegreen"; // color to mark local peaks
		std::string m_local_marker = "N";        // marker for local peak notes
		std::string m_leap_color  = "purple";    // color to mark leap notes before peaks
		std::string m_leap_marker = "k";         // marker for leap notes

		// Negative peak markers:
		std::string m_local_color_n = "green";   // color to mark local peaks
		std::string m_local_marker_n = "K";      // marker for local peak notes
		int         m_local_count_n = 0;         // used for coloring local peaks


		// Analysis variables:
		std::vector<std::vector<HTp>> notelist;  // list of all notes in a part before analysis.
		std::vector<int>    m_barNum;            // starting bar number of lines in input score.

		// m_noteGroups == storage for analized CMRs.
		std::vector<cmr_group_info> m_noteGroups;

};


// END_MERGE

} // end namespace hum

#endif /* _TOOL_CMR */
