// Description: Print the parameters for lines and tokens.

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
   // print line parameters:
   cout << "Global parameters in file:" << endl;
   for (int i=0; i<infile.getLineCount(); i++) {
      if (infile[i].hasParameters()) {
         cout << (HumHash)infile[i];
      }
   }

   cout << "Local parameters in file:" << endl;
   for (int i=0; i<infile.getLineCount(); i++) {
      for (int j=0; j<infile[i].getTokenCount(); j++) {
         if (infile[i].token(j).hasParameters()) {
            cout << (HumHash)infile[i].token(j);
         }
      }
   }

   return 0;
}



