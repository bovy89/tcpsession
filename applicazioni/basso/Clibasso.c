/* Clibasso.c  */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

#include "TCP_Session.h"
#include "Util.h"

void usage(void) 
{  printf ("usage: ./Clibasso.exe REMOTE_IP_NUMBER REMOTE_PORT_NUMBER [lOCAL_IP]\n");
   exit(1);
}

int main(int argc, char *argv[])
{
  char string_remote_ip_address[100];
  char string_local_ip_address[100];
  short int remote_port_number;
  int socketfd[2], ris;
  int n, nread, nwrite;
  char buflettura[2][LENVETLETTURA];
  char bufscrittura[2][LENVETSCRITTURA];
  unsigned int sumlettura[2];
  unsigned int sum[2];
  int K,H;

  if	( (sizeof(int)!=sizeof(long int)) ||  (sizeof(int)!=4)  )
  {
	printf ("dimensione di int e/o long int != 4  -> TERMINO\n");
	fflush(stdout);
    	exit(1);
  }

  if((argc!=3)&&(argc!=4)) { printf ("necessari 2 o 3 parametri\n"); usage(); exit(1);  }
  else {
    strncpy(string_remote_ip_address, argv[1], 99);
    remote_port_number = atoi(argv[2]);
    if(argc==4)
    	strncpy(string_local_ip_address, argv[3], 99);
  }
  
  if( ! Init_TCP_Session_Module(NULL) )
  {
    printf ("Init_TCP_Session_Module() failed\n");
	fflush(stdout);
    exit(1);
  }

  ris= TCPS_setup_connection(&(socketfd[0]), string_remote_ip_address, remote_port_number);
  if (!ris)  {
	printf ("impossibile connettersi al server %s %d\nTERMINO!!\n", 
			string_remote_ip_address, remote_port_number );
	fflush(stdout);
	return(0);
  }
  printf ("dopo prima Connect()\n");
  fflush(stdout);

  ris= TCPS_setup_connection(&(socketfd[1]), string_remote_ip_address, remote_port_number+1);
  if (!ris)  {
	printf ("impossibile connettersi al server %s %d\nTERMINO!!\n", 
			string_remote_ip_address, remote_port_number+1 );
	fflush(stdout);
	return(0);
  }
  printf ("dopo seconda Connect()\n");
  fflush(stdout);


  for(K=1;K>=0;K--)
  {
  	nread=sizeof(int);
/*printf("nread= %d\n", nread);*/
  	printf ("Readn() sum ");
	printf("fd readn %d \n", socketfd[K]);
  	fflush (stdout);
  	n = Readn (socketfd[K], (char*) &(sum[K]), nread);
  	if (n != nread)
	{	
		printf ("Readn() sum failed \n");
		fflush(stdout);
		return (0);
	}
	sum[K]=ntohl(sum[K]);
	printf("connessione %d ricevuto valore somma %u\n", K, sum[K] );
	fflush(stdout);
  }

  for(K=1;K>=0;K--)
  {
	/* scrittura */
	nwrite = LENVETSCRITTURA;
	printf ("Writen()\n");
	fflush (stdout);
	n = Writen (socketfd[K], bufscrittura[K], nwrite );
	if (n != nwrite)
	{
		printf ("Writen() failed \n");
		fflush(stdout);
		return (0);
	}

	/* lettura */
  	for(H=0;H<10;H++)
  	{
		nread=LENVETLETTURA/10;
		/*nread=LENVETLETTURA;*/
		printf ("Readn()\n");
		fflush (stdout);
		n = Readn (socketfd[K], ((char*)(buflettura[K]))+(H*nread), nread);
		/* n = Readn (socketfd[K], buflettura[K], nread);*/
		if (n != nread)
		{
			printf ("Readn() failed \n ");
			fflush(stdout);
	    		return (0);
		}
	}
  	sumlettura[K]=sommavet(buflettura[K],LENVETLETTURA);
	printf("connessione %d somma ricevuta %u somma calcolata %u\n", K, sum[K], sumlettura[K] );
	/*
	stampavet(buflettura[K],LENVETLETTURA);
	fflush(stdout);
	*/
	if(sumlettura[K]!=sum[K])
	{
		printf ("\033[33;35;1m somma errata: errore in trasmissione dati\n \033[0m ");
		fflush(stdout);
	    	return (0);
	}
  }	

  /* chiusura */
  for(K=0;K<2;K++)
	Close(socketfd[K]);

  if( ! Close_TCP_Session_Module(NULL) )
  {
	printf ("\033[33;35;1m Close_TCP_Session_Module() failed\n \033[0m ");
	fflush(stdout);
	exit(1);
  }

  printf ("TUTTO OK\n");
  return(0);
}

