#ifndef _TOOL_FIGUREDBASS_H
#define _TOOL_FIGUREDBASS_H

#include "HumTool.h"
#include "HumdrumFile.h"
#include "NoteGrid.h"

namespace hum {

// START_MERGE

class FiguredBassNumber {
	public:
		int number;
		string accidentals;
		FiguredBassNumber(int num, string accid);
		string toString(bool nonCompoundIntervalsQ, bool noAccidentalsQ);
};

class FiguredBassSlice {
	public:
		int lineIndex;
		vector<FiguredBassNumber*> numbers;
		FiguredBassSlice(int index);
		void add(FiguredBassNumber* number);
		string toString(bool nonCompoundIntervalsQ, bool noAccidentalsQ);
};

class FiguredBassTrack {
	public:
		int voiceIndex;
		vector<FiguredBassSlice*> slices;
		FiguredBassTrack(int index);
		void add(FiguredBassSlice* slice);
};

class Tool_figuredbass : public HumTool {

	public:
		Tool_figuredbass(void);
		~Tool_figuredbass(){};

		bool run (HumdrumFileSet& infiles);
		bool run (HumdrumFile& infile);
		bool run (const string& indata, ostream& out);
		bool run (HumdrumFile& infile, ostream& out);

		vector<string> getTrackData(FiguredBassTrack* track, int lineCount, bool nonCompoundIntervalsQ, bool noAccidentalsQ);

		FiguredBassNumber* createFiguredBassNumber(NoteCell* base, NoteCell* target);

	// protected:

	private:
		bool nonCompoundIntervalsQ = false;
		bool noAccidentalsQ = false;
		int baseQ = 0;

};

// END_MERGE

} // end namespace hum

#endif /* _TOOL_FIGUREDBASS_H */
