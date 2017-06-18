//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Nov 28 08:55:15 PST 2016
// Last Modified: Mon Nov 28 08:55:38 PST 2016
// Filename:      tool-dissonant.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-dissonant.h
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
		                            vector<vector<NoteCell*> >& attacks,
		                            bool debug);
		void    doAnalysisForVoice (vector<vector<string> >& results,
		                            NoteGrid& grid,
		                            vector<NoteCell*>& attacks,
		                            int vindex, bool debug);
		void    findFakeSuspensions(vector<vector<string> >& results, 
		                            NoteGrid& grid,
		                            vector<NoteCell*>& attacks, int vindex);
		void    findLs			   (vector<vector<string> >& results, 
		                            NoteGrid& grid,
		                            vector<NoteCell*>& attacks, int vindex);
		void    findYs			   (vector<vector<string> >& results, 
		                            NoteGrid& grid,
		                            vector<NoteCell*>& attacks, int vindex);
		void    changePitch        (HTp note2, HTp note1);

		void    printColorLegend   (HumdrumFile& infile);
		int     getNextPitchAttackIndex(NoteGrid& grid, int voicei,
		                            int sliceindex);
		void    fillLabels         (void);
		void    fillLabels2        (void);
		void    printCountAnalysis (vector<vector<string> >& data);
		void    suppressDissonances(HumdrumFile& infile, NoteGrid& grid,
		                            vector<vector<NoteCell* > >& attacks,
		                            vector<vector<string> >& results);
		void    suppressDissonancesInVoice(HumdrumFile& infile, 
		                            NoteGrid& grid, int vindex,
		                            vector<NoteCell*>& attacks,
		                            vector<string>& results);
		void    mergeWithPreviousNote(HumdrumFile& infile,
		                            vector<NoteCell*>& attacks, int index);

	private:
	 	vector<HTp> m_kernspines;
		bool diss2Q = false;
		bool diss7Q = false;
		bool diss4Q = false;
		bool dissL0Q = false;
		bool dissL1Q = false;
		bool dissL2Q = false;
		bool suppressQ = false;

		vector<string> m_labels;

		// unaccdented non-harmonic tones:
		const int PASSING_UP           =  0; // rising passing tone
		const int PASSING_DOWN         =  1; // downward passing tone
		const int NEIGHBOR_UP          =  2; // upper neighbor
		const int NEIGHBOR_DOWN        =  3; // lower neighbor
		const int ECHAPPE_UP           =  4; // upper échappée
		const int ECHAPPE_DOWN         =  5; // lower échappée
		const int CAMBIATA_UP_S        =  6; // ascending short nota cambiata
		const int CAMBIATA_DOWN_S      =  7; // descending short nota cambiata
		const int CAMBIATA_UP_L        =  8; // ascending long nota cambiata
		const int CAMBIATA_DOWN_L      =  9; // descending long nota cambiata
		// const int IPOSTHI_NEIGHBOR     = 10; // incomplete posterior upper neighbor
		// const int IPOSTLOW_NEIGHBOR    = 11; // incomplete posterior lower neighbor
		// const int IANTHI_NEIGHBOR      = 12; // incomplete anterior upper neighbor
		// const int IANTLOW_NEIGHBOR     = 13; // incomplete anterior lower neighbor
		const int ANT_UP               = 10; // rising anticipation
		const int ANT_DOWN             = 11; // descending anticipation

		// accented non-harmonic tones:
		const int THIRD_Q_PASS_UP      = 12; // dissonant third quarter
		const int THIRD_Q_PASS_DOWN    = 13; // dissonant third quarter
		const int THIRD_Q_UPPER_NEI    = 14; // dissonant third quarter
		const int THIRD_Q_LOWER_NEI    = 15; // dissonant third quarter
		const int SUS_BIN  	           = 16; // binary suspension
		const int SUS_TERN  	       = 17; // ternary suspension
		const int AGENT_BIN		       = 18; // binary agent
		const int AGENT_TERN		   = 19; // ternary agent
		const int SUSPENSION_ORNAM     = 20; // suspension ornament
		const int SUSPENSION_REP       = 21; // suspension repeated note
		const int FAKE_SUSPENSION_UP   = 22; // fake suspension approached by step up
		const int FAKE_SUSPENSION_DOWN = 23; // fake suspension approached by step down
		const int SUS_NO_AGENT_UP      = 24; // suspension missing a normal agent approached by step up
		const int SUS_NO_AGENT_DOWN    = 25; // suspension missing a normal agent approached by step down
		const int CHANSON_IDIOM        = 26; // chanson idiom

		// unknown dissonances:
		const int PARALLEL_UP          = 27; // moves in parallel with known dissonant, approached from below
		const int PARALLEL_DOWN        = 28; // moves in parallel with known dissonant, approached from above
		const int ONLY_WITH_VALID_UP   = 29; // only dissonant with identifiable dissonances, approached from below
		const int ONLY_WITH_VALID_DOWN = 30; // only dissonant with identifiable dissonances, approached from above
		const int UNKNOWN_DISSONANCE   = 31; // unknown dissonance type
		const int UNLABELED_Z2         = 32; // unknown dissonance type, 2nd interval
		const int UNLABELED_Z7         = 33; // unknown dissonance type, 7th interval
		const int UNLABELED_Z4         = 34; // unknown dissonance type, 4th interval

		const int LABELS_SIZE          = 35; // one more than last index
};

// END_MERGE

} // end namespace hum

#endif /* _TOOL_DISSONANT_H */



