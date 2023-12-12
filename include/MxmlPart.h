//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Aug  6 10:53:40 CEST 2016
// Last Modified: Sun Sep 18 14:16:18 PDT 2016
// Filename:      MxmlPart.cpp
// URL:           https://github.com/craigsapp/hum2ly/blob/master/include/MxmlPart.h
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   MusicXML parsing abstraction for part elements which
//                contain a list of measures.
//
// part element documentation:
//    http://usermanuals.musicxml.com/MusicXML/Content/EL-MusicXML-part.htm
//

#ifndef _MXMLPART_H
#define _MXMLPART_H

#include "MxmlMeasure.h"

#include "pugiconfig.hpp"
#include "pugixml.hpp"

#include <vector>

using namespace pugi;
using namespace std;

namespace hum {

// START_MERGE

class MxmlMeasure;
class MxmlEvent;

class MxmlPart {
	public:
		              MxmlPart             (void);
		             ~MxmlPart             ();
		void          clear                (void);
		void          enableStems          (void);
		bool          readPart             (const string& id, xml_node partdef,
		                                    xml_node part);
		bool          addMeasure           (xml_node mel);
		bool          addMeasure           (xpath_node mel);
		int           getMeasureCount      (void) const;
		MxmlMeasure*  getMeasure           (int index) const;
		long          getQTicks            (void) const;
		int           setQTicks            (long value);
	   MxmlMeasure*  getPreviousMeasure   (MxmlMeasure* measure) const;
		HumNum        getDuration          (void) const;
		void          allocateSortedEvents (void);
		void          setPartNumber        (int number);
		int           getPartNumber        (void) const;
		int           getPartIndex         (void) const;
		int           getStaffCount        (void) const;
		int           getVerseCount        (void) const;
		int           getVerseCount        (int staffindex) const;
		string        getCaesura           (void) const;
		int           getHarmonyCount      (void) const;
		void          trackStaffVoices     (int staffnum, int voicenum);
		void          printStaffVoiceInfo  (void);
		void          prepareVoiceMapping  (void);
		int           getVoiceIndex        (int voicenum);
		int           getStaffIndex        (int voicenum);
		bool          hasEditorialAccidental(void) const;
		bool          hasDynamics          (void) const;
		bool          hasFiguredBass       (void) const;
		void          parsePartInfo        (xml_node partdeclaration);
		string        getPartName          (void) const;
		string        getPartAbbr          (void) const;
		string        cleanSpaces          (const string& input);
		bool          hasOrnaments         (void) const;

		vector<pair<int, int>> getVoiceMapping (void) { return m_voicemapping; };
		vector<vector<int>> getStaffVoiceHist (void) { return m_staffvoicehist; };


	private:
		void          receiveStaffNumberFromChild (int staffnum, int voicenum);
		void          receiveVerseCount           (int count);
		void          receiveVerseCount           (int staffnum, int count);
		void          receiveHarmonyCount         (int count);
		void          receiveEditorialAccidental  (void);
		void          receiveDynamic              (void);
		void          receiveFiguredBass          (void);
		void          receiveCaesura              (const string& letter);
		void          receiveOrnament             (void);

	protected:
		vector<MxmlMeasure*> m_measures;
		vector<long>         m_qtick;
		int                  m_partnum;
		int                  m_maxstaff;
		vector<int>          m_verseCount;
		int                  m_harmonyCount;
		bool                 m_editorialAccidental;
		bool                 m_stems = false;
		bool                 m_has_dynamics = false;
		bool                 m_has_figured_bass = false;
		string               m_partname;
		string               m_partabbr;
		string               m_caesura;
		bool                 m_hasOrnaments = false;

		// m_staffvoicehist: counts of staff and voice numbers.
		// staff=0 is used for items such as measures.
		// voice=0 is used for nonduration items such as harmony.
		vector<vector<int> > m_staffvoicehist;
	 	vector<pair<int, int> > m_voicemapping; // voicenum -> (staff, voiceindex)

	friend MxmlMeasure;
	friend MxmlEvent;

};


// END_MERGE

} // end namespace hum

#endif /* _MXMLPART_H */



