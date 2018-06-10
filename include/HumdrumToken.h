//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Aug  8 12:24:49 PDT 2015
// Last Modified: Thu Nov 24 08:31:41 PST 2016 Added null token resolving
// Filename:      HumdrumToken.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/HumdrumToken.h
// Syntax:        C++11
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Used to work with Humdrum fields.
//

#ifndef _HUMDRUMTOKEN_H_INCLUDED
#define _HUMDRUMTOKEN_H_INCLUDED

#include <iostream>
#include <string>
#include <vector>

class HumParamSet;

#include "HumNum.h"
#include "HumAddress.h"
#include "HumHash.h"
#include "HumParamSet.h"

namespace hum {

// START_MERGE


typedef HumdrumToken* HTp;

class HumdrumToken : public std::string, public HumHash {
	public:
		         HumdrumToken              (void);
		         HumdrumToken              (const HumdrumToken& token);
		         HumdrumToken              (HumdrumToken* token);
		         HumdrumToken              (const HumdrumToken& token,
		                                    HumdrumLine* owner);
		         HumdrumToken              (HumdrumToken* token,
		                                    HumdrumLine* owner);
		         HumdrumToken              (const char* token);
		         HumdrumToken              (const std::string& token);
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
		bool     isExclusive               (void) const
		                                  { return isExclusiveInterpretation(); }
		bool     isExInterp                (void) const
		                                  { return isExclusiveInterpretation(); }
		bool     isSplit                   (void) const
		                                      { return isSplitInterpretation(); }
		bool     isMerge                   (void) const
		                                      { return isMergeInterpretation(); }
		bool     isExchange                (void) const
		                                   { return isExchangeInterpretation(); }
		bool     isTerminate               (void) const
		                                  { return isTerminateInterpretation(); }
		bool     isTerminator              (void) const
		                                  { return isTerminateInterpretation(); }
		bool     isAdd                     (void) const
		                                      { return isSplitInterpretation(); }

		bool     isBarline                 (void) const;
		bool     isCommentLocal            (void) const;
		bool     isCommentGlobal           (void) const;
		bool     isComment                 (void) const;
		bool     isData                    (void) const;
		bool     isInterpretation          (void) const;
		bool     isNonNullData             (void) const;
		bool     isNullData                (void) const;
		bool     isChord                   (const std::string& separator = " ");
		bool     isLabel                   (void) const;
		bool     hasRhythm                 (void) const;
		bool     hasBeam                   (void) const;
		bool     equalTo                   (const std::string& pattern);

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
		bool     hasLigatureBegin          (void);
		bool     hasLigatureEnd            (void);

		HumNum   getDuration               (void) const;
		HumNum   getDuration               (HumNum scale) const;
		HumNum   getTiedDuration           (void);
		HumNum   getTiedDuration           (HumNum scale);
		HumNum   getDurationNoDots         (void) const;
		HumNum   getDurationNoDots         (HumNum scale) const;
		int      getDots                   (char separator = ' ') const;

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
		const std::string& getDataType          (void) const;
		bool     isDataType                (const std::string& dtype) const;
		bool     isKern                    (void) const;
		bool     isMens                    (void) const;
		std::string   getSpineInfo              (void) const;
		int      getTrack                  (void) const;
		int      getSubtrack               (void) const;
		bool     noteInLowerSubtrack       (void);
		std::string   getTrackString            (void) const;
		int      getSubtokenCount          (const std::string& separator = " ") const;
		std::string   getSubtoken               (int index,
		                                    const std::string& separator = " ") const;
		void     setParameters             (HTp ptok);
		void     setParameters             (const std::string& pdata, HTp ptok = NULL);
		int      getStrandIndex            (void) const;
		int      getSlurStartElisionLevel  (int index = 0) const;
		int      getSlurEndElisionLevel    (int index = 0) const;
		HTp      getSlurStartToken         (int number = 0);
		HTp      getSlurEndToken           (int number = 0);
		void     storeLinkedParameters     (void);
		bool     linkedParameterIsGlobal   (int index);
		std::ostream& printCsv                  (std::ostream& out = std::cout);
		std::ostream& printXml                  (std::ostream& out = std::cout, int level = 0,
		                                    const std::string& indent = "\t");
		std::ostream& printGlobalXmlParameterInfo(std::ostream& out = std::cout, int level = 0,
		                                   const std::string& indent = "\t");
		std::string   getXmlId                  (const std::string& prefix = "") const;
		std::string   getXmlIdPrefix            (void) const;
		void     setText                   (const std::string& text);
		std::string   getText                   (void) const;
		int      addLinkedParameter        (HTp token);
		int      getLinkedParameterCount   (void);
		HumParamSet* getLinkedParameter    (int index);
		HumParamSet* getLinkedParameter    (void);
		std::ostream& printXmlLinkedParameterInfo(std::ostream& out, int level, const std::string& indent);

		// layout parameter accessors
		std::string   getVisualDuration         (void);

		HumdrumToken& operator=            (HumdrumToken& aToken);
		HumdrumToken& operator=            (const std::string& aToken);
		HumdrumToken& operator=            (const char* aToken);

		// next/previous token functions:
		int         getNextTokenCount      (void) const;
		int         getPreviousTokenCount  (void) const;
		HTp         getNextToken           (int index = 0) const;
		HTp         getPreviousToken       (int index = 0) const;
		std::vector<HTp> getNextTokens     (void) const;
		std::vector<HTp> getPreviousTokens (void) const;

		// next/previous token on line:
		HTp      getNextFieldToken           (void) const;
		HTp      getPreviousFieldToken       (void) const;

		int      getPreviousNonNullDataTokenCount(void);
		int      getPreviousNNDTCount      (void)
		                           { return getPreviousNonNullDataTokenCount(); }
		HTp      getPreviousNonNullDataToken(int index = 0);
		HTp      getPreviousNNDT           (int index = 0)
		                           { return getPreviousNonNullDataToken(index); }
		int      getNextNonNullDataTokenCount(void);
		int      getNextNNDTCount          (void)
		                               { return getNextNonNullDataTokenCount(); }
		HTp      getNextNonNullDataToken   (int index = 0);
		HTp      getNextNNDT               (int index = 0)
		                               { return getNextNonNullDataToken(index); }

		// slur-analysis based functions:
		HumNum   getSlurDuration           (HumNum scale = 1);

		void     setTrack                  (int aTrack, int aSubtrack);
		void     setTrack                  (int aTrack);

	protected:
		void     setLineIndex              (int lineindex);
		void     setFieldIndex             (int fieldlindex);
		void     setSpineInfo              (const std::string& spineinfo);
		void     setSubtrack               (int aSubtrack);
		void     setSubtrackCount          (int count);
		void     setPreviousToken          (HTp aToken);
		void     setNextToken              (HTp aToken);
		void     addNextNonNullToken       (HTp token);
		void     makeForwardLink           (HumdrumToken& nextToken);
		void     makeBackwardLink          (HumdrumToken& previousToken);
		void     setOwner                  (HumdrumLine* aLine);
		int      getState                  (void) const;
		void     incrementState            (void);
		void     setDuration               (const HumNum& dur);
		void     setStrandIndex            (int index);

		bool     analyzeDuration           (std::string& err);
		std::ostream& printXmlBaseInfo     (std::ostream& out = std::cout, int level = 0,
		                                    const std::string& indent = "\t");
		std::ostream& printXmlContentInfo  (std::ostream& out = std::cout, int level = 0,
		                                    const std::string& indent = "\t");
		std::ostream& printXmlStructureInfo(std::ostream& out = std::cout, int level = 0,
		                                    const std::string& indent = "\t");
		std::ostream& printXmlParameterInfo(std::ostream& out = std::cout, int level = 0,
		                                    const std::string& indent = "\t");
		std::ostream&	printXmlLinkedParameters (std::ostream& out = std::cout, int level = 0,
		                                    const std::string& indent = "\t");

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
		std::vector<HTp> m_nextTokens;     // link to next token(s) in spine

		// previousTokens: Simiar to nextTokens, but for the immediately
		// follow token(s) in the data.  Typically there will be one
		// preceding token, but there can be multiple tokens when the previous
		// line has *v merge tokens for the spine.  Exclusive interpretations
		// have no tokens preceding them.
		std::vector<HTp> m_previousTokens; // link to last token(s) in spine

		// nextNonNullTokens: This is a list of non-tokens in the spine
		// that follow this one.
		std::vector<HTp> m_nextNonNullTokens;

		// previousNonNullTokens: This is a list of non-tokens in the spine
		// that preced this one.
		std::vector<HTp> m_previousNonNullTokens;

		// rhycheck: Used to perfrom HumdrumFileStructure::analyzeRhythm
		// recursively.
		int m_rhycheck;

		// strand: Used to keep track of contiguous voice connections between
		// secondary spines/tracks.  This is the 1-D strand index number
		// (not the 2-d one).
		int m_strand;

		// m_nullresolve: used to point to the token that a null token
		// refers to.
		HTp m_nullresolve;

		// m_linkedParameters: List of Humdrum tokens which are parameters
		// (mostly only layout parameters at the moment).
		std::vector<HTp> m_linkedParameters;

		// m_linkedParameter: A single parameter encoded in the text of the
		// token.
		HumParamSet* m_linkedParameter = NULL;

	friend class HumdrumLine;
	friend class HumdrumFileBase;
	friend class HumdrumFileStructure;
	friend class HumdrumFileContent;
	friend class HumdrumFile;
};


std::ostream& operator<<(std::ostream& out, const HumdrumToken& token);
std::ostream& operator<<(std::ostream& out, HTp token);

std::ostream& printSequence(std::vector<std::vector<HTp> >& sequence, std::ostream& out=std::cout);
std::ostream& printSequence(std::vector<HTp>& sequence, std::ostream& out = std::cout);


// END_MERGE

} // end namespace hum

#endif /* _HUMDRUMTOKEN_H_INCLUDED */



