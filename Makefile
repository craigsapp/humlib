##
## Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
## Creation Date: Sun Aug  9 22:20:14 PDT 2015
## Last Modified: Sat Jun 17 00:14:11 CEST 2017
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
TOOLDIR       = cli
SRCDIR_MIN    = src
INCDIR        = include
INCDIR_MIN    = include
LIBDIR        = lib
LIBFILE       = libhumdrum.a
LIBFILE_MIN   = libhumlib.a
#DYLIBFILE     = libhumlib.dylib
LIBFILE_PUGI  = libpugixml.a

DLIBFILE      = libhumdrum.a
DLIBFILE_MIN  = libhumlib.a
ifeq ($(OS),OSX)
   DLIBFILE_MIN  = humlib.dylib
else
   DLIBFILE_MIN  = humlib.so
endif

AR            = ar
RANLIB        = ranlib

PREFLAGS  = -c -g $(CFLAGS) $(DEFINES) -I$(INCDIR) 
PREFLAGS += -O3 -Wall

# using C++ 2011 standard in Humlib:
PREFLAGS += -std=c++11

# Add -static flag to compile without dynamics libraries for better portability:
POSTFLAGS =
# POSTFLAGS += -static

ifeq ($(CXX),)
   COMPILER      = LANG=C $(ENV) g++ $(ARCH)
else 
   COMPILER      = LANG=C $(ENV) $(CXX) $(ARCH)
endif
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
.PHONY: examples myprograms src include dynamic cli


###########################################################################


all: pugixml library tools

pugi:    pugixml
pugixml: makedirs pugixml.o
	@-rm -f $(LIBDIR)/$(LIBFILE_PUGI)
	@$(AR) r $(LIBDIR)/$(LIBFILE_PUGI) $(OBJDIR)/pugixml.o
	@$(RANLIB) $(LIBDIR)/$(LIBFILE_PUGI)


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
#	@$(COMPILER) -dynamiclib -o $(LIBDIR)/$(DYLIBFILE) $(OBJDIR)/humlib.o $(OBJDIR)/pugixml.o


lib: makedirs $(OBJS)
	@-rm -f $(LIBDIR)/$(LIBFILE)
	@$(AR) r $(LIBDIR)/$(LIBFILE) $(addprefix $(OBJDIR)/, $(OBJS))
	@$(RANLIB) $(LIBDIR)/$(LIBFILE)


cli: tools
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
	@echo 'if [ "$<" == "" ]; then $(MAKE) -f Makefile.programs $@; fi' | bash -s


pugixml.o: pugixml.cpp
	@echo [CC] $@
	@$(COMPILER) $(PREFLAGS) -o $(OBJDIR)/$(notdir $@) $(POSTFLAGS) $<

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
  HumHash.h HumRegex.h

GridMeasure.o: GridMeasure.cpp HumGrid.h \
  GridMeasure.h GridCommon.h HumdrumFile.h \
  HumdrumFileContent.h HumdrumFileStructure.h \
  HumdrumFileBase.h HumdrumLine.h HumdrumToken.h \
  HumNum.h HumAddress.h HumHash.h \
  GridSlice.h MxmlPart.h MxmlMeasure.h \
  GridPart.h \
  GridStaff.h GridSide.h GridVoice.h

GridPart.o: GridPart.cpp GridPart.h GridStaff.h \
  GridCommon.h GridSide.h HumdrumToken.h \
  HumNum.h HumAddress.h HumHash.h \
  GridVoice.h

GridSide.o: GridSide.cpp HumGrid.h GridMeasure.h \
  GridCommon.h HumdrumFile.h \
  HumdrumFileContent.h HumdrumFileStructure.h \
  HumdrumFileBase.h HumdrumLine.h HumdrumToken.h \
  HumNum.h HumAddress.h HumHash.h \
  GridSlice.h MxmlPart.h MxmlMeasure.h \
  GridPart.h \
  GridStaff.h GridSide.h GridVoice.h

GridSlice.o: GridSlice.cpp HumGrid.h GridMeasure.h \
  GridCommon.h HumdrumFile.h \
  HumdrumFileContent.h HumdrumFileStructure.h \
  HumdrumFileBase.h HumdrumLine.h HumdrumToken.h \
  HumNum.h HumAddress.h HumHash.h \
  GridSlice.h MxmlPart.h MxmlMeasure.h \
  GridPart.h \
  GridStaff.h GridSide.h GridVoice.h

GridStaff.o: GridStaff.cpp HumGrid.h GridMeasure.h \
  GridCommon.h HumdrumFile.h \
  HumdrumFileContent.h HumdrumFileStructure.h \
  HumdrumFileBase.h HumdrumLine.h HumdrumToken.h \
  HumNum.h HumAddress.h HumHash.h \
  GridSlice.h MxmlPart.h MxmlMeasure.h \
  GridPart.h \
  GridStaff.h GridSide.h GridVoice.h

GridVoice.o: GridVoice.cpp GridVoice.h HumdrumToken.h \
  HumNum.h HumAddress.h HumHash.h

HumAddress.o: HumAddress.cpp HumAddress.h \
  HumdrumLine.h HumdrumToken.h HumNum.h \
  HumHash.h

HumGrid.o: HumGrid.cpp HumGrid.h GridMeasure.h \
  GridCommon.h HumdrumFile.h \
  HumdrumFileContent.h HumdrumFileStructure.h \
  HumdrumFileBase.h HumdrumLine.h HumdrumToken.h \
  HumNum.h HumAddress.h HumHash.h \
  GridSlice.h MxmlPart.h MxmlMeasure.h \
  GridPart.h \
  GridStaff.h GridSide.h GridVoice.h \
  Convert.h

HumHash.o: HumHash.cpp HumHash.h HumNum.h \
  Convert.h HumdrumToken.h HumAddress.h

HumInstrument.o: HumInstrument.cpp HumInstrument.h

HumNum.o: HumNum.cpp HumNum.h

HumRegex.o: HumRegex.cpp HumRegex.h

HumTool.o: HumTool.cpp HumTool.h Options.h

HumdrumFile.o: HumdrumFile.cpp HumdrumFile.h \
  HumdrumFileContent.h HumdrumFileStructure.h \
  HumdrumFileBase.h HumdrumLine.h HumdrumToken.h \
  HumNum.h HumAddress.h HumHash.h \
  Convert.h

HumdrumFileBase-net.o: HumdrumFileBase-net.cpp \
  HumdrumFileBase.h HumdrumLine.h HumdrumToken.h \
  HumNum.h HumAddress.h HumHash.h \
  Convert.h

HumdrumFileBase.o: HumdrumFileBase.cpp HumdrumFileBase.h \
  HumdrumLine.h HumdrumToken.h HumNum.h \
  HumAddress.h HumHash.h Convert.h \
  HumRegex.h

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

HumdrumFileStream.o: HumdrumFileStream.cpp \
  HumdrumFileStream.h HumdrumFile.h \
  HumdrumFileContent.h HumdrumFileStructure.h \
  HumdrumFileBase.h HumdrumLine.h HumdrumToken.h \
  HumNum.h HumAddress.h HumHash.h \
  Options.h HumRegex.h

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

MxmlEvent.o: MxmlEvent.cpp MxmlEvent.h GridCommon.h \
  HumNum.h MxmlMeasure.h

MxmlMeasure.o: MxmlMeasure.cpp MxmlEvent.h \
  GridCommon.h HumNum.h \
  MxmlMeasure.h MxmlPart.h

MxmlPart.o: MxmlPart.cpp MxmlMeasure.h GridCommon.h \
  HumNum.h MxmlPart.h

NoteCell.o: NoteCell.cpp NoteCell.h HumdrumFile.h \
  HumdrumFileContent.h HumdrumFileStructure.h \
  HumdrumFileBase.h HumdrumLine.h HumdrumToken.h \
  HumNum.h HumAddress.h HumHash.h \
  NoteGrid.h Convert.h

NoteGrid.o: NoteGrid.cpp NoteGrid.h NoteCell.h \
  HumdrumFile.h HumdrumFileContent.h \
  HumdrumFileStructure.h HumdrumFileBase.h \
  HumdrumLine.h HumdrumToken.h HumNum.h \
  HumAddress.h HumHash.h HumRegex.h

Options.o: Options.cpp Options.h

humlib.o: humlib.cpp humlib.h

pugixml.o: pugixml.cpp 

tool-autobeam.o: tool-autobeam.cpp tool-autobeam.h \
  HumTool.h Options.h HumdrumFile.h \
  HumdrumFileContent.h HumdrumFileStructure.h \
  HumdrumFileBase.h HumdrumLine.h HumdrumToken.h \
  HumNum.h HumAddress.h HumHash.h

tool-autostem.o: tool-autostem.cpp tool-autostem.h \
  HumTool.h Options.h HumdrumFile.h \
  HumdrumFileContent.h HumdrumFileStructure.h \
  HumdrumFileBase.h HumdrumLine.h HumdrumToken.h \
  HumNum.h HumAddress.h HumHash.h \
  HumRegex.h Convert.h

tool-cint.o: tool-cint.cpp tool-cint.h HumTool.h \
  Options.h HumdrumFile.h HumdrumFileContent.h \
  HumdrumFileStructure.h HumdrumFileBase.h \
  HumdrumLine.h HumdrumToken.h HumNum.h \
  HumAddress.h HumHash.h NoteGrid.h \
  NoteCell.h HumRegex.h Convert.h

tool-dissonant.o: tool-dissonant.cpp tool-dissonant.h \
  HumTool.h Options.h HumdrumFile.h \
  HumdrumFileContent.h HumdrumFileStructure.h \
  HumdrumFileBase.h HumdrumLine.h HumdrumToken.h \
  HumNum.h HumAddress.h HumHash.h \
  NoteGrid.h NoteCell.h Convert.h \
  HumRegex.h

tool-esac2hum.o: tool-esac2hum.cpp tool-esac2hum.h \
  HumTool.h Options.h HumdrumFile.h \
  HumdrumFileContent.h HumdrumFileStructure.h \
  HumdrumFileBase.h HumdrumLine.h HumdrumToken.h \
  HumNum.h HumAddress.h HumHash.h \
  Convert.h

tool-extract.o: tool-extract.cpp tool-extract.h \
  HumTool.h Options.h HumdrumFile.h \
  HumdrumFileContent.h HumdrumFileStructure.h \
  HumdrumFileBase.h HumdrumLine.h HumdrumToken.h \
  HumNum.h HumAddress.h HumHash.h \
  HumRegex.h

tool-filter.o: tool-filter.cpp tool-filter.h \
  HumTool.h Options.h HumdrumFile.h \
  HumdrumFileContent.h HumdrumFileStructure.h \
  HumdrumFileBase.h HumdrumLine.h HumdrumToken.h \
  HumNum.h HumAddress.h HumHash.h \
  tool-autobeam.h tool-autostem.h tool-cint.h \
  NoteGrid.h NoteCell.h HumRegex.h \
  tool-dissonant.h tool-extract.h tool-recip.h \
  tool-satb2gs.h tool-metlev.h tool-myank.h \
  tool-transpose.h

tool-metlev.o: tool-metlev.cpp tool-metlev.h \
  HumTool.h Options.h HumdrumFile.h \
  HumdrumFileContent.h HumdrumFileStructure.h \
  HumdrumFileBase.h HumdrumLine.h HumdrumToken.h \
  HumNum.h HumAddress.h HumHash.h \
  Convert.h

tool-musicxml2hum.o: tool-musicxml2hum.cpp \
  tool-musicxml2hum.h Options.h HumTool.h \
  MxmlPart.h MxmlMeasure.h GridCommon.h HumNum.h \
  MxmlEvent.h HumGrid.h GridMeasure.h \
  HumdrumFile.h HumdrumFileContent.h \
  HumdrumFileStructure.h HumdrumFileBase.h \
  HumdrumLine.h HumdrumToken.h HumAddress.h \
  HumHash.h GridSlice.h GridPart.h \
  GridStaff.h GridSide.h GridVoice.h

tool-myank.o: tool-myank.cpp tool-myank.h HumTool.h \
  Options.h HumdrumFile.h HumdrumFileContent.h \
  HumdrumFileStructure.h HumdrumFileBase.h \
  HumdrumLine.h HumdrumToken.h HumNum.h \
  HumAddress.h HumHash.h HumRegex.h

tool-recip.o: tool-recip.cpp tool-recip.h HumTool.h \
  Options.h HumdrumFile.h HumdrumFileContent.h \
  HumdrumFileStructure.h HumdrumFileBase.h \
  HumdrumLine.h HumdrumToken.h HumNum.h \
  HumAddress.h HumHash.h Convert.h \
  HumRegex.h

tool-satb2gs.o: tool-satb2gs.cpp tool-satb2gs.h \
  HumTool.h Options.h HumdrumFile.h \
  HumdrumFileContent.h HumdrumFileStructure.h \
  HumdrumFileBase.h HumdrumLine.h HumdrumToken.h \
  HumNum.h HumAddress.h HumHash.h \
  HumRegex.h Convert.h

tool-transpose.o: tool-transpose.cpp tool-transpose.h \
  HumTool.h Options.h HumdrumFile.h \
  HumdrumFileContent.h HumdrumFileStructure.h \
  HumdrumFileBase.h HumdrumLine.h HumdrumToken.h \
  HumNum.h HumAddress.h HumHash.h \
  Convert.h HumRegex.h

