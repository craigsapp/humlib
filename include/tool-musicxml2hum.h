//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Aug  6 10:53:40 CEST 2016
// Last Modified: Wed Oct 26 18:25:45 PDT 2016 Renamed class
// Filename:      tool-musicxml2hum.h
// URL:           https://github.com/craigsapp/musicxml2hum/blob/master/include/tool-musicxml2hum.h
// Syntax:        C++11
// vim:           ts=3 noexpandtab
//
// Description:   Inteface to convert a MusicXml file into a Humdrum file.
//

#ifndef _TOOL_MUSICXML2HUM_H
#define _TOOL_MUSICXML2HUM_H

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

class Tool_musicxml2hum : public HumTool {
	public:
		        Tool_musicxml2hum    (void);
		       ~Tool_musicxml2hum    () {}

		bool    convertFile          (ostream& out, const char* filename);
		bool    convert              (ostream& out, xml_document& infile);
		bool    convert              (ostream& out, const char* input);
		bool    convert              (ostream& out, istream& input);

		void    setOptions           (int argc, char** argv);
		void    setOptions           (const vector<string>& argvlist);
		Options getOptionDefinitions (void);

	protected:
		void   initialize           (void);
		string getChildElementText  (xml_node root, const char* xpath);
		string getChildElementText  (xpath_node root, const char* xpath);
		string getAttributeValue    (xml_node xnode, const string& target);
		string getAttributeValue    (xpath_node xnode, const string& target);
		void   printAttributes      (xml_node node);
		bool   getPartInfo          (map<string, xml_node>& partinfo,
		                             vector<string>& partids, xml_document& doc);
		bool   stitchParts          (HumGrid& outdata,
		                             vector<string>& partids,
		                             map<string, xml_node>& partinfo,
		                             map<string, xml_node>& partcontent,
		                             vector<MxmlPart>& partdata);
		bool   getPartContent       (map<string, xml_node>& partcontent,
		                             vector<string>& partids, xml_document& doc);
		void   printPartInfo        (vector<string>& partids,
		                             map<string, xml_node>& partinfo,
		                             map<string, xml_node>& partcontent,
		                             vector<MxmlPart>& partdata);
		bool   fillPartData         (vector<MxmlPart>& partdata,
		                             const vector<string>& partids,
		                             map<string, xml_node>& partinfo,
		                             map<string, xml_node>& partcontent);
		bool   fillPartData         (MxmlPart& partdata, const string& id,
		                             xml_node partdeclaration,
		                             xml_node partcontent);
		void   appendZeroEvents     (GridMeasure* outfile,
		                             vector<SimultaneousEvents*>& nowevents,
		                             HumNum nowtime,
		                             vector<MxmlPart>& partdata);
		void   appendNonZeroEvents   (GridMeasure* outdata,
		                              vector<SimultaneousEvents*>& nowevents,
		                              HumNum nowtime,
		                              vector<MxmlPart>& partdata);
		void   addGraceLines         (GridMeasure* outdata,
		                              vector<vector<vector<vector<MxmlEvent*> > > >& notes,
		                              vector<MxmlPart>& partdata, HumNum nowtime);
		void   addEventToList        (vector<vector<vector<vector<MxmlEvent*> > > >& list, 
		                              MxmlEvent* event);
		void   addHeaderRecords      (HumdrumFile& outfile, xml_document& doc);
		void   addFooterRecords      (HumdrumFile& outfile, xml_document& doc);
		string cleanSpaces           (string& input);
		void setEditorialAccidental  (int accidental, GridSlice* slice, 
		                              int partindex, int staffindex, int voiceindex);

		bool convert          (ostream& out);
		bool convertPart      (ostream& out, const string& partname,
		                       int partindex);
		bool insertMeasure    (HumGrid& outdata, int mnum,
		                       vector<MxmlPart>& partdata,
		                       vector<int> partstaves);
		bool convertNowEvents (GridMeasure* outdata, 
		                       vector<SimultaneousEvents*>& nowevents,
		                       vector<int>& nowparts, 
		                       HumNum nowtime,
		                       vector<MxmlPart>& partdata, 
		                       vector<int>& partstaves);
		void appendNullTokens (HumdrumLine* line, MxmlPart& part);
		void appendEvent      (HumdrumLine* line, MxmlEvent* event);
		void insertExclusiveInterpretationLine(HumdrumFile& outfile,
		                       vector<MxmlPart>& partdata);
		void insertAllToken   (HumdrumFile& outfile, vector<MxmlPart>& partdata,
		                       const string& common);
		void insertSingleMeasure(HumdrumFile& outfile);
		void cleanupMeasures   (HumdrumFile& outfile,
		                        vector<HumdrumLine*> measures);

		void addClefLine       (GridMeasure* outdata, vector<vector<xml_node> >& clefs,
		                        vector<MxmlPart>& partdata, HumNum nowtime);
		void insertPartClefs   (xml_node clef, GridPart& part);
		xml_node convertClefToHumdrum(xml_node clef, HTp& token, int& staffindex);

		void addKeySigLine    (GridMeasure* outdata, vector<vector<xml_node> >& keysigs,
		                        vector<MxmlPart>& partdata, HumNum nowtime);
		void insertPartKeySigs (xml_node keysig, GridPart& part);
		xml_node convertKeySigToHumdrum(xml_node keysig, 
		                        HTp& token, int& staffindex);

		void addTimeSigLine    (GridMeasure* outdata, vector<vector<xml_node> >& timesigs,
		                        vector<MxmlPart>& partdata, HumNum nowtime);
		bool insertPartTimeSigs (xml_node timesig, GridPart& part);
		void insertPartMensurations(xml_node timesig, GridPart& part);
		bool checkForMensuration(xml_node timesig);
		xml_node convertTimeSigToHumdrum(xml_node timesig, 
		                        HTp& token, int& staffindex);
		xml_node convertMensurationToHumdrum(xml_node timesig,
		                        HTp& token, int& staffindex);

		void addEvent          (GridSlice* slice, GridMeasure* outdata, MxmlEvent* event);
		void fillEmpties       (GridPart* part, const char* string);
		void addSecondaryChordNotes (ostream& output, MxmlEvent* head, const string& recip);
		bool isInvisible       (MxmlEvent* event);
		int  addLyrics         (GridStaff* staff, MxmlEvent* event);
		int  addHarmony        (GridPart* oart, MxmlEvent* event);
		void addDynamic        (GridPart* part, MxmlEvent* event);
		void addTexts          (GridSlice* slice, GridMeasure* measure, int partindex,
		                        int staffindex, int voiceindex, MxmlEvent* event);
		void addText           (GridSlice* slice, GridMeasure* measure, int partindex, 
		                        int staffindex, int voiceindex, xml_node node);
		string getHarmonyString(xml_node hnode);
		string getDynamicString(xml_node element);
		string getDynamicsParameters(xml_node element);
		string getHairpinString(xml_node element);
		string cleanSpaces     (const string& input);
		void checkForDummyRests(MxmlMeasure* measure);
		void reindexVoices     (vector<MxmlPart>& partdata);
		void reindexMeasure    (MxmlMeasure* measure);

	public:

	static bool nodeType      (xml_node node, const char* testname);

	private:
		Options m_options;
		bool DebugQ;
		bool VoiceDebugQ;
		bool m_recipQ = false;
		bool m_stemsQ = false;
		int m_slurabove = 0;
		int m_slurbelow = 0;
		char m_hasEditorial = '\0';

		xml_node m_current_dynamic = xml_node(NULL);
		vector<xml_node> m_current_text;

};


// END_MERGE

}  // end of namespace hum

#endif /* _TOOL_MUSICXML2HUM_H */



