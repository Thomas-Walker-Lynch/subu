#subdirectories=$(shell /usr/bin/find . -maxdepth 1 -printf "%f " | sed y/\./\ /)
#subdirectories=src src/1_tests src/1_try
subdirectories=src

all :
	$(foreach dir, $(subdirectories), \
		if [ -f $(dir)/7_makefile ]; then \
			make -C $(dir) all && make -C $(dir) install; \
		fi;\
	)

clean :
	$(foreach dir, $(subdirectories), \
		cd $(dir);\
		make clean;\
	)

dist-clean :
	$(foreach dir, $(subdirectories), \
		cd $(dir);\
		make dist-clean;\
	)




