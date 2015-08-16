// Description: Print the duration from the start of the Humdrum file to the
// start of the current line.

#include "minhumdrum.h"

int main(int argc, char** argv) {
   if (argc != 2) {
      return 1;
   }
   HumdrumFile infile;
   if (!infile.read(argv[1])) {
      return 1;
   }
   HumNum forward;
   HumNum backward;
   for (int i=0; i<infile.getLineCount(); i++) {
      forward = infile[i].getDurationFromBarline();
      backward = infile[i].getDurationToBarline();
      cout << forward << "\t";
      cout << backward << "\t";
      cout << (forward+backward) << "\t";
      cout << "::\t" << infile[i] << endl;
   }
   return 0;
}

