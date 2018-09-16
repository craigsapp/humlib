//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Fri Aug 24 17:32:47 PDT 2018
// Last Modified: Fri Aug 24 17:32:51 PDT 2018
// Filename:      tool-trillspell.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-trill.h
// Syntax:        C++11
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Interface for trillspell tool, which assigns intervals to
//                trill, mordent and turn ornaments based on the key
//                signature and previous notes in a measure.
//

#ifndef _TOOL_TRILLSPELL_H_INCLUDED
#define _TOOL_TRILLSPELL_H_INCLUDED

#include "HumTool.h"
#include "HumdrumFile.h"

#include <vector>
#include <string>

namespace hum {

// START_MERGE

class Tool_trillspell : public HumTool {
	public:
		      Tool_trillspell     (void);
		     ~Tool_trillspell     () {};

		bool  run                 (HumdrumFile& infile);
		bool  run                 (const string& indata, ostream& out);
		bool  run                 (HumdrumFile& infile, ostream& out);

	protected:
		void  processFile         (HumdrumFile& infile);
		bool  analyzeOrnamentAccidentals(HumdrumFile& infile);
		void  resetDiatonicStatesWithKeySignature(vector<int>& states,
		                                          vector<int>& signature);
		void  fillKeySignature    (vector<int>& states, const string& keysig);
		int   getBase40           (int diatonic, int accidental);

	private:
		bool m_xmark = false;

};


// END_MERGE

} // end namespace hum

#endif /* _TOOL_TRILLSPELL_H_INCLUDED */



