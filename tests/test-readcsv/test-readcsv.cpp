// Description: Print the duration of each line.

#include "humlib.h"

using namespace hum;

int main(int argc, char** argv) {
   if (argc != 2) {
      return 1;
   }
   HumdrumFile infile;
   if (!infile.readCSV(argv[1])) {
      return 1;
   }
   cout << infile;
   return 0;
}



