//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon May  1 07:19:59 PDT 2023
// Last Modified: Mon May  1 07:20:02 PDT 2023
// Filename:      tool-ordergps.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-ordergps.h
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Make *group, *part, and *staff interpretations adjacent and in
//                that order.  Only looking before the first data line.
//

#ifndef _TOOL_ORDERGPS_H
#define _TOOL_ORDERGPS_H

#include "HumTool.h"
#include "HumdrumFile.h"

#include <ostream>
#include <string>

namespace hum {

// START_MERGE

class Tool_ordergps : public HumTool {
	public:
		      Tool_ordergps  (void);
		     ~Tool_ordergps  () {};

		bool  run            (HumdrumFileSet& infiles);
		bool  run            (HumdrumFile& infile);
		bool  run            (const std::string& indata, std::ostream& out);
		bool  run            (HumdrumFile& infile, std::ostream& out);

	protected:
		void  initialize     (void);
		void  processFile    (HumdrumFile& infile);
		void  printStaffLine (HumdrumFile& infile);
		void  printFile      (HumdrumFile& infile, int gindex, int pindex, int sindex);
		void  printFileTop   (HumdrumFile& infile, int gindex, int pindex, int sindex);

	private:
		bool m_emptyQ   = false;
		bool m_fileQ    = false;
		bool m_listQ    = false;
		bool m_problemQ = false;
		bool m_reverseQ = false;
		bool m_staffQ   = false;
		bool m_topQ     = false;

};

// END_MERGE

} // end namespace hum

#endif /* _TOOL_ORDERGPS_H */



