
# nice idea, but the modules have to be made in the correct order, perhpas run this to check the module list
# MAKEABLE= $(shell find module tool -name 'makefile' | grep -v deprecated)

CLEANABLE=\
  module/da\
  module/da/test\
  module/tranche\
  module/debug\
  module/dispatch\
  module/subu-0

MAKEABLE=\
  module/da\
  module/da/test\
  module/tranche\
  module/debug\
  module/dispatch\
  module/subu-0


.PHONY: all
all:
	for dir in $(MAKEABLE); do make -C $$dir dist-clean dep lib exec share || true; done

.PHONY: info
info:
	@echo "MAKEABLE:" $(MAKEABLE)

.PHONY: setup
setup:
	for dir in $(MAKEABLE); do make -C $$dir setup || true; done

.PHONY: dep
dep:
	for dir in $(MAKEABLE); do make -C $$dir dep || true; done

.PHONY: update
update:
	for dir in $(MAKEABLE); do make -C $$dir lib exec share || true; done

.PHONY: clean
clean:
	for dir in $(CLEANABLE); do make -C $$dir clean || true; done

.PHONY: dist-clean
dist-clean : clean
	for i in $(wildcard module/share/lib/*); do rm $$i; done
	for i in $(wildcard module/share/inc/*); do rm $$i; done
	for i in $(wildcard module/share/bin/*); do rm $$i; done





