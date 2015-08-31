#
# Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
# Creation Date: Sun Feb  8 23:56:35 PST 2015
# Last Modified: Sun Feb  8 23:56:37 PST 2015
# Filename:      scorelib/class/Makefile
# Syntax:        GNU Makefile
#
# Description:   Adds source-code lines for functions in ATON files for
#                class documentation.
#

all: aton

SRC = Convert HumAddress HumHash HumNum HumdrumFile HumdrumFileBase HumdrumFileContent HumdrumFileStructure HumdrumLine HumdrumToken

lines:
	@for i in */Makefile;						\
	do								\
		DIRNAME=`echo $$i | sed 's/.Makefile//'`;		\
		(cd $$DIRNAME; $(MAKE) lines);				\
	done


makefiles:
	-for i in *; \
	do          \
		(cd $$i && ln -s ../Makefile.class Makefile); \
	done

aton: pull
	for i in $(SRC);	\
	do			\
		(cd $$i ^&& ../bin/code2aton $$i);	\
	done

pull:
	(cd ../../common/minHumdrum && git pull)



