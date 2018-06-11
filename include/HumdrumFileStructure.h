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

#include <iostream>
#include <set>
#include <string>
#include <vector>

#include "HumdrumFileBase.h"

namespace hum {

// START_MERGE

class HumdrumFileStructure : public HumdrumFileBase {
	public:
		              HumdrumFileStructure         (void);
		              HumdrumFileStructure         (const std::string& filename);
		              HumdrumFileStructure         (std::istream& contents);
		             ~HumdrumFileStructure         ();
		bool          hasFilters                   (void);

		// TSV reading functions:
		bool          read                         (std::istream& contents);
		bool          read                         (const char* filename);
		bool          read                         (const std::string& filename);
		bool          readString                   (const char* contents);
		bool          readString                   (const std::string& contents);
		bool parse(std::istream& contents)      { return read(contents); }
		bool parse(const char* contents)   { return readString(contents); }
		bool parse(const std::string& contents) { return readString(contents); }
		bool          readNoRhythm                 (std::istream& contents);
		bool          readNoRhythm                 (const char* filename);
		bool          readNoRhythm                 (const std::string& filename);
		bool          readStringNoRhythm           (const char* contents);
		bool          readStringNoRhythm           (const std::string& contents);

		// CSV reading functions:
		bool          readCsv                      (std::istream& contents,
		                                            const std::string& separator=",");
		bool          readCsv                      (const char* filename,
		                                            const std::string& separator=",");
		bool          readCsv                      (const std::string& filename,
		                                            const std::string& separator=",");
		bool          readStringCsv                (const char* contents,
		                                            const std::string& separator=",");
		bool          readStringCsv                (const std::string& contents,
		                                            const std::string& separator=",");
		bool parseCsv(std::istream& contents, const std::string& separator = ",")
		                                 { return readCsv(contents, separator); }
		bool parseCsv(const char* contents, const std::string& separator = ",")
		                           { return readStringCsv(contents, separator); }
		bool parseCsv(const std::string& contents, const std::string& separator = ",")
		                           { return readStringCsv(contents, separator); }
		bool          readNoRhythmCsv              (std::istream& contents,
		                                            const std::string& separator = ",");
		bool          readNoRhythmCsv              (const char* filename,
		                                            const std::string& separator = ",");
		bool          readNoRhythmCsv              (const std::string& filename,
		                                            const std::string& separator = ",");
		bool          readStringNoRhythmCsv        (const char* contents,
		                                            const std::string& separator = ",");
		bool          readStringNoRhythmCsv        (const std::string& contents,
		                                            const std::string& separator = ",");

		// rhythmic analysis related functionality:
		HumNum        getScoreDuration             (void) const;
		std::ostream&      printDurationInfo       (std::ostream& out = std::cout);
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
		HumNum        getMinDur                    (std::vector<HumNum>& durs,
		                                            std::vector<HumNum>& durstate);
		bool          getTokenDurations            (std::vector<HumNum>& durs,
		                                            int line);
		bool          cleanDurs                    (std::vector<HumNum>& durs,
		                                            int line);
		bool          decrementDurStates           (std::vector<HumNum>& durs,
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
		std::set<HumNum>   getNonZeroLineDurations      (void);
		std::set<HumNum>   getPositiveLineDurations     (void);
		void          processLocalParametersForStrand(int index);
		bool          processLocalParametersForTrack (HTp starttok, HTp current);
		void          checkForLocalParameters      (HTp token, HTp current);
		bool          assignDurationsToNonRhythmicTrack(HTp endtoken, HTp ptoken);
		void          analyzeSpineStrands          (std::vector<TokenPair>& ends,
		                                            HTp starttok);
		void          analyzeSignifiers            (void);
};


// END_MERGE

} // end namespace hum

#endif /* _HUMDRUMFILESTRUCTURE_H_INCLUDED */



