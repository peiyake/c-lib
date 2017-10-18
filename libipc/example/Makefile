CC=gcc
PUBLIC_INCLUDE=-I../public \
			   -I../src
LIB_DIR=../src

LFLAGS=-lpthread -lipc

all:piakipc.o susieipc.o
	$(CC) -o piak  piakipc.o  -L$(LIB_DIR)  $(LFLAGS)
	$(CC) -o susie susieipc.o -L$(LIB_DIR)  $(LFLAGS)

piakipc.o:piakipc.c
	$(CC) -c piakipc.c $(PUBLIC_INCLUDE) 
susieipc.o:susieipc.c
	$(CC) -c susieipc.c $(PUBLIC_INCLUDE) 

.PHONY:clean
clean:
	rm -f *.o piak susie

