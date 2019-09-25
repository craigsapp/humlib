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

	cout << mds;

	return 0;
}
