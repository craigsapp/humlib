//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat May  5 21:03:59 PDT 2018
// Last Modified: Sat May  5 21:04:01 PDT 2018
// Filename:      tool-kern2mens.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-kern2mens.h
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Convert **kern data into **mens data.  The conversion is
//                approximate and meant as data entry for further editing.
//

#ifndef _TOOL_KERN2MENS_H
#define _TOOL_KERN2MENS_H

#include "HumTool.h"
#include "HumdrumFile.h"
#include "NoteGrid.h"

namespace hum {

// START_MERGE

class Tool_kern2mens : public HumTool {
	public:
		         Tool_kern2mens           (void);
		        ~Tool_kern2mens           () {};

		bool     run                      (HumdrumFileSet& infiles);
		bool     run                      (HumdrumFile& infile);
		bool     run                      (const string& indata, ostream& out);
		bool     run                      (HumdrumFile& infile, ostream& out);

	protected:
		void        convertToMens         (HumdrumFile& infile);
		std::string convertKernTokenToMens(HTp token);
		void        printBarline          (HumdrumFile& infile, int line);
		std::string getClefConversion     (HTp token);
		void        storeKernEditorialAccidental(HumdrumFile& infile);
		void        addVerovioStyling     (HumdrumFile& infile);

	private:
		bool        m_numbersQ   = true; // used with -N option
		bool        m_measuresQ  = true; // used with -M option
		bool        m_invisibleQ = true; // used with -I option
		bool        m_doublebarQ = true; // used with -D option
		bool        m_noverovioQ = false; // used with -V option
		std::string m_clef;              // used with -c option

		std::string m_kernEditorialAccidental;  // used with !!!RDF**kern:
		int         m_kernEdAccLineIndex = -1;
		std::string m_mensEdAccLine;

};

// END_MERGE

} // end namespace hum

#endif /* _TOOL_KERN2MENS_H */



