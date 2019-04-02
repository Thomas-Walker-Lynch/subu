
MAKEABLE=\
	module/da\
	module/da/test\
	module/debug\
	module/tranche\
	module/dispatch

.PHONY: all
all:
	$(foreach dir, $(MAKEABLE), \
	  -make -C $$dir dist-clean dep lib exec share
	)

.PHONY: all
update:
	$(foreach dir, $(MAKEABLE), \
	  -make -C $$dir lib exec share
	)

.PHONY: info
info:
	@echo "SRCDIRS:" $(SRCDIRS)
	@echo "MAKEABLE:" $(MAKEABLE)

.PHONY: clean
clean:
	for dir in $(MAKEABLE); do pushd $$dir; make clean; popd; done

.PHONY: dist-clean
dist-clean : 
	for dir in $(MAKEABLE); do pushd $$dir; make dist-clean; popd; done
	for i in $(wildcard module/share/lib/*); do rm $$i; done
	for i in $(wildcard module/share/inc/*); do rm $$i; done
	for i in $(wildcard module/share/bin/*); do rm $$i; done





