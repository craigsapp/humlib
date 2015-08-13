// Description: Print the duration of each line.

#include "minhumdrum.h"

int main(int argc, char** argv) {
   if (argc != 2) {
      return 1;
   }
   HumdrumFile infile;
   if (!infile.read(argv[1])) {
      return 1;
   }
   for (int i=0; i<infile.size(); i++) {
      cout << infile[i].getDuration() << "\t" << infile[i] << endl;
   }
   return 0;
}


