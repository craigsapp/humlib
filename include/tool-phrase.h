//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Thu May 31 16:56:54 PDT 2018
// Last Modified: Sun Jun  3 19:14:07 PDT 2018
// Filename:      tool-phrase.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-phrase.h
// Syntax:        C++11; humlib
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Interface for phrase tool.
//

#ifndef _TOOL_PHRASE_H_INCLUDED
#define _TOOL_PHRASE_H_INCLUDED

#include "HumTool.h"
#include "HumdrumFile.h"

#include <ostream>
#include <string>
#include <vector>

namespace hum {

// START_MERGE

class Tool_phrase : public HumTool {
	public:
		     Tool_phrase          (void);
		    ~Tool_phrase          () {};

		bool  run                 (HumdrumFileSet& infiles);
		bool  run                 (HumdrumFile& infile);
		bool  run                 (const std::string& indata, std::ostream& out);
		bool  run                 (HumdrumFile& infile, std::ostream& out);

	protected:
		void  analyzeSpineByRests (int index);
		void  analyzeSpineByPhrase(int index);
		void  initialize          (HumdrumFile& infile);
		void  prepareAnalysis     (HumdrumFile& infile);
		void  addAverageLines     (HumdrumFile& infile);
		bool  hasPhraseMarks      (HTp start);
		void  removePhraseMarks   (HTp start);

	private:
		std::vector<std::vector<std::string>>    m_results;
		std::vector<HTp>               m_starts;
		HumdrumFile               m_infile;
		std::vector<int>               m_pcount;
		std::vector<HumNum>            m_psum;
		bool                      m_markQ;
		bool                      m_removeQ;
		bool                      m_remove2Q;
		bool                      m_averageQ;
		std::string               m_color;

};


// END_MERGE

} // end namespace hum

#endif /* _TOOL_PHRASE_H_INCLUDED */



