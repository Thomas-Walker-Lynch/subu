... turned into a notes file ... #!/bin/bash

good doc:  https://www.gnu.org/software/automake/manual/automake.html#Hello-World

->create src/makefile.am
bin_PROGRAMS=subu-mk-0
subu_mk_0_SOURCES=subu-mk-0.c

->create makefile.am
SUBDIRS = src
dist_doc_DATA = README

->create configure.ac
AC_INIT([mk-subu-0], [1.0], [bug-automake@gnu.org])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_PROG_CC
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([
 makefile
 src/makefile
])
AC_OUTPUT

autoreconf --install
./configure

after changing makefile.am or configure.ac, it is enough to run make to update the build files

