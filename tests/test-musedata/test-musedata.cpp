#include "humlib.h"
// vim: ts=3


using namespace hum;
using namespace std;

void printKernInfo(MuseRecord& mr, int tpq);

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
	int tpq = md.getInitialTpq();

	for (int i=0; i<md.getLineCount(); i++) {
		cout << "LINE:" << i 
		     << "\tABSQ:" << md.getAbsBeat(i).getFloat() << "\t" 
		     << "\tTDUR:" << md[i].getLineTickDuration() << "\t";
		printKernInfo(md[i], tpq);
		cout << "\t" << md[i] << endl;
	}

	return 0;
}


void printKernInfo(MuseRecord& mr, int tpq) {
	string output = ".";
	if (mr.isBarline()) {
		output = mr.getKernMeasureStyle();
	} else if (mr.isNote()) {
		output = mr.getKernNoteStyle(1, 1);
	} else if (mr.isRest()) {
		output = mr.getKernRestStyle(tpq);
	}
	cout << output;
}


