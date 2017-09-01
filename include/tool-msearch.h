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

class MSearchQueryToken {
	public:

		MSearchQueryToken(void) {
			clear();
		}

		MSearchQueryToken(const MSearchQueryToken& token) {
			pc        = token.pc;
			base      = token.base;
			direction = token.direction;
			duration  = token.duration;
			rhythm    = token.rhythm;
			anything  = token.anything;
		}

		MSearchQueryToken& operator=(const MSearchQueryToken& token) {
			if (this == &token) {
				return *this;
			}
			pc        = token.pc;
			base      = token.base;
			direction = token.direction;
			duration  = token.duration;
			rhythm    = token.rhythm;
			anything  = token.anything;
			return *this;
		}

		void clear(void) {
			pc        = NAN;
			base      = 0;
			direction = 0;
			duration  = -1;
			rhythm    = "";
			anything  = false;
		}

		double pc;           // NAN = rest
		int    base;
		int    direction; 
		HumNum duration;
		string rhythm;
		bool   anything;
};


class Tool_msearch : public HumTool {
	public:
		         Tool_msearch      (void);
		        ~Tool_msearch      () {};

		bool     run               (HumdrumFile& infile);
		bool     run               (const string& indata, ostream& out);
		bool     run               (HumdrumFile& infile, ostream& out);

	protected:
		void    doAnalysis         (HumdrumFile& infile, NoteGrid& grid,
		                            vector<MSearchQueryToken>& query);
		void    fillQuery          (vector<MSearchQueryToken>& query,
		                            const string& input);
		bool    checkForMatchDiatonicPC(vector<NoteCell*>& notes, int index, 
		                            vector<MSearchQueryToken>& dpcQuery,
		                            vector<NoteCell*>& match);
		void     markMatch         (HumdrumFile& infile, vector<NoteCell*>& match);

	private:
	 	vector<HTp> m_kernspines;
};

// END_MERGE

} // end namespace hum

#endif /* _TOOL_MSEARCH_H */



