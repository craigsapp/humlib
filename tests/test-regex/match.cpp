// $Smake: g++ -O3 -o %b %f && strip %b
// vim: ts=3
// References:
//     https://solarianprogrammer.com/2011/10/12/cpp-11-regex-tutorial
//     http://en.cppreference.com/w/cpp/regex/regex_match

#include <iostream>
#include <string>
#include <regex>

using namespace std;

int main(void) {
 	string input;
 	// regex integer("(\\+|-)?[[:digit:]]+");
 	regex integer("(\\+|-)?\\d+");

	cout << "Type 'q' to quit." << endl;
 	while (true) {
 		cout << "Type an integer: ";
 		cin >> input;
		cout << "You typed \"" << input << "\"" << endl;
 		if (!cin) break;
 		if (input=="q") {
 			break;
		}
		bool match = regex_match(input, integer);
 		if (regex_match(input, integer)) {
 			cout << "integer" << endl;
		} else {
 			cout << "Invalid input" << endl;
		}
 	}
	return 0;
}


