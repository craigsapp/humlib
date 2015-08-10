//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Aug  8 12:24:49 PDT 2015
// Last Modified: Sun Aug  9 21:19:53 PDT 2015
// Filename:      HumAddress.h
// URL:           https://github.com/craigsapp/minHumdrum/blob/master/include/HumAddress.h
// Syntax:        C++11
// vim:           ts=3
//
// Description:   Used to store the location of a token within a HumdrumFile.
//

#ifndef _HUMADDRESS_H
#define _HUMADDRESS_H

#include <iostream>

using namespace std;

// START_MERGE

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

// END_MERGE

#endif /* _HUMADDRESS */



