//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Wed Sep 25 19:16:44 PDT 2019
// Last Modified: Wed Sep 25 19:16:53 PDT 2019
// Filename:      tool-musedata2hum.h
// URL:           https://github.com/craigsapp/musedata2hum/blob/master/include/tool-musedata2hum.h
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Inteface to convert a MuseData file into a Humdrum file.
//

#ifndef _TOOL_MUSEDATA2HUM_H
#define _TOOL_MUSEDATA2HUM_H

#define _USE_HUMLIB_OPTIONS_

#include "Options.h"
#include "MuseDataSet.h"
#include "HumdrumToken.h"
#include "HumTool.h"
#include "HumGrid.h"

#include <string>
#include <vector>

namespace hum {

// START_MERGE

class Tool_musedata2hum : public HumTool {
	public:
		        Tool_musedata2hum    (void);
		       ~Tool_musedata2hum    () {}

		bool    convertFile          (ostream& out, const string& filename);
		bool    convertString        (ostream& out, const string& input);
		bool    convert              (ostream& out, MuseDataSet& mds);
		bool    convert              (ostream& out, istream& input);

		void    setOptions           (int argc, char** argv);
		void    setOptions           (const std::vector<std::string>& argvlist);
		Options getOptionDefinitions (void);
		void    setInitialOmd        (const string& omd);

	protected:
		void    initialize           (void);
		void    convertLine          (GridMeasure* gm, MuseRecord& mr);
		bool    convertPart          (HumGrid& outdata, MuseDataSet& mds, int index);
		int     convertMeasure       (HumGrid& outdata, MuseData& part, int partindex, int startindex);
		GridMeasure* getMeasure      (HumGrid& outdata, HumNum starttime);
		void    setTimeSigDurInfo    (const std::string& mtimesig);
		void    setMeasureStyle      (GridMeasure* gm, MuseRecord& mr);
		void    setMeasureNumber     (GridMeasure* gm, MuseRecord& mr);
		void    storePartName        (HumGrid& outdata, MuseData& part, int index);
		void    addNoteDynamics      (GridSlice* slice, int part, 
		                              MuseRecord& mr);
		void    addLyrics            (GridSlice* slice, int part, int staff, MuseRecord& mr);
		void    addFiguredHarmony    (MuseRecord& mr, GridMeasure* gm,
		                              HumNum timestamp, int part, int maxstaff);
		std::string trimSpaces       (std::string input);

	private:
		// options:
		Options m_options;
		bool    m_stemsQ = false;    // used with -s option
		bool    m_recipQ = false;    // used with -r option
		std::string m_omd = "";      // initial tempo designation (store for later output)

		// state variables:
		int m_tpq      = 1;          // Ticks per quarter note
		int m_part     = 0;          // staff index currently being processed
		int m_maxstaff = 0;          // total number of staves (parts)
		HumNum m_timesigdur = 4;     // duration of current time signature in quarter notes
		HTp m_lastfigure = NULL;     // last figured bass token
		int m_lastbarnum = -1;       // barnumber carried over from previous bar
		HTp m_lastnote = NULL;       // for dealing with chords.

};


// END_MERGE

}  // end of namespace hum

#endif /* _TOOL_MUSEDATA2HUM_H */


