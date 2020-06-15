//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sun Jun 14 10:26:30 PDT 2020
// Last Modified: Mon Jun 15 06:55:26 PDT 2020
// Filename:      rhythms.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/cli/rhythms.cpp
// Syntax:        C++11
// vim:           ts=3 noexpandtab nowrap
//
// Description:   List rhythms of notes/rests in input data.
//
// To do:         Add tied-note analyses.
//

#include "humlib.h"
#include <iostream>
#include <map>

using namespace hum;
using namespace std;

void  processFile        (HumdrumFile& infile);
bool  processStrand      (map<string, int>& rhythms, map<HumNum, int>& durations,
                          HTp sstart, HTp send);
bool  processKernString  (const string& subtoken, map<string, int>& rhythms, map<HumNum, int>& durations);
void  printRhythms       (HumdrumFile& infile, map<string, int>& f_rhys);
void  printDurations     (HumdrumFile& infile, map<HumNum, int>& f_durs);
void  printAllRhythms    (void);
void  printAllDurations  (void);
int   getSum             (map<string, int>& histogram);
int   getSum             (map<HumNum, int>& histogram);

map<string, int> m_rhys;  // rhythm histogram for all files.
map<HumNum, int> m_durs;  // duration histogram for all files.

bool m_tuplet    = false; // used with -t option
bool m_chord     = false; // used with -c option
bool m_durations = false; // used with -d option
bool m_all       = false; // used with -a option
bool m_percent   = false; // used with -p option
bool m_search    = false; // used with -s option
bool m_no_rests  = false; // used with -R option
string m_query   = "";    // used with -s option

///////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv) {
	Options options;
	options.define("c|chord=b", "treat chords as single notes (count once)");
	options.define("d|dur|durs|duration|durations=b", "process durations instead of rhythms");
	options.define("a|all=b", "only print data for all input files");
	options.define("p|percent=b", "print rhythm counts as a precentage of all rhythms");
	options.define("s|search=s", "print names of files with given rhythm");
	options.define("t|tuplet=b", "Print filename if tuplet present in file");
	options.define("R|no-rests=b", "ignore rests in analysis");
	options.process(argc, argv);

	m_tuplet    = options.getBoolean("tuplet");
	m_chord     = options.getBoolean("chord");
	m_durations = options.getBoolean("duration");
	m_all       = options.getBoolean("all");
	m_percent   = options.getBoolean("percent");
	m_search    = options.getBoolean("search");
	m_no_rests  = options.getBoolean("no-rests");
	m_query     = options.getString("search");

	HumdrumFileStream instream(options);
	HumdrumFile infile;
	while (instream.read(infile)) {
		processFile(infile);
	}

	if (!m_search && m_all) {
		if (m_durations) {
			printAllDurations();
		} else {
			printAllRhythms();
		}
	}

	return 0;
}



//////////////////////////////
//
// processFile --
//

void processFile(HumdrumFile& infile) {
	int scount = infile.getStrandCount();
	map<HumNum, int> f_durs;
	map<string, int> f_rhys;
	for (int i=0; i<scount; i++) {
		HTp sstart = infile.getStrandStart(i);
		if (!sstart->isKern()) {
			continue;
		}
		HTp send = infile.getStrandStop(i);
		int status = processStrand(f_rhys, f_durs, sstart, send);
		if (m_tuplet && status) {
			cout << infile.getFilenameBase() << endl;
			return;
		}
		if (m_search && status) {
			cout << infile.getFilenameBase() << endl;
			return;
		}
	}
	if (!m_search) {
		if (!m_tuplet && !m_all) {
			if (m_durations) {
				printDurations(infile, f_durs);
			} else {
				printRhythms(infile, f_rhys);
			}
		}
	}
}



//////////////////////////////
//
// printRhythms --
//

void printRhythms(HumdrumFile& infile, map<string, int>& rhys) {
	cout << infile.getFilenameBase() << endl;
	double factor = 1.0;
	if (m_percent) {
		factor = 100.0 / getSum(rhys);
	}
	for (auto element : rhys) {
		cout << "\t" << element.first << "\t";
		cout << element.second * factor << endl;
	}
}



//////////////////////////////
//
// printAllRhythms --
//

void printAllRhythms(void) {
	double factor = 1.0;
	if (m_percent) {
		factor = 100.0 / getSum(m_rhys);
	}
	for (auto element : m_rhys) {
		cout << element.first << "\t";
		cout << element.second * factor << endl;
	}
}



//////////////////////////////
//
// printDurations --
//

void printDurations(HumdrumFile& infile, map<HumNum, int>& durs) {
	cout << infile.getFilenameBase() << endl;
	double factor = 1.0;
	if (m_percent) {
		factor = 100.0 / getSum(durs);
	}
	for (auto element : durs) {
		string recip = Convert::durationToRecip(element.first);
		cout << "\t" << element.first << "\t";
		cout << recip << "\t";
		cout << element.second * factor << endl;
	}
}



//////////////////////////////
//
// printAllDurations --
//

void printAllDurations(void) {
	double factor = 1.0;
	if (m_percent) {
		factor = 100.0 / getSum(m_durs);
	}
	for (auto element : m_durs) {
		string recip = Convert::durationToRecip(element.first);
		cout << element.first << "\t";
		cout << recip << "\t";
		cout << element.second * factor << endl;
	}
}



////////////////////
//
// processStrand -- Returns true if a search match was found.
//

bool processStrand(map<string, int>& rhythms, map<HumNum, int>& durations,
		HTp sstart, HTp send) {
	HTp current = sstart;
	while (current && (current != send)) {
		if (current->isNull()) {
			current = current->getNextToken();
			continue;
		}
		if (!current->isData()) {
			current = current->getNextToken();
			continue;
		}
		if (m_no_rests && current->isRest()) {
			continue;
		}
		if (m_chord) {
			// treat chord as a single unit
			int status = processKernString(*current, rhythms, durations);
			if (status) {
				return true;
			}
		} else {
			// process individual notes in chord
			vector<string> subtokens = current->getSubtokens();
			for (int i=0; i<(int)subtokens.size(); i++) {
				bool status = processKernString(subtokens[i], rhythms, durations);
				if (status) {
					return true;
				}
			
			}
		}
		current = current->getNextToken();
	}
	return false;
}



//////////////////////////////
//
// processKernString --
//

bool processKernString(const string& subtoken, map<string, int>& rhythms, map<HumNum, int>& durations) {
	if (m_tuplet) {
		HumNum nodots = Convert::recipToDurationNoDots(subtoken);
		if ((nodots != 0) && (!nodots.isPowerOfTwo())) {
			return true;
		}
	}
	string s = Convert::kernToRecip(subtoken);
	if (s.empty()) {
		s = "null";
	}
	if (m_search) {
		if (m_query == s) {
			return true;
		}
	}
	HumNum d = Convert::recipToDuration(subtoken);
	rhythms[s]++;
	m_rhys[s]++;
	durations[d]++;
	m_durs[d]++;

	return false;
}



//////////////////////////////
//
// getSum -- Calculate sum of counts in histogram.
//

int getSum(map<string, int>& histogram) { 
	int output = 0;
	for (auto element : histogram) {
		output += element.second;
	}
	return output;
}


int getSum(map<HumNum, int>& histogram) { 
	int output = 0;
	for (auto element : histogram) {
		output += element.second;
	}
	return output;
}



