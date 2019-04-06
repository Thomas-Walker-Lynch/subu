# da/

SHELL=/bin/bash
MAKE=/usr/bin/make --no-print-directory -f $(PROJECT_SUBU)/tool/lib/makefile-cc

#MAKE=/usr/bin/make -f $(PROJECT_SUBU)/tool/lib/makefile-cc

-include makefile-flags

.PHONY: all
all: version

.PHONY: dep
dep:
	if [ -e $(DEPFILE) ]; then rm $(DEPFILE); fi
	$(MAKE) $@

.PHONY: lib
lib:
	cp $(SRCDIR)/da.lib.h $(INCDIR)/da.h
	$(MAKE) $@

.PHONY: exec
exec:
	$(MAKE) $@

%::
	$(MAKE) $@




