// Extract a list of notes from a Humdrum score, giving their starting times
// and durations as "ticks" (minimum rhythmic time unit of file).

#include "minhumdrum.h"

using namespace std;
using namespace minHumdrum;

void printNoteInformation(HumdrumFile& infile, int line, 
      int field, int tpq);

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
   cout << "PITCH\tSTART\tDURATION" << endl;

   for (int i=0; i<infile.getLineCount(); i++) {
      if (!infile[i].isData()) {
         continue;
      }
      for (int j=0; j<infile[i].getTokenCount(); j++) {
         if (infile[i].token(j).isNull()) {
            continue;
         }
         if (infile[i].token(j).isDataType("kern")) {
            printNoteInformation(infile, i, j, tpq);
         }
      }
   }
   return 0;
}

void printNoteInformation(HumdrumFile& infile, int line, 
      int field, int tpq) {
   int starttime, duration;
   HumNum value;

   value = infile[line].getDurationFromStart();
   value *= tpq;
   starttime = value.getNumerator();

   value = infile[line].token(field).getDuration();
   value *= tpq;
   duration = value.getNumerator();

   cout << Convert::kernToScientificPitch(infile[line].token(field))
        << '\t' << starttime << '\t' << duration << endl;
}


/* Example input:

**kern	**kern
*M4/4	*M4/4
8C	12d
.	12e
8B	.
.	12f
4A	2g
4G	.
=	=
*-	*-

*/


/* Example output:

TPQ: 6
PITCH	START	DURATION
C3	0	3
D4	0	2
E4	2	2
B3	3	3
F4	4	2
A3	6	6
G4	6	12
G3	12	6

*/


