//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Fri Feb  5 17:31:36 PST 2021
// Last Modified: Fri Feb  5 17:31:39 PST 2021
// Filename:      tool-autoaccid.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-autoaccid.h
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Analyze visible/hidden accidentals for notes (based on key signature
//                and previous notes in the measure.
//

#ifndef _TOOL_AUTOACCID_H
#define _TOOL_AUTOACCID_H

#include "HumTool.h"
#include "HumdrumFile.h"

namespace hum {

// START_MERGE

class Tool_autoaccid : public HumTool {
	public:
		         Tool_autoaccid    (void);
		        ~Tool_autoaccid    () {};

		bool     run               (HumdrumFileSet& infiles);
		bool     run               (HumdrumFile& infile);
		bool     run               (const string& indata, ostream& out);
		bool     run               (HumdrumFile& infile, ostream& out);

	protected:
		void    processFile        (HumdrumFile& infile);
		void    initialize         (void);
		void    addAccidentalInfo  (HTp token);
		void    removeAccidentalQualifications(HumdrumFile& infile);
		void    addAccidentalQualifications(HumdrumFile& infile);
		string  setVisualState     (const string& input, bool state);

	private:
		bool    m_visualQ;
		bool    m_hiddenQ;
		bool    m_removeQ;
		bool    m_cautionQ;

};

// END_MERGE

} // end namespace hum

#endif /* _TOOL_AUTOACCID_H */



