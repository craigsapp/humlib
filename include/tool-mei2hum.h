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
		string keysig;     // such as *k[f#]
		string midibpm;    // such as *MM120
		string tempo;      // such as *MM120
		void clear(void) {
			clef.clear();
			timesig.clear();
			keysig.clear();
			midibpm.clear();
		}
		mei_staffdef& operator=(mei_staffdef& staffdef) {
			if (this == &staffdef) {
				return *this;
			}
			clef     = staffdef.clef;
			timesig  = staffdef.timesig;
			keysig   = staffdef.keysig;
			midibpm  = staffdef.midibpm;
			return *this;
		}
		mei_staffdef(void) {
			// do nothing
		}
		mei_staffdef(const mei_staffdef& staffdef) {
			clef     = staffdef.clef;
			timesig  = staffdef.timesig;
			keysig   = staffdef.keysig;
			midibpm  = staffdef.midibpm;
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
		void minresize(int count) {
			if (count < 1) {
				return;
			} else if (count < (int)staves.size()) {
				return;
			} else {
				staves.resize(count);
			}
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
		void   fillWithStaffDefAttributes(mei_staffdef& staffinfo, xml_node element);
		HumNum parseMeasure         (HumGrid& outdata, xml_node measure, HumNum starttime);
		HumNum parseStaff           (HumGrid& outdata, xml_node staff, HumNum starttime);
		HumNum parseLayer           (HumGrid& outdata, xml_node layer, HumNum starttime);
		int    extractStaffCount    (xml_node element);
		HumNum parseRest            (HumGrid& outdata, xml_node chord, HumNum starttime);
		HumNum parseChord           (HumGrid& outdata, xml_node chord, HumNum starttime);
		HumNum parseNote            (HumGrid& outdata, xml_node note, HumNum starttime);
		HumNum getDuration          (xml_node element);
		string getHumdrumPitch      (xml_node note);
		string getHumdrumRecip      (HumNum duration, int dotcount);

	private:
		Options m_options;
		bool    m_stemsQ = false;
		bool    m_recipQ = false;

		mei_scoredef m_scoredef;    // for keeping track of key/meter/clef etc.
		int          m_staffcount;  // number of staves in score.
		HumNum       m_tupletfactor = 1;

};


// END_MERGE

}  // end of namespace hum

#endif /* _TOOL_MEI2HUM_H */



