//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Aug 13 06:18:14 PDT 2018
// Last Modified: Mon Aug 13 06:18:16 PDT 2018
// Filename:      tool-periodicity.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-periodicity.h
// Syntax:        C++11; humlib
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Interface for periodicity (polyrhythmic) tool.
//

#ifndef _TOOL_PERODICITY_H_INCLUDED
#define _TOOL_PERODICITY_H_INCLUDED

#include "HumTool.h"
#include "HumdrumFile.h"

#include <iostream>
#include <string>
#include <vector>

namespace hum {

// START_MERGE



class Tool_periodicity : public HumTool {
	public:
		         Tool_periodicity   (void);
		        ~Tool_periodicity   () {};

		bool     run                (HumdrumFileSet& infiles);
		bool     run                (HumdrumFile& infile);
		bool     run                (const std::string& indata, std::ostream& out);
		bool     run                (HumdrumFile& infile, std::ostream& out);

	protected:
		void     initialize         (HumdrumFile& infile);
		void     processFile        (HumdrumFile& infile);
		void     fillAttackGrids    (HumdrumFile& infile, std::vector<std::vector<double>>& grids, HumNum minrhy);
		void     printAttackGrid    (std::ostream& out, HumdrumFile& infile, std::vector<std::vector<double>>& grids, HumNum minrhy);
		void     doAnalysis         (std::vector<std::vector<double>>& analysis, int level, std::vector<double>& grid);
		void     doPeriodicityAnalysis(std::vector<std::vector<double>> & analysis, std::vector<double>& grid, HumNum minrhy);
		void     printPeriodicityAnalysis(std::ostream& out, std::vector<std::vector<double>>& analysis);
		void     printSvgAnalysis(std::ostream& out, std::vector<std::vector<double>>& analysis, HumNum minrhy);
		void     getColorMapping(double input, double& hue, double& saturation, double& lightness);

	private:

};

// END_MERGE

} // end namespace hum

#endif /* _TOOL_PERODICITY_H_INCLUDED */



