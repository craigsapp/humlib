//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Fri May  6 19:29:57 PDT 2022
// Last Modified: Fri May  6 19:42:55 PDT 2022
// Filename:      tool-popctext.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-popctext.h
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Convert text to modern characters.
//

#ifndef _TOOL_POPCTEXT_H
#define _TOOL_POPCTEXT_H

#include "HumTool.h"
#include "HumdrumFile.h"

namespace hum {

// START_MERGE

class Tool_popctext : public HumTool {
	public:
		            Tool_popctext     (void);
		           ~Tool_popctext     () {};
  
		bool        run               (HumdrumFileSet& infiles);
		bool        run               (HumdrumFile& infile);
		bool        run               (const string& indata, ostream& out);
		bool        run               (HumdrumFile& infile, ostream& out);

	protected:
		void        processFile       (HumdrumFile& infile);
		void        initialize        (void);

		std::string modernizeText          (const string& input);
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

};

// END_MERGE

} // end namespace hum

#endif /* _TOOL_POPCTEXT_H */



