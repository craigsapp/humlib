

#include "humlib.h"


using namespace std;
using namespace hum;

void processFile(HumdrumFile& infile);
void displayLinkedParameterSet(HTp token, int pindex);
void printInternalParameters(HTp token);


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
	infile.analyzeStructure();
	for (int i=0; i<infile.getLineCount(); i++) {
		for (int j=0; j<infile[0].getFieldCount(); j++) {
			HTp token = infile.token(i, j);
			
			int pcount = token->getLinkedParameterSetCount();
			cout << "\nTOKEN\t=\t" << token << "\tHAS " << pcount << " PARAMETER SETS" << endl;
			printInternalParameters(token);
			for (int k=0; k<pcount; k++) {
				displayLinkedParameterSet(token, k);
			}
		}
	}
}



//////////////////////////////
//
// printInternalParameters --
//

void printInternalParameters(HTp token) {
	token->storeParameterSet();
	HumParamSet* hps = token->getParameterSet();
	if (!hps) {
		return;
	}
	cout << "\tINTERNAL PARAMETER SET:" << endl;
	cout << "\t\tNAMESPACE = " << hps->getNamespace() << endl;
	for (int i=0; i<hps->getCount(); i++) {
		cout << "\t\t\t" << hps->getParameterName(i) << "\t=\t" << hps->getParameterValue(i) << endl;
	}
}



//////////////////////////////
//
// displayLinkedParameterSets --
//

void displayLinkedParameterSet(HTp token, int pindex) {
	HumParamSet* hps = token->getLinkedParameterSet(pindex);
	if (!hps) {
		return;
	}
	cout << "\tNAMESPACE = " << hps->getNamespace() << endl;
	for (int i=0; i<hps->getCount(); i++) {
		cout << "\t\t" << hps->getParameterName(i) << "\t=\t" << hps->getParameterValue(i) << endl;
	}
}



