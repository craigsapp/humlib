//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Nov 23 05:24:14 PST 2009
// Last Modified: Sun Feb  2 22:44:45 PST 2020
// Filename:      tool-rid.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-rid.h
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Remove various components of Humdrum file data structures.
//

#ifndef _TOOL_RID_H
#define _TOOL_RID_H

#include "HumTool.h"
#include "HumdrumFile.h"

namespace hum {

// START_MERGE

class Tool_rid : public HumTool {
	public:
		         Tool_rid          (void);
		        ~Tool_rid          () {};

		bool     run               (HumdrumFileSet& infiles);
		bool     run               (HumdrumFile& infile);
		bool     run               (const string& indata, ostream& out);
		bool     run               (HumdrumFile& infile, ostream& out);

	protected:
		void     processFile       (HumdrumFile& infile);
		void     initialize        (void);

	private:

		// User interface variables:
		int      option_D = 0;   // used with -D option
		int      option_d = 0;   // used with -d option
		int      option_G = 0;   // used with -G option
		int      option_g = 0;   // used with -g option
		int      option_I = 0;   // used with -I option
		int      option_i = 0;   // used with -i option
		int      option_L = 0;   // used with -L option
		int      option_l = 0;   // used with -l option
		int      option_T = 0;   // used with -T option
		int      option_U = 0;   // used with -U and -u option

		int      option_M = 0;   // used with -M option
		int      option_C = 0;   // used with -C option
		int      option_c = 0;   // used with -c option
		int      option_k = 0;   // used with -k option
		int      option_V = 0;   // used with -V option

};

// END_MERGE

} // end namespace hum

#endif /* _TOOL_RID_H */



