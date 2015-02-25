CC=gcc
CFLAGS=-I.
DEPS = limp.h file_io.h jpeg.h

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

limp: limp.o file_io.o jpeg.o
	$(CC) -o limp limp.o file_io.o jpeg.o -I.
