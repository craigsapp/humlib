##
## Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
## Creation Date: Sun Aug  9 22:20:14 PDT 2015
## Last Modified: Sat Aug  6 13:03:46 CEST 2016
## Syntax:        GNU Makefile
## Filename:      humlib/Makefile
## vim:           ts=3
##
## Description: Makefile to run tasks for humlib library.
##
## To compile the humlib library with the Options class, run:
##     make CFLAGS=-D_INCLUDE_HUMLIB_OPTIONS_
## Otherwise to compile:
##     make
## To create the minimal file version:
##     make min
##

# Set the environmental variable $MACOSX_DEPLOYMENT_TARGET to
# "10.9" in Apple OS X to compile for OS X 10.9 and later (for example,
# you can compile for OS X 10.9 computers even if you are using the 10.10
# version of the operating system).
ENV =

OS := $(shell uname -s)

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

BINDIR        = bin
OBJDIR        = obj
SRCDIR        = src
TOOLDIR       = tools
SRCDIR_MIN    = src
INCDIR        = include
INCDIR_MIN    = include
LIBDIR        = lib
LIBFILE       = libhumdrum.a
LIBFILE_MIN   = libhumlib.a

DLIBFILE      = libhumdrum.a
DLIBFILE_MIN  = libhumlib.a
ifeq ($(OS),OSX)
   DLIBFILE_MIN  = humlib.dylib
else
   DLIBFILE_MIN  = humlib.so
endif

AR            = ar
RANLIB        = ranlib

PREFLAGS  = -c -g $(CFLAGS) $(DEFINES) -I$(INCDIR) -I$(INCDIR_MIN)
PREFLAGS += -O3 -Wall

# using C++ 2011 standard in Humlib:
PREFLAGS += -std=c++11

# Add -static flag to compile without dynamics libraries for better portability:
POSTFLAGS =
# POSTFLAGS += -static

COMPILER      = LANG=C $(ENV) g++ $(ARCH)
# Or use clang++ v3.3:
#COMPILER      = clang++
#PREFLAGS     += -stdlib=libc++

#                                                                         #
# End of user-modifiable variables.                                       #
#                                                                         #
###########################################################################


# setting up the directory paths to search for dependency files
vpath %.h   $(INCDIR):$(SRCDIR)
vpath %.cpp $(SRCDIR):$(INCDIR)
vpath %.o   $(OBJDIR)

# generating a list of the object files
OBJS  =
OBJS += $(notdir $(patsubst %.cpp,%.o,$(wildcard $(SRCDIR)/tool-*.cpp)))
OBJS += $(notdir $(patsubst %.cpp,%.o,$(wildcard $(SRCDIR)/[A-Z]*.cpp)))

# targets which don't actually refer to files
.PHONY: examples myprograms src include dynamic tools


###########################################################################


all: library tools

dynamic: min
ifeq ($(OS),OSX)
	g++ -dynamiclib -o $(LIBDIR)/$(DLIBFILE_MIN) $(OBJDIR)/humlib.o
else
	g++ -shared -fPIC -o $(LIBDIR)/$(DLIBFILE_MIN) $(OBJDIR)/humlib.o
endif


library: minlibrary
minlib: minlibrary
minlibrary: makedirs min humlib.o
	@-rm -f $(LIBDIR)/$(LIBFILE_MIN)
	@$(AR) r $(LIBDIR)/$(LIBFILE_MIN) $(OBJDIR)/humlib.o
	@$(RANLIB) $(LIBDIR)/$(LIBFILE_MIN)


lib: makedirs $(OBJS)
	@-rm -f $(LIBDIR)/$(LIBFILE)
	@$(AR) r $(LIBDIR)/$(LIBFILE) $(OBJDIR)/*.o
	@$(RANLIB) $(LIBDIR)/$(LIBFILE)


tool: tools
tools:
	@$(MAKE) -f Makefile.programs


min:
	bin/makehumlib


clean:
	@echo Erasing object files...
	@-rm -f $(OBJDIR)/*.o
	@echo Erasing obj directory...
	@-rmdir $(OBJDIR)


superclean: clean
	-rm -rf $(LIBDIR)
	-rm -f  $(BINDIR)/test*


makedirs:
	@-mkdir -p $(OBJDIR)
	@-mkdir -p $(LIBDIR)

%:
	@if [ "$<" == "" ]; then $(MAKE) -f Makefile.programs $@; fi

###########################################################################
#                                                                         #
# defining an explicit rule for object file dependencies                  #
#                                                                         #

%.o : %.cpp min
	@echo [CC] $@
	@$(COMPILER) $(PREFLAGS) -o $(OBJDIR)/$(notdir $@) $(POSTFLAGS) $<

#                                                                         #
###########################################################################



###########################################################################
#                                                                         #
# Dependencies -- generated with the following command in                 #
#      the src directory (in bash shell):                                 #
#                                                                         #
# for i in src/*.cpp
# do
#    cc -std=c++11 -Iinclude -MM $i | sed 's/include\///g; s/src\///g'
#    echo ""
# done
#

Convert-kern.o: Convert-kern.cpp Convert.h HumNum.h \
  HumdrumToken.h HumAddress.h HumHash.h

Convert-math.o: Convert-math.cpp Convert.h HumNum.h \
  HumdrumToken.h HumAddress.h HumHash.h

Convert-pitch.o: Convert-pitch.cpp Convert.h HumNum.h \
  HumdrumToken.h HumAddress.h HumHash.h

Convert-rhythm.o: Convert-rhythm.cpp Convert.h \
  HumNum.h HumdrumToken.h HumAddress.h \
  HumHash.h

Convert-string.o: Convert-string.cpp Convert.h \
  HumNum.h HumdrumToken.h HumAddress.h \
  HumHash.h

HumAddress.o: HumAddress.cpp HumAddress.h \
  HumdrumLine.h HumdrumToken.h HumNum.h \
  HumHash.h

HumHash.o: HumHash.cpp HumHash.h HumNum.h \
  Convert.h HumdrumToken.h HumAddress.h

HumInstrument.o: HumInstrument.cpp HumInstrument.h

HumNum.o: HumNum.cpp HumNum.h

HumTool.o: HumTool.cpp HumTool.h Options.h

HumdrumFile.o: HumdrumFile.cpp HumdrumFile.h \
  HumdrumFileContent.h HumdrumFileStructure.h \
  HumdrumFileBase.h HumdrumLine.h HumdrumToken.h \
  HumNum.h HumAddress.h HumHash.h \
  Convert.h

HumdrumFileBase.o: HumdrumFileBase.cpp HumdrumFileBase.h \
  HumdrumLine.h HumdrumToken.h HumNum.h \
  HumAddress.h HumHash.h

HumdrumFileContent-accidental.o: HumdrumFileContent-accidental.cpp \
  HumdrumFileContent.h HumdrumFileStructure.h \
  HumdrumFileBase.h HumdrumLine.h HumdrumToken.h \
  HumNum.h HumAddress.h HumHash.h \
  Convert.h

HumdrumFileContent-metlev.o: HumdrumFileContent-metlev.cpp \
  HumdrumFileContent.h HumdrumFileStructure.h \
  HumdrumFileBase.h HumdrumLine.h HumdrumToken.h \
  HumNum.h HumAddress.h HumHash.h \
  Convert.h

HumdrumFileContent-slur.o: HumdrumFileContent-slur.cpp \
  HumdrumFileContent.h HumdrumFileStructure.h \
  HumdrumFileBase.h HumdrumLine.h HumdrumToken.h \
  HumNum.h HumAddress.h HumHash.h

HumdrumFileContent-tie.o: HumdrumFileContent-tie.cpp \
  HumdrumFileContent.h HumdrumFileStructure.h \
  HumdrumFileBase.h HumdrumLine.h HumdrumToken.h \
  HumNum.h HumAddress.h HumHash.h

HumdrumFileContent-timesig.o: HumdrumFileContent-timesig.cpp \
  HumdrumFileContent.h HumdrumFileStructure.h \
  HumdrumFileBase.h HumdrumLine.h HumdrumToken.h \
  HumNum.h HumAddress.h HumHash.h \
  Convert.h

HumdrumFileContent.o: HumdrumFileContent.cpp \
  HumdrumFileContent.h HumdrumFileStructure.h \
  HumdrumFileBase.h HumdrumLine.h HumdrumToken.h \
  HumNum.h HumAddress.h HumHash.h

HumdrumFileStructure.o: HumdrumFileStructure.cpp \
  HumdrumFileStructure.h HumdrumFileBase.h \
  HumdrumLine.h HumdrumToken.h HumNum.h \
  HumAddress.h HumHash.h Convert.h

HumdrumLine.o: HumdrumLine.cpp HumdrumLine.h \
  HumdrumToken.h HumNum.h HumAddress.h \
  HumHash.h HumdrumFile.h HumdrumFileContent.h \
  HumdrumFileStructure.h HumdrumFileBase.h \
  Convert.h

HumdrumToken.o: HumdrumToken.cpp HumAddress.h \
  HumdrumToken.h HumNum.h HumHash.h \
  HumdrumLine.h HumdrumFile.h \
  HumdrumFileContent.h HumdrumFileStructure.h \
  HumdrumFileBase.h Convert.h

NoteCell.o: NoteCell.cpp NoteCell.h HumdrumFile.h \
  HumdrumFileContent.h HumdrumFileStructure.h \
  HumdrumFileBase.h HumdrumLine.h HumdrumToken.h \
  HumNum.h HumAddress.h HumHash.h \
  NoteGrid.h Convert.h

NoteGrid.o: NoteGrid.cpp NoteGrid.h NoteCell.h \
  HumdrumFile.h HumdrumFileContent.h \
  HumdrumFileStructure.h HumdrumFileBase.h \
  HumdrumLine.h HumdrumToken.h HumNum.h \
  HumAddress.h HumHash.h

Options.o: Options.cpp Options.h

humlib.o: humlib.cpp humlib.h

tool-metlev.o: tool-metlev.cpp tool-metlev.h \
  HumTool.h Options.h HumdrumFile.h \
  HumdrumFileContent.h HumdrumFileStructure.h \
  HumdrumFileBase.h HumdrumLine.h HumdrumToken.h \
  HumNum.h HumAddress.h HumHash.h \
  Convert.h

