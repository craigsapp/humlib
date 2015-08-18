//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sun Aug 16 01:23:01 PDT 2015
// Last Modified: Sun Aug 16 01:23:05 PDT 2015
// Filename:      HumHash.h
// URL:           https://github.com/craigsapp/minHumdrum/blob/master/include/HumHash.h
// Syntax:        C++11
// vim:           ts=3 noexpandtab
//
// Description:   Key/value parameters systems for Humdrum tokens, lines,
//                and files.  The HumHash class has a double namespace
//                capability. Parameters are encoded in local or global
//                comments.  Examples:
//                    !LO:N:vis=4
//                Namespace 1: LO (layout codes)
//                Namespace 2: N  (Note layout codes)
//                Key/Value  : vis=4, the "vis" key has the value "4"
//                Local parameters apply to the next non-null token in the
//                spine which follow them (data, measure and interpretation
//                tokens, but not local comment tokens).  For example to apply
//                the above example parameter to a token:
//                   **kern
//                   !LO:N:vis=1
//                   1c
//                   *-
//                 In this case the duration of the note is a whole note, but
//                 is should be displayed in graphical notation as a quarter
//                 note. If there are null data or interpretation tokens
//                 between the parameter and the note, the parameter is passed
//                 on to the next non-null token, such as:
//                   **kern         **kern
//                   1e             2g
//                   !LO:N:vis=1    !
//							.              2a
//                   *              *clefG2
//                   1c             1g
//                   *-             *-
//                 In the above case the parameter is still applied to "1c".
//                 Namespace(s)+Keys must be unique, since including two
//                 parameters with the same namespace(s)/key will only
//                 accept one setting.  Only the value of the first
//                 duplicate parameter will be stored, and all duplicates
//                 after the first occurrence will be ignored.  For example:
//                   **kern
//                   !LO:N:vis=2
//                   !LO:N:vis=4
//                   1c
//                   *-
//                  will have the value LO:N:vis set to "2" for the "1c" token.
//                  Namespaces are optional and are indicated by an empty
//                  string.  For example, a parameter not stored in any
//                  namespace will have this form:
//                     !::vis=4
//                  To give only one namespace, the preferable form is:
//                     !:N:vis=4
//                  although this form can also be given:
//                     !N::vis=4
//                  where the second namespace is the empty string "".
//
//                  Multiple key values can be specified, each separated by
//                  a colon:
//                    !LO:N:vis=2:stem=5
//                  this can be expanded into two local comments:
//                    !LO:N:vis=2
//                    !LO:N:stem=5
//
//                  The namespaces and keys may not contain tabs (obviously),
//                  spaces or colons.  Preferrably they will only contain
//                  letters, digits, and the underscore, but not start with
//                  a digit (but the minHumdrum parser will not enforce
//                  this preference).  Values may contain spaces (but not
//                  tabs or colons.  If the value must include a colon it
//                  should be given as "&colon;" (without the quotes).
//
//                 Global comments affect all tokens on the next non-null
//                 line, and are similar to the above examples, but start
//                 with two exclamation marks:
//                   **kern         **kern
//                   1e             2g
//							.              2a
//                   !!LO:N:vis=4
//                   1c             1g
//                   *-             *-
//                 This will apply the parameter to both "1c" and "1g" on the
//                 following line.  In the following case:
//                   **kern         **kern
//                   1e             2g
//                   !!LO:N:vis=4
//							.              2a
//                   1c             1g
//                   *-             *-
//                  The parameter will apply to "1c", and "2a" rather than
//                  "1g". (Currently the parameter will only be applied to
//                  "2a", but this will be canged in the future).  Typically
//                  global parameters are used to apply parameters to all
//                  measures in all spines, or they may be used to display
//                  a single text string above or below the system in the
//                  full score (or part if it is extracted from the full
//                  score).
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

		string         getValue            (const string& key) const;
		string         getValue            (const string& ns2,
		                                    const string& key) const;
		string         getValue            (const string& ns1, const string& ns2,
		                                    const string& key) const;
		int            getValueInt         (const string& key) const;
		int            getValueInt         (const string& ns2,
		                                    const string& key) const;
		int            getValueInt         (const string& ns1, const string& ns2,
		                                    const string& key) const;
		HumNum         getValueFraction    (const string& key) const;
		HumNum         getValueFraction    (const string& ns2,
		                                    const string& key) const;
		HumNum         getValueFraction    (const string& ns1, const string& ns2,
		                                    const string& key)const ;
		double         getValueFloat       (const string& key)const ;
		double         getValueFloat       (const string& ns2,
		                                    const string& key) const;
		double         getValueFloat       (const string& ns1, const string& ns2,
		                                    const string& key) const;
		bool           getValueBool        (const string& key) const;
		bool           getValueBool        (const string& ns2,
		                                    const string& key) const;
		bool           getValueBool        (const string& ns1, const string& ns2,
		                                    const string& key) const;
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
		bool           isDefined           (const string& key) const;
		bool           isDefined           (const string& ns2,
		                                    const string& key) const;
		bool           isDefined           (const string& ns1, const string& ns2,
		                                    const string& key) const;
		void           deleteValue         (const string& key);
		void           deleteValue         (const string& ns2, const string& key);
		void           deleteValue         (const string& ns1, const string& ns2,
		                                    const string& key);
		vector<string> getKeys             (void) const;
		vector<string> getKeys             (const string& ns) const;
		vector<string> getKeys             (const string& ns1,
		                                    const string& ns2) const;
		bool           hasParameters       (void) const;
		bool           hasParameters       (const string& ns) const;
		bool           hasParameters       (const string& ns1,
		                                    const string& ns2) const;
		int            getParameterCount   (void) const;
		int            getParameterCount   (const string& ns) const;
		int            getParameterCount   (const string& ns1,
		                                    const string& ns2) const;
		void           setPrefix           (const string& value);

	protected:
		void           initializeParameters(void);
		vector<string> getKeyList          (const string& keys) const;

	private:
		MapNNKV* parameters;
		string   prefix;

	friend ostream& operator<<(ostream& out, const HumHash& hash);
};



// END_MERGE

} // end namespace std;

#endif /* _HUMHASH_H */



