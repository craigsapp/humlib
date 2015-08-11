
#include "minhumdrum.h"

int main(int argc, char** argv) {
	HumdrumFile infile;
	if (argc == 2) {
		bool status  = infile.read(argv[1]);
		if (status) {
			cout << infile;
			return 0;
		}
	}
	return 1;
}


