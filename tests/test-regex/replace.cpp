// $Smake: g++ -O3 -o %b %f && strip %b
// vim: ts=3
// References:
//     http://en.cppreference.com/w/cpp/regex/regex_replace

#include <iostream>
#include <string>
#include <regex>

using namespace std;

int main(void) {
 	string input;
 	regex vowel_re("[aeiou]");

	cout << "Type 'q' to quit." << endl;
 	while (true) {
 		cout << "Type text: ";
 		cin >> input;
 		if (!cin) break;
 		if (input=="q") {
 			break;
		}

		// Do a simple replace, putting square brackets around vowels.
		cout << regex_replace(input, vowel_re, "[$&]") << endl;

		// Send directly to ostream
		regex_replace(ostreambuf_iterator<char>(cout),
				input.begin(), input.end(), vowel_re, "*");
		cout << endl;
 	}
	return 0;
}


