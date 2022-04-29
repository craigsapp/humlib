//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Jul 18 11:23:42 PDT 2005
// Last Modified: Mon Jul 18 11:23:46 PDT 2005
// Last Modified: Sun Mar  2 18:58:48 PST 2008 Added -l and -i options
// Last Modified: Mon Mar  3 13:46:34 PST 2008 Added -r option
// Last Modified: Tue Apr  9 08:18:06 PDT 2013 Enabled multiple segment input
// Last Modified: Thu Apr 28 18:28:47 PDT 2022 Ported to humlib parser.
// Filename:      ...sig/examples/all/thrux.cpp
// Web Address:   http://sig.sapp.org/examples/museinfo/humdrum/thrux.cpp
// Syntax:        C++; museinfo
//
// Description:   C++ implementation of the Humdrum Toolkit thru command.
//

#include "humlib.h"

#include <stdlib.h>
#include <string.h>
#include <iostream>

using namespace std;
using namespace hum;

// function declarations
void      checkOptions        (Options& opts, int argc, char* argv[]);
void      example             (void);
void      processData         (HumdrumFile& infile);
void      usage               (const char* command);
void      getLabelSequence    (vector<string>& labelsequence,
                               const string& astring);
int       getLabelIndex       (vector<string>& labels, string& key);
void      printLabelList      (HumdrumFile& infile);
void      printLabelInfo      (HumdrumFile& infile);
int       getBarline          (HumdrumFile& infile, int line);
int       adjustFirstBarline  (HumdrumFile& infile);
void      processFile         (HumdrumFile& infile);


// global variables
Options      options;            // database for command-line arguments
string       variation = "";     // used with -v option
int          listQ = 0;          // used with -l option
int          infoQ = 0;          // used with -i option
int          keepQ = 0;          // used with -k option
string       realization = "";   // used with -r option


///////////////////////////////////////////////////////////////////////////


int main(int argc, char* argv[]) {
	// process the command-line options
	checkOptions(options, argc, argv);
	options.process(argc, argv);

	HumdrumFileStream instream(options);
	HumdrumFile infile;
	while (instream.read(infile)) {
		processFile(infile);
	}
	return 0;
}



void processFile(HumdrumFile& infile) {
	if (listQ) {
		printLabelList(infile);
		return;
	} else if (infoQ) {
		printLabelInfo(infile);
		return;
	}
	processData(infile);

	// analyze the input file according to command-line options
	// infiles[i].printNonemptySegmentLabel(cout);
}


///////////////////////////////////////////////////////////////////////////

//////////////////////////////
//
// printLabelList -- print a list of the thru labels.
//


void printLabelList(HumdrumFile& infile) {
	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isInterpretation()) {
			continue;
		}
		HTp token = infile.token(i, 0);
		if (token->compare(0, 2, "*>") != 0) {
			continue;   // ignore non-labels
		}
		//if (token->find('[') != NULL) {
		//   continue;   // ignore realizations
		//}
		cout << token->substr(2);
		cout << '\n';
	}

}



//////////////////////////////
//
// printLabelInfo -- print a list of the thru labels.
//


void printLabelInfo(HumdrumFile& infile) {
	// infile.analyzeRhythm();
	vector<int> labellines;
	labellines.reserve(1000);

	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isInterpretation()) {
			continue;
		}
		HTp token = infile.token(i, 0);

		if (token->compare(0, 2, "*>") != 0) {
			continue;   // ignore non-labels
		}
		if (token->find('[') != string::npos) {
			cout << "!!>";
			cout << token->substr(2) << endl;
			cout << '\n';
			continue;   // ignore realizations
		}
		labellines.push_back(i);
	}


	vector<int> barlines(1000, -1);
	for (int i=0; i<(int)labellines.size(); i++) {
		barlines[i] = getBarline(infile, labellines[i]);
	}

	if (barlines.size() > 0) {
		barlines[0] = adjustFirstBarline(infile);
	}

	int startline;
	int endline;
	HumNum startbeat;
	HumNum endbeat;
	HumNum duration;

	cout << "**label\t**sline\t**eline\t**sbeat\t**ebeat\t**dur\t**bar\n";
	for (int i=0; i<(int)labellines.size(); i++) {
		startline = labellines[i];
		if (i<(int)labellines.size()-1) {
			endline = labellines[i+1]-1;
		} else {
			endline = infile.getLineCount() - 1;
		}
		startbeat = infile[startline].getDurationFromStart();
		endbeat = infile[endline].getDurationFromStart();
		duration = endbeat - startbeat;
		duration = int(duration.getFloat() * 10000.0 + 0.5) / 10000.0;
		HTp token = infile.token(startline, 0);
		cout << token->substr(2);
		cout << '\t';
		cout << startline + 1;
		cout << '\t';
		cout << endline + 1;
		cout << '\t';
		cout << startbeat;
		cout << '\t';
		cout << endbeat;
		cout << '\t';
		cout << duration;
		cout << '\t';
		cout << barlines[i];
		cout << '\n';

	}
	cout << "*-\t*-\t*-\t*-\t*-\t*-\t*-\n";

}



//////////////////////////////
//
// adjustFirstBarline --
//

int adjustFirstBarline(HumdrumFile& infile) {
	int i;
	int number = 0;
	HumRegex hre;
	for (i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isBarline()) {
			continue;
		}
		if (infile[i].getDurationFromStart() > 0) {
			break;
		}
		HTp token = infile.token(i, 0);
		if (hre.search(token, "=.*(\\d+)")) {
			number = hre.getMatchInt(1);
		}
		break;
	}
	return number;
}



//////////////////////////////
//
// getBarline --
//

int getBarline(HumdrumFile& infile, int line) {

	if (infile[line].getDurationFromStart() == 0) {
		return 0;
	}

	int i;
	int missingcount = 0;
	int number = -1;
	HumRegex hre;
	for (i=line; i>0; i--) {
		if (!infile[i].isBarline()) {
			continue;
		}
		HTp token = infile.token(i, 0);

		if (hre.search(token, "=.*(\\d+)")) {
			number = hre.getMatchInt(1);
			break;
		} else {
			missingcount++;
		}
		if (missingcount > 1) {
			break;
		}
	}

	return number;
}



//////////////////////////////
//
// checkOptions -- validate and process command-line options.
//

void checkOptions(Options& opts, int argc, char* argv[]) {
	opts.define("v|variation=s:", "Choose the expansion variation");
	opts.define("l|list=b:", "Print list of labels in file");
	opts.define("k|keep=b:", "Keep variation interpretations");
	opts.define("i|info=b:", "Print info list of labels in file");
	opts.define("r|realization=s:", "alternate relaization label sequence");

	opts.define("d|debug=b");                    // determine bad input line num
	opts.define("author=b");                     // author of program
	opts.define("version=b");                    // compilation info
	opts.define("example=b");                    // example usages
	opts.define("h|help=b");                     // short description
	opts.process(argc, argv);

	// handle basic options:
	variation   = opts.getString("variation");
	realization = opts.getString("realization");
	listQ       = opts.getBoolean("list");
	infoQ       = opts.getBoolean("info");
	keepQ       = opts.getBoolean("keep");
}



//////////////////////////////
//
// processData --
//

void processData(HumdrumFile& infile) {

	vector<string> labelsequence;
	labelsequence.reserve(1000);

	vector<string> labels;
	labels.reserve(1000);

	vector<int> startline;
	startline.reserve(1000);

	vector<int> stopline;
	stopline.reserve(1000);

	int header = -1;
	int footer = -1;
	char labelsearch[1024] = {0};
	strcpy(labelsearch, "*>");
	strcat(labelsearch, variation.c_str());
	strcat(labelsearch, "[");
	int length = strlen(labelsearch);

	// check for label to expand
	int i;
	int foundlabel = 0;
	string tempseq;
	if (realization.size()  == 0) {
		for (i=0; i<infile.getLineCount(); i++) {
			if (!infile[i].isInterpretation()) {
				continue;
			}
			HTp token = infile.token(i, 0);
			if (token->compare(0, length, labelsearch) != 0) {
				continue;
			}

			tempseq = token->substr(length);
			getLabelSequence(labelsequence, tempseq);
			foundlabel = 1;
			break;
		}
	} else {
		foundlabel = 1;
		getLabelSequence(labelsequence, realization);
	}


	int j;
	if (foundlabel == 0) {
		// did not find the label to expand, so echo the data back
		for (i=0; i<infile.getLineCount(); i++) {
			HTp token = infile.token(i, 0);
			if (*token == "*thru") {
				continue;
			}
			cout << infile[i] << "\n";
			if (token->compare(0, 2, "**") == 0) {
				for (j=0; j<infile[i].getFieldCount(); j++) {
					cout << "*thru";
					if (j < infile[i].getFieldCount() - 1) {
						cout << "\t";
					}
				}
				cout << "\n";
			}
		}
		return;
	}

	// for (i=0; i<(int)labelsequence.size(); i++) {
	//    cout << i+1 << "\t=\t" << labelsequence[i] << endl;
	// }

	// search for the labeled sections in the music
	string label;
	int   location;
	int   index;
	for (i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isInterpretation()) {
			continue;
		}
		HTp token = infile.token(i, 0);
		if (*token == "*-") {
			location = i-1;
			footer = i;
			stopline.push_back(location);
		}
		if (token->compare(0, 2, "*>") != 0) {
			continue;
		}
		if (token->find('[') != string::npos) {
			continue;
		}
		if (token->find(']') != string::npos) {
			continue;
		}

		if (labels.size() == 0) {
			header = i-1;
		}

		label = token->substr(2);
		index = (int)labels.size();
		location = i-1;
		if (startline.size() > 0) {
			stopline.push_back(location);
		}
		labels.resize(index+1);
		labels[index] = label;
		startline.push_back(i);
	}

	// cout << "FOOTER = " << footer << endl;
	// cout << "HEADER = " << header << endl;
	// for (i=0; i<(int)labels.size(); i++) {
	//    cout << "\t" << i << "\t=\t" << labels[i]
	//         << "\t" << startline[i] << "\t" << stopline[i]
	//         << endl;
	// }

	// now ready to copy the labeled segements into a final file.


	// print header:
	for (i=0; i<=header; i++) {
		HTp token = infile.token(i, 0);
		if (*token == "*thru") {
			continue;
		}

		if (!keepQ) {
			if (infile[i].isInterpretation()) {
				if (token->compare(0, 2, "*>") == 0) {
               if (token->find('[') != string::npos) {
						continue;
					}
				}
			}
		}

		cout << infile[i] << "\n";
		if (token->compare(0, 2, "**") == 0) {
			for (j=0; j<infile[i].getFieldCount(); j++) {
				cout << "*thru";
				if (j < infile[i].getFieldCount() - 1) {
					cout << "\t";
				}
			}
			cout << "\n";
		}
	}

	int start;
	int stop;
	for (i=0; i<(int)labelsequence.size(); i++) {
		index = getLabelIndex(labels, labelsequence[i]);
		if (index < 0) {
			cout << "!! THRU ERROR: label " << labelsequence[i]
				  << " does not exist, skipping.\n";
		}
		start = startline[index];
		stop  = stopline[index];
		for (j=start; j<=stop; j++) {
			if (!keepQ) {
				if (infile[j].isInterpretation()) {
					HTp token = infile.token(j, 0);
					if (token->compare(0, 2, "*>") == 0) {
                  if (token->find('[') != string::npos) {
							continue;
						}
					}
				}
			}
			cout << infile[j] << "\n";
		}
	}

	// print footer:
	for (i=footer; i<infile.getLineCount(); i++) {
		if (!keepQ) {
			if (infile[i].isInterpretation()) {
				HTp token = infile.token(i, 0);
				if (token->compare(0, 2, "*>") == 0) {
					if (token->find('[') != string::npos) {
						continue;
					}
				}
			}
		}
		cout << infile[i] << "\n";
	}

}



//////////////////////////////
//
// getLabelIndex --
//

int getLabelIndex(vector<string>& labels, string& key) {
	for (int i=0; i<(int)labels.size(); i++) {
		if (key == labels[i]) {
			return i;
		}
	}
	return -1;
}



//////////////////////////////
//
// getLabelSequence --
//

void getLabelSequence(vector<string>& labelsequence,
		const string& astring) {
	int slength = (int)astring.size();
	char* sdata = new char[slength+1];
	strcpy(sdata, astring.c_str());
	const char* ignorecharacters = ", [] ";
	int index;

	char* strptr = strtok(sdata, ignorecharacters);
	while (strptr != NULL) {
		labelsequence.resize((int)labelsequence.size() + 1);
		index = (int)labelsequence.size() - 1;
		labelsequence[index] = strptr;
		strptr = strtok(NULL, ignorecharacters);
	}

	delete [] sdata;
}

