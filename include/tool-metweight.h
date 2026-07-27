//
// Programmer:    Wolfgang Drescher <drescher.wolfgang@gmail.com>
// Creation Date: Thu Jul 16 2026
// Filename:      tool-metweight.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-metweight.h
// Syntax:        C++11; humlib
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Interface for metweight tool, which labels the metric
//                weight (strong/half-strong/weak) of note and rest attacks
//                in **kern spines; see tool-metweight.cpp for details.
//

#ifndef _TOOL_METWEIGHT_H_INCLUDED
#define _TOOL_METWEIGHT_H_INCLUDED

#include "HumTool.h"
#include "HumdrumFile.h"
#include "HumNum.h"

#include <ostream>
#include <string>
#include <vector>

namespace hum {

// START_MERGE

class Tool_metweight : public HumTool {

	public:
		     Tool_metweight  (void);
		     ~Tool_metweight () {};

		bool run     (HumdrumFileSet& infiles);
		bool run     (HumdrumFile& infile);
		bool run     (const std::string& indata, std::ostream& out);
		bool run     (HumdrumFile& infile, std::ostream& out);

	protected:
		void        initialize       (void);
		void        processFile      (HumdrumFile& infile);
		void        addCompositeSpine(HumdrumFile& infile);
		void        getVoices        (std::vector<HTp>& voices, HumdrumFile& infile);
		void        fillVoiceResults (std::vector<std::vector<int>>& results,
		                             HumdrumFile& infile,
		                             const std::vector<HTp>& voices);
		void        formatWeights    (std::vector<std::string>& tokens,
		                             const std::vector<int>& weightClasses,
		                             int format);
		int         getTokenWeightClass(HumdrumFile& infile, int line, int track);
		HTp         getMeterToken    (HumdrumFile& infile, int line);
		std::string formatWeightClass(int weightClass, int format);
		int         getWeightClass   (int top, int bot, HumNum beat);

		// Metric weight classes (in order from strongest to weakest):
		enum {
			WEIGHT_STRONG       = 1,
			WEIGHT_HALF_STRONG  = 2,
			WEIGHT_WEAK         = 3,
			WEIGHT_UNCLASSIFIED = 4
		};

		// Token representations of a weight class (see formatWeightClass):
		enum {
			FORMAT_ABBREVIATION, // abbreviations: s/hs/w/u (default)
			FORMAT_FULL,         // -f option: strong/half-strong/weak/unclassified
			FORMAT_INTEGER,      // -i option: the weight class values 1/2/3/4
			FORMAT_COLOR         // -c option: a color for each weight class
		};

	private:
		bool m_fullQ      = false; // -f option: print full text labels instead of abbreviations
		bool m_integerQ   = false; // -i option: print integer rank labels instead of abbreviations
		bool m_cdataQ     = false; // -x option: label the spine **cdata-metweight instead of **metweight
		bool m_nullQ      = false; // -n option: always use the null token . for unclassified positions
		bool m_tiedQ      = false; // -t option: label secondary tied notes instead of giving them the null token .
		bool m_compositeQ = false; // -C option: add a **kern-comp spine below the voices and label only that spine
		bool m_colorQ     = false; // -c option: add a **color spine after each **metweight spine

		std::string       m_kernTracks  = ""; // used with -k option
		std::string       m_spineTracks = ""; // used with -s option
		std::vector<bool> m_selectedKernSpines; // used with -k and -s option

};

// END_MERGE

} // end namespace hum

#endif /* _TOOL_METWEIGHT_H_INCLUDED */
