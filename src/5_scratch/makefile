
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
CFLAGS="-std=c11 -fPIC -I. -ggdb -DDEBUG"
LIB="libsubu.a"
LIBPATH="." #no trailing slash
LINKFLAGS="-L. -lsubu"

SOURCES_LIB= $(wildcard *.lib.c)
SOURCES_CLI= $(wildcard *.cli.c)
SOURCES= $(SOURCES_LIB) $(SOURCES_CLI)

OBJECTS_LIB= $(patsubst %.c, %.o, $(SOURCES_LIB))
OBJECTS_CLI= $(patsubst %.c, %.o, $(SOURCES_CLI))
OBJECTS= $(OBJECTS_LIB) $(OBJECTS_CLI)

# sort causes compiles to go in lexical order by file name, this is used to order the tests e.g.
EXECS= $(sort $(patsubst %.cli.c,  %, $(wildcard *.cli.c)))

version:
	@echo '---- make $@:------------------------------------------------------------'
	@echo `pwd`'>'
	@echo makefile version 2.0
	@echo "CC: " $(CC)
	@echo "CFLAGS: " $(CFLAGS)
	@echo "LIB: " $(LIB)
	@echo "LIBPATH: " $(LIBPATH)
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
	$(CC) $(CFLAGS) -MM $(SOURCES) 1> 2_makefile_deps
	for i in $(EXECS) ; do\
	   echo >> 2_makefile_deps;\
	   $(ECHO) "$$i : $$i.cli.o" >> 2_makefile_deps;\
	   $(ECHO) "	$(CC) -o $$i $$i.cli.o" >> 2_makefile_deps;\
	done
	@echo '______end make $@_____'

lib: clear_test_flags
	@echo '---- make $@:------------------------------------------------------------'
	@echo `pwd`'>' 
	if [ ! -e 2_makefile_deps ]; then make deps; fi # the user must delete 2_makefile_deps if deps change!
	make sub_lib
	@echo '______end make $@_____'

sub_lib: $(LIB)


$(LIBPATH)/$(LIB):
	@if [ "$(LIBPATH)" != "$(APPLICATION_BASE)" ]; then \
	  echo "LIBPATH: " "$(LIBPATH)";\
	  echo "$APPLICATION_BASE: " "$(APPLICATION_BASE)";\
	  echo "The library path is not the same as the application base so don't know how to make it.";\
        fi
	[ "$(LIBPATH)" == "$(APPLICATION_BASE)" ]
	cd $(APPLICATION_BASE); make lib

execs: $(LIBPATH)/$(LIB)
	@echo '---- make $@:------------------------------------------------------------'
	@echo `pwd`'>' 
	if [ ! -e 2_makefile_deps ]; then make deps; fi
	make sub_execs
	# for i in $(EXEC_OBJECTS); do rm $$i; done
	@echo '______end make $@_____'

sub_execs: $(EXECS)

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

# "make lib" "make execs" now cleans up the .o files afterward, so clean should not be needed often
# clean works on the current working directory, dist_clean also  descends into 1_tests and 1_try
#
clean:
	@echo '---- make $@:------------------------------------------------------------'
	@echo `pwd`'>' 
	for i in $(wildcard *~); do mv $$i $(SCRATCHDIR); done
	for i in $(wildcard *.lib.o) $(wildcard *.exl.o) $(wildcard *.ex.o); do rm $$i; done 
	if [ -f 2_makefile_deps ]; then rm 2_makefile_deps; fi
	if [ -f $(LIB) ]; then mv $(LIB) 5_scratch; fi
	for i in $(EXEC_WL) $(EXEC); do if [ -e $$i ]; then rm $$i; fi; done 
	@echo '______end make $@_____'


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
%.o : %.cc
	$(CC) $(CFLAGS) -c $<


#
$(LIB) : $(LIB_OBJECTS)
	ar rcs $(LIB) $(LIB_OBJECTS)
#	for i in $(LIB_OBJECTS); do rm $$i; done
 


