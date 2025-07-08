//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Tue Jul  8 07:26:01 CEST 2025
// Last Modified: Tue Jul  8 07:26:04 CEST 2025
// Filename:      tool-got2hum.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-got2hum.h
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Convert 3-staff single-voice music into 2-staff Grand Staff layout.
//

#ifndef _TOOL_GOT2HUM_H
#define _TOOL_GOT2HUM_H

#include "HumTool.h"
#include "HumdrumFile.h"
#include "GotScore.h"

#include <ostream>
#include <string>
#include <vector>

namespace hum {

// START_MERGE

class Tool_got2hum : public HumTool {
	public:
		         Tool_got2hum      (void);
		        ~Tool_got2hum      () {};

		bool     run               (HumdrumFileSet& infiles);
		bool     run               (HumdrumFile& infile);
		bool     run               (const std::string& indata, std::ostream& out);
		bool     run               (HumdrumFile& infile, std::ostream& out);

	protected:
		void    processFile        (const std::string& instring);
		void    initialize         (void);

	private:
		GotScore m_gotscore;

		bool m_editorialQ;
		bool m_cautionaryQ;
		bool m_gotQ;
		bool m_modern_accQ;
};

// END_MERGE

} // end namespace hum

#endif /* _TOOL_GOT2HUM_H */



