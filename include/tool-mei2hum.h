//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Wed Sep 13 14:55:58 PDT 2017
// Last Modified: Wed Sep 13 14:56:01 PDT 2017
// Filename:      tool-mei2hum.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-mei2hum.h
// Syntax:        C++11
// vim:           ts=3 noexpandtab
//
// Description:   Inteface to convert an MEI file into a Humdrum file.
//

#ifndef _TOOL_MEI2HUM_H
#define _TOOL_MEI2HUM_H

#define _USE_HUMLIB_OPTIONS_

#include "Options.h"
#include "HumTool.h"

#include "pugiconfig.hpp"
#include "pugixml.hpp"

#include "MxmlPart.h"
#include "MxmlMeasure.h"
#include "MxmlEvent.h"
#include "HumGrid.h"


using namespace std;
using namespace pugi;

namespace hum {

// START_MERGE

class mei_staffdef {
	public:
		HumNum timestamp;
		string clef;       // such as *clefG2
		string timesig;    // such as *M4/4
		string tempo;      // such as *MM120
		void clear(void) {
			clef.clear();
			timesig.clear();
		}
};

class mei_scoredef {
	public:
		mei_staffdef global;
		vector<mei_staffdef> staves;
		void clear(void) {
			global.clear();
			staves.clear(); // or clear the contents of each staff...
		}
};

class Tool_mei2hum : public HumTool {
	public:
		        Tool_mei2hum    (void);
		       ~Tool_mei2hum    () {}

		bool    convertFile          (ostream& out, const char* filename);
		bool    convert              (ostream& out, xml_document& infile);
		bool    convert              (ostream& out, const char* input);
		bool    convert              (ostream& out, istream& input);

		void    setOptions           (int argc, char** argv);
		void    setOptions           (const vector<string>& argvlist);
		Options getOptionDefinitions (void);

	protected:
		void   initialize           (void);
		HumNum parseScore           (HumGrid& outdata, xml_node score,
		                             HumNum starttime);
		void   getChildrenVector    (vector<xml_node>& children, xml_node parent);
		void   parseScoreDef        (HumGrid& outdata, xml_node item,
		                             HumNum starttime);
		HumNum parseSection         (HumGrid& outdata, xml_node item,
		                             HumNum starttime);
		void   processStaffGrp      (HumGrid& outdata, xml_node item,
		                             HumNum starttime);
		void   processStaffDef      (HumGrid& outdata, xml_node item,
		                             HumNum starttime);

	private:
		Options m_options;
		bool    m_stemsQ = false;
		bool    m_recipQ = false;

		mei_scoredef m_scoredef;

};


// END_MERGE

}  // end of namespace hum

#endif /* _TOOL_MEI2HUM_H */



