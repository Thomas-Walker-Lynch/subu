# tranche/makefile

SHELL=/bin/bash
MAKE=/usr/bin/make --no-print-directory -f $(PROJECT_SUBU)/tool/lib/makefile-cc
#MAKE=/usr/bin/make -f $(PROJECT_SUBU)/tool/lib/makefile-cc

-include makefile-flags

.PHONY: all version deps lib exec
all: version deps lib exec

lib:
	$(MAKE) $@
	cp src/tranche.lib.h include/tranche.h

exec:
	$(MAKE) $@

share:
	if [ ! -z "$(wildcard $(EXECDIR)/*)" ]; then cp $(EXECDIR)/* $(PROJECT_SUBU)/tool/bin; fi

%::
	$(MAKE) $@




