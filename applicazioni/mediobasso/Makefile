DIR_MODULE=../../Sessione
GCCFLAGS=-ansi -Wall -pedantic -ggdb -I${DIR_MODULE}/
LINKERFLAGS=-lpthread

all:  Servmediobasso.exe Climediobasso.exe

Servmediobasso.exe: Servmediobasso.o ${DIR_MODULE}/TCP_Session.o Util.o
	gcc -m32 -o Servmediobasso.exe ${GCCFLAGS} ${LINKERFLAGS} Servmediobasso.o ${DIR_MODULE}/TCP_Session.o Util.o

Servmediobasso.o: Servmediobasso.c ${DIR_MODULE}/TCP_Session.h Util.h
	gcc -m32 -c ${GCCFLAGS} Servmediobasso.c

Climediobasso.exe: Climediobasso.o ${DIR_MODULE}/TCP_Session.o Util.o
	gcc -m32 -o Climediobasso.exe ${GCCFLAGS} Climediobasso.o ${DIR_MODULE}/TCP_Session.o Util.o

Climediobasso.o: Climediobasso.c ${DIR_MODULE}/TCP_Session.h Util.h
	gcc -m32 -c ${GCCFLAGS}  Climediobasso.c

Util.o: Util.c ${DIR_MODULE}/TCP_Session.h
	gcc -m32 -c ${GCCFLAGS}  Util.c

${DIR_MODULE}/TCP_Session.o: ${DIR_MODULE}/TCP_Session.c ${DIR_MODULE}/TCP_Session.h
	gcc -m32 -c ${GCCFLAGS} -o ${DIR_MODULE}/TCP_Session.o ${DIR_MODULE}/TCP_Session.c


clean:	
	rm -f *.exe *~
	rm -f Servmediobasso.o Servmediobasso.exe
	rm -f Climediobasso.o Climediobasso.exe
	rm -f ${DIR_MODULE}/TCP_Session.o Util.o 

# `
# {
# }


