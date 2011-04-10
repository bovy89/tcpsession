/* Servmediobasso.c */



#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <signal.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include "TCP_Session.h"
#include "Util.h"

typedef void* (*ptr_thread_routine)(void *);

int thread_For_Read_Write (int *psocketfd)
{
	int newsocketfd;
	int n, nread, nwrite;
	char bufscrittura[LENVETLETTURA];
	char buflettura[LENVETSCRITTURA];
	int *ptr;
	unsigned int sum;

	newsocketfd=*psocketfd;
	free(psocketfd);
	psocketfd=NULL;

	/* alloco la struttura in cui restituire il risultato */
	ptr=malloc(sizeof(int));
	if(ptr==NULL) {
		perror("malloc failed: ");
		fflush(stderr);
		pthread_exit (NULL);
		return(0);
	}

	sum=inizializza(bufscrittura,LENVETLETTURA);
	printf("inizializza somma vettore %d\n", sum );
	
	/*
	stampavet(bufscrittura,LENVETLETTURA);
	fflush (stdout);
	*/
	
	sum=htonl(sum);
	/* scrittura della somma del vettore */
	nwrite = sizeof(int);
	printf ("Writen()\n");
	fflush (stdout);
	n = Writen (newsocketfd, (char*)&sum, nwrite );
	if (n != nwrite)
	{
		perror ("Writen() sum failed \n");
		fflush(stdout);
		*ptr=0;
		pthread_exit ( ptr );  /* valore restituito dal thread */
		return (0);
	}
	
	/* lettura */
	nread=LENVETSCRITTURA;
	printf ("Readn()\n");
	fflush (stdout);
	n = Readn (newsocketfd, buflettura, nread);
	if (n != nread)
	{
		perror ("Readn() failed \n");
		fflush(stdout);
		*ptr=0;
		pthread_exit ( ptr );  /* valore restituito dal thread */
		return (0);
	}
	
	/* scrittura */
	nwrite = LENVETLETTURA;
	printf ("Writen()\n");
	fflush (stdout);
	n = Writen (newsocketfd, bufscrittura, nwrite );
	if (n != nwrite)
	{
		perror ("Writen() failed \n");
		fflush(stdout);
		*ptr=0;
		pthread_exit ( ptr );  /* valore restituito dal thread */
		return (0);
	}

	/* chiusura */
	printf ("terminazione corretta del pthread\n");
	fflush (stdout);
	*ptr=1;
	pthread_exit ( ptr );  /* valore restituito dal thread */
	return (1);
}


void usage (void)
{
    printf ("usage: ./Servbasso.exe LOCAL_PORT_NUMBER\n");
    exit (1);
}

int main (int argc, char *argv[])
{
    	int tuttoOK=1;
    	short int local_port_number;
    	int listenfd[2], connectedfd[3], socketfd, ris, i, nwrite, nread, n;
    	struct sockaddr_in Cli;
	int *intptr;
	pthread_t thid[10];
	/* pthread_attr_t attr;*/
	unsigned int len;
	char buflettura[LENVETLETTURA];
	char bufscrittura[LENVETSCRITTURA];
	int sum, sumlettura;

	if    ( (sizeof(int)!=sizeof(long int)) ||  (sizeof(int)!=4)  )
	{
		printf ("dimensione di int e/o long int != 4  -> TERMINO\n");
		fflush(stdout);
		exit(1);
	}

    	if (argc != 2)
	{
		printf ("necessario 1 parametro\n");
		usage ();
		exit (1);
    	}
    	else
    	{
		local_port_number = atoi (argv[1]);
    	}
	
	init_random();

	if( ! Init_TCP_Session_Module(NULL) )
	{
		printf ("Init_TCP_Session_Module() failed\n");
		fflush(stdout);
		exit(1);
	}

	ris=TCPS_setup_socket_listening( listenfd, local_port_number);
	if (!ris)
	{	
		printf ("setup_socket_listening() failed\n");
		exit(1);
	}
	ris=TCP_setup_socket_listening( listenfd+1, local_port_number+1);
	if (!ris)
	{	
		printf ("setup_socket_listening() failed\n");
		exit(1);
	}

	ris= TCP_setup_connection(&socketfd, "127.0.0.1", local_port_number+1);
	if (!ris)  {
		printf ("impossibile connettersi al server: TERMINO!!\n" );
		fflush(stdout);
		exit(1);
	}

	/*
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
	*/

	for(i=0;i<3;i++)
	{
		intptr=malloc(sizeof(int));
		if(intptr==NULL) { perror("malloc failed: "); fflush(stderr); exit(1); }

		/* wait for connection request */
		memset (&Cli, 0, sizeof (Cli));
		len = sizeof (Cli);
		for(;;)
		{
			printf ("Accept()\n");
			if( (i==0)||(i==1) )
				*intptr = Accept (listenfd[0], (struct sockaddr *) &Cli, &len);
			else
				*intptr = Accept (listenfd[1], (struct sockaddr *) &Cli, &len);
			if (*intptr == SOCKET_ERROR)
			{
				if(errno==EINTR) continue;
				else
				{
					printf ("Accept() failed, Err: %d \"%s\"\n", errno,
					strerror (errno));
					exit (1);
				}
			}
			else
			{
				connectedfd[i]=*intptr;
				/*
				ris=pthread_create(&(thid[i]), &attr, (ptr_thread_routine) &thread_For_Read_Write, (void *) intptr );
				*/
				ris=pthread_create(&(thid[i]), NULL, (ptr_thread_routine) &thread_For_Read_Write, (void *) intptr );
				if(ris)
				{
					/*
					Close(*intptr);
					Close(listenfd[i]);
					free(intptr);
					*/
					fprintf (stdout, "pthread_create( For_Read_Write ) failed, Err: %d \"%s\"\n", errno,strerror(errno));
					exit(2);
				}
				else
				{
					printf ("pthread_create succeed\n");
					break;
				}
			} /* fine creazione threads */
		} /* fine for(;;) */
	} /* for(i=0;i<3;i++) */

	for(i=0;i<2;i++)
	{
		ris=pthread_join( thid[i] , (void*) &intptr );
		if(ris!=0){
			printf("pthread_join( %d) failed: error=%d\n", i, ris );
			exit(-1);
		}
		else
		{
			if(intptr==NULL)
			{
				printf("pthread failed: malloc error\n");
				fflush(stdout);
				tuttoOK=0;
				exit(-1);
			}
			else if(*intptr==0)
			{
				printf("pthread failed: network error\n");
				fflush(stdout);
				tuttoOK=0;
				exit(-1);
			}
			else
				printf("pthread ok\n");
			fflush(stdout);
		}
	}

	/* parte client del socket tcp normale */
	nread=sizeof(int);
	printf ("Readn() sum \n");
	fflush (stdout);
	n = Readn (socketfd, (char*) &(sum), nread);
	if (n != nread)
	{
		printf ("Readn() sum failed \n");
		fflush(stdout);
		return (0);
	}
	sum=ntohl(sum);

	/* scrittura sul socket tcp normale */
	nwrite = LENVETSCRITTURA;
	printf ("Writen()\n");
	fflush (stdout);
	n = Writen (socketfd, bufscrittura, nwrite );
	if (n != nwrite)
	{
		printf ("Writen() failed \n");
		fflush(stdout);
		return (0);
	}
	/* lettura dal socket tcp normale*/
	nread=LENVETLETTURA;
	n = Readn (socketfd, buflettura, nread);
	if (n != nread)
	{
		printf ("Readn() failed \n");
		fflush(stdout);
		return (0);
	}
	sumlettura=sommavet(buflettura,LENVETLETTURA);
	printf("somma spedita %d  somma calcolata %d\n", sum, sumlettura );
	fflush (stdout);
	/*
	stampavet(buflettura,LENVETLETTURA)
	fflush (stdout);
	*/
	if(sumlettura!=sum)
	{
		printf ("\033[33;35;1m somma del tcp errata: errore in trasmissione dati\n \033[0m ");
		fflush(stdout);
		return (0);
	}

	/* fine parte client del socket tcp normale */

	/* attesa thread che ha gestito il server con il socket tcp normale */
	ris=pthread_join( thid[2] , (void*) &intptr );
	if(ris!=0){
		printf("pthread_join( 2) failed: error=%d\n", ris );
		exit(-1);
	}
	else
	{
		if(intptr==NULL)
		{
			printf("pthread failed: malloc error\n");
			tuttoOK=0;
		}
		else if(*intptr==0)
		{
			printf("pthread failed: network error\n");
			tuttoOK=0;
		}
		else
			printf("pthread ok\n");
		fflush(stdout);
	}

	for(i=0;i<3;i++) {
		Close (connectedfd[i]);
	}
	for(i=0;i<2;i++) {
		Close (listenfd[i]);
	}
	Close (socketfd);

    	printf ("Fine main\n");
	if(tuttoOK==0)
	{
    		printf ("\033[33;35;1m C'e' stato un errore di comunicazione\n \033[0m \n");
		fflush(stdout);
		exit(1);
	}
	if( ! Close_TCP_Session_Module(NULL) )
	{
		printf ("\033[33;35;1m Close_TCP_Session_Module() failed\n \033[0m ");
		fflush(stdout);
		exit(1);
	}
	
    	printf ("Tutto OK\n");
	fflush(stdout);
	return (0);
}


