//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Apr 27 20:57:44 PDT 2024
// Last Modified: Sat Apr 27 20:57:47 PDT 2024
// Filename:      tool-vcross.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-vcross.h
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Highlight voice crossings between adjacent parts.
//

#ifndef _TOOL_VCROSS_H
#define _TOOL_VCROSS_H

#include "HumTool.h"
#include "HumdrumFile.h"

#include <ostream>
#include <string>

namespace hum {

// START_MERGE

class Tool_vcross : public HumTool {
	public:
		         Tool_vcross     (void);
		        ~Tool_vcross     () {};

		bool     run               (HumdrumFileSet& infiles);
		bool     run               (HumdrumFile& infile);
		bool     run               (const std::string& indata, std::ostream& out);
		bool     run               (HumdrumFile& infile, std::ostream& out);

	protected:
		void    processFile        (HumdrumFile& infile);
		void    initialize         (void);
		void    getMidiInfo        (std::vector<int>& midis, HTp token);
		void    compareVoices      (std::vector<HTp>& higher, std::vector<HTp>& lower);
		void    processLine        (HumdrumFile& infile, int index);

	private:
		bool m_redQ = false;
		bool m_greenQ = false;
		bool m_blueQ = false;


};

// END_MERGE

} // end namespace hum

#endif /* _TOOL_VCROSS_H */



