//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon 10 Feb 2020 08:54:53 PM PST
// Last Modified: Mon 10 Feb 2020 08:54:57 PM PST
// Filename:      markov.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/cli/markov.cpp
// Syntax:        C++11
// vim:           ts=3 noexpandtab nowrap
//
// Description:   Generate raw data for markov analysis.
//

#include "humlib.h"
#include <iostream>

using namespace hum;
using namespace std;

void           processFile       (HumdrumFile& infile, Options& options);
vector<string> getVoiceNames     (vector<HTp>& starts);
void           getPitchSequence  (vector<string>& features, HTp start);


///////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv) {
	Options options;
	options.process(argc, argv);
	HumdrumFileStream instream(options);
	HumdrumFile infile;
	while (instream.read(infile)) {
		processFile(infile, options);
	}
	return 0;
}



//////////////////////////////
//
// processFile --
//

void processFile(HumdrumFile& infile, Options& options) {
   vector<HTp> starts = infile.getKernSpineStartList();
	vector<vector<string>> features(starts.size());
	for (int i=0; i<(int)features.size(); i++) {
		features[i].reserve(infile.getLineCount());
	}

	for (int i=0; i<(int)starts.size(); i++) {
		getPitchSequence(features[i], starts[i]);
	}

	vector<string> voices = getVoiceNames(starts);
	string filename = infile.getFilenameBase();
	for (int i=0; i<(int)voices.size(); i++) {
		cout << filename << "\t" << voices[i] << "\t";
		for (int j=0; j<(int)features[i].size(); j++) {
			cout << features[i][j];
			if (j < (int)features[i].size() - 1) {
				cout << ' ';
			}
		}
		cout << endl;
	}
}



//////////////////////////////
//
// getPitchSequence --
//

void getPitchSequence(vector<string>& features, HTp start) {
	HTp current = start;
	while (current) {
		if (!current->isData()) {
			current = current->getNextToken();
			continue;
		}
		if (current->isNull()) {
			current = current->getNextToken();
			continue;
		}
		if (current->isRest()) {
			if ((!features.empty()) && (features.back() != "r")) {
				features.push_back("r");
			}
			current = current->getNextToken();
			continue;
		}
		int value = Convert::kernToBase40(current);
		string svalue = Convert::base40ToKern(value);
		features.push_back(svalue);
		current = current->getNextToken();
	}
}



//////////////////////////////
//
// getVoiceNames -- Return the names of the voices for each spine.
//

vector<string> getVoiceNames(vector<HTp>& starts) {
	vector<string> output;
	for (int i=0; i<(int)starts.size(); i++) {
		string value = "";
		HTp current = starts[i];
		while (current) {
			if (current->isInstrumentName()) {
				value = current->getInstrumentName();
				break;
			}
			if (current->isData()) {
				break;
			}
			current = current->getNextToken();
		}
		if (value.empty())  {
			value = ".";
		}
		output.push_back(value);
	}
	return output;
}


