/* Util.c per MINIMO, BASSO, MEDIOBASSO e MEDIOALTO  */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

#include "TCP_Session.h"

int TCPS_setup_connection(int *pserverfd, char *string_IP_remote_address, int port_number_remote)
{
	struct sockaddr_in Local, Serv;
	int ris;

	*pserverfd = TCP_Session_IPv4_Socket();
	if (*pserverfd<0) {
		printf ("TCP_Session_IPv4_Socket() failed, Err: %d \"%s\"\n", errno,strerror(errno));
		fflush(stdout);
		return(0);
	}

	/* name the socket */
	memset ( &Local, 0, sizeof(Local) );
	Local.sin_family		=	AF_INET;
	Local.sin_addr.s_addr	=	htonl(INADDR_ANY);         /* wildcard */
	Local.sin_port			=	htons(0);

	ris = Bind(*pserverfd, (struct sockaddr*) &Local, sizeof(Local));
	if (ris<0)  {
	    printf ("Bind() failed, Err: %d \"%s\"\n",errno,strerror(errno));
		return(0);
	}

	/* assign our destination address */
	memset ( &Serv, 0, sizeof(Serv) );
	Serv.sin_family	 =	AF_INET;
	Serv.sin_addr.s_addr  =	inet_addr(string_IP_remote_address);
	Serv.sin_port		 =	htons(port_number_remote);

#ifdef VERBOSE
	printf ("connecting to %s %d\n", string_IP_remote_address, port_number_remote);
	fflush(stdout);
#endif
	do {
		ris = Connect(*pserverfd, (struct sockaddr*) &Serv, sizeof(Serv));
	} while( (ris<0)&&(errno==EINTR));


	if (ris<0)  {
		printf ("Connect() failed, Err: %d \"%s\"\n",errno,strerror(errno));
		fflush(stdout);
		return(0);
	}
#ifdef VERBOSE
	printf ("connected to %s %d\n", string_IP_remote_address, port_number_remote);
	fflush(stdout);
#endif
	return(1);

}


int TCPS_setup_socket_listening(int *plistenfd, int numero_porta_locale)
{
	int ris;
	struct sockaddr_in Local;
	
	*plistenfd = TCP_Session_IPv4_Socket();
	if (*plistenfd < 0)
	{	
		printf ("TCP_Session_IPv4_Socket() failed, Err: %d \"%s\"\n", errno,strerror(errno));
		return(0);
	}

	/* avoid EADDRINUSE error on bind() */
	ris = SetsockoptReuseAddr(*plistenfd);
	if (!ris)
	{
		printf ("SetsockoptReuseAddr() failed, Err: %d \"%s\"\n", errno,strerror(errno));
		return(0);
	}

	/*name the socket */
	memset ( &Local, 0, sizeof(Local) );
	Local.sin_family= AF_INET;
	/* specifico l'indirizzo IP attraverso cui voglio ricevere la connessione */
	Local.sin_addr.s_addr = htonl(INADDR_ANY);
	Local.sin_port	      = htons(numero_porta_locale);

	ris = Bind(*plistenfd, (struct sockaddr*) &Local, sizeof(Local));
	if (ris<0)
	{	
		printf ("Bind() failed, Err: %d \"%s\"\n",errno,strerror(errno));
		fflush(stderr);
		return(0);
	}

	/* enable accepting of connection  */
	ris = Listen(*plistenfd, 100 );
	if (ris<0)
	{	
		printf ("Listen() failed, Err: %d \"%s\"\n",errno,strerror(errno));
		exit(1);
	}
	return(1);
}


int TCP_setup_connection(int *pserverfd, char *string_IP_remote_address, int port_number_remote)
{
	struct sockaddr_in Local, Serv;
	int ris;

	*pserverfd = Socket(AF_INET, SOCK_STREAM, 0);
	if (*pserverfd<0) {
		printf ("Socket() failed, Err: %d \"%s\"\n", errno,strerror(errno));
		fflush(stdout);
		return(0);
	}

	/* name the socket */
	memset ( &Local, 0, sizeof(Local) );
	Local.sin_family		=	AF_INET;
	Local.sin_addr.s_addr	=	htonl(INADDR_ANY);         /* wildcard */
	Local.sin_port			=	htons(0);

	ris = Bind(*pserverfd, (struct sockaddr*) &Local, sizeof(Local));
	if (ris<0)  {
	    printf ("Bind() failed, Err: %d \"%s\"\n",errno,strerror(errno));
		return(0);
	}

	/* assign our destination address */
	memset ( &Serv, 0, sizeof(Serv) );
	Serv.sin_family	 =	AF_INET;
	Serv.sin_addr.s_addr  =	inet_addr(string_IP_remote_address);
	Serv.sin_port		 =	htons(port_number_remote);

	printf ("connecting to %s %d\n", string_IP_remote_address, port_number_remote);
	fflush(stdout);
	do {
		ris = Connect(*pserverfd, (struct sockaddr*) &Serv, sizeof(Serv));
	} while( (ris<0)&&(errno==EINTR));

	if (ris<0)  {
		printf ("Connect() failed, Err: %d \"%s\"\n",errno,strerror(errno));
		fflush(stdout);
		return(0);
	}
	printf ("connected to %s %d\n", string_IP_remote_address, port_number_remote);
	fflush(stdout);
	return(1);

}

int TCP_setup_socket_listening(int *plistenfd, int numero_porta_locale)
{
	int ris;
	struct sockaddr_in Local;
	
	*plistenfd = Socket(AF_INET, SOCK_STREAM, 0);
	if (*plistenfd < 0)
	{	
		printf ("Socket() failed, Err: %d \"%s\"\n", errno,strerror(errno));
		return(0);
	}

	/* avoid EADDRINUSE error on bind() */
	ris = SetsockoptReuseAddr(*plistenfd);
	if (!ris)
	{
		printf ("SetsockoptReuseAddr() failed, Err: %d \"%s\"\n", errno,strerror(errno));
		return(0);
	}

	/*name the socket */
	memset ( &Local, 0, sizeof(Local) );
	Local.sin_family= AF_INET;
	/* specifico l'indirizzo IP attraverso cui voglio ricevere la connessione */
	Local.sin_addr.s_addr = htonl(INADDR_ANY);
	Local.sin_port	      = htons(numero_porta_locale);

	ris = Bind(*plistenfd, (struct sockaddr*) &Local, sizeof(Local));
	if (ris<0)
	{	
		printf ("Bind() failed, Err: %d \"%s\"\n",errno,strerror(errno));
		fflush(stderr);
		return(0);
	}

	/* enable accepting of connection  */
	ris = Listen(*plistenfd, 100 );
	if (ris<0)
	{	
		printf ("Listen() failed, Err: %d \"%s\"\n",errno,strerror(errno));
		exit(1);
	}
	return(1);
}


ssize_t  Writen (int fd, const void *buf, size_t n)
{   
	size_t	nleft;     
	ssize_t  nwritten;  
	char	*ptr;

	ptr = (void *)buf;
	nleft = n;
	while (nleft > 0) 
	{
		if ( (nwritten = Write(fd, ptr, nleft)) < 0) {
			if (errno == EINTR)	nwritten = 0;   /* and call write() again*/
			else			return(-1);       /* error */
		}
		nleft -= nwritten;
		ptr   += nwritten;
	}
	return(n);
}

int Readn(int fd, char *ptr, int nbytes)
{
	int nleft,nread;

	nleft=nbytes;
	while(nleft>0)
	{
		do {
			nread=Read(fd,ptr,nleft);
		} while ( (nread<0) && (errno==EINTR) );
		if(nread<0)
		{	/* errore */
			char msg[2000];
			sprintf(msg,"readn: errore in lettura [result %d] :",nread);
			perror(msg);
			return(-1);
		}
		else
		{
			if(nread==0) {
				return(0);
				break;
			}
		}

		nleft -= nread;
		ptr   += nread;
	}
return(nbytes);
}

void init_random(void)
{
	srandom( (unsigned int)getpid() );
}

unsigned int inizializza(char *buf, int len)
{
	unsigned int sum=0;
	int i;

	for(i=0;i<len;i++) 
	{
		buf[i] = '0'+(random()%10);
		sum += buf[i] - '0';
	}
	return(sum);
}

unsigned int sommavet(char *buf, int len)
{
	unsigned int sum=0;
	int i;

	for(i=0;i<len;i++) 
		sum += buf[i] - '0';
	return(sum);
}

unsigned int stampavet(char *buf, int len)
{
	unsigned int sum=0;
	int i;

	printf("\n");
	for(i=0;i<len;i++)
	{
		printf(" %d ", buf[i] );
		sum += (buf[i] - '0');
	}
	printf("   somma %u \n", sum );
	return(sum);
}



