//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sun Apr  9 22:43:34 PDT 2023
// Last Modified: Mon Apr 10 13:02:03 PDT 2023
// Filename:      cli/mvsic.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/cli/mvsic.cpp
// Syntax:        C++11
// vim:           ts=3 noexpandtab nowrap
//
// Description:   Move !SIC:l to the proper **text columns.
//                Related to issue https://github.com/craigsapp/website-polish-scores/issues/170
//

#include "humlib.h"
#include <iostream>

using namespace hum;
using namespace std;

void   processFile         (HumdrumFile& infile);
void   processLocalComment (HumdrumFile& infile, int line);
void   moveSic             (HumdrumFile& infile, int line, int field, int verse);
string cleanSicText        (const string& input);

int sicLyricListQ;
int sicAllListQ;
int plainQ;
int verseQ;
int cleanQ;
int errorQ;
int verboseQ;

///////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv) {
	Options options;
	options.define("l|sic-lyric-list=b", "list all LO:SIC:l entries");
	options.define("L|sic-all-list=b", "list all LO:SIC entries");
	options.define("c|clean=b", "clean identified LO:SIC entries for -l option");
	options.define("v|verse=b", "display extracted verse number for -l option");
	options.define("V|verbose=b", "verbose messages");
	options.define("e|error|errors-only=b", "display errors without writing");
	options.process(argc, argv);
	verseQ        = options.getBoolean("verse");
	cleanQ        = options.getBoolean("clean");
	errorQ        = options.getBoolean("errors-only");
	verboseQ      = options.getBoolean("verbose");
	sicLyricListQ = options.getBoolean("sic-lyric-list");
	sicAllListQ   = options.getBoolean("sic-all-list");

	if (sicAllListQ) {
		sicLyricListQ = false;
	}
	plainQ = !(sicLyricListQ || sicAllListQ || errorQ);

	HumdrumFileStream instream(options);
	HumdrumFile infile;
	while (instream.read(infile)) {
		processFile(infile);
		if (plainQ) {
			infile.generateLinesFromTokens();
			cout << infile;
		}
	}
	return 0;
}



//////////////////////////////
//
// processFile --
//

void processFile(HumdrumFile& infile) {
	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isCommentLocal()) {
			continue;
		}
		processLocalComment(infile, i);
	}
}



//////////////////////////////
//
// processLocalComment --
//

void processLocalComment(HumdrumFile& infile, int line) {
	if (!infile[line].isCommentLocal()) {
		return;
	}
	HumRegex hre;
	for (int j=0; j<infile[line].getFieldCount(); j++) {
		HTp token = infile[line].token(j);
		if (!token->isKern()) {
			continue;
		}
		if (sicAllListQ) {
			if (hre.search(token, "^!LO:SIC:")) {
				cout << token << endl;
				continue;
			}
		}
		if (!hre.search(token, "^!LO:SIC(?=:).*:l(:|$)")) {
			continue;
		}
		int verse = 1;
		if (hre.search(token, ":verse\\s*(\\d+)")) {
			// deal with invalid cases such as:
			//  !LO:SIC:v:l:verse 1:s=-tam
			verse = hre.getMatchInt(1);
		} else if (hre.search(token, ":verse\\s*:\\s*(\\d+)")) {
			// deal with invalid cases such as:
			//  !LO:SIC:v:l:verse1:s=-tam
			verse = hre.getMatchInt(1);
		} else if (hre.search(token, ":verse\\s*=\\s*(\\d+)")) {
			// deal with invalid cases such as:
			//   !LO:SIC:v:l:s=blan:verse=2-
			verse = hre.getMatchInt(1);
		} else if (hre.search(token, ":v\\s*=\\s*(\\d+)")) {
			// deal with invalid cases such as:
			//   !LO:SIC:v:l:v=2:s=tri-
			verse = hre.getMatchInt(1);
		} else if (hre.search(token, ":(\\d+)")) {
			// deal with implicit verse number
			//   !LO:SIC:v:l:s=ти-:4
			verse = hre.getMatchInt(1);
		} else if (hre.search(token, "verse\\s*(\\d+)")) {
			// deal with improper enjambments of verse parameter:
			//    !LO:SIC:v:l:s=-vi-verse 2
			verse = hre.getMatchInt(1);
		}
		if (sicLyricListQ) {
			if (verseQ) {
				cout << verse << "\t";
			}
			cout << token;
			string cleantext = cleanSicText(*token);
			if (cleanQ && (cleantext != *token)) {
				cout << "\t\tCLEANED:\t";
				cout << cleantext;
			}
			cout << endl;
			continue;
		}
		moveSic(infile, line, j, verse);
	}
}



//////////////////////////////
//
// moveSic --
//
// Invalid cases that need clean up:
//     !LO:SIC:l:s:v=-rae    SHOULD BE !LO:SIC:l:v:s=-rae
//     !LO:SIC:l:s= e-       SHOULD BE !LO:SIC:l:s=e-
//     !LO:SIC:v:l:s=-cli-:2 SHOULD BE !LO:SIC:v:l:s=cli-:verse 2
//     !LO:SIC:v:l:v:s=cae   SHOULD BE !LO:SIC:v:l:s=cae
//
// Invalid cases that need to be cleaned up manually:
//    Doubled SIC:
//		    !LO:SIC:l:v:s=sae-LO:SIC:l:v:s=sae-
//    Compound verse:
//         !LO:SIC:v:l:s=-i-:verse 1, verse 2
//    Not a SIC (needs s= parameter:
//         !LO:SIC:v:l:t=custos słowny cu
//    Empty substitution:
//         !LO:SIC:v:s:l:t=za dużo sylab w stosunku do nut
//    Doubled :v parameter:
//         !LO:SIC:v:v:l:s=et
//    Lyric with number instead of letter:
//         !LO:SIC:v:l:s=0-
//

void moveSic(HumdrumFile& infile, int line, int field, int verse) {
	HTp kerntok = infile.token(line, field);
	if (verboseQ) {
		cerr << "ADJUSTING: >>" << kerntok << "<< ON LINE " << (line+1) << " FIELD " << (field+1) << " MOVE TO VERSE " << verse << endl;
	}
	int ktrack = kerntok->getTrack();
	HTp texttok = NULL;
	int verseNum = 0;
	for (int j=field+1; j<infile[line].getFieldCount(); j++) {
		HTp token = infile.token(line, j);
		if (token->isKern()) {
			int track = token->getTrack();
			if (track == ktrack) {
				continue;
			} else {
				if (errorQ) {
					cout << "\tTOKEN " << kerntok << " NO VERSE " << verse << endl;
				} else {
					cerr << "\tCOULD NOT FIND TARGET VERSE" << endl;
					string text = *kerntok;
					HumRegex hre;
					hre.replaceDestructive(text, "LO:ZSIC", "LO:SIC", "g");
					kerntok->setText(text);
				}
				return;
			}
		}
		if (!token->isDataType("**text")) {
			continue;
		}
		verseNum++;
		if (verseNum != verse) {
			continue;
		}
		texttok = token;
		if (*texttok != "!") {
			if (errorQ) {
				cout << "NOT EMPTY : " << texttok << " FOR VERSE " << verse << " AND TOKEN " << kerntok << endl;
			} else {
				cerr << "\tTARGET TEXT TOKEN IS NOT EMPTY: " << texttok << endl;
				// In these cases, the non-empty target is more important than the information
				// being moved, so just clear the information being moved.
				kerntok->setText("!");
			}
			return;
		}
		break;
	}

	if (!texttok) {
		if (errorQ) {
			cout << "NO VERSE " << verse << " FOR MOVE OF " << kerntok << endl;
		} else {
			cerr << "\tCOULD NOT FIND VERSE TO MOVE SIC TO" << endl;
			// In this case change the LO:SIC to LO:ZSIC for review later.
			string text = *kerntok;
			HumRegex hre;
			hre.replaceDestructive(text, "LO:ZSIC", "LO:SIC", "g");
			kerntok->setText(text);
		}
		return;
	}

	HumRegex hre;
	string text = *kerntok;
	text = cleanSicText(text);

	// fix parameter error:
	hre.replaceDestructive(text, ":mod=", ":s:t=", "g");

	if (plainQ) {
		cerr << "\tUPDATING TEXT SIC TO: " << text << endl;
		kerntok->setText("!");
		texttok->setText(text);
	}
}



//////////////////////////////
//
// cleanSicText --
//
// Other manual fixing problems:
//    "ae" should be separate letters?
//       !LO:SIC:v:l:s=cæ
//

string cleanSicText(const string& input) {
	string output = input;
	HumRegex hre;
	// remove basic :l: parameters
	hre.replaceDestructive(output, ":", ":l:", "g");

	hre.replaceDestructive(output, ":", "::+", "g");

	// fix strange cases such as:
	//    !LO:SIC:l:s:v=Sae-
	hre.replaceDestructive(output, ":s=", ":s:v=");

	// remove verse parameters at end of line:
	hre.replaceDestructive(output, ":", ":verse\\s*\\d+$");

	// remove verse construction such as:
	//    !LO:SIC:l:s:v=:verse: 1:
	hre.replaceDestructive(output, ":", ":verse:\\s*\\d+[^:]*");

	// remove verse parameters embedded improperly in other parameters:
	hre.replaceDestructive(output, "", "\\s*verse\\s*\\d+\\s*$");

	// remove double equals:
	hre.replaceDestructive(output, ":s=", ":s==+", "g");
	hre.replaceDestructive(output, ":t=", ":t==+", "g");

	// remove implicit verse numbers as in:
	//   !LO:SIC:v:l:s=ти-:4
	hre.replaceDestructive(output, ":", ":\\d+[^:]*(:|$)");

	// remove :l parameter:
	hre.replaceDestructive(output, ":", ":l(?=:|$)", "g");
	if (!hre.search(output, "verse.*verse")) {   // do not change invalid double verse cases
		// remove verse parameter
		hre.replaceDestructive(output, ":", ":verse\\s*=?\\s*\\d+[^:]*(:|$)", "g");
	}
	// remove invalid verse:
	hre.replaceDestructive(output, ":", ":v\\s*=\\s*\\d+[^:]*(:|$)?", "g");

	// if there is no s= parameter but there is a t= parameter,
	// change to t= to s= to deal with cases such as:
	//    !LO:SIC:v:l:t=-nae
	if (hre.search(output, ":t=")) {
		if (!hre.search(output, ":t=[^:]*custos")) {  // don't convert t= that has the word custos in it
			if (!hre.search(output, ":s=")) {
				if (!hre.search(output, ":s:")) { // don't convert empty s parameters.
					hre.replaceDestructive(output, ":s=", ":t=", "g");
				}
		} 	else if (hre.search(output, ":s:t=")) {
					// probably/usually t= parameter contains substitution text
					hre.replaceDestructive(output, ":s=", ":s:t=", "g");
			}
		}
	}

	// remove all :v parameters
	hre.replaceDestructive(output, ":", ":v=?(:|$)", "g");

	// Add :v parameter immediately after SIC.
	// (not all SIC have :v, so adding to all).
	// hre.replaceDestructive(output, "SIC:v:", "SIC:", "g");

	// Change t= parameters to P= (problem):
	hre.replaceDestructive(output, ":P=", ":t=", "g");

	// Change free-text problems into parameterize problems, such as:
	//    !LO:SIC:l:s=sy:v:poprawione szarym kolorem
	if (hre.search(output, ":([^:]*)")) {
		// above regex not working
		string value = hre.getMatch(1);
		if (!hre.search(value, "=")) {
			if (hre.search(value, " ")) {
				string newvalue = "P=";
				newvalue += value;
				string oldvalue = value;
				hre.makeSafeDestructive(oldvalue);
				hre.replaceDestructive(output, newvalue, oldvalue);
			}
		}
	}

	// clean up possible ":s=:"
	hre.replaceDestructive(output, ":s:", ":s=:", "g");


	// cleanup possibly generated case:
	hre.replaceDestructive(output, ":", "::+");
	hre.replaceDestructive(output, "", ":$");

	if (!output.empty()) {
		if (output.back() == ':') {
			output.resize(output.size() - 1);
		}
	}

	// CONVERT TO MOD:

	// Change SIC to MOD:
	hre.replaceDestructive(output, "LO:MO:", "LO:SIC:");

	// remove empty s parameters:
	hre.replaceDestructive(output, ":", ":s:", "g");

	// Change s= to mod=
	hre.replaceDestructive(output, ":mod=", ":s=", "g");

	// clean up possible ":vmod="
	hre.replaceDestructive(output, ":mod=", ":vmod=", "g");

	// clean up possible ending problem
	hre.replaceDestructive(output, ":P=$1", ":([^:= ]* *[^:=]* *[^:=]*)$");

	return output;
}



