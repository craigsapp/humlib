//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Aug 31 05:59:33 PDT 2024
// Last Modified: Sat Aug 31 06:14:04 PDT 2024
// Filename:      cli/fixPolishChars.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/cli/fixPolishChars.cpp
// Syntax:        C++11
// vim:           ts=3 noexpandtab nowrap
//
// Description:   Fix double-encodings of UTF-8.
//

#include "HumRegex.h"
#include "Options.h"

#include <iostream>
#include <fstream>
#include <string>

using namespace std;
using namespace hum;

void   processContent (istream& infile);
string cleanText      (string& input);

bool changesQ = false;
bool inputQ   = false;
bool prefixQ  = false;
bool numberQ  = false;

int main(int argc, char** argv) {
	Options options;
	options.define("c|changes=b",       "output only changed lines");
	options.define("i|include-input=b", "display input line when displaying only changed lines");
	options.define("n|number=b",        "in -ci, prefix input and output lines numbers");
	options.define("p|prefix=b",        "for -cip option, prefix lines with 'INPUT:' and 'OUTPUT:'");
	options.process(argc, argv);
	changesQ = options.getBoolean("changes");
	inputQ   = options.getBoolean("include-input");
	numberQ  = options.getBoolean("number");
	prefixQ  = options.getBoolean("prefix");
	int filecount = options.getArgCount();
	if (filecount > 0) {
		for (int i=0; i<filecount; i++) {
			string filename = options.getArg(i+1);
			ifstream infile;
			infile.open(filename, ios::binary);
			if (!infile.is_open()) {
				cerr << "Could not open " << filename << ", skipping" << endl;
				continue;
			}
			processContent(infile);
		}
	} else {
		processContent(cin);
	}

	return 0;
}



//////////////////////////////
//
// processContent --
//

void processContent(istream& infile) {
	int lineNumber = 0;
	string line;
	while(std::getline(infile, line, '\n')) {
		lineNumber++;
		string newline = cleanText(line);
		if (changesQ) {
			if (newline != line) {
				if (inputQ) {
					cout << endl;
					if (prefixQ) {
						cout << "INPUT";
						if (numberQ) {
							cout << " " << lineNumber;
						}
						cout << ": ";
					}
					cout.write(line.data(), line.size());
					cout << endl;
					if (prefixQ) {
						cout << "OUTPUT";
						if (numberQ) {
							cout << " " << lineNumber;
						}
						cout << ": ";
					}
					cout .write(newline.data(), newline.size());
					cout << endl;
				} else {
					cout << newline << endl;
				}
			}
		} else {
			cout << newline << endl;
		}
	}
}



//////////////////////////////
//
// cleanText -- remove \x88 and \x98 bytes from string (should not affect UTF-8 encodings)
//     since those bytes do not seem to be involved with any UTF-8 characters.
//

string cleanText(std::string& input) {
	string output = input;
	HumRegex hre;

	// Fix UTF-8 double encodings (related to editing with Windows-1252 or ISO-8859-2 programs):

	// Żółty: c4 8f c5 bc cb 9d c4 8f c5 bc cb 9d 74 79 -> c5 bb c3 b3 c5 82 74 79
	hre.replaceDestructive(output, "=ABCDEFGHI=\xc5\xbb\xc3\xb3\xc5\x82\x74\x79=ABCDEFGHI=", "\xc4\x8f\xc5\xbc\xcb\x9d\xc4\x8f\xc5\xbc\xcb\x9d\x74\x79", "g");

	// Wąchocka: 57 c3 8b c2 87 63 68 6f 63 6b 61 -> 57 c4 85 63 68 6f 63 6b 61
	hre.replaceDestructive(output, "=ABCDEFGHI=\x57\xc4\x85\x63\x68\x6f\x63\x6b\61=ABCDEFGHI=", "\x57\xc3\x8b\xc2\x87\x63\x68\x6f\x63\x6b\x61", "g");

	// Żą: c4 b9 c4 bd c4 84 -> c5 bb c4 85
	hre.replaceDestructive(output, "=ABCDEFGHI=\xc5\xbb\xc4\x85=ABCDEFGHI=", "\xc4\xb9\xc4\xbd\xc4\x84", "g");

	// —: c3 a2 c2 80 c2 93 -> e2 80 94 (emdash)
	hre.replaceDestructive(output, "=ABCDEFGHI=\xe2\x80\x94=ABCDEFGHI=", "\xc3\xa2\xc2\x80\xc2\x93", "g");

	// …: c3 a2 c2 80 c5 9a -> e2 80 a6 (horizontal ellipsis, keep above Ś)
	hre.replaceDestructive(output, "=ABCDEFGHI=\xe2\x80\xa6=ABCDEFGHI=", "\xc3\xa2\xc2\x80\xc5\x9a", "g");

	// „: c3 a2 c2 80 c2 9c -> e2 80 9e (Polish open double quote)
	hre.replaceDestructive(output, "=ABCDEFGHI=\xe2\x80\x9e=ABCDEFGHI=", "\xc3\xa2\xc2\x80\xc2\x9c", "g");

	// ’: c3 a2 c2 80 c2 99 -> 27 (most likely modifier letter prime (ʹ: ca b9) but use UTF-7: ' (apostrophe));
	hre.replaceDestructive(output, "=ABCDEFGHI=\x27=ABCDEFGHI=", "\xc3\xa2\xc2\x80\xc2\x99", "g");

	// Ą: c3 84 c2 84 - c4 84
	hre.replaceDestructive(output, "=ABCDEFGHI=\xc4\x84=ABCDEFGHI=", "\xc3\x84\xc2\x84", "g");

	// ą: c3 b3 87 -> c4 85 (must be above ó)
	hre.replaceDestructive(output, "=ABCDEFGHI=\xc4\x85=ABCDEFGHI=", "\xc3\xb3\x87", "g");

	// ą: c3 84 c2 85 - c4 85
	hre.replaceDestructive(output, "=ABCDEFGHI=\xc4\x85=ABCDEFGHI=", "\xc3\x84\xc2\x85", "g");

	// Ć: c3 84 c2 86 -> c4 86
	hre.replaceDestructive(output, "=ABCDEFGHI=\xc4\x86=ABCDEFGHI=", "\xc3\x84\xc2\x86", "g");

	// ć: c3 84 c2 87 -> c4 87
	hre.replaceDestructive(output, "=ABCDEFGHI=\xc4\x87=ABCDEFGHI=", "\xc3\x84\xc2\x87", "g");

	// Ę: c3 84 c2 98 -> c4 98
	hre.replaceDestructive(output, "=ABCDEFGHI=\xc4\x98=ABCDEFGHI=", "\xc3\x84\xc2\x98", "g");

	// ę: c3 84 c2 99 -> c4 99
	hre.replaceDestructive(output, "=ABCDEFGHI=\xc4\x99=ABCDEFGHI=", "\xc3\x84\xc2\x99", "g");

	// ę: c3 82 c5 a0 -> c4 99
	hre.replaceDestructive(output, "=ABCDEFGHI=\xc4\x99=ABCDEFGHI=", "\xc3\x82\xc5\xa0", "g");

	// Ł: c4 b9 c2 81 -> c5 81
	hre.replaceDestructive(output, "=ABCDEFGHI=\xc5\x81=ABCDEFGHI=", "\xc4\xb9\xc2\x81", "g");

	// ó: c4 82 c5 82 -> c3 b3 (note: not sequential with Ó, keep about ł)
	hre.replaceDestructive(output, "=ABCDEFGHI=\xc3\xb3=ABCDEFGHI=", "\xc4\x82\xc5\x82", "g");

	// ó: c3 8b c2 -> c3 b3
	hre.replaceDestructive(output, "=ABCDEFGHI=\xc3\xb3=ABCDEFGHI=", "\xc3\x8b\xc2", "g");

	// ł: c4 b9 c2 82 -> c5 82
	hre.replaceDestructive(output, "=ABCDEFGHI=\xc5\x82=ABCDEFGHI=", "\xc4\xb9\xc2\x82", "g");

	// ł: c3 85 c2 82 -> c5 82
	hre.replaceDestructive(output, "=ABCDEFGHI=\xc5\x82=ABCDEFGHI=", "\xc3\x85\xc2\x82", "g");

	// ł: c4 b9 c2 92 -> c5 82 (strange mangling of ł?)
	hre.replaceDestructive(output, "=ABCDEFGHI=\xc5\x82=ABCDEFGHI=", "\xc4\xb9\xc2\x92", "g");

	// ł: c5 82 c3 82 c2 -> c5 82 (strange mangling of ł?)
	hre.replaceDestructive(output, "=ABCDEFGHI=\xc5\x82=ABCDEFGHI=", "\xc5\x82\xc3\x82\xc2", "g");

	// ł: c3 82 c2 c5 82 -> c5 82 (strange mangling of ł?)
	hre.replaceDestructive(output, "=ABCDEFGHI=\xc5\x82=ABCDEFGHI=", "\xc3\x82\xc2\xc5\x82", "g");

	// ł: c4 b9 c2 82 -> c5 82 (strange mangling of ł?)
	hre.replaceDestructive(output, "=ABCDEFGHI=\xc5\x82=ABCDEFGHI=", "\xc4\xb9\xc2\x82", "g");

	// Ń: c4 b9 c2 83 -> c5 83
	hre.replaceDestructive(output, "=ABCDEFGHI=\xc5\x83=ABCDEFGHI=", "\xc4\xb9\xc2\x83", "g");

	// ń: c4 b9 c2 84 -> c5 84
	hre.replaceDestructive(output, "=ABCDEFGHI=\xc5\x84=ABCDEFGHI=", "\xc4\xb9\xc2\x84", "g");

	// ń: c4 82 c2 a4 -> c5 84
	hre.replaceDestructive(output, "=ABCDEFGHI=\xc5\x84=ABCDEFGHI=", "\xc4\x82\xc2\xa4", "g");

	// Ó: c4 82 c5 93 -> c3 93 (note: not sequential with ó)
	hre.replaceDestructive(output, "=ABCDEFGHI=\xc3\x93=ABCDEFGHI=", "\xc4\x82\xc5\x93", "g");

	// Ó: c4 82 c2 93 -> c3 93
	hre.replaceDestructive(output, "=ABCDEFGHI=\xc3\x93=ABCDEFGHI=", "\xc4\x82\xc2\x93", "g");

	// Ź: c4 b9 c5 9a -> c5 b9 (keep above Ś)
	hre.replaceDestructive(output, "=ABCDEFGHI=\xc5\xb9=ABCDEFGHI=", "\xc4\xb9\xc5\x9a", "g");

	// Ś: c4 b9 c2 9a -> c5 9a (keep below Ź)
	hre.replaceDestructive(output, "=ABCDEFGHI=\xc5\x9a=ABCDEFGHI=", "\xc4\xb9\xc2\x9a", "g");

   // ś: c5 9b c3 82 c2 -> c5 9b
	hre.replaceDestructive(output, "=ABCDEFGHI=\xc5\x9b=ABCDEFGHI=", "\xc5\x9b\xc3\x82\xc2", "g");

	// ś: c4 b9 c2 9b -> c5 9b
	hre.replaceDestructive(output, "=ABCDEFGHI=\xc5\x9b=ABCDEFGHI=", "\xc4\xb9\xc2\x9b", "g");

	// ś: c3 82 c2 -> c5 9b
	hre.replaceDestructive(output, "=ABCDEFGHI=\xc5\x9b=ABCDEFGHI=", "\xc3\x82\xc2", "g");
	
	// ż:  c4 b9 c5 ba -> c5 bc (keep above ź)
	hre.replaceDestructive(output, "=ABCDEFGHI=\xc5\xbc=ABCDEFGHI=", "\xc4\xb9\xc5\xba", "g");

	// ż: c3 84 c5 be -> c5 bc
	hre.replaceDestructive(output, "=ABCDEFGHI=\xc5\xbc=ABCDEFGHI=", "\xc3\x84\xc5\xbe", "g");

	// ź: c4 b9 c5 9f -> c5 ba (keep below ź)
	hre.replaceDestructive(output, "=ABCDEFGHI=\xc5\xba=ABCDEFGHI=", "\xc4\xb9\xc5\x9f", "g");

	// ź: c3 82 c5 a4 -> c5 ba
	hre.replaceDestructive(output, "=ABCDEFGHI=\xc5\xba=ABCDEFGHI=", "\xc3\x82\xc5\xa4", "g");

	// Ż: c4 b9 c5 a5 -> c5 bb
	hre.replaceDestructive(output, "=ABCDEFGHI=\xc5\xbb=ABCDEFGHI=", "\xc4\xb9\xc5\xa5", "g");

	// Ż: c4 b9 c4 bd -> c5 bb
	hre.replaceDestructive(output, "=ABCDEFGHI=\xc5\xbb=ABCDEFGHI=", "\xc4\xb9\xc4\xbd", "g");

	// Ż: c4 b9 c5 a1 -> c5 bb
	hre.replaceDestructive(output, "=ABCDEFGHI=\xc5\xbb=ABCDEFGHI=", "\xc4\xb9\xc5\xa1", "g");

	// ”: c2 a2 c2 80 c2 9d -> e2 80 9d (Polish close double quote)
	hre.replaceDestructive(output, "=ABCDEFGHI=\xe2\x80\x9d=ABCDEFGHI=", "\xc3\xa2\xc2\x80\xc2\x9d", "g");

	// „: c3 a2 c2 80 c2 9e -> e2 80 9e  (Polish open double quote)
	hre.replaceDestructive(output, "=ABCDEFGHI=\xe2\x80\x9e=ABCDEFGHI=", "\xc3\xa2\xc2\x80\xc2\x9e", "g");

	// remove junk before DWOK: c4 8f c5 a5 bc
	hre.replaceDestructive(output, "=ABCDEFGHI==ABCDEFGHI=", "\xc4\x8f\xc5\xa5\xbc", "g");

	// remove junk before DWOK: c4 8f c5 a5 c5 bd
	hre.replaceDestructive(output, "=ABCDEFGHI==ABCDEFGHI=", "\xc4\x8f\xc5\xa5\xc5\xbd", "g");

	// strange bytes representing standard ASCII:
	// e: c4 82 c5 a0 -> 65
	hre.replaceDestructive(output, "=ABCDEFGHI=e=ABCDEFGHI=", "\xc4\x82\xc5\xa0", "g");

	// S: c3 a2 c2 80 c2 94 -> 53 (Sremu, genitive form of Śrem, town in Poland)
	hre.replaceDestructive(output, "=ABCDEFGHI=Sremu=ABCDEFGHI=", "\xc3\xa2\xc2\x80\xc2\x94remu", "g");

	// Ś: c3 a2 c2 80 c2 94 -> c5 9a
	hre.replaceDestructive(output, "=ABCDEFGHI=\xc5\x9a=ABCDEFGHI=", "\xc3\xa2\xc2\x80\xc2\x94", "g");

	// Ś: c4 b9 c2 9a -> c5 9a (keep below Ź)
	hre.replaceDestructive(output, "=ABCDEFGHI=Sremu=ABCDEFGHI=", "\xc3\xa2\xc2\x80\xc2\x94remu", "g");

	// Żół: c5 bb c3 b3 c5 82 9d -> c5 bb c3 b3 c5 82
	hre.replaceDestructive(output, "=ABCDEFGHI=\xc5\xbb\xc3\xb3\xc5\x82=ABCDEFGHI=", "\xc5\xbb\xc3\xb3\xc5\x92\x9d", "g");

	// Żół: c4 8f c5 bc cb 9d c4 8f c5 bc cb -> c5 bb c3 b3 c5 82
	hre.replaceDestructive(output, "=ABCDEFGHI=\xc5\xbb\xc3\xb3\xc5\x82=ABCDEFGHI=", "\xc4\x8f\xc5\xbc\xcb\x9d\xc4\x8f\xc5\xbc\xcb", "g");

	// There are some stubborn bytes, but cannot remove them in isolation since they could be
	// parts of UTF-8 characters:
	// hre.replaceDestructive(output, "", "\x87", "g");
	// hre.replaceDestructive(output, "", "\x9d", "g");

	// remove temporary segmenting stirng:
	hre.replaceDestructive(output, "", "=ABCDEFGHI=", "g");

	// random leftover characters from some character conversion:
	hre.replaceDestructive(output, "", "[\x88\x98]", "g");

	return output;
}



