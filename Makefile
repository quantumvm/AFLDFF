CC = gcc
CFLAGS = -Wall -std=gnu99 #-fsanitize=address
PKG = `pkg-config --cflags --libs glib-2.0` -pthread -lmenu -lncurses -lpanel -lssl -lcrypto

default: main

main: afldff_networking.o afldff_ncurses.o afldff_access.o afldff_patch.o
	$(CC) $(CFLAGS) afl-dff.c afldff_access.o afldff_ncurses.o afldff_patch.o afldff_networking.o $(PKG) -o afldff 

afldff_networking.o: dev/networking/afldff_networking.c dev/networking/afldff_networking.h
	$(CC) $(CFLAGS) -c dev/networking/afldff_networking.c $(PKG) -o afldff_networking.o

afldff_ncurses.o: dev/interface/afldff_ncurses.c dev/interface/afldff_ncurses.h
	$(CC) $(CFLAGS) -c dev/interface/afldff_ncurses.c $(PKG) -o afldff_ncurses.o

afldff_access.o: dev/networking/afldff_access.c dev/networking/afldff_access.h
	$(CC) $(CFLAGS) -c dev/networking/afldff_access.c $(PKG) -o afldff_access.o

afldff_patch.o: dev/patch/afldff_patch.c dev/patch/afldff_patch.h
	$(CC) $(CFLAGS) -c dev/patch/afldff_patch.c $(PKG)  -o afldff_patch.o

install: afldff
	install -m 0755 afldff /usr/local/bin
	install -m 0755 -d /opt/afldff
	install -m 0755 -d /opt/afldff/patches
	install -m 0755 dev/patch/patch_files/* /opt/afldff/patches
	install -m 0755 dev/networking/afldff_networking.c /opt/afldff
	install -m 0755 dev/networking/afldff_networking.h /opt/afldff

clean:
	rm afldff
	rm afldff_networking.o
	rm afldff_ncurses.o
	rm afldff_access.o
	rm afldff_patch.o
