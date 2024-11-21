//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sun Oct 13 11:42:59 PDT 2019
// Last Modified: Thu May 13 09:42:21 PDT 2021
// Filename:      tool-shed.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-shed.h
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Sort data spines in a Humdrum file.
//

#ifndef _TOOL_SHED_H
#define _TOOL_SHED_H

#include "HumTool.h"
#include "HumdrumFile.h"

#include <ostream>
#include <string>
#include <vector>

namespace hum {

// START_MERGE

class Tool_shed : public HumTool {
	public:
		         Tool_shed       (void);
		        ~Tool_shed       () {};

		bool     run               (HumdrumFileSet& infiles);
		bool     run               (HumdrumFile& infile);
		bool     run               (const std::string& indata, std::ostream& out);
		bool     run               (HumdrumFile& infile, std::ostream& out);

	protected:
		void    processFile                      (HumdrumFile& infile);
		void    processExpression                (HumdrumFile& infile);
		void    searchAndReplaceInterpretation   (HumdrumFile& infile);
		void    searchAndReplaceExinterp         (HumdrumFile& infile);
		void    searchAndReplaceData             (HumdrumFile& infile);
		void    searchAndReplaceBarline          (HumdrumFile& infile);
		void    searchAndReplaceLocalComment     (HumdrumFile& infile);
		void    searchAndReplaceGlobalComment    (HumdrumFile& infile);
		void    searchAndReplaceReferenceRecords (HumdrumFile& infile);
		void    searchAndReplaceReferenceKeys    (HumdrumFile& infile);
		void    searchAndReplaceReferenceValues  (HumdrumFile& infile);

		void    initialize         (void);
		void    initializeSegment  (HumdrumFile& infile);
		bool    isValid            (HTp token);
		bool    isValidDataType    (HTp token);
		bool    isValidSpine       (HTp token);
		std::vector<std::string> addToExInterpList(void);
		void    parseExpression    (const std::string& value);
		void    prepareSearch      (int index);
		std::string getExInterp    (const std::string& value);

	private:
		std::vector<std::string> m_searches;  // search strings
		std::vector<std::string> m_replaces;  // replace strings
		std::vector<std::string> m_options;   // search options

		std::string m_search;
		std::string m_replace;
		std::string m_option;

		bool m_data           = true;  // process data
		bool m_barline        = false; // process barlines
		bool m_exinterp       = false; // process exclusive interpretations
		bool m_interpretation = false; // process interpretations
		bool m_localcomment   = false; // process local comments
		bool m_globalcomment  = false; // process global comments
		bool m_reference      = false; // process reference records
		bool m_referencekey   = false; // process reference records keys
		bool m_referencevalue = false; // process reference records values
		std::string m_xInterp; // used with -x option
		std::string m_yInterp; // used with -y option
		std::string m_zInterp; // used with -z option

		bool m_modified       = false;

		// list of exclusive interpretations to process
		std::vector<std::string> m_exinterps;
		std::string m_exclusion;

		std::vector<bool> m_spines; // usar with -s option
		std::string m_grepoptions;

};

// END_MERGE

} // end namespace hum

#endif /* _TOOL_SHED_H */



