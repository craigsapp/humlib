// 
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Apr 22 21:45:43 PDT 2023
// Last Modified: Sun Apr 23 19:02:58 PDT 2023
// Filename:      cli/mergetime.cpp
// Syntax:        C++11
// 
// Description:   Merge a Humdrum score and a SV time instant
//                annotation (in text/TSV format).  The first
//                column of the SV data is the time in seconds
//                to insert at the start of the Humdrum data lines.
//

/* Example:

input.krn:

**kern
4c
8dL
8eJ
4f
[4g
=
8g]
8g#
4a
4b
2c
==
*-

svdata.txt: 

0.4	1
1.1	2
1.4	3
2.5	4
4.9	5
6.7	6
8.2	7
9.1	8
10.0	9


command:

mergetime input.krn svdata.txt

output:

**time	**kern
0.4	4c
1.1	8dL
1.4	8eJ
2.5	4f
4.9	[4g
=	=
.	8g]
6.7	8g#
8.2	4a
9.1	4b
10	2c
==	==
*	*-

*/

#include "humlib.h"
#include <fstream>
#include <iostream>
#include <string>

using namespace hum;
using namespace std;

// function definitions:
void getSVData (vector<pair<double, string>>& output, const string& filename);
void merge     (HumdrumFile& infile, vector<pair<double, string>>& svdata);
   
// interface options
// bool m_durationQ = false;
         
///////////////////////////////////////////////////////////////////////////
      
int main(int argc, char** argv) {
   Options options;
   options.process(argc, argv);
   
	if (options.getArgCount() != 2) {
		cerr << "Usage: $0 humdrum-file sv-file" << endl;
		exit(1);
	}

	HumdrumFile infile;
	infile.read(options.getArg(1));
	vector<pair<double, string>> svdata;
	getSVData(svdata, options.getArg(2));
	merge(infile, svdata);
	
   return 0;
}


//////////////////////////////
//
// merge --
//

void merge(HumdrumFile& infile, vector<pair<double, string>>& svdata) {
	int counter = 0;
	if (svdata.empty()) {
		cerr << "Error: no time instants to insert" << endl;
		return;
	}

	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].hasSpines()) {
			cout << infile[i] << endl;
		} else if (infile[i].isCommentLocal()) {
			cout << "!\t" << infile[i] << endl;
		} else if (infile[i].isExclusiveInterpretation()) {
			cout << "**time\t" << infile[i] << endl;
		} else if (infile[i].isManipulator()) {
			cout << "*\t" << infile[i] << endl;
		} else if (infile[i].isInterpretation()) {
			HTp token = infile.token(i, 0);
			if (*token == "*-") {
			} else {
				cout << "*\t" << infile[i] << endl;
			}
		} else if (infile[i].isBarline()) {
			HTp token = infile.token(i, 0);
			cout << token << "\t" << infile[i] << endl;
		} else if (!infile[i].isData()) {
			cerr << "STRANGE PROBLEM ON LINE " << (i+1) << ": " << infile[i] << endl;
		}
		if (!infile[i].isData()) {
			continue;
		}
		int attacks = infile[i].getKernNoteAttacks();
		if (attacks == 0) {
			cout << ".\t" << infile[i] << endl;
			continue;
		}
		// insert next time instant into score.
		double seconds = -1.0;
		int index = counter++;
		if ((index < (int)svdata.size())) {
			seconds = svdata[index].first;
			seconds = int(seconds * 1000 + 0.5) / 1000.0;
		}
		cout << seconds << "\t" << infile[i] << endl;
	}
}



//////////////////////////////
//
// getSVData --
//

void getSVData(vector<pair<double, string>>& output, const string& filename) {
	output.clear();

	ifstream infile(filename);
	if (!infile.is_open()) {
		cerr << "Cannot read SV annotation file " << filename << endl;
		exit(1);
	}
	string line;
	HumRegex hre;
	while (getline(infile, line)) {
		if  (hre.search(line, R"(^([\d.]+)\s+(.*)\s*$)")) {
			double seconds = hre.getMatchDouble(1);
			string label = hre.getMatch(2);
			output.emplace_back(seconds, label);
		}
	}
	// cerr << "THERE ARE " << output.size() << " POINTS" << endl;
}



