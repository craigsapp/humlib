//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Aug  8 12:24:49 PDT 2015
// Last Modified: Sun Aug  9 21:03:12 PDT 2015
// Filename:      HumdrumToken.h
// URL:           https://github.com/craigsapp/minHumdrum/blob/master/include/HumdrumToken.h
// Syntax:        C++11
// vim:           ts=3 noexpandtab
//
// Description:   Used to work with Humdrum fields.
//

#ifndef _HUMDRUM_TOKEN_H
#define _HUMDRUM_TOKEN_H

#include <iostream>
#include <vector>

#include "HumNum.h"
#include "HumAddress.h"
#include "HumHash.h"

using namespace std;

namespace minHumdrum {

// START_MERGE

class HumdrumToken : public string, public HumHash {
	public:
		         HumdrumToken              (void);
		         HumdrumToken              (const char* token);
		         HumdrumToken              (const string& token);
		        ~HumdrumToken              ();

		bool     isNull                    (void) const;
		bool     isManipulator             (void) const;
		bool     isExclusive               (void) const;
		bool     isExclusiveInterpretation (void) const { return isExclusive(); }
		bool     isExInterp                (void) const { return isExclusive(); }
		bool     isSplitInterpretation     (void) const;
		bool     isMergeInterpretation     (void) const;
		bool     isExchangeInterpretation  (void) const;
		bool     isTerminateInterpretation (void) const;
		bool     isAddInterpretation       (void) const;
		bool     isBarline                 (void) const;
		bool     isCommentLocal            (void) const;
		bool     isData                    (void) const;
		bool     hasRhythm                 (void) const;
		HumNum   getDuration               (void) const;
		HumNum   getDurationFromStart      (void) const;
		HumdrumLine* getOwner              (void) const;
		HumdrumLine* getLine               (void) const { return getOwner(); }
		bool     equalChar                 (int index, char ch) const;

		int      getPreviousNonNullDataTokenCount(void);
		int      getPreviousNNDTCount(void) {
		               return getPreviousNonNullDataTokenCount(); }
		HumdrumToken* getPreviousNonNullDataToken(int index);
		HumdrumToken* getPreviousNNDT(int index) {
		               return getPreviousNonNullDataToken(index); }
		int      getNextNonNullDataTokenCount(void);
		int      getNextNNDTCount(void) { return getNextNonNullDataTokenCount(); }
		HumdrumToken* getNextNonNullDataToken(int index);
		HumdrumToken* getNextNNDT(int index) {
		               return getNextNonNullDataToken(index); }

		int      getLineIndex              (void) const;
		int      getLineNumber             (void) const;
		int      getFieldIndex             (void) const;
		const string& getDataType          (void) const;
		string   getSpineInfo              (void) const;
		int      getTrack                  (void) const;
		int      getSubtrack               (void) const;
		string   getTrackString            (void) const;
		int      getSubtokenCount          (const string& separator = " ") const;
		string   getSubtoken               (int index,
		                                    const string& separator) const;
		void     setParameters             (HumdrumToken* ptok);
		void     setParameters             (const string& pdata);

		// next/previous token functions:
		int           getNextTokenCount         (void) const;
		int           getPreviousTokenCount     (void) const;
		HumdrumToken* getNextToken              (int index = 0) const;
		HumdrumToken* getPreviousToken          (int index = 0) const;
		vector<HumdrumToken*> getNextTokens     (void) const;
		vector<HumdrumToken*> getPreviousTokens (void) const;

	protected:
		void     setLineIndex      (int lineindex);
		void     setFieldIndex     (int fieldlindex);
		void     setSpineInfo      (const string& spineinfo);
		void     setTrack          (int aTrack, int aSubtrack);
		void     setTrack          (int aTrack);
		void     setSubtrack       (int aSubtrack);
		void     setPreviousToken  (HumdrumToken* aToken);
		void     setNextToken      (HumdrumToken* aToken);
		void     makeForwardLink   (HumdrumToken& nextToken);
		void     makeBackwardLink  (HumdrumToken& previousToken);
		void     setOwner          (HumdrumLine* aLine);
		int      getState          (void) const;
		void     incrementState    (void);
		void     setDuration       (const HumNum& dur);

		bool     analyzeDuration   (void);

	private:

		// address: The address contains information about the location of
		// the token on a HumdrumLine and in a HumdrumFile.
		HumAddress address;

		// duration: The duration of the token.  Non-rhythmic data types
		// will have a negative duration (which should be interpreted
		// as a zero duration--See HumdrumToken::hasRhythm()).
		// Grace note will have a zero duration, even if they have a duration
		// list in the token for a graphical display duration.
		HumNum duration;

		// nextTokens: This is a list of all previous tokens in the spine which
		// immediately precede this token. Typically there will be one
		// following token, but there can be two tokens if the current
		// token is *^, and there will be zero following tokens after a
		// spine terminating token (*-).
		vector<HumdrumToken*> nextTokens;     // link to next token(s) in spine

		// previousTokens: Simiar to nextTokens, but for the immediately
		// follow token(s) in the data.  Typically there will be one
		// preceding token, but there can be multiple tokens when the previous
		// line has *v merge tokens for the spine.  Exclusive interpretations
		// have no tokens preceding them.
		vector<HumdrumToken*> previousTokens; // link to last token(s) in spine

		// nextNonNullTokens: This is a list of non-tokens in the spine
		// that follow this one.
		vector<HumdrumToken*> nextNonNullTokens;

		// previousNonNullTokens: This is a list of non-tokens in the spine
		// that preced this one.
		vector<HumdrumToken*> previousNonNullTokens;

		// rhycheck: Used to perfrom HumdrumFileStructure::analyzeRhythm
		// recursively.
		int rhycheck;

	friend class HumdrumLine;
	friend class HumdrumFileBase;
	friend class HumdrumFileStructure;
	friend class HumdrumFileContent;
	friend class HumdrumFile;
};


ostream& operator<<(ostream& out, const HumdrumToken& token);


// END_MERGE

} // end namespace std;

#endif /* _HUMDRUM_TOKEN_H */



