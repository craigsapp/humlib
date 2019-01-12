//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sun Jun 10 14:29:15 PDT 2018
// Last Modified: Sun Jun 10 14:29:19 PDT 2018
// Filename:      HumSignifiers.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/HumSignifiers.h
// Syntax:        C++11
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   The collection of signifiers in a Humdrum file.
//

#ifndef _HUMSIGNIFIERS_H_INCLUDED
#define _HUMSIGNIFIERS_H_INCLUDED

#include "HumSignifier.h"

#include <string>
#include <vector>

namespace hum {

// START_MERGE

class HumSignifiers {
	public:
		              HumSignifiers    (void);
		             ~HumSignifiers    ();

		void          clear            (void);
		bool          addSignifier     (const std::string& rdfline);
		bool          hasKernLinkSignifier (void);
		std::string   getKernLinkSignifier (void);
		bool          hasKernAboveSignifier (void);
		std::string   getKernAboveSignifier (void);
		bool          hasKernBelowSignifier (void);
		std::string   getKernBelowSignifier (void);
		int           getSignifierCount(void);
		HumSignifier* getSignifier(int index);

	private:
		std::vector<HumSignifier*> m_signifiers;
		int  m_kernLinkIndex = -1;
		int  m_kernAboveIndex = -1;
		int  m_kernBelowIndex = -1;

};


// END_MERGE

} // end namespace hum

#endif /* _HUMSIGNIFIERS_H_INCLUDED */



