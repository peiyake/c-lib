#

CC=gcc
demo:libsem_demo.o libsem.o
	$(CC) -o demo libsem_demo.o libsem.o	-lpthread
libsem_demo.o:libsem_demo.c libpublic.h libsem.h
	$(CC) -c libsem_demo.c libpublic.h libsem.h
libsem.o:libsem.c libpublic.h libsem.h	
	$(CC) -c libsem.c libpublic.h libsem.h

clean:
	rm -f *.o demo


