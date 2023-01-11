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
		int voiceIndex;
		int lineIndex;
		int number;
		string accidentals;
		bool showAccidentals; // Force shoing figured base numbers when they need an accidental
		bool currAttackNumberDidChange;
		bool isAttack;

		FiguredBassNumber(int num, string accid, bool showAccid, int voiceIndex, int lineIndex, bool isAttack);
		string toString(bool nonCompoundIntervalsQ, bool noAccidentalsQ);
		int getNumberWithinOctave(void);
};

class FiguredBassAbbreviationMapping {
	public:
		// String to compare the numbers with
		// e.g. "6 4 3"
		// Sorted by size, larger numbers first
		string str;

		// Figured bass number as int
		vector<int> numbers;

		FiguredBassAbbreviationMapping(string s, vector<int> n);
};

class Tool_fb : public HumTool {

	public:
		Tool_fb(void);
		~Tool_fb(){};

		bool run (HumdrumFileSet& infiles);
		bool run (HumdrumFile& infile);
		bool run (const string& indata, ostream& out);
		bool run (HumdrumFile& infile, ostream& out);

		vector<string> getTrackData(const vector<FiguredBassNumber*>& numbers, int lineCount);
		
		vector<string> getTrackDataForVoice(int voiceIndex, const vector<FiguredBassNumber*>& numbers, int lineCount);

		FiguredBassNumber* createFiguredBassNumber(NoteCell* base, NoteCell* target, string keySignature);
		
		vector<FiguredBassNumber*> filterFiguredBassNumbersForLine(vector<FiguredBassNumber*> numbers, int lineIndex);
		
		vector<FiguredBassNumber*> filterFiguredBassNumbersForLineAndVoice(vector<FiguredBassNumber*> numbers, int lineIndex, int voiceIndex);

		string formatFiguredBassNumbers(const vector<FiguredBassNumber*>& numbers);

		vector<FiguredBassNumber*> getAbbreviatedNumbers(const vector<FiguredBassNumber*>& numbers);

		string getNumberString(vector<FiguredBassNumber*> numbers);

		string getKeySignature(HumdrumFile& infile, int lineIndex);

	// protected:

	private:
		bool compoundQ = false;
		bool accidentalsQ = false;
		int baseQ = 0;
		bool intervallsatzQ = false;
		bool sortQ = false;
		bool lowestQ = false;
		bool normalizeQ = false;
		bool abbrQ = false;
		bool attackQ = false;

};

// END_MERGE

} // end namespace hum

#endif /* _TOOL_FB_H */
