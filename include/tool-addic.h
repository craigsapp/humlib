//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Tue Sep 26 04:25:55 PDT 2023
// Last Modified: Tue Sep 26 04:25:58 PDT 2023
// Filename:      tool-addic.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-addic.h
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Adds and fills in *IC lines for instrument code lines.
//

#ifndef _TOOL_ADDIC_H
#define _TOOL_ADDIC_H

#include "HumTool.h"
#include "HumdrumFile.h"

namespace hum {

// START_MERGE

class Tool_addic : public HumTool {
	public:
		         Tool_addic        (void);
		        ~Tool_addic        () {};

		bool     run               (HumdrumFileSet& infiles);
		bool     run               (HumdrumFile& infile);
		bool     run               (const string& indata, ostream& out);
		bool     run               (HumdrumFile& infile, ostream& out);

		int      getInstrumentCodeIndex(HumdrumFile& infile);
		int      getInstrumentClassIndex(HumdrumFile& infile);
		void     updateInstrumentClassLine(HumdrumFile& infile, int codeIndex, int classIndex);
		std::string makeClassLine(HumdrumFile& infile, int codeIndex);
		std::string getInstrumentClass(const string& code);


	protected:
		void     initialize        (void);
		void     processFile       (HumdrumFile& infile);

	private:
		std::vector<std::pair<std::string, std::string>> m_instrumentList;
		bool m_fixQ = false;  // used with -f option: fix incorrect instrument classes


};

// END_MERGE

} // end namespace hum

#endif /* _TOOL_ADDIC_H */



