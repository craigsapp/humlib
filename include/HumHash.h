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

typedef map<string, map<string, map<string, string> > > MapNNKV;
typedef map<string, map<string, string> > MapNKV;
typedef map<string, string> MapKV;

class HumHash {
	public:
		               HumHash             (void);
		              ~HumHash             ();

		string         getValue            (const string& key);
		string         getValue            (const string& ns2, const string& key);
		string         getValue            (const string& ns1, const string& ns2,
		                                    const string& key);
		int            getValueInt         (const string& key);
		int            getValueInt         (const string& ns2, const string& key);
		int            getValueInt         (const string& ns1, const string& ns2,
		                                    const string& key);
		HumNum         getValueFraction    (const string& key);
		HumNum         getValueFraction    (const string& ns2, const string& key);
		HumNum         getValueFraction    (const string& ns1, const string& ns2,
		                                    const string& key);
		double         getValueFloat       (const string& key);
		double         getValueFloat       (const string& ns2, const string& key);
		double         getValueFloat       (const string& ns1, const string& ns2,
		                                    const string& key);
		bool           getValueBool        (const string& key);
		bool           getValueBool        (const string& ns2, const string& key);
		bool           getValueBool        (const string& ns1, const string& ns2,
		                                    const string& key);
		void           setValue            (const string& key,
		                                    const string& value);
		void           setValue            (const string& ns2,
		                                    const string& key,
		                                    const string& value);
		void           setValue            (const string& ns1,
		                                    const string& ns2,
		                                    const string& key,
		                                    const string& value);
		void           setValue            (const string& key, int value);
		void           setValue            (const string& ns2, const string& key,
		                                    int value);
		void           setValue            (const string& ns1, const string& ns2,
		                                    const string& key, int value);
		void           setValue            (const string& key, HumNum value);
		void           setValue            (const string& ns2, const string& key,
		                                    HumNum value);
		void           setValue            (const string& ns1, const string& ns2,
		                                    const string& key, HumNum value);
		void           setValue            (const string& key, double value);
		void           setValue            (const string& ns2, const string& key,
		                                    double value);
		void           setValue            (const string& ns1, const string& ns2,
		                                    const string& key, double value);
		bool           isDefined           (const string& key);
		bool           isDefined           (const string& ns2, const string& key);
		bool           isDefined           (const string& ns1, const string& ns2,
		                                    const string& key);
		void           deleteValue         (const string& key);
		void           deleteValue         (const string& ns2, const string& key);
		void           deleteValue         (const string& ns1, const string& ns2,
		                                    const string& key);
		vector<string> getKeys             (const string& ns1, const string& ns2);

	protected:
		void           initializeParameters(void);
		vector<string> getKeyList          (const string& keys);

	private:
		MapNNKV* parameters;
};


// END_MERGE

} // end namespace std;

#endif /* _HUMHASH_H */



