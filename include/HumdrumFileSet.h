//
// Copyright 1998-2001 by Craig Stuart Sapp, All Rights Reserved.
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Fri Mar 29 14:49:35 PDT 2013
// Last Modified: Sun Jul 28 20:07:50 CEST 2019 Converted to humlib
// Filename:      HumdrumFileSet.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/HumdrumFileSet.h
// Syntax:        C++11; humlib
//
// Description:   Collection of one or more Humdrum data sequences started
//                with an exclusive interpretation and ending with *-.
//

#ifndef _HUMDRUMFILESET_H_INCLUDED
#define _HUMDRUMFILESET_H_INCLUDED


#include "HumdrumFile.h"
#include "HumdrumFileStream.h"
#include "Options.h"

#include <iostream>
#include <istream>
#include <string>
#include <vector>

namespace hum {

// START_MERGE

///////////////////////////////////////////////////////////////////////////

class HumdrumFileSet {
   public:
                            HumdrumFileSet   (void);
                            HumdrumFileSet   (Options& options);
                            HumdrumFileSet   (const std::string& contents);
                           ~HumdrumFileSet   ();

      void                  clear            (void);
      void                  clearNoFree      (void);
      int                   getSize          (void);
      int                   getCount         (void) { return getSize(); }
      HumdrumFile&          operator[]       (int index);
		bool                  swap             (int index1, int index2);
		bool                  hasFilters       (void);
		bool                  hasGlobalFilters    (void);
		bool                  hasUniversalFilters (void);
		std::vector<HumdrumLine*> getUniversalReferenceRecords(void);

      int                   readFile         (const std::string& filename);
      int                   readString       (const std::string& contents);
      int                   readStringCsv    (const std::string& contents);
      int                   read             (std::istream& inStream);
      int                   read             (Options& options);
      int                   read             (HumdrumFileStream& instream);

      int                   readAppendFile   (const std::string& filename);
      int                   readAppendString (const std::string& contents);
      int                   readAppendStringCsv (const std::string& contents);
      int                   readAppend       (std::istream& inStream);
      int                   readAppend       (Options& options);
      int                   readAppend       (HumdrumFileStream& instream);
      int                   readAppendHumdrum(HumdrumFile& infile);
		int                   appendHumdrumPointer(HumdrumFile* infile);

   protected:
      vector<HumdrumFile*>  m_data;

      void                  appendHumdrumFileContent(const std::string& filename, 
                                               std::stringstream& inbuffer);
};


// END_MERGE

} // end namespace hum

#endif /* _HUMDRUMFILESET_H_INCLUDED */



