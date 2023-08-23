humlib
==========

[![AppVeyor Build Status](https://ci.appveyor.com/api/projects/status/e08c7i6tl17j3ip2?svg=true)](https://ci.appveyor.com/project/craigsapp/humlib)

The humlib repository consists of a set of C++ classes for parsing
[Humdrum](http://www.humdrum.org) data files, used for digital
encodings of musical scores.  In addition, Humlib includes [command-line
tools](https://github.com/craigsapp/humlib/tree/master/cli) that
also can be used via JavaScript in web applications.  This is easiest
done through the notation renderer [verovio](https//www.verovio.org),
and in particular with [Humdrum Notation
Plugin](https://plugin.humdrum.org).  Online use of most tools can
be done when editing Humdrum files in [Verovio Humdrum
Viewer](https://verovio.humdrum.org).


# Using humlib as a C++ library

The library is designed to be portable with only two source-code 
files to copy into your project:

1. An include file [humlib.h](https://github.com/craigsapp/humlib/blob/master/min/humlib.h)
2. And a source file [humlib.cpp](https://github.com/craigsapp/humlib/blob/master/min/humlib.cpp)

Plus three [Pugixml](https://pugixml.org) files used for parsing/writing
data in the XML format:

1. The pugixml include file [pugixml.hpp](https://github.com/craigsapp/humlib/blob/master/min/pugixml.hpp)
2. The pugixml configuration file [pugiconfig.hpp](https://github.com/craigsapp/humlib/blob/master/min/pugiconfig.hpp)
3. The pugixml source file [pugixml.cpp](https://github.com/craigsapp/humlib/blob/master/min/pugixml.cpp)


All of these files can be copied from the
[min](https://github.com/craigsapp/humlib/blob/master/min) directory
into your project.  For example, verovio include the
[humlib.cpp](https://github.com/rism-digital/verovio/blob/develop/src/hum/humlib.cpp)
and
[humlib.h](https://github.com/rism-digital/verovio/blob/develop/include/hum/humlib.h)
and independently uses the pugixml library for processing XML data
([MusicXML](https://en.wikipedia.org/wiki/MusicXML) and
[MEI](https://www.music-encoding.org)).



# Documentation

Documentation for humlib source code can be found at https://humlib.humdrum.org

Documentation for the tools can be found at https://doc.verovio.humdrum.org/filter



# Download and compiling

To download humlib with `git` in a directory where you want to store humlib:

```bash
git clone https://github.org/craigsapp/humlib.git
```

To compile, run the command:

```bash
cd humlib
make
```

This will compile both the library and command-line tools found in [cli](https://github.com/craigsapp/humlib/tree/master/cli).  To compile only the library:


```bash
cd humlib
make library
```

To download updates of the humdrum repository, use the command:

```bash
make update
```

after which you can recompile the library and programs:

```bash
make
```


# Minimal downloading

For minimal use of the library with your own project, you can download just the composite
header and source files (and pugixml library).  In a terminal you can download with
[wget](https://en.wikipedia.org/wiki/Wget) (on linux):

```console
wget https://raw.githubusercontent.com/craigsapp/humlib/master/min/humlib.h
wget https://raw.githubusercontent.com/craigsapp/humlib/master/min/humlib.cpp
wget https://raw.githubusercontent.com/craigsapp/humlib/master/min/pugixml.cpp
wget https://raw.githubusercontent.com/craigsapp/humlib/master/min/pugixml.hpp
wget https://raw.githubusercontent.com/craigsapp/humlib/master/min/pugiconfig.hpp
```

Or with [curl](https://en.wikipedia.org/wiki/CURL) (most common method downloading on the comnmand line in MacOS):

```console
curl https://raw.githubusercontent.com/craigsapp/humlib/master/min/humlib.h -o humlib.h
curl https://raw.githubusercontent.com/craigsapp/humlib/master/min/humlib.cpp -o humlib.cpp
curl https://raw.githubusercontent.com/craigsapp/humlib/master/min/pugixml.hpp -o pugixml.hpp
curl https://raw.githubusercontent.com/craigsapp/humlib/master/min/pugixml.hpp -o pugixml.hpp
curl https://raw.githubusercontent.com/craigsapp/humlib/master/min/pugiconfig.cpp -o pugiconfig.cpp
```

The source code uses some C++11-specific features, so add the
`-stc=c++11` option when compiling with GNU g++ or the clang++
compiler.  (See the humlib
[Makefile](https://github.com/craigsapp/humlib/blob/master/Makefile)).  In
particular, include the `-stdlib=libc++` option when compiling with
[clang](https://en.wikipedia.org/wiki/Clang).  See the
[Makefile](https://github.com/craigsapp/humlib/blob/master/Makefile) for
compiling the library and
[Makefile.examples](https://github.com/craigsapp/humlib/blob/master/Makefile.examples)
for compiling and linking executables for the command-line tools.


# Example

Here is a short example program that uses the humlib library to convert
a Humdrum file into a MIDI-like listing of notes.

```cpp
#include "humlib.h"

using namespace std;
using namespace hum;

void printNoteInformation(HumdrumFile& infile, int line, int field, int tpq) {
	HTp token = infile.token(line, field);
   int starttime = infile[line].getDurationFromStart(tpq).getInteger();
   int duration  = token->getDuration(tpq).getInteger();
   cout << Convert::kernToSciPitch(*token)
        << '\t' << token->getTrackString()
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
			HTp token = infile.token(i, j);
         if (token->isNull()) {
            continue;
         }
         if (token->isKern()) {
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

<img style="width:300px" src="https://cdn.rawgit.com/humdrum-tools/humlib/gh-pages/images/hum2notelist.svg" title="Equivalent graphical representation of Humdrum data.">

<table style="width:100%">
<tr><td style="border:0">
Example input:<br>
<textinput style="tab-stop: 12; font-family: Courier; text-align:left">
**kern	**kern
*M3/4	*M3/4
8C	12d
.	12e
8B	.
.	12f
*	*^
4A	2g	4d
4G	.	4c
*	*v	*v
=	=
*-	*-
</pre>
</td>
<td style="border:0">
Example output:<br>
<textarea style="font-family: Courier; text-align:left">
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


# Writing your own command-line tools with Humlib:

If you are using the humlib project directory to compile your own programs (or this test program), place them
into the [cli](https://github.com/craigsapp/humlib/blob/master/cli)
(Command-Line Interface) subdirectory. 

Then to compile, go to the base directory of the humlib repository
and type `make myprogram` if the program is called
`humlib/cli/myprogram.cpp`.  The compiled program will be created
as `bin/myprogram`.

To run from the `humlib` directory type `bin/myprogram file.krn`,
for example.  You can also either copy your program to `/usr/local/bin`
or set the `$PATH` variable to include `humlib/bin` in the executable
search path.

See the [Coding-examples](http://humlib.humdrum.org/doc/example) documentation for more programming examples.



