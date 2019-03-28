# tranche/makefile

SHELL=/bin/bash

-include makefile-flags

.PHONY: all version deps lib exec
all: version deps lib exec

lib:
	$(MAKE) $@
	cp src/tranche.lib.h include/tranche.h

exec:
	$(MAKE) $@

%::
	$(MAKE) $@




