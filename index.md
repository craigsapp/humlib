---
layout: default
title: minHumdrum
---

The minHumdrum library consists of a set of C++ classes for parsing
[Humdrum](http://www.humdrum.org) data files.  The library is designed
to be portable with only two code files to copy into your project:

1. An include file [minhumdrum.h](https://github.com/craigsapp/minHumdrum/blob/master/include/minhumdrum.h)
2. and a source file [minhumdrum.cpp](https://github.com/craigsapp/minHumdrum/blob/master/src/minhumdrum.cpp)

The source code uses some C++11-specific features, so add the
`-stc=c++11` option when compiling with GNU g++ or the clang++ compiler.
Also include the `-stdlib=libc++` option when compiling with [clang](https://en.wikipedia.org/wiki/Clang).  See the
[Makefile](https://github.com/craigsapp/minHumdrum/blob/master/Makefile)
for compiling the library and
[Makefile.examples](https://github.com/craigsapp/minHumdrum/blob/master/Makefile.examples)
for compiling and linking executables.

Resources
=========

<style>
#resources {
   -webkit-column-count:3;
   -moz-column-count:3;
   -ms-column-count:3;
   -o-column-count:3;
   column-count:3;
   columns:3;
   padding: 0;
   margin: 0;
}
#resources > li {
   list-style: disc outside none;
   display: list-item;
   margin-left: 4em;
}
</style>

<center>
<table style="display:block; padding:0; margin:0;">
<tr><td>
<ul id="resources">
<li style="margin-top:0"> <a href=/doc>Documentation</a> </li>
<li> <a href=/doc/class>Classes</a> </li>
<li> <a href=/doc/snippet>Code snippets</a> </li>
<li> <a href=/doc/example>Example programs</a> </li>
<li> <a href=/doc/topic>Topics</a> </li>
<li> <a href=/doc/tutorial>Tutorial</a> </li>
</ul>
</td></tr></table>
</center>


Downloading
===========

For minimal use of the library, you can download the composite header
and source files.  In a terminal you can download with [wget](https://en.wikipedia.org/wiki/Wget)
(most common method for linux):

```console
wget https://raw.githubusercontent.com/craigsapp/minHumdrum/master/include/minhumdrum.h
wget https://raw.githubusercontent.com/craigsapp/minHumdrum/master/src/minhumdrum.cpp
```

Or with [curl](https://en.wikipedia.org/wiki/CURL) (most common method for OS X):

```console
curl https://raw.githubusercontent.com/craigsapp/minHumdrum/master/include/minhumdrum.h -o minhumdrum.h
curl https://raw.githubusercontent.com/craigsapp/minHumdrum/master/src/minhumdrum.cpp -o minhumdrum.cpp
```

To compile minHumdrum as a stand-alone library, you can download a ZIP or
tarball from the buttons at the top of this page, or you can use
[git](https://en.wikipedia.org/wiki/Git_(software)) in the console to
download and allow easy updating:

```console
git clone https://github.com/craigsapp/minHumdrum
```

To update to the most recent version of minHumdrum if git was used to
download the library, type anywhere in the minHumdrum directory structure:

```console
git pull
```


Compiling
==========

When downloading the git repository or a zip/tarball of the repository,
compile the library with the command:

```console
make
```

This should create the file `lib/libminhumdrum.a`, which can be
used to link to other program code. (But note that you can also
simply copy the .h and .cpp files listed above into your own project
files if you don't want to link against a separately compiled library
file).  Primarily for testing purposes, another form of the library
can be compiled from the individual source files for each class:

```console
make lib
```

This will create the file `lib/libhumdrum.a`.



Example
=============

Here is a short example program that uses the minHumdrum library to convert
a Humdrum file into a MIDI-like listing of notes.

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

<p style="padding-top: 20px;">
Test data for use with the above program:
</p>

<table style="width:100%">
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
</td>
<td style="border:0">
<img style="width:300px" src="https://cdn.rawgit.com/craigsapp/minHumdrum/gh-pages/images/hum2notelist.svg" title="Equivalent graphical representation of Humdrum data.">
</td>
<td style="border:0">
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

If you are using the minHumdrum project directory, place
your own programs into a subdirectory called `myprograms`, and then to compile,
go to the base directory of the minHumdrum code and type `make myprogram`
if the program is called `myprograms/myprogram.cpp`.  The compiled program
will be created as `bin/myprogram`.

See the [Coding-examples](/doc/example) documentation for more programming examples.


