//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sun Aug 16 01:35:04 PDT 2015
// Last Modified: Sun Aug 16 12:58:17 PDT 2015
// Filename:      HumHash.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/HumHash.cpp
// Syntax:        C++11
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Key/value parameters systems for Humdrum tokens, lines,
//                and files.  The HumHash class has a double namespace
//                capability. Parameters are encoded in local or global
//                comments.  Examples:
//                    !LO:N:vis=4
//                Namespace 1: LO (layout codes)
//                Namespace 2: N  (Note layout codes)
//                Key/Value  : vis=4, the "vis" key has the value "4"
//                Local parameters apply to the next non-null token in the
//                spine which follow them (data, measure and interpretation
//                tokens, but not local comment tokens).  For example to apply
//                the above example parameter to a token:
//                   **kern
//                   !LO:N:vis=1
//                   1c
//                   *-
//                 In this case the duration of the note is a whole note, but
//                 is should be displayed in graphical notation as a quarter
//                 note. If there are null data or interpretation tokens
//                 between the parameter and the note, the parameter is passed
//                 on to the next non-null token, such as:
//                   **kern         **kern
//                   1e             2g
//                   !LO:N:vis=1    !
//                   .              2a
//                   *              *clefG2
//                   1c             1g
//                   *-             *-
//                 In the above case the parameter is still applied to "1c".
//                 Namespace(s)+Keys must be unique, since including two
//                 parameters with the same namespace(s)/key will only
//                 accept one setting.  Only the value of the first
//                 duplicate parameter will be stored, and all duplicates
//                 after the first occurrence will be ignored.  For example:
//                   **kern
//                   !LO:N:vis=2
//                   !LO:N:vis=4
//                   1c
//                   *-
//                  will have the value LO:N:vis set to "2" for the "1c" token.
//                  Namespaces are optional and are indicated by an empty
//                  string.  For example, a parameter not stored in any
//                  namespace will have this form:
//                     !::vis=4
//                  To give only one namespace, the preferable form is:
//                     !:N:vis=4
//                  although this form can also be given:
//                     !N::vis=4
//                  where the second namespace is the empty string "".
//
//                  Multiple key values can be specified, each separated by
//                  a colon:
//                    !LO:N:vis=2:stem=5
//                  this can be expanded into two local comments:
//                    !LO:N:vis=2
//                    !LO:N:stem=5
//
//                  The namespaces and keys may not contain tabs (obviously),
//                  spaces or colons.  Preferably they will only contain
//                  letters, digits, and the underscore, but not start with
//                  a digit (but the humlib parser will not enforce
//                  this preference).  Values may contain spaces (but not
//                  tabs or colons.  If the value must include a colon it
//                  should be given as "&colon;" (without the quotes).
//
//                 Global comments affect all tokens on the next non-null
//                 line, and are similar to the above examples, but start
//                 with two exclamation marks:
//                   **kern         **kern
//                   1e             2g
//                   .              2a
//                   !!LO:N:vis=4
//                   1c             1g
//                   *-             *-
//                 This will apply the parameter to both "1c" and "1g" on the
//                 following line.  In the following case:
//                   **kern         **kern
//                   1e             2g
//                   !!LO:N:vis=4
//                   .              2a
//                   1c             1g
//                   *-             *-
//                  The parameter will apply to "1c", and "2a" rather than
//                  "1g". (Currently the parameter will only be applied to
//                  "2a", but this will be changed in the future).  Typically
//                  global parameters are used to apply parameters to all
//                  measures in all spines, or they may be used to display
//                  a single text string above or below the system in the
//                  full score (or part if it is extracted from the full
//                  score).
//

#include <string>
#include <sstream>
#include <iostream>

#include "HumHash.h"
#include "HumNum.h"
#include "Convert.h"
#include "HumdrumToken.h"

using namespace std;

namespace hum {

// START_MERGE


////////////////////////////////
//
// HumParameter::HumParameter -- HumParameter constructor.
//

HumParameter::HumParameter(void) {
	origin = NULL;
}


HumParameter::HumParameter(const string& str) : string(str) {
	origin = NULL;
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
// HumHash::getValueHTp -- Return an address of a HumdrumToken.
//   Presumes 64-bit pointers (or at least not 128-bit pointers).
//

HTp HumHash::getValueHTp(const string& key) const {
	if (parameters == NULL) {
		return NULL;
	}
	vector<string> keys = getKeyList(key);
	if (keys.size() == 1) {
		return getValueHTp("", "", keys[2]);
	} else if (keys.size() == 2) {
		return getValueHTp(keys[0], keys[1]);
	} else {
		return getValueHTp(keys[0], keys[1], keys[2]);
	}
}


HTp HumHash::getValueHTp(const string& ns2, const string& key) const {
	if (parameters == NULL) {
		return NULL;
	}
	return getValueHTp("", ns2, key);
}


HTp HumHash::getValueHTp(const string& ns1, const string& ns2,
		const string& key) const {
	if (parameters == NULL) {
		return NULL;
	}
	string value = getValue(ns1, ns2, key);
	if (value.find("HT_") != 0) {
		return NULL;
	} else {
		HTp pointer = NULL;
		try {
			pointer = (HTp)(stoll(value.substr(3)));
		} catch (invalid_argument& e) {
         std::cerr << e.what() << std::endl;
			pointer = NULL;
		}
		return pointer;
	}
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
			// problem with emscripten with stoi:
			// intvalue = stoi(value);
			stringstream converter(value);
			if (!(converter >> intvalue)) {
				intvalue = 0;
			}
		} catch (invalid_argument& e) {
         std::cerr << e.what() << std::endl;
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
         std::cerr << e.what() << std::endl;
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


void HumHash::setValue(const string& key, const char* value) {
	setValue(key, (string)value);
}


void HumHash::setValue(const string& ns2, const string& key,
		const char* value) {
	setValue(ns2, key, (string)value);
}


void HumHash::setValue(const string& ns1, const string& ns2, const string& key,
		const char* value) {
	setValue(ns1, ns2, key, (string)value);
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
	stringstream ss;
	ss << value;
	(*parameters)[ns1][ns2][key] = ss.str();
}


void HumHash::setValue(const string& key, HTp value) {
	vector<string> keys = getKeyList(key);
	if (keys.size() == 1) {
		setValue("", "", keys[0], value);
	} else if (keys.size() == 2) {
		setValue("", keys[0], keys[1], value);
	} else {
		setValue(keys[0], keys[1], keys[2], value);
	}
}


void HumHash::setValue(const string& ns2, const string& key, HTp value) {
		setValue("", ns2, key, value);
}


void HumHash::setValue(const string& ns1, const string& ns2,
		const string& key, HTp value) {
	initializeParameters();
	stringstream ss;
	ss << "HT_" << ((long long)value);
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
	auto loc = ns.find(":");
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
	auto loc = ns.find(":");
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
	return (int)it2->second.size();
}


int HumHash::getParameterCount(const string& ns) const {
	if (parameters == NULL) {
		return false;
	}
	auto loc = ns.find(":");
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
// HumHash::setPrefix -- initial string to print when using
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
// HumHash::getPrefix -- get the prefix.
//

string HumHash::getPrefix(void) const {
	return prefix;
}



//////////////////////////////
//
// HumHash::setOrigin -- Set the source token for the parameter.
//

void HumHash::setOrigin(const string& key, HumdrumToken* tok) {
	if (parameters == NULL) {
		return;
	} else {
		vector<string> keys = getKeyList(key);
		if (keys.size() == 1) {
			setOrigin("", "", keys[0], tok);
		} else if (keys.size() == 2) {
			setOrigin("", keys[0], keys[1], tok);
		} else {
			setOrigin(keys[0], keys[1], keys[2], tok);
		}
	}
}


void HumHash::setOrigin(const string& key, HumdrumToken& tok) {
	setOrigin(key, &tok);
}


void HumHash::setOrigin(const string& ns2, const string& key,
		HumdrumToken* tok) {
	if (parameters == NULL) {
		return;
	} else {
		setOrigin("", ns2, key, tok);
	}
}


void HumHash::setOrigin(const string& ns2, const string& key,
		HumdrumToken& tok) {
	setOrigin(ns2, key, &tok);
}


void HumHash::setOrigin(const string& ns1, const string& ns2,
		const string& key, HumdrumToken* tok) {
	if (parameters == NULL) {
		return;
	}
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
	it3->second.origin = tok;
}


void HumHash::setOrigin(const string& ns1, const string& ns2,
		const string& key, HumdrumToken& tok) {
	setOrigin(ns1, ns2, key, &tok);
}



//////////////////////////////
//
// HumHash::getOrigin -- Get the source token for the parameter.
//    Returns NULL if there is no origin.
//

HumdrumToken* HumHash::getOrigin(const string& key) const {
	if (parameters == NULL) {
		return NULL;
	} else {
		vector<string> keys = getKeyList(key);
		if (keys.size() == 1) {
			return getOrigin("", "", keys[0]);
		} else if (keys.size() == 2) {
			return getOrigin("", keys[0], keys[1]);
		} else {
			return getOrigin(keys[0], keys[1], keys[2]);
		}
	}
}


HumdrumToken* HumHash::getOrigin(const string& ns2, const string& key) const {
	if (parameters == NULL) {
		return NULL;
	} else {
		return getOrigin("", ns2, key);
	}
}


HumdrumToken* HumHash::getOrigin(const string& ns1, const string& ns2,
		const string& key) const {
	if (parameters == NULL) {
		return NULL;
	}
	MapNNKV& p = *parameters;
	auto it1 = p.find(ns1);
	if (it1 == p.end()) {
		return NULL;
	}
	auto it2 = it1->second.find(ns2);
	if (it2 == it1->second.end()) {
		return NULL;
	}
	auto it3 = it2->second.find(key);
	if (it3 == it2->second.end()) {
		return NULL;
	}
	return it3->second.origin;
}



//////////////////////////////
//
// HumHash::printXml -- Print object as a <parameters> element for
//     in a HumdrumXML file.
//

ostream& HumHash::printXml(ostream& out, int level, const string& indent) {

	if (parameters == NULL) {
		return out;
	}
	if (parameters->size() == 0) {
		return out;
	}
	
	stringstream str;
	bool found = 0;

	HumdrumToken* ref = NULL;
	level++;
	for (auto& it1 : *(parameters)) {
		if (it1.second.size() == 0) {
			continue;
		}
		if (!found) {
			found = 1;
		}
		str << Convert::repeatString(indent, level++);
		str << "<namespace n=\"1\" name=\"" << it1.first << "\">\n";
		for (auto& it2 : it1.second) {
			if (it2.second.size() == 0) {
				continue;
			}

			str << Convert::repeatString(indent, level++);
			str << "<namespace n=\"2\" name=\"" << it2.first << "\">\n";

			for (auto& it3 : it2.second) {
				str << Convert::repeatString(indent, level);
				str << "<parameter key=\"" << it3.first << "\"";
				str << " value=\"";
				str << Convert::encodeXml(it3.second) << "\"";
				ref = it3.second.origin;
				if (ref != NULL) {
					str << " idref=\"";
					str << ref->getXmlId();
					str << "\"";
				}
				str << "/>\n";
			}
			str << Convert::repeatString(indent, --level) << "</namespace>\n";
		}
		str << Convert::repeatString(indent, --level) << "</namespace>\n";
	}
	if (found) {
		str << Convert::repeatString(indent, --level) << "</parameters>\n";
		out << Convert::repeatString(indent, level) << "<parameters>\n";
		out << str.str();
	}

	return out;

}



//////////////////////////////
//
// HumHash::printXmlAsGlobal --
//

ostream& HumHash::printXmlAsGlobal(ostream& out, int level,
		const string& indent) {

	if (parameters == NULL) {
		return out;
	}
	if (parameters->size() == 0) {
		return out;
	}
	
	stringstream str;
	stringstream str2;
	string it1str;
	string it2str;
	int str2count = 0;
	bool found = 0;

	HumdrumToken* ref = NULL;
	level++;
	for (auto& it1 : *(parameters)) {
		if (it1.second.size() == 0) {
			continue;
		}
		str2.str("");
		it1str = it1.first;
		if (!found) {
			found = 1;
		}
		if (it1.first == "") {
			str2 << Convert::repeatString(indent, level++);
			str2 << "<namespace n=\"1\" name=\"" << it1.first << "\">\n";
		} else {
			str << Convert::repeatString(indent, level++);
			str << "<namespace n=\"1\" name=\"" << it1.first << "\">\n";
		}
		for (auto& it2 : it1.second) {
			if (it2.second.size() == 0) {
				continue;
			}
			it2str = it2.first;

			if ((it2.first == "") && (it2.first == "")) {
				str2 << Convert::repeatString(indent, level++);
				str2 << "<namespace n=\"2\" name=\"" << it2.first << "\">\n";
			} else {
				str << Convert::repeatString(indent, level++);
				str << "<namespace n=\"2\" name=\"" << it2.first << "\">\n";
			}

			for (auto& it3 : it2.second) {
				if ((it2.first == "") && (it2.first == "")) {

					if ((it3.first == "global") && (it3.second == "true")) {
						// don't do anything because parameter should be removed
					} else {
						str2count++;
						str2 << Convert::repeatString(indent, level);
						str2 << "<parameter key=\"" << it3.first << "\"";
						str2 << " value=\"";
						str2 << Convert::encodeXml(it3.second) << "\"";
						ref = it3.second.origin;
						if (ref != NULL) {
							str2 << " idref=\"";
							str2 << ref->getXmlId();
							str2 << "\"";
						}
						str2 << "/>\n";
					}
				} else {
					str << Convert::repeatString(indent, level);
					str << "<parameter key=\"" << it3.first << "\"";
					str << " value=\"";
					str << Convert::encodeXml(it3.second) << "\"";
					ref = it3.second.origin;
					if (ref != NULL) {
						str << " idref=\"";
						str << ref->getXmlId();
						str << "\"";
					}
					str << "/>\n";
				}
			}
			if ((it1str == "") && (it2str == "")) {
				if (str2count > 0) {
					str << str2.str();
					str << Convert::repeatString(indent, --level) << "</namespace>\n";
				}
			} else {
				str << Convert::repeatString(indent, --level) << "</namespace>\n";
			}
		}
		if ((it1str == "") && (it2str == "")) {
			if (str2count > 0) {
				str << Convert::repeatString(indent, --level) << "</namespace>\n";
			}
		} else {
			str << Convert::repeatString(indent, --level) << "</namespace>\n";
		}
	}
	if (found) {
		str << Convert::repeatString(indent, --level) << "</parameters>\n";
		out << Convert::repeatString(indent, level) << "<parameters global=\"true\">\n";
		out << str.str();
	}

	return out;
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


// END_MERGE

} // end namespace hum



