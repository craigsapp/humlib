//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Aug  8 12:24:49 PDT 2015
// Last Modified: Thu Nov 24 08:31:41 PST 2016 Added null token resolving
// Filename:      HumdrumToken.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/HumdrumToken.h
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

namespace hum {

// START_MERGE


typedef HumdrumToken* HTp;

class HumdrumToken : public string, public HumHash {
	public:
		         HumdrumToken              (void);
		         HumdrumToken              (const HumdrumToken& token);
		         HumdrumToken              (const char* token);
		         HumdrumToken              (const string& token);
		        ~HumdrumToken              ();

		bool     isNull                    (void) const;
		bool     isManipulator             (void) const;

		bool     isExclusiveInterpretation (void) const;
		bool     isSplitInterpretation     (void) const;
		bool     isMergeInterpretation     (void) const;
		bool     isExchangeInterpretation  (void) const;
		bool     isTerminateInterpretation (void) const;
		bool     isAddInterpretation       (void) const;

		// alises for the above
		bool     isExclusive     (void) const {
		                                   return isExclusiveInterpretation(); }
		bool     isExInterp      (void) const {
		                                   return isExclusiveInterpretation(); }
		bool     isSplit         (void) const { return isSplitInterpretation(); }
		bool     isMerge         (void) const { return isMergeInterpretation(); }
		bool     isExchange      (void) const {
		                                   return isExchangeInterpretation(); }
		bool     isTerminate     (void) const {
		                                   return isTerminateInterpretation(); }
		bool     isTerminator    (void) const {
		                                   return isTerminateInterpretation(); }
		bool     isAdd           (void) const { return isSplitInterpretation(); }

		bool     isBarline                 (void) const;
		bool     isCommentLocal            (void) const;
		bool     isComment                 (void) const;
		bool     isData                    (void) const;
		bool     isInterpretation          (void) const;
		bool     isNonNullData             (void) const;
		bool     isNullData                (void) const;
		bool     isChord                   (const string& separator = " ");
		bool     isLabel                   (void) const;
		bool     hasRhythm                 (void) const;
		bool     hasBeam                   (void) const;

		// kern-specific functions:
		bool     isRest                    (void);
		bool     isNote                    (void);
		bool     isSecondaryTiedNote       (void);
		bool     isSustainedNote           (void);
		bool     isNoteAttack              (void);
		bool     isInvisible               (void);
		bool     isGrace                   (void);
		bool     isClef                    (void);
		bool     isKeySignature            (void);
		bool     isKeyDesignation          (void);
		bool     isTimeSignature           (void);
		bool     isMensurationSymbol       (void);

		bool     hasSlurStart              (void);
		bool     hasSlurEnd                (void);
		int      hasVisibleAccidental      (int subtokenIndex) const;
		int      hasCautionaryAccidental   (int subtokenIndex) const;

		HumNum   getDuration               (void) const;
		HumNum   getDuration               (HumNum scale) const;
		HumNum   getDurationNoDots         (void) const;
		HumNum   getDurationNoDots         (HumNum scale) const;
		int      getDots                   (void) const;

		HumNum   getDurationFromStart      (void) const;
		HumNum   getDurationFromStart      (HumNum scale) const;

		HumNum   getDurationToEnd          (void) const;
		HumNum   getDurationToEnd          (HumNum scale) const;

		HumNum   getDurationFromBarline    (void) const;
		HumNum   getDurationFromBarline    (HumNum scale) const;

		HumNum   getDurationToBarline      (void) const;
		HumNum   getDurationToBarline      (HumNum scale) const;

		HumNum   getBarlineDuration        (void) const;
		HumNum   getBarlineDuration        (HumNum scale) const;

		HumdrumLine* getOwner              (void) const;
		HumdrumLine* getLine               (void) const { return getOwner(); }
		bool     equalChar                 (int index, char ch) const;

		HTp      resolveNull               (void);
		void     setNullResolution         (HTp resolution);
		int      getLineIndex              (void) const;
		int      getLineNumber             (void) const;
		int      getFieldIndex             (void) const;
		int      getFieldNumber            (void) const;
		int      getTokenIndex             (void) const;
		int      getTokenNumber            (void) const;
		const string& getDataType          (void) const;
		bool     isDataType                (string dtype) const;
		bool     isKern                    (void) const;
		string   getSpineInfo              (void) const;
		int      getTrack                  (void) const;
		int      getSubtrack               (void) const;
		string   getTrackString            (void) const;
		int      getSubtokenCount          (const string& separator = " ") const;
		string   getSubtoken               (int index,
		                                    const string& separator = " ") const;
		void     setParameters             (HTp ptok);
		void     setParameters             (const string& pdata, HTp ptok = NULL);
		int      getStrandIndex            (void) const;
		int      getSlurStartElisionLevel  (void) const;
		int      getSlurEndElisionLevel    (void) const;
		HTp      getSlurStartToken         (void);
		HTp      getSlurEndToken           (void);
		ostream& printCsv                  (ostream& out = cout);
		ostream& printXml                  (ostream& out = cout, int level = 0,
		                                    const string& indent = "\t");
		string   getXmlId                  (const string& prefix = "") const;
		string   getXmlIdPrefix            (void) const;

		HumdrumToken& operator=            (HumdrumToken& aToken);
		HumdrumToken& operator=            (const string& aToken);
		HumdrumToken& operator=            (const char* aToken);

		// next/previous token functions:
		int           getNextTokenCount         (void) const;
		int           getPreviousTokenCount     (void) const;
		HTp           getNextToken              (int index = 0) const;
		HTp           getPreviousToken          (int index = 0) const;
		vector<HTp>   getNextTokens             (void) const;
		vector<HTp>   getPreviousTokens         (void) const;

		int      getPreviousNonNullDataTokenCount(void);
		int      getPreviousNNDTCount(void) {
		               return getPreviousNonNullDataTokenCount(); }
		HTp      getPreviousNonNullDataToken(int index = 0);
		HTp      getPreviousNNDT(int index) {
		               return getPreviousNonNullDataToken(index); }
		int      getNextNonNullDataTokenCount(void);
		int      getNextNNDTCount(void) { return getNextNonNullDataTokenCount(); }
		HTp      getNextNonNullDataToken(int index = 0);
		HTp      getNextNNDT(int index = 0) {
		               return getNextNonNullDataToken(index); }

		// slur-analysis based functions:
		HumNum   getSlurDuration   (HumNum scale = 1);

	protected:
		void     setLineIndex       (int lineindex);
		void     setFieldIndex      (int fieldlindex);
		void     setSpineInfo       (const string& spineinfo);
		void     setTrack           (int aTrack, int aSubtrack);
		void     setTrack           (int aTrack);
		void     setSubtrack        (int aSubtrack);
		void     setSubtrackCount   (int count);
		void     setPreviousToken   (HTp aToken);
		void     setNextToken       (HTp aToken);
		void     addNextNonNullToken(HTp token);
		void     makeForwardLink    (HumdrumToken& nextToken);
		void     makeBackwardLink   (HumdrumToken& previousToken);
		void     setOwner           (HumdrumLine* aLine);
		int      getState           (void) const;
		void     incrementState     (void);
		void     setDuration        (const HumNum& dur);
		void     setStrandIndex     (int index);

		bool     analyzeDuration   (string& err);
		ostream& printXmlBaseInfo  (ostream& out = cout, int level = 0,
		                            const string& indent = "\t");
		ostream& printXmlContentInfo(ostream& out = cout, int level = 0,
		                            const string& indent = "\t");
		ostream& printXmlStructureInfo(ostream& out = cout, int level = 0,
		                            const string& indent = "\t");
		ostream& printXmlParameterInfo(ostream& out = cout, int level = 0,
		                            const string& indent = "\t");

	private:

		// address: The address contains information about the location of
		// the token on a HumdrumLine and in a HumdrumFile.
		HumAddress m_address;

		// duration: The duration of the token.  Non-rhythmic data types
		// will have a negative duration (which should be interpreted
		// as a zero duration--See HumdrumToken::hasRhythm()).
		// Grace note will have a zero duration, even if they have a duration
		// list in the token for a graphical display duration.
		HumNum m_duration;

		// nextTokens: This is a list of all previous tokens in the spine which
		// immediately precede this token. Typically there will be one
		// following token, but there can be two tokens if the current
		// token is *^, and there will be zero following tokens after a
		// spine terminating token (*-).
		vector<HTp> m_nextTokens;     // link to next token(s) in spine

		// previousTokens: Simiar to nextTokens, but for the immediately
		// follow token(s) in the data.  Typically there will be one
		// preceding token, but there can be multiple tokens when the previous
		// line has *v merge tokens for the spine.  Exclusive interpretations
		// have no tokens preceding them.
		vector<HTp> m_previousTokens; // link to last token(s) in spine

		// nextNonNullTokens: This is a list of non-tokens in the spine
		// that follow this one.
		vector<HTp> m_nextNonNullTokens;

		// previousNonNullTokens: This is a list of non-tokens in the spine
		// that preced this one.
		vector<HTp> m_previousNonNullTokens;

		// rhycheck: Used to perfrom HumdrumFileStructure::analyzeRhythm
		// recursively.
		int m_rhycheck;

		// strand: Used to keep track of contiguous voice connections between
      // secondary spines/tracks.  This is the 1-D strand index number
		// (not the 2-d one).
		int m_strand;

		// m_nullresolve: used to point to the token that a null token
		// refers to
		HTp m_nullresolve;

	friend class HumdrumLine;
	friend class HumdrumFileBase;
	friend class HumdrumFileStructure;
	friend class HumdrumFileContent;
	friend class HumdrumFile;
};


ostream& operator<<(ostream& out, const HumdrumToken& token);
ostream& operator<<(ostream& out, HTp token);

ostream& printSequence(vector<vector<HTp> >& sequence, ostream& out=std::cout);
ostream& printSequence(vector<HTp>& sequence, ostream& out = std::cout);

// END_MERGE

} // end namespace hum

#endif /* _HUMDRUM_TOKEN_H */



