//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Aug  8 12:24:49 PDT 2015
// Last Modified: Sun Aug  9 21:03:12 PDT 2015
// Filename:      Convert-string.h
// URL:           https://github.com/craigsapp/minHumdrum/blob/master/src/Convert-string.h
// Syntax:        C++11
// vim:           ts=3 noexpandtab
//
// Description:   Conversions related to strings.
//

#include <sstream>

#include "Convert.h"

namespace minHumdrum {

// START_MERGE


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
// Convert::splitString -- Splits a string into a list of strings
//   separated by the given character.  Empty strings will be generated
//   if the separator occurs at the start/end of the input string, and
//   if two or more separates are adjacent to each other.
// default value: separator = ' ';
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



//////////////////////////////
//
// Convert::repeatString -- Returns a string which repeats the given
//   pattern by the given count.
//

string Convert::repeatString(const string& pattern, int count) {
	string output;
	for (int i=0; i<count; i++) {
		output += pattern;
	}
	return output;
}


//////////////////////////////
//
// Convert::encodeXml -- Encode a string for XML printing.  Ampersands
//    get converted to &amp;, < to &lt; > to &gt;, " to &quot; and 
//    ' to &apos;.
//

string Convert::encodeXml(const string& input) {
	string output;
	output.reserve(input.size()*2);
	for (int i=0; i<input.size(); i++) {
		switch (input[i]) {
			case '&':  output += "&amp;";   break;
			case '<':  output += "&lt;";    break;
			case '>':  output += "&gt;";    break;
			case '"':  output += "&quot;";  break;
			case '\'': output += "&apos;";  break;
			default:   output += input[i];
		}
	}
	return output;
}



// END_MERGE

} // end namespace std;



