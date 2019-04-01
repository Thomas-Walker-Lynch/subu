# src-dispatch/makefile

SHELL=/bin/bash
MAKE=/usr/bin/make --no-print-directory -f $(PROJECT_SUBU)/tool/lib/makefile-cc
#MAKE=/usr/bin/make -f $(PROJECT_SUBU)/tool/lib/makefile-cc

-include makefile-flags
DEPFILE=$(TMPDIR)/makefile-cc.deps

.PHONY: all 
all: version

.PHONY: info
info:
	@echo "TRCDIR: " $(TRCDIR)
	$(MAKE) $@

.PHONY: setup
setup:
	[ ! -e $(TRCDIR)  ]  && mkdir $(TRCDIR)  || true
	$(MAKE) $@

.PHONY: dep
dep:
	if [ -e $(DEPFILE) ]; then rm $(DEPFILE); fi
	trcsources=$(wildcard $(TRCDIR)/*.trc.c)$(wildcard $(TRCDIR)/*.trc.cc);\
	$(ECHO) $$trcsources;\
	if [ ! -z "$$trcsources" ]; then\
	  trctargets=$$(tranche-target $$trcsources -sep " " -tdir $(SRCDIR) );\
	  $(ECHO) $$trctargets;\
	  tranche-make $$trcsources -tdir $(SRCDIR) -mfile $(DEPFILE);\
	  $(MAKE) $$trctargets;\
	fi
	$(MAKE) $@

.PHONY: lib
lib:
	$(MAKE) $@

%::
	$(MAKE) $@




