CC = gcc
CFLAGS = -Wall -std=gnu99 #-fsanitize=address
PKG = `pkg-config --cflags --libs glib-2.0` -pthread -lmenu -lncurses -lpanel

default: main

main: afldff_networking.o afldff_ncurses.o afldff_access.o
	$(CC) $(CFLAGS) afl-dff.c afldff_access.o afldff_ncurses.o afldff_networking.o $(PKG) -o afldff 

afldff_networking.o: dev/networking/afldff_networking.c dev/networking/afldff_networking.h
	$(CC) $(CFLAGS) -c dev/networking/afldff_networking.c -o afldff_networking.o

afldff_ncurses.o: dev/interface/afldff_ncurses.c dev/interface/afldff_ncurses.h
	$(CC) $(CFLAGS) -c dev/interface/afldff_ncurses.c -o afldff_ncurses.o

afldff_access.o: dev/networking/afldff_access.c dev/networking/afldff_access.h
	$(CC) $(CFLAGS) -c dev/networking/afldff_access.c $(PKG) -o afldff_access.o


clean:
	rm afldff
	rm afldff_networking.o
	rm afldff_ncurses.o
	rm afldff_access.o
