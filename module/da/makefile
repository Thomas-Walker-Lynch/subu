# da/makefile


SHELL=/bin/bash
MAKE=/usr/bin/make --no-print-directory -f $(PROJECT_SUBU)/tool/lib/makefile-cc
#MAKE=/usr/bin/make -f $(PROJECT_SUBU)/tool/lib/makefile-cc

-include makefile-flags

.PHONY: all
all: version

.PHONY: lib
lib:
	$(MAKE) $@
	cp src/da.lib.h include/da.h

%::
	$(MAKE) $@




