//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sun Jun 10 06:47:03 PDT 2018
// Last Modified: Sun Jun 10 06:47:06 PDT 2018
// Filename:      HumSignifier.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/HumSignifier.h
// Syntax:        C++11
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   A signifier that extends a Humdrum representation.
//

#ifndef _HUMSIGNIFIER_H_INCLUDED
#define _HUMSIGNIFIER_H_INCLUDED

#include <string>
#include <map>

namespace hum {

// START_MERGE

enum signifier_type {
	signifier_unknown,
	signifier_link,
	signifier_above,
	signifier_below
};

class HumSignifier {

	public:
		            HumSignifier     (void);
		            HumSignifier     (const std::string& rdfline);
		           ~HumSignifier     ();
		bool        parseSignifier   (const std::string& rdfline);
		void        clear            (void);
		std::string getSignifier     (void);
		std::string getDefinition    (void);
		std::string getParameter     (const std::string& key);
		bool        isKernLink       (void);
		bool        isKernAbove      (void);
		bool        isKernBelow      (void);

	private:
		std::string m_exinterp;
		std::string m_signifier;
		std::string m_definition;
		int         m_sigtype = signifier_type::signifier_unknown;
		std::map<std::string, std::string> m_parameters;
};


// END_MERGE

} // end namespace hum

#endif /* _HUMSIGNIFIER_H_INCLUDED */



