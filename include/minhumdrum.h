//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Aug  8 12:24:49 PDT 2015
// Last Modified: Wed Aug 12 17:44:39 PDT 2015
// Filename:      /include/minhumdrum.h
// URL:           https://github.com/craigsapp/minHumdrum/blob/master/include/minhumdrum.h
// Syntax:        C++11
// vim:           ts=3
//
// Description:   Include file for minHumdrum library.
//

#ifndef _MINHUMDRUM_H
#define _MINHUMDRUM_H

#include <math.h>

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>

using std::string;
using std::vector;
using std::istream;
using std::ifstream;
using std::ostream;
using std::cout;
using std::cin;
using std::cerr;
using std::endl;
using std::to_string;
using std::stringstream;

class Convert;
class HumNum;
class HumAddress;
class HumdrumToken;
class HumdrumLine;
class HumdrumFile;


class HumAddress {
	public:
		         HumAddress        (void);
		        ~HumAddress        ();

		int      getLineIndex      (void) const;
		int      getLineNumber     (void) const;
		int      getFieldIndex     (void) const;
		string   getDataType       (void) const;
		string   getSpineInfo      (void) const;
		int      getTrack          (void) const;
		int      getSubtrack       (void) const;
		string   getTrackString    (void) const;

	protected:
		void     setLineAddress    (int aLineIndex, int aFieldIndex);
		void     setLineIndex      (int lineindex);
		void     setFieldIndex     (int fieldlindex);
		void     setDataType       (const string& datatype);
		void     setSpineInfo      (const string& spineinfo);
		void     setTrack          (int aTrack, int aSubtrack);
		void     setTrack          (int aTrack);
		void     setSubtrack       (int aSubtrack);

	private:
		int     lineindex;        // Humdrum data line index of token
		int     fieldindex;       // field index of token on line
		string  exinterp;         // data type of token
		string  spining;          // spine manipulation history of token
		int     track;            // track # (starting at 1 for first spine);
		int     subtrack;         // subtrack # (starting at 1 for first track, or
                                // zero if there are no subtracks.

	friend class HumdrumToken;
	friend class HumdrumLine;
	friend class HumdrumFile;
};


class HumNum {
	public:
		         HumNum         (void);
		         HumNum         (int value);
		         HumNum         (int numerator, int denominator);
		         HumNum         (const HumNum& rat);
		        ~HumNum         ();

		bool     isNegative     (void) const;
		bool     isPositive     (void) const;
		bool     isZero         (void) const;
		bool     isNonZero      (void) const;
		bool     isNonNegative  (void) const;
		bool     isNonPositive  (void) const;
		bool     isInfinite     (void) const;
		bool     isFinite       (void) const;
		bool     isNaN          (void) const;
		bool     isInteger      (void) const;
		double   getFloat       (void) const;
		int      getInteger     (double round = 0.0) const;
		int      getNumerator   (void) const;
		int      getDenominator (void) const;
		void     setValue       (int numerator);
		void     setValue       (int numerator, int denominator);
		HumNum   getAbs         (void) const;
		HumNum&  makeAbs        (void);
		HumNum&  operator=      (const HumNum& value);
		HumNum&  operator=      (int value);
		HumNum&  operator+=     (const HumNum& value);
		HumNum&  operator+=     (int value);
		HumNum&  operator-=     (const HumNum& value);
		HumNum&  operator-=     (int value);
		HumNum&  operator*=     (const HumNum& value);
		HumNum&  operator*=     (int value);
		HumNum&  operator/=     (const HumNum& value);
		HumNum&  operator/=     (int value);
		HumNum   operator-      (void);
		HumNum   operator+      (const HumNum& value);
		HumNum   operator+      (int value);
		HumNum   operator-      (const HumNum& value);
		HumNum   operator-      (int value);
		HumNum   operator*      (const HumNum& value);
		HumNum   operator*      (int value);
		HumNum   operator/      (const HumNum& value);
		HumNum   operator/      (int value);
		bool     operator==     (const HumNum& value) const;
		bool     operator==     (double value) const;
		bool     operator==     (int value) const;
		bool     operator<      (const HumNum& value) const;
		bool     operator<      (double value) const;
		bool     operator<      (int value) const;
		bool     operator<=     (const HumNum& value) const;
		bool     operator<=     (double value) const;
		bool     operator<=     (int value) const;
		bool     operator>      (const HumNum& value) const;
		bool     operator>      (double value) const;
		bool     operator>      (int value) const;
		bool     operator>=     (const HumNum& value) const;
		bool     operator>=     (double value) const;
		bool     operator>=     (int value) const;
		ostream& printFraction  (ostream& = cout) const;
		ostream& printMixedFraction (ostream& out = cout,
		                         string separator = "_") const;
      ostream& printList      (ostream& out) const;

	protected:
		void     reduce         (void);
		int      gcdIterative   (int a, int b);
		int      gcdRecursive   (int a, int b);

	private:
		int top;
		int bot;
};

ostream& operator<<(ostream& out, const HumNum& number);

template <typename A>
ostream& operator<<(ostream& out, const vector<A>& v);


class HumdrumFile {
	public:
		             HumdrumFile            (void);
		            ~HumdrumFile            ();

		bool         read                   (istream& infile);
		bool         read                   (const char*   contents);
		bool         read                   (const string& contents);
		bool         readString             (const char*   contents);
		bool         readString             (const string& contents);
		void         createTokensFromLines  (void);
		void         createLinesFromTokens  (void);
		HumdrumLine& operator[]             (int index);
		void         append                 (const char* line);
		void         append                 (const string& line);
		int          size                   (void) const;
		int          getMaxTrack            (void) const;
		ostream&     printSpineInfo         (ostream& out = cout);
		ostream&     printDataTypeInfo      (ostream& out = cout);
		ostream&     printTrackInfo         (ostream& out = cout);
		ostream&     printDurationInfo      (ostream& out = cout);

	protected:
		bool         analyzeSpines          (void);
		bool         analyzeRhythm          (void);
		bool         analyzeMeter           (void);
		bool         analyzeLinks           (void);
		bool         analyzeTokenDurations  (void);
		bool         analyzeTracks          (void);
		bool         analyzeLines           (void);
		int          adjustSpines           (HumdrumLine& line,
		                                     vector<string>& datatype,
		                                     vector<string>& sinfo,
		                                     int trackcount);
		string       getMergedSpineInfo     (vector<string>& info, int starti,
		                                     int extra);
		bool         stitchLinesTogether    (HumdrumLine& previous,
		                                     HumdrumLine& next);
		HumNum       getMinDur              (vector<HumNum>& durs,
		                                     vector<HumNum>& durstate);
      bool         getTokenDurations      (vector<HumNum>& durs, int line);
		bool         cleanDurs              (vector<HumNum>& durs, int line);
		bool         decrementDurStates     (vector<HumNum>& durs, HumNum linedur,
		                                     int line);

	private:
		vector<HumdrumLine*> lines;
		int          maxtrack;              // the number of tracks (primary spines) in the data.
};

ostream& operator<<(ostream& out, HumdrumFile& infile);



class HumdrumLine : public string {
	public:
		         HumdrumLine          (void);
		         HumdrumLine          (const string& aString);
		         HumdrumLine          (const char* aString);
		        ~HumdrumLine          ();

		bool     isComment              (void) const;
		bool     isCommentLocal         (void) const;
		bool     isCommentGlobal        (void) const;
		bool     isExclusive            (void) const;
		bool     isExclusiveInterpretation (void) const { return isExclusive(); }
		bool     isTerminator           (void) const;
		bool     isInterp               (void) const;
		bool     isInterpretation       (void) const { return isInterp(); }
		bool     isBarline              (void) const;
		bool     isData                 (void) const;
		bool     isEmpty                (void) const;
		bool     isManipulator          (void) const;
		bool     hasSpines              (void) const;
      HumdrumToken& token             (int index);
		void     getTokens              (vector<HumdrumToken*>& list);
		int      getTokenCount          (void) const;
      string   getTokenString         (int index) const;
		bool     equalChar              (int index, char ch) const;
		char     getChar                (int index) const;
		ostream& printSpineInfo         (ostream& out = cout);
		ostream& printDataType          (ostream& out = cout);
		ostream& printTrackInfo         (ostream& out = cout);
		ostream& printDataTypeInfo      (ostream& out = cout);
		ostream& printDurationInfo      (ostream& out = cout);
		int      createTokensFromLine   (void);
		void     createLineFromTokens   (void);
		int      getLineIndex           (void) const;
		int      getLineNumber          (void) const;
		HumNum   getDuration            (void) const;
		HumNum   getDurationFromStart   (void) const;
		HumNum   getDurationFromBarline (void) const;
		HumNum   getDurationToBarline   (void) const;
		HumNum   getBeat                (string beatrecip = "4") const;

	protected:
		bool     analyzeTracks          (void);
		bool     analyzeTokenDurations  (void);
		void     setLineIndex           (int index);
		void     clear                  (void);
		void     setDuration            (HumNum aDur);
		void     setDurationFromStart   (HumNum dur);
		void     setDurationFromBarline (HumNum dur);
		void     setDurationToBarline   (HumNum dur);

	private:

		//
		// State variables managed by the HumdrumLine class:
		//

		// lineindex: Used to store the index number of the HumdrumLine in
		// the owning HumdrumFile object.
		// This variable is filled by HumdrumFile::analyzeLines().
		int lineindex;

		// tokens: Used to store the individual tab-separated token fields
		// on a line.  These are prepared automatically after reading in
		// a full line of text (which is accessed throught the string parent
		// class).  If the full line is changed, the tokens are not updated
		// automatically -- use createTokensFromLine().  Likewise the full
		// text line is not updated if any tokens are changed -- use
		// createLineFromTokens() in that case.  The second case is more
		// useful: you can read in a HumdrumFile, tweak the tokens, then
		// reconstruct the full line and print out again.
		// This variable is filled by HumdrumFile::read().
		vector<HumdrumToken*> tokens;

		// duration: This is the "duration" of a line.  The duration is
		// equal to the minimum time unit of all durational tokens on the
		// line.  This also includes null tokens when the duration of a
		// previous note in a previous spine is ending on the line, so it is
		// not just the minimum duration on the line.
		// This variable is filled by HumdrumFile::analyzeRhythm().
		HumNum duration;

		// durationFromStart: This is the cumulative duration of all lines
		// prior to this one in the owning HumdrumFile object.  For example,
		// the first notes in a score start at time 0, If the duration of the
		// first data line is 1 quarter note, then the durationFromStart for
		// the second line will be 1 quarter note.
		// This variable is filled by HumdrumFile::analyzeRhythm().
		HumNum durationFromStart;

		// durationFromBarline: This is the cumulative duration from the
		// last barline to the current data line.  
		// This variable is filled by HumdrumFile::analyzeMeter().
		HumNum durationFromBarline;

		// durationToBarline: This is the duration from the start of the
		// current line to the next barline in the owning HumdrumFile object.
		// This variable is filled by HumdrumFile::analyzeMeter().
		HumNum durationToBarline;

	friend class HumdrumFile;
};

ostream& operator<< (ostream& out, HumdrumLine& line);



class HumdrumToken : public string {
	public:
		         HumdrumToken      (void);
		         HumdrumToken      (const char* token);
		         HumdrumToken      (const string& token);
		        ~HumdrumToken      ();

		bool     isNull            (void) const;
		bool     isManipulator     (void) const;
		bool     isExclusive       (void) const;
		bool     isExclusiveInterpretation(void) const { return isExclusive(); }
		bool     isExInterp        (void) const { return isExclusive(); }
		bool     isSplitInterpretation     (void) const;
		bool     isMergeInterpretation     (void) const;
		bool     isExchangeInterpretation  (void) const;
		bool     isTerminateInterpretation (void) const;
		bool     isAddInterpretation       (void) const;
		HumNum   getDuration       (void) const;
		bool     hasRhythm         (void) const;
		bool     equalChar         (int index, char ch) const;

		int      getLineIndex      (void) const;
		int      getLineNumber     (void) const;
		int      getFieldIndex     (void) const;
		string   getDataType       (void) const;
		string   getSpineInfo      (void) const;
		int      getTrack          (void) const;
		int      getSubtrack       (void) const;
		string   getTrackString    (void) const;
		int      getSubtokenCount  (const string& separator = " ") const;
      string   getSubtoken       (int index, const string& separator) const;

		// next/previous token functions:
		int           getNextTokenCount         (void) const;
		int           getPreviousTokenCount     (void) const;
		HumdrumToken* getNextToken              (int index = 0) const;
		HumdrumToken* getPreviousToken          (int index = 0) const;
		vector<HumdrumToken*> getNextTokens     (void) const;
		vector<HumdrumToken*> getPreviousTokens (void) const;

	protected:
		void     setLineAddress    (int aLineIndex, int aFieldIndex);
		void     setLineIndex      (int lineindex);
		void     setFieldIndex     (int fieldlindex);
		void     setDataType       (const string& datatype);
		void     setSpineInfo      (const string& spineinfo);
		void     setTrack          (int aTrack, int aSubtrack);
		void     setTrack          (int aTrack);
		void     setSubtrack       (int aSubtrack);
		void     setPreviousToken  (HumdrumToken* aToken);
		void     setNextToken      (HumdrumToken* aToken);
		void     makeForwardLink   (HumdrumToken& nextToken);
      void     makeBackwardLink  (HumdrumToken& previousToken);

		bool     analyzeDuration   (void);

	private:

		HumAddress address;   // location of the token in the score
		HumNum duration;      // duration of token; negative if undefined
		vector<HumdrumToken*> nextTokens;     // link to next token(s) in spine
		vector<HumdrumToken*> previousTokens; // link to last token(s) in spine

	friend class HumdrumLine;
	friend class HumdrumFile;
};


class Convert {
	public:
		static HumNum    recipToDuration  (const string& recip, HumNum scale = 4,
		                                    string separator = " ");
};





#endif /* _MINHUMDRUM_H */

