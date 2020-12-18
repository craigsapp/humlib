//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Wed Dec 16 11:16:33 PST 2020
// Last Modified: Wed Dec 16 11:16:37 PST 2020
// Filename:      tool-modori.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-modori.h
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Switch between original and modern clefs/keysig/mensuration/time sig.
//

#ifndef _TOOL_MODORI_H
#define _TOOL_MODORI_H

#include "HumTool.h"
#include "HumdrumFile.h"

namespace hum {

// START_MERGE

class Tool_modori : public HumTool {
	public:
		         Tool_modori         (void);
		        ~Tool_modori         () {};

		bool     run                 (HumdrumFileSet& infiles);
		bool     run                 (HumdrumFile& infile);
		bool     run                 (const string& indata, ostream& out);
		bool     run                 (HumdrumFile& infile, ostream& out);

	protected:
		void     processFile         (HumdrumFile& infile);
		void     initialize          (void);
		void     printInfo           (void);
		void     switchModernOriginal(HumdrumFile& infile);
		bool     swapKeyStyle        (HTp one, HTp two);
		bool     swapClefStyle       (HTp one, HTp two);
		bool     flipMensurationStyle(HTp token);
		void     convertKeySignatureToModern  (HTp token);
		void     convertKeySignatureToOriginal(HTp token);
		void     convertKeySignatureToRegular (HTp token);
		void     convertClefToModern          (HTp token);
		void     convertClefToOriginal        (HTp token);
		void     convertClefToRegular         (HTp token);

	private:
		bool m_modernQ        = false; // show modern key/clef/time signatures
		bool m_originalQ      = false; // show original key/clef/mensuration
		bool m_infoQ          = false; // show key/clef/mensuration tokens in data
		bool m_nokeyQ         = false; // -K option: don't change key signatures
		bool m_noclefQ        = false; // -C option: don't change clefs
		bool m_nomensurationQ = false; // -M option: don't change mensurations

		std::vector<std::map<HumNum, std::vector<HTp>>> m_keys;
		std::vector<std::map<HumNum, std::vector<HTp>>> m_clefs;
		std::vector<std::map<HumNum, std::vector<HTp>>> m_mensurations;

};

// END_MERGE

} // end namespace hum

#endif /* _TOOL_MODORI_H */



