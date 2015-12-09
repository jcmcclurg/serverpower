CC?=gcc
CFLAGS?=-Wall -g -D_GNU_SOURCE
TARGETS=signal_insert_delays
LIBS=cmdline.o get_children.o trie.o

UNAME := $(shell uname)

all::	$(TARGETS) $(LIBS)

#insertDelays:	insertDelays.c $(LIBS)
#	$(CC) -o insertDelays insertDelays.c $(LIBS) $(CFLAGS) $(FLAGS)

signal_insert_delays: signal_insert_delays.c $(LIBS)
	$(CC) -o signal_insert_delays signal_insert_delays.c $(LIBS) $(CFLAGS) $(FLAGS) -lrt

cmdline.o: get_children.o trie.o cmdline.c cmdline.h
	$(CC) -c cmdline.c $(CFLAGS) $(FLAGS)

get_children.o: get_children.c get_children.h
	$(CC) -c get_children.c $(CFLAGS) $(FLAGS)

trie.o: trie.c trie.h
	$(CC) -c trie.c $(CFLAGS) $(FLAGS)

clean:
	rm -f *~ *.o $(TARGETS)
