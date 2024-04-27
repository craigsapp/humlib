//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Thu Jun  3 00:00:21 PDT 2010
// Last Modified: Mon Feb 21 10:29:48 PST 2022
// Filename:      humlib/include/MuseData.h
// Web Address:   https://github.com/craigsapp/humlib/blob/master/include/MuseData.h
// Syntax:        C++
// vim:           ts=3
//
// Description:   A class that stores a list of MuseRecords.
//

#ifndef _MUSEDATA_H_INCLUDED
#define _MUSEDATA_H_INCLUDED

#include "MuseRecord.h"
#include "HumNum.h"

#include <vector>

namespace hum {

// START_MERGE


// A MuseEventSet is a timestamp and then a list of pointers to all
// lines in the original file that occur at that time.
// The MuseData class contains a variable called "sequence" which is
// a list of MuseEventSet object pointers which are sorted by time.

class MuseEventSet {
	public:
		                   MuseEventSet       (void);
		                   MuseEventSet       (const MuseEventSet& aSet);
		                   MuseEventSet       (HumNum atime);
		                  ~MuseEventSet       ()     { clear(); }

		void               clear              (void);
		void               setTime            (HumNum abstime);
		HumNum             getTime            (void);
		void               appendRecord       (MuseRecord* arecord);
		MuseRecord&        operator[]         (int index);
		MuseEventSet       operator=          (MuseEventSet& anevent);
		int                getEventCount      (void);

	protected:
		HumNum     absbeat;              // starting time of events
		std::vector<MuseRecord*> events; // list of events on absbeat
};



class MuseData {
	public:
		                  MuseData            (void);
		                  MuseData            (MuseData& input);
		                 ~MuseData            ();

		void              setFilename         (const std::string& filename);
		std::string       getFilename         (void);
		std::string       getPartName         (void);
		int               isMember            (const std::string& mstring);
		int               getMembershipPartNumber(const std::string& mstring);
		void              selectMembership    (const std::string& selectstring);
		MuseData&         operator=           (MuseData& input);
		int               getLineCount        (void);
		int               getNumLines         (void) { return getLineCount(); }
		MuseRecord&       last                (void);
		int               isEmpty             (void);
		int               append              (MuseRecord& arecord);
		int               append              (MuseData& musedata);
		int               append              (std::string& charstring);
		void              insert              (int index, MuseRecord& arecord);
		void              clear               (void);
		int               getInitialTpq       (void);

		int               read                (std::istream& input);
		int               readString          (const std::string& filename);
		int               readFile            (const std::string& filename);
		void              analyzeLayers       (void);
		int               analyzeLayersInMeasure(int startindex);

		// aliases for access to MuseRecord objects based on line indexes:
		std::string       getLine             (int index);

		bool              isCopyright         (int index);
		bool              isEncoder           (int index);
		bool              isId                (int index);
		bool              isMovementTitle     (int index);
		bool              isAnyNote           (int index);
		bool              isRegularNote       (int index);
		bool              isPartName          (int index);
		bool              isSource            (int index);
		bool              isWorkInfo          (int index);
		bool              isWorkTitle         (int index);
		bool              isHeaderRecord      (int index);
		bool              isBodyRecord        (int index);

		// header information
		std::string       getComposer         (void);
		std::string       getComposerDate     (void);
		std::string       getCopyright        (void);
		std::string       getEncoder          (void);
		std::string       getEncoderDate      (void);
		std::string       getEncoderName      (void);
		std::string       getId               (void);
		std::string       getMovementTitle    (void);
		std::string       getSource           (void);
		std::string       getWorkInfo         (void);
		std::string       getWorkTitle        (void);
		std::string       getOpus             (void);
		std::string       getNumber           (void);
		std::string       getMovementNumber   (void);

		// additional mark-up analysis functions for post-processing:
		void              doAnalyses          (void);
		void              analyzeType         (void);
		void              analyzeRhythm       (void);
		void              analyzeTies         (void);
		void              analyzePitch        (void);
		void              analyzeTpq          (void);

		// line-based (file-order indexing) accessor functions:
		MuseRecord&       operator[]          (int lindex);
		MuseRecord&       getRecord           (int lindex);
		MuseRecord*       getRecordPointer    (int lindex);
		HumNum            getTiedDuration     (int lindex);

		HumNum            getAbsBeat         (int lindex);
		HumNum            getFileDuration    (void);

		int               getLineTickDuration (int lindex);

		// event-based (time-order indexing) accessor functions:
		MuseEventSet&     getEvent            (int eindex);
		int               getEventCount       (void);
		HumNum            getEventTime        (int eindex);
		MuseRecord&       getRecord           (int eindex, int erecord);
		int               getLineIndex        (int eindex, int erecord);
		HumNum            getLineDuration     (int eindex, int erecord);
		HumNum            getNoteDuration     (int eindex, int erecord);
		int               getLastTiedNoteLineIndex(int eindex, int erecord);
		int               getNextTiedNoteLineIndex(int eindex, int erecord);
		HumNum            getTiedDuration     (int eindex, int erecord);
		int               getType             (int eindex, int erecord);
		void              cleanLineEndings    (void);
		std::string       getError            (void);
		bool              hasError            (void);

	private:
		std::vector<MuseRecord*>    m_data;
		std::vector<MuseEventSet*>  m_sequence;
		std::string                 m_name;
		std::string                 m_error;

	protected:
		void         clearError           (void);
		void         setError             (const std::string& error);
		void         processTie           (int eventindex, int recordindex,
		                                        int lastindex);
		int          searchForPitch       (int eventindex, int b40, int track);
		int          getNextEventIndex    (int startindex, HumNum target);
		void         constructTimeSequence(void);
		void         insertEventBackwards (HumNum atime, MuseRecord* arecord);
		int          getPartNameIndex     (void);
		std::string  getPartName          (int index);
		void         assignHeaderBodyState(void);
		void         linkPrintSuggestions (void);
		void         linkMusicDirections  (void);

	public:
		static std::string  trimSpaces    (const std::string& input);
		static std::string  convertAccents(const std::string& input);
		static std::string  cleanString   (const std::string& input);
};


std::ostream& operator<<(std::ostream& out, MuseData& musedata);



// END_MERGE

} // end namespace hum

#endif  /* _MUSEDATA_H_INCLUDED */



