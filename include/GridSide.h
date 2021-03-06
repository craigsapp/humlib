//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sun Oct 16 16:08:05 PDT 2016
// Last Modified: Sun Oct 16 16:08:08 PDT 2016
// Filename:      HumGrid.h
// URL:           https://github.com/craigsapp/hum2ly/blob/master/include/HumGrid.h
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   HumGrid is an intermediate container for converting from
//                MusicXML syntax into Humdrum syntax.
//
//

#ifndef _GRIDSIDE_H
#define _GRIDSIDE_H

#include <iostream>
#include <string>
#include <vector>

#include "HumdrumToken.h"

namespace hum {

// START_MERGE

class GridSide {
	public:
		GridSide(void);
		~GridSide();

		int   getVerseCount     (void);
		HTp   getVerse          (int index);
		void  setVerse          (int index, HTp token);
		void  setVerse          (int index, const std::string& token);

		int   getXmlidCount     (void);
		void  setXmlid          (HTp token);
		void  setXmlid          (const std::string& token);
		void  detachXmlid       (void);
		HTp   getXmlid          (void);

		int   getHarmonyCount   (void);
		void  setHarmony        (HTp token);
		void  setHarmony        (const std::string& token);
		void  detachHarmony     (void);
		HTp   getHarmony        (void);

		int   getDynamicsCount  (void);
		void  setDynamics       (HTp token);
		void  setDynamics       (const std::string& token);
		void  detachDynamics    (void);
		HTp   getDynamics       (void);

		int   getFiguredBassCount (void);
		void  setFiguredBass      (HTp token);
		void  setFiguredBass      (const std::string& token);
		void  detachFiguredBass   (void);
		HTp   getFiguredBass      (void);

	private:
		HumdrumToken* m_xmlid        = NULL;
		std::vector<HumdrumToken*> m_verses;
		HumdrumToken* m_dynamics     = NULL;
		HumdrumToken* m_figured_bass = NULL;
		HumdrumToken* m_harmony      = NULL;
};

std::ostream& operator<<(std::ostream& output, GridSide* side);

// END_MERGE

} // end namespace hum

#endif /* _HUMGRID_H */



