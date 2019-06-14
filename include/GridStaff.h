//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sun Oct 16 16:08:05 PDT 2016
// Last Modified: Sun Oct 16 16:08:08 PDT 2016
// Filename:      GridStaff.h
// URL:           https://github.com/craigsapp/hum2ly/blob/master/include/GridStaff.h
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   HumGrid is an intermediate container for converting from
//                MusicXML syntax into Humdrum syntax. GridStaff is used
//                to store all voices/layers for particular staff in a
//                particular MusicXML <part>.
//
//

#ifndef _GRIDSTAFF_H
#define _GRIDSTAFF_H

#include "GridCommon.h"
#include "GridSide.h"
#include "GridVoice.h"

#include <iostream>
#include <string>
#include <vector>

using namespace std;

namespace hum {

// START_MERGE

class GridStaff : public std::vector<GridVoice*>, public GridSide {
	public:
		GridStaff(void);
		~GridStaff();
		GridVoice* setTokenLayer (int layerindex, HTp token, HumNum duration);
		void setNullTokenLayer   (int layerindex, SliceType type, HumNum nextdur);
		void appendTokenLayer    (int layerindex, HTp token, HumNum duration,
		                          const std::string& spacer = " ");
		int getMaxVerseCount     (void);
		string getString         (void);
};

std::ostream& operator<<(std::ostream& output, GridStaff* staff);


// END_MERGE

} // end namespace hum

#endif /* _GRIDSTAFF_H */



