# da/test

SHELL=/bin/bash
MAKE=/usr/bin/make --no-print-directory -f $(PROJECT_SUBU)/tool/lib/makefile-cc
#MAKE=/usr/bin/make -f $(PROJECT_SUBU)/tool/lib/makefile-cc

-include makefile-flags

.PHONY: all lib exec
all: version deps lib exec

.PHONY: dep
dep:
	if [ -e $(DEPFILE) ]; then rm $(DEPFILE); fi
	$(MAKE) $@

lib:
	$(MAKE) $@

exec:
	$(MAKE) $@

%::
	$(MAKE) $@




