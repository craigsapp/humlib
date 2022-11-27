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
		FiguredBassNumber(int num, string accid, int voiceIndex, int lineIndex);
		string toString(bool nonCompoundIntervalsQ, bool noAccidentalsQ);
};

class Tool_figuredbass : public HumTool {

	public:
		Tool_figuredbass(void);
		~Tool_figuredbass(){};

		bool run (HumdrumFileSet& infiles);
		bool run (HumdrumFile& infile);
		bool run (const string& indata, ostream& out);
		bool run (HumdrumFile& infile, ostream& out);

		vector<string> getTrackData(vector<FiguredBassNumber*> numbers, int lineCount, bool nonCompoundIntervalsQ, bool noAccidentalsQ);

		FiguredBassNumber* createFiguredBassNumber(NoteCell* base, NoteCell* target);
		
		vector<FiguredBassNumber*> findFiguredBassNumbersForLine(vector<FiguredBassNumber*>, int lineIndex);

		string formatFiguredBassNumbers(vector<FiguredBassNumber*> numbers, bool nonCompoundIntervalsQ, bool noAccidentalsQ);

	// protected:

	private:
		bool nonCompoundIntervalsQ = false;
		bool noAccidentalsQ = false;
		int baseQ = 0;

};

// END_MERGE

} // end namespace hum

#endif /* _TOOL_FIGUREDBASS_H */
