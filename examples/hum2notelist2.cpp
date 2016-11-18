#include "humlib.h"

using namespace std;
using namespace hum;

void printNoteInformation(HumdrumToken* tok, int tpq) {
   cout << *tok;
   if (tok->isNonNullData()) {
      cout << "\t->\t" << Convert::kernToSciPitch(*tok)
           << "\t" << tok->getTrackString()
           << "\t" << tok->getDurationFromStart(tpq).getInteger()
           << "\t" << tok->getDuration(tpq).getInteger();
   }
   cout << endl;
}

int main(int argc, char** argv) {
   if (argc != 2) {
      return 1;
   }
   HumdrumFile infile;
   if (!infile.read(argv[1])) {
      return 1;
   }
   int tpq = infile.tpq();
   cout << "TPQ: " << tpq << endl;
   cout << "ORIG\t\tPITCH\tTRACK\tSTART\tDURATION" << endl;

   HumdrumToken* tok = infile.getTrackStart(1);
   while (tok != NULL) {
      printNoteInformation(tok, tpq);
      tok = tok->getNextToken();
   }
   return 0;
}

