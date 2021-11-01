//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Aug  6 10:53:40 CEST 2016
// Last Modified: Wed Oct 26 18:25:45 PDT 2016 Renamed class
// Filename:      tool-musicxml2hum.h
// URL:           https://github.com/craigsapp/musicxml2hum/blob/master/include/tool-musicxml2hum.h
// Syntax:        C++11; humlib
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

#include <string>
#include <vector>

namespace hum {

// START_MERGE

class MusicXmlHarmonyInfo {
	public:
		HTp    token;
		HumNum timestamp;
		int    partindex;
};

class MusicXmlFiguredBassInfo {
	public:
		HTp    token;
		HumNum timestamp;
		int    partindex;
};


class Tool_musicxml2hum : public HumTool {
	public:
		        Tool_musicxml2hum    (void);
		       ~Tool_musicxml2hum    () {}

		bool    convertFile          (ostream& out, const char* filename);
		bool    convert              (ostream& out, pugi::xml_document& infile);
		bool    convert              (ostream& out, const char* input);
		bool    convert              (ostream& out, istream& input);

		void    setOptions           (int argc, char** argv);
		void    setOptions           (const std::vector<std::string>& argvlist);
		Options getOptionDefinitions (void);

	protected:
		void   initialize           (void);
		std::string getChildElementText  (pugi::xml_node root, const char* xpath);
		std::string getChildElementText  (pugi::xpath_node root, const char* xpath);
		std::string getAttributeValue    (pugi::xml_node xnode, const std::string& target);
		std::string getAttributeValue    (xpath_node xnode, const std::string& target);
		void   printAttributes      (pugi::xml_node node);
		bool   getPartInfo          (map<std::string, pugi::xml_node>& partinfo,
		                             std::vector<std::string>& partids, pugi::xml_document& doc);
		bool   stitchParts          (HumGrid& outdata,
		                             std::vector<std::string>& partids,
		                             map<std::string, pugi::xml_node>& partinfo,
		                             map<std::string, pugi::xml_node>& partcontent,
		                             std::vector<MxmlPart>& partdata);
		bool   getPartContent       (map<std::string, pugi::xml_node>& partcontent,
		                             std::vector<std::string>& partids, pugi::xml_document& doc);
		void   printPartInfo        (std::vector<std::string>& partids,
		                             map<std::string, pugi::xml_node>& partinfo,
		                             map<std::string, pugi::xml_node>& partcontent,
		                             std::vector<MxmlPart>& partdata);
		bool   fillPartData         (std::vector<MxmlPart>& partdata,
		                             const std::vector<std::string>& partids,
		                             map<std::string, pugi::xml_node>& partinfo,
		                             map<std::string, pugi::xml_node>& partcontent);
		bool   fillPartData         (MxmlPart& partdata, const std::string& id,
		                             pugi::xml_node partdeclaration,
		                             pugi::xml_node partcontent);
		void   appendZeroEvents     (GridMeasure* outfile,
		                             std::vector<SimultaneousEvents*>& nowevents,
		                             HumNum nowtime,
		                             std::vector<MxmlPart>& partdata);
		void   appendNonZeroEvents   (GridMeasure* outdata,
		                              std::vector<SimultaneousEvents*>& nowevents,
		                              HumNum nowtime,
		                              std::vector<MxmlPart>& partdata);
		void   addGraceLines         (GridMeasure* outdata,
		                              std::vector<std::vector<std::vector<std::vector<MxmlEvent*>>>>& notes,
		                              std::vector<MxmlPart>& partdata, HumNum nowtime);
		void   addEventToList        (std::vector<std::vector<std::vector<std::vector<MxmlEvent*>>>>& list,
		                              MxmlEvent* event);
		void   addHeaderRecords      (HumdrumFile& outfile, pugi::xml_document& doc);
		void   addFooterRecords      (HumdrumFile& outfile, pugi::xml_document& doc);
		std::string& cleanSpaces     (std::string& input);
		std::string cleanSpacesAndColons(const std::string& input);
		void setEditorialAccidental  (int accidental, GridSlice* slice,
		                              int partindex, int staffindex, int voiceindex);
		void moveBreaksToEndOfPreviousMeasure(HumGrid& outdata);

		bool convert          (ostream& out);
		bool convertPart      (ostream& out, const std::string& partname,
		                       int partindex);
		bool insertMeasure    (HumGrid& outdata, int mnum,
		                       std::vector<MxmlPart>& partdata,
		                       std::vector<int> partstaves);
		bool convertNowEvents (GridMeasure* outdata,
		                       std::vector<SimultaneousEvents*>& nowevents,
		                       std::vector<int>& nowparts,
		                       HumNum nowtime,
		                       std::vector<MxmlPart>& partdata,
		                       std::vector<int>& partstaves);
		void appendNullTokens (HLp line, MxmlPart& part);
		void appendEvent      (HLp line, MxmlEvent* event);
		void insertExclusiveInterpretationLine(HumdrumFile& outfile,
		                       std::vector<MxmlPart>& partdata);
		void insertAllToken   (HumdrumFile& outfile, std::vector<MxmlPart>& partdata,
		                       const std::string& common);
		void insertSingleMeasure(HumdrumFile& outfile);
		void cleanupMeasures   (HumdrumFile& outfile,
		                        std::vector<HLp> measures);
		void processPrintElement(GridMeasure* outdata, pugi::xml_node element, HumNum timestamp);
		void insertOffsetHarmonyIntoMeasure(GridMeasure* gm);
		void insertOffsetFiguredBassIntoMeasure(GridMeasure* gm);

		void addStriaLine      (GridMeasure* outdata,
		                        std::vector<std::vector<xml_node> >& stafflines,
		                        std::vector<MxmlPart>& partdata,
                              HumNum nowtime);
		void addClefLine       (GridMeasure* outdata, std::vector<std::vector<pugi::xml_node>>& clefs,
		                        std::vector<MxmlPart>& partdata, HumNum nowtime);
		void addOttavaLine     (GridMeasure* outdata, std::vector<std::vector<std::vector<pugi::xml_node>>>& ottavas,
		                        std::vector<MxmlPart>& partdata, HumNum nowtime);
		void storeOttava       (int pindex, xml_node octaveShift, xml_node direction,
		                        std::vector<std::vector<std::vector<xml_node>>>& ottavas);
		void insertPartClefs   (pugi::xml_node clef, GridPart& part);
		void insertPartStria   (int lines, GridPart& part);
		void insertPartOttavas (pugi::xml_node ottava, GridPart& part, int partindex, int partstaffindex, int staffcount);
		pugi::xml_node convertClefToHumdrum(pugi::xml_node clef, HTp& token, int& staffindex);
		pugi::xml_node convertOttavaToHumdrum(pugi::xml_node ottava, HTp& token, int& staffindex,
		                        int partindex, int partstaffindex, int staffcount);

		void addTranspositionLine(GridMeasure* outdata, std::vector<std::vector<pugi::xml_node>>& transpositions,
		                       std::vector<MxmlPart>& partdata, HumNum nowtime);
		void addKeySigLine    (GridMeasure* outdata, std::vector<std::vector<pugi::xml_node>>& keysigs,
		                        std::vector<MxmlPart>& partdata, HumNum nowtime);
		void addKeyDesignationLine(GridMeasure* outdata, vector<vector<xml_node>>& keydesigs,
		                        vector<MxmlPart>& partdata, HumNum nowtime);
		void insertPartKeySigs (pugi::xml_node keysig, GridPart& part);
		void insertPartKeyDesignations(xml_node keydeg, GridPart& part);
		pugi::xml_node convertKeySigToHumdrum(pugi::xml_node keysig,
		                        HTp& token, int& staffindex);
		pugi::xml_node convertKeySigToHumdrumKeyDesignation(xml_node keysig,
		                        HTp& token, int& staffindex);

		void addTimeSigLine    (GridMeasure* outdata, std::vector<std::vector<pugi::xml_node>>& timesigs,
		                        std::vector<MxmlPart>& partdata, HumNum nowtime);
		bool insertPartTimeSigs (pugi::xml_node timesig, GridPart& part);
		void insertPartMensurations(pugi::xml_node timesig, GridPart& part);
		void insertPartNames    (HumGrid& outdata, std::vector<MxmlPart>& partdata);
		bool checkForMensuration(pugi::xml_node timesig);
		pugi::xml_node convertTimeSigToHumdrum(pugi::xml_node timesig,
		                        HTp& token, int& staffindex);
		pugi::xml_node convertMensurationToHumdrum(pugi::xml_node timesig,
		                        HTp& token, int& staffindex);

		void addEvent          (GridSlice* slice, GridMeasure* outdata, MxmlEvent* event, HumNum nowtime);
		void fillEmpties       (GridPart* part, const char* string);
		void addSecondaryChordNotes (ostream& output, MxmlEvent* head, const std::string& recip);
		bool isInvisible       (MxmlEvent* event);
		int  addLyrics         (GridStaff* staff, MxmlEvent* event);
		int  addHarmony        (GridPart* oart, MxmlEvent* event, HumNum nowtime, int partindex);
		void addDynamic        (GridPart* part, MxmlEvent* event, int partindex);
		void addHairpinEnding  (GridPart* part, MxmlEvent* event, int partindex);
		int  addFiguredBass    (GridPart* part, MxmlEvent* event, HumNum nowtime, int partindex);
		void addTexts          (GridSlice* slice, GridMeasure* measure, int partindex,
		                        int staffindex, int voiceindex, MxmlEvent* event);
		void addText           (GridSlice* slice, GridMeasure* measure, int partindex,
		                        int staffindex, int voiceindex, pugi::xml_node node, bool force = false);
		void addTempos         (GridSlice* slice, GridMeasure* measure, int partindex,
		                        int staffindex, int voiceindex, MxmlEvent* event);
		void addTempo          (GridSlice* slice, GridMeasure* measure, int partindex,
		                        int staffindex, int voiceindex, pugi::xml_node node);
		void addBrackets       (GridSlice* slice, GridMeasure* measure, MxmlEvent* event, HumNum nowtime,
		                        int partindex);
		int         getHarmonyOffset(pugi::xml_node hnode);
		std::string getHarmonyString(pugi::xml_node hnode);
		std::string getDynamicString(pugi::xml_node element);
		std::string getDynamicsParameters(pugi::xml_node element);
		std::string getFiguredBassString(pugi::xml_node element);
		std::string getFiguredBassParameters(pugi::xml_node element);
		std::string convertFiguredBassNumber(const xml_node& figure);
		int         getFiguredBassDuration(xml_node fnode);
		std::string getHairpinString(pugi::xml_node element, int partindex);
		std::string cleanSpaces     (const std::string& input);
		void checkForDummyRests(MxmlMeasure* measure);
		void reindexVoices     (std::vector<MxmlPart>& partdata);
		void reindexMeasure    (MxmlMeasure* measure);
		void setSoftwareInfo   (pugi::xml_document& doc);
		std::string getSystemDecoration(pugi::xml_document& doc, HumGrid& grid, std::vector<std::string>& partids);
		void getChildrenVector (std::vector<pugi::xml_node>& children, pugi::xml_node parent);
		void insertPartTranspositions(pugi::xml_node transposition, GridPart& part);
		pugi::xml_node convertTranspositionToHumdrum(pugi::xml_node transpose, HTp& token, int& staffindex);
		void prepareRdfs       (std::vector<MxmlPart>& partdata);
		void printRdfs         (ostream& out);
		void printResult       (ostream& out, HumdrumFile& outfile);
		void addMeasureOneNumber(HumdrumFile& infile);
		bool isUsedHairpin     (pugi::xml_node hairpin, int partindex);

		int getIndex(std::vector<std::string>& v, const std::string& K);
		std::string getInterval(const std::string& bottomNote, const std::string& topNote,
		                        int bottomAcc = 0, int topAcc = 0);
		std::string decipherHarte(std::vector<int>& degrees);
		std::string alterRoot    (int rootalter);

	public:

	static bool nodeType      (pugi::xml_node node, const char* testname);

	private:
		Options m_options;
		bool DebugQ;
		bool VoiceDebugQ;
		bool m_recipQ        = false;
		bool m_stemsQ        = false;
		int  m_slurabove     = 0;
		int  m_slurbelow     = 0;
		char m_hasEditorial  = '\0';
		bool m_hasOrnamentsQ = false;
		int  m_maxstaff      = 0;
		std::vector<std::vector<std::string>> m_last_ottava_direction;
		std::vector<MusicXmlHarmonyInfo> offsetHarmony;
		std::vector<MusicXmlFiguredBassInfo> offsetFiguredBass;
		std::vector<string> m_stop_char;

		// RDF indications in **kern data:
		std::string  m_caesura_rdf;

		std::string m_software;
		std::string m_systemDecoration;

		std::vector<std::vector<pugi::xml_node>> m_current_dynamic;
		std::vector<std::vector<pugi::xml_node>> m_current_brackets;
		std::map<int, string> m_bracket_type_buffer;
		std::vector<std::vector<pugi::xml_node>> m_used_hairpins;
		std::vector<pugi::xml_node> m_current_figured_bass;
		std::vector<std::pair<int, pugi::xml_node>> m_current_text;
		std::vector<std::pair<int, pugi::xml_node>> m_current_tempo;

		bool m_hasTransposition = false;

		// m_forceRecipQ is used to force the display of the **recip spint
		// when a data line contains no notes or rests.  This is used for
		// harmony/dynamics side spines.
		bool m_forceRecipQ = false;

		// m_hasTremoloQ is used to run the tremolo tool.
		bool m_hasTremoloQ = false;

		// m_post_note_text is used to store interpretations that occur
		// before notes in the MusicXML data, but need to be moved after
		// the note in the Humdrum data.  The text will be stored and then
		// when note is processed, any text in this storage will be processed
		// index is a string: "part staff voice" with a vector list of strings
		// to process.
		std::map<std::string, vector<pugi::xml_node>> m_post_note_text;

};


// END_MERGE

}  // end of namespace hum

#endif /* _TOOL_MUSICXML2HUM_H */



