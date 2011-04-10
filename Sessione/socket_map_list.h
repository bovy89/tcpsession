#ifndef __SOCKET_MAP_LIST__
#define __SOCKET_MAP_LIST__
#include <string.h>
#include "const.h"


/*macro per write e read */
#define get_pkt_number(buffer) \
	 buffer%PKT_DIM == 0 ? buffer/PKT_DIM : (buffer/PKT_DIM)+1
#define get_current_body_bytes(buffer,sended_pkt) \
	 (buffer - (PKT_DIM*sended_pkt)) < PKT_DIM ? (buffer - (PKT_DIM*sended_pkt)) : PKT_DIM


/*struttura socket*/

typedef struct socket_map_list {

	int fd_pub;
	int fd_priv;
	int fd_accept_pub;
	int fd_accept_priv;
	
	int type;	/* 0= normale 1=CCS*/
	struct socket_map_list * next;
	struct socket_map_list * prev;

	
	/* contatore. ce ne serviamo per verificare se è la prima
	connessione o siamo nel caso in cui e' una reconnect */
	int cont_connect;
	

	/*parte reconnect (struttura indirizzo server e dimensione indirizzo)*/
	struct sockaddr info_server;
	socklen_t dim_info;

	/* dati socket del chiamante */
	int info_domain;
	int info_type; 
	int info_protocol;
	
	/* dati bind del chiamante */
	struct sockaddr addr_bind;
	socklen_t addrlen_bind;
	
	/* dati accept del chiamante */
	struct sockaddr cli_addr;
	socklen_t cli_addrlen;


	/* gestione pacchetti */
	int quota;
	int checkpoint;
	char * buffer;
	int sended_byte;
	int readed_byte;
	int lastid;
	int max_id;
	

} socket_map_list;



/*api sulle liste*/

void socket_map_init( struct socket_map_list * new){

	new->fd_pub = 0;
	new->fd_priv = 0;
	new->type = 0;
	new->next = new;
	new->prev = new;
	new->cont_connect = 0;
	new->fd_accept_pub = 0;
	new->dim_info = 0;
	new->addrlen_bind = 0;
	new->info_domain = 0;
	new->info_type = 0;
	new->info_protocol = 0;
	new->quota=0;
	new->checkpoint=0;
	new->buffer=NULL;
	new->sended_byte=0;
	new->readed_byte=0;
	new->fd_accept_priv = 0;
	new->lastid=-1;
	new->max_id=0;


}

void __list_add(struct socket_map_list *new,
		struct socket_map_list *prev,
		struct socket_map_list *next)
{
	next->prev = new;
	new->next = next;
	new->prev = prev;
	prev->next = new;
}

void socket_map_list_add(struct socket_map_list *new, struct socket_map_list *head)
{
	__list_add(new, head, head->next);
}

void socket_map_list_add_tail(struct socket_map_list *new, struct socket_map_list *head)
{
	__list_add(new, head->prev, head);
}


void __list_del(struct socket_map_list * prev, struct socket_map_list * next)
{
	next->prev = prev;
	prev->next = next;
}
void list_del(struct socket_map_list *entry)
{
	__list_del(entry->prev, entry->next);
}
int list_is_last(const struct socket_map_list *list,
		const struct socket_map_list *head)
{
	return list->next == head;
}
int list_empty(const struct socket_map_list *head)
{
	return head->next == head;
}

socket_map_list * list_next(const struct socket_map_list *current)
{

	if (list_empty(current))
		return current;
	else
		return current->next;
}


socket_map_list * list_prev(const struct socket_map_list *current)
{
	if (list_empty(current))
		return current;
	else
		return current->prev;
}


/* macro sulle liste */


#define for_each(pos, head) \
	for (pos = list_next(head); pos != (head); pos = list_next(pos))




/* api sulle socket_map_list */

int is_ccs(const struct socket_map_list *current, int fd){
	socket_map_list *tmp;


	for_each(tmp, current){
	
		if (tmp->fd_pub == fd || tmp->fd_priv == fd) {
		
							return tmp->type;
		}
	}
	
	
	return -1;

}



int get_real_fd(const struct socket_map_list *current, int fd){

	socket_map_list *tmp;
	
	for_each(tmp, current){
	
		if (tmp->fd_pub == fd || tmp->fd_priv == fd) {

								return tmp->fd_priv;
	
		}else if(tmp->fd_accept_pub == fd || tmp->fd_accept_priv==fd) {
			
								return tmp->fd_accept_priv;

			}

	}

	return -1;

}


int stampa_lista(const struct socket_map_list *current){

	socket_map_list *tmp;
	
	for_each(tmp, current){

	printf("FD_PUB: %d  FD_PRIV: %d  FD_ACCEPT_PUB: %d FD_ACCEPT_PRIV: %d\n" ,tmp->fd_pub,tmp->fd_priv,tmp->fd_accept_pub, tmp->fd_accept_priv);

	}

}



socket_map_list *get_connection(const struct socket_map_list *current, int fd){

	socket_map_list *tmp;

	for_each(tmp, current){
		if (tmp->fd_priv == fd) {
			
					return tmp;

		}else if(tmp->fd_pub == fd) {

					return tmp;

			}else if (tmp->fd_accept_pub == fd || tmp->fd_accept_priv == fd) {

					return tmp;
				}
	
	}

	return NULL;    

}

void set_new_buffer(socket_map_list * current_socket, int size){
	
	current_socket->buffer = (char *) malloc(sizeof(char)*size);
	memset(&current_socket->buffer[0],0,size);
}


void set_socket_fd (socket_map_list * current_socket, int fd_pub, int fd_priv, int type){

	current_socket->fd_pub =fd_pub;
	current_socket->fd_priv = fd_priv;
	current_socket->type = type;
	
}

void set_socket_info (socket_map_list * current_socket, int domain, int type, int protocol){

	current_socket->info_domain=domain;
	current_socket->info_type=type;
	current_socket->info_protocol=protocol;
}


void set_bind_info (socket_map_list *tmp, struct sockaddr *my_addr, socklen_t addrlen){

	tmp->addr_bind = *my_addr;
	tmp->addrlen_bind=addrlen;
}


void set_accept_info (socket_map_list *tmp_struct,struct  sockaddr  *addr,  socklen_t *addrlen){

	tmp_struct->cli_addr=*addr;
	tmp_struct->cli_addrlen=*addrlen;
}


void set_accept_fd (socket_map_list *tmp_struct, int ris_pub, int ris_priv){

	tmp_struct->fd_accept_pub=ris_pub;
	tmp_struct->fd_accept_priv=ris_priv;
}


void set_connect_info (socket_map_list *tmp_connect, const struct sockaddr *serv_addr, socklen_t addrlen){

	tmp_connect->info_server=*serv_addr;
	tmp_connect->dim_info=addrlen;
}


void set_connect_fd (socket_map_list *tmp_connect, int fd_priv){
	
	tmp_connect->fd_priv=fd_priv;
}


void update_quota(socket_map_list *tmp_connect,int count){
	
	tmp_connect->quota+=count;	
	
}


void reset_quota(socket_map_list *tmp_connect){
	
	tmp_connect->quota=0;	

}

int get_quota(socket_map_list *tmp_connect){
	
	return tmp_connect->quota;	
	
}

void reset_buffer(socket_map_list *tmp_connect){
	
	free(tmp_connect->buffer);	
	
}

void update_sended_byte(socket_map_list *tmp_connect,int count){
	
	tmp_connect->sended_byte+=get_current_body_bytes(count,tmp_connect->checkpoint);
	
	printf("sended_byte %d \n",tmp_connect->sended_byte);

	tmp_connect->checkpoint++;

}

void reset_sended_byte(socket_map_list *tmp_connect){

		tmp_connect->checkpoint=0;
		tmp_connect->sended_byte=0;

}


void get_header_info(socket_map_list *tmp, int *header, int *checkpoint){

	memcpy(header,&tmp->buffer[0],HDR_DIM);
	memcpy(checkpoint,&tmp->buffer[HDR_DIM],ID_DIM);
	memcpy(&tmp->max_id,&tmp->buffer[HDR_DIM+ID_DIM],MAX_ID_DIM);
	
	printf("::::::::::: CHECKPOINT PKT:%d, LAST: %d MAX_ID %d\n", *checkpoint,tmp->lastid,tmp->max_id);

}


void update_lastid(socket_map_list *tmp, int checkpoint){

	tmp->lastid = checkpoint;	
	if(tmp->lastid == (tmp->max_id - 1)) {
		
		tmp->lastid=-1;
		tmp->max_id=0;
	}
}

/*void free_connection(const struct socket_map_list *current,int fd){
	
	int real_fd;
	socket_map_list *connection_to_remove;

	real_fd=get_real_fd(current,fd);
	connection_to_remove=get_connection(current,real_fd);
	list_del(connection_to_remove);
	free(connection_to_remove);

}*/

void free_all_connection(const struct socket_map_list *current){
	
	socket_map_list *connection_to_remove;

	for_each(connection_to_remove, current){
		close(connection_to_remove->fd_pub);
		free(connection_to_remove);
	}

}



/*void test(){
	static struct socket_map_list socketmap;
	int listempty;
	static struct socket_map_list socketmap2;
	static struct socket_map_list socketmap3;
	socket_map_list * temp,*temp2,*temp3;
	temp = &socketmap;
	temp2 = &socketmap2;
	temp3 = &socketmap3;
	socket_map_init(&socketmap);
	listempty = list_empty(&socketmap);	
	printf("Il valore del fd_pub nella INIT è %d \n",socketmap.fd_pub);
	printf("la lista è vuota prima della add %d \n",listempty);
	socket_map_list_add(&socketmap2,&socketmap);
	socket_map_list_add(&socketmap3,&socketmap);
	listempty = list_empty(&socketmap);
	printf("la lista è vuota dopo la add %d \n",listempty);
        if (temp2 == list_next(&socketmap3)) printf("la list_next funziona \n");
	if (temp3 == list_prev(&socketmap2)) printf("la list_prev funziona \n");
	if (temp == list_next(list_prev(&socketmap))) printf("Proprietà ok \n");
	socketmap2.fd_pub = 3;
	socketmap2.fd_priv = 4;
	socketmap2.type = 1;
	socketmap3.fd_pub = 2;
	socketmap3.fd_priv = 2;
	socketmap3.type = 0;
	if(!(is_ccs(&socketmap,2))) printf("fd 2 è un socket tcp \n");
	if(is_ccs(&socketmap,3)) printf("fd 3 è un socket ccs \n");
	printf("il fd reale di 3 è %d \n",get_real_fd(&socketmap,3));
}*/

#endif 
