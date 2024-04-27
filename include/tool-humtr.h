//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Fri May  6 19:29:57 PDT 2022
// Last Modified: Fri May  6 19:42:55 PDT 2022
// Filename:      tool-humtr.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-humtr.h
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Convert text to modern characters.
//

#ifndef _TOOL_HUMTR_H
#define _TOOL_HUMTR_H

#include "HumTool.h"
#include "HumdrumFile.h"

#include <ostream>
#include <string>

namespace hum {

// START_MERGE

class Tool_humtr : public HumTool {
	public:
		            Tool_humtr        (void);
		           ~Tool_humtr        () {};

		bool        run               (HumdrumFileSet& infiles);
		bool        run               (HumdrumFile& infile);
		bool        run               (const std::string& indata, std::ostream& out);
		bool        run               (HumdrumFile& infile, std::ostream& out);

	protected:
		void        processFile       (HumdrumFile& infile);
		void        initialize        (void);
		void        addFromToCombined (const std::string& value);
		void        fillFromToPair    (const std::string& from, const std::string& to);
		void        displayFromToTable(void);
		std::vector<std::string> getUtf8CharacterArray(const std::string& value);

		std::string transliterateText(const std::string& input);
		std::string transliterateTextNonOverlapping (const std::string& input);
		std::string transliterateTextOverlapping    (const std::string& input);
		void        processTextStrand      (HTp stok, HTp etok);
		void        convertTextSpines      (HumdrumFile& infile);
		void        convertLocalLayoutText (HumdrumFile& infile);
		void        convertGlobalLayoutText(HumdrumFile& infile);
		void        convertReferenceText   (HumdrumFile& infile);

	private:
		bool m_lyricsQ    = true;  // do not convert lyrics in **text spines.
		bool m_localQ     = true;  // do not convert t= fields in !LO: parameters.
		bool m_globalQ    = true;  // do not convert t= fields in !!LO: parameters.
		bool m_referenceQ = true;  // do not convert reference record values.

		bool m_lyricsOnlyQ;
		bool m_localOnlyQ;
		bool m_globalOnlyQ;
		bool m_referenceOnlyQ;

		std::string m_sep1  = " ";
		std::string m_sep2  = ":";

		std::vector<std::string> m_from;
		std::vector<std::string> m_to;

};

// END_MERGE

} // end namespace hum

#endif /* _TOOL_HUMTR_H */



