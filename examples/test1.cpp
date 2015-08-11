
#include "minhumdrum.h"

using namespace std;

int main(int argc, char** argv) {
	HumdrumFile infile;
	bool status  = infile.read(argv[1]);
	if (status) {
		cout << infile;
		return 0;
	}
	return 1;
}


