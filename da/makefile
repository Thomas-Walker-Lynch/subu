# da/makefile

SHELL=/bin/bash
MAKE=

-include makefile-flags

.PHONY: all
all: version dep lib exec

.PHONY: lib
lib:
	$(MAKE) $@
	cp src/da.lib.h include/da.h

%::
	$(MAKE) $@




