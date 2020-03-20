//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Wed Mar 18 12:59:25 PDT 2020
// Last Modified: Wed Mar 18 12:59:28 PDT 2020
// Filename:      tool-tremolo.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-tremolo.h
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Tremolo expansion tool.
//

#ifndef _TOOL_TREMOLO_H
#define _TOOL_TREMOLO_H

#include "HumTool.h"
#include "HumdrumFile.h"

namespace hum {

// START_MERGE

class Tool_tremolo : public HumTool {
	public:
		         Tool_tremolo       (void);
		        ~Tool_tremolo       () {};

		bool     run               (HumdrumFileSet& infiles);
		bool     run               (HumdrumFile& infile);
		bool     run               (const string& indata, ostream& out);
		bool     run               (HumdrumFile& infile, ostream& out);

	protected:
		void    processFile        (HumdrumFile& infile);
		void    removeMarkup       (void);
		void    expandTremolos     (void);
		void    expandTremolo      (HTp token);
		void    addTremoloInterpretations(HumdrumFile& infile);
		void    storeFirstTremoloNoteInfo(HTp token);
		void    storeLastTremoloNoteInfo(HTp token);

	private:
		bool    m_keepQ      = false;
		bool    m_modifiedQ  = false;
		std::vector<HTp> m_markup_tokens;
		std::vector<HumNum> m_first_tremolo_time;
		std::vector<HumNum> m_last_tremolo_time;


};

// END_MERGE

} // end namespace hum

#endif /* _TOOL_TREMOLO_H */



