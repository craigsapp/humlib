//
//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Fri Nov 25 19:41:43 PST 2016
// Last Modified: Fri Nov 25 19:41:49 PST 2016
// Filename:      NoteGrid.cpp
// URL:           TBD
// Syntax:        C++11
// vim:           ts=3 noexpandtab
// Description:   Manages a 2D array of NoteCells for each timeslice
//                in the Humdrum file score.
//

#ifndef _NOTEGRID_H
#define _NOTEGRID_H

#include "humlib.h"
#include "GridCell.h"

using namespace std;


namespace hum {


class NoteGrid {
	public:
		           NoteGrid           (void) { }
		           NoteGrid           (HumdrumFile& infile);
		          ~NoteGrid           ();

		void       clear              (void);

		bool       load               (HumdrumFile& infile);
		GridCell*  cell               (int voiceindex, int sliceindex);
		int        getVoiceCount      (void);
		int        getSliceCount      (void);
		int        getLineIndex       (int sindex);

		void       printDiatonicGrid(ostream& out);
		void       printMidiGrid      (ostream& out);
		void       printBase40Grid    (ostream& out);
		void       printRawGrid       (ostream& out);
		void       printKernGrid      (ostream& out);

		int        getDiatonicPitch   (int vindex, int sindex);
		int        getMidiPitch       (int vindex, int sindex);
		int        getBase40Pitch     (int vindex, int sindex);
		string     getKernPitch       (int vindex, int sindex);
		HTp        getToken           (int vindex, int sindex);

		int        getPrevAttackDiatonic(int vindex, int sindex);
		int        getNextAttackDiatonic(int vindex, int sindex);

		void       printCellInfo(ostream& out);
		void       printCellInfo(ostream& out, int vindex);

	protected:
		void       buildAttackIndexes (void);
		void       buildAttackIndex   (int vindex);

	private:
		vector<vector<GridCell*> > m_grid;
		vector<HTp>                m_kernspines;

};


} // end namespace hum

#endif /* _NOTEGRID_H */



