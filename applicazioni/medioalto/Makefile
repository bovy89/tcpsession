DIR_MODULE=../../Sessione
GCCFLAGS=-ansi -Wall -pedantic -ggdb -Wunused -I${DIR_MODULE}/
LINKERFLAGS=-lpthread

all:  Servmedioalto.exe Climedioalto.exe

Climedioalto.exe: Climedioalto.o ${DIR_MODULE}/TCP_Session.o Util.o
	gcc -o Climedioalto.exe ${GCCFLAGS} Climedioalto.o ${DIR_MODULE}/TCP_Session.o Util.o

Climedioalto.o: Climedioalto.c ${DIR_MODULE}/TCP_Session.h Util.h
	gcc -c ${GCCFLAGS}  Climedioalto.c

Servmedioalto.exe: Servmedioalto.o ${DIR_MODULE}/TCP_Session.o Util.o
	gcc -o Servmedioalto.exe ${GCCFLAGS} ${LINKERFLAGS} Servmedioalto.o ${DIR_MODULE}/TCP_Session.o Util.o

Servmedioalto.o: Servmedioalto.c ${DIR_MODULE}/TCP_Session.h Util.h
	gcc -c ${GCCFLAGS} Servmedioalto.c

Util.o: Util.c ${DIR_MODULE}/TCP_Session.h
	gcc -c ${GCCFLAGS}  Util.c

${DIR_MODULE}/TCP_Session.o: ${DIR_MODULE}/TCP_Session.c ${DIR_MODULE}/TCP_Session.h
	gcc -c ${GCCFLAGS} -o ${DIR_MODULE}/TCP_Session.o ${DIR_MODULE}/TCP_Session.c

clean:	
	rm -f *.exe
	rm -f Servmedioalto.o Servmedioalto.exe
	rm -f Climedioalto.o Climedioalto.exe
	rm -f ${DIR_MODULE}/TCP_Session.o Util.o 

# `
# {
# }


