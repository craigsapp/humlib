//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Aug  8 12:24:49 PDT 2015
// Last Modified: Sun Aug  9 21:03:12 PDT 2015
// Filename:      HumdrumFile.h
// URL:           https://github.com/craigsapp/minHumdrum/blob/master/include/HumdrumFile.h
// Syntax:        C++11
// vim:           ts=3
//
// Description:   Used to store Humdrum text lines from input stream
//                for further parsing.
//

#ifndef _HUMDRUMFILE_H
#define _HUMDRUMFILE_H

#include <iostream>
#include <vector>

#include "HumdrumLine.h"

using namespace std;

// START_MERGE

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

	private:
		vector<HumdrumLine*> lines;
		int          maxtrack;              // the number of tracks (primary spines) in the data.
};

ostream& operator<<(ostream& out, HumdrumFile& infile);


// END_MERGE

#endif /* _HUMDRUMFILE_H */



