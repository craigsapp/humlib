//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Wed Sep 20 13:05:37 PDT 2023
// Last Modified: Wed Sep 20 13:05:41 PDT 2023
// Filename:      tool-nproof.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-nproof.h
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
// Documentation: https://doc.verovio.humdrum.org/filter/nproof
//
// Description:   NIFC Humdrum score proof program.
//

#ifndef _TOOL_NPROOF_H
#define _TOOL_NPROOF_H

#include "HumTool.h"
#include "HumdrumFile.h"

namespace hum {

// START_MERGE

class Tool_nproof : public HumTool {

	public:
		         Tool_nproof        (void);
		        ~Tool_nproof        () {};

		bool     run               (HumdrumFileSet& infiles);
		bool     run               (HumdrumFile& infile);
		bool     run               (const string& indata, std::ostream& out);
		bool     run               (HumdrumFile& infile, std::ostream& out);

		void     checkForBlankLines(HumdrumFile& infile);
		void     checkInstrumentInformation(HumdrumFile& infile);
		void     checkKeyInformation(HumdrumFile& infile);
		void     checkSpineTerminations(HumdrumFile& infile);
		void     checkForValidInstrumentCode(HTp token, vector<pair<string, string>>& instrumentList);
		void     checkReferenceRecords(HumdrumFile& infile);

	protected:
		void     initialize        (void);
		void     processFile       (HumdrumFile& infile);

	private:
		int m_errorCount = 0;
		std::string m_errorList;
		std::string m_errorHtml;

		bool m_noblankQ       = false;
		bool m_noinstrumentQ  = false;
		bool m_nokeyQ         = false;
		bool m_noreferenceQ   = false;
		bool m_noterminationQ = false;

		bool m_fileQ          = false;
		bool m_rawQ           = false;

};

// END_MERGE

} // end namespace hum

#endif /* _TOOL_NPROOF_H */



