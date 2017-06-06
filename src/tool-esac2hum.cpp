//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Tue Mar  5 21:32:27 PST 2002
// Last Modified: Thu Sep 25 17:47:16 PDT 2003 Minor bug fixes
// Last Modified: Mon Jun  5 02:44:53 PDT 2017 Convert to STL
// Last Modified: Ported from Humdrum extras
// Filename:      tool-esac2hum.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/tool-esac2hum.cpp
// Syntax:        C++11
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Converts an EsAC file into Humdrum.
//


#include "tool-esac2hum.h"
#include "Convert.h"

#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <math.h>

#include <iostream>
#include <fstream>
#include <vector>

using namespace std;


namespace hum {

// START_MERGE


/////////////////////////////////
//
// Tool_esac2hum::Tool_esac2hum -- Set the recognized options for the tool.
//

Tool_esac2hum::Tool_esac2hum(void) {
	define("debug=b",            "print debug information");
	define("v|verbose=b",        "verbose output");
	define("h|header=s:",        "Header filename for placement in output");
	define("t|trailer=s:",       "Trailer filename for placement in output");
	define("s|split=s:file",     "Split song info into separate files");
	define("x|extension=s:.krn", "Split filename extension");
	define("f|first=i:1",        "Number of first split filename");
	define("author=b",           "author of program");
	define("version=b",          "compilation info");
	define("example=b",          "example usages");
	define("help=b",             "short description");
}



//////////////////////////////
//
// Tool_esac2hum::convert -- Convert a MusicXML file into
//     Humdrum content.
//

bool Tool_esac2hum::convertFile(ostream& out, const string& filename) {
	ifstream file(filename);
	stringstream s;
	if (file) {
		s << file.rdbuf();
		file.close();
	}
	return convert(out, s.str());
}


bool Tool_esac2hum::convert(ostream& out, istream& input) {
	convertEsacToHumdrum(out, input);
	return true;
}


bool Tool_esac2hum::convert(ostream& out, const string& input) {
	stringstream ss;
	ss << input;
	convertEsacToHumdrum(out, ss);
	return true;
}




//////////////////////////////
//
// Tool_esac2hum::initialize --
//

void Tool_esac2hum::initialize(void) {
	// handle basic options:
	if (getBoolean("author")) {
		cout << "Written by Craig Stuart Sapp, "
			  << "craig@ccrma.stanford.edu, March 2002" << endl;
		exit(0);
	} else if (getBoolean("version")) {
		cout << getCommand() << ", version: 6 June 2017" << endl;
		cout << "compiled: " << __DATE__ << endl;
		exit(0);
	} else if (getBoolean("help")) {
		usage(getCommand());
		exit(0);
	} else if (getBoolean("example")) {
		example();
		exit(0);
	}

	debugQ   = getBoolean("debug");
	verboseQ = getBoolean("verbose");

	if (getBoolean("header")) {
		getFileContents(header, getString("header"));
	} else {
		header.resize(0);
	}
	if (getBoolean("trailer")) {
		getFileContents(trailer, getString("trailer"));
	} else {
		trailer.resize(0);
	}

	if (getBoolean("split")) {
		splitQ = 1;
	}
	namebase = getString("split");
	fileextension = getString("extension");
	firstfilenum = getInteger("first");
}



//////////////////////////////////////////////////////////////////////////


//////////////////////////////
//
// Tool_esac2hum::convertEsacToHumdrum --
//

void Tool_esac2hum::convertEsacToHumdrum(ostream& output, istream& infile) {
	initialize();
	vector<string> song;
	song.reserve(400);
	int init = 0;
	// int filecounter = firstfilenum;
	string outfilename;
	string numberstring;
	// ofstream outfile;
	while (!infile.eof()) {
		if (debugQ) {
			cout << "Getting a song..." << endl;
		}
		getSong(song, infile, init);
		if (debugQ) {
			cout << "Got a song ..." << endl;
		}
		init = 1;
/*
		if (splitQ) {
			outfilename = namebase);
			outfilename += to_string(filecounter);
			if (filecounter < 1000) {
				outfilename += "0";
			}
			if (filecounter < 100) {
				outfilename += "0";
			}
			if (filecounter < 10) {
				outfilename += "0";
			}
			outfilename += numberstring;
			outfilename += fileextension;
			filecounter++;

			outfile.open(outfilename);

			if (!outfile.is_open()) {
				cout << "Error: cannot write to file: " << outfilename << endl;
			}
			convertSong(song, outfile);
			outfile.close();
		} else {
*/
			convertSong(song, output);
/*
		}
*/
	}
}



//////////////////////////////
//
// Tool_esac2hum::getSong -- get a song from the ESac file
//

void Tool_esac2hum::getSong(vector<string>& song, istream& infile, int init) {
	static char holdbuffer[10000] = {0};

	song.resize(0);
	if (init) {
		// do nothing holdbuffer has the CUT[] information
	} else {
		strcpy(holdbuffer, "");
		while (!infile.eof() && strncmp(holdbuffer, "CUT[", 4) != 0) {
			infile.getline(holdbuffer, 256, '\n');
			if (verboseQ) {
				cout << "Contents: " << holdbuffer << endl;
			}
		}
		if (infile.eof()) {
			return;
		}
	}

	song.resize(1);
	song[0] = holdbuffer;
	infile.getline(holdbuffer, 256, '\n');
	chopExtraInfo(holdbuffer);
	inputline++;
	if (verboseQ) {
		cout << "READ LINE: " << holdbuffer << endl;
	}
	while (!infile.eof() && strncmp(holdbuffer, "CUT[", 4) != 0) {
		song.push_back(holdbuffer);
		infile.getline(holdbuffer, 256, '\n');
		chopExtraInfo(holdbuffer);
		inputline++;
		if (verboseQ) {
			cout << "READ ANOTHER LINE: " << holdbuffer << endl;
		}
	}

}



//////////////////////////////
//
// Tool_esac2hum::chopExtraInfo -- remove phrase number information from Luxembourg data.
//

void Tool_esac2hum::chopExtraInfo(char* holdbuffer) {
	int length = strlen(holdbuffer);
	int i;
	int spacecount = 0;
	for (i=length-2; i>=0; i--) {
		if (holdbuffer[i] == ' ') {
			spacecount++;
			if (spacecount > 10) {
				holdbuffer[i] = '\0';
				break;
			}
		} else {
			spacecount = 0;
		}
	}
}



//////////////////////////////
//
// Tool_esac2hum::convertSong --
//

void Tool_esac2hum::convertSong(vector<string>& song, ostream& out) {

	int i;
	if (verboseQ) {
		for (i=0; i<(int)song.size(); i++) {
			out << song[i] << "\n";
		}
	}

	string key;
	double mindur = 1.0;
	string meter;
	int tonic;
	getKeyInfo(song, key, mindur, tonic, meter, out);

	vector<NoteData> songdata;
	songdata.resize(0);
	songdata.reserve(1000);
	getNoteList(song, songdata, mindur, tonic);
	placeLyrics(song, songdata);

	vector<int> numerator;
	vector<int> denominator;
	getMeterInfo(meter, numerator, denominator);

	postProcessSongData(songdata, numerator, denominator);

	printTitleInfo(song, out);
	out << "!!!id: "    << key  << "\n";


	// check for presence of lyrics
	int textQ = 0;
	for (i=0; i<(int)songdata.size(); i++) {
		if (songdata[i].text !=  "") {
			textQ = 1;
			break;
		}
	}

	for (i=0; i<(int)header.size(); i++) {
		out << header[i] << "\n";
	}

	out << "**kern";
	if (textQ) {
		out << "\t**text";
	}
	out << "\n";

	printKeyInfo(songdata, tonic, textQ, out);
	for (i=0; i<(int)songdata.size(); i++) {
		printNoteData(songdata[i], textQ, out);
	}
	out << "*-";
	if (textQ) {
		out << "\t*-";
	}
	out << "\n";

	out << "!!!minrhy: ";
	out << Convert::durationFloatToRecip(mindur)<<"\n";
	out << "!!!meter";
	if (numerator.size() > 1) {
		out << "s";
	}
	out << ": "  << meter;
	if ((meter == "frei") || (meter == "Frei")) {
		out << " [unmetered]";
	} else if (meter.find('/') == string::npos) {
		out << " interpreted as [";
		for (i=0; i<(int)numerator.size(); i++) {
			out << numerator[i] << "/" << denominator[i];
			if (i < (int)numerator.size()-1) {
				out << ", ";
			}
		}
		out << "]";
	}
	out << "\n";

	printBibInfo(song, out);
	printSpecialChars(out);

	for (i=0; i<(int)songdata.size(); i++) {
		if (songdata[i].lyricerr) {
			out << "!!!RWG: Lyric placement mismatch "
				  << "in phrase (too many syllables) " << songdata[i].phnum << " ["
				  << key << "]\n";
			break;
		}
	}

	for (i=0; i<(int)trailer.size(); i++) {
		out << trailer[i] << "\n";
	}

	if (!splitQ) {
		out << "\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
	}
}



//////////////////////////////
//
// Tool_esac2hum::placeLyrics -- extract lyrics (if any) and place on correct notes
//

void Tool_esac2hum::placeLyrics(vector<string>& song, vector<NoteData>& songdata) {
	int start = -1;
	int stop = -1;
	int length;
	getLineRange(song, "TXT", start, stop);
	if (start < 0) {
		// no TXT[] field, so don't do anything
		return;
	}
	int line = 0;
	vector<string> lyrics;
	string buffer;
	for (line=0; line<=stop-start; line++) {
		if (song[line+start].size() <= 4) {
			cout << "Error: lyric line is too short!: "
				  << song[line+start] << endl;
			exit(1);
		}
		buffer = song[line+start].substr(4);
		if (line == stop - start) {
			length = (int)buffer.size();
			auto loc = buffer.rfind(']');
			if (loc != string::npos) {
				buffer.resize(loc);
			}
		}
		if (buffer == "") {
			continue;
		}
		getLyrics(lyrics, buffer);
		cleanupLyrics(lyrics);
		placeLyricPhrase(songdata, lyrics, line);
	}
}



//////////////////////////////
//
// Tool_esac2hum::cleanupLyrics -- add preceeding dashes, avoid starting *'s if any,
//    and convert _'s to spaces.
//

void Tool_esac2hum::cleanupLyrics(vector<string>& lyrics) {
	int length;
	int length2;
	int i, j, m;
	int lastsyl = 0;
	for (i=0; i<(int)lyrics.size(); i++) {
		length = lyrics[i].size();
		for (j=0; j<length; j++) {
			if (lyrics[i][j] == '_') {
				lyrics[i][j] = ' ';
			}
		}

		if (i > 0) {
			if ((lyrics[i] != ".") &&
				 (lyrics[i] != "")  &&
				 (lyrics[i] != "%") &&
				 (lyrics[i] != "^") &&
				 (lyrics[i] != "|") &&
				 (lyrics[i] != " ")) {
				lastsyl = -1;
				for (m=i-1; m>=0; m--) {
					if ((lyrics[m] != ".") &&
						 (lyrics[m] != "")  &&
						 (lyrics[m] != "%") &&
						 (lyrics[i] != "^") &&
						 (lyrics[m] != "|") &&
						 (lyrics[m] != " ")) {
						lastsyl = m;
						break;
					}
				}
				if (lastsyl >= 0) {
					length2 = lyrics[lastsyl].size();
					if (lyrics[lastsyl][length2-1] == '-') {
						for (j=0; j<=length; j++) {
							lyrics[i][length - j + 1] = lyrics[i][length - j];
						}
						lyrics[i][0] = '-';
					}
				}
			}
		}

		// avoid *'s on the start of lyrics by placing a space before
		// them if they exist.
		if (lyrics[i][0] == '*') {
			length = lyrics[i].size();
			for (j=0; j<=length; j++) {
				lyrics[i][length - j + 1] = lyrics[i][length - j];
			}
			lyrics[i][0] = ' ';
		}

		// avoid !'s on the start of lyrics by placing a space before
		// them if they exist.
		if (lyrics[i][0] == '!') {
			length = lyrics[i].size();
			for (j=0; j<=length; j++) {
				lyrics[i][length - j + 1] = lyrics[i][length - j];
			}
			lyrics[i][0] = ' ';
		}

	}

}



///////////////////////////////
//
// Tool_esac2hum::getLyrics -- extract the lyrics from the text string.
//

void Tool_esac2hum::getLyrics(vector<string>& lyrics, const string& buffer) {
	lyrics.resize(0);
	int zero1 = 0;
	string current;
	int zero2 = 0;
	zero2 = zero1 + zero2;

	int length = buffer.size();
	int i;

	i = 0;
	while (i<length) {
		current = "";
		if (buffer[i] == ' ') {
			current = ".";
			lyrics.push_back(current);
			i++;
			continue;
		}

		while (i < length && buffer[i] != ' ') {
			current += buffer[i++];
		}
		lyrics.push_back(current);
		i++;
	}

}



//////////////////////////////
//
// Tool_esac2hum::placeLyricPhrase -- match lyrics from a phrase to the songdata.
//

void Tool_esac2hum::placeLyricPhrase(vector<NoteData>& songdata, vector<string>& lyrics, int line) {
	int i = 0;
	int start = 0;
	int found = 0;

	if (lyrics.size() == 0) {
		return;
	}

	// find the phrase to which the lyrics belongs
	for (i=0; i<(int)songdata.size(); i++) {
		if (songdata[i].phnum == line) {
			found = 1;
			break;
		}
	}
	start = i;

	if (!found) {
		cout << "Error: cannot find music for lyrics line " << line << endl;
		cout << "Error near input data line: " << inputline << endl;
		exit(1);
	}

	for (i=0; i<(int)lyrics.size() && i+start < (int)songdata.size(); i++) {
		if ((lyrics[i] == " ") || (lyrics[i] == ".") || (lyrics[i] == "")) {
			if (songdata[i+start].pitch < 0) {
				lyrics[i] = "%";
			} else {
				lyrics[i] = "|";
			}
			// lyrics[i] = ".";
		}
		songdata[i+start].text = lyrics[i];
		songdata[i+start].lyricnum = line;
		if (line != songdata[i+start].phnum) {
			songdata[i+start].lyricerr = 1;   // lyric does not line up with music
		}
	}

}



//////////////////////////////
//
// Tool_esac2hum::printSpecialChars -- print high ASCII character table
//

void Tool_esac2hum::printSpecialChars(ostream& out) {
	int i;
	for (i=0; i<(int)chartable.size(); i++) {
		if (chartable[i]) {
		switch (i) {
			case 129:   out << "!!!RNB" << ": symbol: &uuml;  = u umlaut (UTF-8: "
							     << (char)0xc3 << (char)0xb3 << ")\n";    break;
			case 130:   out << "!!!RNB" << ": symbol: &eacute;= e acute  (UTF-8: "
							     << (char)0xc3 << (char)0xa9 << ")\n";    break;
			case 132:   out << "!!!RNB" << ": symbol: &auml;  = a umlaut (UTF-8: "
							     << (char)0xc3 << (char)0xa4 << ")\n";    break;
			case 134:   out << "!!!RNB" << ": symbol: $c      = c acute  (UTF-8: "
							     << (char)0xc4 << (char)0x87 << ")\n";    break;
			case 136:   out << "!!!RNB" << ": symbol: $l      = l slash  (UTF-8: "
							     << (char)0xc5 << (char)0x82 << ")\n";    break;
			case 140:   out << "!!!RNB" << ": symbol: &icirc; = i circumflex (UTF-8: "
							     << (char)0xc3 << (char)0xaf << ")\n";    break;
			case 141:   out << "!!!RNB" << ": symbol: $X      = Z acute  (UTF-8: "
							     << (char)0xc5 << (char)0xb9 << ")\n";    break;
			case 142:   out << "!!!RNB" << ": symbol: &auml;  = a umlaut (UTF-8: "
							     << (char)0xc3 << (char)0xa4 << ")\n";    break;
			case 143:   out << "!!!RNB" << ": symbol: $C      = C acute  (UTF-8: "
							     << (char)0xc4 << (char)0x86 << ")\n";    break;
			case 148:   out << "!!!RNB" << ": symbol: &ouml;  = o umlaut (UTF-8: "
							     << (char)0xc3 << (char)0xb6 << ")\n";    break;
			case 151:   out << "!!!RNB" << ": symbol: $S      = S acute  (UTF-8: "
							     << (char)0xc5 << (char)0x9a << ")\n";    break;
			case 152:   out << "!!!RNB" << ": symbol: $s      = s acute  (UTF-8: "
							     << (char)0xc5 << (char)0x9b << ")\n";    break;
			case 156:   out << "!!!RNB" << ": symbol: $s      = s acute  (UTF-8: "
							     << (char)0xc5 << (char)0x9b << ")\n";    break;
			case 157:   out << "!!!RNB" << ": symbol: $L      = L slash  (UTF-8: "
							     << (char)0xc5 << (char)0x81 << ")\n";    break;
			case 159:   out << "!!!RNB" << ": symbol: $vc     = c hachek (UTF-8: "
							     << (char)0xc4 << (char)0x8d << ")\n";    break;
			case 162:   out << "!!!RNB" << ": symbol: &oacute;= o acute  (UTF-8: "
							     << (char)0xc3 << (char)0xb3 << ")\n";    break;
			case 163:   out << "!!!RNB" << ": symbol: &uacute;= u acute  (UTF-8: "
							     << (char)0xc3 << (char)0xba << ")\n";    break;
			case 165:   out << "!!!RNB" << ": symbol: $a      = a hook   (UTF-8: "
							     << (char)0xc4 << (char)0x85 << ")\n";    break;
			case 169:   out << "!!!RNB" << ": symbol: $e      = e hook   (UTF-8: "
							     << (char)0xc4 << (char)0x99 << ")\n";    break;
			case 171:   out << "!!!RNB" << ": symbol: $y      = z acute  (UTF-8: "
							     << (char)0xc5 << (char)0xba << ")\n";    break;
			case 175:   out << "!!!RNB" << ": symbol: $Z      = Z dot    (UTF-8: "
							     << (char)0xc5 << (char)0xbb << ")\n";    break;
			case 179:   out << "!!!RNB" << ": symbol: $l      = l slash  (UTF-8: "
							     << (char)0xc5 << (char)0x82 << ")\n";    break;
			case 185:   out << "!!!RNB" << ": symbol: $a      = a hook   (UTF-8: "
							     << (char)0xc4 << (char)0x85 << ")\n";    break;
			case 189:   out << "!!!RNB" << ": symbol: $Z      = Z dot    (UTF-8: "
							     << (char)0xc5 << (char)0xbb << ")\n";    break;
			case 190:   out << "!!!RNB" << ": symbol: $z      = z dot    (UTF-8: "
							     << (char)0xc5 << (char)0xbc << ")\n";    break;
			case 191:   out << "!!!RNB" << ": symbol: $z      = z dot    (UTF-8: "
							     << (char)0xc5 << (char)0xbc << ")\n";    break;
			case 224:   out << "!!!RNB" << ": symbol: &Oacute;= O acute  (UTF-8: "
							     << (char)0xc3 << (char)0x93 << ")\n";    break;
			case 225:   out << "!!!RNB" << ": symbol: &szlig; = sz ligature (UTF-8: "
							     << (char)0xc3 << (char)0x9f << ")\n";    break;
			case 0xdf:  out << "!!!RNB" << ": symbol: &szlig; = sz ligature (UTF-8: "
							     << (char)0xc3 << (char)0x9f << ")\n";    break;
// Polish version:
//         case 228:   out << "!!!RNB" << ": symbol: $n      = n acute  (UTF-8: "
//                          << (char)0xc5 << (char)0x84 << ")\n";    break;
// Luxembourg version for some reason...:
			case 228:   out << "!!!RNB" << ": symbol: &auml;      = a umlaut  (UTF-8: "
							     << (char)0xc5 << (char)0x84 << ")\n";    break;
			case 230:   out << "!!!RNB" << ": symbol: c       = c\n";           break;
			case 231:   out << "!!!RNB" << ": symbol: $vs     = s hachek (UTF-8: "
							     << (char)0xc5 << (char)0xa1 << ")\n";    break;
			case 234:   out << "!!!RNB" << ": symbol: $e      = e hook   (UTF-8: "
							     << (char)0xc4 << (char)0x99 << ")\n";    break;
			case 241:   out << "!!!RNB" << ": symbol: $n      = n acute  (UTF-8: "
							     << (char)0xc5 << (char)0x84 << ")\n";    break;
			case 243:   out << "!!!RNB" << ": symbol: &oacute;= o acute  (UTF-8: "
							     << (char)0xc3 << (char)0xb3 << ")\n";    break;
			case 252:   out << "!!!RNB" << ": symbol: &uuml;  = u umlaut (UTF-8: "
							     << (char)0xc3 << (char)0xbc << ")\n";    break;
//         default:
		}
		}
		chartable[i] = 0;
	}
}



//////////////////////////////
//
// Tool_esac2hum::printTitleInfo -- print the first line of the CUT[] field.
//

void Tool_esac2hum::printTitleInfo(vector<string>& song, ostream& out) {
	int start = -1;
	int stop = -1;
	getLineRange(song, "CUT", start, stop);
	if (start == -1) {
		cout << "Error: cannot find CUT[] field in song: " << song[0] << endl;
		exit(1);
	}

	string buffer;
	buffer = song[start].substr(4);
	if (buffer.back() == ']') {
		buffer.resize(buffer.size() - 1);
	}

	out << "!!!OTL: ";
	for (int i=0; i<(int)buffer.size(); i++) {
		printChar(buffer[i], out);
	}
	out << "\n";
}



//////////////////////////////
//
// Tool_esac2hum::printChar -- print text characters, translating high-bit data
//    if required.
//

void Tool_esac2hum::printChar(unsigned char c, ostream& out) {
	if (c < 128) {
		out << c;
	} else {
		chartable[c]++;
		switch (c) {
			case 129:   out << "&uuml;";    break;
			case 130:   out << "&eacute;";  break;
			case 132:   out << "&auml;";    break;
			case 134:   out << "$c";        break;
			case 136:   out << "$l";        break;
			case 140:   out << "&icirc;";   break;
			case 141:   out << "$X";        break;   // Z acute
			case 142:   out << "&auml;";    break;   // ?
			case 143:   out << "$C";        break;
			case 148:   out << "&ouml;";    break;
			case 151:   out << "$S";        break;
			case 152:   out << "$s";        break;
			case 156:   out << "$s";        break;  // 1250 encoding
			case 157:   out << "$L";        break;
			case 159:   out << "$vc";       break;  // Cech c with v accent
			case 162:   out << "&oacute;";  break;
			case 163:   out << "&uacute;";  break;
			case 165:   out << "$a";        break;
			case 169:   out << "$e";        break;
			case 171:   out << "$y";        break;
			case 175:   out << "$Z";        break;  // 1250 encoding
			case 179:   out << "$l";        break;  // 1250 encoding
			case 185:   out << "$a";        break;  // 1250 encoding
			case 189:   out << "$Z";        break;  // Z dot
			case 190:   out << "$z";        break;  // z dot
			case 191:   out << "$z";        break;  // 1250 encoding
			case 224:   out << "&Oacute;";  break;
			case 225:   out << "&szlig;";   break;
			case 0xdf:  out << "&szlig;";   break;
			// Polish version:
			// case 228:   out << "$n";        break;
			// Luxembourg version (for some reason...)
			case 228:   out << "&auml;";        break;
			case 230:   out << "c";         break;  // ?
			case 231:   out << "$vs";       break;  // Cech s with v accent
			case 234:   out << "$e";        break;  // 1250 encoding
			case 241:   out << "$n";        break;  // 1250 encoding
			case 243:   out << "&oacute;";  break;  // 1250 encoding
			case 252:   out << "&uuml;";    break;
			default:    out << c;
		}
	}
}



//////////////////////////////
//
// Tool_esac2hum::printKeyInfo --
//

void Tool_esac2hum::printKeyInfo(vector<NoteData>& songdata, int tonic, int textQ,
		ostream& out) {
	vector<int> pitches(40, 0);
	int pitchsum = 0;
	int pitchcount = 0;
	int i;
	for (i=0; i<(int)songdata.size(); i++) {
		if (songdata[i].pitch >= 0) {
			pitches[songdata[i].pitch % 40]++;
			pitchsum += Convert::base40ToMidiNoteNumber(songdata[i].pitch);
			pitchcount++;
		}
	}

	// generate a clef, choosing either treble or bass clef depending
	// on the average pitch.
	double averagepitch = pitchsum * 1.0 / pitchcount;
	if (averagepitch > 60.0) {
		out << "*clefG2";
		if (textQ) {
			out << "\t*clefG2";
		}
		out << "\n";
	} else {
		out << "*clefF4";
		if (textQ) {
			out << "\t*clefF4";
		}
		out << "\n";
	}

	// generate a key signature
	vector<int> diatonic(7, 0);
	diatonic[0] = getAccidentalMax(pitches[1], pitches[2], pitches[3]);
	diatonic[1] = getAccidentalMax(pitches[7], pitches[8], pitches[9]);
	diatonic[2] = getAccidentalMax(pitches[13], pitches[14], pitches[15]);
	diatonic[3] = getAccidentalMax(pitches[18], pitches[19], pitches[20]);
	diatonic[4] = getAccidentalMax(pitches[24], pitches[25], pitches[26]);
	diatonic[5] = getAccidentalMax(pitches[30], pitches[31], pitches[32]);
	diatonic[6] = getAccidentalMax(pitches[36], pitches[37], pitches[38]);

	int flatcount = 0;
	int sharpcount = 0;
	int naturalcount = 0;
	for (i=0; i<7; i++) {
		switch (diatonic[i]) {
			case -1:   flatcount++;      break;
			case  0:   naturalcount++;   break;
			case +1:   sharpcount++;     break;
		}
	}

	char kbuf[32] = {0};
	if (naturalcount == 7) {
		// do nothing
	} else if (flatcount > sharpcount) {
		// print a flat key signature
		if (diatonic[6] == -1) strcat(kbuf, "b-"); else goto keysigend;
		if (diatonic[2] == -1) strcat(kbuf, "e-"); else goto keysigend;
		if (diatonic[5] == -1) strcat(kbuf, "a-"); else goto keysigend;
		if (diatonic[1] == -1) strcat(kbuf, "d-"); else goto keysigend;
		if (diatonic[4] == -1) strcat(kbuf, "g-"); else goto keysigend;
		if (diatonic[0] == -1) strcat(kbuf, "c-"); else goto keysigend;
		if (diatonic[3] == -1) strcat(kbuf, "f-"); else goto keysigend;
	} else {
		// print a sharp key signature
		if (diatonic[3] == +1) strcat(kbuf, "f#"); else goto keysigend;
		if (diatonic[0] == +1) strcat(kbuf, "c#"); else goto keysigend;
		if (diatonic[4] == +1) strcat(kbuf, "g#"); else goto keysigend;
		if (diatonic[1] == +1) strcat(kbuf, "d#"); else goto keysigend;
		if (diatonic[5] == +1) strcat(kbuf, "a#"); else goto keysigend;
		if (diatonic[2] == +1) strcat(kbuf, "e#"); else goto keysigend;
		if (diatonic[6] == +1) strcat(kbuf, "b#"); else goto keysigend;
	}

keysigend:
	out << "*k[" << kbuf << "]";
	if (textQ) {
		out << "\t*k[" << kbuf << "]";
	}
	out << "\n";

	// look at the third scale degree above the tonic pitch
	int minor = pitches[(tonic + 40 + 11) % 40];
	int major = pitches[(tonic + 40 + 12) % 40];

	if (minor > major) {
		// minor key (or related mode)
		out  << "*" << Convert::base40ToKern(40 * 4 + tonic) << ":";
		if (textQ) {
			out  << "\t*" << Convert::base40ToKern(40 * 4 + tonic) << ":";
		}
		out << "\n";
	} else {
		// major key (or related mode)
		out  << "*" << Convert::base40ToKern(40 * 3 + tonic) << ":";
		if (textQ) {
			out  << "\t*" << Convert::base40ToKern(40 * 3 + tonic) << ":";
		}
		out << "\n";
	}

}


//////////////////////////////
//
// Tool_esac2hum::getAccidentalMax --
//

int Tool_esac2hum::getAccidentalMax(int a, int b, int c) {
	if (a > b && a > c) {
		return -1;
	} else if (c > a && c > b) {
		return +1;
	} else {
		return 0;
	}
}


//////////////////////////////
//
// Tool_esac2hum::postProcessSongData -- clean up data and do some interpreting.
//

void Tool_esac2hum::postProcessSongData(vector<NoteData>& songdata, vector<int>& numerator,
		vector<int>& denominator) {
	int i, j;
	// move phrase start markers off of rests and onto the
	// first note that it finds
	for (i=0; i<(int)songdata.size()-1; i++) {
		if (songdata[i].pitch < 0 && songdata[i].phstart) {
			songdata[i+1].phstart = songdata[i].phstart;
			songdata[i].phstart = 0;
		}
	}

	// move phrase ending markers off of rests and onto the
	// previous note that it finds
	for (i=(int)songdata.size()-1; i>0; i--) {
		if (songdata[i].pitch < 0 && songdata[i].phend) {
			songdata[i-1].phend = songdata[i].phend;
			songdata[i].phend = 0;
		}
	}

	// examine barline information
	double dur = 0.0;
	for (i=(int)songdata.size()-1; i>=0; i--) {
		if (songdata[i].bar == 1) {
			songdata[i].bardur = dur;
			dur = songdata[i].duration;
		} else {
			dur += songdata[i].duration;
		}
	}

	int barnum = 0;
	double firstdur = 0.0;
	if (numerator.size() == 1 && numerator[0] > 0) {
		// handle single non-frei meter
		songdata[0].num = numerator[0];
		songdata[0].denom = denominator[0];
		dur = 0;
		double meterdur = 4.0 / denominator[0] * numerator[0];
		for (i=0; i<(int)songdata.size(); i++) {
			if (songdata[i].bar) {
				dur = 0.0;
			} else {
				dur += songdata[i].duration;
				if (fabs(dur - meterdur) < 0.001) {
					songdata[i].bar = 1;
					songdata[i].barinterp = 1;
					dur = 0.0;
				}
			}
		}

		// readjust measure beat counts
		dur = 0.0;
		for (i=(int)songdata.size()-1; i>=0; i--) {
			if (songdata[i].bar == 1) {
				songdata[i].bardur = dur;
				dur = songdata[i].duration;
			} else {
				dur += songdata[i].duration;
			}
		}
		firstdur = dur;

		// number the barlines
		barnum = 0;
		if (fabs(firstdur - meterdur) < 0.001) {
			// music for first bar, next bar will be bar 2
			barnum = 2;
		} else {
			barnum = 1;
			// pickup-measure
		}
		for (i=0; i<(int)songdata.size(); i++) {
			if (songdata[i].bar == 1) {
				songdata[i].barnum = barnum++;
			}
		}

	} else if (numerator.size() == 1 && numerator[0] == -1) {
		// handle free meter

		// number the barline
		firstdur = dur;
		barnum = 1;
		for (i=0; i<(int)songdata.size(); i++) {
			if (songdata[i].bar == 1) {
				songdata[i].barnum = barnum++;
			}
		}

	} else {
		// handle multiple time signatures

		// get the duration of each type of meter:
		vector<double> meterdurs;
		meterdurs.resize(numerator.size());
		for (i=0; i<(int)meterdurs.size(); i++) {
			meterdurs[i] = 4.0 / denominator[i] * numerator[i];
		}

		// measure beat counts:
		dur = 0.0;
		for (i=(int)songdata.size()-1; i>=0; i--) {
			if (songdata[i].bar == 1) {
				songdata[i].bardur = dur;
				dur = songdata[i].duration;
			} else {
				dur += songdata[i].duration;
			}
		}
		firstdur = dur;

		// interpret missing barlines
		int currentmeter = 0;
		// find first meter
		for (i=0; i<(int)numerator.size(); i++) {
			if (fabs(firstdur - meterdurs[i]) < 0.001) {
				songdata[0].num = numerator[i];
				songdata[0].denom = denominator[i];
				currentmeter = i;
			}
		}
		// now handle the meters in the rest of the music...
		int fnd = 0;
		dur = 0;
		for (i=0; i<(int)songdata.size()-1; i++) {
			if (songdata[i].bar) {
				if (songdata[i].bardur != meterdurs[currentmeter]) {
					// try to find the correct new meter

					fnd = 0;
					for (j=0; j<(int)numerator.size(); j++) {
						if (j == currentmeter) {
							continue;
						}
						if (fabs(songdata[i].bardur - meterdurs[j]) < 0.001) {
							songdata[i+1].num = numerator[j];
							songdata[i+1].denom = denominator[j];
							currentmeter = j;
							fnd = 1;
						}
					}
					if (!fnd) {
						for (j=0; j<(int)numerator.size(); j++) {
							if (j == currentmeter) {
							   continue;
							}
							if (fabs(songdata[i].bardur/2.0 - meterdurs[j]) < 0.001) {
							   songdata[i+1].num = numerator[j];
							   songdata[i+1].denom = denominator[j];
							   currentmeter = j;
							   fnd = 1;
							}
						}
					}
				}
				dur = 0.0;
			} else {
				dur += songdata[i].duration;
				if (fabs(dur - meterdurs[currentmeter]) < 0.001) {
					songdata[i].bar = 1;
					songdata[i].barinterp = 1;
					dur = 0.0;
				}
			}
		}

		// perhaps sum duration of measures again and search for error here?

		// finally, number the barlines:
		barnum = 1;
		for (i=0; i<(int)numerator.size(); i++) {
			if (fabs(firstdur - meterdurs[i]) < 0.001) {
				barnum = 2;
				break;
			}
		}
		for (i=0; i<(int)songdata.size(); i++) {
			if (songdata[i].bar == 1) {
				songdata[i].barnum = barnum++;
			}
		}


	}

}



//////////////////////////////
//
// Tool_esac2hum::getMeterInfo --
//

void Tool_esac2hum::getMeterInfo(string& meter, vector<int>& numerator,
		vector<int>& denominator) {
	char buffer[256] = {0};
	strcpy(buffer, meter.c_str());
	numerator.resize(0);
	denominator.resize(0);
	int num = -1;
	int denom = -1;
	char* ptr;
	ptr = strtok(buffer, " \t\n");
	while (ptr != NULL) {
		if (strcmp(ptr, "frei") == 0 || strcmp(ptr, "Frei") == 0) {
			num = -1;
			denom = -1;
			numerator.push_back(num);
			denominator.push_back(denom);
		} else {
			if (strchr(ptr, '/') != NULL) {
				num = -1;
				denom = 4;
				sscanf(ptr, "%d/%d", &num, &denom);
				numerator.push_back(num);
				denominator.push_back(denom);
			} else {
				num = atoi(ptr);
				denom = 4;
				numerator.push_back(num);
				denominator.push_back(denom);
			}
		}
		ptr = strtok(NULL, " \t\n");
	}

}



//////////////////////////////
//
// Tool_esac2hum::getLineRange -- get the staring line and ending line of a data
//     field.  Returns -1 if the data field was not found.
//

void Tool_esac2hum::getLineRange(vector<string>& song, const string& field,
		int& start, int& stop) {
	string searchstring = field;;
	searchstring += "[";
	start = stop = -1;
	for (int i=0; i<(int)song.size(); i++) {
		auto loc = song[i].find(']');
		if (song[i].compare(0, searchstring.size(), searchstring) == 0) {
			start = i;
			if (loc != string::npos) {
				stop = i;
				break;
			}
		} else if ((start >= 0) && (loc != string::npos)) {
			stop = i;
			break;
		}
	}
}



//////////////////////////////
//
// Tool_esac2hum::getNoteList -- get a list of the notes and rests and barlines in
//    the MEL field.
//

void Tool_esac2hum::getNoteList(vector<string>& song, vector<NoteData>& songdata, double mindur,
		int tonic) {
	songdata.resize(0);
	NoteData tempnote;
	int melstart = -1;
	int melstop  = -1;
	int i, j;
	int octave      = 0;
	int degree      = 0;
	int accidental  = 0;
	double duration = mindur;
	int bar    = 0;
	// int tuplet = 0;
	int major[8] = {-1, 0, 6, 12, 17, 23, 29, 35};
	// int oldstate  = -1;
	int state     = -1;
	int nextstate = -1;
	int phend = 0;
	int phnum = 0;
	int phstart = 0;
	int slend = 0;
	int slstart = 0;
	int tie = 0;

	getLineRange(song, "MEL", melstart, melstop);

	for (i=melstart; i<=melstop; i++) {
		if (song[i].size() < 4) {
			cout << "Error: invalid line in MEL[]: " << song[i] << endl;
			exit(1);
		}
		j = 4;
		phstart = 1;
		phend = 0;
		// Note Format: (+|-)*[0..7]_*\.*(  )?
		// ONADB
		// Order of data: Octave, Note, Accidental, Duration, Barline

		#define STATE_SLSTART -1
		#define STATE_OCTAVE   0
		#define STATE_NOTE     1
		#define STATE_ACC      2
		#define STATE_DUR      3
		#define STATE_BAR      4
		#define STATE_SLEND    5

		while (j < 200 && (j < song[i].size())) {
			// oldstate = state;
			switch (song[i][j]) {
				// Octave information:
				case '-': octave--; state = STATE_OCTAVE; break;
				case '+': octave++; state = STATE_OCTAVE; break;

				// Duration information:
				case '_': duration *= 2.0; state = STATE_DUR; break;
				case '.': duration *= 1.5; state = STATE_DUR; break;

				// Accidental information:
				case 'b': accidental--; state = STATE_ACC;  break;
				case '#': accidental++; state = STATE_ACC;  break;

				// Note information:
				case '0': case '1': case '2': case '3': case '4':
				case '5': case '6': case '7':
					degree =  major[song[i][j] - '0'];
					state = STATE_NOTE;
					break;
				case 'O':
					degree =  major[0];
					state = STATE_NOTE;
					break;

				// Barline information:
				case ' ':
					state = STATE_BAR;
					if (song[i][j+1] == ' ') {
						bar = 1;
					}
					break;

				// Other information:
				case '{': slstart = 1;  state = STATE_SLSTART;  break;
				case '}': slend   = 1;  state = STATE_SLEND;    break;
				// case '(': tuplet  = 1;        break;
				// case ')': tuplet  = 0;        break;
				case '/':                     break;
				case ']':                     break;
//            case '>':                     break;   // unknown marker
//            case '<':                     break;   //
				case '^': tie = 1; state = STATE_NOTE; break;
				default : cout << "Error: unknown character " << song[i][j]
							      << " on the line: " << song[i] << endl;
							 exit(1);
			}
			j++;
			switch (song[i][j]) {
				case '-': case '+': nextstate = STATE_OCTAVE; break;
				case 'O':
				case '0': case '1': case '2': case '3': case '4':
				case '5': case '6': case '7': nextstate = STATE_NOTE; break;
				case 'b': case '#': nextstate = STATE_ACC;    break;
				case '_': case '.': nextstate = STATE_DUR; break;
				case '{': nextstate = STATE_SLSTART; break;
				case '}': nextstate = STATE_SLEND; break;
				case '^': nextstate = STATE_NOTE; break;
				case ' ':
					 if (song[i][j+1] == ' ') nextstate = STATE_BAR;
					 else if (song[i][j+1] == '/') nextstate = -2;
					 break;
				case '\0':
					phend = 1;
				default: nextstate = -1;
			}

			if (nextstate < state ||
					((nextstate == STATE_NOTE) && (state == nextstate))) {
				 tempnote.clear();
				 if (degree < 0) { // rest
					 tempnote.pitch = -999;
				 } else {
					 tempnote.pitch = degree + 40*(octave + 4) + accidental + tonic;
				 }
				 if (tie) {
					 tempnote.pitch = songdata[(int)songdata.size()-1].pitch;
					 if (songdata[(int)songdata.size()-1].tieend) {
						 songdata[(int)songdata.size()-1].tiecont = 1;
						 songdata[(int)songdata.size()-1].tieend = 0;
					 } else {
						 songdata[(int)songdata.size()-1].tiestart = 1;
					 }
					 tempnote.tieend = 1;
				 }
				 tempnote.duration = duration;
				 tempnote.phend = phend;
				 tempnote.bar = bar;
				 tempnote.phstart = phstart;
				 tempnote.slstart = slstart;
				 tempnote.slend = slend;
				 if (nextstate == -2) {
					 tempnote.bar = 2;
					 tempnote.phend = 1;
				 }
				 tempnote.phnum = phnum;

				 songdata.push_back(tempnote);
				 duration = mindur;
				 degree = 0;
				 bar = 0;
				 tie = 0;
				 phend = 0;
				 phstart = 0;
				 slend = 0;
				 slstart = 0;
				 octave = 0;
				 accidental = 0;
				 if (nextstate == -2) {
					 return;
				 }
			}
		}
		phnum++;
	}

}



//////////////////////////////
//
// Tool_esac2hum::printNoteData --
//

void Tool_esac2hum::printNoteData(NoteData& data, int textQ, ostream& out) {

	if (data.num > 0) {
		out << "*M" << data.num << "/" << data.denom;
		if (textQ) {
			out << "\t*M" << data.num << "/" << data.denom;
		}
		out << "\n";
	}
	if (data.phstart == 1) {
		out << "{";
	}
	if (data.slstart == 1) {
		out << "(";
	}
	if (data.tiestart == 1) {
		out << "[";
	}
	out << Convert::durationFloatToRecip(data.duration);
	if (data.pitch < 0) {
		out << "r";
	} else {
		out << Convert::base40ToKern(data.pitch);
	}
	if (data.tiecont == 1) {
		out << "_";
	}
	if (data.tieend == 1) {
		out << "]";
	}
	if (data.slend == 1) {
		out << ")";
	}
	if (data.phend == 1) {
		out << "}";
	}

	if (textQ) {
		out << "\t";
		if (data.phstart == 1) {
			out << "{";
		}
		if (data.text == "") {
			if (data.pitch < 0) {
				data.text = "%";
			} else {
				data.text = "|";
			}
		}
		if (data.pitch < 0 && (data.text.find('%') == string::npos)) {
			out << "%";
		}
		if (data.text == " *") {
			if (data.pitch < 0) {
				data.text = "%*";
			} else {
				data.text = "|*";
			}
		}
		if (data.text == "^") {
			data.text = "|^";
		}
		printString(data.text, out);
		if (data.phend == 1) {
			out << "}";
		}
	}

	out << "\n";

	// print barline information
	if (data.bar == 1) {

		out << "=";
		if (data.barnum > 0) {
			out << data.barnum;
		}
		if (data.barinterp) {
			// out << "yy";
		}
		if (debugQ) {
			if (data.bardur > 0.0) {
				out << "[" << data.bardur << "]";
			}
		}
		if (textQ) {
			out << "\t";
			out << "=";
			if (data.barnum > 0) {
				out << data.barnum;
			}
			if (data.barinterp) {
				// out << "yy";
			}
			if (debugQ) {
				if (data.bardur > 0.0) {
					out << "[" << data.bardur << "]";
				}
			}
		}

		out << "\n";
	} else if (data.bar == 2) {
		out << "==";
		if (textQ) {
			out << "\t==";
		}
		out << "\n";
	}
}



//////////////////////////////
//
// Tool_esac2hum::getKeyInfo -- look for a KEY[] entry and extract the data.
//
// ggg fix this function
//

void Tool_esac2hum::getKeyInfo(vector<string>& song, string& key, double& mindur,
		int& tonic, string& meter, ostream& out) {
	int i;
	for (i=0; i<(int)song.size(); i++) {
		if (song[i].compare(0, 4, "KEY[") == 0) {
			key = song[i][4]; // letter
			key += song[i][5]; // number
			key += song[i][6]; // number
			key += song[i][7]; // number
			key += song[i][8]; // number
			if (!isspace(song[i][9])) {
				key += song[i][9];  // optional letter (sometimes ' or ")
			}
			if (!isspace(song[i][10])) {
				key += song[i][10];  // illegal but possible extra letter
			}
			if (song[i][10] != ' ') {
				out << "!! Warning key field is not complete" << endl;
				out << "!!Key field: " << song[i] << endl;
			}

			mindur = (song[i][11] - '0') * 10 + (song[i][12] - '0');
			mindur = 4.0 / mindur;

			string tonicstr;
			if (song[i][14] != ' ') {
				tonicstr[0] = song[i][14];
				if (tolower(song[i][15]) == 'b') {
					tonicstr[1] = '-';
				} else {
					tonicstr[1] = song[i][15];
				}
				tonicstr[2] = '\0';
			} else {
				tonicstr = song[i][15];
			}

			// convert German notation to English for note names
			// Hopefully all references to B will mean English B-flat.
			if (tonicstr == "B") {
				tonicstr = "B-";
			}
			if (tonicstr == "H") {
				tonicstr = "B";
			}

			tonic = Convert::kernToBase40(tonicstr);
			if (tonic <= 0) {
				cout << "Error: invalid tonic on line: " << song[i] << endl;
				exit(1);
			}
			tonic = tonic % 40;
			meter = song[i].substr(17);
			if (meter.back() != ']') {
				cout << "Error with meter on line: " << song[i] << endl;
				exit(1);
			} else {
				meter.resize(meter.size() - 1);
			}
			return;
		}
	}
	cout << "Error: did not find a KEY field" << endl;
	exit(1);
}



///////////////////////////////
//
// Tool_esac2hum::getFileContents -- read a file into the array.
//

void Tool_esac2hum::getFileContents(vector<string>& array, const string& filename) {
	ifstream infile(filename.c_str());
	array.reserve(100);
	array.resize(0);

	if (!infile.is_open()) {
		cout << "Error: cannot open file: " << filename << endl;
		exit(1);
	}

	char holdbuffer[1024] = {0};

	infile.getline(holdbuffer, 256, '\n');
	while (!infile.eof()) {
		array.push_back(holdbuffer);
		infile.getline(holdbuffer, 256, '\n');
	}

	infile.close();
}



//////////////////////////////
//
// Tool_esac2hum::example --
//

void Tool_esac2hum::example(void) {


}



//////////////////////////////
//
// Tool_esac2hum::usage --
//

void Tool_esac2hum::usage(const string& command) {

}



//////////////////////////////
//
// Tool_esac2hum::printBibInfo --
//

void Tool_esac2hum::printBibInfo(vector<string>& song, ostream& out) {
	int i, j;
	char buffer[32] = {0};
	int start = -1;
	int stop  = -1;
	int count = 0;
	string templine;

	for (i=0; i<(int)song.size(); i++) {
		if (song[i] == "") {
			continue;
		}
		if (song[i][0] != ' ') {
			if (song[i].size() < 4 || song[i][3] != '[') {
				out << "!! " << song[i] << "\n";
				continue;
			}
			strncpy(buffer, song[i].c_str(), 3);
			buffer[3] = '\0';
			if (strcmp(buffer, "MEL") == 0) continue;
			if (strcmp(buffer, "TXT") == 0) continue;
			// if (strcmp(buffer, "KEY") == 0) continue;
			getLineRange(song, buffer, start, stop);

			// don't print CUT field if only one line.  !!!OTL: will contain CUT[]
			// if (strcmp(buffer, "CUT") == 0 && start == stop) continue;

			buffer[0] = tolower(buffer[0]);
			buffer[1] = tolower(buffer[1]);
			buffer[2] = tolower(buffer[2]);

			count = 1;
			templine = "";
			for (j=start; j<=stop; j++) {
				if (song[j].size() < 4) {
					continue;
				}
				if (stop - start == 0) {
					templine = song[j].substr(4);
					auto loc = templine.find(']');
					if (loc != string::npos) {
						templine.resize(loc);
					}
					if (templine != "") {
						out << "!!!" << buffer << ": ";
						printString(templine, out);
						out << "\n";
					}

				} else if (j==start) {
					out << "!!!" << buffer << count++ << ": ";
					printString(song[j].substr(4), out);
					out << "\n";
				} else if (j==stop) {
					templine = song[j].substr(4);
					auto loc = templine.find(']');
					if (loc != string::npos) {
						templine.resize(loc);
					}
					if (templine != "") {
						out << "!!!" << buffer << count++ << ": ";
						printString(templine, out);
						out << "\n";
					}
				} else {
					out << "!!!" << buffer << count++ << ": ";
					printString(&(song[j][4]), out);
					out << "\n";
				}
			}
		}
	}
}



//////////////////////////////
//
// Tool_esac2hum::printString -- print characters in string.
//

void Tool_esac2hum::printString(const string& string, ostream& out) {
	for (int i=0; i<(int)string.size(); i++) {
		printChar(string[i], out);
	}
}



// END_MERGE

} // end namespace hum



