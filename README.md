minHumdrum
==========

The minHumdrum library is a set of C++ classes for parsing 
[Humdrum](http://www.humdrum.org) file data.  The library is designed
to be portable, consisting of two files that can be copied into
your project:

1. An include file [minhumdrum.h](/include/minhumdrum.h)
2. and a source file [minhumdrum.cpp](/src/minhumdrum.h)

The classes use some C++11-specific features, so you must add the
`-stc=c++11` option when compiling with GNU g++ or clang++ compiler.
Also include the `-stdlib=libc++` when compiling with clang.  See the
[Makefile](https://github.com/craigsapp/minHumdrum/Makefile) for compiling the library and 
[Makefile.examples](https://github.com/craigsapp/minHumdrum/Makefile.examples) for linking to create executables.

More information and documentation for the library can be found at http://min.humdrum.org.


Class overview
==============

Here are the classes defined in the minHumdrum library:

* [HumdrumFile](https://github.com/craigsapp/minHumdrum/blob/master/include/HumdrumFile.h): primary interface for working with Humdrum file data. <img title="class-organization" align=right width="250" src="https://cdn.rawgit.com/craigsapp/minHumdrum/gh-pages/images/class-organization.svg">
* [HumdrumFileContent](https://github.com/craigsapp/minHumdrum/blob/master/include/HumdrumFileContent.h): manages analysis of data content (particularly of **kern data) beyond basic rhythmic analysis done in HumdrumFileStructure.
* [HumdrumFileStructure](https://github.com/craigsapp/minHumdrum/blob/master/include/HumdrumFileStructure.h): manages rhythmic analysis of the data and extra parameters for data tokens.
* [HumdrumFileBase](https://github.com/craigsapp/minHumdrum/blob/master/include/HumdrumFileBase.h): manages data storage and reading/writing of Humdrum file data.
(particularly from token strings) into other formats.
* [HumdrumLine](https://github.com/craigsapp/minHumdrum/blob/master/include/HumdrumLine.h): manages the content of a line (data record) in a Humdrum file.
* [HumdrumToken](https://github.com/craigsapp/minHumdrum/blob/master/include/HumdrumToken.h): manages tokens (data elements on a line) for HumdrumLine.
* [HumAddress](https://github.com/craigsapp/minHumdrum/blob/master/include/HumAddress.h): location information for a HumdrumToken on a HumdrumLine and in a HumdrumFile.
* [HumNum](https://github.com/craigsapp/minHumdrum/blob/master/include/HumNum.h): rational number class for working with durations. 
* [HumHash](https://github.com/craigsapp/minHumdrum/blob/master/include/HumHash.h): associative array for HumdrumLine and HumdrumToken parameters.
* [HumMultiHash](https://github.com/craigsapp/minHumdrum/blob/master/include/HumMultiHash.h): associative array for HumdrumFile reference records and parameters.
* [Convert](https://github.com/craigsapp/minHumdrum/blob/master/include/Convert.h): utility functions for converting data (primarily for processing HumdrumToken strings).


Downloading
===========

To download using git, type:
```bash
git clone https://github.com/craigsapp/minHumdrum
```
To update to the most recent version of minHumdrum, type in the minHumdrum directory:
```bash
git pull
```

 
Compiling
==========

If you download the git repository or a zip/tarball of the repository, you can
compile the library with the command
```bash
make
```
This should create the file `lib/libminhumdrum.a`, which can be used to
link to other program code.

For testing purposes, another form of the library can be compiled from the individual source files for each class:
```bash
make lib
```
This will create the file `lib/libhumdrum.a`.


Example
========

Here is an example program that uses the minHumdrum library to convert a Humdrum file into a MIDI-like listing of notes in the Humdrum score:


```cpp
#include "minhumdrum.h"

using namespace std;
using namespace minHumdrum;

void printNoteInformation(HumdrumFile& infile, int line, int field, int tpq) {
   int starttime = infile[line].getDurationFromStart(tpq).getInteger();
   int duration  = infile.token(line, field).getDuration(tpq).getInteger();;
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

![Example music](https://cdn.rawgit.com/craigsapp/minHumdrum/master/examples/hum2notelist.svg)

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
PITCH	TRACK	START	DURATION
C3      1       0       3
D4      2       0       2
E4      2       2       2
B3      1       3       3
F4      2       4       2
A3      1       6       6
G4      2.1     6       12
D4      2.2     6       6
G3	1       12      6
C4	2.2	12	6
</pre>
</td></tr></table>
</center>

If you are using the minHumdrum project directory, you can place
programs into a subdirectory called `myprograms` and then to compile,
go to the base directory of the minHumdrum code and type `make myprogram`
if the program is called `myprograms/myprogram.cpp`.  The compiled program
will be created as `bin/myprogram`.
