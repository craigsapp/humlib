//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sun Aug 27 07:18:04 PDT 2017
// Last Modified: Sun Aug 27 07:18:07 PDT 2017
// Filename:      tool-msearch.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-msearch.h
// Syntax:        C++11
// vim:           ts=3 noexpandtab
//
// Description:   Interface for msearch tool.
//

#ifndef _TOOL_MSEARCH_H
#define _TOOL_MSEARCH_H

#include "HumTool.h"
#include "HumdrumFile.h"
#include "NoteGrid.h"

namespace hum {

// START_MERGE

class Tool_msearch : public HumTool {
	public:
		         Tool_msearch      (void);
		        ~Tool_msearch      () {};

		bool     run               (HumdrumFile& infile);
		bool     run               (const string& indata, ostream& out);
		bool     run               (HumdrumFile& infile, ostream& out);

	protected:
		void    doAnalysis         (HumdrumFile& infile, NoteGrid& grid,
		                            vector<double>& query);
		void    fillQueryDiatonicPC(vector<double>& query, const string& input);
		bool    checkForMatchDiatonicPC(vector<NoteCell*>& notes, int index, 
		                            vector<double>& dpcQuery,
		                            vector<NoteCell*>& match);
		void     markMatch         (HumdrumFile& infile, vector<NoteCell*>& match);

	private:
	 	vector<HTp> m_kernspines;
};

// END_MERGE

} // end namespace hum

#endif /* _TOOL_MSEARCH_H */



