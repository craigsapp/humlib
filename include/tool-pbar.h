//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Tue Jul  2 17:22:10 CEST 2024
// Last Modified: Tue Jul  2 17:22:12 CEST 2024
// Filename:      tool-pbar.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-pbar.h
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Convert POPC2 old style barLines to *bar.
//

#ifndef _TOOL_PBAR_H
#define _TOOL_PBAR_H

#include "HumTool.h"
#include "HumdrumFile.h"

#include <ostream>
#include <string>
#include <vector>

namespace hum {

// START_MERGE

class Tool_pbar : public HumTool {
	public:
		            Tool_pbar        (void);
		           ~Tool_pbar        () {};

		bool        run               (HumdrumFileSet& infiles);
		bool        run               (HumdrumFile& infile);
		bool        run               (const std::string& indata, std::ostream& out);
		bool        run               (HumdrumFile& infile, std::ostream& out);

	protected:
		void        processFile       (HumdrumFile& infile);
		void        initialize        (void);
		void        processSpine      (HTp spineStart);
		void        printDataLine                   (HumdrumFile& infile, int index);
		void        printLocalCommentLine           (HumdrumFile& infile, int index);
		void        addBarLineToFollowingNoteOrRest (HTp token);
		void        printInvisibleBarlines          (HumdrumFile& infile, int index);
		void        printBarLine                    (HumdrumFile& infile, int index);

	private:
		bool        m_invisibleQ = false;   // used with -i option

};

// END_MERGE

} // end namespace hum

#endif /* _TOOL_PBAR_H */



