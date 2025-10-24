//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Thu Jan 30 22:31:31 PST 2020
// Last Modified: Thu Jan 30 22:31:35 PST 2020
// Filename:      tool-restit.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-restit.h
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Suppress pitch values by MIDI note number.
//

#ifndef _TOOL_RESTIT_H
#define _TOOL_RESTIT_H

#include "HumTool.h"
#include "HumdrumFile.h"

#include <ostream>
#include <string>
#include <vector>

namespace hum {

// START_MERGE

class Tool_restit : public HumTool {
	public:
		         Tool_restit       (void);
		        ~Tool_restit       () {};

		bool     run               (HumdrumFileSet& infiles);
		bool     run               (HumdrumFile& infile);
		bool     run               (const std::string& indata, std::ostream& out);
		bool     run               (HumdrumFile& infile, std::ostream& out);

	protected:
		void     processFile       (HumdrumFile& infile);
		void     processSpine      (HTp token);
		void     initialize        (HumdrumFile& infile);
		std::string filterNote     (std::string& value);

	private:
		bool     m_modifiedQ = false;
		std::vector<bool> m_spines;
		std::vector<bool> m_midi;

};

// END_MERGE

} // end namespace hum

#endif /* _TOOL_RESTIT_H */



