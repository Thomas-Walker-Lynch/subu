SRCDIRS= $(wildcard src-*)
MAKEABLE=$(shell \
    find src-*\
	-mindepth 1  \
	\( -name '?_makefile' -o -name 'makefile' -o -name 'Makefile' \)\
	-printf "%h\n"\
    | grep -v deprecated | grep -v doc | sort -u | sed ':a;N;$!ba;s/\n/ /g' \
)

all :
	$(foreach dir, $(MAKEABLE), \
	  make -C $$dir
	  -make -C $$dir stage
	)

info:
	@echo "SRCDIRS:" $(SRCDIRS)
	@echo "MAKEABLE:" $(MAKEABLE)

clean :
	for dir in $(MAKEABLE); do pushd $$dir; make clean; popd; done

dist-clean : clean
	for i in $(wildcard stage/lib/*); do rm $$i; done
	for i in $(wildcard stage/inc/*); do rm $$i; done
	for i in $(wildcard stage/bin/*); do rm $$i; done





