//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Aug  6 10:53:40 CEST 2016
// Last Modified: Sun Sep 18 14:16:18 PDT 2016
// Filename:      MxmlEvent.cpp
// URL:           https://github.com/craigsapp/musicxml2hum/blob/master/include/MxmlEvent.h
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   MusicXML parsing abstraction for elements which are children
//                of the measure element.
//

#ifndef _MXMLEVENT_H
#define _MXMLEVENT_H

#include "math.h"

#include "GridCommon.h"
#include "HumNum.h"

#include "pugiconfig.hpp"
#include "pugixml.hpp"

#include <sstream>
#include <string>
#include <vector>

using namespace pugi;

namespace hum {

// START_MERGE

class MxmlMeasure;
class MxmlPart;

// Event types: These are all of the XML elements which can be children of
// the measure element in MusicXML.

enum measure_event_type {
	mevent_unknown,
	mevent_attributes,
	mevent_backup,
	mevent_barline,
	mevent_bookmark,
	mevent_direction,
	mevent_figured_bass,
	mevent_forward,
	mevent_grouping,
	mevent_harmony,
	mevent_link,
	mevent_note,
	mevent_print,
	mevent_sound,
	mevent_float       // category for GridSides not attached to note onsets
};


class MxmlEvent {
	public:
		                   MxmlEvent          (MxmlMeasure* measure);
		                  ~MxmlEvent          ();
		void               clear              (void);
		void               enableStems        (void);
		bool               parseEvent         (xml_node el, xml_node nextel,
		                                       HumNum starttime);
		bool               parseEvent         (xpath_node el, HumNum starttime);
		void               setTickStart       (long value, long ticks);
		void               setTickDur         (long value, long ticks);
		void               setStartTime       (HumNum value);
		void               setDuration        (HumNum value);
		void               setDurationByTicks (long value, xml_node el = xml_node(NULL));
		void               setModification    (HumNum value);
		HumNum             getStartTime       (void) const;
		HumNum             getDuration        (void) const;
		HumNum             getModification    (void) const;
		void               setOwner           (MxmlMeasure* measure);
		MxmlMeasure*       getOwner           (void) const;
		const char*        getName            (void) const;
		int                setQTicks          (long value);
		long               getQTicks          (void) const;
		long               getIntValue        (const char* query) const;
		bool               hasChild           (const char* query) const;
		void               link               (MxmlEvent* event);
		bool               isLinked           (void) const;
		bool               isRest             (void);
		bool               isGrace            (void);
		bool               hasGraceSlash      (void);
		bool               isFloating         (void);
		int                hasSlurStart       (std::vector<int>& directions);
		int                hasSlurStop        (void);
		void               setLinked          (void);
		std::vector<MxmlEvent*> getLinkedNotes     (void);
		void               attachToLastEvent  (void);
		bool               isChord            (void) const;
		std::ostream&      print              (std::ostream& out);
		int                getSequenceNumber  (void) const;
		int                getVoiceNumber     (void) const;
		void               setVoiceNumber     (int value);
		int                getStaffNumber     (void) const;
		int                getStaffIndex      (void) const;
		int                getCrossStaffOffset(void) const;
		int                getVoiceIndex      (int maxvoice = 4) const;
		void               setStaffNumber     (int value);
		measure_event_type getType            (void) const;
		int                getPartNumber      (void) const;
		int                getPartIndex       (void) const;
		std::string        getRecip           (void) const;
		std::string        getKernPitch       (void);
		std::string        getPrefixNoteInfo  (void) const;
		std::string        getPostfixNoteInfo (bool primarynote, const std::string& recip) const;
		xml_node           getNode            (void);
		xml_node           getHNode           (void);
		HumNum             getTimeSigDur      (void);
		std::string        getElementName     (void);
		void               addNotations       (std::stringstream& ss,
		                                       xml_node notations,
		                                       int beamstarts,
		                                       const std::string& recip) const;
		void               reportVerseCountToOwner    (int count);
		void               reportVerseCountToOwner    (int staffnum, int count);
		void               reportHarmonyCountToOwner  (int count);
		void               reportMeasureStyleToOwner  (MeasureStyle style);
		void               reportEditorialAccidentalToOwner(void);
		void               reportDynamicToOwner       (void);
		void               reportFiguredBassToOwner   (void);
		void               reportCaesuraToOwner       (const std::string& letter = "Z") const;
		void               reportOrnamentToOwner      (void) const;
      void               makeDummyRest      (MxmlMeasure* owner,
		                                       HumNum startime,
		                                       HumNum duration,
		                                       int staffindex = 0,
		                                       int voiceindex = 0);
		void               setVoiceIndex      (int voiceindex);
		void               forceInvisible     (void);
		bool               isInvisible        (void);
		void               setBarlineStyle    (xml_node node);
		void               setTexts           (std::vector<std::pair<int, xml_node>>& nodes);
		void               setTempos          (std::vector<std::pair<int, xml_node>>& nodes);
		std::vector<std::pair<int, xml_node>>&  getTexts           (void);
		std::vector<std::pair<int, xml_node>>&  getTempos          (void);
		void               setDynamics        (xml_node node);
		void               setBracket         (xml_node node);
		void               setHairpinEnding   (xml_node node);
		void               addFiguredBass     (xml_node node);
		std::vector<xml_node> getDynamics     (void);
		std::vector<xml_node> getBrackets     (void);
		xml_node           getHairpinEnding   (void);
		int                getFiguredBassCount(void);
		xml_node           getFiguredBass     (int index);
		std::string        getRestPitch       (void) const;

	protected:
		HumNum             m_starttime;    // start time in quarter notes of event
		HumNum             m_duration;     // duration in quarter notes of event
      HumNum             m_modification; // tuplet time adjustment of note
		measure_event_type m_eventtype;    // enumeration type of event
		xml_node           m_node;         // pointer to event in XML structure
		MxmlMeasure*       m_owner;        // measure that contains this event
		std::vector<MxmlEvent*> m_links;   // list of secondary chord notes
		bool               m_linked;       // true if a secondary chord note
		int                m_sequence;     // ordering of event in XML file
		static int         m_counter;      // counter for sequence variable
		short              m_staff;        // staff number in part for event
		short              m_voice;        // voice number in part for event
		int                m_voiceindex;   // voice index of item (remapping)
      int                m_maxstaff;     // maximum staff number for measure
		xml_node           m_hnode;        // harmony label starting at note event
		bool               m_invisible;    // for forceInvisible();
		bool               m_stems;        // for preserving stems

		std::vector<xml_node> m_dynamics;   // dynamics <direction> starting just before note
		xml_node          m_hairpin_ending; // hairpin <direction> starting just after note and before new measure
		std::vector<xml_node>  m_figured_bass; // fb starting just before note
		std::vector<xml_node>  m_brackets;  // brackets to start/end before/after note
		std::vector<std::pair<int, xml_node>>  m_text;   // text <direction> starting just before note
		std::vector<std::pair<int, xml_node>>  m_tempo;   // tempo starting just before note

	private:
   	void   reportStaffNumberToOwner  (int staffnum, int voicenum);
		void   reportTimeSigDurToOwner   (HumNum duration);
		int    getDotCount               (void) const;

	public:
		static HumNum getQuarterDurationFromType (const char* type);
		static bool   nodeType             (xml_node node, const char* testname);
		static HumNum getEmbeddedDuration  (HumNum& modification, xml_node el = xml_node(NULL));


	friend MxmlMeasure;
	friend MxmlPart;
};


std::ostream& operator<<(std::ostream& output, xml_node element);


// END_MERGE

} // end namespace hum

#endif /* _MXMLEVENT_H */



