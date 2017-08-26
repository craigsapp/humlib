//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Aug 17 02:39:28 PDT 2015
// Last Modified: Mon Aug 17 02:39:32 PDT 2015
// Filename:      HumdrumFileStructure.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/HumdrumFileStructure.h
// Syntax:        C++11
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Used to further process HumdrumFileBase content, primarily
//                rhythmic analyses, but also parses global and local
//                token parameters.  The HumdrumFileContent class does
//                further analysis of the Humdrum data, primary of specific
//                data content rather than general structural analysis.
//

#ifndef _HUMDRUMFILESTRUCTURE_H_INCLUDED
#define _HUMDRUMFILESTRUCTURE_H_INCLUDED

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
		bool          hasFilters                   (void);

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
		HTp           getStrandStart               (int index) const;
		HTp           getStrandEnd                 (int index) const;
		HTp           getStrandStart               (int sindex, int index) const;
		HTp           getStrandEnd                 (int sindex, int index) const;
		int           getStrandCount               (void) const;
		int           getStrandCount               (int spineindex) const;
		void          resolveNullTokens            (void);

		HTp           getStrand                    (int index) const
		                                        { return getStrandStart(index); }
		HTp           getStrand                    (int sindex, int index) const
		                                { return getStrandStart(sindex, index); }

		// barline/measure functionality:
		int           getBarlineCount              (void) const;
		HumdrumLine*  getBarline                   (int index) const;
		HumNum        getBarlineDuration           (int index) const;
		HumNum        getBarlineDurationFromStart  (int index) const;
		HumNum        getBarlineDurationToEnd      (int index) const;

		bool          analyzeStructure             (void);
		bool          analyzeStrands               (void);

	protected:

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
		bool          assignDurationsToTrack       (HTp starttoken,
		                                            HumNum startdur);
		bool          prepareDurations             (HTp token, int state,
		                                            HumNum startdur);
		bool          setLineDurationFromStart     (HTp token, HumNum dursum);
		bool          analyzeRhythmOfFloatingSpine (HTp spinestart);
		bool          analyzeNullLineRhythms       (void);
		void          fillInNegativeStartTimes     (void);
		void          assignLineDurations          (void);
		void          assignStrandsToTokens        (void);
		set<HumNum>   getNonZeroLineDurations      (void);
		set<HumNum>   getPositiveLineDurations     (void);
		void          processLocalParametersForStrand(int index);
		bool          processLocalParametersForTrack (HTp starttok, HTp current);
		void          checkForLocalParameters      (HTp token, HTp current);
		bool          assignDurationsToNonRhythmicTrack(HTp endtoken, HTp ptoken);
		void          analyzeSpineStrands          (vector<TokenPair>& ends,
		                                            HTp starttok);
};


// END_MERGE

} // end namespace hum

#endif /* _HUMDRUMFILESTRUCTURE_H_INCLUDED */



