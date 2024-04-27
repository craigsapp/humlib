//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Apr  1 01:06:00 PDT 2024
// Last Modified: Mon Apr  1 23:38:33 PDT 2024
// Filename:      tool-addkey.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-addkey.h
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Insert key designation from -k option or from !!!key: reference
//                record.
//

#ifndef _TOOL_ADDKEY_H
#define _TOOL_ADDKEY_H

#include "HumTool.h"
#include "HumdrumFile.h"

#include <ostream>
#include <string>

namespace hum {

// START_MERGE

class Tool_addkey : public HumTool {
	public:
		         Tool_addkey     (void);
		        ~Tool_addkey     () {};

		bool     run               (HumdrumFileSet& infiles);
		bool     run               (HumdrumFile& infile);
		bool     run               (const std::string& indata, std::ostream& out);
		bool     run               (HumdrumFile& infile, std::ostream& out);

	protected:
		void    processFile        (HumdrumFile& infile);
		void    initialize         (void);
		void    getLineIndexes     (HumdrumFile& infile);
		void    insertReferenceKey (HumdrumFile& infile);
		void    addInputKey        (HumdrumFile& infile);
		void    insertKeyDesig     (HumdrumFile& infile, const std::string& keyDesig);
		void    printKeyDesig      (HumdrumFile& infile, int index, const std::string& desig, int direction);

	private:
		std::string m_key;
		bool        m_keyQ           = false;
		bool        m_addKeyRefQ     = false;

		int         m_exinterpIndex  = -1;
		int         m_refKeyIndex    = -1;
		int         m_keyDesigIndex  = -1;
		int         m_keySigIndex    = -1;
		int         m_dataStartIndex = -1;

};

// END_MERGE

} // end namespace hum

#endif /* _TOOL_ADDKEY_H */



