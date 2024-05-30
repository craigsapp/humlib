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
#include "HumRegex.h"

#include <iostream>
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
		int               readString          (std::istream& input);
		int               readString          (std::stringstream& input);
		int               read                (std::istream& input);
		MuseData&         operator[]          (int index);
		int               getFileCount        (void);
		void              deletePart          (int index);
		void              cleanLineEndings    (void);
		std::vector<int>  getGroupIndexList   (const std::string& group);
		int               appendPart          (MuseData* musedata);

		std::string       getError            (void);
		bool              hasError            (void);
		void              clearError          (void);

		// MIDI related information
		double            getMidiTempo        (void);


	private:
		std::vector<MuseData*>  m_part;
		std::string             m_error;

	protected:
		void              analyzeSetType      (std::vector<int>& types,
		                                       std::vector<std::string>& lines);
		void              analyzePartSegments (std::vector<int>& startindex,
		                                       std::vector<int>& stopindex,
		                                       std::vector<std::string>& lines);
		void              setError            (const std::string& error);

};


std::ostream& operator<<(std::ostream& out, MuseDataSet& musedata);


// END_MERGE

} // end namespace hum

#endif  /* _MUSEDATASET_H_INCLUDED */



