#include "humlib.h"
// vim: ts=3


using namespace hum;
using namespace std;

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

	int partcount = mds.getPartCount();
	cerr << "THERE ARE " << partcount << " parts in data" << endl;
	MuseData& md = mds[0];
	for (int i=0; i<md.getLineCount(); i++) {
		cout << "LINE:" << i 
		     << "\tABSQ:" << md.getAbsBeat(i).getFloat() << "\t" 
		     << "\tTDUR:" << md[i].getLineTickDuration() << "\t" 
		     << md[i] << endl;
	}

	return 0;
}
