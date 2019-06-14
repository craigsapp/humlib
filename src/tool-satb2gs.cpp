//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sun Feb  6 14:33:36 PST 2011
// Last Modified: Fri Dec 16 01:01:58 PST 2016 Ported to humlib.
// Filename:      tool-satb2gs.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/tool-satb2gs.cpp
// Syntax:        C++11; humlib
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Converts Soprano/Alto/Tenor/Bass staves into
//                Grand-staff style.
//

#include "tool-satb2gs.h"
#include "HumRegex.h"
#include "Convert.h"

#include <iostream>
#include <sstream>

using namespace std;

namespace hum {

// START_MERGE


/////////////////////////////////
//
// Tool_satb2gs::Tool_satb2gs -- Set the recognized options for the tool.
//

Tool_satb2gs::Tool_satb2gs(void) {
   define("d|debug=b",    "Debugging information");
   define("author=b",     "Program author");
   define("version=b",    "Program version");
   define("example=b",    "Program examples");
   define("h|help=b",     "Short description");
}



/////////////////////////////////
//
// Tool_satb2gs::run -- Primary interfaces to the tool.
//

bool Tool_satb2gs::run(const string& indata, ostream& out) {
	HumdrumFile infile(indata);
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_satb2gs::run(HumdrumFile& infile, ostream& out) {
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}

//
// In-place processing of file:
//

bool Tool_satb2gs::run(HumdrumFile& infile) {
	initialize(infile);
	processFile(infile);
	infile.createLinesFromTokens();
	return true;
}



//////////////////////////////
//
// Tool_satb2gs::initialize -- extract time signature lines for
//    each **kern spine in file.
//

void Tool_satb2gs::initialize(HumdrumFile& infile) {
   // handle basic options:
   if (getBoolean("author")) {
      m_free_text << "Written by Craig Stuart Sapp, "
           << "craig@ccrma.stanford.edu, Feb 2011" << endl;
      exit(0);
   } else if (getBoolean("version")) {
      m_free_text << getCommand() << ", version: 16 Dec 2016" << endl;
      m_free_text << "compiled: " << __DATE__ << endl;
      exit(0);
   } else if (getBoolean("help")) {
      usage(getCommand());
      exit(0);
   } else if (getBoolean("example")) {
      example();
      exit(0);
   }

   debugQ     =  getBoolean("debug");
}



//////////////////////////////
//
// Tool_satb2gs::processFile -- data is assumed to be in the order from
// bass, tenor, alto, soprano, with non-**kern data found
// in any order.  Only the first four **kern spines in the file
// will be considered.
//

void Tool_satb2gs::processFile(HumdrumFile& infile) {
	vector<int> satbtracks;
	satbtracks.resize(4);
	int exinterpline = getSatbTracks(satbtracks, infile);
	int lastline = -1;
	for (int i=0; i<exinterpline; i++) {
		m_humdrum_text << infile[i] << endl;
	}

	printExInterp(infile, exinterpline, satbtracks);

	for (int i=exinterpline+1; i<infile.getLineCount(); i++) {
		if (infile[i].getFieldCount() == 1) {
			m_humdrum_text << infile[i] << endl;
			continue;
		}
		if (*infile.token(i, 0) == "*-") {
			lastline = i;
			break;
		}
		for (int j=0; j<infile[i].getFieldCount(); j++) {
			printSpine(infile, i, j, satbtracks);
			if (j < infile[i].getFieldCount() - 1) {
				m_humdrum_text << '\t';
			}
		}
		m_humdrum_text << '\n';
	}

	if (lastline < 0) {
		return;
	}
	printLastLine(infile, lastline, satbtracks);

	for (int i=lastline+1; i<infile.getLineCount(); i++) {
		m_humdrum_text << infile[i] << endl;
	}
}



//////////////////////////////
//
// Tool_satb2gs::printLastLine --
//

void Tool_satb2gs::printLastLine(HumdrumFile& infile, int line, vector<int>& tracks) {
	int track;

	stringstream output;
	for (int j=0; j<infile[line].getFieldCount() - 1; j++) {
		track = infile.token(line, j)->getTrack();
		if ((track == tracks[1]) || (track == tracks[3])) {
			continue;
		}
		if (track == tracks[0])  {
			output << "*v\t*v";
		} else if (track == tracks[2])  {
			output << "*\t*";
		} else {
			output << "*";
		}
		output << "\t";
	}

	string strang = output.str();
	HumRegex hre;
	hre.replaceDestructive(strang, "", "\t+$");
	m_humdrum_text << strang;
	m_humdrum_text << endl;

	stringstream output2;
	for (int j=0; j<infile[line].getFieldCount() - 1; j++) {
		track = infile.token(line, j)->getTrack();
		if ((track == tracks[1]) || (track == tracks[3])) {
			continue;
		}
		if (track == tracks[2])  {
			output2 << "*v\t*v";
		} else if (track == tracks[0])  {
			output2 << "*";
		} else {
			output2 << "*";
		}
		output2 << "\t";
	}

	output2 << ends;
	strang = output2.str();
	hre.replaceDestructive(strang, "", "\t+$");
	m_humdrum_text << strang;
	m_humdrum_text << endl;

	for (int j=0; j<infile[line].getFieldCount()-2; j++) {
		m_humdrum_text << infile.token(line, j);
		if (j < infile[line].getFieldCount() - 3) {
			m_humdrum_text << "\t";
		}
	}
	m_humdrum_text << "\n";

}



//////////////////////////////
//
// Tool_satb2gs::printExInterp -- print only tenor and soprano tracks
//

void Tool_satb2gs::printExInterp(HumdrumFile& infile, int line,
		vector<int>& tracks) {
	stringstream output;
	int j;
	int track;

	// first print exclusive interpretations
	for (j=0; j<infile[line].getFieldCount(); j++) {
		track = infile.token(line, j)->getTrack();
		if ((track == tracks[1]) || (track == tracks[3])) {
			continue;
		}
		output << infile.token(line, j) << "\t";
	}
	string strang = output.str();
	HumRegex hre;
	hre.replaceDestructive(strang, "", "\t+$");
	m_humdrum_text << strang;
	m_humdrum_text << endl;

	stringstream output2;
	stringstream output3;
	for (j=0; j<infile[line].getFieldCount(); j++) {
		track = infile.token(line, j)->getTrack();
		if ((track == tracks[1]) || (track == tracks[3])) {
			continue;
		}
		if (track == tracks[0]) {
			output3 << "*clefF4";
			output2 << "*^";
		} else if (track == tracks[2]) {
			output3 << "*clefG2";
			output2 << "*^";
		} else {
			output3 << "*";
			output2 << "*";
		}
		output3 << "\t";
		output2 << "\t";
	}

	strang = output3.str();
	hre.replaceDestructive(strang, "", "\t+$");
	m_humdrum_text << strang;
	m_humdrum_text << endl;

	strang = output2.str();
	hre.replaceDestructive(strang, "", "\t+$");
	m_humdrum_text << strang;
	m_humdrum_text << endl;

}



///////////////////////
//
// Tool_satb2gs::printSpine --
//

void Tool_satb2gs::printSpine(HumdrumFile& infile, int row, int col,
		vector<int>& satbtracks) {
	int track = infile.token(row, col)->getTrack();
	int target = -1;
	for (int k=0; k<(int)satbtracks.size(); k++) {
		if (track == satbtracks[k]) {
			if (k % 2 == 0) {
				target = satbtracks[k+1];
			} else {
				target = satbtracks[k-1];
			}
			break;
		}
	}

	if (target < 0) {
		// does not need to be switched
		m_humdrum_text << infile.token(row, col);
		return;
	}

	// print the SAT or B token(s) (there should be only one token for each)
	//
	// If a tenor has a fermata and a bass has a fermata and both are
	// the same duration, then hide the tenor's fermata.
	//
	// If an alto has a fermata and a soprano has a fermata and both are
	// the same duration, then hide the alto's fermata.
	//


	// first identify the column for each voice, considering only the first
	// layer, if there are multiple layers.
	vector<int> cols(4);
	fill(cols.begin(), cols.end(), -1);
	for (int j=0; j<infile[row].getFieldCount(); j++) {
		track = infile.token(row, j)->getTrack();
		for (int k=0; k<(int)satbtracks.size(); k++) {
			if (cols[k] >= 0) {
				continue;
			}
			if (track == satbtracks[k]) {
				cols[k] = j;
			}
		}
	}


	HumRegex hre;
	string strang;
	int count = 0;
	bool foundnames = false;
	bool foundabbreviations = false;
	for (int j=0; j<infile[row].getFieldCount(); j++) {
		track = infile.token(row, j)->getTrack();
		if (track == target) {
			if (count > 0) {
				m_humdrum_text << '\t';
			}
			strang = *infile.token(row,j);
			hre.replaceDestructive(strang, "!*clef", "^\\*clef");
			if ((!foundnames) && hre.search(strang, R"(^\*I")")) {
				foundnames = true;
				hre.replaceDestructive(strang, R"(!*I"Soprano")", R"(^\*I"Soprano)");
				hre.replaceDestructive(strang, R"(!*I"Alto")"   , R"(^\*I"Alto)");
				hre.replaceDestructive(strang, R"(!*I"Tenor")"  , R"(^\*I"Tenor)");
				hre.replaceDestructive(strang, R"(!*I\"Bass")"  , R"(^\*I"Bass)");
			}
			if ((!foundabbreviations) && hre.search(strang, R"(^\*I')")) {
				foundabbreviations = true;
				hre.replaceDestructive(strang, R"(!*I'S")", R"(^\*I'S)");
				hre.replaceDestructive(strang, R"(!*I'A")", R"(^\*I'A)");
				hre.replaceDestructive(strang, R"(!*I'T")", R"(^\*I'T)");
				hre.replaceDestructive(strang, R"(!*I'B")", R"(^\*I'B)");
			}

			if (infile[row].isData()) {
				if ((cols[0] == col) &&
							(infile.token(row, col)->find(';') != string::npos)) {
					HumNum tenordur;
					HumNum bassdur;
					tenordur = Convert::recipToDuration(infile.token(row, cols[0]));
					bassdur  = Convert::recipToDuration(infile.token(row, cols[1]));
					if (tenordur == bassdur) {
						hre.replaceDestructive(strang, ";y", ";", "g"); // hide fermata
						// hre.replaceDestructive(strang, ";y", ";", "g"); // hide fermata
					}
				}

				if ((cols[3] == col) && (infile.token(row, col)->find(';') != string::npos)) {
					HumNum altodur;
					HumNum sopranodur;
					altodur = Convert::recipToDuration(infile.token(row, cols[3]));
					sopranodur  = Convert::recipToDuration(infile.token(row, cols[2]));
					if (altodur == sopranodur) {
						hre.replaceDestructive(strang, ";y", ";", "g"); // hide fermata
					}
				}

			}

			m_humdrum_text << strang;
			count++;
		}
	}
}



///////////////////////////////
//
// Tool_satb2gs::getSatbTracks -- return the primary track numbers of
//     the satb spines.
//

int Tool_satb2gs::getSatbTracks(vector<int>& tracks, HumdrumFile& infile) {
	tracks.clear();
	int output = -1;
	int track;
	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isInterpretation()) {
			continue;
		}
		output = i;
		for (int j=0; j<infile[i].getFieldCount(); j++) {
			if (!infile.token(i, j)->isKern()) {
				continue;
			}
			track = infile.token(i, j)->getTrack();
			tracks.push_back(track);
			if (tracks.size() == 4) {
				return output;
			}
		}
		break;
	}

	if (tracks.size() != 4) {
		m_error_text << "Error: there are " << tracks.size() << " **kern spines"
			  << " in input data (needs to be 4)" << endl;
		exit(1);
	}

	return output;
}




//////////////////////////////
//
// Tool_satb2gs::example -- example function calls to the program.
//

void Tool_satb2gs::example(void) {


}



//////////////////////////////
//
// Tool_satb2gs::usage -- command-line usage description and brief summary
//

void Tool_satb2gs::usage(const string& command) {

}



// END_MERGE

} // end namespace hum



