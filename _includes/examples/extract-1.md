Here is another example showing one of the methods which can be
used to extract data for a specific part out of the full-score
arrangement of the HumdrumFile data:

```cpp
#include "humlib.h"

using namespace std;
using namespace humlib;

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

   // get the first token in the first spine:
   HumdrumToken* tok = infile.getTrackStart(1);
   // then iterate to the end of the spine:
   while (tok != NULL) {
      printNoteInformation(tok, tpq);
      tok = tok->getNextToken();
   }
   return 0;
}
```

This program should produce the following output given the example input:

```
TPQ: 6
ORIG            PITCH   TRACK   START   DURATION
**kern
*M4/4
8C      ->      C3      1       0       3
.
8B      ->      B3      1       3       3
.
*
4A      ->      A3      1       6       6
4G      ->      G3      1       12      6
*
=
*-
```

