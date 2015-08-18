//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Aug  8 12:24:49 PDT 2015
// Last Modified: Sun Aug  9 21:19:53 PDT 2015
// Filename:      HumAddress.h
// URL:           https://github.com/craigsapp/minHumdrum/blob/master/include/HumAddress.h
// Syntax:        C++11
// vim:           ts=3 noexpandtab
//
// Description:   Used to store the location of a token within a HumdrumFile.
//

#ifndef _HUMADDRESS_H
#define _HUMADDRESS_H

#include <iostream>

using namespace std;

namespace minHumdrum {

class HumdrumLine;
class HumdrumToken;

// START_MERGE

class HumAddress {
	public:
		              HumAddress        (void);
		             ~HumAddress        ();

		int           getLineIndex      (void) const;
		int           getLineNumber     (void) const;
		int           getFieldIndex     (void) const;
		const HumdrumToken& getDataType (void) const;
		const string& getSpineInfo      (void) const;
		int           getTrack          (void) const;
		int           getSubtrack       (void) const;
		string        getTrackString    (string separator = ".") const;
		HumdrumLine*  getLine           (void) const;
		HumdrumLine*  getOwner          (void) const { return getLine(); }
		bool          hasOwner          (void) const;

	protected:
		void          setOwner          (HumdrumLine* aLine);
		void          setFieldIndex     (int fieldlindex);
		void          setSpineInfo      (const string& spineinfo);
		void          setTrack          (int aTrack, int aSubtrack);
		void          setTrack          (int aTrack);
		void          setSubtrack       (int aSubtrack);

	private:

		// fieldindex: This is the index of the token in the HumdrumLine
		// which owns this token.
		int fieldindex;       // field index of token on line

		// spining: This is the spine position of the token. A simple spine
		// position is an integer, starting with "1" for the first spine
		// of the file (left-most spine).  When the spine splits, "(#)a"
		// is wrapped around the left-subspine's spine info, and "(#)b"
		// around the right-subspine's info. Merged spines will add a space
		// between the two or more merged spines information, such as
		// "(#)a (#)b" for two sub-spines merged into a single spine again.
		// But in this case there is a spine info simplification which will
		// convert "(#)a (#)b" into "#" where # is the original spine number.
		// Other more complicated mergers may be simplified in the future.
		string spining;

		// track: This is the track number of the spine.  It is the first
		// number found in the spineinfo string.
		int track;

		// subtrack: This is the subtrack number for the spine.  When a spine
		// is not split, it will be 0, if the spine has been split with *^,
		// then the left-subspine will be in subtrack 1 and the right-spine
		// will be subtrack 2.  If subspines are exchanged with *x, then their
		// subtrack assignments will also change.
		int subtrack;

		// owner: This is the line which manages the given token.
		HumdrumLine* owner;

	friend class HumdrumToken;
	friend class HumdrumLine;
	friend class HumdrumFile;
};

// END_MERGE

} // end namespace std;

#endif /* _HUMADDRESS */



