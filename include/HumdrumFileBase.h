//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Aug  8 12:24:49 PDT 2015
// Last Modified: Fri Aug 14 22:45:49 PDT 2015
// Filename:      HumdrumFileBase.h
// URL:           https://github.com/craigsapp/minHumdrum/blob/master/include/HumdrumFileBase.h
// Syntax:        C++11
// vim:           ts=3
//
// Description:   Used to store Humdrum text lines from input stream
//                for further parsing.
//

#ifndef _HUMDRUMFILEBASE_H
#define _HUMDRUMFILEBASE_H

#include <iostream>
#include <vector>

#include "HumdrumLine.h"

using namespace std;

namespace minHumdrum {

// START_MERGE

class HumdrumFileBase {
	public:
		              HumdrumFileBase              (void);
		             ~HumdrumFileBase              ();

		bool          read                         (istream& infile);
		bool          read                         (const char*   contents);
		bool          read                         (const string& contents);
		bool          readString                   (const char*   contents);
		bool          readString                   (const string& contents);
		bool          readNoRhythm                 (istream& infile);
		bool          readNoRhythm                 (const char*   contents);
		bool          readNoRhythm                 (const string& contents);
		bool          readStringNoRhythm           (const char*   contents);
		bool          readStringNoRhythm           (const string& contents);
		HumdrumLine&  operator[]                   (int index);
		void          append                       (const char* line);
		void          append                       (const string& line);
		int           getLineCount                 (void) const;
		int           getMaxTrack                  (void) const;
		ostream&      printSpineInfo               (ostream& out = cout);
		ostream&      printDataTypeInfo            (ostream& out = cout);
		ostream&      printTrackInfo               (ostream& out = cout);
		ostream&      printDurationInfo            (ostream& out = cout);
		HumdrumToken* getTrackStart                (int track) const;
		int           getTrackEndCount             (int track) const;
		HumdrumToken* getTrackEnd                  (int track,
		                                            int subtrack) const;
		void          createLinesFromTokens        (void);
		HumNum        getScoreDuration             (void) const;

		// barline/measure functionality:
		int           getBarlineCount              (void) const;
		HumdrumLine*  getBarline                   (int index) const;
		HumNum        getBarlineDuration           (int index) const;
		HumNum        getBarlineDurationFromStart  (int index) const;
		HumNum        getBarlineDurationToEnd      (int index) const;

	protected:
		bool          analyzeTokens                (void);
		bool          analyzeSpines                (void);
		bool          analyzeRhythm                (void);
		bool          analyzeMeter                 (void);
		bool          analyzeLinks                 (void);
		bool          analyzeTokenDurations        (void);
		bool          analyzeTracks                (void);
		bool          analyzeLines                 (void);
		bool          analyzeGlobalParameters      (void);
		bool          analyzeLocalParameters       (void);
		bool          analyzeDurationsOfNonRhythmicSpines(void);
		bool          adjustSpines                 (HumdrumLine& line,
		                                            vector<string>& datatype,
		                                            vector<string>& sinfo);
		string        getMergedSpineInfo           (vector<string>& info,
		                                            int starti, int extra);
		bool          stitchLinesTogether          (HumdrumLine& previous,
		                                            HumdrumLine& next);
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
		void          addToTrackStarts             (HumdrumToken* token);
		bool          prepareDurations             (HumdrumToken* token,
		                                            int state,
		                                            HumNum startdur);
		bool          setLineDurationFromStart     (HumdrumToken* token,
		                                            HumNum dursum);
		bool      analyzeRhythmOfFloatingSpine (HumdrumToken* spinestart);
		bool      analyzeNullLineRhythms       (void);
		void      fillInNegativeStartTimes     (void);
		void      assignLineDurations          (void);
		bool      assignDurationsToNonRhythmicTrack(HumdrumToken* starttoken);
		bool      analyzeNonNullDataTokens     (void);
		void      addUniqueTokens              (vector<HumdrumToken*>& target,
		                                        vector<HumdrumToken*>& source);
		bool      processNonNullDataTokensForTrackForward(
		                                        HumdrumToken* starttoken,
		                                        vector<HumdrumToken*> ptokens);
		bool      processNonNullDataTokensForTrackBackward(
		                                        HumdrumToken* starttoken,
		                                        vector<HumdrumToken*> ptokens);
		bool      processLocalParametersForTrack(HumdrumToken* starttok,
		                                         HumdrumToken* current);
		void      checkForLocalParameters       (HumdrumToken *token,
		                                         HumdrumToken *current);
		bool      assignDurationsToNonRhythmicTrack(HumdrumToken* endtoken,
		                                                  HumdrumToken* ptoken);

	protected:

		// lines: an array representing lines from the input file.
		vector<HumdrumLine*> lines;

		// trackstarts: list of addresses of the exclusive interpreations
		// in the file.  The first element in the list is reserved, so the
		// number of tracks (primary spines) is equal to one less than the
		// size of this list.
		vector<HumdrumToken*> trackstarts;

		// trackends: list of the addresses of the spine terminators in the file.
		// It is possible that spines can split and their subspines do not merge
		// before termination; therefore, the ends are stored in a 2d array.
		// The first dimension is the track number, and the second dimension
		// is the list of terminators.
		vector<vector<HumdrumToken*> > trackends;

		// barlines: list of barlines in the data.  If the first measures is
		// a pickup measure, then the first entry will not point to the first
		// starting exclusive interpretation line rather than to a barline.
		vector<HumdrumLine*> barlines;
		// Maybe also add "measures" which are complete metrical cycles.
};

ostream& operator<<(ostream& out, HumdrumFileBase& infile);


// END_MERGE

} // end namespace std;

#endif /* _HUMDRUMFILEBASE_H */



