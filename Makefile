CC = gcc
CFLAGS = -g -W -Wall -Werror -Wno-unused
V = @

all: cs5600sh

cs5600sh: cmdparse.o cmdrun.o main.o
	@echo + link $@
	$(V)$(CC) $(CFLAGS) -lpthread -o $@ cmdparse.o cmdrun.o main.o

cmdrun.o: cmdrun.c cmdparse.h cmdrun.h
	@echo + cc cmdrun.c
	$(V)$(CC) $(CFLAGS) -c cmdrun.c

cmdparse.o: cmdparse.c cmdparse.h
	@echo + cc cmdparse.c
	$(V)$(CC) $(CFLAGS) -c cmdparse.c

main.o: main.c cmdparse.h cmdrun.h
	@echo + cc main.c
	$(V)$(CC) $(CFLAGS) -c main.c

clean:
	@echo + clean
	$(V)rm -rf *.o *~ *.bak core *.core cs5600sh freecheck

test: cs5600sh always
	/usr/bin/perl -w ./tester.pl

always:
	@:

.PHONY: always clean realclean test handin
