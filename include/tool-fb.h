//
// Programmer:    Wolfgang Drescher <drescher.wolfgang@gmail.com>
// Creation Date: Sun Nov 27 2022 00:25:34 CET
// Filename:      tool-fb.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-fb.h
// Syntax:        C++11; humlib
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Interface for fb tool, which automatically adds figured bass numbers.
//

#ifndef _TOOL_FB_H
#define _TOOL_FB_H

#include "HumTool.h"
#include "HumdrumFile.h"
#include "NoteGrid.h"

namespace hum {

// START_MERGE

class FiguredBassNumber {
	public:
		            FiguredBassNumber(int num, string accid, bool showAccid, int voiceIndex, int lineIndex, bool isAttack);
		std::string toString(bool nonCompoundIntervalsQ, bool noAccidentalsQ, bool hideThreeQ);
		int         getNumberWithinOctave(void);

		int         m_voiceIndex;
		int         m_lineIndex;
		int         m_number;
		std::string m_accidentals;
		bool        m_showAccidentals; // Force shoing figured base numbers when they need an accidental
		bool        m_currAttackNumberDidChange;
		bool        m_isAttack;

};

class FiguredBassAbbreviationMapping {
	public:
		FiguredBassAbbreviationMapping(string s, vector<int> n);

		static vector<FiguredBassAbbreviationMapping*> s_mappings;

		// String to compare the numbers with
		// e.g. "6 4 3"
		// Sorted by size, larger numbers first
		string m_str; 

		// Figured bass number as int
		vector<int> m_numbers;

};

class Tool_fb : public HumTool {

	public:
		     Tool_fb (void);
		     ~Tool_fb() {};
		bool run     (HumdrumFileSet& infiles);
		bool run     (HumdrumFile& infile);
		bool run     (const string& indata, ostream& out);
		bool run     (HumdrumFile& infile, ostream& out);

	protected:
		vector<string>             getTrackData                           (const vector<FiguredBassNumber*>& numbers, int lineCount);
		vector<string>             getTrackDataForVoice                   (int voiceIndex, const vector<FiguredBassNumber*>& numbers, int lineCount);
		FiguredBassNumber*         createFiguredBassNumber                (NoteCell* base, NoteCell* target, string keySignature);
		vector<FiguredBassNumber*> filterFiguredBassNumbersForLine        (vector<FiguredBassNumber*> numbers, int lineIndex);
		vector<FiguredBassNumber*> filterFiguredBassNumbersForLineAndVoice(vector<FiguredBassNumber*> numbers, int lineIndex, int voiceIndex);
		string                     formatFiguredBassNumbers               (const vector<FiguredBassNumber*>& numbers);
		vector<FiguredBassNumber*> getAbbreviatedNumbers                  (const vector<FiguredBassNumber*>& numbers);
		string                     getNumberString                        (vector<FiguredBassNumber*> numbers);
		string                     getKeySignature                        (HumdrumFile& infile, int lineIndex);


	private:
		bool m_compoundQ      = false;
		bool m_accidentalsQ   = false;
		int  m_baseQ          = 0;
		bool m_intervallsatzQ = false;
		bool m_sortQ          = false;
		bool m_lowestQ        = false;
		bool m_normalizeQ     = false;
		bool m_abbrQ          = false;
		bool m_attackQ        = false;
		bool m_figuredbassQ   = false;
		bool m_hideThreeQ     = false;

};

// END_MERGE

} // end namespace hum

#endif /* _TOOL_FB_H */
