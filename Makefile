##
## Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
## Creation Date: Sun Aug  9 22:20:14 PDT 2015
## Last Modified: Sun Apr 16 08:27:37 PDT 2023
## Syntax:        GNU Makefile
## Filename:      humlib/Makefile
## vim:           ts=3
##
## Description: Makefile to run tasks for humlib library, particularly compiling code.
##
## To compile library and all command-line tools:
##     make
## To compile only the library:
##     make library
## To compile a specific tool in the "cli" (command line interface) directory, such
## as colortriads:
##     make colortriads
## Note that compiled programs are placed in the "bin" directory.
##
## To create your own command-line program, place the source code into the
## "cli" directory.  To compile that program (called for example cli/xxx.cpp):
##     make xxx
## The library will be compiled before linking to the program if necessary.
##

# Identify the operating system (for MacOS compiling):
OS := $(shell uname -s)

# Set the environmental variable $MACOSX_DEPLOYMENT_TARGET to
# "10.9" in Apple OS X to compile for OS X 10.9 and later (for example,
# you can compile for OS X 10.9 computers even if you are using the 10.10
# version of the operating system).

ENV =
ARCH =
ifeq ($(OS),Darwin)
	OS = OSX
	# Minimum OS X Version for C++11 is OS X 10.9:
   ENV = MACOSX_DEPLOYMENT_TARGET=10.9
   # use the following to compile for 32-bit architecture on 64-bit comps:
   #ARCH = -m32 -arch i386
else
   # use the following to compile for 32-bit architecture on 64-bit comps:
   # (you will need 32-bit libraries in order to do this)
   # ARCH = -m32
endif


###########################################################################
#                                                                         #
# Beginning of user-modifiable configuration variables                    #
#                                                                         #

# BINDIR: Location where compiled programs will be place.
BINDIR = bin

# OBJDIR: Location where object files will be place.
OBJDIR = obj

# SRCDIR: Location of source-code files
SRCDIR = src

# SRCDIR_MIN: Location of the combined source code file (humlib.cpp) is located.
SRCDIR_MIN = min

# INCDIR: Location where the header files are located.
INCDIR = include

# MINDIR: Location for the combined files humlib.cpp and (humlib.h are located.
MINDIR = min

# LIBDIR: Location where library files are placed.
LIBDIR = lib

# LIBFILE: Name of the (static) humlib library files.
LIBFILE = libhumlib.a

# LIBFILE_PUGI: Name of the (static) pugixml library file.
LIBFILE_PUGI = libpugixml.a

# CLIDIR: Location where command-line interface programs are located.
CLIDIR = cli

# DLIBFILE = Name of the dynamic library (operation-system specific):
DLIBFILE =
ifeq ($(OS),OSX)
   DLIBFILE    = humlib.dylib
else
   DLIBFILE    = humlib.so
endif

# AR: Name of the ar program used to create library files:
AR        = ar

# RANLIB: Name of the ranlib program used to create library files:
RANLIB    = ranlib

# PREFLAGS: Compiler options placed before filenames
PREFLAGS = -c -g -I$(INCDIR)
# Add warnings to PREFLAGS:
PREFLAGS += -Wall -Werror
# Optimize the binary code in object files to run faster:
PREFLAGS += -O3

# Set the C++ standard being used to compile code.  Must be C++ 11 or later.
#PREFLAGS += -std=c++11
PREFLAGS += -std=c++17

# POSTFLAGS: Compile options placed after filenames
POSTFLAGS =
# Add -static flag to compile without dynamics libraries for better portability:
POSTFLAGS += -static

# COMPILER: Set the compiling program, which is usually given implicitly by the makefile.
# The LANG=C shell variable is used to set the formatting of compiler errors/warnings.
# Setting LANG=C during compilation ensures that the program is
# compiled with consistent settings across different systems, and can
# help prevent issues related to language-specific features or behavior
# that may vary between different locales.
COMPILER = LANG=C $(ENV) $(CXX) $(ARCH)
ifeq ($(CXX),)
   COMPILER = LANG=C $(ENV) g++ $(ARCH)
endif

# Or use clang++:
#COMPILER      = clang++
#PREFLAGS     += -stdlib=libc++

#                                                                         #
# End of user-modifiable variables.                                       #
#                                                                         #
###########################################################################


# targets which don't actually refer to files or should not be considered dependent files:
.PHONY: examples myprograms src include dynamic cli min humlib.h pugixml.hpp pugiconfig.hpp

# vpath (short for "variable path") directive is used to specify a
# search path for prerequisites (dependencies) of targets. This allows
# you to specify alternate locations for files that a target depends
# on, rather than having to include the full path to each prerequisite
# in the Makefile.
vpath %.h   $(INCDIR)
vpath %.cpp $(SRCDIR)
vpath %.hpp $(INCDIR)
vpath %.o   $(OBJDIR)

# Generate a list of the object files (excluding pugixml files):
OBJS =
OBJS += $(patsubst %.cpp,%.o,$(notdir $(wildcard $(SRCDIR)/tool-*.cpp)))
OBJS += $(patsubst %.cpp,%.o,$(notdir $(wildcard $(SRCDIR)/[A-Z]*.cpp)))

# Add the full path to list of object files:
OBJLIST = $(addprefix $(OBJDIR)/,$(OBJS))

# Create a list of cli programs:
CLILIST += $(patsubst %.cpp,%,$(notdir $(wildcard $(CLIDIR)/*.cpp)))


###########################################################################
#                                                                         #
# Makefile targets                                                        #
#                                                                         #


##############################
##
## all: The default action which is to compile the pugixml library,
##     the humlib library (lib/humlib.o) and the command-line tools.
##

all: tools


##############################
##
## list: List makefile targets.
##

h: list
help: list
list:
	@echo
	@echo "Humlib make targets:"
	@echo "   make            Compile library and command-line tools (default)."
	@echo "   make clean      Delete object files."
	@echo "   make clean-bin  Delete compiled CLI programs."
	@echo "   make clean-lib  Delete library files."
	@echo "   make dirs       Create obj and lib directories if they do not yet exist."
	@echo "   make help       Show this list."
	@echo "   make install    Copy CLI programs to /usr/local/bin"
	@echo "   make library    Compile humlib library."
	@echo "   make min        Compile combined files humlib.cpp and humlib.h (in min directory)"
	@echo "   make min-test   Test Compile combined file humlib.cpp .h (in min directory)"
	@echo "   make programs   Same as default make target."
	@echo "   make pugi       Compile pugixml library."
	@echo "   make strip      Strip (remove debugging info) CLI programs."
	@echo "   make superclean Delete object, library, and compiled CLI programs."
	@echo "   make update     Download most recent code on Github."
	@echo
	@echo "Any other make target will be presumed to compile a specific"
	@echo "CLI program such as cli/flipper.cpp:"
	@echo "   make flipper"
	@echo


###########################################################################
#                                                                         #
# Define an explicit rule for compiling object files:                     #
#                                                                         #

%.o : %.cpp
	@echo [CC] $<
	@$(COMPILER) $(PREFLAGS) -c $< -o $(OBJDIR)/$@ $(POSTFLAGS)

#                                                                         #
###########################################################################


##############################
##
## update: Download the most recent version of the humlib repository from
##     Github (if humlib was downloaded with "git clone" and not as a zip file.
##     There is usually a conflict with the min files when downloading since
##     the date line is always added when creating it, and this causes
##     conflicts when someone else has uploaded new min files.  The "git checkout"
##     command reverts the local copy to the last pull request version to avoid
##     having a merge problem.  After running "make update", you can type
##     "make min" to update the min files to commit and push back got github.
##

update:
	git checkout $(MINDIR)/humlib.cpp
	git checkout $(MINDIR)/humlib.h
	git checkout $(INCDIR)/humlib.h
	git pull



##############################
##
## pugixml: Create a static library for pugixml (https://pugixml.org).
##     Pugixml is used to parse and write XML files in the humlib code,
##     such as for the tool musicxml2hum, or mei2hum.
##

pugi: pugixml
pugixml: makedirs pugixml.o
	@$(AR) r $(LIBDIR)/$(LIBFILE_PUGI) $(OBJDIR)/pugixml.o
	@$(RANLIB) $(LIBDIR)/$(LIBFILE_PUGI)



##############################
##
## dynamic: Create a dynamic library (as compared to a static library).
##     Note: not tested in a while, and may be operating-system specific,
##     such as the MacOS and Linux versions below:
##

dynamic: min
ifeq ($(OS),OSX)
	g++ -dynamiclib -o $(LIBDIR)/$(DLIBFILE_MIN) $(OBJDIR)/humlib.o
else
	g++ -shared -fPIC -o $(LIBDIR)/$(DLIBFILE_MIN) $(OBJDIR)/humlib.o
endif



##############################
##
## library: Create lib/humlib.a from object files in the obj directory
##     (Other than pugixml files).
##

l: library
lib: library
library: makedirs min pugixml $(OBJS)
	@-rm -f $(LIBDIR)/$(LIBFILE)
	@$(AR) r $(LIBDIR)/$(LIBFILE) $(OBJLIST)
	@$(RANLIB) $(LIBDIR)/$(LIBFILE)



##############################
##
## min: Create combined humlib files min/humlib.cpp and min/humlib.h
##   for use in external projects.
##

m: min
min:
	@$(BINDIR)/makeMinDistribution



##############################
##
## min-test: Test compiling of the combined min/humlib.cpp file.
##   for use in external projects.
##

mt: min-test
min-test:
	$(COMPILER) $(PREFLAGS) -I$(MINDIR) -c $(MINDIR)/humlib.cpp -o $(OBJDIR)/humlib.o $(POSTFLAGS)



##############################
##
## clean-lib: Erase library directory.
##

c: clean
co: clean-obj
clean-obj: clean
clean:
	@echo Erasing object files...
	@-rm -f $(OBJDIR)/*.o $(OBJDIR)/humlib.o
	@echo Erasing obj directory...
	@-rmdir $(OBJDIR)



##############################
##
## clean-lib: Erase library directory.
##

cl: clean-lib
clean-library: clean-lib
clean-lib:
	@echo Erasing library files...
	@-rm -f $(LIBDIR)/*.a
	@echo Erasing lib directory...
	@-rmdir $(LIBDIR)



##############################
##
## clean-bin: Erase compiled cli programs in the bin directory.
##

cb: clean-bin
clean-progs: clean-bin
clean-binary: clean-bin
clean-bin:
	# Does not erase test programs
	@echo Erasing compiled programs in bin...
	(cd $(BINDIR) && rm -f $(CLILIST))



##############################
##
## superclean: Erase object, library and compiled cli programs.
##

superclean: clean-obj clean-lib clean-bin



##############################
##
## programs: Compile all programs in the cli directory.  Compiled programs
##    are placed in the bin directory.   After compiling, you might want
##    to run "make strip" to remove debugging information.   You might
##    also want to run "make install" to copy the programs to /usr/local/bin.
##    In that case "make strip" is run automatically.
##

cli: programs
cli: programs
examples: programs
prog: programs
progs: programs
tool: programs
tools: programs
programs:
	@$(MAKE) -f Makefile.programs



##############################
##
## strip: Remove debugging information from compiled programs.
##     This makes the files sizes much smaller, so it is useful
##     to do this if you are not going to debug the programs with
##     gdb, for example.   When installing the programs into /usr/local/bin/,
##     they will be stripped automatically before copying.
##
##

strip:
	@(cd $(BINDIR) && strip $(CLILIST))



##############################
##
## install: Copy CLI programs from "bin" to /usr/local/bin.  You should check
##     that there are not any naming conflicts with programs already in /usr/local/bin.
##     In general this should not be a problem.   Usually /usr/local/bin is already
##     included in the $PATH shell variable.   If not, then you cannot access the
##     programs on the command-line without adding /usr/local/bin to your PATH.
##     After running "install" then type "which flipper" which should reply:
##        /usr/local/bin/flipper
##     If not then check the path:
##        echo $PATH
##     or to view the list of directories more easily:
##        echo $PATH | tr : "\n"
##     /usr/local/bin should be in the list.  If not, then add to the PATH.  This will
##     depend on your shell.   Check the shell name with the command:
##        echo $SHELL
##     For bash shells, typically add the line in the file .profile (if it already exists),
##        ~/.bashrc_profile  or ~/.bashrc  otherwise:
##        export PATH = "/usr/local/bin:$PATH
##     Then close the shell (terminal) then open a new one and try "which flipper" again.
##     If not found, then try another configuration filename to add the PATH update to.
##
##     You will likely have to run this make target as:
##        sudo make install
##     (try without sudo first, then if that fails use sudo).
##

install: strip
	@echo Copying cli tools to /usr/local/bin
	@(cd $(BINDIR) && cp -f $(CLILIST) /usr/local/bin/)



##############################
##
## makedirs: Create directories to store object and library files.
##

md: makedirs
dirs: makedirs
makedirs:
	@-mkdir -p $(OBJDIR)
	@-mkdir -p $(LIBDIR)



##############################
##
## %: Default rule.  If the target is unknown, then assume it is a program
##      to compile from the cli directory.
##

%:
	@echo 'if [ "$<" == "" ]; then $(MAKE) -f Makefile.programs $@; fi' | bash -s



###########################################################################
#                                                                         #
# Dependencies -- generated with the following command in                 #
#      the src directory (in bash shell):                                 #
#                                                                         #
# for i in src/*.cpp
# do
#    cc -std=c++11 -Iinclude -MM $i \
#    | sed 's/include\///g; s/src\///g; s/pugixml.hpp//g; s/pugiconfig.hpp//g'
#    echo
# done
#

Convert-harmony.o: Convert-harmony.cpp Convert.h \
  HumNum.h HumdrumToken.h HumAddress.h \
  HumHash.h HumParamSet.h HumRegex.h

Convert-kern.o: Convert-kern.cpp Convert.h HumNum.h \
  HumdrumToken.h HumAddress.h HumHash.h \
  HumParamSet.h

Convert-math.o: Convert-math.cpp Convert.h HumNum.h \
  HumdrumToken.h HumAddress.h HumHash.h \
  HumParamSet.h

Convert-mens.o: Convert-mens.cpp Convert.h HumNum.h \
  HumdrumToken.h HumAddress.h HumHash.h \
  HumParamSet.h HumRegex.h

Convert-musedata.o: Convert-musedata.cpp Convert.h \
  HumNum.h HumdrumToken.h HumAddress.h \
  HumHash.h HumParamSet.h

Convert-pitch.o: Convert-pitch.cpp Convert.h HumNum.h \
  HumdrumToken.h HumAddress.h HumHash.h \
  HumParamSet.h HumRegex.h

Convert-reference.o: Convert-reference.cpp Convert.h \
  HumNum.h HumdrumToken.h HumAddress.h \
  HumHash.h HumParamSet.h HumRegex.h

Convert-rhythm.o: Convert-rhythm.cpp Convert.h \
  HumNum.h HumdrumToken.h HumAddress.h \
  HumHash.h HumParamSet.h HumRegex.h

Convert-string.o: Convert-string.cpp Convert.h \
  HumNum.h HumdrumToken.h HumAddress.h \
  HumHash.h HumParamSet.h HumRegex.h

Convert-tempo.o: Convert-tempo.cpp Convert.h HumNum.h \
  HumdrumToken.h HumAddress.h HumHash.h \
  HumParamSet.h HumRegex.h

GridMeasure.o: GridMeasure.cpp HumGrid.h \
  GridMeasure.h GridCommon.h HumdrumFile.h \
  HumdrumFileContent.h HumdrumFileStructure.h \
  HumdrumFileBase.h HumSignifiers.h \
  HumSignifier.h HumdrumLine.h HumdrumToken.h \
  HumNum.h HumAddress.h HumHash.h \
  HumParamSet.h GridSlice.h MxmlPart.h \
  MxmlMeasure.h   \
  GridPart.h GridStaff.h GridSide.h \
  GridVoice.h

GridPart.o: GridPart.cpp GridPart.h GridStaff.h \
  GridCommon.h GridSide.h HumdrumToken.h \
  HumNum.h HumAddress.h HumHash.h \
  HumParamSet.h GridVoice.h

GridSide.o: GridSide.cpp HumGrid.h GridMeasure.h \
  GridCommon.h HumdrumFile.h \
  HumdrumFileContent.h HumdrumFileStructure.h \
  HumdrumFileBase.h HumSignifiers.h \
  HumSignifier.h HumdrumLine.h HumdrumToken.h \
  HumNum.h HumAddress.h HumHash.h \
  HumParamSet.h GridSlice.h MxmlPart.h \
  MxmlMeasure.h   \
  GridPart.h GridStaff.h GridSide.h \
  GridVoice.h

GridSlice.o: GridSlice.cpp HumGrid.h GridMeasure.h \
  GridCommon.h HumdrumFile.h \
  HumdrumFileContent.h HumdrumFileStructure.h \
  HumdrumFileBase.h HumSignifiers.h \
  HumSignifier.h HumdrumLine.h HumdrumToken.h \
  HumNum.h HumAddress.h HumHash.h \
  HumParamSet.h GridSlice.h MxmlPart.h \
  MxmlMeasure.h   \
  GridPart.h GridStaff.h GridSide.h \
  GridVoice.h

GridStaff.o: GridStaff.cpp HumGrid.h GridMeasure.h \
  GridCommon.h HumdrumFile.h \
  HumdrumFileContent.h HumdrumFileStructure.h \
  HumdrumFileBase.h HumSignifiers.h \
  HumSignifier.h HumdrumLine.h HumdrumToken.h \
  HumNum.h HumAddress.h HumHash.h \
  HumParamSet.h GridSlice.h MxmlPart.h \
  MxmlMeasure.h   \
  GridPart.h GridStaff.h GridSide.h \
  GridVoice.h

GridVoice.o: GridVoice.cpp GridVoice.h HumdrumToken.h \
  HumNum.h HumAddress.h HumHash.h \
  HumParamSet.h

HumAddress.o: HumAddress.cpp HumAddress.h \
  HumdrumLine.h HumdrumToken.h HumNum.h \
  HumHash.h HumParamSet.h

HumGrid.o: HumGrid.cpp HumGrid.h GridMeasure.h \
  GridCommon.h HumdrumFile.h \
  HumdrumFileContent.h HumdrumFileStructure.h \
  HumdrumFileBase.h HumSignifiers.h \
  HumSignifier.h HumdrumLine.h HumdrumToken.h \
  HumNum.h HumAddress.h HumHash.h \
  HumParamSet.h GridSlice.h MxmlPart.h \
  MxmlMeasure.h   \
  GridPart.h GridStaff.h GridSide.h \
  GridVoice.h Convert.h

HumHash.o: HumHash.cpp HumHash.h HumNum.h \
  Convert.h HumdrumToken.h HumAddress.h \
  HumParamSet.h

HumInstrument.o: HumInstrument.cpp HumInstrument.h

HumNum.o: HumNum.cpp HumNum.h

HumParamSet.o: HumParamSet.cpp HumParamSet.h \
  HumdrumToken.h HumNum.h HumAddress.h \
  HumHash.h Convert.h

HumPitch.o: HumPitch.cpp HumPitch.h HumRegex.h

HumRegex.o: HumRegex.cpp HumRegex.h

HumSignifier.o: HumSignifier.cpp HumSignifier.h \
  HumRegex.h

HumSignifiers.o: HumSignifiers.cpp HumSignifiers.h \
  HumSignifier.h

HumTool.o: HumTool.cpp HumTool.h Options.h \
  HumdrumFileSet.h HumdrumFile.h \
  HumdrumFileContent.h HumdrumFileStructure.h \
  HumdrumFileBase.h HumSignifiers.h \
  HumSignifier.h HumdrumLine.h HumdrumToken.h \
  HumNum.h HumAddress.h HumHash.h \
  HumParamSet.h HumdrumFileStream.h

HumTransposer.o: HumTransposer.cpp HumTransposer.h \
  HumPitch.h

HumdrumFile.o: HumdrumFile.cpp HumdrumFile.h \
  HumdrumFileContent.h HumdrumFileStructure.h \
  HumdrumFileBase.h HumSignifiers.h \
  HumSignifier.h HumdrumLine.h HumdrumToken.h \
  HumNum.h HumAddress.h HumHash.h \
  HumParamSet.h Convert.h

HumdrumFileBase-net.o: HumdrumFileBase-net.cpp \
  HumdrumFileBase.h HumSignifiers.h \
  HumSignifier.h HumdrumLine.h HumdrumToken.h \
  HumNum.h HumAddress.h HumHash.h \
  HumParamSet.h Convert.h

HumdrumFileBase.o: HumdrumFileBase.cpp HumdrumFileBase.h \
  HumSignifiers.h HumSignifier.h HumdrumLine.h \
  HumdrumToken.h HumNum.h HumAddress.h \
  HumHash.h HumParamSet.h Convert.h \
  HumRegex.h

HumdrumFileContent-accidental.o: HumdrumFileContent-accidental.cpp \
  HumdrumFileContent.h HumdrumFileStructure.h \
  HumdrumFileBase.h HumSignifiers.h \
  HumSignifier.h HumdrumLine.h HumdrumToken.h \
  HumNum.h HumAddress.h HumHash.h \
  HumParamSet.h Convert.h

HumdrumFileContent-barline.o: HumdrumFileContent-barline.cpp \
  HumdrumFileContent.h HumdrumFileStructure.h \
  HumdrumFileBase.h HumSignifiers.h \
  HumSignifier.h HumdrumLine.h HumdrumToken.h \
  HumNum.h HumAddress.h HumHash.h \
  HumParamSet.h

HumdrumFileContent-beam.o: HumdrumFileContent-beam.cpp \
  HumdrumFileContent.h HumdrumFileStructure.h \
  HumdrumFileBase.h HumSignifiers.h \
  HumSignifier.h HumdrumLine.h HumdrumToken.h \
  HumNum.h HumAddress.h HumHash.h \
  HumParamSet.h

HumdrumFileContent-metlev.o: HumdrumFileContent-metlev.cpp \
  HumdrumFileContent.h HumdrumFileStructure.h \
  HumdrumFileBase.h HumSignifiers.h \
  HumSignifier.h HumdrumLine.h HumdrumToken.h \
  HumNum.h HumAddress.h HumHash.h \
  HumParamSet.h Convert.h

HumdrumFileContent-note.o: HumdrumFileContent-note.cpp \
  HumdrumFileContent.h HumdrumFileStructure.h \
  HumdrumFileBase.h HumSignifiers.h \
  HumSignifier.h HumdrumLine.h HumdrumToken.h \
  HumNum.h HumAddress.h HumHash.h \
  HumParamSet.h HumRegex.h Convert.h

HumdrumFileContent-ottava.o: HumdrumFileContent-ottava.cpp \
  HumdrumFileContent.h HumdrumFileStructure.h \
  HumdrumFileBase.h HumSignifiers.h \
  HumSignifier.h HumdrumLine.h HumdrumToken.h \
  HumNum.h HumAddress.h HumHash.h \
  HumParamSet.h

HumdrumFileContent-phrase.o: HumdrumFileContent-phrase.cpp \
  HumdrumFileContent.h HumdrumFileStructure.h \
  HumdrumFileBase.h HumSignifiers.h \
  HumSignifier.h HumdrumLine.h HumdrumToken.h \
  HumNum.h HumAddress.h HumHash.h \
  HumParamSet.h

HumdrumFileContent-rest.o: HumdrumFileContent-rest.cpp \
  HumdrumFileContent.h HumdrumFileStructure.h \
  HumdrumFileBase.h HumSignifiers.h \
  HumSignifier.h HumdrumLine.h HumdrumToken.h \
  HumNum.h HumAddress.h HumHash.h \
  HumParamSet.h HumRegex.h Convert.h

HumdrumFileContent-slur.o: HumdrumFileContent-slur.cpp \
  HumdrumFileContent.h HumdrumFileStructure.h \
  HumdrumFileBase.h HumSignifiers.h \
  HumSignifier.h HumdrumLine.h HumdrumToken.h \
  HumNum.h HumAddress.h HumHash.h \
  HumParamSet.h

HumdrumFileContent-stemlengths.o: HumdrumFileContent-stemlengths.cpp \
  HumdrumFileContent.h HumdrumFileStructure.h \
  HumdrumFileBase.h HumSignifiers.h \
  HumSignifier.h HumdrumLine.h HumdrumToken.h \
  HumNum.h HumAddress.h HumHash.h \
  HumParamSet.h Convert.h

HumdrumFileContent-text.o: HumdrumFileContent-text.cpp \
  HumdrumFileContent.h HumdrumFileStructure.h \
  HumdrumFileBase.h HumSignifiers.h \
  HumSignifier.h HumdrumLine.h HumdrumToken.h \
  HumNum.h HumAddress.h HumHash.h \
  HumParamSet.h

HumdrumFileContent-tie.o: HumdrumFileContent-tie.cpp \
  HumdrumFileContent.h HumdrumFileStructure.h \
  HumdrumFileBase.h HumSignifiers.h \
  HumSignifier.h HumdrumLine.h HumdrumToken.h \
  HumNum.h HumAddress.h HumHash.h \
  HumParamSet.h Convert.h

HumdrumFileContent-timesig.o: HumdrumFileContent-timesig.cpp \
  HumdrumFileContent.h HumdrumFileStructure.h \
  HumdrumFileBase.h HumSignifiers.h \
  HumSignifier.h HumdrumLine.h HumdrumToken.h \
  HumNum.h HumAddress.h HumHash.h \
  HumParamSet.h Convert.h

HumdrumFileContent.o: HumdrumFileContent.cpp \
  HumdrumFileContent.h HumdrumFileStructure.h \
  HumdrumFileBase.h HumSignifiers.h \
  HumSignifier.h HumdrumLine.h HumdrumToken.h \
  HumNum.h HumAddress.h HumHash.h \
  HumParamSet.h HumRegex.h Convert.h

HumdrumFileSet.o: HumdrumFileSet.cpp HumdrumFileSet.h \
  HumdrumFile.h HumdrumFileContent.h \
  HumdrumFileStructure.h HumdrumFileBase.h \
  HumSignifiers.h HumSignifier.h HumdrumLine.h \
  HumdrumToken.h HumNum.h HumAddress.h \
  HumHash.h HumParamSet.h HumdrumFileStream.h \
  Options.h

HumdrumFileStream.o: HumdrumFileStream.cpp \
  HumdrumFileStream.h HumdrumFile.h \
  HumdrumFileContent.h HumdrumFileStructure.h \
  HumdrumFileBase.h HumSignifiers.h \
  HumSignifier.h HumdrumLine.h HumdrumToken.h \
  HumNum.h HumAddress.h HumHash.h \
  HumParamSet.h Options.h HumdrumFileSet.h \
  HumRegex.h

HumdrumFileStructure-strophe.o: HumdrumFileStructure-strophe.cpp \
  HumdrumFileStructure.h HumdrumFileBase.h \
  HumSignifiers.h HumSignifier.h HumdrumLine.h \
  HumdrumToken.h HumNum.h HumAddress.h \
  HumHash.h HumParamSet.h

HumdrumFileStructure.o: HumdrumFileStructure.cpp \
  HumdrumFileStructure.h HumdrumFileBase.h \
  HumSignifiers.h HumSignifier.h HumdrumLine.h \
  HumdrumToken.h HumNum.h HumAddress.h \
  HumHash.h HumParamSet.h Convert.h

HumdrumLine-kern.o: HumdrumLine-kern.cpp HumdrumLine.h \
  HumdrumToken.h HumNum.h HumAddress.h \
  HumHash.h HumParamSet.h

HumdrumLine.o: HumdrumLine.cpp HumdrumLine.h \
  HumdrumToken.h HumNum.h HumAddress.h \
  HumHash.h HumParamSet.h HumdrumFile.h \
  HumdrumFileContent.h HumdrumFileStructure.h \
  HumdrumFileBase.h HumSignifiers.h \
  HumSignifier.h Convert.h

HumdrumToken-base40.o: HumdrumToken-base40.cpp HumdrumToken.h \
  HumNum.h HumAddress.h HumHash.h \
  HumParamSet.h Convert.h

HumdrumToken-midi.o: HumdrumToken-midi.cpp HumdrumToken.h \
  HumNum.h HumAddress.h HumHash.h \
  HumParamSet.h Convert.h

HumdrumToken.o: HumdrumToken.cpp HumAddress.h \
  HumdrumToken.h HumNum.h HumHash.h \
  HumParamSet.h HumdrumLine.h HumdrumFile.h \
  HumdrumFileContent.h HumdrumFileStructure.h \
  HumdrumFileBase.h HumSignifiers.h \
  HumSignifier.h Convert.h HumRegex.h

MuseData.o: MuseData.cpp MuseData.h MuseRecord.h \
  MuseRecordBasic.h HumNum.h HumdrumToken.h \
  HumAddress.h HumHash.h HumParamSet.h \
  GridVoice.h HumRegex.h

MuseDataSet.o: MuseDataSet.cpp MuseDataSet.h \
  MuseData.h MuseRecord.h MuseRecordBasic.h \
  HumNum.h HumdrumToken.h HumAddress.h \
  HumHash.h HumParamSet.h GridVoice.h \
  HumRegex.h

MuseRecord.o: MuseRecord.cpp Convert.h HumNum.h \
  HumdrumToken.h HumAddress.h HumHash.h \
  HumParamSet.h MuseRecord.h MuseRecordBasic.h \
  GridVoice.h MuseData.h HumRegex.h

MuseRecordBasic.o: MuseRecordBasic.cpp MuseRecordBasic.h \
  HumNum.h HumdrumToken.h HumAddress.h \
  HumHash.h HumParamSet.h GridVoice.h

MxmlEvent.o: MxmlEvent.cpp MxmlEvent.h GridCommon.h \
  HumNum.h   \
  MxmlMeasure.h Convert.h HumdrumToken.h \
  HumAddress.h HumHash.h HumParamSet.h

MxmlMeasure.o: MxmlMeasure.cpp MxmlEvent.h \
  GridCommon.h HumNum.h  \
   MxmlMeasure.h MxmlPart.h

MxmlPart.o: MxmlPart.cpp MxmlMeasure.h GridCommon.h \
  HumNum.h   \
  MxmlPart.h

NoteCell.o: NoteCell.cpp NoteCell.h HumdrumFile.h \
  HumdrumFileContent.h HumdrumFileStructure.h \
  HumdrumFileBase.h HumSignifiers.h \
  HumSignifier.h HumdrumLine.h HumdrumToken.h \
  HumNum.h HumAddress.h HumHash.h \
  HumParamSet.h NoteGrid.h Convert.h

NoteGrid.o: NoteGrid.cpp NoteGrid.h NoteCell.h \
  HumdrumFile.h HumdrumFileContent.h \
  HumdrumFileStructure.h HumdrumFileBase.h \
  HumSignifiers.h HumSignifier.h HumdrumLine.h \
  HumdrumToken.h HumNum.h HumAddress.h \
  HumHash.h HumParamSet.h HumRegex.h

Options.o: Options.cpp Options.h

PixelColor.o: PixelColor.cpp PixelColor.h

pugixml.o: pugixml.cpp  

tool-autoaccid.o: tool-autoaccid.cpp tool-autoaccid.h \
  HumTool.h Options.h HumdrumFileSet.h \
  HumdrumFile.h HumdrumFileContent.h \
  HumdrumFileStructure.h HumdrumFileBase.h \
  HumSignifiers.h HumSignifier.h HumdrumLine.h \
  HumdrumToken.h HumNum.h HumAddress.h \
  HumHash.h HumParamSet.h HumdrumFileStream.h \
  Convert.h HumRegex.h

tool-autobeam.o: tool-autobeam.cpp Convert.h HumNum.h \
  HumdrumToken.h HumAddress.h HumHash.h \
  HumParamSet.h HumRegex.h tool-autobeam.h \
  HumTool.h Options.h HumdrumFileSet.h \
  HumdrumFile.h HumdrumFileContent.h \
  HumdrumFileStructure.h HumdrumFileBase.h \
  HumSignifiers.h HumSignifier.h HumdrumLine.h \
  HumdrumFileStream.h

tool-autostem.o: tool-autostem.cpp tool-autostem.h \
  HumTool.h Options.h HumdrumFileSet.h \
  HumdrumFile.h HumdrumFileContent.h \
  HumdrumFileStructure.h HumdrumFileBase.h \
  HumSignifiers.h HumSignifier.h HumdrumLine.h \
  HumdrumToken.h HumNum.h HumAddress.h \
  HumHash.h HumParamSet.h HumdrumFileStream.h \
  HumRegex.h Convert.h

tool-binroll.o: tool-binroll.cpp tool-binroll.h \
  HumTool.h Options.h HumdrumFileSet.h \
  HumdrumFile.h HumdrumFileContent.h \
  HumdrumFileStructure.h HumdrumFileBase.h \
  HumSignifiers.h HumSignifier.h HumdrumLine.h \
  HumdrumToken.h HumNum.h HumAddress.h \
  HumHash.h HumParamSet.h HumdrumFileStream.h \
  Convert.h HumRegex.h

tool-chantize.o: tool-chantize.cpp tool-chantize.h \
  HumTool.h Options.h HumdrumFileSet.h \
  HumdrumFile.h HumdrumFileContent.h \
  HumdrumFileStructure.h HumdrumFileBase.h \
  HumSignifiers.h HumSignifier.h HumdrumLine.h \
  HumdrumToken.h HumNum.h HumAddress.h \
  HumHash.h HumParamSet.h HumdrumFileStream.h \
  Convert.h HumRegex.h tool-shed.h

tool-chooser.o: tool-chooser.cpp tool-chooser.h \
  HumTool.h Options.h HumdrumFileSet.h \
  HumdrumFile.h HumdrumFileContent.h \
  HumdrumFileStructure.h HumdrumFileBase.h \
  HumSignifiers.h HumSignifier.h HumdrumLine.h \
  HumdrumToken.h HumNum.h HumAddress.h \
  HumHash.h HumParamSet.h HumdrumFileStream.h \
  HumRegex.h Convert.h

tool-chord.o: tool-chord.cpp tool-chord.h HumTool.h \
  Options.h HumdrumFileSet.h HumdrumFile.h \
  HumdrumFileContent.h HumdrumFileStructure.h \
  HumdrumFileBase.h HumSignifiers.h \
  HumSignifier.h HumdrumLine.h HumdrumToken.h \
  HumNum.h HumAddress.h HumHash.h \
  HumParamSet.h HumdrumFileStream.h Convert.h \
  HumRegex.h

tool-cint.o: tool-cint.cpp tool-cint.h HumTool.h \
  Options.h HumdrumFileSet.h HumdrumFile.h \
  HumdrumFileContent.h HumdrumFileStructure.h \
  HumdrumFileBase.h HumSignifiers.h \
  HumSignifier.h HumdrumLine.h HumdrumToken.h \
  HumNum.h HumAddress.h HumHash.h \
  HumParamSet.h HumdrumFileStream.h NoteGrid.h \
  NoteCell.h HumRegex.h Convert.h

tool-cmr.o: tool-cmr.cpp tool-cmr.h HumTool.h \
  Options.h HumdrumFileSet.h HumdrumFile.h \
  HumdrumFileContent.h HumdrumFileStructure.h \
  HumdrumFileBase.h HumSignifiers.h \
  HumSignifier.h HumdrumLine.h HumdrumToken.h \
  HumNum.h HumAddress.h HumHash.h \
  HumParamSet.h HumdrumFileStream.h Convert.h \
  HumRegex.h

tool-colorgroups.o: tool-colorgroups.cpp tool-colorgroups.h \
  HumTool.h Options.h HumdrumFileSet.h \
  HumdrumFile.h HumdrumFileContent.h \
  HumdrumFileStructure.h HumdrumFileBase.h \
  HumSignifiers.h HumSignifier.h HumdrumLine.h \
  HumdrumToken.h HumNum.h HumAddress.h \
  HumHash.h HumParamSet.h HumdrumFileStream.h \
  tool-shed.h

tool-colortriads.o: tool-colortriads.cpp tool-colortriads.h \
  HumTool.h Options.h HumdrumFileSet.h \
  HumdrumFile.h HumdrumFileContent.h \
  HumdrumFileStructure.h HumdrumFileBase.h \
  HumSignifiers.h HumSignifier.h HumdrumLine.h \
  HumdrumToken.h HumNum.h HumAddress.h \
  HumHash.h HumParamSet.h HumdrumFileStream.h \
  tool-msearch.h NoteGrid.h NoteCell.h \
  Convert.h HumRegex.h

tool-composite.o: tool-composite.cpp tool-composite.h \
  HumTool.h Options.h HumdrumFileSet.h \
  HumdrumFile.h HumdrumFileContent.h \
  HumdrumFileStructure.h HumdrumFileBase.h \
  HumSignifiers.h HumSignifier.h HumdrumLine.h \
  HumdrumToken.h HumNum.h HumAddress.h \
  HumHash.h HumParamSet.h HumdrumFileStream.h \
  tool-extract.h tool-autobeam.h Convert.h \
  HumRegex.h

tool-compositeold.o: tool-compositeold.cpp \
  tool-compositeold.h HumTool.h Options.h \
  HumdrumFileSet.h HumdrumFile.h \
  HumdrumFileContent.h HumdrumFileStructure.h \
  HumdrumFileBase.h HumSignifiers.h \
  HumSignifier.h HumdrumLine.h HumdrumToken.h \
  HumNum.h HumAddress.h HumHash.h \
  HumParamSet.h HumdrumFileStream.h \
  tool-extract.h tool-autobeam.h Convert.h \
  HumRegex.h

tool-deg.o: tool-deg.cpp tool-deg.h HumTool.h \
  Options.h HumdrumFileSet.h HumdrumFile.h \
  HumdrumFileContent.h HumdrumFileStructure.h \
  HumdrumFileBase.h HumSignifiers.h \
  HumSignifier.h HumdrumLine.h HumdrumToken.h \
  HumNum.h HumAddress.h HumHash.h \
  HumParamSet.h HumdrumFileStream.h HumRegex.h \
  Convert.h

tool-dissonant.o: tool-dissonant.cpp tool-dissonant.h \
  HumTool.h Options.h HumdrumFileSet.h \
  HumdrumFile.h HumdrumFileContent.h \
  HumdrumFileStructure.h HumdrumFileBase.h \
  HumSignifiers.h HumSignifier.h HumdrumLine.h \
  HumdrumToken.h HumNum.h HumAddress.h \
  HumHash.h HumParamSet.h HumdrumFileStream.h \
  NoteGrid.h NoteCell.h Convert.h \
  HumRegex.h

tool-double.o: tool-double.cpp tool-double.h \
  HumTool.h Options.h HumdrumFileSet.h \
  HumdrumFile.h HumdrumFileContent.h \
  HumdrumFileStructure.h HumdrumFileBase.h \
  HumSignifiers.h HumSignifier.h HumdrumLine.h \
  HumdrumToken.h HumNum.h HumAddress.h \
  HumHash.h HumParamSet.h HumdrumFileStream.h \
  Convert.h HumRegex.h

tool-esac2hum.o: tool-esac2hum.cpp tool-esac2hum.h \
  HumTool.h Options.h HumdrumFileSet.h \
  HumdrumFile.h HumdrumFileContent.h \
  HumdrumFileStructure.h HumdrumFileBase.h \
  HumSignifiers.h HumSignifier.h HumdrumLine.h \
  HumdrumToken.h HumNum.h HumAddress.h \
  HumHash.h HumParamSet.h HumdrumFileStream.h \
  Convert.h HumRegex.h

tool-extract.o: tool-extract.cpp tool-extract.h \
  HumTool.h Options.h HumdrumFileSet.h \
  HumdrumFile.h HumdrumFileContent.h \
  HumdrumFileStructure.h HumdrumFileBase.h \
  HumSignifiers.h HumSignifier.h HumdrumLine.h \
  HumdrumToken.h HumNum.h HumAddress.h \
  HumHash.h HumParamSet.h HumdrumFileStream.h \
  HumRegex.h

tool-fb.o: tool-fb.cpp tool-fb.h HumTool.h \
  Options.h HumdrumFileSet.h HumdrumFile.h \
  HumdrumFileContent.h HumdrumFileStructure.h \
  HumdrumFileBase.h HumSignifiers.h \
  HumSignifier.h HumdrumLine.h HumdrumToken.h \
  HumNum.h HumAddress.h HumHash.h \
  HumParamSet.h HumdrumFileStream.h NoteGrid.h \
  NoteCell.h Convert.h HumRegex.h

tool-filter.o: tool-filter.cpp tool-filter.h \
  HumTool.h Options.h HumdrumFileSet.h \
  HumdrumFile.h HumdrumFileContent.h \
  HumdrumFileStructure.h HumdrumFileBase.h \
  HumSignifiers.h HumSignifier.h HumdrumLine.h \
  HumdrumToken.h HumNum.h HumAddress.h \
  HumHash.h HumParamSet.h HumdrumFileStream.h \
  tool-autoaccid.h tool-autobeam.h \
  tool-autostem.h tool-binroll.h tool-chantize.h \
  tool-chooser.h tool-chord.h tool-cint.h \
  NoteGrid.h NoteCell.h HumRegex.h \
  tool-cmr.h tool-colorgroups.h \
  tool-colortriads.h tool-composite.h tool-deg.h \
  tool-dissonant.h tool-double.h tool-extract.h \
  tool-fb.h tool-flipper.h tool-gasparize.h \
  tool-grep.h tool-half.h tool-homorhythm.h \
  tool-homorhythm2.h tool-hproof.h \
  tool-humdiff.h tool-humsheet.h tool-humtr.h \
  tool-imitation.h tool-kern2mens.h \
  tool-kernify.h tool-kernview.h tool-mei2hum.h \
    MxmlPart.h \
  MxmlMeasure.h GridCommon.h MxmlEvent.h \
  HumGrid.h GridMeasure.h GridSlice.h \
  GridPart.h GridStaff.h GridSide.h \
  GridVoice.h tool-melisma.h tool-mens2kern.h \
  tool-metlev.h tool-modori.h tool-msearch.h \
  Convert.h tool-myank.h tool-ordergps.h \
  tool-phrase.h tool-recip.h tool-restfill.h \
  tool-rid.h tool-satb2gs.h tool-scordatura.h \
  HumTransposer.h HumPitch.h tool-semitones.h \
  tool-shed.h tool-sic.h tool-simat.h \
  tool-slurcheck.h tool-spinetrace.h \
  tool-strophe.h tool-synco.h tool-tabber.h \
  tool-tassoize.h tool-thru.h tool-tie.h \
  tool-timebase.h tool-tpos.h tool-transpose.h \
  tool-tremolo.h tool-trillspell.h

tool-fixps.o: tool-fixps.cpp tool-fixps.h HumTool.h \
  Options.h HumdrumFileSet.h HumdrumFile.h \
  HumdrumFileContent.h HumdrumFileStructure.h \
  HumdrumFileBase.h HumSignifiers.h \
  HumSignifier.h HumdrumLine.h HumdrumToken.h \
  HumNum.h HumAddress.h HumHash.h \
  HumParamSet.h HumdrumFileStream.h Convert.h

tool-flipper.o: tool-flipper.cpp tool-flipper.h \
  HumTool.h Options.h HumdrumFileSet.h \
  HumdrumFile.h HumdrumFileContent.h \
  HumdrumFileStructure.h HumdrumFileBase.h \
  HumSignifiers.h HumSignifier.h HumdrumLine.h \
  HumdrumToken.h HumNum.h HumAddress.h \
  HumHash.h HumParamSet.h HumdrumFileStream.h \
  Convert.h HumRegex.h

tool-gasparize.o: tool-gasparize.cpp tool-gasparize.h \
  HumTool.h Options.h HumdrumFileSet.h \
  HumdrumFile.h HumdrumFileContent.h \
  HumdrumFileStructure.h HumdrumFileBase.h \
  HumSignifiers.h HumSignifier.h HumdrumLine.h \
  HumdrumToken.h HumNum.h HumAddress.h \
  HumHash.h HumParamSet.h HumdrumFileStream.h \
  Convert.h HumRegex.h tool-shed.h

tool-grep.o: tool-grep.cpp tool-grep.h HumTool.h \
  Options.h HumdrumFileSet.h HumdrumFile.h \
  HumdrumFileContent.h HumdrumFileStructure.h \
  HumdrumFileBase.h HumSignifiers.h \
  HumSignifier.h HumdrumLine.h HumdrumToken.h \
  HumNum.h HumAddress.h HumHash.h \
  HumParamSet.h HumdrumFileStream.h HumRegex.h

tool-half.o: tool-half.cpp tool-half.h HumTool.h \
  Options.h HumdrumFileSet.h HumdrumFile.h \
  HumdrumFileContent.h HumdrumFileStructure.h \
  HumdrumFileBase.h HumSignifiers.h \
  HumSignifier.h HumdrumLine.h HumdrumToken.h \
  HumNum.h HumAddress.h HumHash.h \
  HumParamSet.h HumdrumFileStream.h Convert.h \
  HumRegex.h tool-autobeam.h

tool-homorhythm.o: tool-homorhythm.cpp tool-homorhythm.h \
  HumTool.h Options.h HumdrumFileSet.h \
  HumdrumFile.h HumdrumFileContent.h \
  HumdrumFileStructure.h HumdrumFileBase.h \
  HumSignifiers.h HumSignifier.h HumdrumLine.h \
  HumdrumToken.h HumNum.h HumAddress.h \
  HumHash.h HumParamSet.h HumdrumFileStream.h \
  Convert.h HumRegex.h

tool-homorhythm2.o: tool-homorhythm2.cpp tool-homorhythm2.h \
  HumTool.h Options.h HumdrumFileSet.h \
  HumdrumFile.h HumdrumFileContent.h \
  HumdrumFileStructure.h HumdrumFileBase.h \
  HumSignifiers.h HumSignifier.h HumdrumLine.h \
  HumdrumToken.h HumNum.h HumAddress.h \
  HumHash.h HumParamSet.h HumdrumFileStream.h \
  Convert.h NoteGrid.h NoteCell.h

tool-hproof.o: tool-hproof.cpp tool-hproof.h \
  HumTool.h Options.h HumdrumFileSet.h \
  HumdrumFile.h HumdrumFileContent.h \
  HumdrumFileStructure.h HumdrumFileBase.h \
  HumSignifiers.h HumSignifier.h HumdrumLine.h \
  HumdrumToken.h HumNum.h HumAddress.h \
  HumHash.h HumParamSet.h HumdrumFileStream.h \
  Convert.h

tool-humdiff.o: tool-humdiff.cpp tool-humdiff.h \
  HumTool.h Options.h HumdrumFileSet.h \
  HumdrumFile.h HumdrumFileContent.h \
  HumdrumFileStructure.h HumdrumFileBase.h \
  HumSignifiers.h HumSignifier.h HumdrumLine.h \
  HumdrumToken.h HumNum.h HumAddress.h \
  HumHash.h HumParamSet.h HumdrumFileStream.h \
  HumRegex.h Convert.h

tool-humsheet.o: tool-humsheet.cpp tool-humsheet.h \
  HumTool.h Options.h HumdrumFileSet.h \
  HumdrumFile.h HumdrumFileContent.h \
  HumdrumFileStructure.h HumdrumFileBase.h \
  HumSignifiers.h HumSignifier.h HumdrumLine.h \
  HumdrumToken.h HumNum.h HumAddress.h \
  HumHash.h HumParamSet.h HumdrumFileStream.h \
  Convert.h

tool-humsort.o: tool-humsort.cpp tool-humsort.h \
  HumTool.h Options.h HumdrumFileSet.h \
  HumdrumFile.h HumdrumFileContent.h \
  HumdrumFileStructure.h HumdrumFileBase.h \
  HumSignifiers.h HumSignifier.h HumdrumLine.h \
  HumdrumToken.h HumNum.h HumAddress.h \
  HumHash.h HumParamSet.h HumdrumFileStream.h \
  Convert.h HumRegex.h

tool-humtr.o: tool-humtr.cpp tool-humtr.h HumTool.h \
  Options.h HumdrumFileSet.h HumdrumFile.h \
  HumdrumFileContent.h HumdrumFileStructure.h \
  HumdrumFileBase.h HumSignifiers.h \
  HumSignifier.h HumdrumLine.h HumdrumToken.h \
  HumNum.h HumAddress.h HumHash.h \
  HumParamSet.h HumdrumFileStream.h HumRegex.h

tool-imitation.o: tool-imitation.cpp tool-imitation.h \
  HumTool.h Options.h HumdrumFileSet.h \
  HumdrumFile.h HumdrumFileContent.h \
  HumdrumFileStructure.h HumdrumFileBase.h \
  HumSignifiers.h HumSignifier.h HumdrumLine.h \
  HumdrumToken.h HumNum.h HumAddress.h \
  HumHash.h HumParamSet.h HumdrumFileStream.h \
  NoteGrid.h NoteCell.h Convert.h \
  HumRegex.h

tool-kern2mens.o: tool-kern2mens.cpp tool-kern2mens.h \
  HumTool.h Options.h HumdrumFileSet.h \
  HumdrumFile.h HumdrumFileContent.h \
  HumdrumFileStructure.h HumdrumFileBase.h \
  HumSignifiers.h HumSignifier.h HumdrumLine.h \
  HumdrumToken.h HumNum.h HumAddress.h \
  HumHash.h HumParamSet.h HumdrumFileStream.h \
  NoteGrid.h NoteCell.h Convert.h \
  HumRegex.h

tool-kernify.o: tool-kernify.cpp tool-kernify.h \
  HumTool.h Options.h HumdrumFileSet.h \
  HumdrumFile.h HumdrumFileContent.h \
  HumdrumFileStructure.h HumdrumFileBase.h \
  HumSignifiers.h HumSignifier.h HumdrumLine.h \
  HumdrumToken.h HumNum.h HumAddress.h \
  HumHash.h HumParamSet.h HumdrumFileStream.h \
  HumRegex.h Convert.h

tool-kernview.o: tool-kernview.cpp tool-kernview.h \
  HumTool.h Options.h HumdrumFileSet.h \
  HumdrumFile.h HumdrumFileContent.h \
  HumdrumFileStructure.h HumdrumFileBase.h \
  HumSignifiers.h HumSignifier.h HumdrumLine.h \
  HumdrumToken.h HumNum.h HumAddress.h \
  HumHash.h HumParamSet.h HumdrumFileStream.h \
  Convert.h HumRegex.h

tool-mei2hum.o: tool-mei2hum.cpp tool-mei2hum.h \
  Options.h HumTool.h HumdrumFileSet.h \
  HumdrumFile.h HumdrumFileContent.h \
  HumdrumFileStructure.h HumdrumFileBase.h \
  HumSignifiers.h HumSignifier.h HumdrumLine.h \
  HumdrumToken.h HumNum.h HumAddress.h \
  HumHash.h HumParamSet.h HumdrumFileStream.h \
    MxmlPart.h \
  MxmlMeasure.h GridCommon.h MxmlEvent.h \
  HumGrid.h GridMeasure.h GridSlice.h \
  GridPart.h GridStaff.h GridSide.h \
  GridVoice.h HumRegex.h Convert.h

tool-melisma.o: tool-melisma.cpp tool-melisma.h \
  HumTool.h Options.h HumdrumFileSet.h \
  HumdrumFile.h HumdrumFileContent.h \
  HumdrumFileStructure.h HumdrumFileBase.h \
  HumSignifiers.h HumSignifier.h HumdrumLine.h \
  HumdrumToken.h HumNum.h HumAddress.h \
  HumHash.h HumParamSet.h HumdrumFileStream.h \
  HumRegex.h

tool-mens2kern.o: tool-mens2kern.cpp tool-mens2kern.h \
  HumTool.h Options.h HumdrumFileSet.h \
  HumdrumFile.h HumdrumFileContent.h \
  HumdrumFileStructure.h HumdrumFileBase.h \
  HumSignifiers.h HumSignifier.h HumdrumLine.h \
  HumdrumToken.h HumNum.h HumAddress.h \
  HumHash.h HumParamSet.h HumdrumFileStream.h \
  HumRegex.h

tool-metlev.o: tool-metlev.cpp tool-metlev.h \
  HumTool.h Options.h HumdrumFileSet.h \
  HumdrumFile.h HumdrumFileContent.h \
  HumdrumFileStructure.h HumdrumFileBase.h \
  HumSignifiers.h HumSignifier.h HumdrumLine.h \
  HumdrumToken.h HumNum.h HumAddress.h \
  HumHash.h HumParamSet.h HumdrumFileStream.h \
  Convert.h

tool-modori.o: tool-modori.cpp tool-modori.h \
  HumTool.h Options.h HumdrumFileSet.h \
  HumdrumFile.h HumdrumFileContent.h \
  HumdrumFileStructure.h HumdrumFileBase.h \
  HumSignifiers.h HumSignifier.h HumdrumLine.h \
  HumdrumToken.h HumNum.h HumAddress.h \
  HumHash.h HumParamSet.h HumdrumFileStream.h \
  tool-shed.h HumRegex.h

tool-msearch.o: tool-msearch.cpp tool-msearch.h \
  HumTool.h Options.h HumdrumFileSet.h \
  HumdrumFile.h HumdrumFileContent.h \
  HumdrumFileStructure.h HumdrumFileBase.h \
  HumSignifiers.h HumSignifier.h HumdrumLine.h \
  HumdrumToken.h HumNum.h HumAddress.h \
  HumHash.h HumParamSet.h HumdrumFileStream.h \
  NoteGrid.h NoteCell.h Convert.h \
  HumRegex.h

tool-musedata2hum.o: tool-musedata2hum.cpp \
  tool-musedata2hum.h Options.h MuseDataSet.h \
  MuseData.h MuseRecord.h MuseRecordBasic.h \
  HumNum.h HumdrumToken.h HumAddress.h \
  HumHash.h HumParamSet.h GridVoice.h \
  HumRegex.h HumTool.h HumdrumFileSet.h \
  HumdrumFile.h HumdrumFileContent.h \
  HumdrumFileStructure.h HumdrumFileBase.h \
  HumSignifiers.h HumSignifier.h HumdrumLine.h \
  HumdrumFileStream.h HumGrid.h GridMeasure.h \
  GridCommon.h GridSlice.h MxmlPart.h \
  MxmlMeasure.h   \
  GridPart.h GridStaff.h GridSide.h \
  Convert.h

tool-musicxml2hum.o: tool-musicxml2hum.cpp tool-autobeam.h \
  HumTool.h Options.h HumdrumFileSet.h \
  HumdrumFile.h HumdrumFileContent.h \
  HumdrumFileStructure.h HumdrumFileBase.h \
  HumSignifiers.h HumSignifier.h HumdrumLine.h \
  HumdrumToken.h HumNum.h HumAddress.h \
  HumHash.h HumParamSet.h HumdrumFileStream.h \
  tool-chord.h tool-musicxml2hum.h \
    MxmlPart.h \
  MxmlMeasure.h GridCommon.h MxmlEvent.h \
  HumGrid.h GridMeasure.h GridSlice.h \
  GridPart.h GridStaff.h GridSide.h \
  GridVoice.h tool-ruthfix.h NoteGrid.h \
  NoteCell.h tool-transpose.h tool-tremolo.h \
  tool-trillspell.h Convert.h HumRegex.h

tool-myank.o: tool-myank.cpp tool-myank.h HumTool.h \
  Options.h HumdrumFileSet.h HumdrumFile.h \
  HumdrumFileContent.h HumdrumFileStructure.h \
  HumdrumFileBase.h HumSignifiers.h \
  HumSignifier.h HumdrumLine.h HumdrumToken.h \
  HumNum.h HumAddress.h HumHash.h \
  HumParamSet.h HumdrumFileStream.h HumRegex.h \
  Convert.h

tool-ordergps.o: tool-ordergps.cpp tool-ordergps.h \
  HumTool.h Options.h HumdrumFileSet.h \
  HumdrumFile.h HumdrumFileContent.h \
  HumdrumFileStructure.h HumdrumFileBase.h \
  HumSignifiers.h HumSignifier.h HumdrumLine.h \
  HumdrumToken.h HumNum.h HumAddress.h \
  HumHash.h HumParamSet.h HumdrumFileStream.h

tool-pccount.o: tool-pccount.cpp tool-pccount.h \
  HumTool.h Options.h HumdrumFileSet.h \
  HumdrumFile.h HumdrumFileContent.h \
  HumdrumFileStructure.h HumdrumFileBase.h \
  HumSignifiers.h HumSignifier.h HumdrumLine.h \
  HumdrumToken.h HumNum.h HumAddress.h \
  HumHash.h HumParamSet.h HumdrumFileStream.h \
  HumRegex.h Convert.h

tool-periodicity.o: tool-periodicity.cpp tool-periodicity.h \
  HumTool.h Options.h HumdrumFileSet.h \
  HumdrumFile.h HumdrumFileContent.h \
  HumdrumFileStructure.h HumdrumFileBase.h \
  HumSignifiers.h HumSignifier.h HumdrumLine.h \
  HumdrumToken.h HumNum.h HumAddress.h \
  HumHash.h HumParamSet.h HumdrumFileStream.h \
  Convert.h  

tool-phrase.o: tool-phrase.cpp tool-phrase.h \
  HumTool.h Options.h HumdrumFileSet.h \
  HumdrumFile.h HumdrumFileContent.h \
  HumdrumFileStructure.h HumdrumFileBase.h \
  HumSignifiers.h HumSignifier.h HumdrumLine.h \
  HumdrumToken.h HumNum.h HumAddress.h \
  HumHash.h HumParamSet.h HumdrumFileStream.h \
  Convert.h HumRegex.h

tool-pnum.o: tool-pnum.cpp tool-pnum.h HumTool.h \
  Options.h HumdrumFileSet.h HumdrumFile.h \
  HumdrumFileContent.h HumdrumFileStructure.h \
  HumdrumFileBase.h HumSignifiers.h \
  HumSignifier.h HumdrumLine.h HumdrumToken.h \
  HumNum.h HumAddress.h HumHash.h \
  HumParamSet.h HumdrumFileStream.h Convert.h \
  HumRegex.h

tool-recip.o: tool-recip.cpp tool-recip.h HumTool.h \
  Options.h HumdrumFileSet.h HumdrumFile.h \
  HumdrumFileContent.h HumdrumFileStructure.h \
  HumdrumFileBase.h HumSignifiers.h \
  HumSignifier.h HumdrumLine.h HumdrumToken.h \
  HumNum.h HumAddress.h HumHash.h \
  HumParamSet.h HumdrumFileStream.h Convert.h \
  HumRegex.h

tool-restfill.o: tool-restfill.cpp tool-restfill.h \
  HumTool.h Options.h HumdrumFileSet.h \
  HumdrumFile.h HumdrumFileContent.h \
  HumdrumFileStructure.h HumdrumFileBase.h \
  HumSignifiers.h HumSignifier.h HumdrumLine.h \
  HumdrumToken.h HumNum.h HumAddress.h \
  HumHash.h HumParamSet.h HumdrumFileStream.h \
  Convert.h HumRegex.h

tool-rid.o: tool-rid.cpp tool-rid.h HumTool.h \
  Options.h HumdrumFileSet.h HumdrumFile.h \
  HumdrumFileContent.h HumdrumFileStructure.h \
  HumdrumFileBase.h HumSignifiers.h \
  HumSignifier.h HumdrumLine.h HumdrumToken.h \
  HumNum.h HumAddress.h HumHash.h \
  HumParamSet.h HumdrumFileStream.h Convert.h \
  HumRegex.h

tool-ruthfix.o: tool-ruthfix.cpp tool-ruthfix.h \
  HumTool.h Options.h HumdrumFileSet.h \
  HumdrumFile.h HumdrumFileContent.h \
  HumdrumFileStructure.h HumdrumFileBase.h \
  HumSignifiers.h HumSignifier.h HumdrumLine.h \
  HumdrumToken.h HumNum.h HumAddress.h \
  HumHash.h HumParamSet.h HumdrumFileStream.h \
  NoteGrid.h NoteCell.h Convert.h \
  HumRegex.h

tool-satb2gs.o: tool-satb2gs.cpp tool-satb2gs.h \
  HumTool.h Options.h HumdrumFileSet.h \
  HumdrumFile.h HumdrumFileContent.h \
  HumdrumFileStructure.h HumdrumFileBase.h \
  HumSignifiers.h HumSignifier.h HumdrumLine.h \
  HumdrumToken.h HumNum.h HumAddress.h \
  HumHash.h HumParamSet.h HumdrumFileStream.h \
  Convert.h HumRegex.h

tool-scordatura.o: tool-scordatura.cpp tool-scordatura.h \
  HumTool.h Options.h HumdrumFileSet.h \
  HumdrumFile.h HumdrumFileContent.h \
  HumdrumFileStructure.h HumdrumFileBase.h \
  HumSignifiers.h HumSignifier.h HumdrumLine.h \
  HumdrumToken.h HumNum.h HumAddress.h \
  HumHash.h HumParamSet.h HumdrumFileStream.h \
  HumTransposer.h HumPitch.h HumRegex.h \
  Convert.h

tool-semitones.o: tool-semitones.cpp tool-semitones.h \
  HumTool.h Options.h HumdrumFileSet.h \
  HumdrumFile.h HumdrumFileContent.h \
  HumdrumFileStructure.h HumdrumFileBase.h \
  HumSignifiers.h HumSignifier.h HumdrumLine.h \
  HumdrumToken.h HumNum.h HumAddress.h \
  HumHash.h HumParamSet.h HumdrumFileStream.h \
  Convert.h HumRegex.h

tool-shed.o: tool-shed.cpp tool-shed.h HumTool.h \
  Options.h HumdrumFileSet.h HumdrumFile.h \
  HumdrumFileContent.h HumdrumFileStructure.h \
  HumdrumFileBase.h HumSignifiers.h \
  HumSignifier.h HumdrumLine.h HumdrumToken.h \
  HumNum.h HumAddress.h HumHash.h \
  HumParamSet.h HumdrumFileStream.h Convert.h \
  HumRegex.h

tool-sic.o: tool-sic.cpp tool-sic.h HumTool.h \
  Options.h HumdrumFileSet.h HumdrumFile.h \
  HumdrumFileContent.h HumdrumFileStructure.h \
  HumdrumFileBase.h HumSignifiers.h \
  HumSignifier.h HumdrumLine.h HumdrumToken.h \
  HumNum.h HumAddress.h HumHash.h \
  HumParamSet.h HumdrumFileStream.h Convert.h \
  HumRegex.h

tool-simat.o: tool-simat.cpp tool-simat.h HumTool.h \
  Options.h HumdrumFileSet.h HumdrumFile.h \
  HumdrumFileContent.h HumdrumFileStructure.h \
  HumdrumFileBase.h HumSignifiers.h \
  HumSignifier.h HumdrumLine.h HumdrumToken.h \
  HumNum.h HumAddress.h HumHash.h \
  HumParamSet.h HumdrumFileStream.h Convert.h \
  HumRegex.h  

tool-slurcheck.o: tool-slurcheck.cpp tool-slurcheck.h \
  HumTool.h Options.h HumdrumFileSet.h \
  HumdrumFile.h HumdrumFileContent.h \
  HumdrumFileStructure.h HumdrumFileBase.h \
  HumSignifiers.h HumSignifier.h HumdrumLine.h \
  HumdrumToken.h HumNum.h HumAddress.h \
  HumHash.h HumParamSet.h HumdrumFileStream.h \
  Convert.h HumRegex.h

tool-spinetrace.o: tool-spinetrace.cpp tool-spinetrace.h \
  HumTool.h Options.h HumdrumFileSet.h \
  HumdrumFile.h HumdrumFileContent.h \
  HumdrumFileStructure.h HumdrumFileBase.h \
  HumSignifiers.h HumSignifier.h HumdrumLine.h \
  HumdrumToken.h HumNum.h HumAddress.h \
  HumHash.h HumParamSet.h HumdrumFileStream.h

tool-strophe.o: tool-strophe.cpp tool-strophe.h \
  HumTool.h Options.h HumdrumFileSet.h \
  HumdrumFile.h HumdrumFileContent.h \
  HumdrumFileStructure.h HumdrumFileBase.h \
  HumSignifiers.h HumSignifier.h HumdrumLine.h \
  HumdrumToken.h HumNum.h HumAddress.h \
  HumHash.h HumParamSet.h HumdrumFileStream.h \
  Convert.h HumRegex.h

tool-synco.o: tool-synco.cpp tool-synco.h HumTool.h \
  Options.h HumdrumFileSet.h HumdrumFile.h \
  HumdrumFileContent.h HumdrumFileStructure.h \
  HumdrumFileBase.h HumSignifiers.h \
  HumSignifier.h HumdrumLine.h HumdrumToken.h \
  HumNum.h HumAddress.h HumHash.h \
  HumParamSet.h HumdrumFileStream.h HumRegex.h

tool-tabber.o: tool-tabber.cpp tool-tabber.h \
  HumTool.h Options.h HumdrumFileSet.h \
  HumdrumFile.h HumdrumFileContent.h \
  HumdrumFileStructure.h HumdrumFileBase.h \
  HumSignifiers.h HumSignifier.h HumdrumLine.h \
  HumdrumToken.h HumNum.h HumAddress.h \
  HumHash.h HumParamSet.h HumdrumFileStream.h

tool-tassoize.o: tool-tassoize.cpp tool-tassoize.h \
  HumTool.h Options.h HumdrumFileSet.h \
  HumdrumFile.h HumdrumFileContent.h \
  HumdrumFileStructure.h HumdrumFileBase.h \
  HumSignifiers.h HumSignifier.h HumdrumLine.h \
  HumdrumToken.h HumNum.h HumAddress.h \
  HumHash.h HumParamSet.h HumdrumFileStream.h \
  Convert.h HumRegex.h tool-shed.h

tool-thru.o: tool-thru.cpp tool-thru.h HumTool.h \
  Options.h HumdrumFileSet.h HumdrumFile.h \
  HumdrumFileContent.h HumdrumFileStructure.h \
  HumdrumFileBase.h HumSignifiers.h \
  HumSignifier.h HumdrumLine.h HumdrumToken.h \
  HumNum.h HumAddress.h HumHash.h \
  HumParamSet.h HumdrumFileStream.h HumRegex.h

tool-tie.o: tool-tie.cpp tool-tie.h HumTool.h \
  Options.h HumdrumFileSet.h HumdrumFile.h \
  HumdrumFileContent.h HumdrumFileStructure.h \
  HumdrumFileBase.h HumSignifiers.h \
  HumSignifier.h HumdrumLine.h HumdrumToken.h \
  HumNum.h HumAddress.h HumHash.h \
  HumParamSet.h HumdrumFileStream.h Convert.h \
  HumRegex.h

tool-timebase.o: tool-timebase.cpp tool-timebase.h \
  HumTool.h Options.h HumdrumFileSet.h \
  HumdrumFile.h HumdrumFileContent.h \
  HumdrumFileStructure.h HumdrumFileBase.h \
  HumSignifiers.h HumSignifier.h HumdrumLine.h \
  HumdrumToken.h HumNum.h HumAddress.h \
  HumHash.h HumParamSet.h HumdrumFileStream.h \
  Convert.h

tool-tpos.o: tool-tpos.cpp tool-tpos.h HumTool.h \
  Options.h HumdrumFileSet.h HumdrumFile.h \
  HumdrumFileContent.h HumdrumFileStructure.h \
  HumdrumFileBase.h HumSignifiers.h \
  HumSignifier.h HumdrumLine.h HumdrumToken.h \
  HumNum.h HumAddress.h HumHash.h \
  HumParamSet.h HumdrumFileStream.h HumRegex.h

tool-transpose.o: tool-transpose.cpp tool-transpose.h \
  HumTool.h Options.h HumdrumFileSet.h \
  HumdrumFile.h HumdrumFileContent.h \
  HumdrumFileStructure.h HumdrumFileBase.h \
  HumSignifiers.h HumSignifier.h HumdrumLine.h \
  HumdrumToken.h HumNum.h HumAddress.h \
  HumHash.h HumParamSet.h HumdrumFileStream.h \
  Convert.h HumRegex.h

tool-tremolo.o: tool-tremolo.cpp tool-tremolo.h \
  HumTool.h Options.h HumdrumFileSet.h \
  HumdrumFile.h HumdrumFileContent.h \
  HumdrumFileStructure.h HumdrumFileBase.h \
  HumSignifiers.h HumSignifier.h HumdrumLine.h \
  HumdrumToken.h HumNum.h HumAddress.h \
  HumHash.h HumParamSet.h HumdrumFileStream.h \
  Convert.h HumRegex.h

tool-trillspell.o: tool-trillspell.cpp tool-trillspell.h \
  HumTool.h Options.h HumdrumFileSet.h \
  HumdrumFile.h HumdrumFileContent.h \
  HumdrumFileStructure.h HumdrumFileBase.h \
  HumSignifiers.h HumSignifier.h HumdrumLine.h \
  HumdrumToken.h HumNum.h HumAddress.h \
  HumHash.h HumParamSet.h HumdrumFileStream.h \
  Convert.h HumRegex.h

