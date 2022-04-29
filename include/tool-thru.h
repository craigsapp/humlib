//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Thu Apr 28 20:34:58 PDT 2022
// Last Modified: Thu Apr 28 20:35:01 PDT 2022
// Filename:      tool-thru.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-thru.h
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   C++ implementation of the Humdrum Toolkit thru command.
//

#ifndef _TOOL_THRU_H
#define _TOOL_THRU_H

#include "HumTool.h"
#include "HumdrumFile.h"

namespace hum {

// START_MERGE

class Tool_thru : public HumTool {
	public:
		         Tool_thru         (void);
		        ~Tool_thru         () {};

		bool     run               (HumdrumFileSet& infiles);
		bool     run               (HumdrumFile& infile);
		bool     run               (const string& indata, ostream& out);
		bool     run               (HumdrumFile& infile, ostream& out);

	protected:
		void      processFile         (HumdrumFile& infile);
		void      initialize          (void);

		void      checkOptions        (Options& opts, int argc, char* argv[]);
		void      example             (void);
		void      processData         (HumdrumFile& infile);
		void      usage               (const char* command);
		void      getLabelSequence    (vector<string>& labelsequence,
		                               const string& astring);
		int       getLabelIndex       (vector<string>& labels, string& key);
		void      printLabelList      (HumdrumFile& infile);
		void      printLabelInfo      (HumdrumFile& infile);
		int       getBarline          (HumdrumFile& infile, int line);
		int       adjustFirstBarline  (HumdrumFile& infile);

	private:
		bool      m_listQ = false;    // used with -l option
		bool      m_infoQ = false;    // used with -i option
		bool      m_keepQ = false;    // used with -k option
		bool      m_quietQ = false;   // used with -q option
		string    m_variation = "";   // used with -v option
		string    m_realization = ""; // used with -r option

};

// END_MERGE

} // end namespace hum

#endif /* _TOOL_THRU_H */



