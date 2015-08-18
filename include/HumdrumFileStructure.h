//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Aug 17 02:39:28 PDT 2015
// Last Modified: Mon Aug 17 02:39:32 PDT 2015
// Filename:      HumdrumFileStructure.h
// URL:           https://github.com/craigsapp/minHumdrum/blob/master/include/HumdrumFileStructure.h
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

#include "HumdrumFileBase.h"

using namespace std;

namespace minHumdrum {

// START_MERGE

class HumdrumFileStructure : public HumdrumFileBase {
	public:
		              HumdrumFileStructure         (void);
		             ~HumdrumFileStructure         ();

		bool          read                         (istream& infile);
		bool          read                         (const char*   filename);
		bool          read                         (const string& filename);
		bool          readString                   (const char*   contents);
		bool          readString                   (const string& contents);

		bool          readNoRhythm                 (istream& infile);
		bool          readNoRhythm                 (const char*   filename);
		bool          readNoRhythm                 (const string& filename);
		bool          readStringNoRhythm           (const char*   contents);
		bool          readStringNoRhythm           (const string& contents);

		// rhythmic analysis related functionality:
		HumNum        getScoreDuration             (void) const;
		ostream&      printDurationInfo            (ostream& out = cout);

		// barline/measure functionality:
		int           getBarlineCount              (void) const;
		HumdrumLine*  getBarline                   (int index) const;
		HumNum        getBarlineDuration           (int index) const;
		HumNum        getBarlineDurationFromStart  (int index) const;
		HumNum        getBarlineDurationToEnd      (int index) const;

	protected:

		bool          analyzeStructure             (void);
		bool          analyzeRhythm                (void);
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
		bool      analyzeRhythmOfFloatingSpine (HumdrumToken* spinestart);
		bool      analyzeNullLineRhythms       (void);
		void      fillInNegativeStartTimes     (void);
		void      assignLineDurations          (void);
		bool      processLocalParametersForTrack(HumdrumToken* starttok,
		                                         HumdrumToken* current);
		void      checkForLocalParameters       (HumdrumToken *token,
		                                         HumdrumToken *current);
		bool      assignDurationsToNonRhythmicTrack(HumdrumToken* endtoken,
		                                                  HumdrumToken* ptoken);

};


// END_MERGE

} // end namespace std;

#endif /* _HUMDRUMFILESTRUCTURE_H */



