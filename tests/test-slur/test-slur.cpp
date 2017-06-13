// Description: Print slur linking info

#include "humlib.h"

using namespace hum;

int main(int argc, char** argv) {
   if (argc != 2) {
      return 1;
   }
   HumdrumFile infile;
   if (!infile.read(argv[1])) {
      return 1;
   }
cerr << "ANALYZING SLURS" << endl;
   infile.analyzeKernSlurs();
cerr << "DONE ANALYZING SLURS" << endl;
   return 0;
}



