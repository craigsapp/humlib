//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sun Jun 10 07:10:47 PDT 2018
// Last Modified: Sun Jun 10 07:10:49 PDT 2018
// Filename:      HumSignifier.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/HumSignifier.cpp
// Syntax:        C++11; humlib
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   A signifier that extends a Humdrum representation.
//

#include "HumSignifier.h"
#include "HumRegex.h"

#include <iostream>
#include <map>

using namespace std;

namespace hum {

// START_MERGE


//////////////////////////////
//
// HumSignifier::HumSignifier --
//

HumSignifier::HumSignifier (void) {
	// do nothing
}



HumSignifier::HumSignifier (const string& rdfline) {
	parseSignifier(rdfline);
}



//////////////////////////////
//
// HumSignifier::~HumSignifier --
//

HumSignifier::~HumSignifier() {
	clear();
}



//////////////////////////////
//
// HumSignifier::clear -- Clear contents of object.
//

void HumSignifier::clear(void) {
	m_exinterp.clear();
	m_signifier.clear();
	m_definition.clear();
	m_parameters.clear();
	m_sigtype = signifier_type::signifier_unknown;
}



//////////////////////////////
//
// HumSignifier::parseSignifier --
//
bool HumSignifier::parseSignifier(const string& rdfline) {
	clear();
	HumRegex hre;
	if (!hre.search(rdfline, "!!!RDF(\\*\\*[^\\s:]+)\\s*:\\s*(.*)\\s*$")) {
		return false;
	}
	m_exinterp   = hre.getMatch(1);
	string value = hre.getMatch(2);

	if (!hre.search(value, "\\s*([^\\s=]+)\\s*=\\s*(.*)\\s*$")) {
		clear();
		return false;
	}
	m_signifier  = hre.getMatch(1);
	m_definition = hre.getMatch(2);

	// identify signifier category

	if (m_exinterp == "**kern") {
		if (m_definition.find("link") != std::string::npos) {
			m_sigtype = signifier_type::signifier_link;
		} else if (m_definition.find("above") != std::string::npos) {
			m_sigtype = signifier_type::signifier_above;
		} else if (m_definition.find("below") != std::string::npos) {
			m_sigtype = signifier_type::signifier_below;
		}
	}

	// parse parameters here

	return true;
}



//////////////////////////////
//
// HumSignifier::getSignifier --
//

std::string HumSignifier::getSignifier(void) {
	return m_signifier;
}



//////////////////////////////
//
// HumSignifier::getDefinition --
//

std::string HumSignifier::getDefinition(void) {
	return m_definition;
}



//////////////////////////////
//
// HumSignifier::getParameter --
//

std::string HumSignifier::getParameter(const std::string& key) {
	auto value = m_parameters.find(key);
	if (value == m_parameters.end()) {
		return "";
	} else {
		return value->second;
	}
}



//////////////////////////////
//
// HumSignifier::isKernLink -- Is a linking signifier
//

bool HumSignifier::isKernLink(void) {
	return (m_sigtype == signifier_type::signifier_link);
}



//////////////////////////////
//
// HumSignifier::isKernAbove -- Is an above signifier.
//

bool HumSignifier::isKernAbove(void) {
	return (m_sigtype == signifier_type::signifier_above);
}



//////////////////////////////
//
// HumSignifier::isKernBelow -- Is a below signifier.
//

bool HumSignifier::isKernBelow(void) {
	return (m_sigtype == signifier_type::signifier_below);
}


// END_MERGE

} // end namespace hum



