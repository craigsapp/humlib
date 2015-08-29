---
layout: default
breadcrumbs: [
                ['/', 'home'],
                ['/doc', 'documentation'],
                ['/doc/tutorial', 'tutorial'],
                ['/doc/class', 'classes'],
                ['/doc/snippet', 'snippets'],
                ['/doc/example', 'examples']
        ]
title: Coding examples
---

Below is an example program that uses the minHumdrum library to convert a Humdrum file into a MIDI-like listing of notes in the Humdrum score:

```cpp
#include "minhumdrum.h"

using namespace std;
using namespace minHumdrum;

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

Notice that all minHumdrum code is placed into the minHumdrum namespace.

Test data for use with the above program:

![Example music](https://cdn.rawgit.com/craigsapp/minHumdrum/gh-pages/images/hum2notelist.svg)

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

If you are using the minHumdrum project directory, place
programs into a subdirectory called `myprograms` and then to compile,
go to the base directory of the minHumdrum code and type `make myprogram`
if the program is called `myprograms/myprogram.cpp`.  The compiled program
will be created as `bin/myprogram`.


Extracting spines/tracks (~parts)
---------------------------------

Here is another example showing one of the methods which can be used to extract data for
a specific part out of the full-score arrangement of the HumdrumFile data:

```cpp
#include "minhumdrum.h"

using namespace std;
using namespace minHumdrum;

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

