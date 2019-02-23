# Copyright 2011 (C) Reasoning Technology Ltd. All Rights Reserved
#
# 2010 11 20 TWL Created
# 2011 05 26 TWL Modified to generalize
# 2012 02 23 NLS Add ECHO variable to use on different environnement
#                corrected setup macro --> add include directory in the path to copy
#                corrected install macro --> change the name of installed library : lib$(LIB)$(LIBSUFFIX)
#                changed DOC_DIR directory name to 5_documents
# 2012 02 23 TWL removed LIB variable which is now set from the command line so
#                so that all source dirs can use the same makefile
# 2012 02 23 TWL added target make dist_clean which also deletes the 2_makefile_deps file
# 2012 04 11 AWW added creation of temporary disk before each test is run 
# 2012 06 05 TWL moved tests and try .cc files to directories. caused rtmake tests to
#                dist_clean and make deps
#
#
#----------------------------------------------------------------------------
#      use this makefile to compile and test the code:
#
#               for a first time run, or for regression use the following:
#
#                   $ make setup  # makes the directories, though should already exist
#                   $ make regress 
#
#               the usual development workflow makes use of these:
#
#                   $ make deps              # only when needed, for example if headers includes change or new files introduced
#                   $ cd tests; make deps    # only when needed
#                   $ make lib               # this makes the local library
#                   $ make tests             # this updates tests and compiles
#                   $ make clean             # deletes the .o files and library to force a recompile
#                   $ cd 1_tests; make clean
#
#               for a release of a component
# 
#                   $ make regress
#                   $ make install  # this will only work if all the tests in 1_tests are passing 
#
#               before a checkin
# 
#                   $ make dist_clean # will also clean the tests and try directories
#
#               .lib.cc c++ files taken as source of object files for local build library
#               .exl.cc c++ files taken to have main calls and are linked against local build libary
#               .ex.cc  c++ files taken to have main calls and are not linked against the local build library
#               there are no rules for other files in this makefile
#
#      about dependencies
#               The makefile has no way of knowing if an edit changed the dependencies.  Often they do not
#               and it would be unwieldy to make the deps every time.  Hence *the programmer* must delete
#               the deps file if he has made any changes that change the dependencies.
#
#               The makefile will make the 2_makefile_deps if the file is missing.
# 
#
#      about testing
#  
#                the name of the directory you run make in is taken to also be: the name of the library,
#                the name of the main include file (with a .h added), and the name of the include directory
#                where the individual headers are found. It is called LIB
#
#	         test programs are kept in a subdirectory called 1_tests, and are either .exl.cc, ex.cc,
#                .sh files. When 'make tests' target is invoked they are all run. Test executables return 0
#                if the test fails, non-zero otherwise.  
#
#                to remove a test from the pool move it into the subdirectory in 1_tests, 9_broken,
#                5_more_tests of 5_scratch.  broken tests are things that are known but must be fixed
#                before a release.  5_more_tests are tests being worked on.  5_scratch is stuff that is
#                probably going to be deleted.  if there is a 5_deprecated, that is for good stuff but it
#                is no longer used for some reason or other.
#
#                There is a standard source code template and a
#                messaging convention.  Also, the names, by convention,are test_xxxx_ where xxx is a
#                hexadecimal series nummber.  If all the test executables pass the file 1_TESTS_PASSED is
#                left in the directory.  Otherwise the file 1_TESTS_FAILED is left in the directory.
#
#     about release directory
#
#               this is set in the ApplicationBase variable by rt_init
#
#               after the tests pass stuff might be copied to the release directory using
# 
#                  make install
#
#               the release directory must have these subdirectories:  
#
#                  bin documents include src
#
#
#

# set the shell to use
SHELL=/bin/bash

# the LIB variable is now passed in to us as a parameter to make, see 'rtmake'
# LIB= 

# some versions of Linux need a -e option others complain if there is a -e .. and it isn't the binary for echo ..
ECHO= echo
#ECHO= echo -e

# a single space literal, for example if you wanted to subsitute commas to spaces: $(subst $(space),;,$(string))  
# we ran into this out of a need to send multiple separate command arguments to a shell script from one variable value
#
blank :=
space :=$(blank)  $(blank)


SCRATCHDIR= 5_scratch # clean and others put things here


# see 2_bin/local_build for the compile and link flags
SOURCES= $(wildcard *.lib.cc) $(wildcard *.exl.cc) $(wildcard *.ex.cc)

OBJECTS= $(patsubst %.cc, %.o, $(SOURCES))
LIB_OBJECTS= $(patsubst %.cc, %.o, $(wildcard *.lib.cc))
EXEC_OBJECTS= $(patsubst %.cc, %.o, $(wildcard *.ex.cc) $(wildcard *.exl.cc) )

# the sort causes compiles to go in lexical order by file name, this is used to order the tests e.g.
EXEC=     $(sort   $(patsubst %.ex.cc,  %, $(wildcard *.ex.cc)))
EXEC_WL=  $(sort   $(patsubst %.exl.cc, %, $(wildcard *.exl.cc)))
EXECS= $(sort $(EXEC_WL) $(EXEC))

version:
	@echo '---- rtmake $@:------------------------------------------------------------'
	@echo `pwd`'>' 
	@echo source_local_makefile version 4.0
	@echo "RT_BASE: " $(RT_BASE)
	@echo "APPLICATION_BASE: " $(APPLICATION_BASE)
	@echo "APPLICATION: " $(APPLICATION)
	@echo "CXX: " $(CXX)
	@echo "CXXFLAGS: " $(CXXFLAGS)
	@echo "LIB: " $(LIB)
	@echo "LIBPATH: " $(LIBPATH)
	@echo "LINKFLAGS: " $(LINKFLAGS)
	@echo "note, the rt bin directory, and the application 2_bin directory must be in the path for rtmake to work:"
	@echo "PATH: "  $(PATH)
	@echo '______end rtmake $@_____'


# chances are if you got to this makefile that rt_init has been run, but we can catch application_init here
# 
init:
	@if [ "$(RT_BASE)" == "" ]; then echo "init_rt has not been sourced"; exit 1; fi
	@if [ "$(APPLICATION_BASE)" == "" ]; then echo "init_application has not been sourced"; exit 1; fi



all: regress

# safe to run this in an already setup or partially setup directory
setup:
	@echo '---- rtmake $@:------------------------------------------------------------'
	@echo `pwd`'>' 
	if [ ! -e $(SCRATCHDIR) ]; then mkdir $(SCRATCHDIR); fi
	if [ ! -e $(RT_BASE)/include/$(APPLICATION) ]; then mkdir $(RT_BASE)/include/$(APPLICATION); fi # a place to put the individual header files
	if [ ! -e 0_Copyright_Notice.txt ]; then cp $(RT_BASE)/documents/0_Copyright_Notice.txt .; fi
	if [ ! -e 1_tests ]; then mkdir 1_tests; fi
	if [ ! -e 1_try ]; then mkdir 1_try; fi
	if [ ! -e 2_bin ]; then mkdir 2_bin; fi
	if [ ! -e 3_documents ]; then mkdir 3_documents; fi
	if [ ! -e 3_to_do.txt ]; then touch 3_to_do.txt; fi
	if [ ! -e 5_deprecated ]; then mkdir 5_deprecated; fi
	@echo '______end rtmake $@_____'


deps: init
	@echo '---- rtmake $@:------------------------------------------------------------'
	@echo `pwd`'>' 
	$(CXX) $(CXXFLAGS) -MM $(SOURCES) 1> 2_makefile_deps
	for i in $(EXEC_WL) ; do\
	   $(ECHO) >> 2_makefile_deps;\
	   $(ECHO) "$$i : $$i.exl.o $(LIBPATH)/$(LIB)" >> 2_makefile_deps;\
	   $(ECHO) "	$(CXX) -o $$i $$i.exl.o $(LINKFLAGS)" >> 2_makefile_deps;\
	done
	for i in $(EXEC) ; do\
	   echo >> 2_makefile_deps;\
	   $(ECHO) "$$i : $$i.ex.o" >> 2_makefile_deps;\
	   $(ECHO) "	$(CXX) -o $$i $$i.ex.o" >> 2_makefile_deps;\
	done
	@echo '______end rtmake $@_____'

lib: init clear_test_flags
	@echo '---- rtmake $@:------------------------------------------------------------'
	@echo `pwd`'>' 
	if [ ! -e 2_makefile_deps ]; then rtmake deps; fi # the user must delete 2_makefile_deps if deps change!
	rtmake sub_lib
	@echo '______end rtmake $@_____'

sub_lib: $(LIB)


$(LIBPATH)/$(LIB):
	@if [ "$(LIBPATH)" != "$(APPLICATION_BASE)" ]; then \
	  echo "LIBPATH: " "$(LIBPATH)";\
	  echo "$APPLICATION_BASE: " "$(APPLICATION_BASE)";\
	  echo "The library path is not the same as the application base so don't know how to make it.";\
        fi
	[ "$(LIBPATH)" == "$(APPLICATION_BASE)" ]
	cd $(APPLICATION_BASE); rtmake lib

execs: init $(LIBPATH)/$(LIB)
	@echo '---- rtmake $@:------------------------------------------------------------'
	@echo `pwd`'>' 
	if [ ! -e 2_makefile_deps ]; then rtmake deps; fi
	rtmake sub_execs
	# for i in $(EXEC_OBJECTS); do rm $$i; done
	@echo '______end rtmake $@_____'

sub_execs: $(EXECS)


clear_test_flags:
	@if [ ! -d 1_tests ]; then echo "can only rtmake tests when there is a 1_tests directory, did you mean: rtmake execs ?"; fi
	@ test -d 1_tests # we only need to clear tests flags when there is a tests dir
	@echo '---- rtmake $@:------------------------------------------------------------'
	@echo `pwd`'>' 
	echo `date_stamp` $(APPLICATION)   > 1_tests_log
	if [ -f 1_tests_passed ]; then  \
	   if [ `repo_tracks 1_tests_passed` == "y" ]; then hg rm 1_tests_passed; else rm 1_tests_passed; fi; \
	fi
	if [ -f 1_tests_failed ]; then  \
	   if [ `repo_tracks 1_tests_failed` == "y" ]; then hg rm 1_tests_failed; else rm 1_tests_failed; fi; \
	fi
	echo "tests have not been run" >  1_tests_log
	@echo '______end rtmake $@_____'

tests: clear_test_flags
	@echo '---- rtmake $@:------------------------------------------------------------'
	@echo `pwd`'>' 
	echo `date_stamp` $(APPLICATION)   > 1_tests_log
	cd 1_tests; rtmake sub_test;
	@echo '______end rtmake $@_____'

sub_test: 
	rtmake execs
	for i in $(EXECS); do \
	   echo >> ../1_tests_log; \
	   echo ">$$i launching .." | tee -a ../1_tests_log;\
	   if ./$$i 2>> ../1_tests_log; then echo ">$$i passed" | tee -a ../1_tests_log; else echo "#>$$i failed" | tee -a ../1_tests_log; touch ../1_tests_failed; fi;\
	done
	if [ -e ../1_tests_failed ]; then echo "test set FAIL" | tee -a  ../1_tests_log; \
	   else echo "test set PASS" | tee -a  ../1_tests_log; touch ../1_tests_passed; fi #if none failed, then they passed

regress: init
	@if [ ! -d 1_tests ]; then echo "can only do regression when there is a 1_tests directory"; fi
	@test -d 1_tests 
	@echo '---- rtmake $@:------------------------------------------------------------'
	@echo `pwd`'>' 
	rtmake dist_clean
	rtmake deps
	rtmake lib
	rtmake tests
	@echo '______end rtmake $@_____'

install: all
	@echo '---- rtmake $@:------------------------------------------------------------'
	@echo `pwd`'>' 
	@if[ ! -e 1_tests_passed ]; then echo "can't install as tests have not passed"; fi
	@test -e test_passed
	for i in $(BIN); do cp $$i $(RT_BASE)/bin; done
	cp $(LIB) $(RT_BASE)/lib
	cp $(APPLICATION).h  $(RT_BASE)/include
	if [ -d $(APPLICATION) ]; then cp $(APPLICATION)/*.h $(RT_BASE)/include/$(APPLICATION); fi
	@echo '______end rtmake $@_____'

# "rtmake lib" "rtmake execs" now cleans up the .o files afterward, so clean should not be needed often
# clean works on the current working directory, dist_clean also  descends into 1_tests and 1_try
#
clean: init
	@echo '---- rtmake $@:------------------------------------------------------------'
	@echo `pwd`'>' 
	for i in $(wildcard *~); do mv $$i $(SCRATCHDIR); done
	for i in $(wildcard *.lib.o) $(wildcard *.exl.o) $(wildcard *.ex.o); do rm $$i; done 
	if [ -f 2_makefile_deps ]; then rm 2_makefile_deps; fi
	if [ -f $(LIB) ]; then mv $(LIB) 5_scratch; fi
	for i in $(EXEC_WL) $(EXEC); do if [ -e $$i ]; then rm $$i; fi; done 
	@echo '______end rtmake $@_____'


# dist_clean is used to clean thing up before doing a checkin,  hg add should be safe after a dist_clean
# dist_clean will recurse into the include directory = $(APPLICATION), tests, and try if they are present 
# 
dist_clean: init
	@echo '---- rtmake $@:------------------------------------------------------------'
	@echo `pwd`'>' 
	rtmake clean
	if [ -d $(APPLICATION) ]; then cd $(APPLICATION); rtmake clean; fi
	if [ -d 1_tests ]; then cd 1_tests; rtmake clean; fi
	if [ -d 1_try ] ; then cd 1_try; rtmake clean; fi
	@echo '______end rtmake $@_____'


-include 2_makefile_deps

# recipe for making object files:
#
%.o : %.cc
	$(CXX) $(CXXFLAGS) -c $<


#
$(LIB) : $(LIB_OBJECTS)
	ar rcs $(LIB) $(LIB_OBJECTS)
#	for i in $(LIB_OBJECTS); do rm $$i; done
 


