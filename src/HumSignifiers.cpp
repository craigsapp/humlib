//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sun Jun 10 14:36:51 PDT 2018
// Last Modified: Sun Jun 10 14:36:54 PDT 2018
// Filename:      HumSignifiers.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/HumSignifiers.cpp
// Syntax:        C++11; humlib
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   The collection of signifiers in a Humdrum file.
//

#include "HumSignifiers.h"

namespace hum {

// START_MERGE


//////////////////////////////
//
// HumSignifiers::HumSignifier --
//

HumSignifiers::HumSignifiers(void) {
	// do nothing
}



//////////////////////////////
//
// HumSignifiers::~HumSignifier --
//

HumSignifiers::~HumSignifiers() {
	clear();
}



//////////////////////////////
//
// HumSignifiers::clear --
//

void HumSignifiers::clear(void) {
	m_kernLinkIndex = -1;

	for (int i=0; i<(int)m_signifiers.size(); i++) {
		delete m_signifiers[i];
		m_signifiers[i] = NULL;
	}
	m_signifiers.clear();
}



//////////////////////////////
//
// HumSignifiers::addSignifier --
//

bool HumSignifiers::addSignifier(const std::string& rdfline) {
	HumSignifier *humsig = new HumSignifier;
	if (!humsig->parseSignifier(rdfline)) {
		// ignore malformed RDF reference record.
		return false;
	}
	m_signifiers.push_back(humsig);

	if (m_signifiers.back()->isKernLink()) {
		m_kernLinkIndex = (int)m_signifiers.size() - 1;
	} else if (m_signifiers.back()->isKernAbove()) {
		m_kernAboveIndex = (int)m_signifiers.size() - 1;
	} else if (m_signifiers.back()->isKernBelow()) {
		m_kernBelowIndex = (int)m_signifiers.size() - 1;
	}
	return true;
}



//////////////////////////////
//
// HumSignifiers::hasKernLinkSignifier --
//

bool HumSignifiers::hasKernLinkSignifier(void) {
	return (m_kernLinkIndex >= 0);
}



//////////////////////////////
//
// HumSignifiers::getKernLinkSignifier --
//

std::string HumSignifiers::getKernLinkSignifier(void) {
	if (m_kernLinkIndex < 0) {
		return "";
	}
	return m_signifiers[m_kernLinkIndex]->getSignifier();
}



//////////////////////////////
//
// HumSignifiers::hasKernAboveSignifier --
//

bool HumSignifiers::hasKernAboveSignifier(void) {
	return (m_kernAboveIndex >= 0);
}



//////////////////////////////
//
// HumSignifiers::getKernAboveSignifier --
//

std::string HumSignifiers::getKernAboveSignifier(void) {
	if (m_kernAboveIndex < 0) {
		return "";
	}
	return m_signifiers[m_kernAboveIndex]->getSignifier();
}



//////////////////////////////
//
// HumSignifiers::hasKernBelowSignifier --
//

bool HumSignifiers::hasKernBelowSignifier(void) {
	return (m_kernBelowIndex >= 0);
}



//////////////////////////////
//
// HumSignifiers::getKernBelowSignifier --
//

std::string HumSignifiers::getKernBelowSignifier(void) {
	if (m_kernBelowIndex < 0) {
		return "";
	}
	return m_signifiers[m_kernBelowIndex]->getSignifier();
}



//////////////////////////////
//
// HumSignifiers::getSignifierCount --
//

int HumSignifiers::getSignifierCount(void) {
	return (int)m_signifiers.size();
}



//////////////////////////////
//
// HumSignifiers::getSignifier --
//

HumSignifier* HumSignifiers::getSignifier(int index) {
	if (index < 0) {
		return NULL;
	}
	if (index >= (int)m_signifiers.size()) {
		return NULL;
	}
	return m_signifiers.at(index);
}


// END_MERGE

} // end namespace hum




