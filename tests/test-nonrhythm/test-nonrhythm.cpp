// Description: Print the inferred duration of non-rhythmic spine tokens.

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
         continue;
      }
      if (!infile[i].isData()) {
         continue;
      }
      for (int j=0; j<infile[i].getTokenCount(); j++) {
         if (infile[i].token(j).hasRhythm()) {
            continue;
         }
         if (infile[i].token(j).isNull()) {
            continue;
         }
         cout << infile[i].token(j) << "\t"
              << infile[i].token(j).getDuration() << endl;
      }
   }

   return 0;
}



