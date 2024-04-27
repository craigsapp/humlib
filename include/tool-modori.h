//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Wed Dec 16 11:16:33 PST 2020
// Last Modified: Mon May  2 21:56:24 PDT 2022
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

#include <ostream>
#include <string>
#include <vector>

namespace hum {

// START_MERGE

class Tool_modori : public HumTool {
	public:
		         Tool_modori                  (void);
		        ~Tool_modori                  () {};

		bool     run                          (HumdrumFileSet& infiles);
		bool     run                          (HumdrumFile& infile);
		bool     run                          (const std::string& indata, std::ostream& out);
		bool     run                          (HumdrumFile& infile, std::ostream& out);

	protected:
		void     processFile                  (HumdrumFile& infile);
		void     initialize                   (void);
		void     printInfo                    (void);
		void     switchModernOriginal         (HumdrumFile& infile);
		bool     swapKeyStyle                 (HTp one, HTp two);
		bool     swapClefStyle                (HTp one, HTp two);
		bool     swapInstrumentAbbreviationStyle(HTp one, HTp two);
		bool     swapInstrumentNameStyle      (HTp one, HTp two);
		bool     flipMensurationStyle         (HTp token);

		void     convertKeySignatureToModern  (HTp token);
		void     convertKeySignatureToOriginal(HTp token);
		void     convertKeySignatureToRegular (HTp token);

		void     convertClefToModern          (HTp token);
		void     convertClefToOriginal        (HTp token);
		void     convertClefToRegular         (HTp token);

		void     convertMensurationToModern   (HTp token);
		void     convertMensurationToOriginal (HTp token);
		void     convertMensurationToRegular  (HTp token);

		void     convertInstrumentNameToModern   (HTp token);
		void     convertInstrumentNameToOriginal (HTp token);
		void     convertInstrumentNameToRegular  (HTp token);

		void     convertInstrumentAbbreviationToModern   (HTp token);
		void     convertInstrumentAbbreviationToOriginal (HTp token);
		void     convertInstrumentAbbreviationToRegular  (HTp token);

		int      getPairedReference           (int index, std::vector<std::string>& keys);
		void     storeModOriReferenceRecords  (HumdrumFile& infile);
		void     processExclusiveInterpretationLine(HumdrumFile& infile, int line);
		bool     processStaffCompanionSpines  (std::vector<HTp> tokens);
		bool     processStaffSpines           (std::vector<HTp>& tokens);
		void     updateLoMo                   (HumdrumFile& infile);
		void     processLoMo                  (HTp lomo);
		void     printModoriOutput            (HumdrumFile& infile);
		bool     swapMensurationStyle         (HTp one, HTp two);

	private:
		bool m_modernQ        = false; // -m option: show modern key/clef/time signatures
		bool m_originalQ      = false; // -o option: show original key/clef/mensuration
		bool m_infoQ          = false; // show key/clef/mensuration tokens in data

		bool m_nokeyQ         = false; // -K option: don't change key signatures
		bool m_noclefQ        = false; // -C option: don't change clefs
		bool m_nomensurationQ = false; // -M option: don't change mensurations
		bool m_nolyricsQ      = false; // -L option: don't change **text
		bool m_nolotextQ      = false; // -T option: don't change !LO:TX
		bool m_norefsQ        = false; // -R option: don't change !LO:TX
		bool m_nolabelsQ      = false; // -L option: don't change *I"
		bool m_nolabelAbbrsQ  = false; // -A option: don't change *I"

		std::vector<std::map<HumNum, std::vector<HTp>>> m_keys;
		std::vector<std::map<HumNum, std::vector<HTp>>> m_clefs;
		std::vector<std::map<HumNum, std::vector<HTp>>> m_mensurations;
		std::vector<std::map<HumNum, std::vector<HTp>>> m_labels;
		std::vector<std::map<HumNum, std::vector<HTp>>> m_labelAbbrs;
		std::vector<std::pair<HTp, HTp>> m_references;
		std::vector<HTp> m_lyrics;
		std::vector<HTp> m_lotext;
		std::vector<HTp> m_lomo;

};

// END_MERGE

} // end namespace hum

#endif /* _TOOL_MODORI_H */



