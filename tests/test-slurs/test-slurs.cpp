// Description: Print a file in XML format

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
   infile.analyzeKernSlurs();

   infile.printXml();
   return 0;
   
}



