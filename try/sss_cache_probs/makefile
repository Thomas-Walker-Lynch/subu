
src = $(wildcard *.c)
obj = $(src:.c=.o)
CC=gcc

all: sss_cache

sss_cache: $(obj)
	$(CC) -o $@ $^

.PHONY: clean
clean:
	rm -rf $(obj) sss_cache




