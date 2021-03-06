// Description: Print the duration of each line.

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
   for (int i=0; i<infile.getLineCount(); i++) {
      cout << infile[i].getDurationFromStart() << "\t";
      cout << infile[i].getDurationFromStart().getFloat() << "\t";
      cout << infile[i].getDuration() << "\t";
      cout << "::\t" << infile[i] << endl;
   }
   return 0;
}



