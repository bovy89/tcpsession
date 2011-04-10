DIR_MODULE=../../Sessione
GCCFLAGS= -ansi -Wall -pedantic -ggdb -I${DIR_MODULE}/
LINKERFLAGS=-lpthread

all:  Servminimo.exe Climinimo.exe

Servminimo.exe: Servminimo.o ${DIR_MODULE}/TCP_Session.o Util.o
	gcc -m32 -o Servminimo.exe ${GCCFLAGS} ${LINKERFLAGS} Servminimo.o ${DIR_MODULE}/TCP_Session.o Util.o

Servminimo.o: Servminimo.c ${DIR_MODULE}/TCP_Session.h Util.h
	gcc -m32 -c ${GCCFLAGS} Servminimo.c

Climinimo.exe: Climinimo.o ${DIR_MODULE}/TCP_Session.o Util.o
	gcc -m32 -o Climinimo.exe ${GCCFLAGS} Climinimo.o ${DIR_MODULE}/TCP_Session.o Util.o

Climinimo.o: Climinimo.c ${DIR_MODULE}/TCP_Session.h Util.h
	gcc -m32 -c ${GCCFLAGS}  Climinimo.c

Util.o: Util.c ${DIR_MODULE}/TCP_Session.h
	gcc -m32 -c ${GCCFLAGS}  Util.c

${DIR_MODULE}/TCP_Session.o: ${DIR_MODULE}/TCP_Session.c ${DIR_MODULE}/TCP_Session.h ${DIR_MODULE}/socket_map_list.h
	gcc -m32 -c ${GCCFLAGS} -o ${DIR_MODULE}/TCP_Session.o ${DIR_MODULE}/TCP_Session.c


clean:	
	rm -f *.exe *~
	rm -f Servminimo.o Servminimo.exe
	rm -f Climinimo.o Climinimo.exe
	rm -f ${DIR_MODULE}/TCP_Session.o Util.o 

# `
# {
# }


