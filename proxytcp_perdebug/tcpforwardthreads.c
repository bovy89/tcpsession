//mie aggiunte INIZIO
#include <stdio.h>
#include <string.h> //per le funzioni di parsing sulle stringhe

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>

#include <netdb.h>// per la gethostbyname
#include <stdlib.h> //per la malloc
#include <signal.h>
#include <sys/wait.h>

#include <sys/time.h>   // per gethostbyname e struct timeval
#include <unistd.h>

//mie aggiunte FINE
#include <sys/select.h>
#include <sys/errno.h>

#include <pthread.h>

// #define OUTPUT_VERBOSE 1

typedef void* (*start_routine)(void *);

typedef struct {
	int			browserfd;
	char		string_IP_remote_address[254];
	short int	port_number_remote;
} ParametriThreadPrincipali;

typedef struct {
	int FromToFds[2];
	char label[10];
} ParametriThread;

pthread_mutex_t mutex;
int fine=0;

void segnalafine(void)
{
	pthread_mutex_lock(&mutex);
	fine=1;
	pthread_mutex_unlock(&mutex);
}

int checkfine(void)
{
	int finire;
	pthread_mutex_lock(&mutex);
	if(fine) finire=1;
	else     finire=0; 
	pthread_mutex_unlock(&mutex);
	return(finire);
}

/* This function slays dead children, otherwise they become zombies.
 * Unix signals do not queue correctly, so we check in a nonblocking
 * way (WNOHANG) for all our dead children */

void sig_child (int signo)
{
  pid_t pid;
  while ((pid = waitpid (-1, NULL, WNOHANG)) > 0) { ; }
  if ((signal (SIGCHLD, sig_child)) == SIG_ERR)   { ; }
  return;
}

ssize_t  writen (int fd, const void *buf, size_t n)
{   
	size_t	nleft;     
	ssize_t  nwritten;  
	char	*ptr;

	ptr = (void *)buf;
	nleft = n;
	while (nleft > 0) {
		if(checkfine())
			return(n-nleft);
		if ( (nwritten = send(fd, ptr, nleft, MSG_NOSIGNAL)) < 0) {
			if (errno == EINTR)	nwritten = 0;   /* and call write() again*/
			else			return(-1);       /* error */
		}
		nleft -= nwritten;
		ptr   += nwritten;
	}
	return(n);
}

int forward(ParametriThread *pParam)
{
	int nread, nwritten;
	unsigned long int TOTnread=0;
#define MAXSIZE 50000
	char buf[MAXSIZE];
	int fromfd, tofd;
	char label[10];

	fromfd=pParam->FromToFds[0];
	tofd=pParam->FromToFds[1];
	strcpy(label,pParam->label);
	
	TOTnread=0;
	while(1)
	{
		if(checkfine())
		{
			printf("%s: errore: l'altro thread ha ordinato la chiusura\nTermino dopo avere spedito in tot %ld bytes\n", label, TOTnread);
			fflush(stdout);
			return(0);
		}
		
		do {
			nread = read(fromfd,buf,MAXSIZE);
		} while ( (!checkfine()) && (nread<0)&&(errno==EINTR) );
		if(nread==0) {
			segnalafine();
			printf("%s: il source ha chiuso la connessione: OK\ndopo avere spedito %ld bytes\n", label, TOTnread);
			fflush(stdout);
			return(0);
		}
		else if(nread<0) {
			segnalafine();
			printf("%s: errore: il source ha chiuso MALE\ndopo avere spedito in tot %ld bytes\n", label, TOTnread);
			fflush(stdout);
			return(0);
		}

		TOTnread+=nread;
#ifdef OUTPUT_VERBOSE
		printf("%s: il source ha spedito %d bytes:\n\"", label, nread);
#endif
		nwritten=writen(tofd,buf,nread);
		if(nwritten!=nread) {
			segnalafine();
			printf("%s: errore: spedizione TCP fallita, esco\ndopo avere spedito %ld bytes\n", label, TOTnread);
			fflush(stdout);
			return(0);
		}
	}
}

int setup_connection(int *pserverfd, char *string_IP_remote_address, int port_number_remote)
{
	struct sockaddr_in Local, Serv;
	int ris;

	/* get a stream socket */
	// printf ("socket()\n");
	*pserverfd = socket(AF_INET, SOCK_STREAM, 0);
	if (*pserverfd<0) {
		printf ("socket() failed, Err: %d \"%s\"\n", errno,strerror(errno));
		fflush(stdout);
		return(0);
	}

	/* name the socket */
	memset ( &Local, 0, sizeof(Local) );
	Local.sin_family		=	AF_INET;
	/* indicando INADDR_ANY viene collegato il socket all'indirizzo locale IP     */
	/* dell'interaccia di rete che verrà utilizzata per inoltrare i dati          */
	Local.sin_addr.s_addr	=	htonl(INADDR_ANY);         /* wildcard */
	Local.sin_port			=	htons(0);

	// printf ("bind ()\n");
	ris = bind(*pserverfd, (struct sockaddr*) &Local, sizeof(Local));
	if (ris<0)  {
	    printf ("bind() failed, Err: %d \"%s\"\n",errno,strerror(errno));
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
		ris = connect(*pserverfd, (struct sockaddr*) &Serv, sizeof(Serv));
	} while( (ris<0)&&(errno==EINTR));

	if (ris<0)  {
		printf ("connect() failed, Err: %d \"%s\"\n",errno,strerror(errno));
		fflush(stdout);
		return(0);
	}
	return(1);

}




void *forward_bidirezionale(	void *p )
{

	int ris;
	int serverfd;
	pthread_t thid_BTS;
	ParametriThread BrowserToServer, ServerToBrowser;
	ParametriThreadPrincipali *pPTP;

	int browserfd;
	char string_IP_remote_address[254];
	short int port_number_remote;

	pPTP=(ParametriThreadPrincipali*)p;
	browserfd=pPTP->browserfd;
	strcpy(string_IP_remote_address,pPTP->string_IP_remote_address);
	port_number_remote=pPTP->port_number_remote;

	free(pPTP);
	pPTP=NULL;
	p=NULL;

	pthread_mutex_init(&mutex, NULL);
	fine=0;

	ris= setup_connection(&serverfd, string_IP_remote_address, port_number_remote);
	if (!ris)  {
		printf ("impossibile connettersi al server %s %d\nTERMINO!!\n", 
			string_IP_remote_address, port_number_remote );
		fflush(stdout);
		pthread_exit(NULL);
		return(NULL);
	}

	BrowserToServer.FromToFds[0]=browserfd;
	BrowserToServer.FromToFds[1]=serverfd;
	strcpy(BrowserToServer.label,"BTS");
	ServerToBrowser.FromToFds[0]=serverfd;
	ServerToBrowser.FromToFds[1]=browserfd;
	strcpy(ServerToBrowser.label,"STB");

	ris=pthread_create(&thid_BTS, NULL, (start_routine) &forward, (void *) &BrowserToServer);
	if(ris!=0)
	{
		close(browserfd);
		close(serverfd);
		fprintf (stdout, "pthread_create( Browser To Server ) failed, Err: %d \"%s\"\n", errno,strerror(errno));
		pthread_exit(NULL);
		return(NULL);
	}

	forward(&ServerToBrowser);
	pthread_mutex_lock(&mutex);
	fine=1;
	pthread_mutex_unlock(&mutex);
	close(browserfd);
	close(serverfd);

	/*
	printf("%s: il processo principale terminera' dopo forward(...) tra 20 secondi\n", ServerToBrowser.label);
	fflush(stdout);
	sleep(20);
	*/

	printf("%s: il processo principale termina dopo forward(...)\n", ServerToBrowser.label);
	fflush(stdout);
	pthread_exit(NULL);
	return(NULL);
}


int setup_socket_listening(int *plistenfd, int numero_porta_locale)
{
	int ris, OptVal;
	struct sockaddr_in Local;
	
	// printf ("socket()\n");
	*plistenfd = socket(AF_INET, SOCK_STREAM, 0);
	if (*plistenfd < 0)
	{	
		printf ("socket() failed, Err: %d \"%s\"\n", errno,strerror(errno));
		exit(1);
	}

	/* avoid EADDRINUSE error on bind() */
	OptVal = 1;
	// printf ("setsockopt()\n");
	ris = setsockopt(*plistenfd, SOL_SOCKET, SO_REUSEADDR, (char *)&OptVal, sizeof(OptVal));
	if (ris<0)
	{	
		printf ("setsockopt() SO_REUSEADDR failed, Err: %d \"%s\"\n", errno,strerror(errno));
		exit(1);
	}

	/*name the socket */
	memset ( &Local, 0, sizeof(Local) );
	Local.sin_family= AF_INET;
	// specifico l'indirizzo IP attraverso cui voglio ricevere la connessione
	Local.sin_addr.s_addr = htonl(INADDR_ANY);
	Local.sin_port	      = htons(numero_porta_locale);

	// printf ("bind()\n");
	ris = bind(*plistenfd, (struct sockaddr*) &Local, sizeof(Local));
	if (ris<0)
	{	
		printf ("bind() failed, Err: %d \"%s\"\n",errno,strerror(errno));
		fflush(stderr);
		exit(1);
	}

	/* enable accepting of connection  */
	// printf ("listen()\n");
	ris = listen(*plistenfd, 100 );
	if (ris<0)
	{	
		printf ("listen() failed, Err: %d \"%s\"\n",errno,strerror(errno));
		exit(1);
	}
	return(1);
}

/* usage function */ 
void usage(int numero_porta_locale,
		   int numero_porta_remota, char *string_IP_remote_address)
{
	printf("Specificare tre parametri\n");
	fflush(stdout);
	printf("usage: tcpforward.exe numero_porta_tcp_locale[%d] numero_porta_tcp_remota[%d] IP_remoto[%s]\n",
			numero_porta_locale, numero_porta_remota, string_IP_remote_address);
}


int main(int argc, char *argv[])
{

	#define LOCAL_TCP_PORT 7000	//Porta di ascolto del Proxy Ritardatore
	#define REMOTE_TCP_PORT 7000	//Porta di ascolto del Proxy Ritardatore

	char string_IP_remote_address[254]="137.204.72.183";
	int port_number_remote=REMOTE_TCP_PORT;
	int numero_porta_locale=LOCAL_TCP_PORT;
	int browserfd, listenfd;
	int ris, myerrno;
	fd_set fdR;
	unsigned int len;
	struct sockaddr_in Cli;
	pthread_attr_t attr;
	ParametriThreadPrincipali *p;
	pthread_t pt;

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);

	if ( argc !=4) {	  
		usage(numero_porta_locale, port_number_remote, string_IP_remote_address);
		exit(1);
	}
	else {
		numero_porta_locale = atoi(argv[1]);
		port_number_remote = atoi(argv[2]);
		strcpy(string_IP_remote_address,argv[3]);

		printf("porta locale %d    porta remota %d     indirizzo remoto %s\n",
				numero_porta_locale, port_number_remote, string_IP_remote_address);
		fflush(stdout);
	}  
	
	// Set up signal handlers 
	if ((signal (SIGCHLD, sig_child)) == SIG_ERR)
	{
		printf ("signal(SIGCHLD, sig_child)  FALLITA:  Err: %d \"%s\"\n", errno,strerror(errno));
		return 0;
	}

	printf("proxy TCP: premere <ENTER> per interrompere\n");
	fflush(stdout);

	ris=setup_socket_listening(&listenfd, numero_porta_locale);
	if (ris<0)
	{	
		printf ("setup_socket_listening() failed\n");
		exit(1);
	}
	
	FD_ZERO(&fdR);
	while (1)
	{
		do {
			FD_SET(0,&fdR);
			FD_SET(listenfd,&fdR);
			ris=select(listenfd+1,&fdR,NULL,NULL,NULL);
			myerrno=errno;
		} while( (ris<0)&&(myerrno==EINTR) );
		if(ris<0)
		{
			perror("select failed: ");
			exit(1);
		}

		if(FD_ISSET(0,&fdR))
		{
			printf("interruzione voluta dall'utente: TERMINO\n");
			fflush(stdout);
			exit(2);
		}

		if(FD_ISSET(listenfd,&fdR))
		{
			/* wait for connection request */
			printf ("accept()\n");
			do {
				len=sizeof(Cli);
				memset(&Cli,0,sizeof(struct sockaddr_in) );
				browserfd = accept(listenfd, (struct sockaddr*) &Cli, &len);
			} while ((browserfd<0)&&(errno==EINTR)) ;

			if (browserfd<0)
			{	printf ("accept() failed, Err: %d \"%s\"\n",errno,strerror(errno));
				exit(1);
			}

			/* alloco la struttura in cui passare i parametri */
			p=malloc(sizeof(ParametriThreadPrincipali));
			if(p==NULL) {
				perror("malloc failed: ");
				exit(1);
			}
			p->browserfd=browserfd;
			strcpy(p->string_IP_remote_address,string_IP_remote_address);
			p->port_number_remote=port_number_remote;

			/* printf("Creating thread DETACHED %d\n", t); */
			ris = pthread_create (&pt, &attr, forward_bidirezionale, p );
			if (ris){
				printf("ERROR; return code from pthread_create() is %d\n",ris);
				exit(-1);
			}
			else
				printf("Created thread DETACHED ID %d\n", (int)pt );
		}

	}
	return(0);
}







