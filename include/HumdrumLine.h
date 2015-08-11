//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Aug  8 12:24:49 PDT 2015
// Last Modified: Sun Aug  9 21:19:53 PDT 2015
// Filename:      HumAddress.h
// URL:           https://github.com/craigsapp/minHumdrum/blob/master/include/HumAddress.h
// Syntax:        C++11
// vim:           ts=3
//
// Description:   Used to store Humdrum text lines and analytic markup
//                of the line.
//

#ifndef _HUMDRUMLINE_H
#define _HUMDRUMLINE_H

#include <iostream>

#include "HumdrumToken.h"

using namespace std;

// START_MERGE

class HumdrumLine : public string {
	public:
		         HumdrumLine     (void);
		         HumdrumLine     (const string& aString);
		         HumdrumLine     (const char* aString);
		        ~HumdrumLine     ();

		bool     isComment       (void) const;
		bool     isCommentLocal  (void) const;
		bool     isCommentGlobal (void) const;
		bool     isExclusive     (void) const;
		bool     isExclusiveInterpretation (void) const { return isExclusive(); }
		bool     isTerminator    (void) const;
		bool     isInterp        (void) const;
		bool     isInterpretation(void) const { return isInterp(); }
		bool     isMeasure       (void) const;
		bool     isData          (void) const;
		bool     isEmpty         (void) const;
		bool     hasSpines       (void) const;
		bool     isManipulator   (void) const;
		void     getTokens       (vector<HumdrumToken*>& list);
		int      getTokenCount   (void) const;
      HumdrumToken& token      (int index);
      string   getTokenString  (int index) const;
		bool     equalChar       (int index, char ch) const;
		char     getChar         (int index) const;
		ostream& printSpineInfo  (ostream& out = cout);
		ostream& printDataType   (ostream& out = cout);
		ostream& printTrackInfo  (ostream& out = cout);
		ostream& printDataTypeInfo(ostream& out = cout);
		ostream& printDurationInfo(ostream& out = cout);
		int      createTokensFromLine (void);
		void     createLineFromTokens (void);
		int      getLineIndex    (void) const;
		int      getLineNumber   (void) const;

	protected:
		bool     analyzeTracks   (void);
		bool     analyzeTokenDurations (void);
		void     setLineIndex    (int index);
		void     clear           (void);

	private:
		int                  lineindex;
		vector<HumdrumToken*> tokens;
		HumNum               duration;
		HumNum               absolute;

	friend class HumdrumFile;
};

ostream& operator<< (ostream& out, HumdrumLine& line);


// END_MERGE

#endif /* _HUMDRUMLINE */



