// Description: Print the the previous non-null token assosicated with 
// each data line.

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
   HumdrumToken* token;
   int i, j, k;
   for (i=0; i<infile.getLineCount(); i++) {
      cout << "\t\t\t\t" << infile[i] << endl;
      for (j=0; j<infile[i].getTokenCount(); j++) {
         token = &(infile[i].token(j));
         for (k=0; k<token->getPreviousNNDTCount(); k++) {
            cout << *(token->getPreviousNNDT(k)) << " <- ";
         }
         cout << "\t[" << *token << "]\t";
         for (k=0; k<token->getNextNNDTCount(); k++) {
            cout << " -> " << *(token->getNextNNDT(k));
         }
         cout << endl;
      }
   }
   return 0;
}



