//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Thu May  2 19:59:36 PDT 2024
// Last Modified: Thu May  2 19:59:39 PDT 2024
// Filename:      tool-chint.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-chint.h
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Color harmonic intervals between pairs of parts.
//

#ifndef _TOOL_CHINT_H
#define _TOOL_CHINT_H

#include "HumTool.h"
#include "HumdrumFile.h"

namespace hum {

// START_MERGE

class Tool_chint : public HumTool {
	public:
		         Tool_chint          (void);
		        ~Tool_chint          () {};

		bool     run                 (HumdrumFileSet& infiles);
		bool     run                 (HumdrumFile& infile);
		bool     run                 (const std::string& indata, std::ostream& out);
		bool     run                 (HumdrumFile& infile, std::ostream& out);

	protected:
		void    processFile          (HumdrumFile& infile);
		void    initialize           (void);
		void    fillIntervalNames    (void);
		void    fillIntervalNamesDiatonic(void);
		void    chromaticColoring    (void);
		void    dissonanceColoring   (void);
		void    getPartIntervals     (std::vector<int>& topInterval,
		                              std::vector<int>& botInterval,
		                              HTp botSpine, HTp topSpine, HumdrumFile& infile);
		void    insertPartColors     (HumdrumFile& infile, std::vector<int>& botInterval,
		                              std::vector<int>& topInterval, int botTrack, int topTrack);
		std::string getColorToken    (int interval, HumdrumFile& infile, int line, HTp token);
		std::string getIntervalToken (int interval, HumdrumFile& infile, int line);

	private:
		// m_color: Color mapping for notes, indexed by base-40:
		std::vector<std::string> m_color;

		// m_intervals: Names of intervals indexed by base-40:
		std::vector<std::string> m_intervals;

		// Used in particular to avoid adding interval when both
		// staves have tied notes:
		std::vector<std::string>  m_botPitch;
		std::vector<std::string>  m_topPitch;

		// m_intervalQ: Show interval numbers
		bool m_intervalQ = false;

		// m_octaveQ: Do not collapse octaves to unisons.
		bool m_octaveQ = false;

		// m_noColorBotQ: Do not colorize bottom analysis staff
		bool m_noColorBotQ = false;

		// m_noColortopQ: Do not colorize top analysis staff
		bool m_noColorTopQ = false;

		// m_negativeQ: Add minus sign to intervals
		// when staff notes are crossed.
		bool m_negativeQ = false;

		// m_middle: Add intervals between analysis staves, or actually
		// below top staff. (Only effective in JavaScript compiled code.)
		bool m_middleQ = true;

};

// END_MERGE

} // end namespace hum

#endif /* _TOOL_CHINT_H */



