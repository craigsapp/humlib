//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Aug  8 12:24:49 PDT 2015
// Last Modified: Sun Aug  9 21:03:12 PDT 2015
// Filename:      HumdrumToken.h
// URL:           https://github.com/craigsapp/minHumdrum/blob/master/include/HumdrumToken.h
// Syntax:        C++11
// vim:           ts=3
//
// Description:   Used to work with Humdrum fields.
//

#ifndef _HUMDRUM_TOKEN_H
#define _HUMDRUM_TOKEN_H

#include <iostream>
#include <vector>

#include "HumNum.h"
#include "HumAddress.h"

using namespace std;

// START_MERGE

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
		int      getNextTokenCount     (void);
		int      getPreviousTokenCount (void);

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

// END_MERGE

#endif /* _HUMDRUM_TOKEN_H */



