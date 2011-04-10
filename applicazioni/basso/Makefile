DIR_MODULE=../../Sessione
GCCFLAGS= -ansi -Wall -pedantic -ggdb -I${DIR_MODULE}/
LINKERFLAGS=-lpthread

all:  Servbasso.exe Clibasso.exe

Servbasso.exe: Servbasso.o ${DIR_MODULE}/TCP_Session.o Util.o
	gcc -m32 -o Servbasso.exe ${GCCFLAGS} ${LINKERFLAGS} Servbasso.o ${DIR_MODULE}/TCP_Session.o Util.o

Servbasso.o: Servbasso.c ${DIR_MODULE}/TCP_Session.h Util.h
	gcc -m32 -c ${GCCFLAGS} Servbasso.c

Clibasso.exe: Clibasso.o ${DIR_MODULE}/TCP_Session.o Util.o
	gcc -m32 -o Clibasso.exe ${GCCFLAGS} Clibasso.o ${DIR_MODULE}/TCP_Session.o Util.o

Clibasso.o: Clibasso.c ${DIR_MODULE}/TCP_Session.h Util.h
	gcc -m32 -c ${GCCFLAGS}  Clibasso.c

Util.o: Util.c ${DIR_MODULE}/TCP_Session.h
	gcc -m32 -c ${GCCFLAGS}  Util.c

${DIR_MODULE}/TCP_Session.o: ${DIR_MODULE}/TCP_Session.c ${DIR_MODULE}/TCP_Session.h
	gcc -m32 -c ${GCCFLAGS} -o ${DIR_MODULE}/TCP_Session.o ${DIR_MODULE}/TCP_Session.c


clean:	
	rm -f *.exe *~
	rm -f Servbasso.o Servbasso.exe
	rm -f Clibasso.o Clibasso.exe
	rm -f ${DIR_MODULE}/TCP_Session.o Util.o 

# `
# {
# }


