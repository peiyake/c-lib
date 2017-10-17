all:susieipc libipc piakipc
	gcc -o susie susieipc.o libipc.o -lpthread
	gcc -o piak  piakipc.o  libipc.o -lpthread
libipc:libipc.c libipc.h libpublic.h
	#gcc -c libipc.c libipc.h libpublic.h
	gcc -c libipc.c 
susieipc:susieipc.c libipc.h libpublic.h
	#gcc -c susieipc.c libipc.h libpublic.h
	gcc -c susieipc.c 
piakipc:piakipc.c libipc.h libpublic.h
	#gcc -c piakipc.c libipc.h libpublic.h
	gcc -c piakipc.c 


.PHONY:clean

clean:
	rm -f *.o *.gch susie piak

