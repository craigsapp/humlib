//
// Programmer:    Wolfgang Drescher <drescher.wolfgang@gmail.com>
// Creation Date: Fri Dez 13 08:08:00 UTC 2024
// Filename:      tool-beat.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-beat.h
// Syntax:        C++11; humlib
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Interface for beat tool
//

#ifndef _TOOL_BEAT_H
#define _TOOL_BEAT_H

#include "HumTool.h"
#include "HumdrumFile.h"

using namespace std;

namespace hum {

// START_MERGE

class Tool_beat : public HumTool {

	public:
		     Tool_beat (void);
		     ~Tool_beat() {};

		bool run           (HumdrumFileSet& infiles);
		bool run           (HumdrumFile& infile);
		bool run           (const string& indata, ostream& out);
		bool run           (HumdrumFile& infile, ostream& out);

	protected:
		void               initialize  (void);
        void               processFile (HumdrumFile& infile);
		vector<string>     getTrackData(HumdrumFile& infile, int track);

	private:
		bool        m_floatQ;               // used with -f option
		int         m_digits = 0;           // used with -D option
		bool        m_commaQ;               // used with -c option
		bool        m_durationQ;            // used with -d option
		bool        m_restQ;                // used with -r option
		bool        m_tiedQ;                // used with -t option
		bool        m_fullQ;                // used with --full option
		string      m_beatsize;             // used with -u option
		string      m_spineTracks = "";     // used with -s option
		string      m_kernTracks = "";      // used with -k option
		vector<bool> m_selectedKernSpines;  // used with -k and -s option
};

// END_MERGE

} // end namespace hum

#endif /* _TOOL_BEAT_H */
