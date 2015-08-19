//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Aug  8 12:24:49 PDT 2015
// Last Modified: Tue Aug 18 02:32:51 PDT 2015
// Filename:      /include/minhumdrum.cpp
// URL:           https://github.com/craigsapp/minHumdrum/blob/master/src/minhumdrum.cpp
// Syntax:        C++11
// vim:           ts=3
//
// Description:   Source file for minHumdrum library.
//
/*
Copyright (c) 2015 Craig Stuart Sapp
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   and the following disclaimer in the documentation and/or other materials
   provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#include "minhumdrum.h"

namespace minHumdrum {


//////////////////////////////
//
// HumAddress::HumAddress -- HumAddress constructor.
//

HumAddress::HumAddress(void) {
	track      = -1;
	subtrack   = -1;
	fieldindex = -1;
	owner      = NULL;
}



//////////////////////////////
//
// HumAddress::~HumAddress -- HumAddress deconstructor.
//

HumAddress::~HumAddress() {
	track      = -1;
	subtrack   = -1;
	fieldindex = -1;
	owner      = NULL;
}



//////////////////////////////
//
// HumAddress::getLineIndex -- Returns the line index in the owning HumdrumFile
//    for the token associated with the address.  Returns -1 if not owned by a
//    HumdrumLine (or line assignments have not been made for tokens in the
//    file).
//

int  HumAddress::getLineIndex(void) const {
	if (owner == NULL) {
		return -1;
	} else {
		return owner->getLineIndex();
	}
}



//////////////////////////////
//
// HumAddress::getLineNumber --  Similar to getLineIndex() but adds one.
//

int HumAddress::getLineNumber(void) const {
	return getLineIndex() + 1;
}



//////////////////////////////
//
// HumAddress::getFieldIndex -- Returns the field index on the line of the
//     token associated with the address.
//

int HumAddress::getFieldIndex(void) const {
	return fieldindex;
}



//////////////////////////////
//
// HumAddress::getDataType -- Return the exclusive interpretation string of the
//    token associated with the address.
//

const HumdrumToken& HumAddress::getDataType(void) const {
	static HumdrumToken null("");
	if (owner == NULL) {
		return null;
	}
	HumdrumToken* tok = owner->getTrackStart(getTrack());
	return *tok;
}



//////////////////////////////
//
// HumAddress::getSpineInfo -- Return the spine information for the token
//     associated with the address.  Examples: "1" the token is in the first
//     (left-most) spine, and there are no active subspines for the spine.
//     "(1)a"/"(1)b" are the spine descriptions of the two subspines after
//     a split manipulator (*^).  "((1)a)b" is the second subsubspine of the
//     first sub-spine for spine 1.
//
//

const string& HumAddress::getSpineInfo(void) const {
	return spining;
}



//////////////////////////////
//
// HumAddress::getTrack -- The track number of the given spine.  This is the
//   first number in the spine info string.  The track number is the same
//   as a spine number.
//

int HumAddress::getTrack(void) const {
	return track;
}



//////////////////////////////
//
// HumAddress::getSubTrack -- The subtrack number of the given spine.  This
//   functions in a similar manner to layer numbers in MEI data.  The first
//   sub-spine of a spine is always subtrack 1, regardless of whether or not
//   an exchange manipulator (*x) was used to switch the left-to-right ordering
//   of the spines in the file.  All subspines regardless of their splitting
//   origin are given sequential subtrack numbers.  For example if the spine
//   info is "(1)a"/"((1)b)a"/"((1)b)b" -- the spine is split, then the second
//   sub-spine only is split--then the subspines are labeled as subtracks "1",
//   "2", "3" respectively.  When a track has only one sub-spine (i.e., it has
//   been split), the subtrack value will be "0".
//

int HumAddress::getSubtrack(void) const {
	return subtrack;
}



//////////////////////////////
//
// HumAddress::getTrackString --  Return the track and subtrack as a string.
//      The returned string will have the track number if the sub-spine value
//      is zero.  The optional separator parameter is used to separate the
//      track number from the subtrack number.
//        default value: separator = "."
//

string HumAddress::getTrackString(string separator) const {
	string output;
	int thetrack    = getTrack();
	int thesubtrack = getSubtrack();
	output += to_string(thetrack);
	if (thesubtrack > 0) {
		output += separator + to_string(thesubtrack);
	}
	return output;
}



//////////////////////////////
//
// HumAddress::setOwner -- Stores a pointer to the HumrumLine on which
//   the token associated with this address belongs.  When not owned by
//   a HumdrumLine, the parameter's value should be NULL.
//

void HumAddress::setOwner(HumdrumLine* aLine) {
	owner = aLine;
}



//////////////////////////////
//
// HumAddress::getLine -- return the HumdrumLine which owns the token
//    associated with this address.  Returns NULL if it does not belong
//    to a HumdrumLine object.
//

HumdrumLine* HumAddress::getLine(void) const {
	return owner;
}



//////////////////////////////
//
// HumAddress::hasOwner -- Returns true if a HumdrumLine owns the token
//    associated with the address.
//

bool HumAddress::hasOwner(void) const {
	return owner == NULL ? 0 : 1;
}



//////////////////////////////
//
// HumAddress::setFieldIndex -- Set the field index of associated token
//   in the HumdrumLine owner.  If the token is now owned by a HumdrumLine,
//   then the input parameter should be -1.
//

void HumAddress::setFieldIndex(int index) {
	fieldindex = index;
}



//////////////////////////////
//
// HumAddress::setSpineInfo -- Set the spine description of the associated
//     token.  For example "2" for the second spine (from the left), or
//     "((2)a)b" for a sub-spine created as the left sub-spine of the main
//     spine and then as the right sub-spine of that sub-spine.  This function
//     is used by the HumdrumFileStructure class.
//

void HumAddress::setSpineInfo(const string& spineinfo) {
	spining = spineinfo;
}



//////////////////////////////
//
// HumAddress::setTrack -- Set the track number of the associated token.
//   This should always be the first number in the spine information string,
//   or -1 if the spine info is empty.  Tracks are limited to an arbitrary
//   count of 1000 (could be increased in the future if needed).  This function
//   is used by the HumdrumFileStructure class.
//

void HumAddress::setTrack(int aTrack, int aSubtrack) {
	setTrack(aTrack);
	setSubtrack(aTrack);
}


void HumAddress::setTrack(int aTrack) {
	if (aTrack < 0) {
		aTrack = -1;
	}
	if (aTrack > 1000) {
		aTrack = 1000;
	}
	track = aTrack;
}



//////////////////////////////
//
// HumAddress::setSubtrack -- Set the subtrack of the spine.
//   If the token is the only one active for a spine, the subtrack should
//   be set to zero.  If there are more than one subtracks for the spine, this
//   is the one-offset index of the spine (be careful if a sub-spine column
//   is exchanged with another spine other than the one from which it was
//   created.  In this case the subtrack number is not useful to calculate
//   the field index of other subtracks for the given track.
//   This function is used by the HumdrumFileStructure class.
//

void HumAddress::setSubtrack(int aSubtrack) {
	if (aSubtrack < 0) {
		aSubtrack = -1;
	}
	if (aSubtrack > 1000) {
		aSubtrack = 1000;
	}
	subtrack = aSubtrack;
}



//////////////////////////////
//
// HumHash::HumHash -- HumHash constructor.  The data storage is empty
//    until the first parameter in the Hash is set.
//

HumHash::HumHash(void) {
	parameters = NULL;
}



//////////////////////////////
//
// HumHash::~HumHash -- The HumHash deconstructor, which removed any
//    allocated storage before the object dies.
//

HumHash::~HumHash() {
	if (parameters != NULL) {
		delete parameters;
		parameters = NULL;
	}
}



//////////////////////////////
//
// HumHash::getValue -- Returns the value specified by the given key.
//    If there is no colon in the key then return the value for the key
//    in the default namespaces (NS1="" and NS2="").  If there is one colon,
//    then the two pieces of the string as NS2 and the key, with NS1="".
//    If there are two colons, then that specified the complete namespaces/key
//    address of the value.  The namespaces and key can be specified as
//    separate parameters in a similar manner to the single-string version.
//    But in these cases colon concatenation of the namespaces and/or key
//    are not allowed.
//

string HumHash::getValue(const string& key) const {
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


string HumHash::getValue(const string& ns2, const string& key) const {
	if (parameters == NULL) {
		return "";
	} else {
		return getValue("", ns2, key);
	}
}


string HumHash::getValue(const string& ns1, const string& ns2,
		const string& key) const {
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
// HumHash::getValueInt -- Return the value as an integer.  The value must
//   start with a number and have no text before it; otherwise the
//   returned value will be "0".  The HumHash class is aware of fractional
//   values, so the integer form of the fraction will be returned.  For
//   example if the value is "12/7", then the return value will be "1"
//   since the integer part of 12/7 is 1 with a remainder of 5/7ths
//   which will be chopped off.
//

int HumHash::getValueInt(const string& key) const {
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


int HumHash::getValueInt(const string& ns2, const string& key) const {
	if (parameters == NULL) {
		return 0;
	}
	return getValueInt("", ns2, key);
}


int HumHash::getValueInt(const string& ns1, const string& ns2,
		const string& key) const {
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
// HumHash::getValueFraction -- Return the value as a HumNum fraction.
//    If the string represents an integer, it will be preserved in the
//    HumNum return value.  For floating-point values, the fractional
//    part will be ignored.  For example "1.52" will be returned as "1".
//

HumNum HumHash::getValueFraction(const string& key) const {
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


HumNum HumHash::getValueFraction(const string& ns2, const string& key) const {
	if (parameters == NULL) {
		return 0;
	}
	return getValueFraction("", ns2, key);
}


HumNum HumHash::getValueFraction(const string& ns1, const string& ns2,
		const string& key) const {
	if (!isDefined(ns1, ns2, key)) {
		return 0;
	}
	string value = getValue(ns1, ns2, key);
	HumNum fractionvalue(value);
	return fractionvalue;
}



//////////////////////////////
//
// HumHash::getValueFloat --  Return the floating-point interpretation
//   of the value string.  If the string can represent a HumNum fraction,
//   then convert the HumNum interpretation as a floating point number.
//   For example "1.25" and "5/4" will both return 1.25.  The value
//   cannot contain a slash unless it is part of the first fraction
//   on in the value string (this may be changed when regular expressions
//   are used to implement this function).
//

double HumHash::getValueFloat(const string& key) const {
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


double HumHash::getValueFloat(const string& ns2, const string& key) const {
	if (parameters == NULL) {
		return 0.0;
	}
	return getValueInt("", ns2, key);
}


double HumHash::getValueFloat(const string& ns1, const string& ns2,
		const string& key) const {
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
// HumHash::getValueBool -- Return true or false based on the
//   value.  If the value is "0" or false, then the function
//   will return false.  If the value is anything else, then
//   true will be returned.  If the parameter is not defined
//   in the HumHash, then false will also be defined.
//   See also hasParameter() if you do not like this last
//   behavior.
//

bool HumHash::getValueBool(const string& key) const {
	vector<string> keys = getKeyList(key);
	if (keys.size() == 1) {
		return getValueBool("", "", keys[2]);
	} else if (keys.size() == 2) {
		return getValueBool(keys[0], keys[1]);
	} else {
		return getValueBool(keys[0], keys[1], keys[2]);
	}
}


bool HumHash::getValueBool(const string& ns2, const string& key) const {
	return getValueBool("", ns2, key);
}


bool HumHash::getValueBool(const string& ns1, const string& ns2,
		const string& key) const {
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
// HumHash::setValue -- Set the parameter to the given value,
//     over-writing any previous value for the parameter.  The
//     value is any arbitrary string, but preferably does not
//     include tabs or colons.  If a colon is needed, then specify
//     as "&colon;" without the quotes.  Values such as integers
//     fractions and floats can be specified, and these wil be converted
//     internally into strings (use getValueInt() or getValueFloat()
//     to recover the original type).
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
// HumHash::getKeys -- Return a list of keys in a particular namespace
//     combination.  With no parameters, a complete list of all
//     namespaces/keys will be returned.  Giving one parameter will
//     produce a list will give all NS2:key values in the NS1 namespace.
//     If there is a colon in the single parameter version of the function,
//     then this will be interpreted as "NS1", "NS2" version of the parameters
//     described above.
//

vector<string> HumHash::getKeys(const string& ns1, const string& ns2) const {
	vector<string> output;
	if (parameters == NULL) {
		return output;
	}
	for (auto& it : (*parameters)[ns1][ns2]) {
		output.push_back(it.first);
	}
	return output;
}


vector<string> HumHash::getKeys(const string& ns) const {
	vector<string> output;
	if (parameters == NULL) {
		return output;
	}
	int loc = ns.find(":");
	if (loc != string::npos) {
		string ns1 = ns.substr(0, loc);
		string ns2 = ns.substr(loc+1);
		return getKeys(ns1, ns2);
	}

	for (auto& it1 : (*parameters)[ns]) {
		for (auto& it2 : it1.second) {
			output.push_back(it1.first + ":" + it2.first);
		}
	}
	return output;
}


vector<string> HumHash::getKeys(void) const {
	vector<string> output;
	if (parameters == NULL) {
		return output;
	}
	for (auto& it1 : (*parameters)) {
		for (auto& it2 : it1.second) {
			for (auto it3 : it2.second) {
				output.push_back(it1.first + ":" + it2.first + ":" + it3.first);
			}
		}
	}
	return output;
}



//////////////////////////////
//
// HumHash::hasParameters -- Returns true if at least one parameter is defined
//     in the HumHash object (when no arguments are given to the function).
//     When two strings are given as arguments, the function checks to see if
//     the given namespace pair has any keys.  If only one string argument,
//     then check if the given NS1 has any parameters, unless there is a
//     colon in the string which means to check NS1:NS2.
//

bool HumHash::hasParameters(const string& ns1, const string& ns2) const {
	if (parameters == NULL) {
		return false;
	}
	if (parameters->size() == 0) {
		return false;
	}
	auto it1 = parameters->find(ns1);
	if (it1 == parameters->end()) {
		return false;
	}
	auto it2 = (*parameters)[ns1].find(ns2);
	if (it2 == (*parameters)[ns1].end()) {
		return false;
	} else {
		return true;
	}
}


bool HumHash::hasParameters(const string& ns) const {
	if (parameters == NULL) {
		return false;
	}
	int loc = ns.find(":");
	if (loc != string::npos) {
		string ns1 = ns.substr(0, loc);
		string ns2 = ns.substr(loc+1);
		return hasParameters(ns1, ns2);
	}

	auto it = parameters->find(ns);
	if (it == parameters->end()) {
		return false;
	} else {
		return true;
	}
}


bool HumHash::hasParameters(void) const {
	if (parameters == NULL) {
		return false;
	}
	if (parameters->size() == 0) {
		return false;
	}
	for (auto& it1 : *parameters) {
		for (auto& it2 : it1.second) {
			if (it2.second.size() == 0) {
				continue;
			} else {
				return true;
			}
		}
	}
	return false;
}



//////////////////////////////
//
// HumHash::getParameterCount -- Return a count of the parameters which are
//     stored in the HumHash.  If no arguments, then count all value in
//     all namespaces.  If two arguments, then return the count for a
//     specific NS1:NS2 namespace.  If one argument, then return the
//     parameters in NS1, but if there is a colon in the string,
//     return the parameters in NS1:NS2.
//
//

int HumHash::getParameterCount(const string& ns1, const string& ns2) const {
	if (parameters == NULL) {
		return 0;
	}
	if (parameters->size() == 0) {
		return 0;
	}
	auto it1 = parameters->find(ns1);
	if (it1 == parameters->end()) {
		return 0;
	}
	auto it2 = it1->second.find(ns2);
	if (it2 == it1->second.end()) {
		return 0;
	}
	return it2->second.size();
}


int HumHash::getParameterCount(const string& ns) const {
	if (parameters == NULL) {
		return false;
	}
	int loc = ns.find(":");
	if (loc != string::npos) {
		string ns1 = ns.substr(0, loc);
		string ns2 = ns.substr(loc+1);
		return getParameterCount(ns1, ns2);
	}

	auto it1 = parameters->find(ns);
	if (it1 == parameters->end()) {
		return false;
	}
	int sum = 0;
	for (auto& it2 : it1->second) {
		sum += it2.second.size();
	}
	return sum;
}


int HumHash::getParameterCount(void) const {
	if (parameters == NULL) {
		return 0;
	}
	if (parameters->size() == 0) {
		return 0;
	}
	int sum = 0;
	for (auto& it1 : (*parameters)) {
		for (auto& it2 : it1.second) {
			sum += it2.second.size();
		}
	}
	return sum;
}



//////////////////////////////
//
// HumHash::isDefined -- Returns true if the given parameter exists in the
//    map.   Format of the input string:   NS1:NS2:key or "":NS2:key for the
//    two argument version of the function.  OR "":"":key if no colons in
//    single string argument version.
//

bool HumHash::isDefined(const string& key) const {
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


bool HumHash::isDefined(const string& ns2, const string& key) const {
	if (parameters == NULL) {
		return false;
	}
	return (*parameters)[""][ns2].count(key);
}


bool HumHash::isDefined(const string& ns1, const string& ns2,
		const string& key) const {
	if (parameters == NULL) {
		return false;
	}
	return (*parameters)[ns1][ns2].count(key);
}



//////////////////////////////
//
// HumHash::deleteValue -- Delete the given parameter key from the HumHash
//   object.  Three string version is N1,NS2,key; two string version is
//   "",NS2,key; and one argument version is "","",key.
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
// HumHash::initializeParameters -- Create the map structure if it does not
//     already exist.
//

void HumHash::initializeParameters(void) {
	if (parameters == NULL) {
		parameters = new MapNNKV;
	}
}



//////////////////////////////
//
// HumHash::getKeyList -- Return a list of colon separated values from
//      the string.
//

vector<string> HumHash::getKeyList(const string& keys) const {
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



//////////////////////////////
//
// HumHash::setPrefix: initial string to print when using
//   operator<<.  This is used for including the "!" for local
//   comments or "!!" for global comments.   The prefix will
//   remain the same until it is changed.  The default prefix
//   of the object it the empty string.
//

void HumHash::setPrefix(const string& value) {
	prefix = value;

}



//////////////////////////////
//
// operator<< -- Print a list of the parameters in a HumHash object.
//

ostream& operator<<(ostream& out, const HumHash& hash) {
	if (hash.parameters == NULL) {
		return out;
	}
	if (hash.parameters->size() == 0) {
		return out;
	}

	string cleaned;

	for (auto& it1 : *(hash.parameters)) {
		if (it1.second.size() == 0) {
			continue;
		}
		for (auto& it2 : it1.second) {
			if (it2.second.size() == 0) {
				continue;
			}
			out << hash.prefix;
			out << it1.first << ":" << it2.first;
			for (auto& it3 : it2.second) {
				out << ":" << it3.first;
				if (it3.second != "true") {
					cleaned = it3.second;
					Convert::replaceOccurrences(cleaned, ":", "&colon;");
					out << "=" << cleaned;
				}
			}
			out << endl;
		}
	}

	return out;
}




//////////////////////////////
//
// HumNum::HumNum --
//

HumNum::HumNum(void){
	setValue(0);
}


HumNum::HumNum(int value){
	setValue(value);
}


HumNum::HumNum(int numerator, int denominator){
	setValue(numerator, denominator);
}


HumNum::HumNum(const string& ratstring) {
	setValue(ratstring);
}


HumNum::HumNum(const HumNum& rat) {
	*this = rat;
}



//////////////////////////////
//
// HumNum::~HumNum --
//

HumNum::~HumNum() {
	// do nothing
}



//////////////////////////////
//
// HumNum::isNegative -- Returns true if value is negative.
//

bool HumNum::isNegative(void) const {
	return isFinite() && (top < 0);
}



//////////////////////////////
//
// HumNum::isPositive -- Returns true if value is positive.
//

bool HumNum::isPositive(void) const {
	return isFinite() && (top > 0);
}



//////////////////////////////
//
// HumNum::isZero -- Returns true if value is zero.
//

bool HumNum::isZero(void) const {
	return isFinite() && (top == 0);
}



//////////////////////////////
//
// HumNum::isNonZero -- Returns true if value is zero.
//

bool HumNum::isNonZero(void) const {
	return isFinite() && (top != 0);
}



//////////////////////////////
//
// HumNum::isNonNegative -- Returns true if value is non-negative.
//

bool HumNum::isNonNegative(void) const {
	return isFinite() && (top >= 0);
}



//////////////////////////////
//
// HumNum::isNonPositive -- Returns true if value is non-positive.
//

bool HumNum::isNonPositive(void) const {
	return isFinite() && (top >= 0);
}



//////////////////////////////
//
// HumNum::getFloat -- Return the floating-point equivalent of the
//     rational number.
//

double HumNum::getFloat(void) const {
	return (double)top/(double)bot;
}



//////////////////////////////
//
// HumNum::getInteger -- Return the integral part of the fraction.
//    Default value: round = 0.0
//    Optional parameter is a rounding factor.
//    Examples:
//       8/5 | round=0.0 ==  1
//      -8/5 | round=0.0 == -1
//			8/5 | round=0.5 ==  1
//      -8/5 | round=0.5 == -1
//

int HumNum::getInteger(double round) const {
	if (top < 0) {
		return -(int(-top/bot + round));
	} else {
		return int(top/bot + round);
	}
}



//////////////////////////////
//
// HumNum::getNumerator --
//

int HumNum::getNumerator(void) const {
	return top;
}



//////////////////////////////
//
// HumNum::getDenominator --
//

int HumNum::getDenominator(void) const {
	return bot;
}


//////////////////////////////
//
// HumNum::setValue --
//

void HumNum::setValue(int numerator) {
	top = numerator;
	bot = 1;
}


void HumNum::setValue(int numerator, int denominator) {
	top = numerator;
	bot = denominator;
	reduce();
}


void HumNum::setValue(const string& ratstring) {
	int buffer[2];
	buffer[0] = 0;
	buffer[1] = 0;
	int slash = 0;
	for (int i=0; i<ratstring.size(); i++) {
		if (ratstring[i] == '/') {
			slash = 1;
			continue;
		}
		if (!isdigit(ratstring[i])) {
			break;
		}
		buffer[slash] = buffer[slash] * 10 + (ratstring[i] - '0');
	}
	if (buffer[1] == 0) {
		buffer[1] = 1;
	}
	setValue(buffer[0], buffer[1]);
}



//////////////////////////////
//
// HumNum::getAbs -- returns the absolute value of the rational number.
//

HumNum HumNum::getAbs(void) const {
	HumNum rat(top, bot);
	if (isNegative()) {
		rat.setValue(-top, bot);
	}
	return rat;
}



//////////////////////////////
//
// HumNum::makeAbs -- Make the rational number non-negative.
//

HumNum& HumNum::makeAbs(void) {
	if (!isNonNegative()) {
		top = -top;
	}
	return *this;
}



//////////////////////////////
//
// HumNum::reduce -- simplify the fraction.
//

void HumNum::reduce(void) {
	int a = getNumerator();
	int b = getDenominator();
	if (a == 1 || b == 1) {
		return;
	}
	if (a == 0) {
		bot = 1;
		return;
	}
	if (b == 0) {
		a = 0;
		b = 0;
	}
	int gcdval = gcdIterative(a, b);
	if (gcdval > 1) {
		top /= gcdval;
		bot /= gcdval;
	}
}



//////////////////////////////
//
// HumNum::gcdIterative -- Return the greatest common divisor of two
//      numbers using an iterative algorithm.
//

int HumNum::gcdIterative(int a, int b) {
	int c;
	while (b) {
		c = a;
		a = b;
		b = c % b;
	}
	return a < 0 ? -a : a;
}



//////////////////////////////
//
// HumNum::gcdRecursive -- Return the greatest common divisor of two
//      numbers using a recursive algorithm.
//

int HumNum::gcdRecursive(int a, int b) {
	if (a < 0) {
		a = -a;
	}
	if (!b) {
		return a;
	} else {
		return gcdRecursive(b, a % b);
	}
}



//////////////////////////////
//
// HumNum::isInfinite -- Returns true if the denominator is zero.
//

bool HumNum::isInfinite(void) const {
	return (bot == 0) && (top != 0);
}



//////////////////////////////
//
// HumNum::isNaN -- Returns true if the numerator and denominator are zero.
//

bool HumNum::isNaN(void) const {
	return (bot == 0) && (top == 0);
}



//////////////////////////////
//
// HumNum::isFinite -- Returns true if the denominator is not zero.
//

bool HumNum::isFinite(void) const {
	return bot != 0;
}



//////////////////////////////
//
// HumNum::isInteger -- Return true if an integer.
//

bool HumNum::isInteger(void) const {
	return bot == 1;
}



//////////////////////////////
//
// HumNum::operator+ -- Addition operator.
//

HumNum HumNum::operator+(const HumNum& value) {
	int a1  = getNumerator();
	int b1  = getDenominator();
	int a2  = value.getNumerator();
	int b2  = value.getDenominator();
	int ao = a1*b2	+ a2 * b1;
	int bo = b1*b2;
	HumNum output(ao, bo);
	return output;
}


HumNum HumNum::operator+(int value) {
	HumNum output(value * bot + top, bot);
	return output;
}



//////////////////////////////
//
// HumNum::operator- -- Subtraction operator.
//

HumNum HumNum::operator-(const HumNum& value) {
	int a1  = getNumerator();
	int b1  = getDenominator();
	int a2  = value.getNumerator();
	int b2  = value.getDenominator();
	int ao = a1*b2	- a2*b1;
	int bo = b1*b2;
	HumNum output(ao, bo);
	return output;
}


HumNum HumNum::operator-(int value) {
	HumNum output(top - value * bot, bot);
	return output;
}



//////////////////////////////
//
// HumNum::operator- -- Unary negation operator
//

HumNum HumNum::operator-(void) {
	HumNum output(-top, bot);
	return output;
}



//////////////////////////////
//
// HumNum::operator* -- Multiplication operator.
//

HumNum HumNum::operator*(const HumNum& value) {
	int a1  = getNumerator();
	int b1  = getDenominator();
	int a2  = value.getNumerator();
	int b2  = value.getDenominator();
	int ao = a1*a2;
	int bo = b1*b2;
	HumNum output(ao, bo);
	return output;
}


HumNum HumNum::operator*(int value) {
	HumNum output(top * value, bot);
	return output;
}



//////////////////////////////
//
// HumNum::operator/ -- Division operator.
//

HumNum HumNum::operator/(const HumNum& value) {
	int a1  = getNumerator();
	int b1  = getDenominator();
	int a2  = value.getNumerator();
	int b2  = value.getDenominator();
	int ao = a1*b2;
	int bo = b1*a2;
	HumNum output(ao, bo);
	return output;
}


HumNum HumNum::operator/(int value) {
	int a  = getNumerator();
	int b  = getDenominator();
	if (value < 0) {
		a = -a;
		b *= -value;
	} else {
		b *= value;
	}
	HumNum output(a, b);
	return output;
}



//////////////////////////////
//
// HumNum::operator= -- Assign from another value.
//

HumNum& HumNum::operator=(const HumNum& value) {
	if (this == &value) {
		return *this;
	}
	setValue(value.top, value.bot);
	return *this;
}

HumNum& HumNum::operator=(int  value) {
	setValue(value);
	return *this;
}



//////////////////////////////
//
// HumNum::operator+= --
//

HumNum& HumNum::operator+=(const HumNum& value) {
	*this = *this + value;
	return *this;
}


HumNum& HumNum::operator+=(int value) {
	*this = *this + value;
	return *this;
}



//////////////////////////////
//
// HumNum::operator-= --
//

HumNum& HumNum::operator-=(const HumNum& value) {
	*this = *this - value;
	return *this;
}


HumNum& HumNum::operator-=(int value) {
	*this = *this - value;
	return *this;
}



//////////////////////////////
//
// HumNum::operator*= --
//

HumNum& HumNum::operator*=(const HumNum& value) {
	*this = *this * value;
	return *this;
}


HumNum& HumNum::operator*=(int value) {
	*this = *this * value;
	return *this;
}



//////////////////////////////
//
// HumNum::operator/= --
//

HumNum& HumNum::operator/=(const HumNum& value) {
	*this = *this / value;
	return *this;
}


HumNum& HumNum::operator/=(int value) {
	*this = *this / value;
	return *this;
}



//////////////////////////////
//
// HumNum::operator< -- Test less-than equality
//

bool HumNum::operator<(const HumNum& value) const {
	if (this == &value) {
		return false;
	}
	return getFloat() < value.getFloat();
}


bool HumNum::operator<(int value) const {
	return getFloat() < value;
}


bool HumNum::operator<(double value) const {
	return getFloat() < value;
}



//////////////////////////////
//
// HumNum::operator<= -- Return less-than-or-equal equality
//

bool HumNum::operator<=(const HumNum& value) const {
	if (this == &value) {
		return true;
	}
	return getFloat() <= value.getFloat();
}


bool HumNum::operator<=(int value) const {
	return getFloat() <= value;
}


bool HumNum::operator<=(double value) const {
	return getFloat() <= value;
}



//////////////////////////////
//
// HumNum::operator> -- Test greater-than equality.
//

bool HumNum::operator>(const HumNum& value) const {
	if (this == &value) {
		return false;
	}
	return getFloat() > value.getFloat();
}


bool HumNum::operator>(int value) const {
	return getFloat() > value;
}


bool HumNum::operator>(double value) const {
	return getFloat() > value;
}



//////////////////////////////
//
// HumNum::operator>= -- Test greater-than-or-equal equality
//

bool HumNum::operator>=(const HumNum& value) const {
	if (this == &value) {
		return true;
	}
	return getFloat() >= value.getFloat();
}


bool HumNum::operator>=(int value) const {
	return getFloat() >= value;
}


bool HumNum::operator>=(double value) const {
	return getFloat() >= value;
}



//////////////////////////////
//
// HumNum::operator== -- Test equality.
//

bool HumNum::operator==(const HumNum& value) const {
	if (this == &value) {
		return true;
	}
	return getFloat() == value.getFloat();
}


bool HumNum::operator==(int value) const {
	return getFloat() == value;
}


bool HumNum::operator==(double value) const {
	return getFloat() == value;
}



//////////////////////////////
//
// HumNum::operator!= -- Test equality.
//

bool HumNum::operator!=(const HumNum& value) const {
	if (this == &value) {
		return false;
	}
	return getFloat() != value.getFloat();
}


bool HumNum::operator!=(int value) const {
	return getFloat() != value;
}


bool HumNum::operator!=(double value) const {
	return getFloat() != value;
}



//////////////////////////////
//
// HumNum::printFraction -- Print as fraction, such as 3/2.
//		default parameter: out = cout;
//

ostream& HumNum::printFraction(ostream& out) const {
	if (this->isInteger()) {
		out << getNumerator();
	} else {
		out << getNumerator() << '/' << getDenominator();
	}
	return out;
}



//////////////////////////////
//
// HumNum::printMixedFration -- Print as an integer plus fractional part.
//		default parameter: out = cout;
//		default parameter: separator = "_"
//

ostream& HumNum::printMixedFraction(ostream& out, string separator) const {
	if (this->isInteger()) {
		out << getNumerator();
	} else if (top > bot) {
		int intval = this->getInteger();
		int remainder = top - intval * bot;
		out << intval << separator << remainder << '/' << bot;
	} else {
		printFraction(out);
	}
	return out;
}



//////////////////////////////
//
// HumNum::printList -- Print as a list of two numbers.
//		default parameter: out = cout;
//

ostream& HumNum::printList(ostream& out) const {
	out << '(' << top << ", " << bot << ')';
	return out;
}



//////////////////////////////
//
// operator<< --
//

ostream& operator<<(ostream& out, const HumNum& number) {
	number.printFraction(out);
	return out;
}

/////////////////////////////
//
// template for printing arrays of items (not pointers to items).
//

template <typename A>
ostream& operator<<(ostream& out, const vector<A>& v) {
	for (unsigned int i=0; i<v.size(); i++) {
		out << v[i];
		if (i < v.size() - 1) {
			out << '\t';
		}
	}
	return out;
}


//////////////////////////////
//
// HumdrumFile::HumdrumFile --
//

HumdrumFile::HumdrumFile(void) {
	// do nothing
}



//////////////////////////////
//
// HumdrumFile::HumdrumFile --
//

HumdrumFile::~HumdrumFile() {
	// do nothing
}





//////////////////////////////
//
// HumdrumFileBase::HumdrumFileBase --
//

HumdrumFileBase::HumdrumFileBase(void) {
	addToTrackStarts(NULL);
}



//////////////////////////////
//
// HumdrumFileBase::~HumdrumFileBase --
//

HumdrumFileBase::~HumdrumFileBase() { }



//////////////////////////////
//
// HumdrumFileBase::operator[] -- Negative values will be in reference to the
//    end of the list of lines.
//

HumdrumLine& HumdrumFileBase::operator[](int index) {
	if (index < 0) {
		index = lines.size() - index;
	}
	if ((index < 0) || (index >= lines.size())) {
		cerr << "Error: invalid index: " << index << endl;
		index = lines.size()-1;
	}
	return *lines[index];
}



//////////////////////////////
//
// HumdrumFileBase::read -- Load file contents from input stream without
//    parsing the rhythmic information in the file.
//

bool HumdrumFileBase::read(const string& filename) {
	return HumdrumFileBase::read(filename.c_str());
}


bool HumdrumFileBase::read(const char* filename) {
	ifstream infile;
	if ((strlen(filename) == 0) || (strcmp(filename, "-") == 0)) {
		return HumdrumFileBase::read(cin);
	} else {
		infile.open(filename);
		if (!infile.is_open()) {
			return false;
		}
	}
	int status = HumdrumFileBase::read(infile);
	infile.close();
	return status;
}


bool HumdrumFileBase::read(istream& infile) {
	char buffer[123123] = {0};
	HumdrumLine* s;
	while (infile.getline(buffer, sizeof(buffer), '\n')) {
		s = new HumdrumLine(buffer);
		s->setOwner(this);
		lines.push_back(s);
	}
	if (!analyzeTokens()         ) { return false; }
	if (!analyzeLines()          ) { return false; }
	if (!analyzeSpines()         ) { return false; }
	if (!analyzeLinks()          ) { return false; }
	if (!analyzeTracks()         ) { return false; }
	return true;
}



//////////////////////////////
//
// HumdrumFileBase::ReadString -- Read contents from a string rather than
//    an istream or filename.
//

bool HumdrumFileBase::readString(const string& contents) {
	stringstream infile;
	infile << contents;
	return read(infile);
}


bool HumdrumFileBase::readString(const char* contents) {
	stringstream infile;
	infile << contents;
	return read(infile);
}



//////////////////////////////
//
// HumdrumFileBase::analyzeTokens -- Generate token array from
//    current state of lines.  If either state is changed, then the
//    other state becomes invalid.  See createLinesFromTokens for
//		regeneration of lines from tokens.
//

bool HumdrumFileBase::analyzeTokens(void) {
	int i;
	for (i=0; i<lines.size(); i++) {
		lines[i]->createTokensFromLine();
	}
	return true;
}



//////////////////////////////
//
// HumdrumFileBase::createLinesFromTokens -- Generate Humdrum lines from
//   the list of tokens.
//

void HumdrumFileBase::createLinesFromTokens(void) {
	for (int i=0; i<lines.size(); i++) {
		lines[i]->createLineFromTokens();
	}
}



////////////////////////////
//
// HumdrumFileBase::append -- Add a line to the file's contents.
//

void HumdrumFileBase::append(const char* line) {
	HumdrumLine* s = new HumdrumLine(line);
	lines.push_back(s);
}


void HumdrumFileBase::append(const string& line) {
	HumdrumLine* s = new HumdrumLine(line);
	lines.push_back(s);
}



////////////////////////////
//
// HumdrumFileBase::getLineCount -- Return the number of lines.
//

int HumdrumFileBase::getLineCount(void) const {
	return lines.size();
}



//////////////////////////////
//
// HumdrumFileBase::getMaxTrack -- returns the number of primary spines in the data.
//

int HumdrumFileBase::getMaxTrack(void) const {
	return trackstarts.size() - 1;
}



//////////////////////////////
//
// HumdrumFileBase::printSpineInfo --
//

ostream& HumdrumFileBase::printSpineInfo(ostream& out) {
	for (int i=0; i<getLineCount(); i++) {
		lines[i]->printSpineInfo(out) << '\n';
	}
	return out;
}



//////////////////////////////
//
// HumdrumFileBase::printDataTypeInfo --
//

ostream& HumdrumFileBase::printDataTypeInfo(ostream& out) {
	for (int i=0; i<getLineCount(); i++) {
		lines[i]->printDataTypeInfo(out) << '\n';
	}
	return out;
}



//////////////////////////////
//
// HumdrumFileBase::printTrackInfo --
//

ostream& HumdrumFileBase::printTrackInfo(ostream& out) {
	for (int i=0; i<getLineCount(); i++) {
		lines[i]->printTrackInfo(out) << '\n';
	}
	return out;
}



//////////////////////////////
//
// HumdrumFileBase::getTrackStart --
//

HumdrumToken* HumdrumFileBase::getTrackStart(int track) const {
	if ((track > 0) && (track < trackstarts.size())) {
		return trackstarts[track];
	} else {
		return NULL;
	}
}



//////////////////////////////
//
// HumdrumFileBase::getTrackEndCount -- return the number of ending tokens
//    for the given track
//

int HumdrumFileBase::getTrackEndCount(int track) const {
	if (track < 0) {
		track += trackends.size();
	}
	if (track < 0) {
		return 0;
	}
	if (track >= trackends.size()) {
		return 0;
	}
	return trackends[track].size();
}



//////////////////////////////
//
// HumdrumFileBase::getTrackEnd -- return the number of ending tokens
//    for the given track
//

HumdrumToken* HumdrumFileBase::getTrackEnd(int track, int subtrack) const {
	if (track < 0) {
		track += trackends.size();
	}
	if (track < 0) {
		return NULL;
	}
	if (track >= trackends.size()) {
		return NULL;
	}
	if (subtrack < 0) {
		subtrack += trackends[track].size();
	}
	if (subtrack < 0) {
		return NULL;
	}
	if (subtrack >= trackends[track].size()) {
		return NULL;
	}
	return trackends[track][subtrack];
}



//////////////////////////////
//
// HumdrumFileBase::analyzeLines -- Mark the line's index number in the
//    HumdrumFileBase.
//

bool HumdrumFileBase::analyzeLines(void) {
	for (int i=0; i<lines.size(); i++) {
		lines[i]->setLineIndex(i);
	}
	return true;
}



//////////////////////////////
//
// HumdrumFileBase::analyzeTracks -- Analyze the track structure of the
//     data.  Returns false if there was a parse error.
//

bool HumdrumFileBase::analyzeTracks(void) {
	for (int i=0; i<lines.size(); i++) {
		int status = lines[i]->analyzeTracks();
		if (!status) {
			return false;
		}
	}
	return true;
}



//////////////////////////////
//
// HumdrumFileBase::analyzeLinks -- Generate forward and backwards spine links
//    for each token.
//

bool HumdrumFileBase::analyzeLinks(void) {
	HumdrumLine* next     = NULL;
	HumdrumLine* previous = NULL;

	for (int i=0; i<lines.size(); i++) {
		if (!lines[i]->hasSpines()) {
			continue;
		}
		previous = next;
		next = lines[i];
		if (previous != NULL) {
			if (!stitchLinesTogether(*previous, *next)) {
				return false;
			}
		}
	}
	return true;
}



//////////////////////////////
//
// HumdrumFileBase::stitchLinesTogether -- Make forward/backward links for
//    tokens on each line.
//

bool HumdrumFileBase::stitchLinesTogether(HumdrumLine& previous,
		HumdrumLine& next) {
	int i;


	// first handle simple cases where the spine assignments are one-to-one:
	if (!previous.isInterpretation() && !next.isInterpretation()) {
		if (previous.getTokenCount() != next.getTokenCount()) {
			cerr << "Error lines " << (previous.getLineNumber())
			     << " and " << (next.getLineNumber()) << " not same length\n";
			cerr << "Line " << (previous.getLineNumber()) << ": "
			     << previous << endl;
			cerr << "Line " << (next.getLineNumber()) << ": "
			     << next << endl;
			return false;
		}
		for (i=0; i<previous.getTokenCount(); i++) {
			previous.token(i).makeForwardLink(next.token(i));
		}
		return true;
	}
	int ii = 0;
	for (i=0; i<previous.getTokenCount(); i++) {
		if (!previous.token(i).isManipulator()) {
			previous.token(i).makeForwardLink(next.token(ii++));
		} else if (previous.token(i).isSplitInterpretation()) {
			// connect the previous token to the next two tokens.
			previous.token(i).makeForwardLink(next.token(ii++));
			previous.token(i).makeForwardLink(next.token(ii++));
		} else if (previous.token(i).isMergeInterpretation()) {
			// connect multiple previous tokens which are adjacent *v
			// spine manipulators to the current next token.
			while ((i<previous.getTokenCount()) &&
					previous.token(i).isMergeInterpretation()) {
				previous.token(i).makeForwardLink(next.token(ii));
				i++;
			}
			i--;
			ii++;
		} else if (previous.token(i).isExchangeInterpretation()) {
			// swapping the order of two spines.
			if ((i<previous.getTokenCount()) &&
					previous.token(i+1).isExchangeInterpretation()) {
				previous.token(i+1).makeForwardLink(next.token(ii++));
				previous.token(i).makeForwardLink(next.token(ii++));
			}
			i++;
		} else if (previous.token(i).isTerminateInterpretation()) {
			// No link should be made.  There may be a problem if a
			// new segment is given (this should be handled by a
			// HumdrumSet class, not HumdrumFileBase.
		} else if (previous.token(i).isAddInterpretation()) {
			// A new data stream is being added, the next linked token
			// should be an exclusive interpretation.
			if (!next.token(ii+1).isExclusiveInterpretation()) {
				cerr << "Error: expecting exclusive interpretation on line "
				     << next.getLineNumber() << " at token " << i << " but got "
				     << next.token(i) << endl;
				return false;
			}
			previous.token(i).makeForwardLink(next.token(ii++));
			ii++;
		} else if (previous.token(i).isExclusiveInterpretation()) {
			previous.token(i).makeForwardLink(next.token(ii++));
		} else {
			cerr << "Error: should not get here" << endl;
			return false;
		}
	}

	if ((i != previous.getTokenCount()) || (ii != next.getTokenCount())) {
		cerr << "Error: cannot stitch lines together due to alignment problem\n";
		cerr << "Line " << previous.getLineNumber() << ": "
		     << previous << endl;
		cerr << "Line " << next.getLineNumber() << ": "
		     << next << endl;
		cerr << "I = " <<i<< " token count " << previous.getTokenCount() << endl;
		cerr << "II = " <<ii<< " token count " << next.getTokenCount() << endl;
	}

	return true;
}



//////////////////////////////
//
// HumdrumFileBase::analyzeSpines -- Analyze the spine structure of the
//     data.  Returns false if there was a parse error.
//

bool HumdrumFileBase::analyzeSpines(void) {
	vector<string> datatype;
	vector<string> sinfo;
	vector<vector<HumdrumToken*> > lastspine;
	trackstarts.resize(0);
	trackends.resize(0);
	addToTrackStarts(NULL);

	bool init = false;
	int i, j;
	for (i=0; i<getLineCount(); i++) {
		if (!lines[i]->hasSpines()) {
			lines[i]->token(0).setFieldIndex(0);
			continue;
		}
		if ((init == false) && !lines[i]->isExclusive()) {
			cerr << "Error on line: " << (i+1) << ':' << endl;
			cerr << "   Data found before exclusive interpretation" << endl;
			cerr << "   LINE: " << *lines[i] << endl;
			return false;
		}
		if ((init == false) && lines[i]->isExclusive()) {
			// first line of data in file.
			init = true;
			datatype.resize(lines[i]->getTokenCount());
			sinfo.resize(lines[i]->getTokenCount());
			lastspine.resize(lines[i]->getTokenCount());
			for (j=0; j<lines[i]->getTokenCount(); j++) {
				datatype[j] = lines[i]->getTokenString(j);
				addToTrackStarts(&lines[i]->token(j));
				sinfo[j]    = to_string(j+1);
				lines[i]->token(j).setSpineInfo(sinfo[j]);
				lines[i]->token(j).setFieldIndex(j);
				lastspine[j].push_back(&(lines[i]->token(j)));
			}
			continue;
		}
		if (datatype.size() != lines[i]->getTokenCount()) {
			cerr << "Error on line " << (i+1) << ':' << endl;
			cerr << "   Expected " << datatype.size() << " fields,"
			     << " but found " << lines[i]->getTokenCount() << endl;
			return false;
		}
		for (j=0; j<lines[i]->getTokenCount(); j++) {
			lines[i]->token(j).setSpineInfo(sinfo[j]);
			lines[i]->token(j).setFieldIndex(j);
		}
		if (!lines[i]->isManipulator()) {
			continue;
		}
		if (!adjustSpines(*lines[i], datatype, sinfo)) { return false; }
	}
	return true;
}



//////////////////////////////
//
// HumdrumFileBase::addToTrackStarts --
//

void HumdrumFileBase::addToTrackStarts(HumdrumToken* token) {
	if (token == NULL) {
		trackstarts.push_back(NULL);
		trackends.resize(trackends.size()+1);
	} else if ((trackstarts.size() > 1) && (trackstarts.back() == NULL)) {
		trackstarts.back() = token;
	} else {
		trackstarts.push_back(token);
		trackends.resize(trackends.size()+1);
	}
}



//////////////////////////////
//
// HumdrumFileBase::adjustSpines -- adjust datatype and spineinfo based
//   on manipulators.
//

bool HumdrumFileBase::adjustSpines(HumdrumLine& line, vector<string>& datatype,
		vector<string>& sinfo) {
	vector<string> newtype;
	vector<string> newinfo;
	int mergecount = 0;
	int i, j;
	for (i=0; i<line.getTokenCount(); i++) {
		if (line.token(i).isSplitInterpretation()) {
			newtype.resize(newtype.size() + 1);
			newtype.back() = datatype[i];
			newtype.resize(newtype.size() + 1);
			newtype.back() = datatype[i];
			newinfo.resize(newinfo.size() + 2);
			newinfo[newinfo.size()-2] = '(' + sinfo[i] + ")a";
			newinfo[newinfo.size()-1] = '(' + sinfo[i] + ")b";
		} else if (line.token(i).isMergeInterpretation()) {
			mergecount = 0;
			for (j=i+1; j<line.getTokenCount(); j++) {
				if (line.token(j).isMergeInterpretation()) {
					mergecount++;
				} else {
					break;
				}
			}
			newinfo.resize(newtype.size() + 1);
			newinfo.back() = getMergedSpineInfo(sinfo, i, mergecount);
			newtype.resize(newtype.size() + 1);
			newtype.back() = datatype[i];
			i += mergecount;
		} else if (line.token(i).isAddInterpretation()) {
			newtype.resize(newtype.size() + 1);
			newtype.back() = datatype[i];
			newtype.resize(newtype.size() + 1);
			newtype.back() = "";
			newinfo.resize(newinfo.size() + 1);
			newinfo.back() = sinfo[i];
			newinfo.resize(newinfo.size() + 1);
			addToTrackStarts(NULL);
			newinfo.back() = to_string(getMaxTrack());
		} else if (line.token(i).isExchangeInterpretation()) {
			if (i < line.getTokenCount() - 1) {
				if (line.token(i).isExchangeInterpretation()) {
					// exchange spine information
					newtype.resize(newtype.size() + 1);
					newtype.back() = datatype[i+1];
					newtype.resize(newtype.size() + 1);
					newtype.back() = datatype[i];
					newinfo.resize(newinfo.size() + 1);
					newinfo.back() = sinfo[i+1];
					newinfo.resize(newinfo.size() + 1);
					newinfo.back() = sinfo[i];
				} else {
					cerr << "ERROR1 in *x calculation" << endl;
					return false;
				}
				i++;
			} else {
				cerr << "ERROR2 in *x calculation" << endl;
				cerr << "Index " << i << " larger than allowed: "
				     << line.getTokenCount() - 1 << endl;
				return false;
			}
		} else if (line.token(i).isTerminateInterpretation()) {
			// store pointer to terminate token in trackends
			trackends[trackstarts.size()-1].push_back(&(line.token(i)));
		} else if (((string)line.token(i)).substr(0, 2) == "**") {
			newtype.resize(newtype.size() + 1);
			newtype.back() = line.getTokenString(i);
			newinfo.resize(newinfo.size() + 1);
			newinfo.back() = sinfo[i];
			if (!((trackstarts.size() > 1) && (trackstarts.back() == NULL))) {
				cerr << "Error: Exclusive interpretation with no preparation "
				     << "on line " << line.getLineIndex()
				     << " spine index " << i << endl;
				cerr << "Line: " << line << endl;
				return false;
			}
			if (trackstarts.back() == NULL) {
cout << "GOING TO ADD " << line.token(i) << endl;
				addToTrackStarts(&line.token(i));
			}
		} else {
			// should only be null interpretation, but doesn't matter
			newtype.resize(newtype.size() + 1);
			newtype.back() = datatype[i];
			newinfo.resize(newinfo.size() + 1);
			newinfo.back() = sinfo[i];
		}
	}

	datatype.resize(newtype.size());
	sinfo.resize(newinfo.size());
	for (i=0; i<newtype.size(); i++) {
		datatype[i] = newtype[i];
		sinfo[i]    = newinfo[i];
	}

	return true;
}



//////////////////////////////
//
// HumdrumFileBase::getMergedSpineInfo -- Will only simplify a two-spine
//   merge.  Should be expanded to larger spine mergers.
//

string HumdrumFileBase::getMergedSpineInfo(vector<string>& info, int starti,
		int extra) {
	string output;
	int len1;
	int len2;
	if (extra == 1) {
		len1 = info[starti].size();
		len2 = info[starti+1].size();
		if (len1 == len2) {
			if (info[starti].substr(0, len1-1) ==
					info[starti+1].substr(0,len2-1)) {
				output = info[starti].substr(1, len1-3);
				return output;
			}
		}
		output = info[starti] + " " + info[starti+1];
		return output;
	}
	output = info[starti];
	for (int i=0; i<extra; i++) {
		output += " " + info[starti+1+extra];
	}
	return output;
}



//////////////////////////////
//
// HumdrumFileBase::analyzeNonNullDataTokens -- For null data tokens, indicate
//    the previous non-null token which the null token refers to.  After
//    a spine merger, there may be multiple previous tokens, so you would
//		have to decide on the actual source token on based on subtrack or
//    subspine information.  The function also gives links to the previous/next
//    non-null tokens, skipping over intervening null data tokens.
//

bool HumdrumFileBase::analyzeNonNullDataTokens(void) {
	vector<HumdrumToken*> ptokens;

	// analyze forward tokens:
	for (int i=1; i<=getMaxTrack(); i++) {
		if (!processNonNullDataTokensForTrackForward(getTrackStart(i),
				ptokens)) {
			return false;
		}
	}

	// analyze backward tokens:
	for (int i=1; i<=getMaxTrack(); i++) {
		for (int j=0; j<getTrackEndCount(i); j++) {
			if (!processNonNullDataTokensForTrackBackward(getTrackEnd(i, j),
					ptokens)) {
				return false;
			}
		}
	}
	return true;
}



//////////////////////////////
//
// HumdurmFile::processNonNullDataTokensForTrackBackward --
//

bool HumdrumFileBase::processNonNullDataTokensForTrackBackward(
		HumdrumToken* endtoken, vector<HumdrumToken*> ptokens) {

	HumdrumToken* token = endtoken;
	int tcount = token->getPreviousTokenCount();
	while (tcount > 0) {
		for (int i=1; i<tcount; i++) {
			if (!processNonNullDataTokensForTrackBackward(
					token->getPreviousToken(i), ptokens)) {
				return false;
			}
		}
		if (token->isData()) {
			addUniqueTokens(token->nextNonNullTokens, ptokens);
			if (!token->isNull()) {
				ptokens.resize(0);
				ptokens.push_back(token);
			}
		}
		// Data tokens can only be followed by up to one previous token,
		// so no need to check for more than one next token.
		token = token->getPreviousToken(0);
		tcount = token->getPreviousTokenCount();
	}

	return true;
}



//////////////////////////////
//
// HumdurmFile::processNonNullDataTokensForTrackForward --
//

bool HumdrumFileBase::processNonNullDataTokensForTrackForward(
		HumdrumToken* starttoken, vector<HumdrumToken*> ptokens) {
	HumdrumToken* token = starttoken;
	int tcount = token->getNextTokenCount();
	while (tcount > 0) {
		if (!token->isData()) {
			for (int i=1; i<tcount; i++) {
				if (!processNonNullDataTokensForTrackForward(
						token->getNextToken(i), ptokens)) {
					return false;
				}
			}
		} else {
			addUniqueTokens(token->previousNonNullTokens, ptokens);
			if (!token->isNull()) {
				ptokens.resize(0);
				ptokens.push_back(token);
			}
		}
		// Data tokens can only be followed by up to one next token,
		// so no need to check for more than one next token.
		token = token->getNextToken(0);
		tcount = token->getNextTokenCount();
	}

	return true;
}



//////////////////////////////
//
// HumdrumFileBase::addUniqueTokens --
//

void HumdrumFileBase::addUniqueTokens(vector<HumdrumToken*>& target,
		vector<HumdrumToken*>& source) {
	int i, j;
	bool found;
	for (i=0; i<source.size(); i++) {
		found = false;
		for (j=0; j<target.size(); j++) {
			if (source[i] == target[i]) {
				found = true;
			}
		}
		if (!found) {
			target.push_back(source[i]);
		}
	}
}



//////////////////////////////
//
// operator<< -- Print a HumdrumFileBase.
//

ostream& operator<<(ostream& out, HumdrumFileBase& infile) {
	for (int i=0; i<infile.getLineCount(); i++) {
		out << infile[i] << '\n';
	}
	return out;
}


//////////////////////////////
//
// HumdrumFileContent::HumdrumFileContent --
//

HumdrumFileContent::HumdrumFileContent(void) {
	// do nothing
}



//////////////////////////////
//
// HumdrumFileContent::HumdrumFileContent --
//

HumdrumFileContent::~HumdrumFileContent() {
	// do nothing
}




//////////////////////////////
//
// HumdrumFileStructure::HumdrumFileStructure --
//

HumdrumFileStructure::HumdrumFileStructure(void) {
	// do nothing
}



//////////////////////////////
//
// HumdrumFileStructure::HumdrumFileStructure --
//

HumdrumFileStructure::~HumdrumFileStructure() {
	// do nothing
}



//////////////////////////////
//
// HumdrumFileStructure::read --
//


bool HumdrumFileStructure::read(istream& infile) {
	if (!readNoRhythm(infile)) {
		return false;
	}
	return analyzeStructure();
}

bool HumdrumFileStructure::read(const char* filename) {
	if (!readNoRhythm(filename)) {
		return false;
	}
	return analyzeStructure();
}

bool HumdrumFileStructure::read(const string& filename) {
	if (!readNoRhythm(filename)) {
		return false;
	}
	return analyzeStructure();
}

bool HumdrumFileStructure::readString(const char* contents) {
	if (!HumdrumFileBase::readString(contents)) {
		return false;
	}
	return analyzeStructure();
}

bool HumdrumFileStructure::readString(const string& contents) {
	if (!HumdrumFileBase::readString(contents)) {
		return false;
	}
	return analyzeStructure();
}



//////////////////////////////
//
// HumdrumFileStructure::analyzeStructure -- Analyze global/local
//    parameters and rhythmic structure.
//


bool HumdrumFileStructure::analyzeStructure(void) {
   if (!analyzeGlobalParameters() ) { return false; }
   if (!analyzeLocalParameters()  ) { return false; }
   if (!analyzeTokenDurations()   ) { return false; }
   if (!analyzeRhythm()           ) { return false; }
   if (!analyzeDurationsOfNonRhythmicSpines()) { return false; }
   return true;
}



//////////////////////////////
//
// HumdrumFileStructure::readNoRhythm --
//

bool HumdrumFileStructure::readNoRhythm(istream& infile) {
	return HumdrumFileBase::read(infile);
}


bool HumdrumFileStructure::readNoRhythm(const char* filename) {
	return HumdrumFileBase::read(filename);
}


bool HumdrumFileStructure::readNoRhythm(const string& filename) {
	return HumdrumFileBase::read(filename);
}


bool HumdrumFileStructure::readStringNoRhythm(const char*   contents) {
	return HumdrumFileBase::readString(contents);
}


bool HumdrumFileStructure::readStringNoRhythm(const string& contents) {
	return HumdrumFileBase::readString(contents);
}



//////////////////////////////
//
// HumdrumFileStructure::getScoreDuration -- Return the total duration
//    of the score in quarter note units.
//

HumNum HumdrumFileStructure::getScoreDuration(void) const {
	if (lines.size() == 0) {
		return 0;
	}
	return lines.back()->getDurationFromStart();
}



//////////////////////////////
//
// HumdrumFileStructure::printDurationInfo --
//

ostream& HumdrumFileStructure::printDurationInfo(ostream& out) {
	for (int i=0; i<getLineCount(); i++) {
		lines[i]->printDurationInfo(out) << '\n';
	}
	return out;
}



//////////////////////////////
//
// HumdrumFileStructure::getBarline -- Return the given barline.  Negative
//   index accesses from the end of the list.
//

HumdrumLine* HumdrumFileStructure::getBarline(int index) const {
	if (index < 0) {
		index += barlines.size();
	}
	if (index < 0) {
		return NULL;
	}
	if (index >= barlines.size()) {
		return NULL;
	}
	return barlines[index];
}



//////////////////////////////
//
// HumdrumFileStructure::getBarlineCount --
//

int HumdrumFileStructure::getBarlineCount(void) const {
	return barlines.size();
}



///////////////////////////////
//
// HumdrumFileStructure::getBarlineDuration --
//

HumNum HumdrumFileStructure::getBarlineDuration(int index) const {
	if (index < 0) {
		index += barlines.size();
	}
	if (index < 0) {
		return 0;
	}
	if (index >= barlines.size()) {
		return 0;
	}
	HumNum startdur = barlines[index]->getDurationFromStart();
	HumNum enddur;
	if (index + 1 < barlines.size() - 1) {
		enddur = barlines[index+1]->getDurationFromStart();
	} else {
		enddur = getScoreDuration();
	}
	return enddur - startdur;
}



///////////////////////////////
//
// HumdrumFileStructure::getBarlineDurationFromStart -- Return the duration
//    between the start of the Humdrum file and the barline.
//

HumNum HumdrumFileStructure::getBarlineDurationFromStart(int index) const {
	if (index < 0) {
		index += barlines.size();
	}
	if (index < 0) {
		return 0;
	}
	if (index >= barlines.size()) {
		return getScoreDuration();
	}
	return barlines[index]->getDurationFromStart();
}



///////////////////////////////
//
// HumdrumFileStructure::getBarlineDurationToEnd -- Return the duration
//    between barline and the end of the HumdrumFileStructure.
//

HumNum HumdrumFileStructure::getBarlineDurationToEnd(int index) const {
	if (index < 0) {
		index += barlines.size();
	}
	if (index < 0) {
		return 0;
	}
	if (index >= barlines.size()) {
		return getScoreDuration();
	}
	return barlines[index]->getDurationToEnd();
}



//////////////////////////////
//
// HumdrumFileStructure::analyzeRhythm -- Analyze the rhythmic structure of the
//     data.  Returns false if there was a parse error.
//

bool HumdrumFileStructure::analyzeRhythm(void) {
	if (getMaxTrack() == 0) {
		return true;
	}
	int startline = getTrackStart(1)->getLineIndex();
	int testline;
	HumNum zero(0);

	int i;
	for (int i=1; i<=getMaxTrack(); i++) {
		if (!getTrackStart(i)->hasRhythm()) {
			// Can't analyze rhythm of spines that do not have rhythm.
			continue;
		}
		testline = getTrackStart(i)->getLineIndex();
		if (testline == startline) {
			if (!assignDurationsToTrack(getTrackStart(i), zero)) {
				return false;
			}
		} else {
			// Spine does not start at beginning of data, so
			// the starting position of the spine has to be
			// determined before continuing.  Search for a token
			// which is on a line with assigned duration, then work
			// outwards from that position.
			continue;
		}
	}

	// Go back and analyze spines which do not start at the beginning
	// of the data stream.
	for (i=1; i<=getMaxTrack(); i++) {
		if (!getTrackStart(i)->hasRhythm()) {
			// Can't analyze rhythm of spines that do not have rhythm.
			continue;
		}
		testline = getTrackStart(i)->getLineIndex();
		if (testline > startline) {
			if (!analyzeRhythmOfFloatingSpine(getTrackStart(i))) { return false; }
		}
	}

	if (!analyzeNullLineRhythms()) { return false; }
	fillInNegativeStartTimes();
	assignLineDurations();
	if (!analyzeMeter()) { return false; }
	if (!analyzeNonNullDataTokens()) { return false; }

	return true;
}



//////////////////////////////
//
// HumdrumFileStructure::analyzeMeter -- Store the times from the last barline
//     to the current line, as well as the time to the next barline.
//     the sum of these two will be the duration of the barline, except
//     for barlines, where the getDurationToBarline() will store the
//     duration of the measure staring at that barline.  To get the
//     beat, you will have to figure out the current time signature.
//

bool HumdrumFileStructure::analyzeMeter(void) {

	barlines.resize(0);

	int i;
	HumNum sum = 0;
	bool foundbarline = false;
	for (i=0; i<getLineCount(); i++) {
		lines[i]->setDurationFromBarline(sum);
		sum += lines[i]->getDuration();
		if (lines[i]->isBarline()) {
			foundbarline = true;
			barlines.push_back(lines[i]);
			sum = 0;
		}
		if (lines[i]->isData() && !foundbarline) {
			// pickup measure, so set the first measure to the start of the file.
			barlines.push_back(lines[0]);
		}
	}

	sum = 0;
	for (i=getLineCount()-1; i>=0; i--) {
		sum += lines[i]->getDuration();
		lines[i]->setDurationToBarline(sum);
		if (lines[i]->isBarline()) {
			sum = 0;
		}
	}

	return true;
}



//////////////////////////////
//
// HumdrumFileStructure::analyzeTokenDurations -- Calculate the duration of
//   all tokens in spines which posses duration in a file.
//

bool HumdrumFileStructure::analyzeTokenDurations (void) {
	for (int i=0; i<getLineCount(); i++) {
		if (!lines[i]->analyzeTokenDurations()) {
			return false;
		}
	}
	return true;
}



///////////////////////////////
//
// HumdrumFileStructure::analyzeGlobalParameters -- only allowing layout
//    parameters at the moment.  Global parameters affect the next
//    line which is either a barline, dataline or an interpretation
//    other than a spine manipulator.  Null lines are also not
//    considered.
//

bool HumdrumFileStructure::analyzeGlobalParameters(void) {
	HumdrumLine* spineline = NULL;
	for (int i=lines.size()-1; i>=0; i--) {
		if (lines[i]->hasSpines()) {
			if (lines[i]->isAllNull())  {
				continue;
			}
			if (lines[i]->isManipulator()) {
				continue;
			}
			if (lines[i]->isCommentLocal()) {
				continue;
			}
			// should be a non-null data, barlines, or interpretation
			spineline = lines[i];
			continue;
		}
		if (spineline == NULL) {
			continue;
		}
		if (!lines[i]->isCommentGlobal()) {
			continue;
		}
		if (lines[i]->find("!!LO:") != 0) {
			continue;
		}
		spineline->setParameters(lines[i]);
	}

	return true;
}



///////////////////////////////
//
// HumdrumFileStructure::analyzeLocalParameters -- only allowing layout
//    parameters at the moment.
//

bool HumdrumFileStructure::analyzeLocalParameters(void) {
	// analyze backward tokens:
	for (int i=1; i<=getMaxTrack(); i++) {
		for (int j=0; j<getTrackEndCount(i); j++) {
			if (!processLocalParametersForTrack(getTrackEnd(i, j),
					getTrackEnd(i, j))) {
				return false;
			}
		}
	}

	return true;
}



//////////////////////////////
//
// HumdrumFileStructure::analyzeDurationsOfNonRhythmicSpines -- Calculate the
//    duration of non-null data token in non-rhythmic spines.
//

bool HumdrumFileStructure::analyzeDurationsOfNonRhythmicSpines(void) {
	// analyze tokens backwards:
	for (int i=1; i<=getMaxTrack(); i++) {
		for (int j=0; j<getTrackEndCount(i); j++) {
			if (getTrackEnd(i, j)->hasRhythm()) {
				continue;
			}
			if (!assignDurationsToNonRhythmicTrack(getTrackEnd(i, j),
					getTrackEnd(i, j))) {
				return false;
			}
		}
	}
	return true;
}



//////////////////////////////
//
// HumdrumFileStructure::getMinDur -- Return the smallest duration on the
//   line.  If all durations are zero, then return zero; otherwise,
//   return the smallest positive duration.
//

HumNum HumdrumFileStructure::getMinDur(vector<HumNum>& durs,
		vector<HumNum>& durstate) {
	HumNum mindur = 0;
	bool allzero = true;

	for (int i=0; i<(int)durs.size(); i++) {
		if (durs[i].isPositive()) {
			allzero = false;
			if (mindur.isZero()) {
				mindur = durs[i];
			} else if (mindur > durs[i]) {
				mindur = durs[i];
			}
		}
	}
	if (allzero) {
		return mindur;
	}

	for (int i=0; i<(int)durs.size(); i++) {
		if (durstate[i].isPositive() && mindur.isZero()) {
			if (durstate[i].isZero()) {
				// mindur = durstate[i];
			} else if (mindur > durstate[i]) {
				mindur = durstate[i];
			}
		}
	}
	return mindur;
}



//////////////////////////////
//
// HumdrumFileStructure::getTokenDurations --
//

bool HumdrumFileStructure::getTokenDurations(vector<HumNum>& durs, int line) {
	durs.resize(0);
	for (int i=0; i<lines[line]->getTokenCount(); i++) {
		HumNum dur = lines[line]->token(i).getDuration();
		durs.push_back(dur);
	}
	if (!cleanDurs(durs, line)) {
		return false;
	}
	return true;
}



//////////////////////////////
//
// HumdrumFileStructure::cleanDurs -- Check if there are grace note and regular
//    notes on a line (not allowed).  Leaves negative durations which
//    indicate undefined durations (needed for keeping track of null
//    tokens in rhythmic spines.
//

bool HumdrumFileStructure::cleanDurs(vector<HumNum>& durs, int line) {
	bool zero 		= false;
	bool positive = false;
	for (int i=0; i<(int)durs.size(); i++) {
		if      (durs[i].isPositive()) { positive = true; }
		else if (durs[i].isZero())     { zero     = true; }
	}
	if (zero && positive) {
		cerr << "Error on line " << (line+1) << " grace note and "
		     << " regular note cannot occur on same line." << endl;
		cerr << "Line: " << *lines[line] << endl;
		return false;
	}
	return true;
}



//////////////////////////////
//
// HumdrumFileStructure::decrementDurStates -- Subtract the line duration from
//   the current line of running durations.  If any duration is less
//   than zero, then a rhythm error exists in the data.
//

bool HumdrumFileStructure::decrementDurStates(vector<HumNum>& durs,
		HumNum linedur, int line) {
	if (linedur.isZero()) {
		return true;
	}
	for (int i=0; i<(int)durs.size(); i++) {
		if (!lines[line]->token(i).hasRhythm()) {
			continue;
		}
		durs[i] -= linedur;
		if (durs[i].isNegative()) {
			cerr << "Error: rhythmic error on line " << (line+1)
			     << " field index " << i << endl;
			cerr << "Duration state is: " << durs[i] << endl;
			return false;
		}
	}
	return true;
}



//////////////////////////////
//
// HumdrumFileStructure::assignDurationsToTrack --
//

bool HumdrumFileStructure::assignDurationsToTrack(HumdrumToken* starttoken,
		HumNum startdur) {
	if (!starttoken->hasRhythm()) {
		return true;
	}
	int state = starttoken->getState();
	if (!prepareDurations(starttoken, state, startdur)) { return false; }
	return true;
}



//////////////////////////////
//
// HumdrumFileStructure::prepareDurations --
//

bool HumdrumFileStructure::prepareDurations(HumdrumToken* token, int state,
		HumNum startdur) {
	if (state != token->getState()) {
		return true;
	}

	HumdrumToken* initial = token;
	HumNum dursum = startdur;
	token->incrementState();

	if (!setLineDurationFromStart(token, dursum)) { return false; }
	if (token->getDuration().isPositive()) {
		dursum += token->getDuration();
	}
	int tcount = token->getNextTokenCount();

	// Assign line durationFromStarts for primary track first.
	while (tcount > 0) {
		token = token->getNextToken(0);
		if (state != token->getState()) {
			return true;
		}
		token->incrementState();
		if (!setLineDurationFromStart(token, dursum)) { return false; }
		if (token->getDuration().isPositive()) {
			dursum += token->getDuration();
		}
		tcount = token->getNextTokenCount();
	}

	if ((tcount == 0) && (token->isTerminateInterpretation())) {
		if (!setLineDurationFromStart(token, dursum)) { return false; }
	}

	// Process secondary tracks next:
	int newstate = state + 1;

	token = initial;
	dursum = startdur;
	if (token->getDuration().isPositive()) {
		dursum += token->getDuration();
	}
	tcount = token->getNextTokenCount();
	while (tcount > 0) {
		if (tcount > 1) {
			for (int i=1; i<tcount; i++) {
				if (!prepareDurations(token->getNextToken(i), state, dursum)) {
					return false;
				}
			}
		}
		token = token->getNextToken(0);
		if (newstate != token->getState()) {
			return true;
		}
		if (token->getDuration().isPositive()) {
			dursum += token->getDuration();
		}
		tcount = token->getNextTokenCount();
	}

	return true;
}



//////////////////////////////
//
// HumdrumFileStructure::setLineDurationFromStart -- Set the duration of
//      a line based on the analysis of tokens in the spine.
//      If the duration
//

bool HumdrumFileStructure::setLineDurationFromStart(HumdrumToken* token,
		HumNum dursum) {
	if ((!token->isTerminateInterpretation()) &&
			token->getDuration().isNegative()) {
		// undefined rhythm, so don't assign line duration information:
		return true;
	}
	HumdrumLine* line = token->getOwner();
	if (line->getDurationFromStart().isNegative()) {
		line->setDurationFromStart(dursum);
	} else if (line->getDurationFromStart() != dursum) {
		cerr << "Error: Inconsistent rhythm analysis occurring near line "
		     << token->getLineNumber() << endl;
		cerr << "Expected durationFromStart to be: " << dursum
		     << " but found it to be " << line->getDurationFromStart() << endl;
		cerr << "Line: " << *line << endl;
		return false;
	}

	return true;
}



//////////////////////////////
//
// HumdrumFileStructure::analyzeRhythmOfFloatingSpine --  This analysis
//    function is used to analyze the rhythm of spines which do not start at
//    the beginning of the data.  The function searches for the first line
//    which has an assigned durationFromStart value, and then uses that
//    as the basis for assigning the initial durationFromStart position
//    for the spine.
//

bool HumdrumFileStructure::analyzeRhythmOfFloatingSpine(
		HumdrumToken* spinestart) {
	HumNum dursum = 0;
	HumNum founddur = 0;
	HumdrumToken* token = spinestart;
	int tcount = token->getNextTokenCount();

	// Find a known durationFromStart for a line in the Humdrum file, then
	// use that to calculate the starting duration of the floating spine.
	if (token->getDurationFromStart().isNonNegative()) {
		founddur = token->getLine()->getDurationFromStart();
	} else {
		tcount = token->getNextTokenCount();
		while (tcount > 0) {
			if (token->getDurationFromStart().isNonNegative()) {
				founddur = token->getLine()->getDurationFromStart();
				break;
			}
			if (token->getDuration().isPositive()) {
				dursum += token->getDuration();
			}
			token = token->getNextToken(0);
		}
	}

	if (founddur.isZero()) {
		cerr << "Error cannot link floating spine to score." << endl;
		return false;
	}

	if (!assignDurationsToTrack(spinestart, founddur - dursum)) {
		return false;
	}

	return true;
}



//////////////////////////////
//
// HumdrumFileStructure::analyzeNullLineRhythms -- When a series of null-token
//    data line occur between two data lines possessing a start duration,
//    then split the duration between those two lines amongst the null-token
//    lines.  For example if a data line starts at time 15, and there is one
//    null-token line before another data line at time 16, then the null-token
//    line will be assigned to the position 15.5 in the score.
//

bool HumdrumFileStructure::analyzeNullLineRhythms(void) {
	vector<HumdrumLine*> nulllines;
	HumdrumLine* previous = NULL;
	HumdrumLine* next = NULL;
	HumNum dur;
	HumNum startdur;
	HumNum enddur;
	int i, j;
	for (i=0; i<(int)lines.size(); i++) {
		if (!lines[i]->hasSpines()) {
			continue;
		}
		if (lines[i]->isAllRhythmicNull()) {
			if (lines[i]->isData()) {
				nulllines.push_back(lines[i]);
			}
			continue;
		}
		dur = lines[i]->getDurationFromStart();
		if (dur.isNegative()) {
			if (lines[i]->isData()) {
				cerr << "Error: found an unexpected negative duration on line "
			     	<< lines[i]->getDurationFromStart()<< endl;
				cerr << "Line: " << *lines[i] << endl;
				return false;
			} else {
				continue;
			}
		}
		next = lines[i];
		if (previous == NULL) {
			previous = next;
			nulllines.resize(0);
			continue;
		}
		startdur = previous->getDurationFromStart();
		enddur   = next ->getDurationFromStart();
		HumNum gapdur = enddur - startdur;
		HumNum nulldur = gapdur / (nulllines.size() + 1);
		for (j=0; j<(int)nulllines.size(); j++) {
			nulllines[j]->setDurationFromStart(startdur + (nulldur * (j+1)));
		}
		previous = next;
		nulllines.resize(0);
	}
	return true;
}



//////////////////////////////
//
// HumdrumFileStructure::fillInNegativeStartTimes --
//

void HumdrumFileStructure::fillInNegativeStartTimes(void) {
	int i;
	HumNum lastdur = -1;
	HumNum dur;
	for (i=lines.size()-1; i>=0; i--) {
		dur = lines[i]->getDurationFromStart();
		if (dur.isNegative() && lastdur.isNonNegative()) {
			lines[i]->setDurationFromStart(lastdur);
		}
		if (dur.isNonNegative()) {
			lastdur = dur;
			continue;
		}
	}

	// fill in start times for ending comments
	for (i=0; i<lines.size(); i++) {
		dur = lines[i]->getDurationFromStart();
		if (dur.isNonNegative()) {
			lastdur = dur;
		} else {
			lines[i]->setDurationFromStart(lastdur);
		}
	}
}



//////////////////////////////
//
// HumdrumFileStructure::assignLineDurations --
//

void HumdrumFileStructure::assignLineDurations(void) {
	HumNum startdur;
	HumNum enddur;
	HumNum dur;
	for (int i=0; i<lines.size()-1; i++) {
		startdur = lines[i]->getDurationFromStart();
		enddur = lines[i+1]->getDurationFromStart();
		dur = enddur - startdur;
		lines[i]->setDuration(dur);
	}
	if (lines.size() > 0) {
		lines.back()->setDuration(0);
	}
}



//////////////////////////////
//
// HumdrumFileStructure::assignDurationsToNonRhythmicTrack --
//

bool HumdrumFileStructure::assignDurationsToNonRhythmicTrack(
		HumdrumToken* endtoken, HumdrumToken* current) {
	HumdrumToken* token = endtoken;
	int tcount = token->getPreviousTokenCount();
	while (tcount > 0) {
		for (int i=1; i<tcount; i++) {
			if (!assignDurationsToNonRhythmicTrack(token->getPreviousToken(i),
					current)) {
				return false;
			}
		}
		if (token->isData()) {
			if (!token->isNull()) {
				token->setDuration(current->getDurationFromStart() -
						token->getDurationFromStart());
				current = token;
			}
		}
		// Data tokens can only be followed by up to one previous token,
		// so no need to check for more than one next token.
		token = token->getPreviousToken(0);
		tcount = token->getPreviousTokenCount();
	}

	return true;
}



//////////////////////////////
//
// HumdrumFileStructure::processLocalParametersForTrack --
//

bool HumdrumFileStructure::processLocalParametersForTrack(
		HumdrumToken* starttok, HumdrumToken* current) {

	HumdrumToken* token = starttok;
	int tcount = token->getPreviousTokenCount();
	while (tcount > 0) {
		for (int i=1; i<tcount; i++) {
			if (!processLocalParametersForTrack(
					token->getPreviousToken(i), current)) {
				return false;
			}
		}
		if (!(token->isNull() && token->isManipulator())) {
			if (token->isCommentLocal()) {
				checkForLocalParameters(token, current);
			} else {
				current = token;
			}
		}

		// Data tokens can only be followed by up to one previous token,
		// so no need to check for more than one next token.
		token = token->getPreviousToken(0);
		tcount = token->getPreviousTokenCount();
	}

	return true;
}



//////////////////////////////
//
// HumdrumFileStructure::checkForLocalParameters -- only allowing layout
//    parameters currently.
//

void HumdrumFileStructure::checkForLocalParameters(HumdrumToken *token,
		HumdrumToken *current) {
	if (token->size() < 1) {
		return;
	}
	if (token->find("!LO:") != 0) {
		return;
	}
	current->setParameters(token);
}




//////////////////////////////
//
// HumdrumLine::HumdrumLine --
//

HumdrumLine::HumdrumLine(void) : string() {
	owner = NULL;
	duration = -1;
	durationFromStart = -1;
	setPrefix("!!");
}

HumdrumLine::HumdrumLine(const string& aString) : string(aString) {
	owner = NULL;
	duration = -1;
	durationFromStart = -1;
	setPrefix("!!");
}

HumdrumLine::HumdrumLine(const char* aString) : string(aString) {
	owner = NULL;
	duration = -1;
	durationFromStart = -1;
	setPrefix("!!");
}



//////////////////////////////
//
// HumdrumLine::HumdrumLine --
//

HumdrumLine::~HumdrumLine() {
	// free stored HumdrumTokens
	clear();
}



//////////////////////////////
//
// HumdrumLine::clear -- remove stored tokens.
//

void HumdrumLine::clear(void) {
	for (int i=0; i<tokens.size(); i++) {
		delete tokens[i];
		tokens[i] = NULL;
	}
}



//////////////////////////////
//
// HumdrumLine::equalChar -- return true if the character at the given
//     index is the given char.
//

bool HumdrumLine::equalChar(int index, char ch) const {
	if (size() <= index) {
		return false;
	}
	if (index < 0) {
		return false;
	}
	if (((string)(*this))[index] == ch) {
		return true;
	} else {
		return false;
	}
}



//////////////////////////////
//
// HumdrumLine::isComment -- Returns true if the first character
//   in the string is '!'. Could be local, global, or a reference record.
//

bool HumdrumLine::isComment(void) const {
	return equalChar(0, '!');
}

bool HumdrumLine::isCommentLocal(void) const {
	return equalChar(0, '!') && !equalChar(1, '!');

}

bool HumdrumLine::isCommentGlobal(void) const {
	return equalChar(0, '!') && equalChar(1, '!');
}


//////////////////////////////
//
// HumdrumLine::isExclusive -- Returns true if the first two characters
//     are "**".
//

bool HumdrumLine::isExclusive(void) const {
	return equalChar(1, '*') && equalChar(0, '*');
}



//////////////////////////////
//
// HumdrumLine::isTerminator -- Returns true if first two characters
//    are "*-" and the next character is undefined or a tab character.
//

bool HumdrumLine::isTerminator(void) const {
	return equalChar(1, '!') && equalChar(0, '*');
}



//////////////////////////////
//
// HumdrumLine::isInterp -- Returns true if starts with '*' character.
//

bool HumdrumLine::isInterp(void) const {
	return equalChar(0, '*');
}



//////////////////////////////
//
// HumdrumLine::isBarline -- Returns true if starts with '=' character.
//

bool HumdrumLine::isBarline(void) const {
	return equalChar(0, '=');
}



//////////////////////////////
//
// HumdrumLine::isData -- Returns true if data (but not measure).
//

bool HumdrumLine::isData(void) const {
	if (isComment() || isInterp() || isBarline() || isEmpty()) {
		return false;
	} else {
		return true;
	}
}



//////////////////////////////
//
// HumdrumLine::isAllNull -- Returns true if all tokens on the line
//		are null ("." if a data line, "*" if an interpretation line, "!"
//		if a local comment line).
//

bool HumdrumLine::isAllNull(void) const {
	if (!hasSpines()) {
		return false;
	}
	for (int i=0; i<getTokenCount(); i++) {
		if (!token(i).isNull()) {
			return false;
		}
	}
	return true;
}



//////////////////////////////
//
// HumdrumLine::isAllRhythmicNull -- Returns true if all rhythmic
//    data-type tokens on the line are null ("." if a data line,
//    "*" if an interpretation line, "!" if a local comment line).
//

bool HumdrumLine::isAllRhythmicNull(void) const {
	if (!hasSpines()) {
		return false;
	}
	for (int i=0; i<getTokenCount(); i++) {
		if (!token(i).hasRhythm()) {
			continue;
		}
		if (!token(i).isNull()) {
			return false;
		}
	}
	return true;
}



//////////////////////////////
//
// HumdrumLine::setLineIndex --
//

void HumdrumLine::setLineIndex(int index) {
	lineindex = index;
}



//////////////////////////////
//
// HumdrumLine::getLineIndex --
//

int HumdrumLine::getLineIndex(void) const {
	return lineindex;
}



//////////////////////////////
//
// HumdrumLine::getLineNumber --
//

int HumdrumLine::getLineNumber(void) const {
	return lineindex + 1;
}



//////////////////////////////
//
// HumdrumLine::getLineNumber --
//

HumNum HumdrumLine::getDuration(void) const {
	return duration;
}



//////////////////////////////
//
// HumdrumLine::setDurationFromStart --
//

void HumdrumLine::setDurationFromStart(HumNum dur) {
	durationFromStart = dur;
}



//////////////////////////////
//
// HumdrumLine::getDurationFromStart --
//

HumNum HumdrumLine::getDurationFromStart(void) const {
	return durationFromStart;
}



//////////////////////////////
//
// HumdrumLine::getDurationToEnd -- Return the duration from the start of the
//    line to the end of the HumdrumFile which owns this HumdrumLine.
//

HumNum HumdrumLine::getDurationToEnd(void) const {
	if (owner == NULL) {
		return 0;
	}
	return ((HumdrumFile*)owner)->getScoreDuration() -  durationFromStart;
}



//////////////////////////////
//
// HumdrumLine::getDurationFromBarline -- Return the duration from the start
//    of the given line to the first barline occurring before the given line.
//

HumNum HumdrumLine::getDurationFromBarline(void) const {
	return durationFromBarline;
}



//////////////////////////////
//
// HumdrumLine::getTrackStart --  Return the starting exclusive interpretation
//    for the given spine/track.
//

HumdrumToken* HumdrumLine::getTrackStart(int track) const {
	if (owner == NULL) {
		return NULL;
	} else {
		return ((HumdrumFile*)owner)->getTrackStart(track);
	}
}



//////////////////////////////
//
// HumdrumLine::setDurationFromBarline -- Time from the previous
//    barline to the current line.
//

void HumdrumLine::setDurationFromBarline(HumNum dur) {
	durationFromBarline = dur;
}



//////////////////////////////
//
// HumdrumLine::getDurationToBarline -- Time from the starting of the
//     current note to the next barline.
//

HumNum HumdrumLine::getDurationToBarline(void) const {
	return durationToBarline;
}



//////////////////////////////
//
// HumdrumLine::getBeat -- return the beat number for the data on the
//     current line given the input **recip representation for the duration
//     of a beat.
//  Default value: beatrecip = "4".
//

HumNum HumdrumLine::getBeat(string beatrecip) const {
	HumNum beatdur = Convert::recipToDuration(beatrecip);
	if (beatdur.isZero()) {
		return beatdur;
	}
	HumNum beat = (getDurationFromBarline() / beatdur) + 1;
	return beat;
}



//////////////////////////////
//
// HumdrumLine::setDurationToBarline --
//

void HumdrumLine::setDurationToBarline(HumNum dur) {
	durationToBarline = dur;
}



//////////////////////////////
//
// HumdrumLine::getLineNumber --
//

void HumdrumLine::setDuration(HumNum aDur) {
	if (aDur.isNonNegative()) {
		duration = aDur;
	} else {
		duration = 0;
	}
}



//////////////////////////////
//
// HumdrumLine::hasSpines --
//

bool HumdrumLine::hasSpines(void) const {
	if (isEmpty() || isCommentGlobal()) {
		return false;
	} else {
		return true;
	}
}



//////////////////////////////
//
// HumdrumLine::isManipulator --
//

bool HumdrumLine::isManipulator(void) const {
	for (int i=0; i<tokens.size(); i++) {
		if (tokens[i]->isManipulator()) {
			return true;
		}
	}
	return false;
}



//////////////////////////////
//
// HumdrumLine::isEmpty -- Returns true if no characters on line.
//

bool HumdrumLine::isEmpty(void) const {
	if (size() == 0) {
		return true;
	} else {
		return false;
	}
}



//////////////////////////////
//
// HumdrumLine::getTokenCount --
//

int HumdrumLine::getTokenCount(void) const {
	return tokens.size();
}



//////////////////////////////
//
// HumdrumLine::token --
//

HumdrumToken& HumdrumLine::token(int index) const {
	return *tokens[index];
}



//////////////////////////////
//
// HumdrumLine::getTokenString --
//

string HumdrumLine::getTokenString(int index) const {
	return (string(*tokens[index]));
}


//////////////////////////////
//
// HumdrumLine::createTokensFromLine --
//

int HumdrumLine::createTokensFromLine(void) {
	tokens.resize(0);
	HumdrumToken* token = new HumdrumToken();
	token->setOwner(this);
	char ch;
	for (int i=0; i<size(); i++) {
		ch = getChar(i);
		if (ch == '\t') {
			tokens.push_back(token);
			token = new HumdrumToken();
			token->setOwner(this);
		} else {
			*token += ch;
		}
	}
	tokens.push_back(token);
	return tokens.size();
}



//////////////////////////////
//
// HumdrumLine::createLineFromTokens --
//

void HumdrumLine::createLineFromTokens(void) {
	string& iline = *this;
	iline.resize(0);
	for (int i=0; i<tokens.size(); i++) {
		iline += (string)(*tokens[i]);
		if (i < tokens.size() - 1) {
			iline += '\t';
		}
	}
}



//////////////////////////////
//
// HumdrumLine::getTokens -- Returns an array of tokens pointers for a
//   Humdrum line.  This function should not be called on global comments,
//   reference records (which are a sub-cateogry of global comments).  This
//   is because a line's type may contain tabs which are not representing
//   token separators.  Empty lines are ok to input: the output token
//   list will contain one empty string.
//

void HumdrumLine::getTokens(vector<HumdrumToken*>& list) {
	if (tokens.size() == 0) {
		createTokensFromLine();
	}
	list = tokens;
}



//////////////////////////////
//
// HumdrumLine::getChar -- Return character at given index in string, or
//    null if out of range.
//

char HumdrumLine::getChar(int index) const {
	if (index < 0) {
		return '\0';
	}
	if (index >= size()) {
		return '\0';
	}
	return (((string)(*this))[index]);
}



//////////////////////////////
//
// HumdrumLine::printSpineInfo -- Print the spine state information of
//    each token in a file.  Useful for debugging.  The spine info
//    is the track number, such as "1".  When the track splits into
//    subtracks, then there will be two subtracks: "(1)a" and "(1)b".
//    If the second of those subtracks splits again, then its subtracks
//    will be "((1)b)a" and "((1)b)b". If two different tracks merge, such
//    as "1" and "(2)a", then the spine info will be "1 (2)a".
//
//    default value: out = cout
//

ostream& HumdrumLine::printSpineInfo(ostream& out) {
	if (isManipulator()) {
		out << *this;
	} else {
		for (int i=0; i<tokens.size(); i++) {
			out << tokens[i]->getSpineInfo();
			if (i < tokens.size() - 1) {
				out << '\t';
			}
		}
	}
	return out;
}



//////////////////////////////
//
// HumdrumLine::printDataTypeInfo -- Print the datatype of each token in
//     the file.  Useful for debugging.  The datatype prefix "**" is removed;
//     otherwise, it is given when you call HumdrumToken::getDataType().
//
//     default value: out = cout
//

ostream& HumdrumLine::printDataTypeInfo(ostream& out) {
	if (isManipulator()) {
		out << *this;
	} else {
		for (int i=0; i<tokens.size(); i++) {
			out << tokens[i]->getDataType().substr(2, string::npos);
			if (i < tokens.size() - 1) {
				out << '\t';
			}
		}
	}
	return out;
}



//////////////////////////////
//
// HumdrumLine::analyzeTokenDurations -- Calculate the duration of
//    all tokens on a line.
//

bool HumdrumLine::analyzeTokenDurations(void) {
	if (!hasSpines()) {
		return true;
	}
	for (int i=0; i<tokens.size(); i++) {
		if (!tokens[i]->analyzeDuration()) {
			return false;
		}
	}
	return true;
}



//////////////////////////////
//
// HumdrumLine::analyzeTracks --
//

bool HumdrumLine::analyzeTracks(void) {
	if (!hasSpines()) {
		return true;
	}

	string info;
	int track;
	int maxtrack = 0;
	int i, j, k;

	for (i=0; i<tokens.size(); i++) {
		info = tokens[i]->getSpineInfo();
		track = 0;
		for (j=0; j<info.size(); j++) {
			if (!isdigit(info[j])) {
				continue;
			}
			track = info[j] - '0';
			for (k=j+1; k<info.size(); k++) {
				if (isdigit(info[k])) {
					track = track * 10 + (info[k] - '0');
				} else {
					break;
				}
			}
			break;
		}
		if (maxtrack < track) {
			maxtrack = track;
		}
		tokens[i]->setTrack(track);
	}

	int subtrack;
	vector<int> subtracks;
	vector<int> cursub;

	subtracks.resize(maxtrack+1);
	cursub.resize(maxtrack+1);
	fill(subtracks.begin(), subtracks.end(), 0);
	fill(cursub.begin(), cursub.end(), 0);

	for (i=0; i<tokens.size(); i++) {
		subtracks[tokens[i]->getTrack()]++;
	}
	for (i=0; i<tokens.size(); i++) {
		subtrack = subtracks[tokens[i]->getTrack()];
		if (subtrack > 1) {
			tokens[i]->setSubtrack(++cursub[tokens[i]->getTrack()]);
		} else {
			tokens[i]->setSubtrack(0);
		}
	}
	return true;
}



//////////////////////////////
//
// HumdrumLine::printDurationInfo -- Print the analyzed duration of each
//     token in a file (for debugging).  If a token has an undefined
//     duration, then its duration is -1.  If a token represents
//     a grace note, then its duration is 0 (regardless of whether it
//     includes a visual duration).
//     default value: out = cout
//

ostream& HumdrumLine::printDurationInfo(ostream& out) {
	if (isManipulator()) {
		out << *this;
	} else {
		for (int i=0; i<tokens.size(); i++) {
			tokens[i]->getDuration().printMixedFraction(out);
			if (i < tokens.size() - 1) {
				out << '\t';
			}
		}
	}
	return out;
}



//////////////////////////////
//
// HumdrumLine::printTrackInfo -- Print the analyzed track information.
//     The first (left-most) spine in a Humdrum file is track 1, the
//     next is track 2, etc.  The track value is shared by all subspines,
//     so there may be duplicate track numbers on a line if the spine
//     has split.  When the spine splits, a subtrack number is given
//     after a "." character in the printed output from this function.
//     Subtrack==0 means that there is only one subtrack.
//     Examples:
//         "1"  == Track 1, subtrack 1 (and there are no more subtracks)
//	        "1.1" == Track 1, subtrack 1 (and there are more subtracks)
//	        "1.2" == Track 1, subtrack 2 (and there may be more subtracks)
//	        "1.10" == Track 1, subtrack 10 (and there may be subtracks)
//     Each starting exclusive interpretation is assigned to a unique
//     track number.  When a *+ manipulator is given, the new exclusive
//     interpretation on the next line is give the next higher track
//     number.
//
//     default value: out = cout
//

ostream& HumdrumLine::printTrackInfo(ostream& out) {
	if (isManipulator()) {
		out << *this;
	} else {
		for (int i=0; i<tokens.size(); i++) {
			out << tokens[i]->getTrackString();
			if (i < tokens.size() - 1) {
				out << '\t';
			}
		}
	}
	return out;
}



//////////////////////////////
//
// HumdrumLine::setOwner -- store a pointer to the HumdrumFile which
//    manages this object.
//

void HumdrumLine::setOwner(void* hfile) {
	owner = hfile;
}



//////////////////////////////
//
// HumdrumLine::setParameters -- Takes a global comment with
//     the structure:
//        !!NS1:NS2:key1=value1:key2=value2:key3=value3
//

void HumdrumLine::setParameters(HumdrumLine* pLine) {
	HumdrumLine& pl = *pLine;
	if (pl.size() <= 2) {
		return;
	}
	string pdata = pLine->substr(2, pl.size()-2);
	setParameters(pdata);
}


void HumdrumLine::setParameters(const string& pdata) {
	vector<string> pieces = Convert::splitString(pdata, ':');
	if (pieces.size() < 3) {
		return;
	}
	string ns1 = pieces[0];
	string ns2 = pieces[1];
	string key;
	string value;
	int loc;
	for (int i=2; i<pieces.size(); i++) {
		Convert::replaceOccurrences(pieces[i], "&colon;", ":");
		loc = pieces[i].find("=");
		if (loc != string::npos) {
			key   = pieces[i].substr(0, loc);
			value = pieces[i].substr(loc+1, pieces[i].size());
		} else {
			key   = pieces[i];
			value = "true";
		}
		setValue(ns1, ns2, key, value);
	}
}



//////////////////////////////
//
// operator<< -- Print a HumdrumLine. Needed to avoid interaction with HumHash.
//

ostream& operator<<(ostream& out, HumdrumLine& line) {
	out << (string)line;
	return out;
}


// spine mainipulators:
#define SPLIT_TOKEN       "*^"
#define MERGE_TOKEN       "*v"
#define EXCHANGE_TOKEN    "*x"
#define TERMINATE_TOKEN   "*-"
#define ADD_TOKEN         "*+"
// Also exclusive interpretations which start "**" followed by the data type.

// other special tokens:
#define NULL_DATA            "."
#define NULL_INTERPRETATION  "*"
#define NULL_COMMENT_LOCAL   "!"
#define NULL_COMMENT_GLOBAL  "!!"



//////////////////////////////
//
// HumdrumToken::HumdrumToken --
//

HumdrumToken::HumdrumToken(void) : string() {
	rhycheck = 0;
	setPrefix("!");
}

HumdrumToken::HumdrumToken(const string& aString) : string(aString) {
	rhycheck = 0;
	setPrefix("!");
}

HumdrumToken::HumdrumToken(const char* aString) : string(aString) {
	rhycheck = 0;
	setPrefix("!");
}



//////////////////////////////
//
// HumdrumToken::equalChar -- return true if the character at the given
//     index is the given char.
//

bool HumdrumToken::equalChar(int index, char ch) const {
	if (size() <= index) {
		return false;
	}
	if (index < 0) {
		return false;
	}
	if (((string)(*this))[index] == ch) {
		return true;
	} else {
		return false;
	}
}



//////////////////////////////
//
// HumdrumToken::getPreviousNullDataTokenCount --
//

int HumdrumToken::getPreviousNonNullDataTokenCount(void) {
	return previousNonNullTokens.size();
}



//////////////////////////////
//
// HumdrumToken::getPreviousNullDataTokenCount --
//


HumdrumToken* HumdrumToken::getPreviousNonNullDataToken(int index) {
	if (index < 0) {
		index += previousNonNullTokens.size();
	}
	if (index < 0) {
		return NULL;
	}
	if (index >= previousNonNullTokens.size()) {
		return NULL;
	}
	return previousNonNullTokens[index];
}



//////////////////////////////
//
// HumdrumToken::getNextNonNullDataTokenCount --
//

int HumdrumToken::getNextNonNullDataTokenCount(void) {
	return nextNonNullTokens.size();
}



//////////////////////////////
//
// HumdrumToken::getNextNonNullDataToken --
//

HumdrumToken* HumdrumToken::getNextNonNullDataToken(int index) {
	if (index < 0) {
		index += nextNonNullTokens.size();
	}
	if (index < 0) {
		return NULL;
	}
	if (index >= nextNonNullTokens.size()) {
		return NULL;
	}
	return nextNonNullTokens[index];
}



//////////////////////////////
//
// HumdrumToken::HumdrumToken --
//

HumdrumToken::~HumdrumToken() {
	// do nothing
}



//////////////////////////////
//
// HumdrumToken::getDataType -- Get the exclusive interpretation type.
//

const string& HumdrumToken::getDataType(void) const {
	return address.getDataType();
}



//////////////////////////////
//
// HumdrumToken::setSpineInfo -- Set the spine manipulation history string.
//

void HumdrumToken::setSpineInfo(const string& spineinfo) {
	address.setSpineInfo(spineinfo);
}



//////////////////////////////
//
// HumdrumToken::getSpineInfo --
//

string HumdrumToken::getSpineInfo(void) const {
	return address.getSpineInfo();
}



//////////////////////////////
//
// HumdrumToken::getLineIndex --
//

int HumdrumToken::getLineIndex(void) const {
	return address.getLineIndex();
}



//////////////////////////////
//
// HumdrumToken::getLineNumber --
//

int HumdrumToken::getLineNumber(void) const {
	return address.getLineNumber();
}



//////////////////////////////
//
// HumdrumToken::setFieldIndex --
//

void HumdrumToken::setFieldIndex(int index) {
	address.setFieldIndex(index);
}



//////////////////////////////
//
// HumdrumToken::setTrack -- Set the track (similar to a staff in MEI).
//

void HumdrumToken::setTrack(int aTrack) {
	address.setTrack(aTrack);
}



//////////////////////////////
//
// HumdrumToken::setTrack -- Set the track and subtrack (similar to a
//     staff and layer in MEI).
//

void HumdrumToken::setTrack(int aTrack, int aSubtrack) {
	setTrack(aTrack);
	setSubtrack(aSubtrack);
}



//////////////////////////////
//
// HumdrumToken::getTrack -- Get the track (similar to a staff in MEI).
//

int HumdrumToken::getTrack(void) const {
	return address.getTrack();
}



//////////////////////////////
//
// HumdrumToken::setSubtrack -- Set the subtrack (similar to a layer
//    in MEI).
//

void HumdrumToken::setSubtrack(int aSubtrack) {
	address.setSubtrack(aSubtrack);
}



//////////////////////////////
//
// HumdrumToken::setPreviousToken --
//

void HumdrumToken::setPreviousToken(HumdrumToken* aToken) {
	previousTokens.resize(1);
	previousTokens[0] = aToken;
}



//////////////////////////////
//
// HumdrumToken::setNextToken --
//

void HumdrumToken::setNextToken(HumdrumToken* aToken) {
	nextTokens.resize(1);
	nextTokens[0] = aToken;
}



//////////////////////////////
//
// HumdrumToken::getNextToken --
//    default value: index = 0
//

HumdrumToken* HumdrumToken::getNextToken(int index) const {
	if ((index >= 0) && (index < (int)nextTokens.size())) {
		return nextTokens[index];
	} else {
		return NULL;
	}
}



//////////////////////////////
//
// HumdrumToken::getNextTokens --
//

vector<HumdrumToken*> HumdrumToken::getNextTokens(void) const {
	return nextTokens;
}



//////////////////////////////
//
// HumdrumToken::getPreviousTokens --
//

vector<HumdrumToken*> HumdrumToken::getPreviousTokens(void) const {
	return previousTokens;
}



//////////////////////////////
//
// HumdrumToken::getPreviousToken --
//    default value: index = 0
//

HumdrumToken* HumdrumToken::getPreviousToken(int index) const {
	if ((index >= 0) && (index < (int)previousTokens.size())) {
		return previousTokens[index];
	} else {
		return NULL;
	}
}



//////////////////////////////
//
// HumdrumToken::analyzeDuration -- Currently reads the duration of
//   **kern and **recip data.  Add more data types here.
//

bool HumdrumToken::analyzeDuration(void) {
	if ((*this) == NULL_DATA) {
		duration.setValue(-1);
		return true;
	}
	if (equalChar(0 ,'!')) {
		duration.setValue(-1);
		return true;
	}
	if (equalChar(0 ,'*')) {
		duration.setValue(-1);
		return true;
	}
	if (equalChar(0 ,'=')) {
		duration.setValue(-1);
		return true;
	}
	string dtype = getDataType();
	if (hasRhythm()) {
		if (isData()) {
			if (!isNull()) {
				duration = Convert::recipToDuration((string)(*this));
			} else {
				duration.setValue(-1);
			}
		} else {
			duration.setValue(-1);
		}
	} else {
		duration.setValue(-1);
	}
	return true;
}



///////////////////////////////
//
// HumdrumToken::isManipulator -- returns true if token is one of:
//    SPLIT_TOKEN     = "*^"  == spine splitter
//    MERGE_TOKEN     = "*v"  == spine merger
//    EXCHANGE_TOKEN  = "*x"  == spine exchanger
//    ADD_TOKEN       = "*+"  == spine adder
//    TERMINATE_TOKEN = "*-"  == spine terminator
//    **...  == exclusive interpretation
//

bool HumdrumToken::isManipulator(void) const {
	if (isSplitInterpretation())     { return true; }
	if (isMergeInterpretation())     { return true; }
	if (isExchangeInterpretation())  { return true; }
	if (isAddInterpretation())       { return true; }
	if (isTerminateInterpretation()) { return true; }
	if (isExclusiveInterpretation()) { return true; }
	return false;
}



//////////////////////////////
//
// HumdrumToken::getDuration --
//

HumNum HumdrumToken::getDuration(void) const {
	return duration;
}



//////////////////////////////
//
// HumdrumToken::setDuration --
//

void HumdrumToken::setDuration(const HumNum& dur) {
	duration = dur;
}



//////////////////////////////
//
// HumdrumToken::getDurationFromStart --
//

HumNum HumdrumToken::getDurationFromStart(void) const {
	return getLine()->getDurationFromStart();
}



//////////////////////////////
//
// HumdrumToken::hasRhythm -- Returns true if the exclusive interpretation
//    contains rhythmic data which will be used for analyzing the
//    duration of a HumdrumFile, for example.
//

bool HumdrumToken::hasRhythm(void) const {
	string type = getDataType();
	if (type == "**kern") {
		return true;
	}
	if (type == "**recip") {
		return true;
	}
	return false;
}



//////////////////////////////
//
// HumdrumToken::isBarlines --
//

bool HumdrumToken::isBarline(void) const {
	if (size() == 0) {
		return false;
	}
	if ((*this)[0] == '=') {
		return true;
	} else {
		return false;
	}
}



//////////////////////////////
//
// HumdrumToken::isCommentLocal -- Presumed to be in a spine where
//   only local comments are allowed.
//

bool HumdrumToken::isCommentLocal(void) const {
	if (size() == 0) {
		return false;
	}
	if ((*this)[0] == '!') {
		if (size() > 1) {
			if ((*this)[1] == '!') {
				// global comment
				return false;
			}
		}
		return true;
	} else {
		return false;
	}
}



//////////////////////////////
//
// HumdrumToken::isData -- Returns true if not an interpretation, barline
//      or local comment;
//

bool HumdrumToken::isData(void) const {
	if (size() == 0) {
		return false;
	}
	int firstchar = (*this)[0];
	if ((firstchar == '*') || (firstchar == '!') || (firstchar == '=')) {
		return false;
	}
	return true;
}



//////////////////////////////
//
// HumdrumToken::isExclusive -- Returns true if first two characters
//     are "**".
//

bool HumdrumToken::isExclusive(void) const {
	const string& tok = (string)(*this);
	return tok.substr(0, 2) == "**";
}



//////////////////////////////
//
// HumdrumToken::isSplitInterpretation --
//

bool HumdrumToken::isSplitInterpretation(void) const {
	return ((string)(*this)) == SPLIT_TOKEN;
}



//////////////////////////////
//
// HumdrumToken::isMergeInterpretation --
//

bool HumdrumToken::isMergeInterpretation(void) const {
	return ((string)(*this)) == MERGE_TOKEN;
}



//////////////////////////////
//
// HumdrumToken::isExchangeInterpretation --
//

bool HumdrumToken::isExchangeInterpretation(void) const {
	return ((string)(*this)) == EXCHANGE_TOKEN;
}



//////////////////////////////
//
// HumdrumToken::isTerminateInterpretation --
//

bool HumdrumToken::isTerminateInterpretation(void) const {
	return ((string)(*this)) == TERMINATE_TOKEN;
}



//////////////////////////////
//
// HumdrumToken::isAddInterpretation --
//

bool HumdrumToken::isAddInterpretation(void) const {
	return ((string)(*this)) == ADD_TOKEN;
}



//////////////////////////////
//
// HumdrumToken::isNull -- Returns true if the token is a null token,
//   either for data, comments, or interpretations.  Does not consider
//   null global comments since they are not part of the spine structure.
//

bool HumdrumToken::isNull(void) const {
	const string& tok = (string)(*this);
	if (tok == NULL_DATA)           { return true; }
	if (tok == NULL_INTERPRETATION) { return true; }
	if (tok == NULL_COMMENT_LOCAL)  { return true; }
	return false;
}



//////////////////////////////
//
// HumdrumToken::getSubtrack -- Get the subtrack (similar to a layer
//    in MEI).
//

int HumdrumToken::getSubtrack(void) const {
	return address.getSubtrack();
}



//////////////////////////////
//
// HumdrumToken::getTrackString -- Get "track.subtrack" as a string.
//

string HumdrumToken::getTrackString(void) const {
	return address.getTrackString();
}



/////////////////////////////
//
// HumdrumToken::getSubtokenCount --
//   default value: separator = " "
//

int HumdrumToken::getSubtokenCount(const string& separator) const {
	int count = 0;
	string::size_type start = 0;
	while ((start = string::find(separator, start)) != string::npos) {
		count++;
		start += separator.size();
	}
	return count;
}



/////////////////////////////
//
// HumdrumToken::getSubtoken --
//   default value: separator = " "
//

string HumdrumToken::getSubtoken(int index, const string& separator) const {
	if (index < 0) {
		return "";
	}
	int count = 0;
	int start = 0;
	int end   = 0;
	while ((end = string::find(separator, start)) != string::npos) {
		count++;
		if (count == index) {
			return string::substr(start, end-start);
		}
		start += separator.size();
	}
	if (count == index) {
		return string::substr(start, string::size()-start);
	}
	return "";
}



//////////////////////////////
//
// HumdrumToken::setParameters -- Process a local comment with
//     the structure:
//        !NS1:NS2:key1=value1:key2=value2:key3=value3
//

void HumdrumToken::setParameters(HumdrumToken* ptok) {
	HumdrumToken& pl = *ptok;
	if (pl.size() <= 1) {
		return;
	}
	string pdata = pl.substr(1, pl.size()-1);
	setParameters(pdata);
}


void HumdrumToken::setParameters(const string& pdata) {
	vector<string> pieces = Convert::splitString(pdata, ':');
	if (pieces.size() < 3) {
		return;
	}
	string ns1 = pieces[0];
	string ns2 = pieces[1];
	string key;
	string value;
	int loc;
	for (int i=2; i<pieces.size(); i++) {
		Convert::replaceOccurrences(pieces[i], "&colon;", ":");
		loc = pieces[i].find("=");
		if (loc != string::npos) {
			key   = pieces[i].substr(0, loc);
			value = pieces[i].substr(loc+1, pieces[i].size());
		} else {
			key   = pieces[i];
			value = "true";
		}
		setValue(ns1, ns2, key, value);
	}
}



//////////////////////////////
//
// HumdrumToken::makeForwardLink --
//

void HumdrumToken::makeForwardLink(HumdrumToken& nextToken) {
	nextTokens.push_back(&nextToken);
	nextToken.previousTokens.push_back(this);
}



//////////////////////////////
//
// HumdrumToken::makeBackwarddLink --
//

void HumdrumToken::makeBackwardLink(HumdrumToken& previousToken) {
	previousTokens.push_back(&previousToken);
	previousToken.nextTokens.push_back(this);
}



//////////////////////////////
//
// HumdrumToken::setOwner --
//

void HumdrumToken::setOwner(HumdrumLine* aLine) {
	address.setOwner(aLine);
}



//////////////////////////////
//
// HumdrumToken::getOwner --
//

HumdrumLine* HumdrumToken::getOwner(void) const {
	return address.getOwner();
}



//////////////////////////////
//
// HumdrumToken::getState --
//

int HumdrumToken::getState(void) const {
	return rhycheck;
}



//////////////////////////////
//
// HumdrumToken::getState --
//

void HumdrumToken::incrementState(void) {
	rhycheck++;
}



//////////////////////////////
//
// HumdrumToken::getNextTokenCount -- Return the number of tokens in the
//   spine/subspine which follow this token.  Typically this will be 1,
//   but will be zero for a terminator interpretation (*-), and will be
//   2 for a split interpretation (*^).
//

int HumdrumToken::getNextTokenCount(void) const {
	return nextTokens.size();
}



//////////////////////////////
//
// HumdrumToken::getPreviousTokenCount -- Return the number of tokens
//   in the spine/subspine which precede this token.  Typically this will
//   be 1, but will be zero for an exclusive interpretation (starting with
//   "**"), and will be greater than one for a token which follows a
//   spine merger (using *v interpretations).
//

int HumdrumToken::getPreviousTokenCount(void) const {
	return previousTokens.size();
}



//////////////////////////////
//
// operator<< -- Needed to avoid interaction with HumHash.
//

ostream& operator<<(ostream& out, const HumdrumToken& token) {
	out << (string)token;
	return out;
}




//////////////////////////////
//
// Convert::recipToDuration -- Convert **recip rhythmic values into
//     rational number durations in terms of quarter toes.  For example "4"
//     will be converted to 1, "4." to 3/2 (1+1/2).  The second parameter
//     is a scaling factor which can change the rhythmic value's base duration.
//     Giving a scale of 1 will return the duration in whole note units, so
//     "4" will return a value of 1/4 (one quarter of a whole note).  Using
//     3/2 will give the duration in terms of dotted-quarter note units.
//     The third parameter is the subtoken separate.  For example if the input
//     string contains a space, anything after the first space will be ignored
//     when extracting the string.  **kern data which also includes the pitch
//     along with the rhythm can also be given and will be ignored.
//        default value: scale = 4 (duration in terms of quarter notes)
//        default value: separator = " " (subtoken separator)
//

HumNum Convert::recipToDuration(const string& recip, HumNum scale,
		string separator) {
	size_t loc;
	loc = recip.find(separator);
	string subtok;
	if (loc != string::npos) {
		subtok = recip.substr(0, loc);
	} else {
		subtok = recip;
	}

	loc = recip.find('q');
	if (loc != string::npos) {
		// grace note, ignore printed rhythm
		HumNum zero(0);
		return zero;
	}

	int dotcount = 0;
	int i;
	int numi = -1;
	for (i=0; i<subtok.size(); i++) {
		if (subtok[i] == '.') {
			dotcount++;
		}
		if ((numi < 0) && isdigit(subtok[i])) {
			numi = i;
		}
	}
	loc = subtok.find("%");
	int numerator = 1;
	int denominator = 1;
	HumNum output;
	if (loc != string::npos) {
		// reciprocal rhythm
		numerator = 1;
		denominator = subtok[numi++] - '0';
		while ((numi < subtok.size()) && isdigit(subtok[numi])) {
			denominator = denominator * 10 + (subtok[numi++] - '0');
		}
		if ((loc + 1 < subtok.size()) && isdigit(subtok[loc+1])) {
			int xi = loc + 1;
			numerator = subtok[xi++] - '0';
			while ((xi < subtok.size()) && isdigit(subtok[xi])) {
				numerator = numerator * 10 + (subtok[xi++] - '0');
			}
		}
		output.setValue(numerator, denominator);
	} else if (numi < 0) {
		// no rhythm found
		HumNum zero(0);
		return zero;
	} else if (subtok[numi] == '0') {
		// 0-symbol
		int zerocount = 1;
		for (i=numi+1; i<subtok.size(); i++) {
			if (subtok[i] == '0') {
				zerocount++;
			} else {
				break;
			}
		}
		numerator = (int)pow(2, zerocount);
		output.setValue(numerator, 1);
	} else {
		// plain rhythm
		denominator = subtok[numi++] - '0';
		while ((numi < subtok.size()) && isdigit(subtok[numi])) {
			denominator = denominator * 10 + (subtok[numi++] - '0');
		}
		output.setValue(1, denominator);
	}

	if (dotcount <= 0) {
		return output * scale;
	}

	int bot = (int)pow(2.0, dotcount);
	int top = (int)pow(2.0, dotcount + 1) - 1;
	HumNum factor(top, bot);
	return output * factor * scale;
}



//////////////////////////////
//
// Convert::replaceOccurrences -- Similar to s// regular expressions
//    operator.  This function replaces the search string in the source
//    string with the replace string.
//

void Convert::replaceOccurrences(string& source, const string& search,
		const string& replace) {
	for (int loc=0; ; loc += replace.size()) {
		loc = source.find(search, loc);
		if (loc == string::npos) {
			break;
		}
		source.erase(loc, search.length());
		source.insert(loc, replace);
	}
}



//////////////////////////////
//
// Convert::splitString -- split a string into a list of strings
//   separated by the given character.  Empty strings will be generated
//   if the separator occurs at the start/end of the input string, and
//   if two or more separates are adjacent to each other.
//

vector<string> Convert::splitString(const string& data, char separator) {
	stringstream ss(data);
	string key;
	vector<string> output;
	while (getline(ss, key, separator)) {
		output.push_back(key);
	}
	if (output.size() == 0) {
		output.push_back(data);
	}
	return output;
}


} // end namespace minHumdrum
