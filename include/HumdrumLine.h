//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Aug  8 12:24:49 PDT 2015
// Last Modified: Wed Aug 12 10:42:07 PDT 2015
// Filename:      HumAddress.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/HumAddress.h
// Syntax:        C++11; humlib
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Used to store Humdrum text lines and analytic markup
//                of the line.
//

#ifndef _HUMDRUMLINE_H_INCLUDED
#define _HUMDRUMLINE_H_INCLUDED

#include "HumdrumToken.h"
#include "HumHash.h"

#include <iostream>
#include <string>
#include <vector>

namespace hum {

class HumdrumFile;

// START_MERGE

typedef HumdrumLine* HLp;

class HumdrumLine : public std::string, public HumHash {
	public:
		            HumdrumLine            (void);
		            HumdrumLine            (const std::string& aString);
		            HumdrumLine            (const char* aString);
		            HumdrumLine            (HumdrumLine& line);
		            HumdrumLine            (HumdrumLine& line, void* owner);
		           ~HumdrumLine            ();

		HumdrumLine& operator=             (HumdrumLine& line);
		bool        isComment              (void) const;
		bool        isCommentLocal         (void) const;
		bool        isLocalComment         (void) const { return isCommentLocal(); }
		bool        isCommentGlobal        (void) const;
		bool        isCommentUniversal     (void) const;
		bool        isReference            (void) const;
		bool        isGlobalReference      (void) const;
		bool        isUniversalReference   (void) const;
		bool        isSignifier            (void) const;
		std::string getReferenceKey        (void) const;
		std::string getReferenceValue      (void) const;
		std::string getGlobalReferenceKey      (void) const;
		std::string getGlobalReferenceValue    (void) const;
		std::string getUniversalReferenceKey   (void) const;
		std::string getUniversalReferenceValue (void) const;
		bool        isUniversalComment     (void) const { return isCommentUniversal(); }
		bool        isGlobalComment        (void) const { return isCommentGlobal(); }
		bool        isExclusive            (void) const;
		bool        isExclusiveInterpretation (void) const { return isExclusive(); }
		bool        isTerminator           (void) const;
		bool        isInterp               (void) const;
		bool        isInterpretation       (void) const { return isInterp(); }
		bool        isBarline              (void) const;
		bool        isData                 (void) const;
		bool        isAllNull              (void) const;
		bool        isAllRhythmicNull      (void) const;
		bool        isEmpty                (void) const;
		bool        isBlank                (void) const { return isEmpty(); }
		bool        isManipulator          (void) const;
		bool        hasSpines              (void) const;
		bool        isGlobal               (void) const;
		bool        equalFieldsQ           (const std::string& exinterp, const std::string& value);
		HTp         token                  (int index) const;
		void        getTokens              (std::vector<HTp>& list);
		int         getTokenCount          (void) const;
		int         getFieldCount          (void) const { return getTokenCount(); }
		std::string getTokenString         (int index) const;
		bool        equalChar              (int index, char ch) const;
		char        getChar                (int index) const;
		bool        isKernBoundaryStart    (void) const;
		bool        isKernBoundaryEnd      (void) const;
		std::ostream& printSpineInfo       (std::ostream& out = std::cout);
		std::ostream& printTrackInfo       (std::ostream& out = std::cout);
		std::ostream& printDataTypeInfo    (std::ostream& out = std::cout);
		std::ostream& printDurationInfo    (std::ostream& out = std::cout);
		std::ostream& printCsv             (std::ostream& out = std::cout,
		                                    const std::string& separator = ",");
		std::ostream& printXml             (std::ostream& out = std::cout, int level = 0,
		                                    const std::string& indent = "\t");
		std::ostream& printXmlParameterInfo(std::ostream& out, int level,
		                                    const std::string& indent);
		std::ostream& printGlobalXmlParameterInfo(std::ostream& out, int level,
		                                    const std::string& indent);
		std::string   getXmlId             (const std::string& prefix = "") const;
		std::string   getXmlIdPrefix       (void) const;
		void          createLineFromTokens (void);
		void          removeExtraTabs      (void);
		void          addExtraTabs         (std::vector<int>& trackWidths);
		int           getLineIndex         (void) const;
		int           getLineNumber        (void) const;
		HumdrumFile*  getOwner             (void);
		void          setText              (const std::string& text);
		std::string   getText              (void);
		int           getBarNumber         (void);
		int           getMeasureNumber     (void) { return getBarNumber(); }

		HumNum      getDuration            (void);
		HumNum      getDurationFromStart   (void);
		HumNum      getDurationToEnd       (void);
		HumNum      getDurationFromBarline (void);
		HumNum      getDurationToBarline   (void);
		HumNum      getBarlineDuration     (void);

		HumNum      getDuration            (HumNum scale);
		HumNum      getDurationFromStart   (HumNum scale);
		HumNum      getDurationToEnd       (HumNum scale);
		HumNum      getDurationFromBarline (HumNum scale);
		HumNum      getDurationToBarline   (HumNum scale);
		HumNum      getBarlineDuration     (HumNum scale);
		int         getKernNoteAttacks     (void);
		int         addLinkedParameter     (HTp token);

		HumNum   getBeat                (HumNum beatdur = 1);
		HumNum   getBeatStr             (std::string beatrecip = "4");
		HTp      getTrackStart          (int track) const;
		void     setLineFromCsv         (const char* csv,
		                                 const std::string& separator = ",");
		void     setLineFromCsv         (const std::string& csv,
		                                 const std::string& separator = ",");

		// low-level editing functions (need to re-analyze structure after using)
		void     appendToken            (HTp token, int tabcount = 1);
		void     appendToken            (const HumdrumToken& token, int tabcount = 1);
		void     appendToken            (const std::string& token, int tabcount = 1);
		void     appendToken            (const char* token, int tabcount = 1);

		void     appendToken            (int index, HTp token, int tabcount = 1);
		void     appendToken            (int index, const HumdrumToken& token, int tabcount = 1);
		void     appendToken            (int index, const std::string& token, int tabcount = 1);
		void     appendToken            (int index, const char* token, int tabcount = 1);

		void     insertToken            (int index, HTp token, int tabcount = 1);
		void     insertToken            (int index, const HumdrumToken& token, int tabcount = 1);
		void     insertToken            (int index, const std::string& token, int tabcount = 1);
		void     insertToken            (int index, const char* token, int tabcount = 1);

		void     setDuration            (HumNum aDur);
		void     setDurationFromStart   (HumNum dur);
		void     setDurationFromBarline (HumNum dur);
		void     setDurationToBarline   (HumNum dur);

	protected:
		bool     analyzeTracks          (std::string& err);
		bool     analyzeTokenDurations  (std::string& err);
		void     setLineIndex           (int index);
		void     clear                  (void);
		void     setOwner               (void* hfile);
		int      createTokensFromLine   (void);
		void     setLayoutParameters    (void);
		void     setParameters          (const std::string& pdata);
		void     storeGlobalLinkedParameters(void);
		std::ostream&	printXmlGlobalLinkedParameterInfo(std::ostream& out = std::cout, int level = 0,
		                                 const std::string& indent = "\t");
		std::ostream& printXmlGlobalLinkedParameters(std::ostream& out = std::cout, int level = 0,
		                                 const std::string& indent = "\t");

	private:

		//
		// State variables managed by the HumdrumLine class:
		//

		// m_lineindex: Used to store the index number of the HumdrumLine in
		// the owning HumdrumFile object.
		// This variable is filled by HumdrumFileStructure::analyzeLines().
		int m_lineindex;

		// m_tokens: Used to store the individual tab-separated token fields
		// on a line.  These are prepared automatically after reading in
		// a full line of text (which is accessed throught the string parent
		// class).  If the full line is changed, the tokens are not updated
		// automatically -- use createTokensFromLine().  Likewise the full
		// text line is not updated if any tokens are changed -- use
		// createLineFromTokens() in that case.  The second case is more
		// useful: you can read in a HumdrumFile, tweak the tokens, then
		// reconstruct the full line and print out again.
		// This variable is filled by HumdrumFile::read().
		// The contents of this vector should be deleted when deconstructing
		// a HumdrumLine object.
		std::vector<HumdrumToken*> m_tokens;

		// m_tabs: Used to store a count of the number of tabs between
		// each token on a line.  This is the number of tabs after the
		// token at the given index (so no tabs before the first token).
		std::vector<int> m_tabs;

		// m_duration: This is the "duration" of a line.  The duration is
		// equal to the minimum time unit of all durational tokens on the
		// line.  This also includes null tokens when the duration of a
		// previous note in a previous spine is ending on the line, so it is
		// not just the minimum duration on the line.
		// This variable is filled by HumdrumFileStructure::analyzeRhythm().
		HumNum m_duration;

		// m_durationFromStart: This is the cumulative duration of all lines
		// prior to this one in the owning HumdrumFile object.  For example,
		// the first notes in a score start at time 0, If the duration of the
		// first data line is 1 quarter note, then the durationFromStart for
		// the second line will be 1 quarter note.
		// This variable is filled by HumdrumFileStructure::analyzeRhythm().
		HumNum m_durationFromStart;

		// m_durationFromBarline: This is the cumulative duration from the
		// last barline to the current data line.
		// This variable is filled by HumdrumFileStructure::analyzeMeter().
		HumNum m_durationFromBarline;

		// m_durationToBarline: This is the duration from the start of the
		// current line to the next barline in the owning HumdrumFile object.
		// This variable is filled by HumdrumFileStructure::analyzeMeter().
		HumNum m_durationToBarline;

		// m_linkedParameters: List of Humdrum tokens which are parameters
		// (mostly only layout parameters at the moment)
		std::vector<HTp> m_linkedParameters;

		// m_rhythm_analyzed: True if duration information from HumdrumFile
		// has been added to line.
		bool m_rhythm_analyzed = false;

		// owner: This is the HumdrumFile which manages the given line.
		void* m_owner;

	friend class HumdrumFileBase;
	friend class HumdrumFileStructure;
	friend class HumdrumFileContent;
	friend class HumdrumFile;
};

std::ostream& operator<< (std::ostream& out, HumdrumLine& line);
std::ostream& operator<< (std::ostream& out, HumdrumLine* line);


// END_MERGE

} // end namespace hum

#endif /* _HUMDRUMLINE_H_INCLUDED */



