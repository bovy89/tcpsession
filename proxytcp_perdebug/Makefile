GCCFLAGS=-ansi -Wall -pedantic -ggdb
LINKERFLAGS=-lpthread

all:  tcpforward2directionthreads.exe

tcpforward2directionthreads.exe: tcpforward2directionthreads.o Makefile
	gcc -m32 -o tcpforward2directionthreads.exe ${GCCFLAGS} ${LINKERFLAGS} tcpforward2directionthreads.o

tcpforward2directionthreads.o: tcpforward2directionthreads.c
	gcc -m32 -c ${GCCFLAGS}  tcpforward2directionthreads.c

#tcpforwardthreads.exe: tcpforwardthreads.o Makefile
#	gcc -o tcpforwardthreads.exe ${GCCFLAGS} ${LINKERFLAGS} tcpforwardthreads.o
#
#tcpforward2threads.o: tcpforwardthreads.c
#	gcc -c ${GCCFLAGS}  tcpforwardthreads.c

clean:	
	rm -f tcpforward2directionthreads.o tcpforward2directionthreads.exe
	rm -f *~
#	rm -f tcpforwardthreads.o tcpforwardthreads.exe


