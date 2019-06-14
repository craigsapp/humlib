//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Wed Sep 13 14:55:58 PDT 2017
// Last Modified: Thu Sep 21 14:04:12 PDT 2017
// Filename:      tool-mei2hum.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-mei2hum.h
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Inteface to convert an MEI file into a Humdrum file.
//
// See: http://www.aosabook.org/en/posa/parsing-xml-at-the-speed-of-light.html
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

class mei_staffDef {
	public:
		HumNum timestamp;
		string clef;       // such as *clefG2
		string timesig;    // such as *M4/4
		string keysig;     // such as *k[f#]
		string midibpm;    // such as *MM120
		string transpose;  // such as *Trd-1c-2
		int base40 = 0;    // used for transposing to C score
		string label;      // such as *I"violin 1
		string labelabbr;  // such as *I'v1

		void clear(void) {
			clef.clear();
			timesig.clear();
			keysig.clear();
			midibpm.clear();
			transpose.clear();
			base40 = 0;
			label.clear();
			labelabbr.clear();
		}
		mei_staffDef& operator=(mei_staffDef& staffDef) {
			if (this == &staffDef) {
				return *this;
			}
			clef       = staffDef.clef;
			timesig    = staffDef.timesig;
			keysig     = staffDef.keysig;
			midibpm    = staffDef.midibpm;
			transpose  = staffDef.transpose;
			base40     = staffDef.base40;
			label      = staffDef.label;
			labelabbr  = staffDef.labelabbr;
			return *this;
		}
		mei_staffDef(void) {
			// do nothing
		}
		mei_staffDef(const mei_staffDef& staffDef) {
			clef       = staffDef.clef;
			timesig    = staffDef.timesig;
			keysig     = staffDef.keysig;
			midibpm    = staffDef.midibpm;
			transpose  = staffDef.transpose;
			base40     = staffDef.base40;
			label      = staffDef.label;
			labelabbr  = staffDef.labelabbr;
		}
};


class mei_scoreDef {
	public:
		mei_staffDef global;
		vector<mei_staffDef> staves;
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


class hairpin_info {
	public:
		xml_node hairpin;
		GridMeasure *gm = NULL;
		int mindex = 0;
};


class grace_info {
	public:
		xml_node node; // note or chord
		string beamprefix;
		string beampostfix;
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
		HumNum parseScore           (xml_node score, HumNum starttime);
		void   getChildrenVector    (vector<xml_node>& children, xml_node parent);
		void   parseScoreDef        (xml_node scoreDef, HumNum starttime);
		void   parseSectionScoreDef (xml_node scoreDef, HumNum starttime);
		void   processPgHead        (xml_node pgHead, HumNum starttime);
		void   processPgFoot        (xml_node pgFoot, HumNum starttime);
		HumNum parseSection         (xml_node section, HumNum starttime);
		HumNum parseApp             (xml_node app, HumNum starttime);
		HumNum parseLem             (xml_node lem, HumNum starttime);
		HumNum parseRdg             (xml_node rdg, HumNum starttime);
		void   parseStaffGrp        (xml_node staffGrp, HumNum starttime);
		void   parseStaffDef        (xml_node staffDef, HumNum starttime);
		void   fillWithStaffDefAttributes(mei_staffDef& staffinfo, xml_node element);
		HumNum parseMeasure         (xml_node measure, HumNum starttime);
		HumNum parseStaff           (xml_node staff, HumNum starttime);
		void   parseReh             (xml_node reh, HumNum starttime);
		HumNum parseLayer           (xml_node layer, HumNum starttime, vector<bool>& layerPresent);
		int    extractStaffCount    (xml_node element);
		HumNum parseRest            (xml_node chord, HumNum starttime);
		HumNum parseMRest           (xml_node mrest, HumNum starttime);
		HumNum parseChord           (xml_node chord, HumNum starttime, int gracenumber);
		HumNum parseNote            (xml_node note, xml_node chord, string& output, HumNum starttime, int gracenumber);
		HumNum parseBeam            (xml_node note, HumNum starttime);
		HumNum parseTuplet          (xml_node note, HumNum starttime);
		void   parseClef            (xml_node clef, HumNum starttime);
		void   parseDynam           (xml_node dynam, HumNum starttime);
		void   parseHarm            (xml_node harm, HumNum starttime);
		void   parseTempo           (xml_node tempo, HumNum starttime);
		void   parseDir             (xml_node dir, HumNum starttime);
		HumNum getDuration          (xml_node element);
		string getHumdrumPitch      (xml_node note, vector<xml_node>& children);
		string getHumdrumRecip      (HumNum duration, int dotcount);
		void   buildIdLinkMap       (xml_document& doc);
		void   processNodeStartLinks(string& output, xml_node node,
		                             vector<xml_node>& nodelist);
		void   processNodeStopLinks(string& output, xml_node node,
		                             vector<xml_node>& nodelist);
		void   processPreliminaryLinkedNodes(xml_node node);
		void   processNodeStartLinks2(xml_node node, vector<xml_node>& nodelist);
		void   parseFermata         (string& output, xml_node node, xml_node fermata);
		void   parseSlurStart       (string& output, xml_node node, xml_node slur);
		void   parseSlurStop        (string& output, xml_node node, xml_node slur);
		void   parseTieStart        (string& output, xml_node node, xml_node tie);
		void   parseTieStop         (string& output, xml_node node, xml_node tie);
		void   parseArpeg           (string& output, xml_node node, xml_node arpeg);
		void   parseTrill           (string& output, xml_node node, xml_node trill);
		void   parseTupletSpanStart (xml_node node, xml_node tupletSpan);
		void   parseTupletSpanStop  (string& output, xml_node node, xml_node tupletSpan);
		void   parseSb              (xml_node sb, HumNum starttime);
		void   parsePb              (xml_node pb, HumNum starttime);
		void   processLinkedNodes   (string& output, xml_node node);
		int    getDotCount          (xml_node node);
		void   processFermataAttribute(string& output, xml_node node);
		string getNoteArticulations (xml_node note, xml_node chord);
		string getHumdrumArticulation(const string& tag, const string& humdrum,
		                              const string& attribute_artic,
		                              vector<xml_node>& element_artic,
		                              const string& chord_attribute_artic,
		                              vector<xml_node>& chord_element_artic);
		string setPlacement          (const string& placement);
		void   addFooterRecords      (HumdrumFile& outfile, xml_document& doc);
		void   addExtMetaRecords     (HumdrumFile& outfile, xml_document& doc);
		void   addHeaderRecords      (HumdrumFile& outfile, xml_document& doc);
		void   parseVerse            (xml_node verse, GridStaff* staff);
		string parseSyl              (xml_node syl);
		void   parseSylAttribute     (const string& attsyl, GridStaff* staff);
		void   reportVerseNumber     (int pmax, int staffindex);
		string getEditorialAccidental(vector<xml_node>& children);
		string getCautionaryAccidental(vector<xml_node>& children);
		string makeHumdrumClef       (const string& shape,
		                              const string& line,
		                              const string& clefdis,
		                              const string& clefdisplace);
		string cleanDirText          (const string& input);
		string cleanWhiteSpace       (const string& input);
		string cleanReferenceRecordText(const string& input);
		string cleanVerseText        (const string& input);
		bool   beamIsValid           (vector<xml_node>& beamlist);
		bool   beamIsGrace           (vector<xml_node>& beamlist);
		void   parseHairpin          (xml_node hairpin, HumNum starttime);
		void   processHairpins       (void);
		void   processHairpin        (hairpin_info& info);
		void   processGraceNotes     (HumNum timestamp);
		string prepareSystemDecoration(xml_node scoreDef);
		void   getRecursiveSDString  (string& output, xml_node current);
		void   parseBareSyl          (xml_node syl, GridStaff* staff);
		string getChildAccidGes      (vector<xml_node>& children);
		string getChildAccidVis      (vector<xml_node>& children);

		// static functions
		static string accidToKern(const string& accid);

	private:
		Options        m_options;
		bool           m_stemsQ = false;
		bool           m_recipQ = false;
		bool           m_placeQ = false;

		mei_scoreDef   m_scoreDef;    // for keeping track of key/meter/clef etc.
		int            m_staffcount;  // number of staves in score.
		HumNum         m_tupletfactor = 1;
		HumGrid        m_outdata;
		int            m_currentLayer = 0;
		int            m_currentStaff = 0;
		int            m_maxStaffInFile = 0; // valid after parsing staves in first measure
		int            m_currentMeasure = -1;
		vector<int>    m_currentMeterUnit;
		string         m_beamPrefix;
		string         m_beamPostfix;
		bool           m_aboveQ = false;
		bool           m_belowQ = false;
		bool           m_editorialAccidentalQ = false;
		string         m_appLabel;
		string         m_systemDecoration;

		vector<int>    m_maxverse;
		vector<HumNum> m_measureDuration;
		vector<bool>   m_hasDynamics;
		vector<bool>   m_hasHarm;
		const int      m_maxstaff = 1000;

		bool           m_fermata = false;     // set priority of note/fermata over note@fermata
		vector<grace_info> m_gracenotes;      // buffer for storing grace notes
		HumNum			m_gracetime = 0;       // performance time of buffered grace notes

		vector<hairpin_info> m_hairpins;

		map<string, vector<xml_node>> m_startlinks;
		map<string, vector<xml_node>> m_stoplinks;

};


// END_MERGE

}  // end of namespace hum

#endif /* _TOOL_MEI2HUM_H */



