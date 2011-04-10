/* Servmedioalto.c */



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



/* da eliminare fuori da cygwin
#define MSG_DONTWAIT 0x40
*/


#define STEPSIZE   55555L
#define NUMCICLITRAATTESA 5

/* #define VERBOSE */

int not_empty( int maxfd, fd_set *pfdR, fd_set *pfdW)
{
	int i;
	for(i=0;i<=maxfd;i++)
	{
		if( FD_ISSET(i,pfdR) ) {
			/* printf("FD_ISSET %d in fdR",i); fflush(stdout); */
			return(1);
		}
		if( FD_ISSET(i,pfdW) ) {
			/* printf("FD_ISSET %d in fdW",i); fflush(stdout);  */
			return(1);
		}
	}
	return(0);
}


void usage (void)
{
    printf ("usage: ./Servmedioalto.exe LOCAL_PORT_NUMBER\n");
    exit (1);
}

int main (int argc, char *argv[])
{

    short int local_port_number;
    int listenfd[2], connectedfd[3], to_close[3], status[3];
	int Vnwritten[3], Vnread[3];
	int socketfd, nreadTCP, nwriteTCP, statusTCP=0;
    struct sockaddr_in Cli;
	int ris, i, nwrite, nread, n;
	int noattesa;

	unsigned int len;
	char minibuf[3][10], minibufclient[10];
	char *bufclient;
	char *bufsend[3], *bufrecv[3];
	fd_set fdR, fdW, temp_fdR, temp_fdW;
	struct timeval tv;
	int myerrno, maxfd;
	int nconnected=0, num;

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

	bufclient=malloc(MAXSIZE);
	if(bufclient==NULL) { perror("malloc failed: "); exit(1);}
	for(i=0;i<3;i++)
	{
		bufsend[i]=malloc(MAXSIZE);
		if(bufsend[i]==NULL) { perror("malloc failed: "); exit(1);}
		bufrecv[i]=calloc(1,MAXSIZE);
		if(bufrecv[i]==NULL) { perror("malloc failed: "); exit(1);}
	}
	init_random();
	for(i=0;i<3;i++)
		inizializza(bufsend[i],MAXSIZE);
	
	if( ! Init_TCP_Session_Module(NULL) )
	{
		printf ("Init_TCP_Session_Module() failed\n");
		fflush(stdout);
		exit(1);
	}

	ris=TCPS_setup_socket_listening( &(listenfd[0]), local_port_number);
	if (!ris)
	{	
		printf ("setup_socket_listening() failed\n");
		exit(1);
	}

	ris=TCP_setup_socket_listening( &(listenfd[1]), local_port_number+1);
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
#ifdef VERBOSE
	printf("socket TCP client %d\n", socketfd );
	fflush(stdout);
#endif

	FD_ZERO(&fdR);
	FD_ZERO(&fdW);
	FD_SET(listenfd[0],&fdR);
	FD_SET(listenfd[1],&fdR);
	FD_SET(socketfd,&fdW);
	if(socketfd>listenfd[0])	maxfd=socketfd;
	else				maxfd=listenfd[0];
	if(listenfd[1]>maxfd)		maxfd=listenfd[1];

#ifdef VERBOSE
	printf("socket listening: %d %d\n", listenfd[0], listenfd[1] );
	fflush(stdout);
#endif

	noattesa=NUMCICLITRAATTESA;

	do
	{
		if(noattesa==0)
		{
#ifdef VERBOSE
			printf("sleep(1) \n");
			fflush(stdout);
#endif
			sleep(1);
			noattesa=NUMCICLITRAATTESA;
		}
		else
		{
#ifdef VERBOSE
			printf("no sleep() \n");
			fflush(stdout);
#endif
			noattesa--;
		}

		do {
			temp_fdR=fdR;
			temp_fdW=fdW;
			tv.tv_sec=20;
			tv.tv_usec=0;

			/*
			printf("Select ...  ");
			fflush(stdout);
            		*/

			ris=Select(maxfd+1, &temp_fdR, &temp_fdW, NULL, &tv );
			myerrno=errno;

			/*
       		printf(" fine Select\n");
			fflush(stdout);
            		*/

		} while( (ris<0)&&(myerrno==EINTR) );

		if(ris==0)
		{
			printf("Select timeout: repeat\n");
			fflush(stdout);
		}
		else if(ris<0)
		{
			perror("Select() failed: ");
			printf("TERMINO !!!!!\n");
			fflush(stdout);
			exit(1);
		}
		else
		{
			/* ris>0 */

			/* accetto al massimo due "connessioni TCP con Sessione" ed una "normale" */
			for(i=0;i<2;i++)
			{
				if( listenfd[i] >= 0 )
				{
					if( FD_ISSET(listenfd[i], &temp_fdR) )
					{
						len=sizeof(Cli);
						ris=Accept (listenfd[i], (struct sockaddr *) &Cli, &len);
						myerrno=errno;
						if(ris<0)
						{
							if(myerrno!=EINTR)
							{
								printf ("Accept(%d) failed, Err: %d \"%s\"\nTERMINO!!!",
											listenfd[i], myerrno, strerror (myerrno));
								exit (1);
							}
						}
						else
						{
							connectedfd[nconnected]=ris;
							to_close[nconnected]=ris;
							status[nconnected]=0; 
							FD_SET(connectedfd[nconnected],&fdR);
							if(connectedfd[nconnected]>maxfd)	maxfd=connectedfd[nconnected];

							printf("Accettata connessione %d dal socket listening %d\n",
									connectedfd[nconnected], listenfd[i] );

							nconnected++;
							if((nconnected==3)||(i==1))
							{
								FD_CLR(listenfd[i], &temp_fdR);
								FD_CLR(listenfd[i], &fdR);
								/* attenzione qui, a chi deve gestire le riconnessioni */
								printf("Chiuso socket listening %d\n", listenfd[i] );

								Close(listenfd[i]);
								listenfd[i]=-1;
	
							}
		
						}
					}
				}
			}

			/* le connessioni "lato server" */
			for(i=0;i<nconnected;i++)
			{
				if( connectedfd[i] >= 0 )
				{
					if( FD_ISSET(connectedfd[i], &temp_fdR) )
					{
						/* disponibile in lettura */
						ris=AvailableBytes(connectedfd[i],&num);
						if(ris==0)
						{
							printf("AvailableBytes from server %d failed\nTERMINO!!!", connectedfd[i]);
							fflush(stdout);
							exit(1);
						}
						if(num==0)
						{
							/* nessun byte disponibile -> e' stata chiusa la connessione
							 Close(connectedfd[i]);
               				*/
							
							FD_CLR(connectedfd[i], &fdR);
							FD_CLR(connectedfd[i], &fdW);
							printf("Connessione %d Chiusa inaspettatamente\nTERMINO!!!!\n", connectedfd[i]);
							fflush(stdout);
							connectedfd[i]=-1;
							exit(1);
						}
						else
						{
							/* qualcosa da leggere */
							if(status[i]==0)
							{
								/* leggo 10 bytes, in modo bloccante */
								nread=10;
								n = Readn (connectedfd[i], minibuf[i], nread);
								if (n != nread)
								{
									printf ("Readn() failed \nTermino\n");
									fflush(stdout);
									exit(1);
								}
								else
								{
									FD_SET(connectedfd[i],&fdW);
									Vnwritten[i]=0;
									status[i]=1;
									Vnread[i]=0;
#ifdef VERBOSE
									printf("Read %d bytes using %d\n", n, connectedfd[i] );
									fflush(stdout);
#endif
								}
							}
							else
							{
								/* status[i]==1   seconda lettura */
								n = Read (connectedfd[i], bufrecv[i]+(Vnread[i]), MAXSIZE-Vnread[i]>STEPSIZE?STEPSIZE:MAXSIZE-Vnread[i] );
								myerrno=errno;
								if(n<0)
								{
									if(myerrno!=EINTR)
									{
										printf ("Read() failed \nTermino\n");
										fflush(stdout);
										exit(1);
									}
								}
								else
								{
#ifdef VERBOSE
									printf("read %d bytes from connection %d\n", n, connectedfd[i] );
									fflush(stdout);
#endif
									Vnread[i]+=n;

									if(Vnread[i]==MAXSIZE)
									{

										printf("La Connessione %d ha finito correttamente il proprio lavoro\n", connectedfd[i] );
										fflush(stdout);

										FD_CLR(connectedfd[i], &fdR);
										FD_CLR(connectedfd[i], &temp_fdR);
										FD_CLR(connectedfd[i], &fdW);
										FD_CLR(connectedfd[i], &temp_fdW);
										connectedfd[i]=-1;
									}
								}
							}
						}
					}
				}
				if( connectedfd[i] >= 0 )
				{
					if( FD_ISSET(connectedfd[i], &temp_fdW) )
					{
						nwrite=MAXSIZE;
						do {
							n = Send (	connectedfd[i], bufsend[i]+(Vnwritten[i]),
										nwrite-Vnwritten[i]>STEPSIZE?STEPSIZE:nwrite-Vnwritten[i],
										MSG_NOSIGNAL|MSG_DONTWAIT
										);
							myerrno=errno;
						} while ( (n<0)&&(myerrno==EINTR) );
						if ( n < 0 )
						{
							printf ("Send() failed \nTermino\n");
							fflush(stdout);
							exit(1);
						}
						else
						{
#ifdef VERBOSE
							printf("Sent %d bytes using server %d\n", n, connectedfd[i]);
							fflush(stdout);
#endif
							Vnwritten[i]+=n;
							if(Vnwritten[i]==nwrite)
							{
#ifdef VERBOSE
								printf("Una Connessione %d ha finito di inviare\n", connectedfd[i] );
								fflush(stdout);
#endif
								FD_CLR(connectedfd[i], &fdW);
								FD_CLR(connectedfd[i], &temp_fdW);

							}
						}
					}
				}
			}

			if(socketfd>=0)  /* funge da client locale, deve scrivere poi leggere poi scrivere */
			{
				if( FD_ISSET(socketfd, &temp_fdW) )
				{
					if(statusTCP==0)
					{
						nwrite = 10;
#ifdef VERBOSE
						printf ("Writen() %d bytes su TCP client locale %d\n", nwrite, socketfd );
						fflush (stdout);
#endif
						n = Writen (socketfd, minibufclient, nwrite );
						if (n != nwrite)
						{
							printf ("Writen() using %d TCP client failed \nTERMINO\n", socketfd);
							fflush(stdout);
							exit(2);
						}
						/* non devo piu' spedire */
						FD_CLR(socketfd, &fdW);
						FD_CLR(socketfd, &temp_fdW);
						/* devo ricevere */
						FD_SET(socketfd, &fdR);
						nreadTCP=0;

#ifdef VERBOSE
						printf ("fine Writen() %d bytes su TCP client locale %d \n", n, socketfd );
						fflush (stdout);
#endif
					}
					else  /* statusTCP==1 */
					{
						n = Send (socketfd, bufclient+nwriteTCP, MAXSIZE-nwriteTCP, MSG_NOSIGNAL|MSG_DONTWAIT );
						myerrno=errno;
						if ( n < 0 )
						{
							if(myerrno!=EINTR)
							{
								printf ("Send() using %d TCP client failed \nTermino\n",socketfd);
								fflush(stdout);
								exit(1);
							}
						}
						else if ( n == 0 )
						{
							printf ("Send() using %d TCP client failed: Sent 0 bytes: strano\nTermino\n", socketfd);
							fflush(stdout);
							exit(1);
						}
						else /* n>0 */
						{
#ifdef VERBOSE
							printf("Sent %d bytes using %d TCP client\n", n, socketfd );
							fflush(stdout);
#endif
							nwriteTCP+=n;
							if(nwriteTCP==MAXSIZE)
							{
								printf("la Connessione %d TCP client ha finito il proprio lavoro\n", socketfd );
								fflush(stdout);

								FD_CLR(socketfd, &fdR);
								FD_CLR(socketfd, &fdW);
								FD_CLR(socketfd, &temp_fdR);
								FD_CLR(socketfd, &temp_fdW);

							}
						}

					}
				}
			}

			/* la connessione TCP "client" dopo avere spedito vuole ricevere */
			if(socketfd>=0)
			{
				if( FD_ISSET(socketfd, &temp_fdR) )
				{
					/* disponibile in lettura */
					ris=AvailableBytes(socketfd,&num);
					if(ris==0)
					{
						printf("AvailableBytes from TCP client %d failed\nTERMINO!!!", socketfd );
						fflush(stdout);
						exit(1);
					}
					if(num==0)
					{
						/* nessun byte disponibile -> e' stata chiusa la connessione */
						Close(socketfd);
						FD_CLR(socketfd, &fdR);
						FD_CLR(socketfd, &fdW);
						FD_CLR(socketfd, &temp_fdR);
						FD_CLR(socketfd, &temp_fdW);
						printf("Connessione socketfd %d Chiusa troppo presto\nTERMINO!!!", socketfd );
						fflush(stdout);
						socketfd=-1;
						exit(1);
					}
					else
					{
						/* qualcosa da leggere, lettura non bloccante */
						do {
							n = Read (socketfd, bufclient+nreadTCP, MAXSIZE-nreadTCP );
							myerrno=errno;
						} while ( (n<0)&&(myerrno==EINTR) );
						if ( n < 0 )
						{
							printf ("Read() failed \nTermino\n");
							fflush(stdout);
							exit(1);
						}
						else
						{
							nreadTCP+=n;

#ifdef VERBOSE
							printf("read %d bytes from connection TCP client %d\n", n, socketfd );
							fflush(stdout);
#endif
							if(nreadTCP==MAXSIZE)
							{
#ifdef VERBOSE
								printf("La Connessione TCP client ha finito di leggere\n");
								fflush(stdout);
#endif
								/* abilito anche la scrittura */
								FD_SET(socketfd, &fdW);
								statusTCP=1;
								nwriteTCP=0;
								FD_CLR(socketfd, &temp_fdR);
								FD_CLR(socketfd, &temp_fdW);

							}
						}
					}
				}
			}



		}  /* fine ris>0  */

	}  while( not_empty( maxfd, &fdR,&fdW) );
	
#ifdef VERBOSE
    printf ("Fine main\n");
	fflush(stdout);
#endif
    /* printf ("sleep 10 secs\n");
	fflush(stdout);
	sleep(10);
    */

    printf ("Close varie\n");
	fflush(stdout);

	if( Close(socketfd) != 0 )
		perror("Close failed\n");
	for(i=0;i<nconnected;i++)
		if(Close(to_close[i]) != 0 )
			perror("Close failed\n");

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

	/* controllo quel che ho ricevuto */
	for(i=0;i<3;i++)
	{
		unsigned int j;

		for(j=0;j<MAXSIZE;j++) 
			if(bufrecv[i][j] != bufsend[i][j])
			{
			printf ("\033[33;35;1m bufrecv[%d][%d] %d DIVERSO da bufsend[%d][%d] %d  : errore in trasmissione dati\n \033[0m ",
					i,j, bufrecv[i][j], i,j, bufsend[i][j]  );
			fflush(stdout);
			return (0);
			}
	}
    return (0);
}

