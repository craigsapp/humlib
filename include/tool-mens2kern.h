//
// Programmer:    Martha Thomae
// Programmer:    Craig Stuart Sapp
// Creation Date: Wed Dec 16 11:16:33 PST 2020
// Last Modified: Sun Sep 24 17:17:36 PDT 2023
// Filename:      tool-mens2kern.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-mens2kern.h
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Convert **mens to **kern.
//

#ifndef _TOOL_MENS2KERN_H
#define _TOOL_MENS2KERN_H

#include "HumTool.h"
#include "HumdrumFile.h"

#include <ostream>
#include <string>
#include <vector>

namespace hum {

// START_MERGE

class Tool_mens2kern : public HumTool {
	public:
		         Tool_mens2kern      (void);
		        ~Tool_mens2kern      () {};

		bool     run                 (HumdrumFileSet& infiles);
		bool     run                 (HumdrumFile& infile);
		bool     run                 (const std::string& indata, std::ostream& out);
		bool     run                 (HumdrumFile& infile, std::ostream& out);

	protected:
		void     processFile         (HumdrumFile& infile);
		void     initialize          (void);
		void     processMelody       (std::vector<HTp>& melody);
		std::string mens2kernRhythm  (const std::string& rhythm,
		                              bool altera,  bool perfecta,
		                              bool imperfecta, int maxima_def, int longa_def,
		                              int brevis_def, int semibrevis_def);
		void     getMensuralInfo     (HTp token, int& maximodus, int& modus,
		                              int& tempus, int& prolatio);
		std::string getClefConversion(HTp token);

	private:
		bool     m_debugQ;


};

// END_MERGE

} // end namespace hum

#endif /* _TOOL_MENS2KERN_H */



