//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sun Aug 16 01:23:01 PDT 2015
// Last Modified: Sun Aug 16 01:23:05 PDT 2015
// Filename:      HumHash.h
// URL:           https://github.com/craigsapp/minHumdrum/blob/master/include/HumHash.h
// Syntax:        C++11
// vim:           ts=3
//
// Description:   Key/value parameters systems for Humdrum tokens, lines,
//                and files.
//

#ifndef _HUMHASH_H
#define _HUMHASH_H

#include <string>
#include <vector>
#include <map>

using namespace std;

namespace minHumdrum {

class HumNum;

// START_MERGE

class HumHash {
	public:
		               HumHash             (void);
		              ~HumHash             ();

		string         getParameter        (const string& key);
		string         getParameter        (const string& ns2, const string& key);
		string         getParameter        (const string& ns1, const string& ns2,
		                                    const string& key);
		int            getParameterInt     (const string& key);
		int            getParameterInt     (const string& ns2, const string& key);
		int            getParameterInt     (const string& ns1, const string& ns2,
		                                    const string& key);
		HumNum         getParameterFraction(const string& key);
		HumNum         getParameterFraction(const string& ns2, const string& key);
		HumNum         getParameterFraction(const string& ns1, const string& ns2,
		                                    const string& key);
		double         getParameterFloat   (const string& key);
		double         getParameterFloat   (const string& ns2, const string& key);
		double         getParameterFloat   (const string& ns1, const string& ns2,
		                                    const string& key);
		bool           getParameterBool    (const string& key);
		bool           getParameterBool    (const string& ns2, const string& key);
		bool           getParameterBool    (const string& ns1, const string& ns2,
		                                    const string& key);
		void           setParameter        (const string& key,
		                                    const string& value);
		void           setParameter        (const string& ns2,
		                                    const string& key,
		                                    const string& value);
		void           setParameter        (const string& ns1,
		                                    const string& ns2,
		                                    const string& key,
		                                    const string& value);
/*
		void           setParameter        (const string& key, int value);
		void           setParameter        (const string& ns2, const string& key,
		                                    int value);
		void           setParameter        (const string& ns1, const string& ns2,
		                                    const string& key, int value);
		void           setParameter        (const string& key, float value);
		void           setParameter        (const string& ns2, const string& key,
		                                    float value);
		void           setParameter        (const string& ns1, const string& ns2,
		                                    const string& key, float value);
		void           setParameter        (const string& key, bool value);
		void           setParameter        (const string& ns2, const string& key,
		                                    bool value);
		void           setParameter        (const string& ns1, const string& ns2,
		                                    const string& key, bool value);
*/

		bool           hasParameter        (const string& key);
		bool           hasParameter        (const string& ns2, const string& key);
		bool           hasParameter        (const string& ns1, const string& ns2,
		                                    const string& key);
		void           deleteParameter     (const string& key);
		void           deleteParameter     (const string& ns2, const string& key);
		void           deleteParameter     (const string& ns1, const string& ns2,
		                                    const string& key);

	protected:
		void           initializeParameters(void);
		vector<string> getKeyList          (const string& keys);

	private:
		map<string, map<string, map<string, string> > >* parameters;
};


// END_MERGE

} // end namespace std;

#endif /* _HUMHASH_H */



