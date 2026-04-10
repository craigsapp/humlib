//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sun Feb 22 23:57:58 PST 2026
// Last Modified: Sun Feb 22 23:58:06 PST 2026
// Filename:      tool-text.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-text.h
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Extract **text spines
//

#ifndef _TOOL_TEXT_H
#define _TOOL_TEXT_H

#include "HumTool.h"
#include "HumdrumFile.h"

#include <ostream>
#include <sstream>
#include <string>

namespace hum {

// START_MERGE

class Tool_text : public HumTool {
	public:
		         Tool_text       (void);
		        ~Tool_text       () {};

		bool     run               (HumdrumFileSet& infiles);
		bool     run               (HumdrumFile& infile);
		bool     run               (const std::string& indata, std::ostream& out);
		bool     run               (HumdrumFile& infile, std::ostream& out);

	protected:
		void     processFile       (HumdrumFile& infile);
		void     initialize        (void);
		void     processTextSpine  (HTp tspine);
		void     processPlineSpine (HTp tspine);
		bool     hasParam          (HTp tspine, const std::string& target);
		std::string getParamList   (std::vector<HTp>& tspine, const std::string& target);
		std::string getParamList   (std::vector<std::vector<HTp>>& tspine, const std::string& target);
		std::string getParmTimestamp(HTp token, const std::string& target);
                void     removePartText    (HTp startspine);
		void     removeText        (HumdrumFile& infile);
		std::string getSyllable    (const std::string& token);
		void     fillPlines        (std::vector<std::vector<HTp>>& plines, HTp tspine);
		std::string getPlineLabel  (std::vector<HTp>& pieces);
		void     printPlineSyllables(std::vector<HTp>& pieces);
		std::string getPlineRow    (std::vector<HTp>& pieces);
		void        zprintPlineRow (std::vector<HTp>& pieces);
		void        makeTextArray  (std::vector<std::vector<HTp>>& texts, std::vector<HTp> spines);
		std::string makeStyle      (void);

	private:
		bool     m_onlyQ      = false;
		bool     m_aboveQ     = false;
		bool     m_belowQ     = false;
		bool     m_joinQ      = false;
		bool     m_removeQ    = false;
		bool     m_removeAllQ = false;
		bool     m_mergeQ     = true;

		std::vector<std::vector<std::string>> m_text;
		std::stringstream m_output;
};

// END_MERGE

} // end namespace hum

#endif /* _TOOL_TEXT_H */



