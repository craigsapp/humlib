//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Fri Aug 25 18:04:41 PDT 2017
// Last Modified: Fri Aug 25 18:04:44 PDT 2017
// Filename:      HumParamSet.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/HumParamSet.cpp
// Syntax:        C++11; humlib
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Set of parameters, specifically for Layout codes.
//                The HumParamSet class has a double namespace capability.
//                Parameters are encoded in local or global comments.
//                Examples:
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
//                  spaces or colons.  Preferrably they will only contain
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
//                  "2a", but this will be canged in the future).  Typically
//                  global parameters are used to apply parameters to all
//                  measures in all spines, or they may be used to display
//                  a single text string above or below the system in the
//                  full score (or part if it is extracted from the full
//                  score).
//


#include "HumParamSet.h"
#include "Convert.h"

using namespace std;

namespace hum {

// START_MERGE


//////////////////////////////
//
// HumParamSet::HumParamSet --
//

HumParamSet::HumParamSet(void) {
	// do nothing
}

HumParamSet::HumParamSet(const string& token) {
	readString(token);
}

HumParamSet::HumParamSet(HTp token) {
	readString(*((string*)token));
}



//////////////////////////////
//
// HumParamSet::~HumParamSet --
//

HumParamSet::~HumParamSet() {
	clear();
}


//////////////////////////////
//
// HumParamSet::getNamespace1 --
//

const string& HumParamSet::getNamespace1(void) {
	return m_ns1;
}



//////////////////////////////
//
// HumParamSet::getNamespace2 --
//

const string& HumParamSet::getNamespace2(void) {
	return m_ns2;
}



//////////////////////////////
//
// HumParamSet::getNamespace --
//

string HumParamSet::getNamespace(void) {
	return m_ns1 + ":" + m_ns2;
}



//////////////////////////////
//
// HumParamSet::setNamespace1 --
//

void HumParamSet::setNamespace1(const string& name) {
	m_ns1 = name;
}



//////////////////////////////
//
// HumParamSet::setNamespace2 --
//

void HumParamSet::setNamespace2(const string& name) {
	m_ns2 = name;
}



//////////////////////////////
//
// HumParamSet::setNamespace --
//

void HumParamSet::setNamespace(const string& name) {
	auto loc = name.find(':');
	if (loc == string::npos) {
		m_ns1 = "";
		m_ns2 = name;
	} else {
		m_ns1 = name.substr(0, loc);
		m_ns2 = name.substr(loc+1, string::npos);
	}
}



//////////////////////////////
//
// HumParamSet::setNamespace --
//

void HumParamSet::setNamespace(const string& name1, const string& name2) {
	m_ns1 = name1;
	m_ns2 = name2;
}



//////////////////////////////
//
// HumParamSet::getCount --
//

int HumParamSet::getCount(void) {
	return (int)m_parameters.size();
}



//////////////////////////////
//
// HumParamSet::getParameterName --
//

const string& HumParamSet::getParameterName(int index) {
	return m_parameters.at(index).first;
}



//////////////////////////////
//
// HumParamSet::getParameterValue --
//

const string& HumParamSet::getParameterValue(int index) {
	return m_parameters.at(index).second;
}



//////////////////////////////
//
// HumParamSet::addParameter --
//

int HumParamSet::addParameter(const string& name, const string& value) {
	m_parameters.push_back(make_pair(name, value));
	return (int)m_parameters.size() - 1;
}



//////////////////////////////
//
// HumParamSet::setParameter --
//

int HumParamSet::setParameter(const string& name, const string& value) {
	for (int i=0; i<(int)m_parameters.size(); i++) {
		if (m_parameters[i].first == name) {
			m_parameters[i].second = value;
			return i;
		}
	}
	// Parameter does not exist so create at end of list.
	m_parameters.push_back(make_pair(name, value));
	return (int)m_parameters.size() - 1;
}



//////////////////////////////
//
// HumParamSet::clear --
//

void HumParamSet::clear(void) {
	m_ns1.clear();
	m_ns2.clear();
	m_parameters.clear();
}



//////////////////////////////
//
// HumParamSet::readString --
//

void HumParamSet::readString(const string& text) {
	vector<string> pieces(1);
	bool bangs = true;
	for (int i=0; i<(int)text.size(); i++) {
		if (bangs && text[i] == '!') {
			continue;
		}
		bangs = false;
		if (text[i] == ':') {
			pieces.resize(pieces.size() + 1);
			continue;
		}
		pieces.back() += text[i];
	}

	if (pieces.size() < 3) {
		// not enough information
		return;
	}

	m_ns1 = pieces[0];
	m_ns2 = pieces[1];

	string key;
	string value;
	int loc;
	for (int i=2; i<(int)pieces.size(); i++) {
		Convert::replaceOccurrences(pieces[i], "&colon;", ":");
		loc = (int)pieces[i].find("=");
		if (loc != (int)string::npos) {
			key   = pieces[i].substr(0, loc);
			value = pieces[i].substr(loc+1, pieces[i].size());
		} else {
			key   = pieces[i];
			value = "true";
		}
		addParameter(key, value);
	}
}



//////////////////////////////
//
// HumParamSet::printXml --
//

ostream& HumParamSet::printXml(ostream& out, int level,
		const string& indent) {

	if (getCount() == 0) {
		return out;
	}

	out << Convert::repeatString(indent, level++) << "<linked-parameter-set>\n";
	out << Convert::repeatString(indent, level++);
	out << "<namespace n=\"1\" name=\"" << getNamespace1() << "\">\n";
	out << Convert::repeatString(indent, level++);
	out << "<namespace n=\"2\" name=\"" << getNamespace2() << "\">\n";

	for (int i=0; i<getCount(); i++) {
		out << Convert::repeatString(indent, level);
		out << "<parameter key=\"" << getParameterName(i) << "\"";
		out << " value=\"";
		out << Convert::encodeXml(getParameterValue(i)) << "\"";
		out << "/>\n";
	}

	out << Convert::repeatString(indent, --level) << "</namespace>\n";
	out << Convert::repeatString(indent, --level) << "</namespace>\n";
	out << Convert::repeatString(indent, --level) << "<linked-parameter-set>\n";
	return out;
}



//////////////////////////////
//
// operator<< -- print HumParamSetData as a layout command
//

ostream& operator<<(ostream& out, HumParamSet* hps) {
	out << *hps;
	return out;
}


ostream& operator<<(ostream& out, HumParamSet& hps) {
	out << hps.getNamespace();
	int count = hps.getCount();
	for (int i=0; i<count; i++) {
		out << ":" << hps.getParameterName(i) << "=";
		// should colon-escape the following line's output:
		out << "=" << hps.getParameterValue(i);
	}
	return out;
}


// END_MERGE

} // end namespace hum



