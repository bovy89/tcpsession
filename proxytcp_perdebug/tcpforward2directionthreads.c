/* tcpforfard2directionthreads.c */

/* mie aggiunte INIZIO */
#include <stdio.h>
#include <string.h> /*per le funzioni di parsing sulle stringhe*/

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>

#include <netdb.h>	/* per la gethostbyname*/
#include <stdlib.h> /*per la malloc*/
#include <signal.h>
#include <sys/wait.h>

#include <sys/time.h>   /* per gethostbyname e struct timeval */
#include <unistd.h>

/*mie aggiunte FINE*/
#include <sys/select.h>
#include <sys/errno.h>

#include <pthread.h>

/* PER FARE OUTPUT per DEBUG DECOMMENTARE LA SEGUENTE RIGA */
/*#define OUTPUT_VERBOSE 1 */

#define P(X) do{printf(X);fflush(stdout);}while(0);

/*
#define USACOLORI
*/
#ifdef USACOLORI
#undef USACOLORI
#endif

#ifdef USACOLORI
#define DEFAULTCOLOR "\033[0m"
#define MARRONE  "\033[22;31m"
#define VERDE  "\033[22;32m"
/* SBAGLIATO #define ROSSO "\033[33;34;1m" */
#define GIALLO "\033[01;33m"
#define VIOLA "\033[33;35;2m"
#define ROSA "\033[33;35;1m"
#define BLU "\033[34;34;3m"
#define AZZURRO "\033[34;34;1m"
#define VERDECHIARO "\033[22;33m"

char *SEQUENZA_ESCAPE_COLORE(int caratteristica)
{
	switch(caratteristica)
	{
		case 1: return(MARRONE);
		case 2: return(ROSA);
		case 3: return(VERDE);
		case 4: return(AZZURRO);
	}
	return(DEFAULTCOLOR);
}

#define COLORE(X)        do { 								\
			int VARMACROcaratteristica;					\
			char *VARMACROptr;						\
			VARMACROcaratteristica=get_pthread_caratteristica();		\
			VARMACROptr=SEQUENZA_ESCAPE_COLORE(VARMACROcaratteristica);	\
			printf(VARMACROptr); 						\
			fflush(stdout);							\
			(X);								\
			fflush(stdout);							\
			printf(DEFAULTCOLOR);						\
			fflush(stdout);							\
			} while(0)

#else
#define COLORE(X)        X
#endif



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

void segnalasimulatimeout(void)
{
	pthread_mutex_lock(&mutex);
	fine=2;
	pthread_mutex_unlock(&mutex);
}

void segnalacontinua(void)
{
	pthread_mutex_lock(&mutex);
	fine=0;
	pthread_mutex_unlock(&mutex);
}

int remove_pthread_id(void);
int checkfine(void)
{
	int finire, fai_timeout=0;
	pthread_mutex_lock(&mutex);
	if(fine) finire=1;
	else     finire=0;
	if(fine==2) fai_timeout=1;
	pthread_mutex_unlock(&mutex);
	if(fai_timeout)
	{
		remove_pthread_id();
		pthread_exit(NULL);
	}
	return(finire);
}

struct StructNodoPthreadId;
typedef struct StructNodoPthreadId {
	pthread_t			ptid;
	int				caratteristica;
	struct StructNodoPthreadId	*next;
	
} NodoPthreadId;
NodoPthreadId *ListaPthread=NULL;
pthread_mutex_t mutexlista;

int get_pthread_caratteristica(void)
{
	NodoPthreadId *ptr;
	pthread_t pt;
	int caratteristica=0;

	pt=pthread_self();
	pthread_mutex_lock(&mutexlista);
	ptr=ListaPthread;
	while(ptr!=NULL)
	{
		if(ptr->ptid==pt)
		{
			caratteristica=ptr->caratteristica;
			ptr=NULL;
			break;
		}
		else
			ptr=ptr->next;
		
	}
	pthread_mutex_unlock(&mutexlista);
	return(caratteristica);
}
		
int add_pthread_id(pthread_t pt,int caratteristica)
{
	NodoPthreadId *ptr;

	pthread_mutex_lock(&mutexlista);
	ptr=ListaPthread;
	ListaPthread=(NodoPthreadId*)malloc(sizeof(NodoPthreadId));
	if(ListaPthread==NULL)
	{
		perror("malloc failed: ");
		exit(1);
	}
	ListaPthread->ptid=pt;
	ListaPthread->caratteristica=caratteristica;
	ListaPthread->next=ptr;
	ptr=NULL;
	pthread_mutex_unlock(&mutexlista);
	return(1);
}
	
int remove_pthread_id(void)
{
	NodoPthreadId* *pptr;
	pthread_t pt;

	pt=pthread_self();
	pthread_mutex_lock(&mutexlista);
	pptr=&ListaPthread;
	while(*pptr!=NULL)
	{
		if( (*pptr)->ptid==pt)
		{
			NodoPthreadId *pnexttemp;
			pnexttemp=(*pptr)->next;
			free( (*pptr) );
			*pptr=pnexttemp;
			pnexttemp=NULL;
			pthread_mutex_unlock(&mutexlista);
			return(1);
		}
		else
			pptr=&(  (*pptr)->next );
		
	}
	pthread_mutex_unlock(&mutexlista);
	return(0);
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

int  sendn (int fd, const void *buf, int n)
{   
	int nleft;     
	int nwritten;  
	char	*ptr;
	int myerrno;

	ptr = (char*)buf;
	nleft = n;
	while (nleft > 0) {

		/* 
		modifica 4 gennaio 05
		non faccio terminare l'altro pthread
		*/
		/*
		if(checkfine())
			return(n-nleft);
		*/
		
		nwritten = send(fd, ptr, nleft, MSG_NOSIGNAL);
		myerrno=errno;
		
		if ( nwritten < 0 ) {
			errno=myerrno;
			if (errno == EINTR)	nwritten = 0;   /* and call send() again*/
			else			
			{   
				COLORE( printf ("sendn->send failed, Err: %d \"%s\"\n", errno,strerror(errno)) );
				errno=myerrno;
				return(-1);       /* error */
			}
		}
#ifdef OUTPUT_VERBOSE
		else
		{   
			int i;
			
			COLORE( printf("\n SEND ") );
			for(i=0;i<nwritten;i++)
			{   
				COLORE( (printf("%d ", ptr[i] )) );
				fflush(stdout);
			}
			COLORE( printf("FINE SEND\n") );
		}
#endif 
		nleft -= nwritten;
		ptr   += nwritten;
	}
	return(n);
}


void shutdown_RDfromsender_WRtoreceiver(int fromfd, int tofd)
{
	int ris;
	ris=shutdown(fromfd,SHUT_RD);
	if(ris<0)
	{
		COLORE( printf ("shutdown(fromfd,SHUT_RD) failed, Err: %d \"%s\"\n", errno,strerror(errno)) );
		fflush(stdout);
	}
	ris=shutdown(tofd,SHUT_WR);
	if(ris<0)
	{
		COLORE( printf ("shutdown(fromfd,SHUT_RD) failed, Err: %d \"%s\"\n", errno,strerror(errno)) );
		fflush(stdout);
	}
}

int forward(ParametriThread *pParam)
{
	int nread, nwritten;
	unsigned long int TOTnread=0;
#define MAXSIZE 50000
	char buf[MAXSIZE];
	int fromfd, tofd;
	char label[10];
	int ris;

	fromfd=pParam->FromToFds[0];
	tofd=pParam->FromToFds[1];
	strcpy(label,pParam->label);
	
	TOTnread=0;
	while(1)
	{
		/*
		modifica 4 gennaio 05
		non faccio terminare l'altro pthread
		*/
		/*
		if(checkfine())
		{
			shutdown_RDfromsender_WRtoreceiver(fromd,tofd);
			COLORE( printf("%s: errore: l'altro thread ha ordinato la chiusura\nTermino dopo avere spedito in tot %ld bytes\n", label, TOTnread) );
			fflush(stdout);
			return(0);
		}
		*/
		
		do {
			nread = read(fromfd,buf,MAXSIZE);
		} while ( (!checkfine()) && (nread<0)&&(errno==EINTR) );
		if(nread==0) {
			ris=shutdown(tofd,SHUT_WR);
			if(ris<0)
			{
				COLORE( printf ("shutdown(fromfd,SHUT_RD) failed, Err: %d \"%s\"\n", errno,strerror(errno)) );
				fflush(stdout);
			}
			segnalafine();
			COLORE( printf("%s: il source ha chiuso la connessione: OK\ndopo avere spedito %ld bytes\n", label, TOTnread) );
			fflush(stdout);
			return(0);
		}
		else if(nread<0) {
			ris=shutdown(tofd,SHUT_WR);
			if(ris<0)
			{
				COLORE( printf ("shutdown(fromfd,SHUT_RD) failed, Err: %d \"%s\"\n", errno,strerror(errno)) );
				fflush(stdout);
			}
			segnalafine();
			COLORE( printf("%s: errore: il source ha chiuso MALE\ndopo avere spedito in tot %ld bytes\n", label, TOTnread) );
			fflush(stdout);
			return(0);
		}

		TOTnread+=nread;
#ifdef OUTPUT_VERBOSE
		COLORE( printf("%s: il source ha spedito %d bytes:\n\"", label, nread) );
#endif
		nwritten=sendn(tofd,buf,nread);
		if(nwritten!=nread) {
			ris=shutdown(fromfd,SHUT_RD);
			if(ris<0)
			{
				COLORE( printf ("shutdown(fromfd,SHUT_RD) failed, Err: %d \"%s\"\n", errno,strerror(errno)) );
				fflush(stdout);
			}
			segnalafine();
			COLORE( printf("%s: errore: spedizione TCP fallita, esco\ndopo avere spedito %ld bytes\n", label, TOTnread) );
			fflush(stdout);
			return(0);
		}
	}
}

int thread_forward(ParametriThread *pParam)
{
	int ris; 
	ris=forward(pParam);
	remove_pthread_id();
	pthread_exit(NULL);
	return(0);
}


int setup_connection(int *pserverfd, char *string_IP_remote_address, int port_number_remote)
{
	struct sockaddr_in Local, Serv;
	int ris;

	/* get a stream socket */
	/* printf ("socket()\n"); */
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

	/* printf ("bind ()\n"); */
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
	pthread_attr_t attr;
	int *intptr;

	pPTP=(ParametriThreadPrincipali*)p;
	browserfd=pPTP->browserfd;
	strcpy(string_IP_remote_address,pPTP->string_IP_remote_address);
	port_number_remote=pPTP->port_number_remote;

	free(pPTP);
	pPTP=NULL;
	p=NULL;

	pthread_mutex_init(&mutex, NULL);
	pthread_mutex_init(&mutexlista, NULL);
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
	fine=0;

	ris= setup_connection(&serverfd, string_IP_remote_address, port_number_remote);
	if (!ris)  {
		COLORE( printf ("impossibile connettersi al server %s %d\nTERMINO!!\n", 
			string_IP_remote_address, port_number_remote ) );
		fflush(stdout);

		/* inizio modifica Andrea e Annalisa 17/03/05 GRAZIE !!! */
		close(browserfd);
		/* fine modifica Andrea e Annalisa 17/03/05 GRAZIE !!! */
		
		remove_pthread_id();
		pthread_exit(NULL);
		return(NULL);
	}

	BrowserToServer.FromToFds[0]=browserfd;
	BrowserToServer.FromToFds[1]=serverfd;
	strcpy(BrowserToServer.label,"BTS");
	ServerToBrowser.FromToFds[0]=serverfd;
	ServerToBrowser.FromToFds[1]=browserfd;
	strcpy(ServerToBrowser.label,"STB");

	ris=pthread_create(&thid_BTS, &attr, (start_routine) &thread_forward, (void *) &BrowserToServer);
	if(ris!=0)
	{
		close(browserfd);
		close(serverfd);
		COLORE( printf("pthread_create( Browser To Server ) failed, Err: %d \"%s\"\n", errno,strerror(errno)) );
		remove_pthread_id();
		pthread_exit(NULL);
		return(NULL);
	}
	else
	{
		int caratteristica;
		caratteristica=get_pthread_caratteristica();
		add_pthread_id(thid_BTS,caratteristica+1);
	}

	forward(&ServerToBrowser);
	segnalafine();
	ris=pthread_join( thid_BTS , (void*) &intptr );
	if(ris!=0){
		COLORE( printf ("pthead_join()  failed, Err: %d \"%s\"\n", errno,strerror(errno)) );
		fflush(stdout);
	}
	
	/*
	close(browserfd);
	close(serverfd);
	*/
	
	/*
	printf("%s: il processo principale terminera' dopo forward(...) tra 20 secondi\n", ServerToBrowser.label);
	fflush(stdout);
	sleep(20);
	*/

	COLORE( printf("%s: il processo principale termina dopo forward(...)\n", ServerToBrowser.label) );
	fflush(stdout);
	remove_pthread_id();
	pthread_exit(NULL);
	return(NULL);
}


int setup_socket_listening(int *plistenfd, int numero_porta_locale)
{
	int ris, OptVal;
	struct sockaddr_in Local;
	
	/*printf ("socket()\n");*/
	*plistenfd = socket(AF_INET, SOCK_STREAM, 0);
	if (*plistenfd < 0)
	{	
		printf ("socket() failed, Err: %d \"%s\"\n", errno,strerror(errno));
		exit(1);
	}

	/* avoid EADDRINUSE error on bind() */
	OptVal = 1;
	/* printf ("setsockopt()\n");*/
	ris = setsockopt(*plistenfd, SOL_SOCKET, SO_REUSEADDR, (char *)&OptVal, sizeof(OptVal));
	if (ris<0)
	{	
		printf ("setsockopt() SO_REUSEADDR failed, Err: %d \"%s\"\n", errno,strerror(errno));
		exit(1);
	}

	/*name the socket */
	memset ( &Local, 0, sizeof(Local) );
	Local.sin_family= AF_INET;
	/* specifico l'indirizzo IP attraverso cui voglio ricevere la connessione */
	Local.sin_addr.s_addr = htonl(INADDR_ANY);
	Local.sin_port	      = htons(numero_porta_locale);

	/* printf ("bind()\n");*/
	ris = bind(*plistenfd, (struct sockaddr*) &Local, sizeof(Local));
	if (ris<0)
	{	
		printf ("bind() failed, Err: %d \"%s\"\n",errno,strerror(errno));
		fflush(stderr);
		exit(1);
	}

	/* enable accepting of connection  */
	/* printf ("listen()\n");*/
	ris = listen(*plistenfd, 100 );
	if (ris<0)
	{	
		printf ("listen() failed, Err: %d \"%s\"\n",errno,strerror(errno));
		exit(1);
	}
	return(1);
}

/* usage function */ 
void usage(int numero_porta_localeA, int numero_porta_remotaA, char *string_IP_remote_addressA,
		   int numero_porta_localeB, int numero_porta_remotaB, char *string_IP_remote_addressB)
{
	printf(	"l'eseguibile tcpforward2directionthreads e' usato come proxy TCP\n"
		"e permette di lanciare le connect in una sola direzione o in entrambe\n"
		"nel primo caso (monodirezione) servono solo i primi 3 parametri\n"
		"nel secondo caso (due direzioni) servono tutti i 6 parametri\n\n"
		);
	
	printf("Specificare tre o sei parametri\n");
	fflush(stdout);
	printf("usage: ./tcpforward2directionthreads.exe numero_porta_tcp_localeA[%d] numero_porta_tcp_remotaA[%d] IP_remotoA[%s] numero_porta_tcp_localeB[%d] numero_porta_tcp_remotaB[%d] IP_remotoB[%s]\n",
			numero_porta_localeA, numero_porta_remotaA, string_IP_remote_addressA,
			numero_porta_localeB, numero_porta_remotaB, string_IP_remote_addressB);
}


int main(int argc, char *argv[])
{

	#define LOCAL_TCP_PORTA 7000	/*Porta di ascolto del Proxy Ritardatore in una direzione*/
	#define REMOTE_TCP_PORTA 9000	/*Porta di ascolto del Processo Remoto in una direzione*/
	#define LOCAL_TCP_PORTB 7001	/*Porta di ascolto del Proxy Ritardatore nell'altra direzione*/
	#define REMOTE_TCP_PORTB 9001	/*Porta di ascolto del Processo Remoto  nell'altra direzione*/

	char string_IP_remote_addressA[254]="127.0.0.1";
	int port_number_remoteA=REMOTE_TCP_PORTA;
	int numero_porta_localeA=LOCAL_TCP_PORTA;
	char string_IP_remote_addressB[254]="127.0.0.1";
	int port_number_remoteB=REMOTE_TCP_PORTB;
	int numero_porta_localeB=LOCAL_TCP_PORTB;

	int browserfd, listenfdA, listenfdB, maxfd;
	int connect_from_2_directions;
	int ris, myerrno;
	fd_set fdR;
	unsigned int len;
	struct sockaddr_in Cli;
	pthread_attr_t attr;
	ParametriThreadPrincipali *p;
	pthread_t pt;

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);

	if ( (argc!=7)&&(argc!=4) ) {	  
		usage( numero_porta_localeA, port_number_remoteA, string_IP_remote_addressA,
			   numero_porta_localeB, port_number_remoteB, string_IP_remote_addressB);
		exit(1);
	}
	else {
		numero_porta_localeA = atoi(argv[1]);
		port_number_remoteA = atoi(argv[2]);
		strcpy(string_IP_remote_addressA,argv[3]);

		if (argc==7) {
			connect_from_2_directions=1;
			numero_porta_localeB = atoi(argv[4]);
			port_number_remoteB = atoi(argv[5]);
			strcpy(string_IP_remote_addressB,argv[6]);

			printf(
					"porta locale A %d    porta remota A %d     indirizzo remoto A %s\n" 
					"porta locale B %d    porta remota B %d     indirizzo remoto B %s\n",
					numero_porta_localeA, port_number_remoteA, string_IP_remote_addressA,
					numero_porta_localeB, port_number_remoteB, string_IP_remote_addressB
					);
			fflush(stdout);
		}
		else  /* argc==4*/
		{
			connect_from_2_directions=0;
			printf(
					"porta locale A %d    porta remota A %d     indirizzo remoto A %s\n",
					numero_porta_localeA, port_number_remoteA, string_IP_remote_addressA
					);
			fflush(stdout);
		}
	}  
	
	/* Set up signal handlers */
	if ((signal (SIGCHLD, sig_child)) == SIG_ERR)
	{
		printf ("signal(SIGCHLD, sig_child)  FALLITA:  Err: %d \"%s\"\n", errno,strerror(errno));
		return 0;
	}

	printf("proxy TCP: premere <ENTER> per interrompere\n");
	printf("           premere T <ENTER> per simulare TIMEOUT e poi <ENTER> per riprendere\n");
	fflush(stdout);

	ris=setup_socket_listening(&listenfdA, numero_porta_localeA);
	if (ris<0)
	{	
		printf ("setup_socket_listening(A) failed\n");
		exit(1);
	}
	if(connect_from_2_directions==1)
	{	
		ris=setup_socket_listening(&listenfdB, numero_porta_localeB);
		if (ris<0)
		{	
			printf ("setup_socket_listening(B) failed\n");
			exit(1);
		}
	}
	
	FD_ZERO(&fdR);
	maxfd=listenfdA;
	if(connect_from_2_directions==1)
	{	
		if(listenfdB>listenfdA)	maxfd=listenfdB;
	}

	while (1)
	{
		do {
			FD_SET(0,&fdR);
			FD_SET(listenfdA,&fdR);
			if(connect_from_2_directions==1)
				FD_SET(listenfdB,&fdR);
			ris=select(maxfd+1,&fdR,NULL,NULL,NULL);
			myerrno=errno;
		} while( (ris<0)&&(myerrno==EINTR) );
		if(ris<0)
		{
			perror("select failed: ");
			exit(1);
		}

		if(FD_ISSET(0,&fdR))
		{
			char str[256];
				
			printf("attendo caratteri ... per gets ....  ");
			fflush(stdout);
			fgets(str,255,stdin);
			if(str[0]=='T')
			{
				printf("provoca timeout");
				fflush(stdout);
				segnalasimulatimeout();
				printf(" ... attesa <ENTER> .... \n");
				fflush(stdout);
				fgets(str,255,stdin);
				segnalacontinua();
				printf("ripristina dopo timeout");
				fflush(stdout);
				continue;
			}
			else
			{
				printf("interruzione voluta dall'utente: TERMINO\n");
				fflush(stdout);
				exit(2);
			}
		}

		if(FD_ISSET(listenfdA,&fdR))
		{
			/* wait for connection request */
			printf ("accept(A)\n");
			do {
				len=sizeof(Cli);
				memset(&Cli,0,sizeof(struct sockaddr_in) );
				browserfd = accept(listenfdA, (struct sockaddr*) &Cli, &len);
			} while ((browserfd<0)&&(errno==EINTR)) ;

			if (browserfd<0)
			{	printf ("accept(A) failed, Err: %d \"%s\"\n",errno,strerror(errno));
				exit(1);
			}

			/* alloco la struttura in cui passare i parametri */
			p=malloc(sizeof(ParametriThreadPrincipali));
			if(p==NULL) {
				perror("malloc failed: ");
				exit(1);
			}
			p->browserfd=browserfd;
			strcpy(p->string_IP_remote_address,string_IP_remote_addressA);
			p->port_number_remote=port_number_remoteA;

			/* printf("Creating thread DETACHED %d\n", t); */
			ris = pthread_create (&pt, &attr, forward_bidirezionale, p );
			if (ris){
				printf("ERROR; return code A from pthread_create() is %d\n",ris);
				exit(-1);
			}
			else
			{
				add_pthread_id(pt,1);
				printf("Created A thread DETACHED ID %d\n", (int)pt );
			}
		}

		if(connect_from_2_directions==1)
		{
			if(FD_ISSET(listenfdB,&fdR))
			{
				/* wait for connection request */
				printf ("accept(B)\n");
				do {
					len=sizeof(Cli);
					memset(&Cli,0,sizeof(struct sockaddr_in) );
					browserfd = accept(listenfdB, (struct sockaddr*) &Cli, &len);
				} while ((browserfd<0)&&(errno==EINTR)) ;

				if (browserfd<0)
				{	printf ("accept(B) failed, Err: %d \"%s\"\n",errno,strerror(errno));
					exit(1);
				}

				/* alloco la struttura in cui passare i parametri */
				p=malloc(sizeof(ParametriThreadPrincipali));
				if(p==NULL) {
					perror("malloc failed: ");
					exit(1);
				}
				p->browserfd=browserfd;
				strcpy(p->string_IP_remote_address,string_IP_remote_addressB);
				p->port_number_remote=port_number_remoteB;

				/* printf("Creating thread DETACHED %d\n", t); */
				ris = pthread_create (&pt, &attr, forward_bidirezionale, p );
				if (ris){
					printf("ERROR; return code from B pthread_create() is %d\n",ris);
					exit(-1);
				}
				else
				{
					add_pthread_id(pt,3);
					printf("Created B thread DETACHED ID %d\n", (int)pt );
				}
			}
		}

	}
	return(0);
}




