
MAKEABLE=\
	module/da\
	module/da/test\
	module/debug\
	module/tranche\
	module/dispatch\
	module/subu-0

.PHONY: all
all:
	for dir in $(MAKEABLE); do pushd $$dir; make dist-clean dep lib exec share; popd; done

.PHONY: dep
dep:
	for dir in $(MAKEABLE); do pushd $$dir; make dep; popd; done

.PHONY: update
update:
	for dir in $(MAKEABLE); do pushd $$dir; make lib exec share; popd; done

.PHONY: info
info:
	@echo "MAKEABLE:" $(MAKEABLE)

.PHONY: clean
clean:
	for dir in $(MAKEABLE); do pushd $$dir; make clean; popd; done

.PHONY: dist-clean
dist-clean : clean
	for i in $(wildcard module/share/lib/*); do rm $$i; done
	for i in $(wildcard module/share/inc/*); do rm $$i; done
	for i in $(wildcard module/share/bin/*); do rm $$i; done





