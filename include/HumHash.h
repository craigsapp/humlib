//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sun Aug 16 01:23:01 PDT 2015
// Last Modified: Sun Aug 16 01:23:05 PDT 2015
// Filename:      HumHash.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/HumHash.h
// Syntax:        C++11; humlib
// vim:           syntax=cpp ts=3 noexpandtab nowrap
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
//                   .              2a
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
//                  a digit (but the humlib parser will not enforce
//                  this preference).  Values may contain spaces (but not
//                  tabs or colons.  If the value must include a colon it
//                  should be given as "&colon;" (without the quotes).
//
//                 Global comments affect all tokens on the next non-null
//                 line, and are similar to the above examples, but start
//                 with two exclamation marks:
//                   **kern         **kern
//                   1e             2g
//                   .              2a
//                   !!LO:N:vis=4
//                   1c             1g
//                   *-             *-
//                 This will apply the parameter to both "1c" and "1g" on the
//                 following line.  In the following case:
//                   **kern         **kern
//                   1e             2g
//                   !!LO:N:vis=4
//                   .              2a
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

#ifndef _HUMHASH_H_INCLUDED
#define _HUMHASH_H_INCLUDED

#include <iostream>
#include <string>
#include <vector>
#include <map>

namespace hum {

class HumNum;
class HumdrumToken;
typedef HumdrumToken* HTp;

// START_MERGE

class HumParameter : public std::string {
	public:
		HumParameter(void);
		HumParameter(const std::string& str);
		HumdrumToken* origin;
};

typedef std::map<std::string, std::map<std::string, std::map<std::string, HumParameter> > > MapNNKV;
typedef std::map<std::string, std::map<std::string, HumParameter> > MapNKV;
typedef std::map<std::string, HumParameter> MapKV;

class HumHash {
	public:
		               HumHash             (void);
		              ~HumHash             ();

		std::string    getValue            (const std::string& key) const;
		std::string    getValue            (const std::string& ns2,
		                                    const std::string& key) const;
		std::string    getValue            (const std::string& ns1, const std::string& ns2,
		                                    const std::string& key) const;
		HTp            getValueHTp         (const std::string& key) const;
		HTp            getValueHTp         (const std::string& ns2,
		                                    const std::string& key) const;
		HTp            getValueHTp         (const std::string& ns1, const std::string& ns2,
		                                    const std::string& key) const;
		int            getValueInt         (const std::string& key) const;
		int            getValueInt         (const std::string& ns2,
		                                    const std::string& key) const;
		int            getValueInt         (const std::string& ns1, const std::string& ns2,
		                                    const std::string& key) const;
		HumNum         getValueFraction    (const std::string& key) const;
		HumNum         getValueFraction    (const std::string& ns2,
		                                    const std::string& key) const;
		HumNum         getValueFraction    (const std::string& ns1, const std::string& ns2,
		                                    const std::string& key)const ;
		double         getValueFloat       (const std::string& key)const ;
		double         getValueFloat       (const std::string& ns2,
		                                    const std::string& key) const;
		double         getValueFloat       (const std::string& ns1, const std::string& ns2,
		                                    const std::string& key) const;
		bool           getValueBool        (const std::string& key) const;
		bool           getValueBool        (const std::string& ns2,
		                                    const std::string& key) const;
		bool           getValueBool        (const std::string& ns1, const std::string& ns2,
		                                    const std::string& key) const;

		void           setValue            (const std::string& key,
		                                    const std::string& value);
		void           setValue            (const std::string& ns2,
		                                    const std::string& key,
		                                    const std::string& value);
		void           setValue            (const std::string& ns1,
		                                    const std::string& ns2,
		                                    const std::string& key,
		                                    const std::string& value);
		void           setValue            (const std::string& key,
		                                    const char* value);
		void           setValue            (const std::string& ns2,
		                                    const std::string& key,
		                                    const char* value);
		void           setValue            (const std::string& ns1,
		                                    const std::string& ns2,
		                                    const std::string& key,
		                                    const char* value);
		void           setValue            (const std::string& key, int value);
		void           setValue            (const std::string& ns2, const std::string& key,
		                                    int value);
		void           setValue            (const std::string& ns1, const std::string& ns2,
		                                    const std::string& key, int value);
		void           setValue            (const std::string& key, HTp value);
		void           setValue            (const std::string& ns2, const std::string& key,
		                                    HTp value);
		void           setValue            (const std::string& ns1, const std::string& ns2,
		                                    const std::string& key, HTp value);
		void           setValue            (const std::string& key, HumNum value);
		void           setValue            (const std::string& ns2, const std::string& key,
		                                    HumNum value);
		void           setValue            (const std::string& ns1, const std::string& ns2,
		                                    const std::string& key, HumNum value);
		void           setValue            (const std::string& key, double value);
		void           setValue            (const std::string& ns2, const std::string& key,
		                                    double value);
		void           setValue            (const std::string& ns1, const std::string& ns2,
		                                    const std::string& key, double value);
		bool           isDefined           (const std::string& key) const;
		bool           isDefined           (const std::string& ns2,
		                                    const std::string& key) const;
		bool           isDefined           (const std::string& ns1, const std::string& ns2,
		                                    const std::string& key) const;
		void           deleteValue         (const std::string& key);
		void           deleteValue         (const std::string& ns2, const std::string& key);
		void           deleteValue         (const std::string& ns1, const std::string& ns2,
		                                    const std::string& key);

		std::vector<std::string> getKeys   (void) const;
		std::vector<std::string> getKeys   (const std::string& ns) const;
		std::vector<std::string> getKeys   (const std::string& ns1,
		                                    const std::string& ns2) const;

		std::map<std::string, std::string> getParameters(std::string& ns1);
		std::map<std::string, std::string> getParameters(const std::string& ns1,
		                                    const std::string& ns2);

		bool           hasParameters       (void) const;
		bool           hasParameters       (const std::string& ns) const;
		bool           hasParameters       (const std::string& ns1,
		                                    const std::string& ns2) const;
		int            getParameterCount   (void) const;
		int            getParameterCount   (const std::string& ns) const;
		int            getParameterCount   (const std::string& ns1,
		                                    const std::string& ns2) const;
		void           setPrefix           (const std::string& value);
		std::string    getPrefix           (void) const;
		std::ostream&  printXml            (std::ostream& out = std::cout, int level = 0,
		                                    const std::string& indent = "\t");
		std::ostream&  printXmlAsGlobal    (std::ostream& out = std::cout, int level = 0,
		                                    const std::string& indent = "\t");

		void           setOrigin           (const std::string& key,
		                                    HumdrumToken* tok);
		void           setOrigin           (const std::string& key,
		                                    HumdrumToken& tok);
		void           setOrigin           (const std::string& ns2, const std::string& key,
		                                    HumdrumToken* tok);
		void           setOrigin           (const std::string& ns2, const std::string& key,
		                                    HumdrumToken& tok);
		void           setOrigin           (const std::string& ns1, const std::string& ns2,
		                                    const std::string& parameter,
		                                    HumdrumToken* tok);
		void           setOrigin           (const std::string& ns1, const std::string& ns2,
		                                    const std::string& parameter,
		                                    HumdrumToken& tok);

		HumdrumToken*  getOrigin           (const std::string& key) const;
		HumdrumToken*  getOrigin           (const std::string& ns2,
		                                    const std::string& key) const;
		HumdrumToken*  getOrigin           (const std::string& ns1,
		                                    const std::string& ns2,
		                                    const std::string& parameter) const;

	protected:
		void                     initializeParameters  (void);
		std::vector<std::string> getKeyList            (const std::string& keys) const;

	private:
		MapNNKV*    parameters;
		std::string prefix;

	friend std::ostream& operator<<(std::ostream& out, const HumHash& hash);
	friend std::ostream& operator<<(std::ostream& out, HumHash* hash);
};


// END_MERGE

} // end namespace hum

#endif /* _HUMHASH_H_INCLUDED */



