
#CFLAGS= -std=c11 -Werror -H -fsyntax-only 
#CFLAGS= -std=c11 -Werror -O 
CFLAGS= -std=c11 -Werror -ggdb

all: subu-mk-0

subu-mk-0: subu-mk-0.c
	gcc -o subu-mk-0 $(CFLAGS) subu-mk-0.c 
	sudo ./setuid_root_subu-mk-0

