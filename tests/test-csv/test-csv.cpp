// Description: Print a Humdrum file in CSV format.

#include "humlib.h"

using namespace Humdrum;

int main(int argc, char** argv) {
   if (argc != 2) {
      return 1;
   }
   HumdrumFile infile;
   if (!infile.read(argv[1])) {
      return 1;
   }
   infile.printCsv();
   return 0;
}



