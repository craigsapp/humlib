//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Aug 12 10:58:43 PDT 2024
// Last Modified: Sun Aug 18 00:19:45 PDT 2024
// Filename:      tool-tandeminfo.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-tandeminfo.h
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Analyze tandem interpretations.
//

#ifndef _TOOL_TANDEMINFO_H
#define _TOOL_TANDEMINFO_H

#include "HumTool.h"
#include "HumdrumFile.h"

#include <ostream>
#include <string>
#include <utility>
#include <vector>

namespace hum {

// START_MERGE

class Tool_tandeminfo : public HumTool {
	public:
	class Entry {
		public:
			HTp token = NULL;
			std::string description;
	};

		         Tool_tandeminfo   (void);
		        ~Tool_tandeminfo   () {};

		bool     run               (HumdrumFileSet& infiles);
		bool     run               (HumdrumFile& infile);
		bool     run               (const std::string& indata, std::ostream& out);
		bool     run               (HumdrumFile& infile, std::ostream& out);


	protected:
		void     initialize        (void);
		void     processFile       (HumdrumFile& infile);
		void     printEntries      (HumdrumFile& infile);
		void     printEntriesHtml  (HumdrumFile& infile);
		void     printEntriesText  (HumdrumFile& infile);

		std::string getDescription         (HTp token);
		std::string checkForKeySignature   (const std::string& tok);
		std::string checkForKeyDesignation (const std::string& tok);
		std::string checkForInstrumentInfo (const std::string& tok);
		std::string checkForLabelInfo      (const std::string& tok);
		std::string checkForTimeSignature  (const std::string& tok);
		std::string checkForMeter          (const std::string& tok);
		std::string checkForTempoMarking   (const std::string& tok);
		std::string checkForClef           (const std::string& tok);
		std::string checkForStaffPartGroup (const std::string& tok);
		std::string checkForTuplet         (const std::string& tok);
		std::string checkForHands          (const std::string& tok);
		std::string checkForPosition       (const std::string& tok);
		std::string checkForCue            (const std::string& tok);
		std::string checkForFlip           (const std::string& tok);
		std::string checkForTremolo        (const std::string& tok);
		std::string checkForOttava         (const std::string& tok);
		std::string checkForPedal          (const std::string& tok);
		std::string checkForBracket        (const std::string& tok);
		std::string checkForRscale         (const std::string& tok);
		std::string checkForTimebase       (const std::string& tok);
		std::string checkForTransposition  (const std::string& tok);
		std::string checkForGrp            (const std::string& tok);
		std::string checkForStria          (const std::string& tok);
		std::string checkForFont           (const std::string& tok);
		std::string checkForVerseLabels    (const std::string& tok);
		std::string checkForLanguage       (const std::string& tok);
		std::string checkForStemInfo       (const std::string& tok);
		std::string checkForXywh           (const std::string& tok);
		std::string checkForCustos         (const std::string& tok);
		std::string checkForTextInterps    (const std::string& tok);
		std::string checkForRep            (const std::string& tok);
		std::string checkForPline          (const std::string& tok);
		std::string checkForTacet          (const std::string& tok);
		std::string checkForFb             (const std::string& tok);
		std::string checkForColor          (const std::string& tok);
		std::string checkForThru           (const std::string& tok);

	private:
		bool m_exclusiveQ   = true;   // used with -X option (don't print exclusive interpretation)
		bool m_unknownQ     = false;  // used with -u option (print only unknown tandem interpretations)
		bool m_filenameQ    = false;  // used with -f option (print only unknown tandem interpretations)
		bool m_descriptionQ = false;  // used with -m option (print description of interpretation)
		bool m_locationQ    = false;  // used with -l option (print location of interpretation in file)
		bool m_zeroQ        = false;  // used with -z option (location address by 0-index)
		bool m_tableQ       = false;  // used with -t option (display results as inline HTML table)
		bool m_closeQ       = false;  // used with --close option (HTML details closed by default)
		bool m_sortQ        = false;  // used with -s (sort entries alphabetically by tandem interpretation)

		std::string m_unknown = "unknown";

		std::vector<Tool_tandeminfo::Entry> m_entries;
};

// END_MERGE

} // end namespace hum

#endif /* _TOOL_TANDEMINFO_H */



