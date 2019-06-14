//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Aug 17 02:39:28 PDT 2015
// Last Modified: Mon Aug 17 02:39:32 PDT 2015
// Filename:      HumdrumFile.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/HumdrumFile.h
// Syntax:        C++11; humlib
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Placeholder class to serve as interface to HumdrumFileBase,
//                HumdrumFileStructure and HumdrumFileContent.
//

#ifndef _HUMDRUMFILE_H_INCLUDED
#define _HUMDRUMFILE_H_INCLUDED

#include "HumdrumFileContent.h"

#include <iostream>
#include <string>

namespace hum {

// START_MERGE

#ifndef HUMDRUMFILE_PARENT
	#define HUMDRUMFILE_PARENT HumdrumFileContent
#endif

class HumdrumFile : public HUMDRUMFILE_PARENT {
	public:
		              HumdrumFile          (void);
		              HumdrumFile          (const std::string& filename);
		              HumdrumFile          (std::istream& filename);
		             ~HumdrumFile          ();

		std::ostream& printXml             (std::ostream& out = std::cout, int level = 0,
		                                    const std::string& indent = "\t");
		std::ostream& printXmlParameterInfo(std::ostream& out, int level,
		                                    const std::string& indent);
};


// END_MERGE

} // end namespace hum

#endif /* _HUMDRUMFILE_H_INCLUDED */



