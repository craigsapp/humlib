//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Jul 29 16:48:22 CEST 2019
// Last Modified: Mon May  3 15:45:21 PDT 2021
// Filename:      tool-compositeold.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-compositeold.h
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Fill in kern measures that have no duration with rests.
//

#ifndef _TOOL_COMPOSITEOLD_H
#define _TOOL_COMPOSITEOLD_H

#include "HumTool.h"
#include "HumdrumFile.h"

#include <ostream>
#include <string>
#include <vector>

namespace hum {

// START_MERGE

class Tool_compositeold : public HumTool {
	public:
		            Tool_compositeold        (void);
		           ~Tool_compositeold        () {};

		bool        run                  (HumdrumFileSet& infiles);
		bool        run                  (HumdrumFile& infile);
		bool        run                  (const std::string& indata, std::ostream& out);
		bool        run                  (HumdrumFile& infile, std::ostream& out);

	protected:
		void        processFile          (HumdrumFile& infile);
		void        prepareMultipleGroups(HumdrumFile& infile);
		void        prepareSingleGroup   (HumdrumFile& infile);
		void        initialize           (void);
		void        initializeAnalysisArrays(HumdrumFile& infile);
		int         typeStringToInt      (const std::string& value);
		HumNum      getLineDuration      (HumdrumFile& infile, int index, std::vector<bool>& isNull);
		void        getGroupStates       (std::vector<std::vector<int>>& groupstates, HumdrumFile& infile);
		void        assignGroups         (HumdrumFile& infile);
		void        analyzeLineGroups    (HumdrumFile& infile);
		void        analyzeLineGroup     (HumdrumFile& infile, int line, const std::string& target);
		void        printGroupAssignments(HumdrumFile& infile);
		int         getGroupNoteType     (HumdrumFile& infile, int line, const std::string& group);
		void        getGroupDurations    (std::vector<std::vector<HumNum>>& groupdurs,
		                                  std::vector<std::vector<int>>& groupstates, HumdrumFile& infile);
		void        getGroupDurations    (std::vector<HumNum>& groupdurs, std::vector<int>& groupstates,
		                                  HumdrumFile& infile);
		void        getGroupRhythms      (std::vector<std::vector<std::string>>& rhythms,
		                                  std::vector<std::vector<HumNum>>& groupdurs,
		                                  std::vector<std::vector<int>>& groupstates,
		                                  HumdrumFile& infile);
		void        getGroupRhythms      (std::vector<std::string>& rhythms,
		                                  std::vector<HumNum>& durs,
		                                  std::vector<int>& states, HumdrumFile& infile);
		bool        hasGroupInterpretations(HumdrumFile& infile);
		void        checkForTremoloReduction(HumdrumFile& infile, int line, int field);
		void        reduceTremolos       (HumdrumFile& infile);
		bool        areAllEqual          (std::vector<HTp>& notes);
		void        getBeamedNotes       (std::vector<HTp>& notes, HTp starting);
		void        getPitches           (std::vector<int>& pitches, HTp token);
		void        addLabelsAndStria    (HumdrumFile& infile);
		void        addLabels            (HTp sstart, int labelIndex, const std::string& label,
		                                  int abbrIndex, const std::string& abbr);
		void        addStria             (HumdrumFile& infile, HTp spinestart);
		void        addVerseLabels       (HumdrumFile& infile, HTp spinestart);
		void        addVerseLabels2      (HumdrumFile& infile, HTp spinestart);
		bool        pitchesEqual         (std::vector<int>& pitches1, std::vector<int>& pitches2);
		void        mergeTremoloGroup    (std::vector<HTp>& notes, std::vector<int> groups, int group);
		bool        onlyAuxTremoloNotes  (HumdrumFile& infile, int line);
		void        removeAuxTremolosFromCompositeRhythm(HumdrumFile& infile);
		void        markTogether         (HumdrumFile& infile, int direction);
		void        markCoincidences     (HumdrumFile& infile, int direction);
		void        markCoincidencesMusic(HumdrumFile& infile);
		bool        isOnsetInBothGroups (HumdrumFile& infile, int line);
		void        extractNestingData   (HumdrumFile& infile);
		void        analyzeNestingDataGroups(HumdrumFile& infile, int direction);
		void        analyzeNestingDataAll(HumdrumFile& infile, int direction);
		void        getNestData          (HTp spine, int& total, int& coincide);
		void        getCoincidenceRhythms(std::vector<std::string>& rhythms, std::vector<int>& coincidences,
		                                  HumdrumFile& infile);
		void        fillInCoincidenceRhythm(std::vector<int>& coincidences,
		                                  HumdrumFile& infile, int direction);
		void        processCoincidenceInterpretation(HumdrumFile& infile, HTp token);
		bool        hasPipeRdf           (HumdrumFile& infile);
		void        extractGroup         (HumdrumFile& infile, const std::string &target);
		void        backfillGroup        (std::vector<std::vector<std::string>>& curgroup, HumdrumFile& infile,
		                                  int line, int track, int subtrack, const std::string& group);

		void        analyzeComposite      (HumdrumFile& infile);
		void        analyzeCompositeOnsets(HumdrumFile& infile, std::vector<HTp>& groups, std::vector<bool>& tracks);
		void        analyzeCompositeAccents(HumdrumFile& infile, std::vector<HTp>& groups, std::vector<bool>& tracks);
		void        analyzeCompositeOrnaments(HumdrumFile& infile, std::vector<HTp>& groups, std::vector<bool>& tracks);
		void        analyzeCompositeSlurs(HumdrumFile& infile, std::vector<HTp>& groups, std::vector<bool>& tracks);
		void        analyzeCompositeTotal(HumdrumFile& infile, std::vector<HTp>& groups, std::vector<bool>& tracks);

		void        getCompositeSpineStarts(std::vector<HTp>& groups, HumdrumFile& infile);
		std::vector<int> getExpansionList(std::vector<bool>& tracks, int maxtrack, int count);
		std::string makeExpansionString(std::vector<int>& tracks);
		void        doCoincidenceAnalysis(HumdrumFile& outfile, HumdrumFile& infile,
		                                  int ctrack, HTp compositeoldStart);
		void        doTotalAnalysis(HumdrumFile& outfile, HumdrumFile& infile, int ctrack);
		void        doGroupAnalyses(HumdrumFile& outfile, HumdrumFile& infile);
		int         countNoteOnsets(HTp token);
		void        doTotalOnsetAnalysis(std::vector<double>& analysis, HumdrumFile& infile,
		                                  int track, std::vector<bool>& tracks);
		void        doGroupOnsetAnalyses(std::vector<double>& analysisA,
		                                  std::vector<double>& analysisB,
		                                  HumdrumFile& infile);
		void        doCoincidenceOnsetAnalysis(std::vector<std::vector<double>>& analysis);
		void        insertAnalysesIntoFile(HumdrumFile& outfile, std::vector<std::string>& spines,
		                                   std::vector<int>& trackMap, std::vector<bool>& tracks);
		void        assignAnalysesToVdataTracks(std::vector<std::vector<double>*>& data,
		                                   std::vector<std::string>& spines, HumdrumFile& outfile);

	private:
		std::string m_pitch       = "eR";   // pitch to display for compositeold rhythm
		bool        m_onlygroupsQ = false;  // only split compositeold rhythms into markup groups
		bool        m_addgroupsQ  = false;  // do not split compositeold rhythms into markup groups
		bool        m_nogroupsQ   = false;  // has no groups in output
		bool        m_extractQ    = false;  // output only compositeold rhythm analysis (not input data)
		bool        m_appendQ     = false;  // display analysis at top of system
		bool        m_debugQ      = false;  // display debug information
		bool        m_graceQ      = false;  // include grace notes in compositeold rhythm
		bool        m_tremoloQ    = false;  // preserve tremolos
		bool        m_upQ         = false;  // force stem up
		bool        m_hasGroupsQ  = false;  // used with -M, -N option
		bool        m_nestQ       = false;  // used with --nest option
		bool        m_onlyQ       = false;  // used with --only option
		std::string m_only;                 // used with --only option
		bool        m_coincidenceQ = false; // used with -c option
		bool        m_assignedGroups = false;
		bool        m_suppressCMarkQ = false; // used with -c option when -M -m -N and -n not present
		std::string m_togetherInScore;    // used with -n option
		std::string m_together;           // used with -m option
		bool        m_coincideDisplayQ = true; // used with m_together and m_togetherInScore

		// Analysis variables:
		bool        m_analysisOnsetsQ    = false;   // used with -P option
		bool        m_analysisAccentsQ   = false;   // used with -A option
		bool        m_analysisOrnamentsQ = false;   // used with -O option
		bool        m_analysisSlursQ     = false;   // used with -S option
		bool        m_analysisTotalQ    = false;   // used with -T option
		bool        m_analysisQ          = false;   // union of -paost options
		bool        m_nozerosQ           = false;   // used with -Z option
		std::vector<std::vector<double>> m_analysisOnsets;    // used with -P
		std::vector<std::vector<double>> m_analysisAccents;   // used with -A
		std::vector<std::vector<double>> m_analysisOrnaments; // used with -O
		std::vector<std::vector<double>> m_analysisSlurs;     // used with -S
		std::vector<std::vector<double>> m_analysisTotal;    // used with -T

};

// END_MERGE

} // end namespace hum

#endif /* _TOOL_COMPOSITE_H */



