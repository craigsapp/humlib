//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Wed Sep 25 19:16:44 PDT 2019
// Last Modified: Wed Sep 25 19:16:53 PDT 2019
// Filename:      tool-md2hum.h
// URL:           https://github.com/craigsapp/md2hum/blob/master/include/tool-md2hum.h
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Inteface to convert a MuseData file into a Humdrum file.
//

#ifndef _TOOL_MD2HUM_H
#define _TOOL_MD2HUM_H

#define _USE_HUMLIB_OPTIONS_

#include "Options.h"
#include "MuseDataSet.h"
#include "HumTool.h"
#include "HumGrid.h"

#include <string>
#include <vector>

namespace hum {

// START_MERGE

class Tool_md2hum : public HumTool {
	public:
		        Tool_md2hum          (void);
		       ~Tool_md2hum          () {}

		bool    convertFile          (ostream& out, const string& filename);
		bool    convertString        (ostream& out, const string& input);
		bool    convert              (ostream& out, MuseDataSet& mds);
		bool    convert              (ostream& out, istream& input);

		void    setOptions           (int argc, char** argv);
		void    setOptions           (const std::vector<std::string>& argvlist);
		Options getOptionDefinitions (void);

	protected:
		void    initialize           (void);
		void    printKernInfo        (ostream& out, MuseRecord& mr, int tpq);

	private:
		Options m_options;
		bool    m_stemsQ = false;    // used with -s option
		bool    m_recipQ = false;    // used with -r option

};


// END_MERGE

}  // end of namespace hum

#endif /* _TOOL_MD2HUM_H */


