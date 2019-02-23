#subdirectories=$(shell /usr/bin/find . -maxdepth 1 -printf "%f " | sed y/\./\ /)
subdirectories=src

all :
	$(foreach dir, $(subdirectories), \
		if [ -f $(dir)/makefile ]; then \
			make -C $(dir) all && make -C $(dir) install; \
		fi;\
	)

clean :
	$(foreach dir, $(subdirectories), \
		if [ -f ./$(dir)/makefile ]; then \
			make -C $(dir) clean; \
		fi;\
	)
