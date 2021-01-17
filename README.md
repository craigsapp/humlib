humlib
==========

[![Travis Build Status](https://travis-ci.org/craigsapp/humlib.svg?branch=master)](https://travis-ci.org/craigsapp/humlib) [![AppVeyor Build Status](https://ci.appveyor.com/api/projects/status/e08c7i6tl17j3ip2?svg=true)](https://ci.appveyor.com/project/craigsapp/humlib)


The humlib library consists of a set of C++ classes for parsing
[Humdrum](http://www.humdrum.org) data files, used for digital encodings
of musical scores.  The library is designed to be portable with
only two source-code files to copy into your project:

1. An include file [humlib.h](https://github.com/craigsapp/humlib/blob/master/include/humlib.h)
2. And a source file [humlib.cpp](https://github.com/craigsapp/humlib/blob/master/src/humlib.cpp)

All other source-code files are irrelevant unless you are developing
the library or need to create the command-line tools.  For example
[verovio](https://github.com/rism-ch/verovio), a music notation
rendering library,  uses humlib as an internal sublibrary for
importing Humdrum data, placing `humlib.h`
[here](https://github.com/rism-ch/verovio/blob/master/include/hum), and
`humlib.cpp`
[here](https://github.com/rism-ch/verovio/blob/master/src/hum).
(see the `.download` scripts in those directories for how updates
are managed).

Most command-line tools are implemented as C++ classes, found in
files that start with `tool-` in the
[src](https://github.com/craigsapp/humlib/blob/master/src) directory. For
example, here is a use of the `colortriads` and `satb2gs` tools
inside of verovio to create [colorized version of Bach chorales on
the Grand
Staff](https://verovio.humdrum.org/?file=chorales&filter=colortriads%7csatb2gs).


Resources
=========
<center>
<table style="display:block; padding:0; margin:0;">
<tr><td>
<ul id="resources">
<li style="margin-top:0"> <a href=http://humlib.humdrum.org>Humlib website</a> </li>
<li> <a href=http://humlib.humdrum.org/doc>Documentation</a> </li>
<li> <a href=http://humlib.humdrum.org/doc/class>Class overview</a> </li>
<li> <a href=http://humlib.humdrum.org/doc/snippet>Code snippets</a> </li>
<li> <a href=http://humlib.humdrum.org/doc/example>Example programs</a> </li>
<li> <a href=http://humlib.humdrum.org/doc/topic>Topics</a> </li>
<li> <a href=http://humlib.humdrum.org/doc/tutorial>Tutorial</a> </li>
</ul>
</td></tr></table>
</center>


Downloading
===========

To compile humlib as a stand-alone library, you can download a ZIP or
tarball from the buttons at the top of this page, or you can use
[git](https://en.wikipedia.org/wiki/Git_(software)) in the console to
download and allow easy updating:

```console
git clone https://github.com/craigsapp/humlib
```

To update to the most recent version of humlib if git was used to
download the library, type anywhere in the humlib directory structure:

```console
git pull
```

Minimal downloading
======================

For minimal use of the library, you can download just the composite
header and source files.  In a terminal you can download with
[wget](https://en.wikipedia.org/wiki/Wget) (most common method for
linux):

```console
wget https://raw.githubusercontent.com/craigsapp/humlib/master/include/humlib.h
wget https://raw.githubusercontent.com/craigsapp/humlib/master/src/humlib.cpp
```

Or with [curl](https://en.wikipedia.org/wiki/CURL) (most common method for OS X):

```console
curl https://raw.githubusercontent.com/craigsapp/humlib/master/include/humlib.h -o humlib.h
curl https://raw.githubusercontent.com/craigsapp/humlib/master/src/humlib.cpp -o humlib.cpp
```

The source code uses some C++11-specific features, so add the
`-stc=c++11` option when compiling with GNU g++ or the clang++ compiler.
Also include the `-stdlib=libc++` option when compiling with [clang](https://en.wikipedia.org/wiki/Clang).  See the
[Makefile](https://github.com/craigsapp/humlib/blob/master/Makefile)
for compiling the library and
[Makefile.examples](https://github.com/craigsapp/humlib/blob/master/Makefile.examples)
for compiling and linking executables.


Compiling
==========

When downloading the git repository or a zip/tarball of the repository,
compile the library with the command:

```console
make
```

This should compile the file `lib/libhumlib.a` as well as the
command-line tools in
[cli](https://github.com/craigsapp/humlib/blob/master/cli) into the
`humlib/bin` directory..

```console
make library
```

This is similar to `make`, but only compiles the library file and
not the command-line tools.

```console
make lib
```

Used for testing purposes only, this make target compiles
the uncollated library files, making it easier to debug
the source code.



Example
=============

Here is a short example program that uses the humlib library to convert
a Humdrum file into a MIDI-like listing of notes.

```cpp
#include "humlib.h"

using namespace std;
using namespace hum;

void printNoteInformation(HTp token, int tpq) {
   int duration  = token->getTiedDuration(tpq).getInteger();
   int starttime = token->getDurationFromStart(tpq).getInteger();
   vector<string> subtokens = token->getSubtokens();
   for (size_t i=0; i<subtokens.size(); i++) {
      cout << Convert::kernToSciPitch(subtokens[i])
         << '\t' << token->getTrackString()
         << '\t' << starttime
         << '\t' << duration << endl;
   }
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
         HTp token = infile.token(i, j);
         if (token->isNull()) {
            continue;
         }
         if (token->isDataType("kern")) {
            printNoteInformation(token, tpq);
         }
      }
   }
   return 0;
}
```

<p style="padding-top: 20px;">
Test data for use with the above program:
</p>

<img style="width:300px" src="https://cdn.rawgit.com/humdrum-tools/humlib/gh-pages/images/hum2notelist.svg" title="Equivalent graphical representation of Humdrum data.">

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

If you are using the humlib project directory to compile your own programs (or this test program), place them
into the [cli](https://github.com/craigsapp/humlib/blob/master/cli)
(Command-Line Interface) subdirectory. 

Then to compile, go to
the base directory of the humlib repository and type `make myprogram` if
the program is called `humlib/cli/myprogram.cpp`.  

The compiled
program will be created as `bin/myprogram`.  

To run from the `humlib
directory type `bin/myprogram file.krn`, for example.  You can also
either copy your program to `/usr/local/bin` or set the `$PATH`
variable to include `humlib/bin` in the executable search path.

See the [Coding-examples](http://humlib.humdrum.org/doc/example) documentation for more programming examples.



