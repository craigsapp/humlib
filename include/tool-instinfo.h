//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Thu May  9 23:21:12 PDT 2024
// Last Modified: Thu May  9 23:21:18 PDT 2024
// Filename:      tool-instinfo.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-instinfo.h
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Insert instrument info into Humdrum score.
//

#ifndef _TOOL_INSTINFO_H
#define _TOOL_INSTINFO_H

#include "HumTool.h"
#include "HumdrumFile.h"

#include <map>
#include <ostream>
#include <string>

namespace hum {

// START_MERGE

class Tool_instinfo : public HumTool {
	public:
		         Tool_instinfo      (void);
		        ~Tool_instinfo      () {};

		bool     run                (HumdrumFileSet& infiles);
		bool     run                (HumdrumFile& infile);
		bool     run                (const std::string& indata, std::ostream& out);
		bool     run                (HumdrumFile& infile, std::ostream& out);

	protected:
		void    processFile         (HumdrumFile& infile);
		void    initialize          (HumdrumFile& infile);
		void    updateInstrumentLine(HumdrumFile& infile, int inumIndex,
		                             std::map<int, std::string>& value,
		                             std::map<int, int>& track2kindex,
		                             const std::string& prefix);
		void    insertInstrumentInfo(HumdrumFile& infile, int index,
		                             std::map<int, std::string>& info, const std::string& prefix,
		                             const std::string& key, std::map<int, int>& track2kindex);
		void    printLine           (HumdrumFile& infile, int index);
		void    printLine           (HumdrumFile& infile, int index,
		                             const std::string& key);

	private:
		std::map<int, std::string> m_icode;  // instrument code, e.g., *Iflt
		std::map<int, std::string> m_iclass; // instrument class, e.g., *Iww
		std::map<int, std::string> m_iname;  // instrument name, e.g., *I"flute
		std::map<int, std::string> m_iabbr;  // instrument name, e.g., *I'flt.
		std::map<int, std::string> m_inum;   // instrument number, e.g., *I#2

};

// END_MERGE

} // end namespace hum

#endif /* _TOOL_INSTINFO_H */



