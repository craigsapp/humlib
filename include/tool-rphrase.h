//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Jul 15 23:31:37 PDT 2024
// Last Modified: Mon Jul 15 23:31:40 PDT 2024
// Filename:      tool-rphrase.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-rphrase.h
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Calculate duration of notes as phrases delimited by rests.
//

#ifndef _TOOL_RPHRASE_H
#define _TOOL_RPHRASE_H

#include "HumTool.h"
#include "HumdrumFile.h"

#include <ostream>
#include <string>
#include <vector>

namespace hum {

// START_MERGE

class Tool_rphrase : public HumTool {
	public:

	class VoiceInfo {
		public:
			std::string name;
			std::vector<double> phraseDurs;
			std::vector<int>    barStarts;
	};

		         Tool_rphrase      (void);
		        ~Tool_rphrase      () {};

		bool     run               (HumdrumFileSet& infiles);
		bool     run               (HumdrumFile& infile);
		bool     run               (const std::string& indata, std::ostream& out);
		bool     run               (HumdrumFile& infile, std::ostream& out);
		void     finally           (void);

	protected:
		void     initialize        (void);
		void     processFile       (HumdrumFile& infile);
		void     fillVoiceInfo     (std::vector<Tool_rphrase::VoiceInfo>& voiceInfo, std::vector<HTp>& kstarts);
		void     fillVoiceInfo     (Tool_rphrase::VoiceInfo& voiceInfo, HTp& kstart);
		void     printVoiceInfo    (std::vector<Tool_rphrase::VoiceInfo>& voiceInfo);
		void     printVoiceInfo    (Tool_rphrase::VoiceInfo& voiceInfo);

	private:
		bool        m_averageQ      = false; // for -a option
		bool        m_allAverageQ   = false; // for -A option
		bool        m_barlineQ      = false; // for -m option
		bool        m_filenameQ     = false; // for -f option
		bool        m_fullFilenameQ = false; // for -F option
		std::string m_filename;              // for -f or -F option
		bool        m_sortQ         = false; // for -s option
		bool        m_reverseSortQ  = false; // for -S option
		int         m_pcount        = 0;     // for -a option
		double      m_sum           = 0.0;   // for -a option

};

// END_MERGE

} // end namespace hum

#endif /* _TOOL_RPHRASE_H */



