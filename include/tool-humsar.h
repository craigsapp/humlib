//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sun Oct 13 11:42:59 PDT 2019
// Last Modified: Sun Oct 13 11:43:02 PDT 2019
// Filename:      tool-humsar.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-humsar.h
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Sort data spines in a Humdrum file.
//

#ifndef _TOOL_HUMSAR_H
#define _TOOL_HUMSAR_H

#include "HumTool.h"
#include "HumdrumFile.h"

namespace hum {

// START_MERGE

class Tool_humsar : public HumTool {
	public:
		         Tool_humsar       (void);
		        ~Tool_humsar       () {};

		bool     run               (HumdrumFileSet& infiles);
		bool     run               (HumdrumFile& infile);
		bool     run               (const string& indata, ostream& out);
		bool     run               (HumdrumFile& infile, ostream& out);

	protected:
		void    processFile        (HumdrumFile& infile);
		void    searchAndReplaceInterpretation(HumdrumFile& infile);
		void    searchAndReplaceData(HumdrumFile& infile);
		void    searchAndReplaceBarline(HumdrumFile& infile);
		void    initialize         (void);
		void    initializeSegment  (HumdrumFile& infile);
		bool    isValid            (HTp token);
		bool    isValidDataType    (HTp token);
		bool    isValidSpine       (HTp token);
		void    fillInExInterpList (void);

	private:
		std::string m_search;      // search string
		std::string m_replace;     // replace string
		bool        m_interpretation = false; // process only interpretation records
		bool        m_modified = false;
		std::vector<std::string> m_exinterps; // list of exclusive interpretations to process
		std::vector<bool> m_spines; // usar with -s option
		std::string m_grepoptions;

};

// END_MERGE

} // end namespace hum

#endif /* _TOOL_HUMSAR_H */



