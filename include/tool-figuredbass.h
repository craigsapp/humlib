#ifndef _TOOL_FIGUREDBASS_H
#define _TOOL_FIGUREDBASS_H

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
		FiguredBassNumber(int num, string accid, bool showAccid, int voiceIndex, int lineIndex);
		string toString(bool nonCompoundIntervalsQ, bool noAccidentalsQ);
		int getNumberB7();
};

class FiguredBassAbbr {
	public:
		string str;
		vector<int> numbers;
		FiguredBassAbbr(string s, vector<int> n);
};

class Tool_figuredbass : public HumTool {

	public:
		Tool_figuredbass(void);
		~Tool_figuredbass(){};

		bool run (HumdrumFileSet& infiles);
		bool run (HumdrumFile& infile);
		bool run (const string& indata, ostream& out);
		bool run (HumdrumFile& infile, ostream& out);

		vector<string> getTrackData(vector<FiguredBassNumber*> numbers, int lineCount, bool nonCompoundIntervalsQ, bool noAccidentalsQ, bool sortQ, bool normalizeQ, bool abbrQ);
		
		vector<string> getTrackDataForVoice(int voiceIndex, vector<FiguredBassNumber*> numbers, int lineCount, bool nonCompoundIntervalsQ, bool noAccidentalsQ, bool sortQ, bool normalizeQ, bool abbrQ);

		FiguredBassNumber* createFiguredBassNumber(NoteCell* base, NoteCell* target, string keySignature);
		
		vector<FiguredBassNumber*> filterFiguredBassNumbersForLine(vector<FiguredBassNumber*>, int lineIndex);
		
		vector<FiguredBassNumber*> filterFiguredBassNumbersForLineAndVoice(vector<FiguredBassNumber*>, int lineIndex, int voiceIndex);

		string formatFiguredBassNumbers(vector<FiguredBassNumber*> numbers, bool nonCompoundIntervalsQ, bool noAccidentalsQ, bool sortQ, bool normalizeQ, bool abbrQ);

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

};

// END_MERGE

} // end namespace hum

#endif /* _TOOL_FIGUREDBASS_H */
