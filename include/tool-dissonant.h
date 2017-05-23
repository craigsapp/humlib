//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Nov 28 08:55:15 PST 2016
// Last Modified: Mon Nov 28 08:55:38 PST 2016
// Filename:      tool-dissonant.h
// URL:           https://github.com/craigsapp/minHumdrum/blob/master/include/tool-dissonant.h
// Syntax:        C++11
// vim:           ts=3 noexpandtab
//
// Description:   Interface for dissonant tool.
//

#ifndef _TOOL_DISSONANT_H
#define _TOOL_DISSONANT_H

#include "HumTool.h"
#include "HumdrumFile.h"
#include "NoteGrid.h"

namespace hum {

// START_MERGE

class Tool_dissonant : public HumTool {
	public:
		         Tool_dissonant    (void);
		        ~Tool_dissonant    () {};

		bool     run               (HumdrumFile& infile);
		bool     run               (const string& indata, ostream& out);
		bool     run               (HumdrumFile& infile, ostream& out);

	protected:
		void    doAnalysis         (vector<vector<string> >& results,
		                            NoteGrid& grid,
		                            bool debug);
		void    doAnalysisForVoice (vector<vector<string> >& results, NoteGrid& grid,
		                            int vindex, bool debug);
		void    printColorLegend   (HumdrumFile& infile);
		int     getNextPitchAttackIndex(NoteGrid& grid, int voicei, int sliceindex);
		void    fillLabels         (void);
		void    fillLabels2        (void);

	private:
	 	vector<HTp> m_kernspines;
		bool diss2Q = false;
		bool diss7Q = false;
		bool diss4Q = false;
		bool dissL0Q = false;
		bool dissL1Q = false;
		bool dissL2Q = false;

		vector<string> m_labels;

		// unaccdented non-harmonic tones:
      const int UNKNOWN_DISSONANCE =  0; // unknown dissonance type
		const int UNLABELED_Z2       =  1; // unknown dissonance type, 2nd interval
		const int UNLABELED_Z7       =  2; // unknown dissonance type, 7th interval
		const int UNLABELED_Z4       =  3; // unknown dissonance type, 4th interval
		const int PASSING_UP         =  4; // rising passing tone
		const int PASSING_DOWN       =  5; // downward passing tone
		const int NEIGHBOR_UP        =  6; // upper neighbor
		const int NEIGHBOR_DOWN      =  7; // lower neighbor
		const int ECHAPPE_UP         =  8; // upper échappée
		const int ECHAPPE_DOWN       =  9; // lower échappée
		const int CAMBIATA_UP_S      = 10; // ascending short nota cambiata
		const int CAMBIATA_DOWN_S    = 11; // descending short nota cambiata
		const int CAMBIATA_UP_L      = 12; // ascending long nota cambiata
		const int CAMBIATA_DOWN_L    = 13; // descending long nota cambiata
		const int IPOSTHI_NEIGHBOR   = 14; // incomplete posterior upper neighbor
		const int IPOSTLOW_NEIGHBOR  = 15; // incomplete posterior lower neighbor
		const int IANTHI_NEIGHBOR    = 16; // incomplete anterior upper neighbor
		const int IANTLOW_NEIGHBOR   = 17; // incomplete anterior lower neighbor
		const int ANT_UP             = 18; // rising anticipation
		const int ANT_DOWN           = 19; // descending anticipation

		// unaccdented non-harmonic tones:
		const int THIRD_QUARTER      = 20; // dissonant third quarter
		const int SUSPENSION         = 21; // suspension
		const int SUSPENSION_AGENT   = 22; // suspension agent
		const int SUSPENSION_ORNAM   = 23; // suspension ornament
		const int SUSPENSION_REP     = 24; // suspension repeated note
		const int CHANSON_IDIOM      = 25; // chanson idiom
		const int LABELS_SIZE        = 26; // one more than last index
};

// END_MERGE

} // end namespace hum

#endif /* _TOOL_DISSONANT_H */



