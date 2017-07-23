//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sun Oct 16 16:08:05 PDT 2016
// Last Modified: Sun Oct 16 16:08:08 PDT 2016
// Filename:      HumGrid.h
// URL:           https://github.com/craigsapp/hum2ly/blob/master/include/HumGrid.h
// Syntax:        C++11
// vim:           ts=3 noexpandtab
//
// Description:   HumGrid is an intermediate container for converting from
//                MusicXML syntax into Humdrum syntax.
//
//

#ifndef _GRIDSIDE_H
#define _GRIDSIDE_H

#include <vector>
#include <list>

#include "HumdrumToken.h"

using namespace std;

namespace hum {

// START_MERGE

class GridSide {
	public:
		GridSide(void);
		~GridSide();

		int   getVerseCount     (void);
		HTp   getVerse          (int index);
		void  setVerse          (int index, HTp token);

		int   getHarmonyCount   (void);
		void  setHarmony        (HTp token);
		void  detachHarmony     (void);
		HTp   getHarmony        (void);

		int   getDynamicsCount  (void);
		void  setDynamics       (HTp token);
		void  detachDynamics    (void);
		HTp   getDynamics       (void);

	private:
		vector<HumdrumToken*> m_verses;
		HumdrumToken* m_dynamics = NULL;
		HumdrumToken* m_harmony = NULL;
};


// END_MERGE

} // end namespace hum

#endif /* _HUMGRID_H */



