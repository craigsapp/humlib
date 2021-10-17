//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Fri Apr 16 09:47:57 PDT 2021
// Last Modified: Fri Apr 16 19:57:21 PDT 2021
// Filename:      activevoices.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/cli/activevoices.cpp
// Syntax:        C++11
// vim:           ts=3 noexpandtab nowrap
//
// Description:   Create a spine showing the number of voices, both resting and sounding
//                for a section of music bounded by double barlines.  This is to
//                be used with JRP data where mass sections are subdivided into subsections
//                which may have different voice counts from section to section, with
//                the unused voices being all rests in the subsection (in which case
//                they will be removed from the active-voices count).
//

#include "humlib.h"

using namespace std;
using namespace hum;

void processFile(HumdrumFile &infile);
vector<int> getSectionStarts(HumdrumFile& infile);
vector<int> getVoiceCounts(HumdrumFile& infile, vector<int>& sections);
int         getActiveVoicesInRange(HumdrumFile& infile, int startline, int endline);



int main(int argc, char **argv) {
  Options options;
  options.process(argc, argv);

  HumdrumFileStream instream(options);
  HumdrumFile infile;
  while (instream.read(infile)) {
	 processFile(infile);
  }

  return 0;
}



//////////////////////////////
//
// processFile --
//

void processFile(HumdrumFile &infile) {
	vector<int> sectionStarts = getSectionStarts(infile);
	vector<int> voicecount    = getVoiceCounts(infile, sectionStarts);

	vector<int> spinedata(infile.getLineCount(), -1);
	for (int i=0; i<(int)sectionStarts.size(); i++) {
		int starting = sectionStarts[i];
		int ending   = infile.getLineCount() - 1;
		if (i < (int)sectionStarts.size() - 1) {
			ending = sectionStarts.at(i+1);
		}
		for (int j=starting; j<=ending; j++) {
			spinedata.at(j) = voicecount[i];
		}
	}

	// for (int i=0; i<sectionStarts.size(); i++) {
	// 	cerr << "SEGMENT START " << sectionStarts[i] << "\t" << voicecount[i] << endl;
	// }

	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].hasSpines()) {
			cout << infile[i] << endl;
			continue;
		}
		if (infile[i].isInterpretation()) {
			HTp token = infile.token(i, 0);
			if (*token == "*-") {
				cout << "*-";
			} else if (token->compare(0, 2, "**") == 0) {
				cout << "**avoc";
			} else {
				cout << "*";
			}
		} else if (infile[i].isBarline()) {
			cout << infile.token(i, 0);
		} else if (infile[i].isComment()) {
			cout << "!";
		} else if (infile[i].isData()) {
			cout << spinedata.at(i);
		} else {
			cerr << "STRANGE PROBLEM" << endl;
		}
		cout << "\t" << infile[i] << endl;
	}
}



//////////////////////////////
//
// getVoiceCounts --
//

vector<int> getVoiceCounts(HumdrumFile& infile, vector<int>& sections) {
	if (sections.empty()) {
		vector<int> output;
		return output;
	}

	vector<int> activevoices(sections.size(), 0);
	int lastindex = infile.getLineCount() - 1;
	int endline;
	int startline;

	for (int i=0; i<(int)sections.size(); i++) {
		startline = sections[i];
		if (i < (int)sections.size() - 1) {
			endline = sections[i+1];
		} else {
			endline = lastindex;
		}
		activevoices[i] = getActiveVoicesInRange(infile, startline, endline);
	}

	return activevoices;
}



//////////////////////////////
//
// getActiveVoicesInRange --
//

int getActiveVoicesInRange(HumdrumFile& infile, int startline, int endline) {
	vector<int> tracks(infile.getMaxTrack() + 1, 0);
	for (int i=startline; i<=endline; i++) {
		if (!infile[i].isData()) {
			continue;
		}
		for (int j=0; j<infile[i].getFieldCount(); j++) {
			HTp token = infile.token(i, j);
			if (!token->isKern()) {
				continue;
			}
			if (token->isNull()) {
				continue;
			}
			if (token->isRest()) {
				continue;
			}
			int track = token->getTrack();
			tracks[track]++;
		}
	}

	int sum = 0;
	for (int i=0; i<(int)tracks.size(); i++) {
		if (tracks[i]) {
			sum++;
		}
	}
	return sum;
}



//////////////////////////////
//
// getSectionStarts --
//

vector<int> getSectionStarts(HumdrumFile& infile) {
	vector<int> output;
	output.push_back(0);
	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isBarline()) {
			continue;
		}
		HTp token = infile.token(i, 0);
		if (token->find("||") != string::npos) {
			output.push_back(i);
		} else if (token->find("==") != string::npos) {
			output.push_back(i);
		}
	}

	if (infile.token(output.back(), 0)->getDurationToEnd() == 0) {
		output.resize((int)output.size() - 1);
	}

	return output;
}




