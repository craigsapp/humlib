//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Thu Jun 17 13:17:50 PDT 2010
// Last Modified: Wed Sep 25 07:04:59 PDT 2019 Convert to STL
// Filename:      humlib/include/MuseDataSet.h
// Web Address:   https://github.com/craigsapp/humlib/blob/master/include/MuseDataSetSet.h
// Syntax:        C++ 
// vim:           ts=3
//
// Description:   A class that stores a collection of MuseDataSet files
//                representing parts in the same score.
//

#ifndef _MUSEDATASET_H_INCLUDED
#define _MUSEDATASET_H_INCLUDED

#include "MuseData.h"

#include <vector>

namespace hum {

// START_MERGE

class MuseDataSet {
	public:
		                  MuseDataSet         (void);
		                  MuseDataSet         (MuseDataSet& input);
		                 ~MuseDataSet         () { clear(); }

		void              clear               (void);
		int               readPartFile        (const std::string& filename);
		int               readPartString      (const std::string& data);
		int               readPart            (std::istream& input);
		int               readFile            (const std::string& filename);
		int               readString          (const std::string& data);
		int               read                (std::istream& input);
		MuseData&         operator[]          (int index);
		int               getPartCount        (void);
		void              deletePart          (int index);
		void              cleanLineEndings    (void);

	private:
		std::vector<MuseData*>  m_part;

	protected:
		int               appendPart          (MuseData* musedata);
		void              analyzeSetType      (std::vector<int>& types, 
		                                       std::vector<std::string>& lines);
		void              analyzePartSegments (std::vector<int>& startindex, 
		                                       std::vector<int>& stopindex, 
		                                       std::vector<std::string>& lines);
};


std::ostream& operator<<(std::ostream& out, MuseDataSet& musedata);


// END_MERGE

} // end namespace hum

#endif  /* _MUSEDATASET_H_INCLUDED */



