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
		bool showAccidentals;
		bool currAttackNumberDidChange;
		bool isAttack;
		FiguredBassNumber(int num, string accid, bool showAccid, int voiceIndex, int lineIndex, bool isAttack);
		string toString(bool nonCompoundIntervalsQ, bool noAccidentalsQ);
		int getNumberB7();
};

class FiguredBassAbbr {
	public:
		string str;
		vector<int> numbers;
		FiguredBassAbbr(string s, vector<int> n);
};

class Tool_fb : public HumTool {

	public:
		Tool_fb(void);
		~Tool_fb(){};

		bool run (HumdrumFileSet& infiles);
		bool run (HumdrumFile& infile);
		bool run (const string& indata, ostream& out);
		bool run (HumdrumFile& infile, ostream& out);

		vector<string> getTrackData(vector<FiguredBassNumber*> numbers, int lineCount);
		
		vector<string> getTrackDataForVoice(int voiceIndex, vector<FiguredBassNumber*> numbers, int lineCount);

		FiguredBassNumber* createFiguredBassNumber(NoteCell* base, NoteCell* target, string keySignature);
		
		vector<FiguredBassNumber*> filterFiguredBassNumbersForLine(vector<FiguredBassNumber*>, int lineIndex);
		
		vector<FiguredBassNumber*> filterFiguredBassNumbersForLineAndVoice(vector<FiguredBassNumber*>, int lineIndex, int voiceIndex);

		string formatFiguredBassNumbers(vector<FiguredBassNumber*> numbers);

		vector<FiguredBassNumber*> getAbbrNumbers(vector<FiguredBassNumber*>);

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
