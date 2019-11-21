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
#include <cctype>

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
			[](int c) {return !std::isspace(c);}));
	s.erase(std::find_if(s.rbegin(), s.rend(),
			[](int c) {return !std::isspace(c);}).base(), s.end());
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



//////////////////////////////
//
// Convert::extractIntegerList -- Convert a list such as 1-4 into the vector 1,2,3,4.
//   $ (or %) can be used to represent the maximum value, so if the input
//   is 1-$ (or 1-%), and the maximum should be 5, then the output will be a
//   vector 1,2,3,4,5.  In addition commas can be used to generate non-consecutive
//   sequences, and adding a number after the $/% sign means to subtract that
//   value from the maximum.  So if the string is 1,$-$2 and the maximum is 5,
//   then the vector will be 1,5,4,3.  Notice that ranges can be reversed to
//   place the sequence in reverse order, such as $-1 with a maximum of 5 will
//   result in the vector 5,4,3,2,1.  This function does not expect negative
//   values.
//

std::vector<int> Convert::extractIntegerList(const std::string& input, int maximum) {
	std::vector<int> output;
	if (maximum < 0) {
		maximum = 0;
	}
	if (maximum < 1000) {
		output.reserve(maximum);
	} else {
		output.reserve(1000);
	}
	HumRegex hre;
	string buffer = input;
	hre.replaceDestructive(buffer, "", "\\s", "gs");
	int start = 0;
	string tempstr;
	vector<int> tempdata;
	while (hre.search(buffer,  start, "^([^,]+,?)")) {
		tempdata.clear();
		processSegmentEntry(tempdata, hre.getMatch(1), maximum);
		start += hre.getMatchEndIndex(1);
		output.insert(output.end(), tempdata.begin(), tempdata.end());
	}
	return output;
}



//////////////////////////////
//
// Convert::processSegmentEntry --
//   3-6 expands to 3 4 5 6
//   $   expands to maximum file number
//   $-1 expands to maximum file number minus 1, etc.
//

void Convert::processSegmentEntry(vector<int>& field,
		const string& astring, int maximum) {

	HumRegex hre;
	string buffer = astring;

	// remove any comma left at end of input astring (or anywhere else)
	hre.replaceDestructive(buffer, "", ",", "g");

	// first remove $ symbols and replace with the correct values
	removeDollarsFromString(buffer, maximum);

	if (hre.search(buffer, "^(\\d+)-(\\d+)$")) {
		int firstone = hre.getMatchInt(1);
		int lastone  = hre.getMatchInt(2);

		if ((firstone < 1) && (firstone != 0)) {
			cerr << "Error: range token: \"" << astring << "\""
				  << " contains too small a number at start: " << firstone << endl;
			cerr << "Minimum number allowed is " << 1 << endl;
			return;
		}
		if ((lastone < 1) && (lastone != 0)) {
			cerr << "Error: range token: \"" << astring << "\""
				  << " contains too small a number at end: " << lastone << endl;
			cerr << "Minimum number allowed is " << 1 << endl;
			return;
		}
		if (firstone > maximum) {
			cerr << "Error: range token: \"" << astring << "\""
				  << " contains number too large at start: " << firstone << endl;
			cerr << "Maximum number allowed is " << maximum << endl;
			return;
		}
		if (lastone > maximum) {
			cerr << "Error: range token: \"" << astring << "\""
				  << " contains number too large at end: " << lastone << endl;
			cerr << "Maximum number allowed is " << maximum << endl;
			return;
		}

		if (firstone > lastone) {
			for (int i=firstone; i>=lastone; i--) {
				field.push_back(i);
			}
		} else {
			for (int i=firstone; i<=lastone; i++) {
				field.push_back(i);
			}
		}
	} else if (hre.search(buffer, "^(\\d+)")) {
		int value = hre.getMatchInt(1);
		if ((value < 1) && (value != 0)) {
			cerr << "Error: range token: \"" << astring << "\""
				  << " contains too small a number at end: " << value << endl;
			cerr << "Minimum number allowed is " << 1 << endl;
			return;
		}
		if (value > maximum) {
			cerr << "Error: range token: \"" << astring << "\""
				  << " contains number too large at start: " << value << endl;
			cerr << "Maximum number allowed is " << maximum << endl;
			return;
		}
		field.push_back(value);
	}
}



//////////////////////////////
//
// Convert::removeDollarsFromString -- substitute $ sign for maximum file count.
//

void Convert::removeDollarsFromString(string& buffer, int maximum) {
	HumRegex hre;
	string buf2 = to_string(maximum);
	if (hre.search(buffer, "[%$]$")) {
		hre.replaceDestructive(buffer, buf2, "[$%]$");
	} else if (hre.search(buffer, "[%$](?![\\d-])")) {
		// don't know how this case could happen, however...
		hre.replaceDestructive(buffer, buf2, "[%$](?![\\d-])", "g");
	} else if (hre.search(buffer, "[%$]$0")) {
		// replace $0 with maximum (used for reverse orderings)
		hre.replaceDestructive(buffer, buf2, "[%$]0", "g");
	} else if (hre.search(buffer, "^[%$]-")) {
		// replace $ with maximum at start of string
		hre.replaceDestructive(buffer, buf2, "^[%$]", "");
	}

	while (hre.search(buffer, "[%$](\\d+)")) {
		int value2 = maximum - abs(hre.getMatchInt(1));
		buf2 = to_string(value2);
		hre.replaceDestructive(buffer, buf2, "[%$]\\d+");
	}
}



// END_MERGE


// END_MERGE

} // end namespace hum



