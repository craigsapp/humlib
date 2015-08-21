##
## Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
## Creation Date: Sun Aug  9 22:20:14 PDT 2015
## Last Modified: Sun Aug  9 22:20:16 PDT 2015
## Syntax:        GNU Makefile
## Filename:      /Makefile
## vim:           ts=3
##
## Description: Makefile to run tasks for minHumdrum library.
##


# Set the environmental variable $MACOSX_DEPLOYMENT_TARGET to
# "10.9" in Apple OS X to compile for OS X 10.9 and later (for example,
# you can compile for OS X 10.9 computers even if you are using the 10.10
# version of the operating system).
ENV =

ifeq ($(shell uname),Darwin)
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
SRCDIR_MIN    = src
INCDIR        = include
INCDIR_MIN    = include
LIBDIR        = lib
LIBFILE       = libhumdrum.a
LIBFILE_MIN   = libminhumdrum.a
AR            = ar
RANLIB        = ranlib

PREFLAGS  = -c -g -Wall $(DEFINES) -I$(INCDIR) -I$(INCDIR_MIN)
PREFLAGS += -O3

# using C++ 2011 standard:
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
OBJS = $(notdir $(patsubst %.cpp,%.o,$(wildcard $(SRCDIR)/[A-Z]*.cpp)))

# targets which don't actually refer to files
.PHONY: examples transfer myprograms src include t trans r


###########################################################################
#                                                                         #
# Hardware Configurations:                                                #
#                                                                         #

all: minlibrary


minlib: minlibrary
minlibrary: makedirs min minhumdrum.o
	@echo "Creating minhumdrum library file for OS X..."
	@-rm -f $(LIBDIR)/$(LIBFILE_MIN)
	@$(AR) r $(LIBDIR)/$(LIBFILE_MIN) $(OBJDIR)/minhumdrum.o
	@$(RANLIB) $(LIBDIR)/$(LIBFILE_MIN)


lib: library
library: makedirs $(OBJS)
	@echo "Creating humdrum library file for OS X..."
	@-rm -f $(LIBDIR)/$(LIBFILE)
	@$(AR) r $(LIBDIR)/$(LIBFILE) $(OBJDIR)/*.o
	@$(RANLIB) $(LIBDIR)/$(LIBFILE)


both: library minlibrary


min:
	bin/makeminhumdrum


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


t:        transfer
trans:    transfer
transfer:
	rm -rf transfer
	mkdir -p transfer
	cp src/[A-Z]*.cpp transfer
	cp include/[A-Z]*.h transfer

r:      return
return:
	-cp transfer/[A-Z]*.cpp src
	-cp transfer/[A-Z]*.h include
	rm -rf transfer

%:
	@echo compiling example $@
	$(MAKE) -f Makefile.examples $@


###########################################################################
#                                                                         #
# defining an explicit rule for object file dependencies                  #
#                                                                         #

%.o : %.cpp min
	@echo [CC] $@
	$(COMPILER) $(PREFLAGS) -o $(OBJDIR)/$(notdir $@) $(POSTFLAGS) $<

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

Convert.o: Convert.cpp

HumNum.o: HumNum.cpp

HumdrumAddress.o: HumdrumAddress.cpp

HumdrumFile.o: HumdrumFile.cpp

HumdrumLine.o: HumdrumLine.cpp

HumdrumToken.o: HumdrumToken.cpp



