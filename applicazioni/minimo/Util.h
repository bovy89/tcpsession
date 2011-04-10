/* Util.h  per MINIMO, BASSO, MEDIOBASSO e MEDIOALTO  */

#ifndef __UTIL_H__
#define __UTIL_H__

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
*/

#include "TCP_Session.h"

#define LENVETLETTURA 100000L
/* #define LENVETLETTURA 10L  */
#define LENVETSCRITTURA 10

extern int		TCPS_setup_connection(int *pserverfd, char *string_IP_remote_address, int port_number_remote);
extern int		TCPS_setup_socket_listening(int *plistenfd, int numero_porta_locale);
extern int		TCP_setup_connection(int *pserverfd, char *string_IP_remote_address, int port_number_remote);
extern int		TCP_setup_socket_listening(int *plistenfd, int numero_porta_locale);
extern ssize_t  	Writen (int fd, const void *buf, size_t n);
extern int		Readn(int fd, char *ptr, int nbytes);
extern void		init_random(void);
extern int		inizializza(char *buf, int len);
extern int		sommavet(char *buf, int len);
extern int		stampavet(char *buf, int len);

#endif   /*  __UTIL_H__  */ 


