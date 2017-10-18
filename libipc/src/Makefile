CC=gcc

PUBLIC_INCLUDE=../public

libipc.a:libipc.o
	ar rcs -o libipc.a libipc.o

libipc.o:libipc.c
	$(CC) -c libipc.c -I$(PUBLIC_INCLUDE)

.PHONY:clean
clean:
	rm -f *.o *.a
