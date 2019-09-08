
#include "humlib.h"

using namespace std;
using namespace hum;

void processFile(HumdrumFile& infile);

int main(int argc, char** argv) {
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

void processFile(HumdrumFile& infile) {
	infile.analyzeNonNullDataTokens();
	for (int i=0; i<infile.getLineCount(); i++) {
		HTp token = infile.token(i, 0);
cerr << "LINE " << i << endl;
		cout << "\t" << infile[i] << "\t" << token->getNextNNDT() << endl;
	}
}



