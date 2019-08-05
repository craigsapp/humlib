//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Wed Apr 10 08:58:07 EDT 2019
// Last Modified: Wed Apr 10 10:14:50 EDT 2019
// Filename:      tool-pnum.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-pnum.h
// Syntax:        C++11; humlib
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Interface for pnum tool.
//

#ifndef _TOOL_PNUM_H_INCLUDED
#define _TOOL_PNUM_H_INCLUDED

#include "HumTool.h"
#include "HumdrumFile.h"

namespace hum {

// START_MERGE

class Tool_pnum : public HumTool {
	public:
		      Tool_pnum               (void);
		     ~Tool_pnum               () {};

		bool  run                     (HumdrumFileSet& infiles);
		bool  run                     (HumdrumFile& infile);
		bool  run                     (const string& indata, ostream& out);
		bool  run                     (HumdrumFile& infile, ostream& out);

	protected:
		void  initialize              (HumdrumFile& infile);
		void  processFile             (HumdrumFile& infile);
		std::string convertSubtokenToBase(const std::string& text);
		void  convertTokenToBase      (HTp token);

	private:
		int  m_base = 12;
		int  m_midiQ;
		bool m_durationQ;
		bool m_classQ;
		bool m_octaveQ;
		bool m_attacksQ;
		std::string m_rest;
		bool m_restQ;

};


// END_MERGE

} // end namespace hum

#endif /* _TOOL_PNUM_H_INCLUDED */



