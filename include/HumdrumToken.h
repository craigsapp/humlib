//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Aug  8 12:24:49 PDT 2015
// Last Modified: Thu Nov 24 08:31:41 PST 2016 Added null token resolving
// Filename:      HumdrumToken.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/HumdrumToken.h
// Syntax:        C++11; humlib
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Used to work with Humdrum fields.
//

#ifndef _HUMDRUMTOKEN_H_INCLUDED
#define _HUMDRUMTOKEN_H_INCLUDED

#include <algorithm>
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
		         HumdrumToken              (const HumdrumToken& token, HLp owner);
		         HumdrumToken              (HumdrumToken* token, HLp owner);
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
		bool     isLocalComment            (void) const { return isCommentLocal(); }
		bool     isCommentGlobal           (void) const;
		bool     isGlobalComment           (void) const { return isCommentGlobal(); }
		bool     isComment                 (void) const;
		bool     isData                    (void) const;
		bool     isInterpretation          (void) const;
		bool     isNonNullData             (void) const;
		bool     isNullData                (void) const;
		bool     isChord                   (const std::string& separator = " ");
		bool     isLabel                   (void) const;
		bool     isExpansionList           (void) const;
		bool     hasRhythm                 (void) const;
		bool     hasBeam                   (void) const;
		bool     hasFermata                (void) const;
		bool     equalTo                   (const std::string& pattern);
		bool     isStaff                   (void) const;

		// kern-specific functions:
		bool     isRest                    (void);
		bool     isNote                    (void);
		bool     isUnpitched               (void);
		bool     isPitched                 (void);
		bool     isSecondaryTiedNote       (void);
		bool     isSustainedNote           (void);
		bool     isNoteSustain             (void) { return isSustainedNote(); }
		bool     isNoteAttack              (void);
		bool     isInvisible               (void);
		bool     isGrace                   (void);
		bool     isClef                    (void);
		bool     isModernClef              (void);
		bool     isOriginalClef            (void);
		bool     isKeySignature            (void);
		bool     isModernKeySignature      (void);
		bool     isOriginalKeySignature    (void);
		bool     isKeyDesignation          (void);
		bool     isTimeSignature           (void);
		bool     isMetricSymbol            (void);
		bool     isMeterSymbol             (void) { return isMetricSymbol(); }
		bool     isMeterSignature          (void) { return isMetricSymbol(); }
		bool     isMetricSignature         (void) { return isMetricSymbol(); }
		bool     isTempo                   (void);
		bool     isMensurationSymbol       (void);
		bool     isMensuration             (void) { return isMensurationSymbol(); }
		bool     isOriginalMensurationSymbol(void);
		bool     isModernMensurationSymbol (void);
		bool     isOriginalMensuration     (void) { return isOriginalMensurationSymbol(); }
		bool     isModernMensuration       (void) { return isModernMensurationSymbol(); }
		bool     isInstrumentDesignation   (void);
		bool     isInstrumentName          (void);
		bool     isInstrumentAbbreviation  (void);
		bool     isModernInstrumentName    (void);
		bool     isModernInstrumentAbbreviation(void);
		bool     isOriginalInstrumentName    (void);
		bool     isOriginalInstrumentAbbreviation(void);
		bool     isStria                   (void);

		std::string getInstrumentName        (void);
		std::string getInstrumentAbbreviation(void);

		bool     hasSlurStart              (void);
		bool     hasSlurEnd                (void);
		int      hasVisibleAccidental      (int subtokenIndex) const;
		int      hasCautionaryAccidental   (int subtokenIndex) const;
		bool     hasLigatureBegin          (void);
		bool     hasRectaLigatureBegin     (void);
		bool     hasObliquaLigatureBegin   (void);
		bool     hasLigatureEnd            (void);
		bool     hasRectaLigatureEnd       (void);
		bool     hasObliquaLigatureEnd     (void);
		char     hasStemDirection          (void);
		bool     allSameBarlineStyle       (void);


		// pitch-related functions (in HumdrumToken-midi.cpp):
		int              getMidiPitch         (void);
		void             getMidiPitches       (std::vector<int>& output);
		std::vector<int> getMidiPitches       (void);
		void             getMidiPitchesSortHL (std::vector<int>& output);
		std::vector<int> getMidiPitchesSortHL (void);
		void             getMidiPitchesSortLH (std::vector<int>& output);
		std::vector<int> getMidiPitchesSortLH (void);

		int              getMidiPitchResolveNull         (void);
		void             getMidiPitchesResolveNull       (std::vector<int>& output);
		std::vector<int> getMidiPitchesResolveNull       (void);
		void             getMidiPitchesResolveNullSortHL (std::vector<int>& output);
		std::vector<int> getMidiPitchesResolveNullSortHL (void);
		void             getMidiPitchesResolveNullSortLH (std::vector<int>& output);
		std::vector<int> getMidiPitchesResolveNullSortLH (void);

		// pitch-related functions (in HumdrumToken-base40.cpp):
		int              getBase40Pitch         (void);
		void             getBase40Pitches       (std::vector<int>& output);
		std::vector<int> getBase40Pitches       (void);
		void             getBase40PitchesSortHL (std::vector<int>& output);
		std::vector<int> getBase40PitchesSortHL (void);
		void             getBase40PitchesSortLH (std::vector<int>& output);
		std::vector<int> getBase40PitchesSortLH (void);

		int              getBase40PitchResolveNull         (void);
		void             getBase40PitchesResolveNull       (std::vector<int>& output);
		std::vector<int> getBase40PitchesResolveNull       (void);
		void             getBase40PitchesResolveNullSortHL (std::vector<int>& output);
		std::vector<int> getBase40PitchesResolveNullSortHL (void);
		void             getBase40PitchesResolveNullSortLH (std::vector<int>& output);
		std::vector<int> getBase40PitchesResolveNullSortLH (void);

		// duration-related functions:
		HumNum   getDuration               (void);
		HumNum   getDuration               (HumNum scale);
		HumNum   getTiedDuration           (void);
		HumNum   getTiedDuration           (HumNum scale);
		HumNum   getDurationNoDots         (void);
		HumNum   getDurationNoDots         (HumNum scale);
		int      getDots                   (char separator = ' ') const;

		HumNum   getDurationFromStart      (void);
		HumNum   getDurationFromStart      (HumNum scale);

		HumNum   getDurationToEnd          (void);
		HumNum   getDurationToEnd          (HumNum scale);

		HumNum   getDurationFromNoteStart  (void);
		HumNum   getDurationFromNoteStart  (HumNum scale);

		HumNum   getDurationToNoteEnd      (void);
		HumNum   getDurationToNoteEnd      (HumNum scale);

		HumNum   getDurationFromBarline    (void);
		HumNum   getDurationFromBarline    (HumNum scale);

		HumNum   getDurationToBarline      (void);
		HumNum   getDurationToBarline      (HumNum scale);

		HumNum   getBarlineDuration        (void);
		HumNum   getBarlineDuration        (HumNum scale);

		HLp      getOwner                  (void) const;
		HLp      getLine                   (void) const { return getOwner(); }
		bool     equalChar                 (int index, char ch) const;

		HTp      resolveNull               (void);
		void     setNullResolution         (HTp resolution);
		int      getLineIndex              (void) const;
		int      getLineNumber             (void) const;
		int      getFieldIndex             (void) const;
		int      getFieldNumber            (void) const;
		int      getTokenIndex             (void) const;
		int      getTokenNumber            (void) const;
		const std::string& getDataType     (void) const;
		const std::string& getExInterp     (void) { return getDataType(); }
		bool     isDataType                (const std::string& dtype) const;
		bool     isDataTypeLike            (const std::string& dtype) const;
		bool     isKern                    (void) const;
		bool     isKernLike                (void) const;
		bool     isMens                    (void) const;
		bool     isMensLike                (void) const;
		bool     isStaffLike               (void) const { return isKernLike() || isMensLike(); }
		std::string   getSpineInfo         (void) const;
		int      getTrack                  (void) const;
		int      getSubtrack               (void) const;
		bool     noteInLowerSubtrack       (void);
		std::string   getTrackString       (void) const;
		int      getSubtokenCount          (const std::string& separator = " ") const;
		std::string   getSubtoken          (int index,
		                                    const std::string& separator = " ") const;
		std::vector<std::string> getSubtokens (const std::string& separator = " ") const;
		void     replaceSubtoken           (int index, const std::string& newsubtok,
		                                    const std::string& separator = " ");
		void     setParameters             (HTp ptok);
		void     setParameters             (const std::string& pdata, HTp ptok = NULL);
		int      getStrandIndex            (void) const;

		int      getBeamStartElisionLevel  (int index = 0) const;
		int      getBeamEndElisionLevel    (int index = 0) const;

		int      getSlurStartElisionLevel  (int index = 0) const;
		int      getSlurEndElisionLevel    (int index = 0) const;

		int      getPhraseStartElisionLevel(int index) const;
		int      getPhraseEndElisionLevel  (int index = 0) const;

		HTp      getSlurStartToken         (int number = 1);
		int      getSlurStartNumber        (int endnumber);
		HTp      getSlurEndToken           (int number = 1);
		HTp      getPhraseStartToken       (int number = 1);
		HTp      getPhraseEndToken         (int number = 1);
		void     storeParameterSet         (void);
		bool     linkedParameterIsGlobal   (int index);
		std::ostream& printCsv             (std::ostream& out = std::cout);
		std::ostream& printXml             (std::ostream& out = std::cout, int level = 0,
		                                    const std::string& indent = "\t");
		std::ostream& printGlobalXmlParameterInfo(std::ostream& out = std::cout, int level = 0,
		                                   const std::string& indent = "\t");
		std::string   getXmlId             (const std::string& prefix = "") const;
		std::string   getXmlIdPrefix       (void) const;
		void     setText                   (const std::string& text);
		std::string   getText              (void) const;
		int      addLinkedParameterSet     (HTp token);
		int      getLinkedParameterSetCount(void);
		HumParamSet* getLinkedParameterSet (int index);
		HumParamSet* getParameterSet       (void);
		void         clearLinkInfo         (void);
		std::string getSlurLayoutParameter (const std::string& keyname, int subtokenindex = -1);
		std::string getPhraseLayoutParameter(const std::string& keyname, int subtokenindex = -1);
		std::string getLayoutParameter     (const std::string& category, const std::string& keyname,
		                                    int subtokenindex = -1);
		std::string getLayoutParameterChord(const std::string& category,
		                                    const std::string& keyname);
		std::string getLayoutParameterNote (const std::string& category,
		                                    const std::string& keyname, int subtokenindex);
		std::ostream& printXmlLinkedParameterInfo(std::ostream& out, int level, const std::string& indent);

		// layout parameter accessors
		std::string   getVisualDuration    (int subtokenindex = -1);
		std::string   getVisualDurationChord(void);
		std::string   getVisualDurationNote(int subtokenindex = -1);

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
		void        insertTokenAfter       (HTp newtoken);

		// next/previous token on line:
		HTp      getNextFieldToken     (void) const;
		HTp      getPreviousFieldToken (void) const;
		HTp      getNextField          (void) const { return getNextFieldToken(); }
		HTp      getPreviousField      (void) const { return getPreviousFieldToken(); }

		int      getPreviousNonNullDataTokenCount(void);
		int      getPreviousNNDTCount      (void)
		                           { return getPreviousNonNullDataTokenCount(); }
		HTp      getPreviousNonNullDataToken(int index = 0);
		HTp      getPreviousNNDT           (int index = 0)
		                           { return getPreviousNonNullDataToken(index); }
		int      getNextNonNullDataTokenCount(void);
		int      getNextNNDTCount           (void)
		                               { return getNextNonNullDataTokenCount(); }
		HTp      getNextNonNullDataToken   (int index = 0);
		HTp      getNextNNDT               (int index = 0)
		                               { return getNextNonNullDataToken(index); }

		// slur-analysis based functions:
		HumNum   getSlurDuration           (HumNum scale = 1);

		void     setTrack                  (int aTrack, int aSubtrack);
		void     setTrack                  (int aTrack);
		void     copyStructure             (HTp token);

		// strophe related functions:
		HTp      getStrophe                (void);
		std::string getStropheLabel        (void);
		void     setStrophe                (HTp strophe);
		bool     hasStrophe                (void);
		void     clearStrophe              (void);
		bool     isStrophe                 (const std::string& label);
		int      getStropheStartIndex      (void);
		bool     isFirstStrophe            (void);
		bool     isPrimaryStrophe          (void);

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
		void     setOwner                  (HLp aLine);
		int      getState                  (void) const;
		void     incrementState            (void);
		void     setDuration               (const HumNum& dur);
		void     setStrandIndex            (int index);

		bool     analyzeDuration           (void);
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

		// m_linkedParameterTokens: List of Humdrum tokens which are parameters
		// (mostly only layout parameters at the moment).
		// Was previously called m_linkedParameters;
		std::vector<HTp> m_linkedParameterTokens;

		// m_parameterSet: A single parameter encoded in the text of the
		// token.  Was previously called m_linkedParameter.
		HumParamSet* m_parameterSet = NULL;

		// m_rhythm_analyzed: Set to true when HumdrumFile assigned duration
		bool m_rhythm_analyzed = false;

		// m_strophe: Starting point of a strophe that the token belongs to.
		// NULL means that it is not in a strophe.
		HTp m_strophe = NULL;

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



