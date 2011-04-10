GCCFLAGS= -ansi -Wall -pedantic -ggdb -I../Sessione/
LINKERFLAGS=-lpthread

all: TCP_Session.o


TCP_Session.o: TCP_Session.c TCP_Session.h socket_map_list.h const.h Makefile
	gcc -m32 -c ${GCCFLAGS}  TCP_Session.c

clean:	
	rm -f TCP_Session.o
	rm -f *~

