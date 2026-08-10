//
// Programmer:    Alexander Morgan
// Creation Date: Fri Jul 24 2026
// Filename:      tool-textract.h
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Reconstruct the poetic text being set in a score by
//                examining each voice's **text underlay (not !!@VERSE).
//

#ifndef _TOOL_TEXTRACT_H
#define _TOOL_TEXTRACT_H

#include "HumTool.h"
#include "HumdrumFile.h"

#include <ostream>
#include <string>
#include <vector>

namespace hum {

// START_MERGE

class Tool_textract : public HumTool {
	public:
		         Tool_textract    (void);
		        ~Tool_textract    () {};

		bool     run              (HumdrumFileSet& infiles);
		bool     run              (HumdrumFile& infile);
		bool     run              (const std::string& indata, std::ostream& out);
		bool     run              (HumdrumFile& infile, std::ostream& out);

	protected:
		struct SungWord {
			std::string original;
			std::string norm;
			int  syllables   = 0;
			bool capitalized = false;
			bool bis         = false;
		};

		struct Voice {
			HTp textStart = NULL;
			std::vector<SungWord> words;
			std::vector<std::vector<SungWord>> lines;
		};

		struct LineCluster {
			std::vector<std::vector<SungWord>> members; // one entry per contributing voice line
			std::vector<int> voiceIds;
			double avgPos = 0.0;
		};

		void     initialize       (void);
		void     processFile      (HumdrumFile& infile);

		void     getVoices        (HumdrumFile& infile, std::vector<Voice>& voices);
		void     buildSungWords   (HTp textStart, std::vector<SungWord>& words);
		std::string normalizeWord (const std::string& text);
		std::string cleanOrigPiece(const std::string& text);
		void     collapseRepeats  (std::vector<SungWord>& words);
		void     segmentLines     (Voice& voice);
		int      lineSyllables    (const std::vector<SungWord>& line);
		int      distanceToAllowed(int syllables);
		bool     isAllowedLength  (int syllables, int tol = 0);
		int      minAllowedLength (void);
		int      maxAllowedLength (void);
		bool     endsWithVowel    (const std::string& norm);
		bool     startsWithVowel  (const std::string& norm);
		bool     elidesWith       (const SungWord& left, const SungWord& right);
		bool     likelyLineStart  (const std::string& norm);
		bool     linesSimilar     (const std::vector<SungWord>& a,
		                           const std::vector<SungWord>& b);
		bool     isSubSequence    (const std::vector<SungWord>& shorter,
		                           const std::vector<SungWord>& longer);
		void     dedupeVoiceLines (Voice& voice);
		void     reconstructText  (std::vector<Voice>& voices);
		void     refineLines      (std::vector<std::vector<SungWord>>& lines);
		int      detectGenreLineCount(HumdrumFile& infile);
		void     enforceLineCount (std::vector<std::vector<SungWord>>& lines);
		std::vector<SungWord> consensusLine(LineCluster& cluster);
		std::string lineToString  (const std::vector<SungWord>& line);

	private:
		std::vector<int> m_sylCounts; // empty = unused
		int m_expectedLines = 0;      // 0 = unset / auto
};

// END_MERGE

} // end namespace hum

#endif /* _TOOL_TEXTRACT_H */
