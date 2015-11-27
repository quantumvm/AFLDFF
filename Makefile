CC = gcc
CFLAGS = -Wall -std=gnu99
PKG = `pkg-config --cflags --libs glib-2.0` 

default: main

main: afl_networking.o
	$(CC) $(CFLAGS) afl-dff.c afldff_networking.o $(PKG) -o afldff 

afl_networking.o:
	$(CC) $(CFLAGS) -c dev/networking/afldff_networking.c -o afldff_networking.o

clean:
	rm afldff
	rm afldff_networking.o
