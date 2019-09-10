//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Sep  7 20:13:13 PDT 2019
// Last Modified: Sat Sep  7 20:13:15 PDT 2019
// Filename:      tool-pccount.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-pccount.h
// Syntax:        C++11; humlib
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Count pitch-classes by voice and all voices.
//

#ifndef _TOOL_PCCOUNT_H_INCLUDED
#define _TOOL_PCCOUNT_H_INCLUDED

#include "HumTool.h"
#include "HumdrumFile.h"
#include <map>
#include <vector>

namespace hum {

// START_MERGE


class Tool_pccount : public HumTool {
	public:
		      Tool_pccount              (void);
		     ~Tool_pccount              () {};

		bool  run                       (HumdrumFileSet& infiles);
		bool  run                       (HumdrumFile& infile);
		bool  run                       (const string& indata, ostream& out);
		bool  run                       (HumdrumFile& infile, ostream& out);
 
	protected:
		void   initialize               (HumdrumFile& infile);
		void   processFile              (HumdrumFile& infile);
		void   initializePartInfo       (HumdrumFile& infile);
		void   addCounts                (HTp sstart, HTp send);
		void   countPitches             (HumdrumFile& infile);
		void   printHumdrumTable        (void);
		void   printPitchClassList      (void);
		void   printVegaLiteJsonTemplate(const std::string& datavariable, HumdrumFile& infile);
		void   printVegaLiteJsonData    (void);
		void   printVoiceList           (void);
		void   printReverseVoiceList    (void);
		void   printColorList           (void);
		std::string getPitchClassString (int b40);
		void   printVegaLiteScript      (const string& jsonvar,
		                                 const string& target,
		                                 const string& datavar,
		                                 HumdrumFile& infile);
		void   printVegaLiteHtml        (const string& jsonvar,
		                                 const string& target,
		                                 const string& datavar,
		                                 HumdrumFile& infile);
		void   printVegaLitePage        (const string& jsonvar,
		                                 const string& target,
		                                 const string& datavar,
		                                 HumdrumFile& infile);
		std::string getFinal            (HumdrumFile& infile);
		double  getPercent              (const string& pitchclass);
		void    setFactorMaximum        (void);
		void    setFactorNormalize      (void);

	private:
		std::vector<int>               m_rkern;
		std::vector<int>               m_parttracks;
		std::vector<std::string>       m_names;
		std::vector<std::string>       m_abbreviations;
		std::vector<std::vector<double>> m_counts;
		bool m_attack       = false;
		bool m_full         = false;
		bool m_doublefull   = false;
		bool m_normalize    = false;
		bool m_maximum      = false;
		bool m_template     = false;
		bool m_data         = false;
		bool m_script       = false;
		bool m_html         = false;
		bool m_page         = false;
		int  m_width        = 500;
		double m_ratio      = 0.67;
		bool m_key          = true;
		double m_factor     = 1.0;
		std::string m_title = "";
		std::string m_id    = "id";
		std::map<std::string, std::string> m_vcolor;

};


// END_MERGE

} // end namespace hum

#endif /* _TOOL_PCCOUNT_H_INCLUDED */



