//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Aug 17 02:39:28 PDT 2015
// Last Modified: Mon Aug 17 02:39:32 PDT 2015
// Filename:      HumdrumFileStructure.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/HumdrumFileStructure.h
// Syntax:        C++11
// vim:           ts=3 noexpandtab
//
// Description:   Used to further process HumdrumFileBase content, primarily
//                rhythmic analyses, but also parses global and local
//                token parameters.  The HumdrumFileContent class does
//                further analysis of the Humdrum data, primary of specific
//                data content rather than general structural analysis.
//

#ifndef _HUMDRUMFILESTRUCTURE_H
#define _HUMDRUMFILESTRUCTURE_H

#include <set>

#include "HumdrumFileBase.h"

using namespace std;

namespace hum {

// START_MERGE

class HumdrumFileStructure : public HumdrumFileBase {
	public:
		              HumdrumFileStructure         (void);
		              HumdrumFileStructure         (const string& filename);
		              HumdrumFileStructure         (istream& contents);
		             ~HumdrumFileStructure         ();

		// TSV reading functions:
		bool          read                         (istream& contents);
		bool          read                         (const char*   filename);
		bool          read                         (const string& filename);
		bool          readString                   (const char*   contents);
		bool          readString                   (const string& contents);
		bool parse(istream& contents)      { return read(contents); }
		bool parse(const char* contents)   { return readString(contents); }
		bool parse(const string& contents) { return readString(contents); }
		bool          readNoRhythm                 (istream& contents);
		bool          readNoRhythm                 (const char*   filename);
		bool          readNoRhythm                 (const string& filename);
		bool          readStringNoRhythm           (const char*   contents);
		bool          readStringNoRhythm           (const string& contents);

		// CSV reading functions:
		bool          readCsv                      (istream& contents,
		                                            const string& separator=",");
		bool          readCsv                      (const char*   filename,
		                                            const string& separator=",");
		bool          readCsv                      (const string& filename,
		                                            const string& separator=",");
		bool          readStringCsv                (const char*   contents,
		                                            const string& separator=",");
		bool          readStringCsv                (const string& contents,
		                                            const string& separator=",");
		bool parseCsv(istream& contents, const string& separator = ",")
		                                 { return readCsv(contents, separator); }
		bool parseCsv(const char* contents, const string& separator = ",")
		                     { return readStringCsv(contents, separator); }
		bool parseCsv(const string& contents, const string& separator = ",")
		                     { return readStringCsv(contents, separator); }
		bool          readNoRhythmCsv              (istream& contents,
		                     const string& separator = ",");
		bool          readNoRhythmCsv              (const char*   filename,
		                     const string& separator = ",");
		bool          readNoRhythmCsv              (const string& filename,
		                     const string& separator = ",");
		bool          readStringNoRhythmCsv        (const char*   contents,
		                     const string& separator = ",");
		bool          readStringNoRhythmCsv        (const string& contents,
		                     const string& separator = ",");

		// rhythmic analysis related functionality:
		HumNum        getScoreDuration             (void) const;
		ostream&      printDurationInfo            (ostream& out = cout);
		int           tpq                          (void);

		// strand functionality:
		HumdrumToken* getStrandStart(int index) const;
		HumdrumToken* getStrandEnd(int index) const;
		HumdrumToken* getStrandStart(int sindex, int index) const;
		HumdrumToken* getStrandEnd(int sindex, int index) const;

		HumdrumToken* getStrand(int index) const {
			return getStrandStart(index); }
		HumdrumToken* getStrand(int sindex, int index) const {
			return getStrandStart(sindex, index); }

		// barline/measure functionality:
		int           getBarlineCount              (void) const;
		HumdrumLine*  getBarline                   (int index) const;
		HumNum        getBarlineDuration           (int index) const;
		HumNum        getBarlineDurationFromStart  (int index) const;
		HumNum        getBarlineDurationToEnd      (int index) const;

	protected:

		bool          analyzeStructure             (void);
		bool          analyzeStrands               (void);
		bool          analyzeRhythm                (void);
		bool          assignRhythmFromRecip        (HTp spinestart);
		bool          analyzeMeter                 (void);
		bool          analyzeTokenDurations        (void);
		bool          analyzeGlobalParameters      (void);
		bool          analyzeLocalParameters       (void);
		bool          analyzeDurationsOfNonRhythmicSpines(void);
		HumNum        getMinDur                    (vector<HumNum>& durs,
		                                            vector<HumNum>& durstate);
		bool          getTokenDurations            (vector<HumNum>& durs,
		                                            int line);
		bool          cleanDurs                    (vector<HumNum>& durs,
		                                            int line);
		bool          decrementDurStates           (vector<HumNum>& durs,
		                                            HumNum linedur, int line);
		bool          assignDurationsToTrack       (HumdrumToken* starttoken,
		                                            HumNum startdur);
		bool          prepareDurations             (HumdrumToken* token,
		                                            int state,
		                                            HumNum startdur);
		bool          setLineDurationFromStart     (HumdrumToken* token,
		                                            HumNum dursum);
		bool          analyzeRhythmOfFloatingSpine (HumdrumToken* spinestart);
		bool          analyzeNullLineRhythms       (void);
		void          fillInNegativeStartTimes     (void);
		void          assignLineDurations          (void);
		void          assignStrandsToTokens        (void);
		set<HumNum>   getNonZeroLineDurations      (void);
		set<HumNum>   getPositiveLineDurations     (void);
		bool          processLocalParametersForTrack (HumdrumToken* starttok,
		                                            HumdrumToken* current);
		void          checkForLocalParameters      (HumdrumToken *token,
		                                            HumdrumToken *current);
		bool          assignDurationsToNonRhythmicTrack(HumdrumToken* endtoken,
		                                            HumdrumToken* ptoken);
		void          analyzeSpineStrands          (vector<TokenPair>& ends,
		                                            HumdrumToken* starttok);

};


// END_MERGE

} // end namespace hum

#endif /* _HUMDRUMFILESTRUCTURE_H */



