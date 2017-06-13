## humlib external makefile
##
## Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
## Creation Date: Sat Aug  6 10:00:27 CEST 2016
## Last Modified: Tue Jun 13 13:03:00 CEST 2017
## Filename:      ...humlib/external/Makefile
##
## Description: This Makefile compiles external libraries used in
##              humlib source code.  In particular:
##              (1) pugixml for parsing XML files (musicxml2hum)
##
## To run this makefile, type (without quotes) "make", or "gmake" on
## FreeBSD computers.  This makefile is typically run automatically from the
## parent directory's Makefile.
##

.PHONY: pugixml

all: pugixml

pugixml:
ifeq ($(wildcard pugixml),)
	wget https://github.com/zeux/pugixml/archive/master.zip -O pugixml.zip
	unzip pugixml.zip
	mv pugixml-master pugixml
endif
	(cd pugixml && cmake . && make)


clean:
	-rm -f pugixml.zip
	-rm -rf pugixml



