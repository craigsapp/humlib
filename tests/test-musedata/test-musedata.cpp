#include "humlib.h"
// vim: ts=3


using namespace hum;
using namespace std;

void printKernInfo(MuseRecord& mr);

int main(int argc, char** argv) {
	Options options;
	options.process(argc, argv);
	MuseDataSet mds;
	if (options.getArgCount() >= 1) {
		mds.readFile(options.getArg(1));
	} else {
		cerr << "Usage: " << options.getCommand() << " file" << endl;
		exit(1);
	}

	// int partcount = mds.getPartCount();
	MuseData& md = mds[0];
	for (int i=0; i<md.getLineCount(); i++) {
		cout << "LINE:" << i 
		     << "\tABSQ:" << md.getAbsBeat(i).getFloat() << "\t" 
		     << "\tTDUR:" << md[i].getLineTickDuration() << "\t";
		printKernInfo(md[i]);
		cout << "\t" << md[i] << endl;
	}

	return 0;
}


void printKernInfo(MuseRecord& mr) {
	string output = ".";
	if (mr.isBarline()) {
		output = "bar";
	} else if (mr.isNote()) {
		output = "note";
	} else if (mr.isRest()) {
		output = "rest";
	}
	cout << output;
}


