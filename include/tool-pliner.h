//
// Programmer:    Alexander Morgan
// Creation Date: Wed Jul 15 2026
// Filename:      tool-pliner.h
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Insert *pline poetic-line annotations into a **kern score
//                by aligning the sung **text declamation of each voice
//                against a poem given in a !!@VERSE: global comment block.
//

#ifndef _TOOL_PLINER_H
#define _TOOL_PLINER_H

#include "HumTool.h"
#include "HumdrumFile.h"

#include <map>
#include <ostream>
#include <string>
#include <vector>

namespace hum {

// START_MERGE

class Tool_pliner : public HumTool {
	public:
		         Tool_pliner     (void);
		        ~Tool_pliner     () {};

		bool     run               (HumdrumFileSet& infiles);
		bool     run               (HumdrumFile& infile);
		bool     run               (const std::string& indata, std::ostream& out);
		bool     run               (HumdrumFile& infile, std::ostream& out);

	protected:
		void     initialize        (void);
		void     processFile       (HumdrumFile& infile);

		// poem model:
		struct PoemWord {
			std::string original;
			std::string norm;
			int line   = -1;
			int pos    = -1;
		};

		bool     parseVerse        (HumdrumFile& infile, std::vector<std::vector<PoemWord>>& poem);

		// voice model:
		struct Voice {
			HTp kernStart = NULL;
			HTp textStart = NULL;
		};

		struct SungWord {
			HTp token       = NULL; // starting token of the reconstructed word
			std::string norm;
			bool capitalized = false;
		};

		struct Span {
			int line      = -1;
			int startPos  = -1;
			int endPos    = -1;
			bool repeat   = false;
			HTp startToken = NULL;
		};

		void     getVoices          (HumdrumFile& infile, std::vector<Voice>& voices);
		void     buildSungWords     (HTp textStart, std::vector<SungWord>& words);
		std::string cleanSyllable   (const std::string& text);
		std::string normalizeWord   (const std::string& text);
		void     alignVoice         (std::vector<SungWord>& words,
		                             std::vector<std::vector<PoemWord>>& poem,
		                             std::vector<Span>& spans);
		std::string getModifier     (std::vector<std::vector<PoemWord>>& poem, Span& span);
		void     emitOutput         (HumdrumFile& infile,
		                             std::map<int, std::map<int, std::string>>& insertions);

	private:
		std::vector<std::vector<PoemWord>> m_poem;
		std::vector<Voice> m_voices;

};

// END_MERGE

} // end namespace hum

#endif /* _TOOL_PLINER_H */
