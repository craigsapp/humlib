//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Fri Aug 25 12:08:02 PDT 2017
// Last Modified: Fri Aug 25 12:08:05 PDT 2017
// Filename:      HumParamSet.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/HumParamSet.h
// Syntax:        C++11
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Set of parameters, specifically for Layout codes.
//                The HumParamSet class has a double namespace capability.
//                Parameters are encoded in local or global comments.
//                Examples:
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

#ifndef _HUMPARAMSET_H_INCLUDED
#define _HUMPARAMSET_H_INCLUDED

#include <iostream>
#include <string>
#include <vector>

class HumdrumToken;
typedef HumdrumToken* HTp;

#include "HumdrumToken.h"

namespace hum {

// START_MERGE

class HumParamSet {

	public:
		              HumParamSet        (void);
		              HumParamSet        (const std::string& token);
		              HumParamSet        (HTp token);
		             ~HumParamSet        ();

		const std::string& getNamespace1      (void);
		const std::string& getNamespace2      (void);
		std::string   getNamespace       (void);
		void          setNamespace1      (const std::string& name);
		void          setNamespace2      (const std::string& name);
		void          setNamespace       (const std::string& name);
		void          setNamespace       (const std::string& name1, const std::string& name2);

		void          clear              (void);
		int           getCount           (void);
		const std::string& getParameterName   (int index);
		const std::string& getParameterValue  (int index);
		int           addParameter       (const std::string& name, const std::string& value);
		int           setParameter       (const std::string& name, const std::string& value);
		void          readString         (const std::string& text);
		std::ostream& printXml           (std::ostream& out = std::cout, int level = 0,
		                                  const std::string& indent = "\t");

	private:
		std::string m_ns1;
		std::string m_ns2;
		std::vector<std::pair<std::string, std::string>> m_parameters;

};


std::ostream& operator<<(std::ostream& out, HumParamSet* hps);
std::ostream& operator<<(std::ostream& out, HumParamSet& hps);


// END_MERGE

} // end namespace hum

#endif /* _HUMPARAMSET_H_INCLUDED */



