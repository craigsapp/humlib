// Description: Print spine info for each token in a file.

#include "minhumdrum.h"

using namespace minHumdrum;

int main(int argc, char** argv) {
   if (argc != 2) {
      return 1;
   }
   HumdrumFile infile;
   if (!infile.read(argv[1])) {
      return 1;
   }
   for (int i=0; i<infile.getLineCount(); i++) {
      if (!infile[i].hasSpines()) {
         cout << infile[i] << endl;
         continue;
      }
      for (int j=0; j<infile[i].getTokenCount(); j++) {
         cout << infile[i].token(j).getSpineInfo() << "\t";
      }
      cout << "::\t" << infile[i] << endl;
   }
   return 0;
}


