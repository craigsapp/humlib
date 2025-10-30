//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Thu Jan 30 22:31:31 PST 2020
// Last Modified: Thu Jan 30 22:31:35 PST 2020
// Filename:      tool-rfilt.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-rfilt.h
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Suppress pitch values by MIDI note number.
//

#ifndef _TOOL_RFILT_H
#define _TOOL_RFILT_H

#include "HumTool.h"
#include "HumdrumFile.h"

#include <ostream>
#include <string>
#include <vector>

namespace hum {

// START_MERGE

class Tool_rfilt : public HumTool {
	public:
		         Tool_rfilt       (void);
		        ~Tool_rfilt       () {};

		bool     run               (HumdrumFileSet& infiles);
		bool     run               (HumdrumFile& infile);
		bool     run               (const std::string& indata, std::ostream& out);
		bool     run               (HumdrumFile& infile, std::ostream& out);

	protected:
		void        processFile  (HumdrumFile& infile);
		void        processSpine (HTp token);
		void        initialize   (HumdrumFile& infile);
		bool        isDelete     (std::string& value);
		std::string addRest      (const std::string& input);

	private:
		bool     m_modifiedQ = false;
		std::vector<bool> m_spines;
		std::vector<bool> m_midi;

};

// END_MERGE

} // end namespace hum

#endif /* _TOOL_RFILT_H */



