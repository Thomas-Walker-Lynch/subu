# Copyright 2011 (C) Reasoning Technology Ltd. All Rights Reserved
#
# 2010 11 20 TWL Created
# 2019 02 24 TWL modified for subu project and placed under MIT license

# a single space literal, for example if you wanted to subsitute commas to
# spaces: $(subst $(space),;,$(string)) we ran into this out of a need to send
# multiple separate command arguments to a shell script from one variable value
blank :=
space :=$(blank)  $(blank)

# some versions of Linux need a -e option others complain if there is a -e .. and it isn't the binary for echo ..
ECHO= echo
#ECHO= echo -e

SHELL=/bin/bash
SCRATCHDIR= 5_scratch # clean and others put things here
CC=gcc
CFLAGS=-std=c11 -fPIC -I. -ggdb -DDEBUG -Werror
LIB="libsubu.a"
LINKFLAGS=-L. -lsubu  -lsqlite3

#these are the source files that exist
SOURCES_LIB= $(wildcard *.lib.c)
SOURCES_CLI= $(wildcard *.cli.c)
SOURCES= $(SOURCES_LIB) $(SOURCES_CLI)

#these are the object files to be made
OBJECTS_LIB= $(patsubst %.c, %.o, $(SOURCES_LIB))
OBJECTS_CLI= $(patsubst %.c, %.o, $(SOURCES_CLI))
OBJECTS= $(OBJECTS_LIB) $(OBJECTS_CLI)

#these are the header files that exist, makeheaders will want to see them
HFILES = $(wildcard *.lib.h) $(wildcard *.cli.h)

# sort causes compiles to go in lexical order by file name, this is used to order the tests e.g.
EXECS= $(sort $(patsubst %.cli.c,  %, $(wildcard *.cli.c)))

all: version deps lib execs

version:
	@echo '---- make $@:------------------------------------------------------------'
	@echo `pwd`'>'
	@echo makefile version 2.0
	@echo "CC: " $(CC)
	@echo "CFLAGS: " $(CFLAGS)
	@echo "LIB: " $(LIB)
	@echo "LINKFLAGS: " $(LINKFLAGS)
	@echo '______end make $@_____'

# safe to run this in an already setup or partially setup directory
setup:
	@echo '---- make $@:------------------------------------------------------------'
	@echo `pwd`'>' 
	if [ ! -e $(SCRATCHDIR) ]; then mkdir $(SCRATCHDIR); fi
	if [ ! -e 1_tests ]; then mkdir 1_tests; fi
	if [ ! -e 1_try ]; then mkdir 1_try; fi
	if [ ! -e 2_bin ]; then mkdir 2_bin; fi
	if [ ! -e 3_documents ]; then mkdir 3_documents; fi
	if [ ! -e 3_to_do.txt ]; then touch 3_to_do.txt; fi
	if [ ! -e 5_deprecated ]; then mkdir 5_deprecated; fi
	@echo '______end make $@_____'


deps:
	@echo '---- make $@:------------------------------------------------------------'
	@echo `pwd`'>' 
	makeheaders $(SOURCES) $(HFILES)
	$(CC) $(CFLAGS) -MM $(SOURCES) 1> 2_makefile_deps
	for i in $(EXECS) ; do\
           $(ECHO) >> 2_makefile_deps;\
           $(ECHO) "$$i : $$i.cli.o $(LIB)" >> 2_makefile_deps;\
           $(ECHO) "	$(CXX) -o $$i $$i.cli.o $(LINKFLAGS)" >> 2_makefile_deps;\
        done
	@echo '______end make $@_____'

lib: 
	@echo '---- make $@:------------------------------------------------------------'
	@echo `pwd`'>' 
	if [ ! -e 2_makefile_deps ]; then make deps; fi # the user must delete 2_makefile_deps if deps change!
	make sub_lib
	@echo '______end make $@_____'

sub_lib: $(LIB)


execs: $(LIB)
	@echo '---- make $@:------------------------------------------------------------'
	@echo `pwd`'>' 
	if [ ! -e 2_makefile_deps ]; then make deps; fi
	make sub_execs
	@echo '______end make $@_____'

sub_execs: $(EXECS)

#not ready yet
install: all
	@echo '---- make $@:------------------------------------------------------------'
	@echo `pwd`'>' 
	@if[ ! -e 1_tests_passed ]; then echo "can't install as tests have not passed"; fi
	@test -e test_passed
	for i in $(BIN); do cp $$i $(RT_BASE)/bin; done
	cp $(LIB) $(RT_BASE)/lib
	cp $(APPLICATION).h  $(RT_BASE)/include
	if [ -d $(APPLICATION) ]; then cp $(APPLICATION)/*.h $(RT_BASE)/include/$(APPLICATION); fi
	@echo '______end make $@_____'

clean:
	@echo '---- make $@:------------------------------------------------------------'
	@echo `pwd`'>' 
	if [ -f subu.db ]; then rm subu.db; fi
	for i in $(wildcard *~); do mv $$i $(SCRATCHDIR); done
	for i in $(wildcard *.lib.o) $(wildcard *.cli.o); do rm $$i; done 
	for i in $(HFILES); do mv $$i 5_scratch; done # just in case someone wrote a header file
	if [ -f 2_makefile_deps ]; then rm 2_makefile_deps; fi
	if [ -f $(LIB) ]; then rm $(LIB); fi
	for i in $(EXECS); do if [ -e $$i ]; then rm $$i; fi; done 
	@echo '______end make $@_____'


# not ready ...
# dist_clean is used to clean thing up before doing a checkin,  hg add should be safe after a dist_clean
# dist_clean will recurse into the include directory = $(APPLICATION), tests, and try if they are present 
# 
dist_clean:
	@echo '---- make $@:------------------------------------------------------------'
	@echo `pwd`'>' 
	make clean
	if [ -d $(APPLICATION) ]; then cd $(APPLICATION); make clean; fi
	if [ -d 1_tests ]; then cd 1_tests; make clean; fi
	if [ -d 1_try ] ; then cd 1_try; make clean; fi
	@echo '______end make $@_____'


-include 2_makefile_deps

# recipe for making object files:
#
%.o : %.c
	$(CC) $(CFLAGS) -c $<


#
$(LIB) : $(OBJECTS_LIB)
	ar rcs $(LIB) $(OBJECTS_LIB)
