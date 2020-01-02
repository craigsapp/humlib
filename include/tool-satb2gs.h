//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sun Feb  6 14:33:36 PST 2011
// Last Modified: Wed Jan  1 22:13:58 PST 2020
// Filename:      tool-satb2gs.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-satb2gs.h
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Sort data spines in a Humdrum file.
//

#ifndef _TOOL_SATB2GS_H
#define _TOOL_SATB2GS_H

#include "HumTool.h"
#include "HumdrumFile.h"

namespace hum {

// START_MERGE

class Tool_satb2gs : public HumTool {
	public:
		         Tool_satb2gs      (void);
		        ~Tool_satb2gs      () {};

		bool     run               (HumdrumFileSet& infiles);
		bool     run               (HumdrumFile& infile);
		bool     run               (const string& indata, ostream& out);
		bool     run               (HumdrumFile& infile, ostream& out);

	protected:
		void    processFile        (HumdrumFile& infile);
		void    initialize         (void);
		void    getTrackInfo       (std::vector<std::vector<int>>& tracks,
		                            HumdrumFile& infile);

		void    printTerminatorLine(std::vector<std::vector<int>>& tracks);
		int     getNewTrackCount   (std::vector<std::vector<int>>& tracks);
		void    printRegularLine   (HumdrumFile& infile, int line,
		                            std::vector<std::vector<int>>& tracks);
		void    printSpineMergeLine(std::vector<std::vector<int>>& tracks);
		void    printSpineSplitLine(std::vector<std::vector<int>>& tracks);
		void    printHeaderLine    (HumdrumFile& infile, int line,
		                            std::vector<std::vector<int>>& tracks);
		bool    validateHeader     (HumdrumFile& infile);

};

// END_MERGE

} // end namespace hum

#endif /* _TOOL_SATB2GS_H */



