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
		bool        run                  (const string& indata, ostream& out);
		bool        run                  (HumdrumFile& infile, ostream& out);

	protected:
		void        processFile          (HumdrumFile& infile);
		void        prepareMultipleGroups(HumdrumFile& infile);
		void        prepareSingleGroup   (HumdrumFile& infile);
		void        initialize           (void);
		int         typeStringToInt      (const string& value);
		HumNum      getLineDuration      (HumdrumFile& infile, int index, vector<bool>& isNull);
		void        getGroupStates       (vector<vector<int>>& groupstates, HumdrumFile& infile);
		void        assignGroups         (HumdrumFile& infile);
		void        analyzeLineGroups    (HumdrumFile& infile);
		void        analyzeLineGroup     (HumdrumFile& infile, int line, const string& target);
		void        printGroupAssignments(HumdrumFile& infile);
		string      getGroup             (vector<vector<string>>& current, int spine, int subspine);
		int         getGroupNoteType     (HumdrumFile& infile, int line, const string& group);
		void        getGroupDurations    (vector<vector<HumNum>>& groupdurs,
		                                  vector<vector<int>>& groupstates, HumdrumFile& infile);
		void        getGroupDurations    (vector<HumNum>& groupdurs, vector<int>& groupstates,
		                                  HumdrumFile& infile);
		void        getGroupRhythms      (vector<vector<string>>& rhythms, 
		                                  vector<vector<HumNum>>& groupdurs, 
		                                  vector<vector<int>>& groupstates, 
		                                  HumdrumFile& infile);
		void        getGroupRhythms      (vector<string>& rhythms, vector<HumNum>& durs,
		                                  vector<int>& states, HumdrumFile& infile);
		bool        hasGroupInterpretations(HumdrumFile& infile);

	private:
		string      m_pitch     = "eR";   // pitch to display for composite rhythm
		bool        m_nogroupsQ = false;  // do not split composite rhythms into markup groups
		bool        m_extractQ  = false;  // output only composite rhythm analysis (not input data)
		bool        m_appendQ   = false;  // display analysis at top of system
		bool        m_debugQ    = false;  // display debug information
		bool        m_graceQ    = false;  // include grace notes in composite rhythm

};

// END_MERGE

} // end namespace hum

#endif /* _TOOL_COMPOSITE_H */



