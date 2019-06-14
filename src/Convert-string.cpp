//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Aug  8 12:24:49 PDT 2015
// Last Modified: Sun Aug  9 21:03:12 PDT 2015
// Filename:      Convert-string.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/Convert-string.cpp
// Syntax:        C++11; humlib
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Conversions related to strings.
//

#include "Convert.h"
#include "HumRegex.h"

#include <algorithm>
#include <sstream>
#include <functional>

using namespace std;

namespace hum {

// START_MERGE


//////////////////////////////
//
// Convert::replaceOccurrences -- Similar to s// regular expressions
//    operator.  This function replaces the search string in the source
//    string with the replace string.
//

void Convert::replaceOccurrences(string& source, const string& search,
		const string& replace) {
	for (int loc=0; ; loc += (int)replace.size()) {
		loc = (int)source.find(search, loc);
		if (loc == (int)string::npos) {
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
	for (int i=0; i<(int)input.size(); i++) {
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



//////////////////////////////
//
// Convert::getHumNumAttributes -- Returns XML attributes for a HumNum
//   number.  First @float which gives the floating-point representation.
//   If the number has a fractional part, then also add @ratfrac with the
//   fractional representation of the non-integer portion number.
//

string Convert::getHumNumAttributes(const HumNum& num) {
	string output;
	if (num.isInteger()) {
		output += " float=\"" + to_string(num.getNumerator()) + "\"";
	} else {
		stringstream sstr;
		sstr << num.toFloat();
		output += " float=\"" + sstr.str() + "\"";
	}
	if (!num.isInteger()) {
		HumNum rem = num.getRemainder();
		output += " ratfrac=\"" + to_string(rem.getNumerator()) +
				+ "/" + to_string(rem.getDenominator()) + "\"";
	}
	return output;
}



//////////////////////////////
//
// Convert::trimWhiteSpace -- remove spaces, tabs and/or newlines
//     from the beginning and end of input string.
//

string Convert::trimWhiteSpace(const string& input) {
	string s = input;
	s.erase(s.begin(), std::find_if(s.begin(), s.end(),
			std::not1(std::ptr_fun<int, int>(isspace))));
	s.erase(std::find_if(s.rbegin(), s.rend(),
			std::not1(std::ptr_fun<int, int>(isspace))).base(), s.end());
	return s;
}



//////////////////////////////
//
// Convert::startsWith --
//

bool Convert::startsWith(const string& input, const string& searchstring) {
	return input.compare(0, searchstring.size(), searchstring) == 0;
}


/////////////////////////////
//
// Convert::contains -- Returns true if the character or string
//    is found in the string.
//

bool Convert::contains(const string& input, const string& pattern) {
	return input.find(pattern) != string::npos;
}

bool Convert::contains(const string& input, char pattern) {
	return input.find(pattern) != string::npos;
}

bool Convert::contains(string* input, const string& pattern) {
	return Convert::contains(*input, pattern);
}

bool Convert::contains(string* input, char pattern) {
	return Convert::contains(*input, pattern);
}


//////////////////////////////
//
// Convert::makeBooleanTrackList -- Given a string
//   such as "1,2,3" and a max track of 5, then
//   create a vector with contents:
//      0:false, 1:true, 2:true, 3:true, 4:false, 5:false.
//   The 0 track is not used, and the two tracks not specified
//   in the string are set to false.  Special abbreviations:
//     $ = maxtrack
//     $1 = maxtrack - 1
//     $2 = maxtrack - 2
//     etc.
//   Ranges can be given, such as 1-3 instead of 1,2,3
//

void Convert::makeBooleanTrackList(vector<bool>& spinelist,
		 const string& spinestring, int maxtrack) {
   spinelist.resize(maxtrack+1);

	if (spinestring.size() == 0) {
		fill(spinelist.begin()+1, spinelist.end(), true);
		return;
	}
	fill(spinelist.begin(), spinelist.end(), false);

   string buffer = spinestring;;
	vector<string> entries;
	string separator = "[^\\d\\$-]+";
   HumRegex hre;

	// create an initial list of values:
	hre.split(entries, buffer, separator);

	// Now process each token in the extracted list:
	int val = -1;
	int val2 = -1;
	bool range = false;
	string tbuff;
	for (int i=0; i<(int)entries.size(); i++) {

		if (hre.search(entries[i], "\\$(\\d*)")) {
			if (hre.getMatch(1).size() == 0) {
				tbuff = to_string(maxtrack);
			} else {
				val = hre.getMatchInt(1);
				tbuff = to_string(maxtrack - val);
			}
			hre.replaceDestructive(entries[i], tbuff, "\\$\\d+");
		}

		range = false;
		if (entries[i].find('-') != string::npos) {
			range = true;
			// check for second $ abbreviation at end of range:
			if (hre.search(entries[i], "\\$(\\d*)")) {
				if (hre.getMatch(1).size() == 0) {
					tbuff = to_string(maxtrack);
				} else {
					val = hre.getMatchInt(1);
					tbuff = to_string(maxtrack - val);
				}
				hre.replaceDestructive(entries[i], tbuff, "\\$\\d+");
			}
			if (entries[i].back() == '$') {
				entries[i].pop_back();
				entries[i] += to_string(maxtrack);
			}
			// extract second vlaue
			if (hre.search(entries[i], "-(\\d+)")) {
				val2 = hre.getMatchInt(1);
			} else {
				range = false;
			}
		}


		// get first value:
		if (hre.search(entries[i], "(\\d+)")) {
			val = stoi(hre.getMatch(1));
		}
		if (range) {
			int direction = 1;
			if (val > val2) {
				direction = -1;
			}
			for (int j=val; j != val2; j += direction) {
				if ((j > 0) && (j < maxtrack + 1)) {
					spinelist[j] = true;
				}
			}
			if ((val2 > 0) && (val2 < maxtrack + 1)) {
				spinelist[val2] = true;
			}
		} else {
			// not a range
			if ((val > 0) && (val < maxtrack+1)) {
				spinelist[val] = true;
			}
		}
	}
}


// END_MERGE

} // end namespace hum



