all:
	gcc afl-dff.c networking.c  `pkg-config --cflags --libs glib-2.0` -std=c99 -o afldff 


clean:
	rm afldff
