#subdirectories=$(shell /usr/bin/find . -maxdepth 1 -printf "%f " | sed y/\./\ /)
subdirectories=src

all :
#	$(foreach dir, $(subdirectories), \
#		if [ -f $(dir)/0_makefile ]; then \
#			make -C $(dir) all && make -C $(dir) install; \
#		fi;\
#	)

dist-clean :
	$(foreach dir, $(subdirectories), \
		cd $(dir);\
		make dist-clean;\
	)




