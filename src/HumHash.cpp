//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sun Aug 16 01:35:04 PDT 2015
// Last Modified: Sun Aug 16 01:35:08 PDT 2015
// Filename:      HumHash.cpp
// URL:           https://github.com/craigsapp/minHumdrum/blob/master/src/HumHash.cpp
// Syntax:        C++11
// vim:           ts=3
//
// Description:   Key/value parameters systems for Humdrum tokens, lines, 
//                and files.
//

#include <sstream>
#include <iostream>

#include "HumHash.h"
#include "HumNum.h"

using namespace std;

namespace minHumdrum {

// START_MERGE


//////////////////////////////
//
// HumHash::HumHash --
//

HumHash::HumHash(void) { 
	parameters = NULL;
}



//////////////////////////////
//
// HumHash::~HumHash --
//

HumHash::~HumHash() { 
	if (parameters != NULL) {
		delete parameters;
		parameters = NULL;
	}
}



//////////////////////////////
//
// HumHash::getParameter --
//

string HumHash::getParameter(const string& key) {
	if (parameters == NULL) {
		return "";
	} else {
		vector<string> keys = getKeyList(key);
		if (keys.size() == 1) {
			return getParameter("", "", keys[0]);
		} else if (keys.size() == 2) {
			return getParameter("", keys[0], keys[1]);
		} else {
			return getParameter(keys[0], keys[1], keys[2]);
		}
	}
}


string HumHash::getParameter(const string& ns2, const string& key) {
	if (parameters == NULL) {
		return "";
	} else {
		return getParameter("", ns2, key);
	}
}


string HumHash::getParameter(const string& ns1, const string& ns2, 
		const string& key) {
	if (parameters == NULL) {
		return "";
	} else {
		return (*parameters)[ns1][ns2][key];
	}
}



//////////////////////////////
//
// HumHash::getParameterInt --
//

int HumHash::getParameterInt(const string& key) { 
	if (parameters == NULL) {
		return 0;
	}
	vector<string> keys = getKeyList(key);
	if (keys.size() == 1) {
		return getParameterInt("", "", keys[2]);
	} else if (keys.size() == 2) {
		return getParameterInt(keys[0], keys[1]);
	} else {
		return getParameterInt(keys[0], keys[1], keys[2]);
	}
}


int HumHash::getParameterInt(const string& ns2, const string& key) { 
	if (parameters == NULL) {
		return 0;
	}
	return getParameterInt("", ns2, key);
}


int HumHash::getParameterInt(const string& ns1, const string& ns2,
		const string& key) { 
	if (parameters == NULL) {
		return 0;
	}
	int value;
	try {
		value = stoi((*parameters)[ns1][ns2][key]);
	} catch (invalid_argument& e) {
		value = 0;
	}
	return value;
}



//////////////////////////////
//
// HumHash::getParameterFraction --
//

HumNum HumHash::getParameterFraction(const string& key) { 
	if (parameters == NULL) {
		return 0;
	}
	vector<string> keys = getKeyList(key);
	if (keys.size() == 1) {
		return getParameterInt("", "", keys[2]);
	} else if (keys.size() == 2) {
		return getParameterInt(keys[0], keys[1]);
	} else {
		return getParameterInt(keys[0], keys[1], keys[2]);
	}
}


HumNum HumHash::getParameterFraction(const string& ns2, const string& key) { 
	if (parameters == NULL) {
		return 0;
	}
	return getParameterFraction("", ns2, key);
}


HumNum HumHash::getParameterFraction(const string& ns1, const string& ns2,
		const string& key) { 
	if (!hasParameter(ns1, ns2, key)) {
		return 0;
	}
	string p = (*parameters)[ns1][ns2][key];
	if (p.size() == 0) {
		return 0;
	}
	HumNum value(p);
	return value;
}



//////////////////////////////
//
// HumHash::getParameterFloat --
//

double HumHash::getParameterFloat(const string& key) { 
	if (parameters == NULL) {
		return 0.0;
	}
	vector<string> keys = getKeyList(key);
	if (keys.size() == 1) {
		return getParameterFloat("", "", keys[2]);
	} else if (keys.size() == 2) {
		return getParameterFloat(keys[0], keys[1]);
	} else {
		return getParameterFloat(keys[0], keys[1], keys[2]);
	}
}


double HumHash::getParameterFloat(const string& ns2, const string& key) { 
	if (parameters == NULL) {
		return 0.0;
	}
	return getParameterInt("", ns2, key);
}


double HumHash::getParameterFloat(const string& ns1, const string& ns2,
		const string& key) { 
	if (parameters == NULL) {
		return 0.0;
	}
	int value;
	try {
		value = stod((*parameters)[ns1][ns2][key]);
	} catch (invalid_argument& e) {
		value = 0.0;
	}
	return value;
}





//////////////////////////////
//
// HumHash::getParameterBool --
//

bool HumHash::getParameterBool(const string& key) { 
	vector<string> keys = getKeyList(key);
	if (keys.size() == 1) {
		return getParameterBool("", "", keys[2]);
	} else if (keys.size() == 2) {
		return getParameterBool(keys[0], keys[1]);
	} else {
		return getParameterBool(keys[0], keys[1], keys[2]);
	}
}


bool HumHash::getParameterBool(const string& ns2, const string& key) { 
	return getParameterBool("", ns2, key);
}


bool HumHash::getParameterBool(const string& ns1, const string& ns2,
		const string& key) { 
	if (parameters == NULL) {
		return false;
	}
	if (!hasParameter(ns1, ns2, key)) {
		return false;
	}
	if ((*parameters)[ns1][ns2][key] == "false") {
		return false;
	} else if ((*parameters)[ns1][ns2][key] == "0") {
		return false;
	} else {
		return true;
	}
}



//////////////////////////////
//
// HumHash::setParameter --
//

void HumHash::setParameter(const string& key, const string& value) {
	vector<string> keys = getKeyList(key);
	if (keys.size() == 1) {
		setParameter("", "", keys[0], value);
	} else if (keys.size() == 2) {
		setParameter("", keys[0], keys[1], value);
	} else {
		setParameter(keys[0], keys[1], keys[2], value);
	}
}


void HumHash::setParameter(const string& ns2, const string& key, 
		const string& value) {
		setParameter("", ns2, key, value);
}


void HumHash::setParameter(const string& ns1, const string& ns2, 
		const string& key, const string& value) {
	initializeParameters();
	(*parameters)[ns1][ns2][key] = value;
}



//////////////////////////////
//
// HumHash::hasParameter -- Returns true if the given parameter exists in the
//    map.
//

bool HumHash::hasParameter(const string& key) {
	if (parameters == NULL) {
		return false;
	}
	vector<string> keys = getKeyList(key);
	if (keys.size() == 1) {
		return (*parameters)[""][""].count(keys[0]);
	} else if (keys.size() == 2) {
		return (*parameters)[""][keys[0]].count(keys[1]);
	} else {
		return (*parameters)[keys[0]][keys[1]].count(keys[2]);
	}
}


bool HumHash::hasParameter(const string& ns2, const string& key) {
	if (parameters == NULL) {
		return false;
	}
	return (*parameters)[""][ns2].count(key);
}


bool HumHash::hasParameter(const string& ns1, const string& ns2,
		const string& key) {
	if (parameters == NULL) {
		return false;
	}
	return (*parameters)[ns1][ns2].count(key);
}



//////////////////////////////
//
// HumHash::deleteParameter --
//

void HumHash::deleteParameter(const string& key) { 
	if (parameters == NULL) {
		return;
	}
	vector<string> keys = getKeyList(key);
	if (keys.size() == 1) {
		deleteParameter("", "", keys[0]);
	} else if (keys.size() == 2) {
		deleteParameter("", keys[0], keys[1]);
	} else {
		deleteParameter(keys[0], keys[1], keys[2]);
	}
}


void HumHash::deleteParameter(const string& ns2, const string& key) { 
	if (parameters == NULL) {
		return;
	}
	deleteParameter("", ns2, key);
}


void HumHash::deleteParameter(const string& ns1, const string& ns2, 
		const string& key) { 
	if (parameters == NULL) {
		return;
	}
	(*parameters)[ns1][ns2].erase(key);
}



//////////////////////////////
//
// HumHash::initializeParameters --
//

void HumHash::initializeParameters(void) {
	if (parameters == NULL) {
		parameters = new map<string, map<string, map<string, string> > >;
	}
}



//////////////////////////////
//
// HumHash::getKeyList --
//

vector<string> HumHash::getKeyList(const string& keys) {
	stringstream ss(keys);
	string key;
	vector<string> output;
	while (getline(ss, key, ':')) {
		output.push_back(key);
	}
	return output;
}


// END_MERGE

} // end namespace std;



