
src = $(wildcard *.c)
obj = $(src:.c=.o)
CC=gcc

all: user-mk

user-mk: $(obj)
	$(CC) -o $@ $^

.PHONY: clean
clean:
	rm -rf $(obj) user-mk


