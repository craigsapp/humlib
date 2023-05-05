//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Jul 29 16:48:22 CEST 2019
// Last Modified: Mon May  3 15:45:21 PDT 2021
// Filename:      tool-composite.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-composite.h
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Fill in kern measures that have no duration with rests.
//

#ifndef _TOOL_COMPOSITE_H
#define _TOOL_COMPOSITE_H

#include "HumTool.h"
#include "HumdrumFile.h"

namespace hum {

// START_MERGE

class Tool_composite : public HumTool {
	public:
		            Tool_composite       (void);
		           ~Tool_composite       () {};

		bool        run                  (HumdrumFileSet& infiles);
		bool        run                  (HumdrumFile& infile);
		bool        run                  (const std::string& indata, std::ostream& out);
		bool        run                  (HumdrumFile& infile, std::ostream& out);

	protected:
		void        processFile               (HumdrumFile& infile);
		void        initialize                (HumdrumFile& infile);
		void        initializeNumericAnalyses (HumdrumFile& infile);
		bool        hasGroupInterpretations   (HumdrumFile& infile);
		void        prepareOutput             (HumdrumFile& infile);
		void        analyzeFullCompositeRhythm(HumdrumFile& infile);
		void        analyzeGroupCompositeRhythms(HumdrumFile& infile);
		void        analyzeCoincidenceRhythms (HumdrumFile& infiel);
		void        assignGroups              (HumdrumFile& infile);
		void        analyzeLineGroups         (HumdrumFile& infile);
		void        analyzeLineGroup          (HumdrumFile& infile, int line,
		                                       const string& target);
		void        extractGroup              (HumdrumFile& infile, const std::string &target);
		void        getNumericGroupStates     (std::vector<int>& states, HumdrumFile& infile, const std::string& tgroup);
		int         getGroupNoteType          (HumdrumFile& infile, int line, const std::string& group);
		HumNum      getLineDuration           (HumdrumFile& infile, int index,
		                                       std::vector<bool>& isNull);
		void        backfillGroup             (std::vector<std::vector<std::string>>& curgroup,
		                                       HumdrumFile& infile, int line, int track,
		                                       int subtrack, const std::string& group);

		void        getAnalysisOutputLine     (std::ostream& output, HumdrumFile& infile,
		                                       int line);
		std::string getFullCompositeToken     (HumdrumFile& infile, int line);
		std::string getCoincidenceToken       (HumdrumFile& infile, int line);
		std::string getGroupCompositeToken    (HumdrumFile& infile, int line, int group);
		void        getGroupStates            (std::vector<std::vector<int>>& groupstates,
		                                       HumdrumFile& infile);
		void        getGroupDurations         (std::vector<std::vector<HumNum>>& groupdurs,
		                                       std::vector<std::vector<int>>& groupstates,
		                                       HumdrumFile& infile);
		void        getGroupDurations         (std::vector<HumNum>& groupdurs,
		                                       std::vector<int>& groupstates,
		                                       HumdrumFile& infile);
		void        printGroupAssignments     (HumdrumFile& infile);
		void        getGroupRhythms           (std::vector<std::vector<std::string>>& rhythms,
		                                       std::vector<std::vector<HumNum>>& groupdurs,
		                                       std::vector<std::vector<int>>& groupstates,
		                                       HumdrumFile& infile);
		void        getGroupRhythms           (std::vector<std::string>& rhythms,
		                                       std::vector<HumNum>& durs, std::vector<int>& states,
		                                       HumdrumFile& infile);
		int         typeStringToInt           (const string& value);
		void        addNumericAnalyses        (ostream& output, HumdrumFile& infile, int line,
		                                       std::vector<std::vector<double>>&  rhythmIndex);
		void        analyzeOutputVariables(HumdrumFile& infile);
		std::string getTimeSignature          (HumdrumFile& infile, int line, const std::string& group);
		std::string getMetricSymbol           (HumdrumFile& infile, int line, const std::string& group);
		std::string generateVerseLabelLine    (HumdrumFile& output, HumdrumFile& input, int line);
		std::string generateStriaLine         (HumdrumFile& output, HumdrumFile& input, int line);
		std::string getFullCompositeMarker    (int line);
		void        addStaffInfo              (HumdrumFile& output, HumdrumFile& infile);
		void        addTimeSignatureChanges   (HumdrumFile& output, HumdrumFile& infile);
		void        addMeterSignatureChanges  (HumdrumFile& output, HumdrumFile& infile);
		void        adjustBadCoincidenceRests (HumdrumFile& output, HumdrumFile& infile);
		HTp         fixBadRestRhythm          (HTp token, string& rhythm, HumNum tstop, HumNum tsbot);
		std::string generateSizeLine          (HumdrumFile& output, HumdrumFile& input, int line);
		void        convertNotesToRhythms     (HumdrumFile& infile);
		int         getEventCount             (std::vector<string>& data);
		void        fixTiedNotes              (std::vector<string>& data, HumdrumFile& infile);
		void        doOnsetAnalysisCoincidence(vector<double>& output,
		                                       vector<double>& inputA, vector<double>& inputB);
		void        checkForAutomaticGrouping (HumdrumFile& infile);

		// Numeric analysis functions:
		void        doNumericAnalyses         (HumdrumFile& infile);
		void        doOnsetAnalyses           (HumdrumFile& infile);
		void        doOnsetAnalysis           (std::vector<double>& analysis,
		                                       HumdrumFile& infile,
		                                       const string& targetGroup);

		void        doAccentAnalyses          (HumdrumFile& infile);

		void        doOrnamentAnalyses        (HumdrumFile& infile);

		void        doSlurAnalyses            (HumdrumFile& infile);

		void        doTotalAnalyses           (HumdrumFile& infile);

		// Numeric analysis support functions:
		int         countNoteOnsets           (HTp token);

		bool        needsCoincidenceMarker    (int line, bool forceQ = false);
		void        addCoincidenceMarks       (HumdrumFile& infile);

	private:
		bool        m_debugQ      = false;  // used with --debug option
		bool        m_appendQ     = false;  // append analysis data to input data spines
		bool        m_prependQ    = true;   // default position is to place output at start of line
		bool        m_extractQ    = false;  // output only comp. rhythm analyses (no input)
		bool        m_beamQ       = true;   // used with -B option
		bool        m_hasGroupsQ  = false;  // true if contains *grp:(A|B) interpretations
		std::string m_pitch       = "eR";   // pitch to display for composite rhythm
		bool        m_graceQ      = false;  // include grace notes in composite rhythm

		// Composite rhythm analysis variables:
		bool        m_fullCompositeQ  = true;  // used with -F option
		bool        m_coincidenceQ    = false; // used with -c option
		bool        m_groupsQ         = false; // used with -g option
		bool        m_upstemQ         = false; // used with -u option


		bool        m_onlyQ = false; // used with -o option
		std::string m_only;          // used with -o option

		bool        m_assignedGroups = false; // Have group labels been added to notes?

		// Storage for composite rhythm analysis spines:
		std::vector<std::string> m_fullComposite;
		std::vector<std::string> m_coincidence;
		std::vector<std::vector<std::string>> m_groups;  // Groups A and B

		// Numerical analysis variables:
		bool        m_analysisOnsetsQ    = false;    // used with -P option
		bool        m_analysisAccentsQ   = false;    // used with -A option
		bool        m_analysisOrnamentsQ = false;    // used with -O option
		bool        m_analysisSlursQ     = false;    // used with -S option
		bool        m_analysisTotalQ     = false;    // used with -T option
		std::vector<bool> m_analysisIndex;           // -PAOST booleans in array

		bool        m_analysesQ          = false;    // union of -PAOST options
		int         m_numericAnalysisSpineCount = 0; // sum of -PAOST options
		bool        m_nozerosQ           = false;    // used with -Z option

		bool        m_assignedQ          = false;    // used to keep track of group analysis initialization

		// Data storage for numerical anslysis.
		//
		// First index is the rhythm type:
		//    0 index for Coincidence rhythm
		//    1 index for full Composite rhythm
		//    2 index for Group A composite rhythym
		//    3 index for Group B composite rhythym
		//    [4 and higher: index for Group C (future?)]
		//
		// Second index is type of analysis:
		//    0 = onsets
		//    1 = accents
		//    2 = ornaments
		//    3 = slurs
		//    4 = total
		//
		// Third index is line number in original file
		//
		std::vector<std::vector<std::vector<double>>> m_analyses;

		// first two dimension indexes into m_analyses:

		const int m_ANALYSES_DIM1  = 4;
		const int m_COINCIDENCE    = 0;
		const int m_COMPOSITE_FULL = 1;
		const int m_COMPOSITE_A    = 2;
		const int m_COMPOSITE_B    = 3;

		const int m_ANALYSES_DIM2  = 5;
		const int m_ONSET          = 0;
		const int m_ACCENT         = 1;
		const int m_ORNAMENT       = 2;
		const int m_SLUR           = 3;
		const int m_TOTAL          = 4;

		// output line variables (zero means unset, and negative means add
		// before next line.
		int m_clefIndex             = 0;
		int m_striaIndex            = 0;
		int m_sizeIndex             = 0;
		int m_firstDataIndex        = 0;
		int m_instrumentNameIndex   = 0;
		int m_instrumentAbbrIndex   = 0;
		int m_timeSignatureIndex    = 0;
		int m_meterSymbolIndex      = 0;
		int m_groupAssignmentIndex  = 0;
		int m_verseLabelIndex       = 0;

		// analytic counts:
		int m_coincidenceEventCount   = -1; // number of events in coincidence
		int m_fullCompositeEventCount = -1; // number of events in full composite rhythm
		int m_groupAEventCount        = -1; // number of events in group A composite rhythm
		int m_groupBEventCount        = -1; // number of events in group B composite rhythm

		double m_scoreSize          = 100.0;
		double m_analysisSize       = 100.0;

		bool m_eventQ                = false;
		bool m_rhythmQ              = false;
		bool m_colorFullCompositeQ  = false;
		bool m_extractInputQ        = false;
		bool m_coinMarkQ            = false;
		std::string m_coinMark      = "|";
		std::string m_coinMarkColor = "limegreen";
		std::string m_AMark         = "@";
		std::string m_AMarkColor    = "crimson";
		std::string m_BMark         = "Z";
		std::string m_BMarkColor    = "dodgerblue";

};

// END_MERGE

} // end namespace hum

#endif /* _TOOL_COMPOSITE_H */



