//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Fri Jul 30 05:14:13 CEST 2021
// Last Modified: Fri Jul 30 05:14:16 CEST 2021
// Filename:      tool-gasparize.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-gasparize.h
// Syntax:        C++11; humlib
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Interface for gasparize tool.
//                ** Create tied notes across barlines where there is an invisible rest
//                  afterwards.
//                ** Remove staccatos (these were used to create visual augmentation dots
//                  across invisible barlines.
//                ** Expand instrument names.
//                ** Create instrument abbreviations.
//                ** Add bibliographic records.
//                ** Adjust system decoration (to "[*]").
//                ** Delete dummy transpositions (if present)
//                ** Delete finale page breaks (hand-encoded printed edition breaks)
//                ** Remove key designations
//                ** Convert medial == to =||.
//                ** Add terminal ==
//                ** Remove double sharp/flats (usually related to transpositions)
//                ** Interpret "(   )" as "j" editorial accidental
//

#ifndef _TOOL_GASPARIZE_H_INCLUDED
#define _TOOL_GASPARIZE_H_INCLUDED

#include "HumTool.h"
#include "HumdrumFile.h"

namespace hum {

// START_MERGE

class Tool_gasparize : public HumTool {
	public:
		         Tool_gasparize     (void);
		        ~Tool_gasparize     () {};

		bool     run                (HumdrumFileSet& infiles);
		bool     run                (HumdrumFile& infile);
		bool     run                (const string& indata, ostream& out);
		bool     run                (HumdrumFile& infile, ostream& out);

	protected:
		void     initialize         (HumdrumFile& infile);
		void     processFile        (HumdrumFile& infile);
		void     checkDataLine      (HumdrumFile& infile, int lineindex);
		void     addBibliographicRecords(HumdrumFile& infile);
		void     fixEditorialAccidentals(HumdrumFile& infile);
		void     fixInstrumentAbbreviations(HumdrumFile& infile);
		void     addTerminalLongs   (HumdrumFile& infile);
		void     deleteDummyTranspositions(HumdrumFile& infile);
		string   getDate            (void);
		void     adjustSystemDecoration(HumdrumFile& infile);
		void     deleteBreaks       (HumdrumFile& infile);
		void     updateKeySignatures(HumdrumFile& infile, int lineindex);
		void     convertBreaks      (HumdrumFile& infile);
		void     clearStates        (void);
		void     removeArticulations(HumdrumFile& infile);
		void     fixTies            (HumdrumFile& infile);
		void     fixTiesForStrand   (HTp sstart, HTp send);
		void     fixTieToInvisibleRest(HTp first, HTp second);
		void     fixHangingTie      (HTp first, HTp second);
		void     addMensurations    (HumdrumFile& infile);
		void     addMensuration     (int top, HumdrumFile& infile, int i);
		void     createEditText     (HumdrumFile& infile);
		bool     addEditStylingForText(HumdrumFile& infile, HTp sstart, HTp send);
		string   getEditLine        (const string& text, int fieldindex, HLp line);
		bool     insertEditText     (const string& text, HumdrumFile& infile, int line, int field);
		void     adjustIntrumentNames(HumdrumFile& infile);
		void     removeKeyDesignations(HumdrumFile& infile);
		void     fixBarlines        (HumdrumFile& infile);
		void     fixFinalBarline    (HumdrumFile& infile);
		void     removeDoubledAccidentals(HumdrumFile& infile);
		void     createJEditorialAccidentals(HumdrumFile& infile);
		void     createJEditorialAccidentals(HTp sstart, HTp send);
		void     convertNextNoteToJAccidental(HTp current);
		void     fixTieStartEnd(HumdrumFile& infile);
		void     fixTiesStartEnd(HTp starts, HTp ends);

	private:
		vector<vector<int>> m_pstates;
		vector<vector<int>> m_kstates;
		vector<vector<bool>> m_estates;

};

// END_MERGE

} // end namespace hum

#endif /* _TOOL_GASPARIZE_H_INCLUDED */



