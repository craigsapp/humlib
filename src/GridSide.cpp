//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sun Oct 16 16:08:05 PDT 2016
// Last Modified: Sun Oct 16 16:08:08 PDT 2016
// Filename:      GridSide.cpp
// URL:           https://github.com/craigsapp/hum2ly/blob/master/src/GridSide.cpp
// Syntax:        C++11
// vim:           ts=3 noexpandtab
//
// Description:   GridSide is an intermediate container for converting from
//                MusicXML syntax into Humdrum syntax.
//
//

#include "HumGrid.h"

using namespace std;

namespace hum {

// START_MERGE

//////////////////////////////
//
// GridSide::GridSide -- Constructor.
//

GridSide::GridSide(void) {
	// do nothing
}



//////////////////////////////
//
// GridSide::~GridSide -- Deconstructor.
//

GridSide::~GridSide(void) {

	for (int i=0; i<(int)m_verses.size(); i++) {
		if (m_verses[i]) {
			delete m_verses[i];
			m_verses[i] = NULL;
		}
	}
	m_verses.resize(0);

	if (m_dynamics) {
		delete m_dynamics;
		m_dynamics = NULL;
	}

	if (m_harmony) {
		delete m_harmony;
		m_harmony = NULL;
	}
}



//////////////////////////////
//
// GridSide::setVerse --
//

void GridSide::setVerse(int index, HTp token) {
	if (token == NULL) {
		// null tokens are written in the transfer process when responsibility
		// for deleting the pointer is given to another object (HumdrumFile class).
	}
   if (index == (int)m_verses.size()) {
		// Append to the end of the verse list.
		m_verses.push_back(token);
	} else if (index < (int)m_verses.size()) {
		// Insert in a slot which might already have a verse token
		if ((token != NULL) && (m_verses.at(index) != NULL)) {
			// don't delete a previous non-NULL token if a NULL
			// token is being stored, as it is assumed that the
			// token has been transferred to a HumdrumFile object.
			delete m_verses[index];
		}
		m_verses[index] = token;
	} else {
		// Add more than one verse spot and insert verse:
		int oldsize = (int)m_verses.size();
		int newsize = index + 1;
		m_verses.resize(newsize);
		for (int i=oldsize; i<newsize; i++) {
			m_verses.at(i) = NULL;
		}
		m_verses.at(index) = token;
	}
}


void GridSide::setVerse(int index, const string& token) {
	HTp newtoken = new HumdrumToken(token);
	setVerse(index, newtoken);
}


//////////////////////////////
//
// GridSide::getVerse --
//

HTp GridSide::getVerse(int index) {
	if (index < 0 || index >= getVerseCount()) {
		return NULL;
	}
	return m_verses[index];
}



//////////////////////////////
//
// GridSide::getVerseCount --
//

int GridSide::getVerseCount(void) {
 	return (int)m_verses.size();
}



//////////////////////////////
//
// GridSide::getHarmonyCount --
//

int GridSide::getHarmonyCount(void) {
	if (m_harmony == NULL) {
		return 0;
	} else {
		return 1;
	}
}



//////////////////////////////
//
// GridSide::setHarmony --
//

void GridSide::setHarmony(HTp token) {
	if (m_harmony) {
		delete m_harmony;
		m_harmony = NULL;
	}
	m_harmony = token;
}



//////////////////////////////
//
// GridSide::setDynamics --
//

void GridSide::setDynamics(HTp token) {
	if (m_dynamics) {
		delete m_dynamics;
		m_dynamics = NULL;
	}
	m_dynamics = token;
}


void GridSide::setDynamics(const string& token) {
	HTp newtoken = new HumdrumToken(token);
	setDynamics(newtoken);
}



///////////////////////////
//
// GridSide::detachHarmony --
//

void GridSide::detachHarmony(void) {
	m_harmony = NULL;
}



///////////////////////////
//
// GridSide::detachDynamics --
//

void GridSide::detachDynamics(void) {
	m_dynamics = NULL;
}



//////////////////////////////
//
// GridSide::getHarmony --
//

HTp GridSide::getHarmony(void) {
	return m_harmony;
}



//////////////////////////////
//
// GridSide::getDynamics --
//

HTp GridSide::getDynamics(void) {
	return m_dynamics;
}



//////////////////////////////
//
// GridSide::getDynamicsCount --
//

int GridSide::getDynamicsCount(void) {
	if (m_dynamics == NULL) {
		return 0;
	} else {
		return 1;
	}
}



//////////////////////////////
//
// operator<< --
//

ostream& operator<<(ostream& output, GridSide* side) {
	output << " [";

	if (side->getVerseCount() > 0) {
		output << " verse:";
	}
	for (int i=0; i<(int)side->getVerseCount(); i++) {
		output << side->getVerse(i);
		if (i < (int)side->getVerseCount() - 1) {
			output << "; ";
		}

	}

	if (side->getDynamicsCount() > 0) {
		output << "dyn:" << side->getDynamics();
	}

	if (side->getHarmonyCount() > 0) {
		output << "harm:" << side->getHarmony();
	}

	output << "] ";
	return output;
}



// END_MERGE

} // end namespace hum



