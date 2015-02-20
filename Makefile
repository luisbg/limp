CC=gcc
CFLAGS=-I.
DEPS = limp.h file_io.h

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

limp: limp.o file_io.o
	$(CC) -o limp limp.o file_io.o -I.
