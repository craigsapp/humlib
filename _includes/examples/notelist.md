Below is an example program that uses the humlib library to convert a Humdrum file into a MIDI-like listing of notes in the Humdrum score:

```cpp
#include "humlib.h"

using namespace std;
using namespace humlib;

void printNoteInformation(HumdrumFile& infile, int line, int field, int tpq) {
   int starttime = infile[line].getDurationFromStart(tpq).getInteger();
   int duration  = infile.token(line, field).getDuration(tpq).getInteger();
   cout << Convert::kernToSciPitch(infile.token(line, field))
        << '\t' << infile.token(line, field).getTrackString()
        << '\t' << starttime << '\t' << duration << endl;
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
   cout << "PITCH\tTRACK\tSTART\tDURATION" << endl;

   for (int i=0; i<infile.getLineCount(); i++) {
      if (!infile[i].isData()) {
         continue;
      }
      for (int j=0; j<infile[i].getTokenCount(); j++) {
         if (infile.token(i, j).isNull()) {
            continue;
         }
         if (infile.token(i, j).isDataType("kern")) {
            printNoteInformation(infile, i, j, tpq);
         }
      }
   }
   return 0;
}
```

Notice that all humlib code is placed into the humlib namespace.

Test data for use with the above program:

![Example music](https://cdn.rawgit.com/craigsapp/humlib/gh-pages/images/hum2notelist.svg)

<table style="border-collapse: separate; margin-left:auto; margin-right:auto">
<tr><td style="border:0">
Example input:<br>
<pre style="tab-stop: 12; font-family: Courier; text-align:left">
**kern  **kern
*M3/4   *M3/4
8C      12d
.       12e
8B      .
.       12f
*       *^
4A      2g      4d
4G      .       4c
*       *v      *v
=       =
*-      *-
</pre>
</td><td style="border:0">
Example output:<br>
<pre style="font-family: Courier; text-align:left">
TPQ: 6
PITCH   TRACK   START   DURATION
C3      1       0       3
D4      2       0       2
E4      2       2       2
B3      1       3       3
F4      2       4       2
A3      1       6       6
G4      2.1     6       12
D4      2.2     6       6
G3      1       12      6
C4      2.2     12      6
</pre>
</td></tr></table>
</center>

If you are using the humlib project directory, place
programs into a subdirectory called `myprograms` and then to compile,
go to the base directory of the humlib code and type `make myprogram`
if the program is called `myprograms/myprogram.cpp`.  The compiled program
will be created as `bin/myprogram`.


