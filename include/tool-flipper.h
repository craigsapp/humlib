//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Wed Aug 26 15:19:49 PDT 2020
// Last Modified: Wed Aug 26 15:19:55 PDT 2020
// Filename:      tool-flipper.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-flipper.h
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Switch the order of subspines.
//

#ifndef _TOOL_FLIPPER_H
#define _TOOL_FLIPPER_H

#include "HumTool.h"
#include "HumdrumFile.h"

namespace hum {

// START_MERGE

class Tool_flipper : public HumTool {
	public:
		         Tool_flipper      (void);
		        ~Tool_flipper      () {};

		bool     run               (HumdrumFileSet& infiles);
		bool     run               (HumdrumFile& infile);
		bool     run               (const string& indata, ostream& out);
		bool     run               (HumdrumFile& infile, ostream& out);

	protected:
		void     processFile       (HumdrumFile& infile);
		void     initialize        (void);

		void     processLine        (HumdrumFile& infile, int index);
		void     checkForFlipChanges(HumdrumFile& infile, int index);
		bool     flipSubspines      (vector<vector<HTp>>& flipees);
		void     flipSpineTokens    (vector<HTp>& subtokens);
		void     extractFlipees     (vector<vector<HTp>>& flipees,
		                             HumdrumFile& infile, int index);

	private:
		bool     m_allQ         = false;
		bool     m_keepQ        = false;
		bool     m_kernQ        = false;
		bool     m_stropheQ     = false;
		std::string m_interp;
		std::vector<bool> m_flipState;
		std::vector<bool> m_fliplines;
		std::vector<bool> m_strophe;

};

// END_MERGE

} // end namespace hum

#endif /* _TOOL_FLIPPER_H */



