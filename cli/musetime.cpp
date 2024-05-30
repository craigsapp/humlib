//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Wed May 29 18:46:42 PDT 2024
// Last Modified: Wed May 29 18:46:46 PDT 2024
// Filename:      cli/musetime.cpp
// URL:           https://github.com/craigsapp/musetime/blob/master/src/musetime.cpp
// Syntax:        C++11
// vim:           ts=3 noexpandtab
//
// Description:   Command-line interface for converting MusicXML files into
//                Humdrum files.
//

#include "MuseDataSet.h"
#include "MuseData.h"
#include "Options.h"

#include <iostream>

using namespace std;
using namespace hum;

void processData(MuseDataSet& mds);
void processData(MuseData& md);

Options options;
bool floatQ = false;

int main(int argc, char** argv) {
	options.define("f|float=b", "Display quarter-note time as floating-point numbers");
	options.process(argc, argv);
	floatQ = options.getBoolean("float");
	MuseDataSet mds;
	int output = 1;
	if (options.getArgCount() == 0) {
		mds.readString(cin);
	} else {
		for (int i=0; i<options.getArgCount(); i++) {
			MuseData* md;
			md = new MuseData;
			output &= md->readFile(options.getArg(i+1));
			mds.appendPart(md);
		}
	}

	processData(mds);

	return !output;
}


//////////////////////////////
//
// processData -- List QStamp for each line.
//

void processData(MuseDataSet& mds) {
	for (int i=0; i<mds.getFileCount(); i++) {
		cout << "###########################################################" << endl;
		processData(mds[i]);
	}
	cout << "###########################################################" << endl;
}


// Print MuseData part contents with a quarter-note timestamp before each line.
void processData(MuseData& md) {
	for (int i=0; i<md.getLineCount(); i++) {
		if (floatQ) {
			cout << md[i].getQStamp().getFloat() << "\t" << md[i] << endl;
		} else {
			cout << md[i].getQStamp() << "\t" << md[i] << endl;
		}
	}
}


