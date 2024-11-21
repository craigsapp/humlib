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
		            FiguredBassNumber(int num, string accid, bool showAccid, int voiceIndex, int lineIndex, bool isAttack, bool intervallsatz, string intervalQuality, bool hint);
		std::string toString(bool nonCompoundIntervalsQ, bool noAccidentalsQ, bool hideThreeQ);
		int         getNumberWithinOctave(void);

		int         m_voiceIndex;
		int         m_lineIndex;
		int         m_number;
		std::string m_accidentals;
		bool        m_showAccidentals; // Force shoing figured base numbers when they need an accidental
		bool        m_baseOfSustainedNoteDidChange;
		bool        m_isAttack;
		bool        m_convert2To9 = false;
		bool        m_intervallsatz = false;
		std::string m_intervalQuality;
		bool        m_hint = false;

};

class FiguredBassAbbreviationMapping {
	public:
		FiguredBassAbbreviationMapping(string s, vector<int> n);

		const static vector<FiguredBassAbbreviationMapping> s_mappings;

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
		void                       initialize                             (void);
        void                       processFile                            (HumdrumFile& infile);
		bool                       hideNumbersForTokenLine                (HTp token, pair<int, HumNum> timeSig);
		vector<string>             getTrackData                           (const vector<FiguredBassNumber*>& numbers, int lineCount);
		vector<string>             getTrackDataForVoice                   (int voiceIndex, const vector<FiguredBassNumber*>& numbers, int lineCount);
		FiguredBassNumber*         createFiguredBassNumber                (int basePitchBase40, int targetPitchBase40, int voiceIndex, int lineIndex, bool isAttack, string keySignature);
		vector<FiguredBassNumber*> filterNegativeNumbers                  (vector<FiguredBassNumber*> numbers);
		vector<FiguredBassNumber*> filterFiguredBassNumbersForLine        (vector<FiguredBassNumber*> numbers, int lineIndex);
		vector<FiguredBassNumber*> filterFiguredBassNumbersForLineAndVoice(vector<FiguredBassNumber*> numbers, int lineIndex, int voiceIndex);
		string                     formatFiguredBassNumbers               (const vector<FiguredBassNumber*>& numbers);
		vector<FiguredBassNumber*> analyzeChordNumbers                    (const vector<FiguredBassNumber*>& numbers);
		vector<FiguredBassNumber*> getAbbreviatedNumbers                  (const vector<FiguredBassNumber*>& numbers);
		string                     getNumberString                        (vector<FiguredBassNumber*> numbers);
		string                     getKeySignature                        (HumdrumFile& infile, int lineIndex);
		int                        getLowestBase40Pitch                   (vector<int> base40Pitches);
		string                     getIntervalQuality                     (int basePitchBase40, int targetPitchBase40);


	private:
		bool   m_compoundQ      = false;
		bool   m_accidentalsQ   = false;
		int    m_baseTrackQ     = 1;
		bool   m_intervallsatzQ = false;
		bool   m_sortQ          = false;
		bool   m_lowestQ        = false;
		bool   m_normalizeQ     = false;
		bool   m_reduceQ        = false;
		bool   m_attackQ        = false;
		bool   m_figuredbassQ   = false;
		bool   m_hideThreeQ     = false;
		bool   m_showNegativeQ  = false;
		bool   m_aboveQ         = false;
		string m_rateQ          = "";
		bool   m_hintQ          = false;

		string m_spineTracks    = ""; // used with -s option
		string m_kernTracks     = ""; // used with -k option
		vector<bool> m_selectedKernSpines;  // used with -k and -s option

};

// END_MERGE

} // end namespace hum

#endif /* _TOOL_FB_H */



