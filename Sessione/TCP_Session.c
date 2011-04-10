#define __TCP_SESSION_C__

#include "TCP_Session.h"
#include "socket_map_list.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/ioctl.h>


/* "QUESTO CODICE E' SOLO UN ESEMPIO che
    NON RISPONDE ALLE SPECIFICHE */

/* 
	#define VERBOSE
*/

/* UTILITY, solo per uso interno a questo modulo */
int ReserveFileDescriptor(int *pfd);

static struct socket_map_list socketmap_listhead;

int read_errno;
int select_exit=0;


/* FINE UTILITY */

int	Init_TCP_Session_Module(void *ptr) 
{ 
	
	socket_map_init(&socketmap_listhead);

	return(1);
}



/* Socket: alloca lo spazio per una struttura di tipo socket_map_list
esegue la socket di sistema e assegna correttamente i campi del nodo della lista.

Valori di ritorno:
	- -1 in caso di errore della socket di sistema;
	- int (il fd pubblico), altrimenti.
 */

int	Socket(int domain, int type, int protocol)
{
	int fd_pub;
	struct socket_map_list *current_socket;
	
	current_socket = (socket_map_list *) malloc(sizeof(socket_map_list));
	socket_map_init(current_socket);
	fd_pub = socket(domain,type,protocol);
	
	if (fd_pub < 0) return SOCKET_ERROR;
	
	set_socket_fd(current_socket,fd_pub,fd_pub,0);	
	set_socket_info(current_socket,domain,type,protocol);
	
	socket_map_list_add_tail(current_socket,&socketmap_listhead);
	
	printf ("TCP: fd pubblico generato %d\n", fd_pub);
	printf ("TCP: fd privato generato %d\n", fd_pub);

#ifdef VERBOSE
	printf ("TCP: fd pubblico generato %d\n", fd_pub);
	printf ("TCP: fd privato generato %d\n", fd_pub);
	printf ("TCP: il primo fd_pub in lista è %d\n",list_next(&socketmap_listhead)->fd_pub);
	printf ("TCP: il primo fd_priv in lista è %d\n",list_next(&socketmap_listhead)->fd_priv);
	printf ("TCP: il tipe in lista è %d\n",list_next(&socketmap_listhead)->type);
#endif
	
	return fd_pub;
}



/* TCP_Session_IPv4_Socket: alloca lo spazio per una struttura di tipo socket_map_list
esegue la socket di sistema e assegna correttamente i campi del nodo della lista.

Valori di ritorno:
	- -1 in caso di errore della socket di sistema o della creazione del fd pubblico;
	- int (il fd pubblico), altrimenti.
 */
int	TCP_Session_IPv4_Socket(void)
{
	int fd_pub,fd_priv;
	struct socket_map_list *current_socket;

	current_socket = (socket_map_list *) malloc(sizeof(socket_map_list));
	socket_map_init(current_socket);
	
	fd_priv = socket(AF_INET, SOCK_STREAM, 0);
	ReserveFileDescriptor(&fd_pub);

	if (fd_priv < 0) return SOCKET_ERROR;
	if (fd_pub < 0) return SOCKET_ERROR;

	set_socket_fd(current_socket,fd_pub,fd_priv,1);
	set_socket_info(current_socket,AF_INET,SOCK_STREAM,0);

	socket_map_list_add_tail(current_socket,&socketmap_listhead);
	
	printf ("TCPS: fd pubblico generato %d\n", fd_pub);
	printf ("TCPS: fd privato generato %d\n", fd_priv);

#ifdef VERBOSE
	printf ("TCPS: fd pubblico generato %d\n", fd_pub);
	printf ("TCPS: fd privato generato %d\n", fd_priv);
	printf ("TCPS: il primo fd_pub in lista è %d\n",list_next(&socketmap_listhead)->fd_pub);
	printf ("TCPS: il primo fd_priv in lista è %d\n",list_next(&socketmap_listhead)->fd_priv);
	printf ("TCPS; il tipe in lista è %d\n",list_next(&socketmap_listhead)->type);
	printf ("TCPS: il secondo fd_pub in lista è %d\n",list_next(list_next(&socketmap_listhead))->fd_pub);
	printf ("TCPS: il secondo fd_priv in lista è %d\n",list_next(list_next(&socketmap_listhead))->fd_priv);
#endif
	
	return fd_pub;

}


int	SetsockoptReuseAddr(int s)
{
	int OptVal, ris;

	s = get_real_fd(&socketmap_listhead,s);

	if(s < 0) return 0;

	/* avoid EADDRINUSE error on bind() */
	OptVal = 1;
	ris = setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char *)&OptVal, sizeof(OptVal));
	if (ris != 0 )  {
		printf ("setsockopt() SO_REUSEADDR failed, Err: %d \"%s\"\n", errno,strerror(errno));
		return(0);
	}
	else
		return(1);
}

int	GetsockoptReuseAddr(int s, int *pFlag)
{

	
	int OptVal, ris;
	unsigned int OptLen;

	s = get_real_fd(&socketmap_listhead,s);
	if(s < 0) return 0;

	ris = getsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char *)&OptVal, &OptLen );
	if (ris != 0 )  {
		printf ("getsockopt() SO_REUSEADDR failed, Err: %d \"%s\"\n", errno,strerror(errno));
		fflush(stdout);
		return(0);
	}
	else {
		*pFlag=OptVal;
		return(1);
	}
}

int	SetNoBlocking(int s)
{
	int flags;

	if ( (flags=fcntl(s,F_GETFL,0)) <0 ) {
		printf ("fcntl(F_GETFL) failed, Err: %d \"%s\"\n", errno,strerror(errno));
		return(0);
	}
	flags |= O_NONBLOCK; 
	if ( fcntl(s,F_SETFL,flags) <0 ) {
		printf ("fcntl(F_SETFL) failed, Err: %d \"%s\"\n", errno,strerror(errno));
		return(0);
	}
	return(1);
}

int	SetBlocking(int s)
{
	int flags;

	if ( (flags=fcntl(s,F_GETFL,0)) <0 ) {
		printf ("fcntl(F_GETFL) failed, Err: %d \"%s\"\n", errno,strerror(errno));
		return(0);
	}
	flags &= (~O_NONBLOCK);
	if ( fcntl(s,F_SETFL,flags) <0 ) {
		printf ("fcntl(F_SETFL) failed, Err: %d \"%s\"\n", errno,strerror(errno));
		return(0);
	}
	return(1);
}

int	IsBlocking(int s, int *pIsBlocking)
{
	int flags;

	if ( (flags=fcntl(s,F_GETFL,0)) <0 ) {
		printf ("fcntl(F_GETFL) failed, Err: %d \"%s\"\n", errno,strerror(errno));
		return(0);
	}
	flags &= O_NONBLOCK;
	/* ora flags e' diverso da zero se il socket e' NON bloccante */ 
	if(flags)	*pIsBlocking=0;
	else		*pIsBlocking=1;
	return(1);
}




/* Bind: prende in input un fd in uscita da una socket, lo mappa nel privato,
ne prende la relativa connessione, assegna i campi della struttura con quelli del
chiamante e fa la bind di sistema.

Valori di ritorno:
	- -1 in caso di errore nel valore del fd in input o in caso di fallimento della
	   bind di sistema;
	- 0, altrimenti.
*/

int	Bind(int  sockfd, struct sockaddr *my_addr, socklen_t addrlen)
{

	socket_map_list *tmp;

	sockfd = get_real_fd(&socketmap_listhead,sockfd);

	if(sockfd < 0) return SOCKET_ERROR;

	printf("Bind\n");

	tmp=get_connection(&socketmap_listhead,sockfd);

	set_bind_info(tmp,my_addr,addrlen);

	printf("sa_family: %d",my_addr->sa_family);
	printf("addrlen %d", addrlen);
	
	return(bind(sockfd,my_addr,addrlen));
}




/* Listen: mappa il fd in ingresso (in uscita da una bind) nel corrispondente
fd privato ed esegue la listen di sistema.

Valori di ritorno:
	- -1 in caso di valore errato del fd o in caso di fallimento della 
	   listen di sistema;
	- 0, altrimenti.
*/

int	Listen(int s, int backlog)
{
	s = get_real_fd(&socketmap_listhead,s);

	if(s < 0) return SOCKET_ERROR;

	printf("Listen\n");

	return(listen(s,backlog));
}




/* Accept: viene eseguita solamente una volta per connessione (la prima).
Mappa il fd in ingresso (in uscita da una listen) nel corrispondente
fd privato. Prende la connessione relativa a quel fd privato ed assegna ai corretti
campi della struttura connessione i valori in ingresso, esegue la accept di sistema
ed assegna quel fd ai campi relativi nella connessione.

Valori di ritorno:
	- -1 in caso di errore nel valore del fd in ingresso o della accept di sistema;
	- int (fd creato dalla accept).
*/

int	Accept(int   s,  struct  sockaddr  *addr,  socklen_t *addrlen)
{
	int fd_accept_pub, fd_accept_priv, myerrno;
	socket_map_list *tmp_struct;

	s = get_real_fd(&socketmap_listhead,s);	

	if(s < 0){ 

			printf("::::::::::: Accept in errore... s < 0 \n");
			return SOCKET_ERROR;

	}

	tmp_struct = get_connection(&socketmap_listhead, s);

	set_accept_info(tmp_struct,addr,addrlen);
	
	printf("Accept(%d) ... \n",s);
	fflush(stdout);
	
	fd_accept_priv=accept(s,addr,addrlen);
	myerrno=errno;
	ReserveFileDescriptor(&fd_accept_pub);
	
	set_accept_fd(tmp_struct,fd_accept_pub,fd_accept_priv);	


/*#ifdef VERBOSE*/
	printf("fine Accept(%d)\n",s);
	fflush(stdout);
/*#endif*/

	printf("fd_accept_pub %d pub: %d private: %d\n", tmp_struct->fd_accept_pub, tmp_struct->fd_pub, tmp_struct->fd_priv);

	errno=myerrno;

	return(fd_accept_pub);
}


int 	ReConnect (socket_map_list *tmp_connect){
	
	int new_fd_priv, ris;
	
	Close(tmp_connect->fd_priv);

	printf("::::::::::: Tentativo di Reconnect in corso... \n");
		
	new_fd_priv = socket(tmp_connect->info_domain, tmp_connect->info_type, tmp_connect->info_protocol);

	if(new_fd_priv < 0){	

		printf("::::::::::: Reconnect in errore... new_fd_priv < 0 \n");

	}

	printf("::::::::::: Reconnect: risultato fd_priv socket di sistema: %d \n",new_fd_priv);		
	
	set_connect_fd(tmp_connect,new_fd_priv);	
					
	printf("::::::::::: ReConnect(%d) ... \n...FD_PUB = %d \n",new_fd_priv, tmp_connect->fd_pub);
	fflush(stdout);	

	ris=connect(new_fd_priv,&(tmp_connect->info_server),tmp_connect->dim_info);

	sleep(1);	
			
	printf("::::::::::: fine ReConnect(%d) ris_connect = %d \n",new_fd_priv, ris);

	fflush(stdout);

	return(ris);

}


int	Connect(int sockfd, const struct sockaddr *serv_addr, socklen_t addrlen){


	int ris, myerrno;
	socket_map_list *tmp_connect;
	

	if(is_ccs(&socketmap_listhead, sockfd)){ /* caso in cui è una ccs*/

		sockfd = get_real_fd(&socketmap_listhead,sockfd);

		printf("Connect(%d) ...sono ccs \n",sockfd);

		if( sockfd < 0 ) {

			printf("::::::::::: Connect in errore (caso ccs) \n");
			return SOCKET_ERROR;
		}

		
		tmp_connect = get_connection(&socketmap_listhead, sockfd);
		
		tmp_connect->cont_connect++;

		printf("Connect(%d) ... \n",sockfd);
		fflush(stdout);	
		printf("addrlen %d", addrlen);
		ris=connect(sockfd,serv_addr,addrlen);
		myerrno=errno;

		set_connect_info(tmp_connect,serv_addr,addrlen);

		if(ris < 0){	

				printf("::::::::::: Connect caso prima connessione in errore... ris < 0 \n");
				return Ripristino(sockfd);

		}


		printf("fine Connect(%d)\n",sockfd);
		fflush(stdout);
		errno=myerrno;

		return(ris);

	}else { /*inizio caso socket normale*/

	#ifdef VERBOSE
	printf("Connect(%d) ... \n",sockfd);
	fflush(stdout);
	#endif
	ris=connect(sockfd,serv_addr,addrlen);
	myerrno=errno;
	#ifdef VERBOSE
	printf("fine Connect(%d)\n",sockfd);
	fflush(stdout);
	#endif
	errno=myerrno;
	return(ris);

	}
}

ssize_t	Read(int fd, void *buf, size_t count){

	/* Se e il client che chiama la read viene chiamata con il fd della socket 
	   quindi la ricerca del fd si limitera ai campi fd_pub e fd_priv
	*/

	/* Se e il server che chiama la read viene chiamata con il fd della accept 
	   quindi la ricerca del fd riguardera il campo fd_pub fd_priv e fd_accept
	*/
	
	int ris;
	int new_fd=0;
	socket_map_list * tmp;	

	fd = get_real_fd(&socketmap_listhead,fd);

	printf("fd readdddddd: %d\n", fd);
	if(fd < 0) return SOCKET_ERROR;


/*#ifdef VERBOSE*/
	printf("/+++++++++++ Richiesta di read dal sistema (su fd = %d,.., di count = %d)\n",fd,count);
	fflush(stdout);
/*#endif*/

	tmp=get_connection(&socketmap_listhead, fd);
	
	ris=Read_from_packet(fd,buf,count);
	

/*#ifdef VERBOSE*/
	printf("/+++++++++++ fine Read di sistema (su fd = %d,..,di count = %d)\n/+++++++++++ con dati letti dal pkt = %d\n",fd,count,ris);
	fflush(stdout);
/*#endif*/
	sleep(1);
	if (ris <= 0){

		new_fd=Ripristino(fd);

		switch (new_fd){
			case SOCKET_ERROR: return SOCKET_ERROR;
			case 0: return Read(fd,buf,count);
			default: return Read(new_fd,buf,count); 
		}
	}

	tmp->readed_byte += ris; 

	printf("/+++++++++++ READED_BYTE %d \n", tmp->readed_byte);
	printf("/+++++++++++ fine Read di sistema abbiamo letto %d \n", ris);

	errno=read_errno;

	return(ris);
}




ssize_t Ripristino(int fd){

	socket_map_list * tmp;
	int ris_connect, new_fd_connect;
	int ntentativi=0;
	int tmp_accept_fd;
	int ntentativi_accept =0;

	int ris_select;

	fd_set rset,wset;
	socket_map_list *tmp_list;

	struct timeval timeout;
	timeout.tv_sec=RETRY_INTERVAL_SECONDS;
	timeout.tv_usec=0;

	
	printf("******* Ripristino connessione in corso...\n");
					
	tmp=get_connection(&socketmap_listhead, fd);
	fd=tmp->fd_pub;

	if(is_ccs(&socketmap_listhead, fd)){
		if(tmp->fd_accept_pub==0){

			

			while(ntentativi < MAX_TENTATIVI){
	
			
				printf("TENTATIVO DI RICONNESSIONE: %d \n", ntentativi);

				ris_connect=0;
			

					for_each(tmp_list,&socketmap_listhead){

						if(tmp_list->fd_accept_pub==0 && tmp_list->type==1){

							ris_connect|=ReConnect(tmp_list);


						}


					}


				
					if(ris_connect==0){
						printf("ok reconnect \n");
						ntentativi=0;
						break;
					}

				ntentativi++;

				sleep(RETRY_INTERVAL_SECONDS);

			}

			if(ntentativi == MAX_TENTATIVI){
			
				printf("chiudo....tentativi di riconnessione esauriti");
				return SOCKET_ERROR;

			}else{

				return ris_connect;

			}


		}else{

			printf("reACCEPT....fd(%d)\n", fd);

			
			tmp_accept_fd=tmp->fd_priv;	

			FD_ZERO(&rset);
			FD_SET(tmp_accept_fd, &rset);
			FD_ZERO(&wset);
			FD_SET(tmp_accept_fd, &wset);

			do{

			timeout.tv_sec=RETRY_INTERVAL_SECONDS;
			timeout.tv_usec=0;

			printf("SELECT SU: %d\n", tmp_accept_fd);
			ris_select=select(tmp_accept_fd+1,&rset,NULL,NULL,&timeout);

			FD_ZERO(&rset);
			FD_SET(tmp_accept_fd, &rset);


			ntentativi_accept++;

			/*if(select_exit==1){break;}*/

			printf("TENTATIVO NUMERO: %d SU FD:%d RIS_SELECT: %d\n", ntentativi_accept,tmp_accept_fd, ris_select);

			}while(ris_select<=0 && ntentativi_accept < MAX_TENTATIVI);

			if(ntentativi_accept==MAX_TENTATIVI && ris_select <= 0){

				printf("chiudo....tentativi di riconnessione esauriti\n");
				return SOCKET_ERROR;
			}else{

			
			/*if(select_exit==1){select_exit=0;}else{select_exit=1;}*/
			printf("FD_PRONTO!!!! FACCIO LA REACCEPT\n");
			new_fd_connect = ReAccept(fd, &(tmp->cli_addr), &(tmp->cli_addrlen));

			printf("reACCEPT_ripristino new fd (%d)\n",new_fd_connect);

			return new_fd_connect; 

			}
		}

	}else{ /*inizio caso socket normale*/

		return SOCKET_ERROR;

	}
				
}

ssize_t ReAccept (int   s,  struct  sockaddr  *addr,  socklen_t *addrlen){

	int ris, myerrno,closeris;
	socket_map_list *tmp_struct;

	s = get_real_fd(&socketmap_listhead,s);	

	if(s < 0){ 

		printf("::::::::::: reAccept in errore... s < 0 \n");

		return SOCKET_ERROR;

	}

	tmp_struct = get_connection(&socketmap_listhead, s);

	set_accept_info(tmp_struct,addr,addrlen);	

	printf("reAccept(%d) ... \n",s);
	fflush(stdout);
	
	ris=accept(s,addr,addrlen);
	myerrno=errno;

	if(ris <=0){

		printf("LA REACCEPT SBAGLIA LA CHIAMATA ALLA ACCEPT");
	}
	
	closeris=Close(tmp_struct->fd_accept_priv);
	printf("CLOSE SU VECCHIO ACCEPT PRIV(%d) ... \n",closeris);

	set_accept_fd(tmp_struct,tmp_struct->fd_accept_pub,ris);
	
	printf("fine reAccept(%d)\n",s);

	printf("fd_accept %d pub: %d private: %d\n", tmp_struct->fd_accept_pub, tmp_struct->fd_pub, tmp_struct->fd_priv);
	errno=myerrno;

	return(ris);

}




/* Read_from_packet:	legge mano a mano i pacchetti che pervengono e ne invia l'ack.

CARICAMENTO PKT:	prepara un nuovo buffer, legge un pacchetto dal buffer di sistema
			e gestisce gli eventuali errori. Infine estrae le informazioni di
			header, checkpoint e massimo id pacchetto possibile.

CONTROLLO PKTs DOPPI:	manda all'altro end-system un ack per quel pacchetto, che in realtà
			viene scartato, per passare alla lettura (comprensiva di gestione
			errori) del pacchetto successivo.

CASO 1 ... count < dati nel pkt:
			ritorna "count" al chiamante e aggiorna la quota (il segnalibro).

CASO 2 ... count >= dati nel pkt:
			invia un ack per quel pkt all'altro end-system, gestisce le variabili
			per il controllo di eventuali pacchetti doppi, azzera la quota ed
			infine ritorna al chiamante il totale byte letti.

Valori di ritorno:
	- -1 in caso di errore di lettura pkt o scrittura ack;
	- int (byte letti) altrimenti.
*/

ssize_t Read_from_packet(int fd, void *buf, size_t count){

	int checkpoint,letti,ris;
	int header = 0;
	int count_spezzato=0;
	
	printf("::::::::::: Read from Packet di count = %d \n",count);

	socket_map_list * tmp;	
	tmp=get_connection(&socketmap_listhead, fd);

	
	/*************************************/
	/********* CARICAMENTO PKT ***********/
	/*************************************/
	if(tmp->quota==0){

		set_new_buffer(tmp,PROTOCOL_PKT_DIM);
				
		printf("::::::::::: Read from Packet inizio quota = 0\n");
		
		letti=read(fd,&tmp->buffer[0],PROTOCOL_PKT_DIM);
		read_errno=errno;
		printf("::::::::::: Read from Packet: ha letto %d\n", letti);
										
		if (letti != PROTOCOL_PKT_DIM) {
			printf("::::::::::: Read from Packet: letti è in ERRORE (diverso da protocol_pkt_dim)\n");
			return SOCKET_ERROR;						
		}
				
	}
		
	get_header_info(tmp,&header,&checkpoint);
	
	
	
	/*************************************/
	/******* CONTROLLO PKTs DOPPI ********/
	/*************************************/
	
	if(checkpoint == tmp->lastid){
	
		printf("::::::::::: PACCHETTO DOPPIO, REINVIO L'ACK %d\n", checkpoint);		
		ris=write(fd,&checkpoint,ID_DIM);
			
		if(ris != ID_DIM){ 
			printf("::::::::::: Read from Packet: SEND_ACK_ERROR!!!(minore di 0) \n");
			return SOCKET_ERROR; 
		}
		
		printf("::::::::::: PACCHETTO DOPPIO, LO SCARTO E MI RIMETTO IN READ\n");
		letti=read(fd,&tmp->buffer[0],PROTOCOL_PKT_DIM);
		read_errno=errno;
		printf("::::::::::: Read from Packet: ha letto %d\n", letti);
										
		if (letti != PROTOCOL_PKT_DIM) {
			printf("::::::::::: Read from Packet: letti è in ERRORE (diverso da protocol_pkt_dim)\n");
			return SOCKET_ERROR;
		}

		get_header_info(tmp,&header,&checkpoint);			
	
	}



	/*************************************/
	/************** CASO 1 ***************/
	/******* count < dati nel pkt ********/
	/*************************************/

	printf("::::::::::: Read from Packet: quota:  %d, count: %d , header: %d\n", tmp->quota, count, header);
					
	if(((tmp->quota)+count) < header){

		printf("::::::::::: Read from Packet: count < quantità dati nel pkt\n");
		sleep(2);
		memcpy(&buf[0],&tmp->buffer[HDR_DIM+ID_DIM+MAX_ID_DIM+(tmp->quota)],count); 
		update_quota(tmp,count);
		return count;
	}




	/*************************************/
	/************** CASO 2 ***************/
	/******* count >= dati nel pkt ********/
	/*************************************/

	printf("::::::::::: INVIO L'ACK DEL PACCHETTO %d\n", checkpoint);
	ris=write(fd,&checkpoint,ID_DIM);
	
	if(ris != ID_DIM){
		printf("::::::::::: Read from Packet: SEND_ACK_ERROR!!!(minore di 0) \n");
		return SOCKET_ERROR; 
	}
		
	update_lastid(tmp,checkpoint);
	
	memcpy(&buf[0],&tmp->buffer[HDR_DIM+ID_DIM+MAX_ID_DIM+(tmp->quota)],(header-(tmp->quota))); 

	count_spezzato=header-get_quota(tmp);
	
	reset_quota(tmp);
	reset_buffer(tmp);

	printf("::::::::::: Read from Packet: FINE ritorna = %d \n",count_spezzato);
	return count_spezzato;		
			
}



ssize_t	Write(int fd, const void *buf, size_t count){

	/* Se e il client che chiama la read viene chiamata con il fd della socket 
	   quindi la ricerca del fd si limitera ai campi fd_pub e fd_priv
	*/

	/* Se e il server che chiama la read viene chiamata con il fd della accept 
	   quindi la ricerca del fd riguardera il campo fd_pub fd_priv e fd_accept
	*/
	
	int ris, myerrno,n_pkt_to_send,ack_ok,header,return_sended;
	int ack=0;
	int new_fd=0;
	char single_pkt[PROTOCOL_PKT_DIM];
	socket_map_list * tmp;

	printf("prima di get_real_fd %d",fd);

	fd = get_real_fd(&socketmap_listhead,fd);
	
	if(fd < 0) return SOCKET_ERROR;

	printf("dopo get_real_fd %d",fd);
	
	tmp=get_connection(&socketmap_listhead, fd);

	printf("Write(%d,..,%d) ... \n",fd,count);
	fflush(stdout);
	
	n_pkt_to_send = get_pkt_number(count);
	tmp->max_id = n_pkt_to_send;

	printf("numero di pacchetti da inviare per questa Write = %d\n",n_pkt_to_send);			
	sleep(1);

	while(tmp->checkpoint != n_pkt_to_send){
	
		memset(&single_pkt,0,PROTOCOL_PKT_DIM);
		header = get_current_body_bytes(count,tmp->checkpoint);
		memcpy(&single_pkt,&header,HDR_DIM);
		memcpy(&single_pkt[HDR_DIM],&tmp->checkpoint,ID_DIM);
		memcpy(&single_pkt[HDR_DIM+ID_DIM],&tmp->max_id,MAX_ID_DIM);
		memcpy(&single_pkt[HDR_DIM+ID_DIM+MAX_ID_DIM],&buf[tmp->checkpoint*PKT_DIM],get_current_body_bytes(count,tmp->checkpoint));

		printf("faccio la write piccola e voglio scrivere %d \n",PROTOCOL_PKT_DIM);

		ris=send(fd,&single_pkt,PROTOCOL_PKT_DIM,MSG_NOSIGNAL);
		myerrno=errno;
		printf("in realta scrivo %d \n",ris);
		printf("VALORE CHECKPOINT %d \n", tmp->checkpoint);
		
		if (ris < PROTOCOL_PKT_DIM) {

			printf("Risultato SEND nella write: %d < dim pkt \n");

			tmp=get_connection(&socketmap_listhead, fd);
			fd=tmp->fd_pub;
			new_fd=Ripristino(fd);
			printf("return ripristino %d", new_fd);
			switch (new_fd){
				case SOCKET_ERROR: return SOCKET_ERROR;
				case 0: return Write(fd,buf,count);
				default: return Write(new_fd,buf,count); 
			}


		}

		if (ris == PROTOCOL_PKT_DIM) {
			
			ack_ok=read(fd,&ack,ACK_DIM);
			

			printf("SIZE ACK: %d \n",ack_ok);
			printf("dati letti ACK: %d HEADER:%d \n",ack,header);

			if(ack_ok==0){

				tmp=get_connection(&socketmap_listhead, fd);
				fd=tmp->fd_pub;
				new_fd=Ripristino(fd);

				if(new_fd==0){

						return Write(fd,buf,count);

						}else{

						return Write(new_fd,buf,count);

						}

			}

			if(ack==tmp->checkpoint){

					printf("return macro\n %d", get_current_body_bytes(count,tmp->checkpoint));

					update_sended_byte(tmp,count);
			}
		
		}
		
	sleep(1);

	}	
	
	
	
	printf("fine Write(%d,..,%d)=%d\n",fd,count,ris);
	fflush(stdout);

	errno=myerrno;

	if((tmp->checkpoint) == n_pkt_to_send){
		printf("AZZERO !!!!!!!!!!!!!!!!!!!\n");
		return_sended=tmp->sended_byte;
		reset_sended_byte(tmp);
	}

	return return_sended;
}

int	Send(int s, const void *msg, size_t len, int flags)
{
	int ris, myerrno;
#ifdef VERBOSE
	printf("Send(%d,..,%d,..) ... \n",s,len);
	fflush(stdout);
#endif
	ris=send(s,msg,len,flags);
	myerrno=errno;
#ifdef VERBOSE
	printf("fine Send(%d,..,%d,..)=%d\n",s,len,ris);
	fflush(stdout);
#endif
	errno=myerrno;
	return(ris);
}

int	AvailableBytes(int s, int *pnum)
{
	if (ioctl (s,FIONREAD,pnum)==0) 
		return(1);
	else
		return(0);
}

int	Select(int  n,  fd_set  *readfds,  fd_set  *writefds, fd_set *exceptfds, struct timeval *timeout)
{
	
	int ris, myerrno;
#ifdef VERBOSE
	printf("Select(timeout %p ) ... \n", (void*)timeout);
	fflush(stdout);
#endif
	ris=select(n,readfds,writefds,exceptfds, timeout);
	myerrno=errno;
#ifdef VERBOSE
	printf("fine Select(timeout %p )\n", (void*)timeout);
	fflush(stdout);
#endif
	errno=myerrno;
	return(ris);
}

int	Close(int fd)
{
	int ris, myerrno,fd_priv;
	
	fd_priv = get_real_fd(&socketmap_listhead,fd);

	ris=close(fd_priv);
	myerrno=errno;
	printf("Close(%d) FD PRIVATO ... \n",fd_priv);

	fflush(stdout);
	
#ifdef VERBOSE
	printf("fine Close(%d)\n",fd);
	fflush(stdout);
#endif
	errno=myerrno;
	return(ris);
}

int	CloseWait(int fd, int seconds){
	
	int ris, myerrno;
	struct linger lin;
	
#ifdef VERBOSE
	printf("CloseWait(%d) ... \n",fd);
	fflush(stdout);
#endif
	lin.l_onoff=1;
	lin.l_linger=seconds;
	ris = setsockopt(fd, SOL_SOCKET, SO_LINGER, (char *)&lin, sizeof(lin));
	myerrno=errno;
	if (ris != 0 )  
	{
		printf ("setsockopt() SO_LINGER failed, Err: %d \"%s\"\n", errno,strerror(errno));
		errno=myerrno;
		return(-1);
	}
	ris=close(fd);
	myerrno=errno;
#ifdef VERBOSE
	printf("fine CloseWait(%d)\n",fd);
	if(ris==0) printf("connessione chiusa correttamente: dati spediti\n");
	else printf("timeout scaduto, connessione non chiusa correttamente\n");
	fflush(stdout);
#endif
	errno=myerrno;
	return(ris);
}

int   CloseWait_TCP_Session_Module(void *ptr, struct timeval *timeout)
{ 
	return(1);
}

int   Close_TCP_Session_Module(void *ptr)
{ 
	free_all_connection(&socketmap_listhead);
	return(1);
}



int ReserveFileDescriptor(int *pfd)
{
	int fd;

	fd=open("/dev/null", O_RDONLY);
	if(fd>=0) 
	{
		*pfd=fd;
		return(1);
	}
	else
	{
		printf ("fcntl(F_GETFL) failed, Err: %d \"%s\"\n", errno,strerror(errno));
		return(0);
	}
}
