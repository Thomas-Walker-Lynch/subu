# src-dispatch/makefile

SHELL=/bin/bash
MAKE=/usr/bin/make --no-print-directory -f $(PROJECT_SUBU)/tool/lib/makefile-cc

-include makefile-flags

.PHONY: all 
all: version

.PHONY: info
info:
	@echo "TRCDIR: " $(TRCDIR)
	@echo "SHAREDIR: " $(SHAREDIR)
	$(MAKE) $@

.PHONY: setup
setup:
	[ ! -e $(TRCDIR)  ]  && mkdir $(TRCDIR)  || true
	$(MAKE) $@

.PHONY: dep
dep:
	if [ -e $(DEPFILE) ]; then rm $(DEPFILE); fi
	trcsources=($(wildcard $(TRCDIR)/*.trc.c)$(wildcard $(TRCDIR)/*.trc.cc));\
	if [ ! -z "$$trcsources" ]; then\
	  trctargets=$$(tranche-target $${trcsources[@]} -sep " " -tdir $(SRCDIR) );\
	  $(ECHO) "trcsources: " $${trcsources[@]};\
	  $(ECHO) "trctargets: " $$trctargets;\
	  tranche-make $${trcsources[@]} -tdir $(SRCDIR) -mfile $(DEPFILE);\
	  $(MAKE) $$trctargets;\
	fi
	$(MAKE) $@

.PHONY: lib
lib:
	cp $(SRCDIR)/common.lib.h $(INCDIR)/subu.h
	cat $(SRCDIR)/subudb.lib.h >> $(INCDIR)/subu.h
	cat $(SRCDIR)/subu.lib.h >> $(INCDIR)/subu.h
	$(MAKE) $@

%::
	@echo default target
	$(MAKE) $@




