//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Jul 29 16:48:22 CEST 2019
// Last Modified: Mon Jan 18 11:30:01 PST 2021
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
		       	   Tool_composite        (void);
		       	  ~Tool_composite        () {};

		bool        run                  (HumdrumFileSet& infiles);
		bool        run                  (HumdrumFile& infile);
		bool        run                  (const std::string& indata, ostream& out);
		bool        run                  (HumdrumFile& infile, ostream& out);

	protected:
		void        processFile          (HumdrumFile& infile);
		void        prepareMultipleGroups(HumdrumFile& infile);
		void        prepareSingleGroup   (HumdrumFile& infile);
		void        initialize           (void);
		int         typeStringToInt      (const std::string& value);
		HumNum      getLineDuration      (HumdrumFile& infile, int index, std::vector<bool>& isNull);
		void        getGroupStates       (std::vector<std::vector<int>>& groupstates, HumdrumFile& infile);
		void        assignGroups         (HumdrumFile& infile);
		void        analyzeLineGroups    (HumdrumFile& infile);
		void        analyzeLineGroup     (HumdrumFile& infile, int line, const std::string& target);
		void        printGroupAssignments(HumdrumFile& infile);
		std::string getGroup             (std::vector<std::vector<std::string>>& current, int spine, int subspine);
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
		void        addLabels            (HumdrumFile& infile, int amount);
		void        addStria             (HumdrumFile& infile, int amount);
		bool        pitchesEqual         (vector<int>& pitches1, vector<int>& pitches2);
		void        mergeTremoloGroup    (vector<HTp>& notes, vector<int> groups, int group);
		bool        onlyAuxTremoloNotes  (HumdrumFile& infile, int line);
		void        removeAuxTremolosFromCompositeRhythm(HumdrumFile& infile);

	private:
		std::string m_pitch     = "eR";   // pitch to display for composite rhythm
		bool        m_nogroupsQ = false;  // do not split composite rhythms into markup groups
		bool        m_extractQ  = false;  // output only composite rhythm analysis (not input data)
		bool        m_appendQ   = false;  // display analysis at top of system
		bool        m_debugQ    = false;  // display debug information
		bool        m_graceQ    = false;  // include grace notes in composite rhythm

};

// END_MERGE

} // end namespace hum

#endif /* _TOOL_COMPOSITE_H */



