#ifndef _TOOL_FIGUREDBASS_H
#define _TOOL_FIGUREDBASS_H

#include "HumTool.h"
#include "HumdrumFile.h"
#include "NoteGrid.h"

namespace hum {

// START_MERGE

class Tool_figuredbass : public HumTool {

	public:
		Tool_figuredbass(void);
		~Tool_figuredbass(){};

		bool run (HumdrumFileSet& infiles);
		bool run (HumdrumFile& infile);
		bool run (const string& indata, ostream& out);
		bool run (HumdrumFile& infile, ostream& out);

	// protected:

	private:
		bool nonCompoundIntervalsQ = false;
		bool noAccidentalsQ = false;
		int baseQ = 0;

};

// END_MERGE

} // end namespace hum

#endif /* _TOOL_FIGUREDBASS_H */
