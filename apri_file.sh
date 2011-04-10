#!/bin/bash

if [ -n "$1" ]

then

gedit ./Sessione/TCP_Session.c ./Sessione/TCP_Session.h ./Sessione/const.h ./Sessione/socket_map_list.h ./applicazioni/$1/Cli$1.c ./applicazioni/$1/Serv$1.c ./applicazioni/$1/Util.c

else

gedit ./Sessione/TCP_Session.c ./Sessione/TCP_Session.h ./Sessione/const.h ./Sessione/socket_map_list.h ./applicazioni/basso/Clibasso.c ./applicazioni/basso/Servbasso.c ./applicazioni/basso/Util.c

fi
