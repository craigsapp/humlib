//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sun Aug 16 01:35:04 PDT 2015
// Last Modified: Sun Aug 16 12:58:17 PDT 2015
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
// HumHash::getValue --
//

string HumHash::getValue(const string& key) {
	if (parameters == NULL) {
		return "";
	} else {
		vector<string> keys = getKeyList(key);
		if (keys.size() == 1) {
			return getValue("", "", keys[0]);
		} else if (keys.size() == 2) {
			return getValue("", keys[0], keys[1]);
		} else {
			return getValue(keys[0], keys[1], keys[2]);
		}
	}
}


string HumHash::getValue(const string& ns2, const string& key) {
	if (parameters == NULL) {
		return "";
	} else {
		return getValue("", ns2, key);
	}
}


string HumHash::getValue(const string& ns1, const string& ns2, 
		const string& key) {
	if (parameters == NULL) {
		return "";
	}

	MapNNKV& p = *parameters;
	auto it1 = p.find(ns1);
	if (it1 == p.end()) {
		return "";
	}
	auto it2 = it1->second.find(ns2);
	if (it2 == it1->second.end()) {
		return "";
	}
	auto it3 = it2->second.find(key);
	if (it3 == it2->second.end()) {
		return "";
	}
	return it3->second;
}



//////////////////////////////
//
// HumHash::getValueInt --
//

int HumHash::getValueInt(const string& key) { 
	if (parameters == NULL) {
		return 0;
	}
	vector<string> keys = getKeyList(key);
	if (keys.size() == 1) {
		return getValueInt("", "", keys[2]);
	} else if (keys.size() == 2) {
		return getValueInt(keys[0], keys[1]);
	} else {
		return getValueInt(keys[0], keys[1], keys[2]);
	}
}


int HumHash::getValueInt(const string& ns2, const string& key) { 
	if (parameters == NULL) {
		return 0;
	}
	return getValueInt("", ns2, key);
}


int HumHash::getValueInt(const string& ns1, const string& ns2,
		const string& key) { 
	if (parameters == NULL) {
		return 0;
	}
	string value = getValue(ns1, ns2, key);
 	if (value.find("/") != string::npos) {
		HumNum nvalue(value);
		return  nvalue.getInteger();
	} else {
		int intvalue;
		try {
			intvalue = stoi(value);
		} catch (invalid_argument& e) {
			intvalue = 0;
		}
		return intvalue;
	}
}



//////////////////////////////
//
// HumHash::getValueFraction --
//

HumNum HumHash::getValueFraction(const string& key) { 
	if (parameters == NULL) {
		return 0;
	}
	vector<string> keys = getKeyList(key);
	if (keys.size() == 1) {
		return getValueFraction("", "", keys[0]);
	} else if (keys.size() == 2) {
		return getValueFraction(keys[0], keys[1]);
	} else {
		return getValueFraction(keys[0], keys[1], keys[2]);
	}
}


HumNum HumHash::getValueFraction(const string& ns2, const string& key) { 
	if (parameters == NULL) {
		return 0;
	}
	return getValueFraction("", ns2, key);
}


HumNum HumHash::getValueFraction(const string& ns1, const string& ns2,
		const string& key) { 
	if (!isDefined(ns1, ns2, key)) {
		return 0;
	}
	string value = getValue(ns1, ns2, key);
	HumNum fractionvalue(value);
	return fractionvalue;
}



//////////////////////////////
//
// HumHash::getValueFloat --
//

double HumHash::getValueFloat(const string& key) { 
	if (parameters == NULL) {
		return 0.0;
	}
	vector<string> keys = getKeyList(key);
	if (keys.size() == 1) {
		return getValueFloat("", "", keys[2]);
	} else if (keys.size() == 2) {
		return getValueFloat(keys[0], keys[1]);
	} else {
		return getValueFloat(keys[0], keys[1], keys[2]);
	}
}


double HumHash::getValueFloat(const string& ns2, const string& key) { 
	if (parameters == NULL) {
		return 0.0;
	}
	return getValueInt("", ns2, key);
}


double HumHash::getValueFloat(const string& ns1, const string& ns2,
		const string& key) { 
	if (parameters == NULL) {
		return 0.0;
	}
	string value = getValue(ns1, ns2, key);
 	if (value.find("/") != string::npos) {
		HumNum nvalue(value);
		return nvalue.getFloat();
	} else {
		double floatvalue;
		try {
			floatvalue = stod(value);
		} catch (invalid_argument& e) {
			floatvalue = 0;
		}
		return floatvalue;
	}
}



//////////////////////////////
//
// HumHash::getValueBool --
//

bool HumHash::getValueBool(const string& key) { 
	vector<string> keys = getKeyList(key);
	if (keys.size() == 1) {
		return getValueBool("", "", keys[2]);
	} else if (keys.size() == 2) {
		return getValueBool(keys[0], keys[1]);
	} else {
		return getValueBool(keys[0], keys[1], keys[2]);
	}
}


bool HumHash::getValueBool(const string& ns2, const string& key) { 
	return getValueBool("", ns2, key);
}


bool HumHash::getValueBool(const string& ns1, const string& ns2,
		const string& key) { 
	if (parameters == NULL) {
		return false;
	}
	if (!isDefined(ns1, ns2, key)) {
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
// HumHash::setValue --
//

void HumHash::setValue(const string& key, const string& value) {
	vector<string> keys = getKeyList(key);
	if (keys.size() == 1) {
		setValue("", "", keys[0], value);
	} else if (keys.size() == 2) {
		setValue("", keys[0], keys[1], value);
	} else {
		setValue(keys[0], keys[1], keys[2], value);
	}
}


void HumHash::setValue(const string& ns2, const string& key, 
		const string& value) {
		setValue("", ns2, key, value);
}


void HumHash::setValue(const string& ns1, const string& ns2, 
		const string& key, const string& value) {
	initializeParameters();
	(*parameters)[ns1][ns2][key] = value;
}



void HumHash::setValue(const string& key, int value) {
	vector<string> keys = getKeyList(key);
	if (keys.size() == 1) {
		setValue("", "", keys[0], value);
	} else if (keys.size() == 2) {
		setValue("", keys[0], keys[1], value);
	} else {
		setValue(keys[0], keys[1], keys[2], value);
	}
}


void HumHash::setValue(const string& ns2, const string& key, int value) {
		setValue("", ns2, key, value);
}


void HumHash::setValue(const string& ns1, const string& ns2, 
		const string& key, int value) {
	initializeParameters();
	stringstream ss(value);
	(*parameters)[ns1][ns2][key] = ss.str();
}


void HumHash::setValue(const string& key, HumNum value) {
	vector<string> keys = getKeyList(key);
	if (keys.size() == 1) {
		setValue("", "", keys[0], value);
	} else if (keys.size() == 2) {
		setValue("", keys[0], keys[1], value);
	} else {
		setValue(keys[0], keys[1], keys[2], value);
	}
}


void HumHash::setValue(const string& ns2, const string& key, HumNum value) {
		setValue("", ns2, key, value);
}


void HumHash::setValue(const string& ns1, const string& ns2, 
		const string& key, HumNum value) {
	initializeParameters();
	stringstream ss;
	ss << value;
	(*parameters)[ns1][ns2][key] = ss.str();
}



void HumHash::setValue(const string& key, double value) {
	vector<string> keys = getKeyList(key);
	if (keys.size() == 1) {
		setValue("", "", keys[0], value);
	} else if (keys.size() == 2) {
		setValue("", keys[0], keys[1], value);
	} else {
		setValue(keys[0], keys[1], keys[2], value);
	}
}


void HumHash::setValue(const string& ns2, const string& key, double value) {
		setValue("", ns2, key, value);
}


void HumHash::setValue(const string& ns1, const string& ns2, 
		const string& key, double value) {
	initializeParameters();
	stringstream ss;
	ss << value;
	(*parameters)[ns1][ns2][key] = ss.str();
}



//////////////////////////////
//
// HumHash::getKeys --
//

vector<string> HumHash::getKeys(const string& ns1, const string& ns2) {
	vector<string> output;
	if (parameters == NULL) {
		return output;
	}
	for (auto& it : (*parameters)[ns1][ns2]) {
		output.push_back(it.first);
	}
	return output;
}



//////////////////////////////
//
// HumHash::isDefined -- Returns true if the given parameter exists in the
//    map.
//

bool HumHash::isDefined(const string& key) {
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


bool HumHash::isDefined(const string& ns2, const string& key) {
	if (parameters == NULL) {
		return false;
	}
	return (*parameters)[""][ns2].count(key);
}


bool HumHash::isDefined(const string& ns1, const string& ns2,
		const string& key) {
	if (parameters == NULL) {
		return false;
	}
	return (*parameters)[ns1][ns2].count(key);
}



//////////////////////////////
//
// HumHash::deleteValue --
//

void HumHash::deleteValue(const string& key) { 
	if (parameters == NULL) {
		return;
	}
	vector<string> keys = getKeyList(key);
	if (keys.size() == 1) {
		deleteValue("", "", keys[0]);
	} else if (keys.size() == 2) {
		deleteValue("", keys[0], keys[1]);
	} else {
		deleteValue(keys[0], keys[1], keys[2]);
	}
}


void HumHash::deleteValue(const string& ns2, const string& key) { 
	if (parameters == NULL) {
		return;
	}
	deleteValue("", ns2, key);
}


void HumHash::deleteValue(const string& ns1, const string& ns2, 
		const string& key) { 
	if (parameters == NULL) {
		return;
	}
	(*parameters)[ns1][ns2].erase(key);

	MapNNKV& p = *parameters;
	auto it1 = p.find(ns1);
	if (it1 == p.end()) {
		return;
	}
	auto it2 = it1->second.find(ns2);
	if (it2 == it1->second.end()) {
		return;
	}
	auto it3 = it2->second.find(key);
	if (it3 == it2->second.end()) {
		return;
	}
	it2->second.erase(key);
}



//////////////////////////////
//
// HumHash::initializeParameters --
//

void HumHash::initializeParameters(void) {
	if (parameters == NULL) {
		parameters = new MapNNKV;
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
	if (output.size() == 0) {
		output.push_back(keys);
	}
	return output;
}


// END_MERGE

} // end namespace std;



