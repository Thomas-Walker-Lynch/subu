subdirectories=$(shell /usr/bin/find . -maxdepth 1 -printf "%f " | sed y/\./\ /)

all :
	$(foreach dir, $(subdirectories), \
		if [ -f $(dir)/Makefile ]; then \
			make -C $(dir) all && make -C $(dir) install; \
		fi;\
	)

clean :
	$(foreach dir, $(subdirectories), \
		if [ -f ./$(dir)/Makefile ]; then \
			make -C $(dir) clean; \
		fi;\
	)
