/* Climedioalto.c  */
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
{  printf ("usage: ./Climedioalto.exe REMOTE_IP_NUMBER REMOTE_PORT_NUMBER [lOCAL_IP]\n");
   exit(1);
}

int main(int argc, char *argv[])
{
  char string_remote_ip_address[100];
  char string_local_ip_address[100];
  short int remote_port_number;
  int socketfd[2], ris;
  int n, nread, nwrite, i;
  char buf[2][MAXSIZE];
  int K;

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

  ris= TCPS_setup_connection(&(socketfd[1]), string_remote_ip_address, remote_port_number);
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

	printf ("sleep(1)\n");
	fflush(stdout);
	sleep(1);

	/* scrittura */
	nwrite = 10;
	printf ("Writen()\n");
	fflush (stdout);
	n = Writen (socketfd[K], buf[K], nwrite );
	if (n != nwrite)
	{
		printf ("Writen() failed \n");
		fflush(stdout);
		return (0);
	}
  }

  for(i=0;i<10;i++)
  {
	printf ("sleep(1)\n");
	fflush(stdout);
	sleep(1);
	  
    for(K=1;K>=0;K--)
	{

		/* lettura */
		nread=MAXSIZE/10;
		printf ("Readn()\n");
		fflush (stdout);
		n = Readn (socketfd[K], buf[K]+(nread*i), nread);
		if (n != nread)
		{
			printf ("Readn() failed \n");
			fflush(stdout);
		    return (0);
		}
	}
  }

  for(i=0;i<10;i++)
  {
	/*
	printf ("sleep(1)\n");
	fflush(stdout);
	sleep(1);
	*/
	  
    for(K=1;K>=0;K--)
	{

		/* scrittura */
		nwrite=MAXSIZE/10;
		printf ("Writen()\n");
		fflush (stdout);
		n = Writen (socketfd[K], buf[K]+(nwrite*i), nwrite);
		if (n != nwrite)
		{
			printf ("Writen() failed \n");
			fflush(stdout);
		    return (0);
		}
	}
  }

  /* chiusura */
  for(K=0;K<2;K++)
	Close(socketfd[K]);

  printf ("call to CloseWait_TCP_Session_Module ...\n");
  fflush(stdout);
  n=CloseWait_TCP_Session_Module(NULL,NULL);
  switch(n) {
    case -1: printf ("CloseWait_TCP_Session_Module returns %d: error\n", n); break;
    case 0:  printf ("CloseWait_TCP_Session_Module returns %d: timeout expired\n", n); break;
    case 1:  printf ("CloseWait_TCP_Session_Module returns %d: ok\n", n); break;
    default: printf ("CloseWait_TCP_Session_Module returns %d: unknown error code\n", n); break;
  }
  fflush(stdout);

  return(0);
}

